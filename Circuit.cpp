#include <iostream>
#include <fstream>

#include "Circuit.h"
#include "json.hpp"
using json = nlohmann::json;

Circuit::Circuit(const std::string& json_file_path, const std::string& top_module_name)
{
    std::ifstream f; f.exceptions(std::ifstream::badbit);
    f.open(json_file_path);
    std::string data(std::istreambuf_iterator<char>{f}, {});
    f.close();
    auto jdata = json::parse(data);
    const auto& module = jdata.at("modules").at(top_module_name);

    m_signals.insert(signal_id_t::S_0);
    m_signals.insert(signal_id_t::S_1);
    m_signals.insert(signal_id_t::S_X);
    m_signals.insert(signal_id_t::S_Z);

    // Register all module_ports of the circuit
    const auto& module_ports = module.at("ports");
    for (const auto& port_data: module_ports.items())
    {
        const auto& key = port_data.key();
        const auto& value = port_data.value();

        // The port name is the key
        std::string name = key;

        // Determine the direction, make sure it is valid
        std::string direction = value.at("direction");
        if (direction != "input" && direction != "output")
            { throw std::logic_error(ILLEGAL_PORT_DIRECTION); }

        // Determine the bits, make sure it is an array
        const auto& bits = value.at("bits");
        if (!bits.is_array())
            { throw std::logic_error(ILLEGAL_SIGNAL_LIST);}

        // Register signal name with empty signal list
        const auto emplace_it = m_name_bits.emplace(name, std::vector<signal_id_t>());

        // If we fail, it is due to a re-definition of the same name
        if (!emplace_it.second)
            { throw std::logic_error(ILLEGAL_NAME_REDECLARATION); }

        // Convert all the bit indexes into signal ids
        std::vector<signal_id_t>& typed_signals = emplace_it.first->second;
        typed_signals.reserve(bits.size());
        for (const auto& bit_id : bits)
            { typed_signals.push_back(get_signal_any(bit_id)); }

        // Fill the port signals and overall known signals with those we found
        std::unordered_set<signal_id_t>& direction_ports = direction == "input" ? m_in_ports : m_out_ports;
        for (const signal_id_t sig : typed_signals)
        {
            direction_ports.insert(sig);
            if (direction == "input") m_signals.insert(sig);
        }
    }

    // Register all module_cells of the circuit
    const auto& module_cells = module.at("cells");
    std::unordered_set<signal_id_t> missing;
    for (const auto& cell_data: module_cells.items())
    {
        const auto& key = cell_data.key();
        const auto& value = cell_data.value();

        const char* name = key.c_str();
        std::string str_type = value.at("type");
        cell_type_t type = cell_type_from_string(str_type);
        if(type == cell_type_t::CELL_NONE)
            { throw std::logic_error(ILLEGAL_CELL_TYPE); }

        const auto& connections = value.at("connections");
        if (is_unary(type))
        {
            signal_id_t a = get_signal_any(connections.at("A").at(0));
            signal_id_t y = get_signal_any(connections.at("Y").at(0));
            if (a == y) throw std::logic_error(ILLEGAL_CELL_CYCLE);

            if(m_signals.find(a) == m_signals.end()) { missing.insert(a); }
            assert(m_signals.find(y) == m_signals.end());
            m_signals.insert(y);
            missing.erase(y);
            m_cells.push_back(new Cell(name, type, UnaryPorts(a, y)));
        }
        else if (is_binary(type))
        {
            signal_id_t a = get_signal_any(connections.at("A").at(0));
            signal_id_t b = get_signal_any(connections.at("B").at(0));
            signal_id_t y = get_signal_any(connections.at("Y").at(0));
            if (a == y) throw std::logic_error(ILLEGAL_CELL_CYCLE);
            if (b == y) throw std::logic_error(ILLEGAL_CELL_CYCLE);

            if(m_signals.find(a) == m_signals.end()) { missing.insert(a); }
            if(m_signals.find(b) == m_signals.end()) { missing.insert(b); }
            assert(m_signals.find(y) == m_signals.end());
            m_signals.insert(y);
            missing.erase(y);
            m_cells.push_back(new Cell(name, type, BinaryPorts(a, b, y)));
        }
        else if (is_multiplexer(type))
        {
            signal_id_t a = get_signal_any(connections.at("A").at(0));
            signal_id_t b = get_signal_any(connections.at("B").at(0));
            signal_id_t s = get_signal_any(connections.at("S").at(0));
            signal_id_t y = get_signal_any(connections.at("Y").at(0));
            if (a == y) throw std::logic_error(ILLEGAL_CELL_CYCLE);
            if (b == y) throw std::logic_error(ILLEGAL_CELL_CYCLE);
            if (s == y) throw std::logic_error(ILLEGAL_CELL_CYCLE);

            if(m_signals.find(a) == m_signals.end()) { missing.insert(a); }
            if(m_signals.find(b) == m_signals.end()) { missing.insert(b); }
            if(m_signals.find(s) == m_signals.end()) { missing.insert(s); }
            assert(m_signals.find(y) == m_signals.end());
            m_signals.insert(y);
            missing.erase(y);
            m_cells.push_back(new Cell(name, type, MultiplexerPorts(a, b, s, y)));
        }
        else if (is_register(type))
        {
            signal_id_t c = get_signal_any(connections.at("C").at(0));
            signal_id_t d = get_signal_any(connections.at("D").at(0));
            signal_id_t q = get_signal_any(connections.at("Q").at(0));
            if (c == q) throw std::logic_error(ILLEGAL_CELL_CYCLE);

            if(m_signals.find(c) == m_signals.end()) { missing.insert(c); }
            if(m_signals.find(d) == m_signals.end()) { missing.insert(d); }
            assert(m_signals.find(q) == m_signals.end());

            Cell* p_cell = nullptr;
            if (!dff_has_enable(type) && !dff_has_reset(type))
            {
                p_cell = new Cell(name, type, DffPorts(c, d, q));
            }
            else if (dff_has_reset(type) && !dff_has_enable(type))
            {
                signal_id_t r = get_signal_any(connections.at("R").at(0));
                if (r == q) throw std::logic_error(ILLEGAL_CELL_CYCLE);
                if(m_signals.find(r) == m_signals.end()) { missing.insert(r); }
                p_cell = new Cell(name, type, DffrPorts(c, d, q, r));
            }
            else if (!dff_has_reset(type) && dff_has_enable(type))
            {
                signal_id_t e = get_signal_any(connections.at("E").at(0));
                if (e == q) throw std::logic_error(ILLEGAL_CELL_CYCLE);
                if(m_signals.find(e) == m_signals.end()) { missing.insert(e); }
                p_cell = new Cell(name, type, DffePorts(c, d, q, e));
            }
            else
            {
                signal_id_t r = get_signal_any(connections.at("R").at(0));
                signal_id_t e = get_signal_any(connections.at("E").at(0));
                if (r == q) throw std::logic_error(ILLEGAL_CELL_CYCLE);
                if (e == q) throw std::logic_error(ILLEGAL_CELL_CYCLE);
                if(m_signals.find(r) == m_signals.end()) { missing.insert(r); }
                if(m_signals.find(e) == m_signals.end()) { missing.insert(e); }
                p_cell = new Cell(name, type, DfferPorts(c, d, q, r, e));
            }
            m_signals.insert(q);
            missing.erase(q);
            m_cells.push_back(p_cell);
        }
    }

    // Check that all outputs are actually defined
    if (!missing.empty()) throw std::logic_error(ILLEGAL_MISSING_SIGNALS);
    for (signal_id_t sig: m_out_ports)
    {
        if (m_signals.find(sig) == m_signals.end())
            { throw std::logic_error(ILLEGAL_MISSING_SIGNALS); }
    }

    // Check that the clock edge matches on all registers
    bool found_pos_edge = false;
    bool found_neg_edge = false;
    for (const Cell* p_cell : m_cells)
    {
        if (!is_register(p_cell->type())) continue;
        const bool clock_trigger = dff_clock_trigger(p_cell->type());
        found_pos_edge |= clock_trigger;
        if (!clock_trigger) std::cout << p_cell->name() << std::endl;
        found_neg_edge |= !clock_trigger;
    }

    if (found_neg_edge && found_pos_edge)
        { throw std::logic_error(ILLEGAL_CLOCK_EDGE); }

    std::unordered_set<signal_id_t> visited_sig(m_in_ports);
    visited_sig.insert(signal_id_t::S_0);
    visited_sig.insert(signal_id_t::S_1);
    visited_sig.insert(signal_id_t::S_X);
    visited_sig.insert(signal_id_t::S_Z);

    std::unordered_set<const Cell*> visited_cell;
    std::vector<const Cell*> cell_order;
    for (const Cell* p_cell : m_cells)
    {
        if (!is_register(p_cell->type())) continue;
        cell_order.push_back(p_cell);
        visited_cell.insert(p_cell);
        visited_sig.insert(p_cell->m_ports.m_dff.m_out_q);
        // Technically this could not hold due to compiler shenanigans
        assert((&(p_cell->m_ports.m_dff.m_out_q)) == (&(p_cell->m_ports.m_dffr.m_out_q)));
        assert((&(p_cell->m_ports.m_dff.m_out_q)) == (&(p_cell->m_ports.m_dffe.m_out_q)));
        assert((&(p_cell->m_ports.m_dff.m_out_q)) == (&(p_cell->m_ports.m_dffer.m_out_q)));
    }

    while (cell_order.size() != m_cells.size())
    {
        std::cout << cell_order.size() << " " << m_cells.size() << std::endl;
        for (const Cell* p_cell : m_cells)
        {
            if (visited_cell.find(p_cell) != visited_cell.end()) continue;
            const cell_type_t type = p_cell->type();
            const Ports& ports = p_cell->m_ports;
            assert(!is_register(type));
            #define sig_visited(x) (visited_sig.find(x) != visited_sig.end())
            // Check whether the predecessors are visited, otherwise continue
            if (is_unary(type) && sig_visited(ports.m_unr.m_in_a))
            {
                visited_sig.insert(ports.m_unr.m_out_y);
            }
            else if (is_binary(type) && sig_visited(ports.m_bin.m_in_a)
                                        && sig_visited(ports.m_bin.m_in_b))
            {
                visited_sig.insert(ports.m_bin.m_out_y);
            }
            else if (is_multiplexer(type) && sig_visited(ports.m_mux.m_in_a)
                                          && sig_visited(ports.m_mux.m_in_b)
                                          && sig_visited(ports.m_mux.m_in_s))
            {
                visited_sig.insert(ports.m_mux.m_out_y);
            }
            else { continue; }

            // If predecessors are visited, add the cell
            visited_cell.insert(p_cell);
            cell_order.push_back(p_cell);
        }
    }
    m_cells = cell_order;

    // Extract the remaining names from module_netnames
    const auto& module_netnames = module.at("netnames");
    for (const auto& name_data: module_netnames.items())
    {
        const auto& key = name_data.key();
        const auto& value = name_data.value();

        // The port name is the key
        std::string name = key;

        // Determine the bits, make sure it is an array
        const auto& bits = value.at("bits");
        if (!bits.is_array())
            { throw std::logic_error(ILLEGAL_SIGNAL_LIST);}

        // Convert the bits into signals
        std::vector<signal_id_t> typed_signals;
        typed_signals.reserve(bits.size());
        for (const auto& bit_id : bits)
        { typed_signals.push_back(get_signal_any(bit_id)); }

        // If the name already exists, it is re-defined (possibly first within input ports)
        // Make sure that everything is consistent, otherwise throw an error
        const auto& found_it = m_name_bits.find(name);
        if (found_it != m_name_bits.end())
        {
            const std::vector<signal_id_t>& other = found_it->second;
            if (other.size() != typed_signals.size())
                { throw std::logic_error(ILLEGAL_NAME_REDECLARATION); }
            for (size_t i = 0; i < other.size(); i++)
                if (typed_signals[i] != other[i])
                    { throw std::logic_error(ILLEGAL_NAME_REDECLARATION); }
        }
        else
        {
            // If it was not there in the first place, add it as new
            const auto& emplace_it = m_name_bits.emplace(name, typed_signals);
            assert(emplace_it.second);
        }
    }
}