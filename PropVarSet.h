#ifndef PROPVARSET_H
#define PROPVARSET_H

#include <unordered_map>
#include <memory>
#include "common.h"

#define OPT_FRESH_BIASED
#define OPT_NONLINEAR_COLLAPSE

class PropVarSet;
using PropVarSetPtr = std::shared_ptr<PropVarSet>;
class SatSolver;

// Declaration for purposes of passing default arguments
PropVarSetPtr bias(const PropVarSetPtr& p_pvs_a, var_t* p_var = nullptr);

class PropVarSet {
private:
    /// Map from label index to solver variable.
    /// Invariant: ZERO is never stored in this map.
    std::unordered_map<lidx_t, var_t> m_vars;
    #ifdef OPT_FRESH_BIASED
    /// Whether the PVS is assumed biased in computations
    bool m_biased = false;
    #endif
    /// Points to the solver managing the solver variables
    SatSolver* m_solver;

    /// Allow creating a shared pointer with private copy constructor
    inline static PropVarSetPtr make_pvs(const PropVarSet& pvs);
    /// Private copy constructor
    PropVarSet(const PropVarSet&) = default;
public:
    /// Default constructor is disabled
    PropVarSet() = delete;
    /// Constructs PVS with one single active label
    PropVarSet(SatSolver* sol, lidx_t id);

    /// Get the solver variable at a given label index
    var_t operator[](const lidx_t& pos) const;
    /// Get number of solver variables hosted
    uint64_t size() const { return m_vars.size(); }

    /// Creates a biased copy of set \a pvs
    friend PropVarSetPtr bias(const PropVarSetPtr& p_pvs_a, var_t* p_var);
    /// Creates an XOR of two sets \a pvs_a and \a pvs_b
    friend PropVarSetPtr operator^(const PropVarSetPtr& p_pvs_a, const PropVarSetPtr& p_pvs_b);
    /// Either return \a p_pvs if already biased or a biased copy
    friend PropVarSetPtr operator+(const PropVarSetPtr& p_pvs);
    /// Creates an AND/OR of two sets \a pvs_a and \a pvs_b
    friend PropVarSetPtr operator&(const PropVarSetPtr& p_pvs_a, const PropVarSetPtr& p_pvs_b);
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

#endif // PROPVARSET_H