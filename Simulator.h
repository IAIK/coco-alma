#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <list>
#include <utility>
#include "Circuit.h"
#include "Value.h"
#include "SatSolver.h"
#include <map>
#include <fstream>
#include <chrono>

template<verif_mode_t mode>
class Simulator : protected Circuit
{
public:
    SatSolver m_solver;
private:
    bool m_prepared;
    using SimulatorValueMap = std::unordered_map<signal_id_t, Value<mode>>;
    std::list<SimulatorValueMap> m_trace;

    std::vector<lidx_t> m_secret_indexes;
    std::vector<lidx_t> m_mask_indexes;

    uint32_t m_num_lidx;
    inline lidx_t new_lidx() { lidx_t res = m_num_lidx; m_num_lidx += 1; return res; }
    inline lidx_t new_secret_lidx() { lidx_t res = new_lidx(); m_secret_indexes.push_back(res); return res; }
    inline lidx_t new_mask_lidx() { lidx_t res = new_lidx(); m_mask_indexes.push_back(res); return res; }

    std::unordered_map<size_t, ValueVector<mode>> m_secrets;
    std::unordered_map<size_t, lidx_t> m_check_lidx;
    std::unordered_map<size_t, Value<mode>> m_masks;

    using ValueLookupKey = std::pair<Value<mode>, Value<mode>>;
    std::map<ValueLookupKey, Value<mode>> m_and_map;
    std::map<ValueLookupKey, Value<mode>> m_xor_map;

    inline void insert_consts(SimulatorValueMap& vals);
public:
    /// Shallow copy constructor from a Circuit class, without pointer ownership transfer
    explicit Simulator(const Circuit& circ);
    /// Custom destructor that obeys the pointer ownership, calling ~Circuit() implicitly
    ~Simulator();
    /// Prepares the execution of a cycle, defining inputs like in previous cycle
    void prepare_cycle();
    /// Executes one clock cycle and stores the results in \m m_trace
    void step_cycle();
    /// Performs both cycle preparation and stepping
    inline void step() { prepare_cycle(); step_cycle(); }
    /// Allocates secret identifiers in range \a range with \a num_shares many shares
    void allocate_secrets(Range range, size_t num_shares);
    /// Allocates mask identifiers in range \a range
    void allocate_masks(Range range);
    /// Get the ith share of secrets in \a secret_range
    ValueVector<mode> ith_share(Range secret_range, size_t which_share);
    /// Get shares in \a share_range of the ith secret
    ValueVector<mode> ith_secret(Range share_range, size_t which_secret);
    /// Get the ith share of all secrets in \a range
    ValueVector<mode> masks(Range range);
    /// Get the values of wires corresponding to \a name
    ValueViewVector<mode> operator[](const std::string& name);

    /// Dump the current trace (either assigned or unassigned) as a VCD file
    void dump_vcd(const std::string& file_name);

    auto and_find(ValueLookupKey p) const { return m_and_map.find(p); }
    auto and_end() const { return m_and_map.end(); }
    void and_insert(ValueLookupKey p, const Value<mode>& res)
    { auto it = m_and_map.insert(std::make_pair(p, res)); assert(it.second); }

    auto xor_find(ValueLookupKey p) const { return m_xor_map.find(p); }
    auto xor_end() const { return m_xor_map.end(); }
    void xor_insert(ValueLookupKey p, const Value<mode>& res)
    { auto it = m_xor_map.insert(std::make_pair(p, res)); assert(it.second); }
};

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
    { second_cycle.emplace(sig, first_cycle.at(sig)); }
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
            value_vector.emplace_back(p_pvs);
        }
        for (size_t sh = 1; sh < num_shares; sh++)
        {
            lidx_t mask_idx = new_mask_lidx();
            PropVarSetPtr p_pvs = std::make_shared<PropVarSet>(&m_solver, mask_idx);
            std::cout << *p_pvs << std::endl;
            Value<mode> val(p_pvs);
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
        m_masks.emplace(std::piecewise_construct, std::forward_as_tuple(i), std::forward_as_tuple(p_pvs));
    }
}

template <verif_mode_t mode>
ValueViewVector<mode> Simulator<mode>::operator[](const std::string& name)
{
    const std::vector<signal_id_t>& signals = m_name_bits.at(name);
    ValueViewVector<mode> ret_vector;
    for (const signal_id_t sig : signals)
        ret_vector.push_back(ValueView<mode>(m_trace.back().at(sig)));
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

    out << "$scope module " << m_module_name << " $end" << std::endl;

    // Get all named signals
    std::unordered_map<signal_id_t, const std::string> signals_in_vcd;
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
            out << "\t$var " << type << " " << sig_str << " " << name;
            out << "[" << pos << "]" << " [0] $end" << std::endl;
        }
    }

    if (m_sig_clock != signal_id_t::S_0)
        signals_in_vcd.erase(m_sig_clock);

    out << "$upscope $end" << std::endl;
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
            { out << "b1 " << vcd_identifier(m_sig_clock) << std::endl; }

        for (auto it: signals_in_vcd)
        {
            auto curr_find_it = curr_map.find(it.first);
            auto prev_find_it = prev_map.find(it.first);

            if (curr_tick == 0)
            {
                if (curr_find_it != curr_map.end())
                    { out << "s" << curr_find_it->second << " " << it.second << std::endl; }
                else
                    { out << "bx " << it.second << std::endl; }

            }
            else if (curr_find_it != curr_map.end())
            {
                if (prev_find_it == prev_map.end() || curr_find_it->second != prev_find_it->second)
                    { out << "s" << curr_find_it->second << " " << it.second << std::endl; }
            }
        }

        if (curr_tick == 0) out << "$end" << std::endl;

        if (m_sig_clock != signal_id_t::S_0)
            { out << "#" << curr_tick + 500 << std::endl << "b0 " << vcd_identifier(m_sig_clock) << std::endl; }

        curr_tick += 1000;
        prev_ptr = curr_ptr;
        curr_ptr++;
    }

    out << "#" << curr_tick << std::endl;
}


#endif // SIMULATOR_H
