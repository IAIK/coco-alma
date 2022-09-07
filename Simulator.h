#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <list>
#include <utility>
#include "Circuit.h"
#include "Value.h"
#include "SatSolver.h"

template<verif_mode_t mode>
class Simulator : protected Circuit
{
    SatSolver m_solver;
    bool m_prepared;
    using SimulatorValueMap = std::unordered_map<signal_id_t, Value<mode>>;
    std::list<SimulatorValueMap> m_trace;
    static inline void insert_consts(SimulatorValueMap& vals);
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

    ValueViewVector<mode> operator[](const std::string& name);
};

template <verif_mode_t mode>
inline void Simulator<mode>::insert_consts(SimulatorValueMap& vals)
{
    vals.emplace(signal_id_t::S_0, false);
    vals.emplace(signal_id_t::S_1, true);
    vals.emplace(signal_id_t::S_X, false);
    vals.emplace(signal_id_t::S_Z, false);
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
        Circuit(circ), m_solver(), m_prepared(false)
{
    // Create a dummy pre-cycle that is computed with all zero inputs and registers
    m_trace.emplace_back();
    SimulatorValueMap& first_cycle = m_trace.back();
    for (const signal_id_t sig : m_in_ports)
        { first_cycle.emplace(sig, false); }
    for (const signal_id_t sig : m_reg_outs)
        { first_cycle.emplace(sig, false); }

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
    SimulatorValueMap& curr_cycle = m_trace.back();
    const SimulatorValueMap& prev_cycle = *(++m_trace.crbegin());

    insert_consts(curr_cycle);
    for (const Cell* cell : m_cells)
        { cell->eval<Value<mode>, ValueView<mode>>(prev_cycle, curr_cycle); }
    m_prepared = false;
}

template <verif_mode_t mode>
Simulator<mode>::~Simulator()
{
    m_cells.clear();
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

#endif // SIMULATOR_H
