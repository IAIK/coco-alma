#ifndef CIRCUIT_H
#define CIRCUIT_H

#include <string>
#include <cstdint>
#include <utility>
#include <cassert>
#include <cstring>
#include <unordered_set>
#include <unordered_map>
#include <vector>

#include "Cell.h"

class Circuit
{
private:
    std::unordered_set<signal_id_t> m_in_ports;
    std::unordered_set<signal_id_t> m_out_ports;
    std::unordered_set<signal_id_t> m_signals;
    std::vector<const Cell*> m_cells;
    std::unordered_map<std::string, std::vector<signal_id_t>> m_name_bits;
    template <typename T> signal_id_t get_signal_any(T& bit);
public:
    Circuit(const std::string& json_file_path, const std::string& top_module_name);
};

template <typename T> signal_id_t Circuit::get_signal_any(T& bit)
{
    if(bit.is_number_unsigned())
        return static_cast<signal_id_t>((uint32_t)bit);
    else if(bit.is_string()) // it must be a constant
        return signal_from_str(bit);
    throw std::logic_error(ILLEGAL_SIGNAL_TYPE);
}

#endif // CIRCUIT_H
