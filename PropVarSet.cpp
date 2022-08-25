#include <memory>
#include <cassert>
#include <iostream>

#include "SatSolver.h"
#include "PropVarSet.h"

PropVarSet::PropVarSet(SatSolver* sol, lidx_t id) :
        m_solver(sol), m_biased(false)
{
    // Just set that one position to 1
    m_vars[id] = ONE;
    DEBUG << "PropVarSet::PropVarSet id=" << id << std::endl;
}

var_t PropVarSet::operator[](const lidx_t& pos) const
{
    // Return content or an implied 0
    auto var_iter = m_vars.find(pos);
    return (var_iter == m_vars.end()) ? ZERO : var_iter->second;
}

PropVarSetPtr operator^(const PropVarSetPtr& p_pvs_a, const PropVarSetPtr& p_pvs_b)
{
    assert(p_pvs_a.get() != nullptr);
    assert(p_pvs_b.get() != nullptr);
    assert(p_pvs_a->m_solver == p_pvs_b->m_solver);

    // Separate into larger and smaller for insertion later
    const PropVarSetPtr& p_larger = p_pvs_a->size() > p_pvs_b->size() ? p_pvs_a : p_pvs_b;
    const PropVarSetPtr& p_smaller = p_pvs_a->size() > p_pvs_b->size() ? p_pvs_b : p_pvs_a;

    // Start as a copy of the larger one
    PropVarSetPtr p_res = PropVarSet::make_pvs(*p_larger);

    #ifdef OPT_FRESH_BIASED
    // Inherit and steal biasing from parents
    p_res->m_biased &= p_smaller->m_biased;
    if (p_res->m_biased)
    {
        p_larger->m_biased = false;
        p_smaller->m_biased = false;
    }
    #endif

    // Go through all elements of smaller set and insert them after XORing
    for (auto& var_it : p_smaller->m_vars)
    {
        const lidx_t label = var_it.first;
        const var_t s1 = (*p_larger)[label];
        const var_t s2 = var_it.second;
        const var_t r = p_res->m_solver->make_xor(s1, s2);
        DEBUG << s1 << " " << s2 << " " << r << std::endl;
        // Keep invariant correct, never add 0 into the set
        if (r == ZERO) continue;
        p_res->m_vars[label] = r;
    }
    DEBUG << "CALC " << *p_pvs_a << " ^ " << *p_pvs_b << " = " << *p_res << std::endl;
    return p_res;
}

PropVarSetPtr operator+(const PropVarSetPtr& p_pvs)
{
    #ifdef OPT_FRESH_BIASED
    // In case it is empty or already fresh, return it
    if (p_pvs->m_biased || p_pvs->size() == 0)
        return p_pvs;
    #endif
    return p_pvs->bias();
}

PropVarSetPtr PropVarSet::bias(var_t* p_var) const
{
    // Create a clone of the current set
    PropVarSetPtr p_res = make_pvs(*this);

    #ifdef OPT_FRESH_BIASED
    // Mark as freshly biased
    p_res->m_biased = true;
    #endif
    if (p_res->size() == 0) return p_res;

    // Create biasing variable that is also returned through \a p_var
    const var_t var = p_res->m_solver->new_var();

    // Bias all variables (making them maybe 0, depending on new variable)
    for (auto& var_it : p_res->m_vars) {
        var_t& solver_var = var_it.second;
        assert (solver_var != ZERO);
        solver_var = p_res->m_solver->make_and(var, solver_var);
    }

    // Return new variable if desired
    if (p_var != nullptr) *p_var = var;
    DEBUG << "CALC +" << this << " = " << *p_res << std::endl;
    return p_res;
}

std::ostream& operator<<(std::ostream &stream, const PropVarSet& pvs) {
    stream << "{";
    bool first = true;
    for (const auto& var_it : pvs.m_vars)
    {
        const lidx_t label = var_it.first;
        const var_t solver_var = var_it.second;
        if (!first) stream << ", ";
        stream << label << ": ";
        if (solver_var == ONE) {
            stream << "T";
        } else {
            stream << solver_var;
        }
        first = false;
    }
    stream << "}";
    return stream;
}