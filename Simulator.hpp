#ifdef SIMULATOR_H

#include "Simulator.h"
#include "Symbol.h"


template <verif_mode_t mode>
inline void Simulator<mode>::insert_consts(SimulatorValueMap& vals)
{
    vals.emplace(std::piecewise_construct, std::forward_as_tuple(signal_id_t::S_0), std::forward_as_tuple(false));
    vals.emplace(std::piecewise_construct, std::forward_as_tuple(signal_id_t::S_1), std::forward_as_tuple(true));
    vals.emplace(std::piecewise_construct, std::forward_as_tuple(signal_id_t::S_X), std::forward_as_tuple(false));
    vals.emplace(std::piecewise_construct, std::forward_as_tuple(signal_id_t::S_Z), std::forward_as_tuple(false));
}

template <verif_mode_t mode>
inline void Simulator<mode>::prepare_cycle()
{
    // Prepare the inputs for the next cycle
    m_trace.emplace_back();
    SimulatorValueMap& second_cycle = m_trace.back();
    const SimulatorValueMap& first_cycle = *(++m_trace.crbegin());
    for (const signal_id_t sig : m_in_ports)
    { second_cycle.emplace(sig, +first_cycle.at(sig)); }
    m_prepared = true;
}

template <verif_mode_t mode>
Simulator<mode>::Simulator(const Circuit& circ) :
        Circuit(circ), m_solver(), m_prepared(false), m_num_lidx(0), m_and_map()
{
    // Create a dummy pre-cycle that is computed with all zero inputs and registers
    m_trace.emplace_back();
    SimulatorValueMap& first_cycle = m_trace.back();
    for (const signal_id_t sig : m_in_ports)
    { first_cycle.emplace(std::piecewise_construct, std::forward_as_tuple(sig), std::forward_as_tuple(false)); }
    for (const signal_id_t sig : m_reg_outs)
    { first_cycle.emplace(std::piecewise_construct, std::forward_as_tuple(sig), std::forward_as_tuple(false)); }

    insert_consts(first_cycle);

    for (const Cell* cell : m_cells)
    {
        if (is_register(cell->type())) continue;
        cell->eval<Value<mode>, ValueView<mode>>(first_cycle, first_cycle);
    }
}

template <verif_mode_t mode>
void Simulator<mode>::step_cycle()
{
    if (!m_prepared)
    { throw std::logic_error("Trying to step an unprepared cycle."); }
    SimulatorValueMap& curr_cycle = m_trace.back();
    const SimulatorValueMap& prev_cycle = *(++m_trace.crbegin());

    insert_consts(curr_cycle);
    for (const Cell* cell : m_cells)
    { cell->eval<Value<mode>, ValueView<mode>>(prev_cycle, curr_cycle); }
    m_prepared = false;
    std::cout << "There are " << m_solver.num_vars() << " variables and " << m_solver.num_clauses() << " clauses" << std::endl;
}

template <verif_mode_t mode>
Simulator<mode>::~Simulator()
{
    m_cells.clear();
}

template <verif_mode_t mode>
void Simulator<mode>::allocate_secrets(const Range range, const size_t num_shares)
{
    const size_t low  = range.first < range.second ? range.first : range.second;
    const size_t high = range.first < range.second ? range.second : range.first;

    if (num_shares == 0)
    { throw std::logic_error("A secret cannot have no shares"); }
    ValueVector<mode> value_vector;
    value_vector.reserve(num_shares);
    for (size_t i = low; i <= high; i++)
    {
        if (m_secrets.find(i) != m_secrets.end())
        { throw std::logic_error("Attempting to redefine a secret"); }

        value_vector.clear();
        lidx_t secret_idx = new_secret_lidx();
        m_check_lidx.emplace(i, secret_idx);
        std::cout << secret_idx << std::endl;
        {  // Create a local scope here since the value should be inaccessible later
            PropVarSetPtr p_pvs = std::make_shared<PropVarSet>(&m_solver, secret_idx);
            Symbol sym = create_symbol();
            value_vector.emplace_back(sym, p_pvs);
        }
        for (size_t sh = 1; sh < num_shares; sh++)
        {
            lidx_t mask_idx = new_mask_lidx();
            PropVarSetPtr p_pvs = std::make_shared<PropVarSet>(&m_solver, mask_idx);
            Symbol sym = create_symbol();
            std::cout << *p_pvs << std::endl;
            Value<mode> val(sym, p_pvs);
            value_vector.back() = value_vector.back() ^ val;
            value_vector.push_back(val);
        }
        m_secrets[i] = value_vector;
    }
}

template<verif_mode_t mode>
void Simulator<mode>::allocate_masks(Range range)
{
    const size_t low  = range.first < range.second ? range.first : range.second;
    const size_t high = range.first < range.second ? range.second : range.first;
    for (size_t i = low; i <= high; i++)
    {
        if (m_masks.find(i) != m_masks.end())
        { throw std::logic_error("Attempting to redefine a mask"); }

        lidx_t mask_idx = new_mask_lidx();
        PropVarSetPtr p_pvs = std::make_shared<PropVarSet>(&m_solver, mask_idx);
        Symbol sym = create_symbol();
        m_masks.emplace(std::piecewise_construct, std::forward_as_tuple(i), std::forward_as_tuple(sym, p_pvs));
    }
}

template <verif_mode_t mode>
ValueViewVector<mode> Simulator<mode>::operator[](const std::string& name)
{
    const std::vector<signal_id_t>& signals = m_name_bits.at(name);
    ValueViewVector<mode> ret_vector;
    using ValueT = typename ValueViewVector<mode>::ValueT;
    for (const signal_id_t sig : signals)
    {
        ret_vector.push_back(ValueT(m_trace.back().at(sig)));
    }
    return ret_vector;
}

template<verif_mode_t mode>
ValueVector<mode> Simulator<mode>::ith_share(const Range secret_range, const size_t which_share)
{
    const size_t front = secret_range.second;
    const size_t back = secret_range.first;
    const size_t direction = (front < back) ? 1 : -1;

    ValueVector<mode> result;
    result.reserve(std::abs((int64_t)front - (int64_t)back) + 1);
    for (size_t i = front; ; i += direction)
    {
        result.push_back(m_secrets.at(i).at(which_share));
        if (i == back) break;
    }

    return result;
}

template<verif_mode_t mode>
ValueVector<mode> Simulator<mode>::ith_secret(const Range share_range, const size_t which_secret)
{
    const size_t front = share_range.second;
    const size_t back = share_range.first;
    const size_t direction = (front < back) ? 1 : -1;

    ValueVector<mode> result;
    result.reserve(std::abs((int64_t)front - (int64_t)back) + 1);
    for (size_t i = front; ; i += direction)
    {
        result.push_back(m_secrets.at(which_secret).at(i));
        if (i == back) break;
    }

    return result;
}

template<verif_mode_t mode>
ValueVector<mode> Simulator<mode>::masks(const Range range)
{
    const size_t front = range.second;
    const size_t back = range.first;
    const size_t direction = (front < back) ? 1 : -1;

    ValueVector<mode> result;
    result.reserve(std::abs((int64_t)front - (int64_t)back) + 1);
    for (size_t i = front; ; i += direction)
    {
        result.push_back(m_masks.at(i));
        if (i == back) break;
    }

    return result;
}

template<verif_mode_t mode>
void Simulator<mode>::dump_vcd(const std::string& file_name)
{
    std::ofstream out(file_name);

    // Print the current time
    {
        auto now_timepoint = std::chrono::system_clock::now();
        std::time_t curr_time = std::chrono::system_clock::to_time_t(now_timepoint);
        out << "$date" << std::endl;
        out << "\t" << std::ctime(&curr_time) << std::endl;
        out << "$end" << std::endl;
    }

    // Print the generator version
    {
        out << "$version" << std::endl;
        out << "\t" << "Coco++: Execution-aware Masking Verifier v1.0" << std::endl;
        out << "$end" << std::endl;
    }

    // Print the timescale
    {
        out << "$timescale" << std::endl;
        out << "\t" << "1ps" << std::endl;
        out << "$end" << std::endl;
    }

    // Get all named signals
    std::unordered_map<signal_id_t, const std::string> signals_in_vcd;
    using scope_entry_t = std::tuple<const char*, std::string, std::string, uint32_t>;
    std::vector<scope_entry_t> scope_data;
    for (auto& it : m_name_bits)
    {
        const std::string name = it.first;
        const std::vector<signal_id_t> bits = it.second;
        for (uint32_t pos = 0; pos < bits.size(); pos++)
        {
            const signal_id_t sig_id = bits[pos];
            const std::string sig_str = vcd_identifier(sig_id);
            signals_in_vcd.emplace(sig_id, sig_str);
            const char* type = (m_sig_clock != signal_id_t::S_0 && sig_id == m_sig_clock) ? "wire 1" : "string 0";
            scope_data.emplace_back(type, sig_str, name, pos);

        }
    }

    if (m_sig_clock != signal_id_t::S_0)
        signals_in_vcd.erase(m_sig_clock);

    const char* scope_types[2] = {"stable", "glitchy"};
    const value_display_t display_types[2] = {value_display_t::DISPLAY_STABLE, value_display_t::DISPLAY_GLITCH};

    for (auto & scope_type : scope_types) {
        out << "$scope module " << m_module_name << ":" << scope_type << " $end" << std::endl;
        for (const auto& entry : scope_data)
        {
            out << "\t$var " << std::get<0>(entry) << " " << scope_type[0] << std::get<1>(entry);
            out << " " << std::get<2>(entry) << "[" << std::get<3>(entry) << "]" << " [0] $end" << std::endl;
        }
        out << "$upscope $end" << std::endl;
    }
    out << "$enddefinitions $end" << std::endl;

    if (m_trace.empty())
    {
        out.close();
        return;
    }

    // Dump the first cycle
    auto prev_ptr = m_trace.cbegin();
    auto curr_ptr = m_trace.cbegin();
    uint32_t curr_tick = 0;

    while(curr_ptr != m_trace.end())
    {
        out << "#" << curr_tick << std::endl;
        if (curr_tick == 0) out << "$dumpvars" << std::endl;

        const SimulatorValueMap& curr_map = *curr_ptr;
        const SimulatorValueMap& prev_map = *prev_ptr;

        if (m_sig_clock != signal_id_t::S_0)
        {
            out << "b1 " << scope_types[0][0] << vcd_identifier(m_sig_clock) << std::endl;
            out << "b1 " << scope_types[1][0] << vcd_identifier(m_sig_clock) << std::endl;
        }

        for (int scope_id = 0; scope_id < 1 + has_glitches(mode); scope_id++)
        {
            const char* scope_type = scope_types[scope_id];
            Value<mode>::s_display = display_types[scope_id];
            for (auto it: signals_in_vcd)
            {
                auto curr_find_it = curr_map.find(it.first);
                auto prev_find_it = prev_map.find(it.first);
                const std::string vcd_id = scope_type[0] + it.second;
                if (curr_tick == 0)
                {
                    if (curr_find_it != curr_map.end())
                    { out << "s" << curr_find_it->second << " " << vcd_id << std::endl; }
                    else
                    { out << "bx " << vcd_id << std::endl; }

                }
                else if (curr_find_it != curr_map.end())
                {
                    if (prev_find_it == prev_map.end() || curr_find_it->second != prev_find_it->second)
                    { out << "s" << curr_find_it->second << " " << vcd_id << std::endl; }
                }
            }
        }

        if (curr_tick == 0) out << "$end" << std::endl;

        if (m_sig_clock != signal_id_t::S_0)
        {
            out << "#" << curr_tick + 500 << std::endl;
            out << "b0 " << scope_types[0][0] << vcd_identifier(m_sig_clock) << std::endl;
            out << "b0 " << scope_types[1][0] << vcd_identifier(m_sig_clock) << std::endl;
        }

        curr_tick += 1000;
        prev_ptr = curr_ptr;
        curr_ptr++;
    }

    out << "#" << curr_tick << std::endl;
}

#endif // SIMULATOR_H