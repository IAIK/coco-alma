#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <list>
#include <utility>
#include "Circuit.h"
#include "Value.h"
#include "SatSolver.h"
#include <map>

template<verif_mode_t mode>
class Simulator : protected Circuit
{
    SatSolver m_solver;
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
    /// Get the ith share of all secrets in \a range
    ValueVector<mode> shares(Range range, size_t which_share);
    /// Get the ith share of all secrets in \a range
    ValueVector<mode> masks(Range range);

    ValueViewVector<mode> operator[](const std::string& name);

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
    vals.emplace(std::piecewise_construct, std::forward_as_tuple(signal_id_t::S_0), std::forward_as_tuple(this, false));
    vals.emplace(std::piecewise_construct, std::forward_as_tuple(signal_id_t::S_1), std::forward_as_tuple(this, true));
    vals.emplace(std::piecewise_construct, std::forward_as_tuple(signal_id_t::S_X), std::forward_as_tuple(this, false));
    vals.emplace(std::piecewise_construct, std::forward_as_tuple(signal_id_t::S_Z), std::forward_as_tuple(this, false));
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
        { first_cycle.emplace(std::piecewise_construct, std::forward_as_tuple(sig), std::forward_as_tuple(this, false)); }
    for (const signal_id_t sig : m_reg_outs)
        { first_cycle.emplace(std::piecewise_construct, std::forward_as_tuple(sig), std::forward_as_tuple(this, false)); }

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
    std::cout << "There are " << m_solver.num_vars() << " variables" << std::endl;
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
        std::cout << secret_idx << std::endl;
        {  // Create a local scope here since the value should be inaccessible later
            PropVarSetPtr p_pvs = std::make_shared<PropVarSet>(&m_solver, secret_idx);
            value_vector.emplace_back(this, p_pvs);
        }
        for (size_t sh = 1; sh < num_shares; sh++)
        {
            lidx_t mask_idx = new_mask_lidx();
            PropVarSetPtr p_pvs = std::make_shared<PropVarSet>(&m_solver, mask_idx);
            std::cout << *p_pvs << std::endl;
            Value<mode> val(this, p_pvs);
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
        m_masks.emplace(std::piecewise_construct, std::forward_as_tuple(i), std::forward_as_tuple(this, p_pvs));
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
ValueVector<mode> Simulator<mode>::shares(const Range range, const size_t which_share)
{
    const size_t front = range.second;
    const size_t back = range.first;
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


#endif // SIMULATOR_H
