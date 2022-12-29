#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <list>
#include <utility>
#include "Circuit.h"
#include "Value.h"
#include "SatSolver.h"
#include "SymbolManager.h"

#include <map>
#include <fstream>
#include <chrono>

template<verif_mode_t mode>
class Simulator : protected Circuit
{
public:
    SatSolver m_solver; // TODO: make this private
    SymbolManager m_symbols;
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

#include "Simulator.hpp"

#endif // SIMULATOR_H
