#pragma once

#include <unordered_map>
#include <memory>
#include "common.h"

#define OPT_FRESH_BIASED

class PropVarSet;
using PropVarSetPtr = std::shared_ptr<PropVarSet>;
class SatSolver;

class PropVarSet {
private:
    /// Map from label index to solver variable.
    /// Invariant: ZERO is never stored in this map.
    std::unordered_map<lidx_t, var_t> m_vars;
    /// Whether the PVS is assumed biased in computations
    bool m_biased = false;
    /// Points to the solver managing the solver variables
    SatSolver* m_solver;
    /// Default constructor is disabled
    PropVarSet() = delete;

    // PropVarSet(SatSolver* sol, bool _fresh_biased);

    // TODO check if needed
    void set_svar(const lidx_t& pvar, const var_t& svar) {
        if (svar == ZERO) {
            m_vars.erase(pvar);
        } else {
            m_vars[pvar] = svar;
        }
    }

    /// Allow creating a shared pointer with private copy constructor
    inline static PropVarSetPtr make_pvs(const PropVarSet& pvs);
private:
    /// Private copy constructor
    PropVarSet(const PropVarSet&) = default;
public:
    /// Constructs PVS with one single active label
    PropVarSet(SatSolver* sol, lidx_t id);

    /// Get the solver variable at a given label index
    var_t operator[](const lidx_t& pos) const;
    /// Get number of solver variables hosted
    uint64_t size() const { return m_vars.size(); }

    /// Creates a biased copy of set \a pvs
    PropVarSetPtr bias(var_t* p_var = nullptr) const;

    /// Creates an XOR of two sets \a pvs_a and \a pvs_b
    friend PropVarSetPtr operator^(const PropVarSetPtr& p_pvs_a, const PropVarSetPtr& p_pvs_b);
    /// Either return \a p_pvs if already biased or a biased copy
    friend PropVarSetPtr operator+(const PropVarSetPtr& p_pvs);
    /// Prints a set \a pvs to output stream \a stream
    friend std::ostream& operator<<(std::ostream &stream, const PropVarSet& pvs);

};

inline PropVarSetPtr PropVarSet::make_pvs(const PropVarSet& pvs)
{
    struct enabler : public PropVarSet
    {
        explicit enabler(const PropVarSet& pvs) : PropVarSet(pvs) {};
    };
    return std::make_shared<enabler>(pvs);
}


