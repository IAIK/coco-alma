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

    // Register all ports of the circuit
    const auto& ports = module.at("ports");
    for (const auto& port_data: ports.items())
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

    // Register all cells of the circuit
    const auto& cells = module.at("cells");
    std::unordered_set<signal_id_t> missing;
    for (const auto& cell_data: cells.items())
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

    if (!missing.empty()) throw std::logic_error(ILLEGAL_MISSING_SIGNALS);
    for (signal_id_t sig: m_out_ports)
    {
        if (m_signals.find(sig) == m_signals.end())
            { throw std::logic_error(ILLEGAL_MISSING_SIGNALS); }
    }

}