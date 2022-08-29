#include <iostream>
#include <fstream>

#include "Circuit.h"
#include "json.hpp"
using json = nlohmann::json;

signal_id_t signal_from_str(const std::string& s)
{
    if (s.length() != 1) throw CircuitParsingError(ILLEGAL_SIGNAL_STRING);
    const char c = s[0];
    if (c == '0' || c == '1') return static_cast<signal_id_t>(c - '0');
    else if (c == 'x' || c == 'X') return signal_id_t::S_X;
    else if (c == 'z' || c == 'Z') return signal_id_t::S_Z;
    throw CircuitParsingError(ILLEGAL_SIGNAL_STRING);
}

Circuit::Circuit(const std::string& json_file_path, const std::string& top_module_name)
{
    std::ifstream f; f.exceptions(std::ifstream::badbit);
    f.open(json_file_path);
    std::string data(std::istreambuf_iterator<char>{f}, {});
    f.close();
    auto jdata = json::parse(data);
    const auto& module = jdata.at("modules").at(top_module_name);

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
            { throw CircuitParsingError(ILLEGAL_PORT_DIRECTION); }

        // Determine the bits, make sure it is an array
        const auto& bits = value.at("bits");
        if (!bits.is_array())
            { throw CircuitParsingError(ILLEGAL_SIGNAL_LIST);}

        // Register signal name with empty signal list
        const auto emplace_it = m_name_bits.emplace(name, std::vector<signal_id_t>());

        // If we fail, it is due to a re-definition of the same name
        if (!emplace_it.second)
            { throw CircuitParsingError(ILLEGAL_NAME_REDECLARATION); }

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
            m_signals.insert(sig);
        }
    }

    /*
    // Register all cells of the circuit
    const auto& cells = module.at("cells");
    for (const auto& cell_data: cells.items())
    {
        const auto& key = cell_data.key();
        const auto& value = cell_data.value();

        const std::string name = key;
        std::string str_type = value.at("type");
        size_t pos_b = str_type.find('_') + 1;
        size_t pos_e = str_type.find('_', pos_b);
        std::string stype = str_type.substr(pos_b, pos_e - pos_b);
        std::transform(stype.begin(), stype.end(), stype.begin(), ::tolower);
        // CellType type = cell_type(stype);

    }
    */
}