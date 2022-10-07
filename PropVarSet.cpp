#include <memory>
#include <cassert>
#include <iostream>
#include <unordered_set>

#include "SatSolver.h"
#include "PropVarSet.h"

PropVarSet::PropVarSet(SatSolver* sol, lidx_t id) :
        m_solver(sol), m_biased(false)
{
    // Just set that one position to 1
    m_vars[id] = ONE;
    DEBUG(1) << "CREATE " << *this << std::endl;
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
    p_larger->m_biased = false;
    p_smaller->m_biased = false;
    #endif

    // Go through all elements of smaller set and insert them after XORing
    for (auto& var_it : p_smaller->m_vars)
    {
        const lidx_t label = var_it.first;
        const var_t s1 = (*p_larger)[label];
        const var_t s2 = var_it.second;
        const var_t r = p_res->m_solver->make_xor(s1, s2);
        // Keep invariant correct, never add 0 into the set
        if (r == ZERO)
        {
            p_res->m_vars.erase(label);
            continue;
        }
        p_res->m_vars[label] = r;
    }
    DEBUG(1) << "CALC " << *p_pvs_a << " ^ " << *p_pvs_b << " = " << *p_res << std::endl;
    return p_res;
}

PropVarSetPtr operator+(const PropVarSetPtr& p_pvs)
{
    PropVarSetPtr p_res = bias(p_pvs);
    DEBUG(1) << "CALC +" << *p_pvs << " = " << *p_res << std::endl;
    return p_res;
}

PropVarSetPtr bias(const PropVarSetPtr& p_pvs, var_t* p_var)
{
    #ifdef OPT_FRESH_BIASED
    // In case it is empty or already fresh, return it
    if (p_pvs->m_biased || p_pvs->size() == 0)
    {
        if (p_var != nullptr) *p_var = ZERO;
        return p_pvs;
    }
    #endif

    // Create a clone of the current set
    PropVarSetPtr p_res = PropVarSet::make_pvs(*p_pvs);

    #ifdef OPT_FRESH_BIASED
    // Mark as freshly biased
    p_res->m_biased = true;
    #endif

    // Create biasing variable that is also returned through \a p_var
    const var_t var = p_res->m_solver->new_var();
    // Return new variable if desired
    if (p_var != nullptr) *p_var = var;

    // Bias all variables (making them maybe 0, depending on new variable)
    for (auto& var_it : p_res->m_vars)
    {
        var_t& solver_var = var_it.second;
        assert (solver_var != ZERO);
        solver_var = p_res->m_solver->make_and(var, solver_var);
    }
    return p_res;
}

PropVarSetPtr operator&(const PropVarSetPtr& p_pvs_a, const PropVarSetPtr& p_pvs_b)
{
    assert(p_pvs_a.get() != nullptr);
    assert(p_pvs_b.get() != nullptr);
    assert(p_pvs_a->m_solver == p_pvs_b->m_solver);

    // In case the sets are trivially the same, return biased copy
    if (p_pvs_a.get() == p_pvs_b.get() || *p_pvs_a == *p_pvs_b)
    {
        PropVarSetPtr p_res = +p_pvs_a;
        DEBUG(0) << "CALC " << *p_pvs_a << " & " << *p_pvs_b << " = " << *p_res << std::endl;
        return p_res;
    }

    #ifdef OPT_NONLINEAR_COLLAPSE
    SatSolver* solver = p_pvs_a->m_solver;

    // Separate into larger and smaller for insertion later
    const PropVarSetPtr& p_large = p_pvs_a->size() > p_pvs_b->size() ? p_pvs_a : p_pvs_b;
    const PropVarSetPtr& p_small = p_pvs_a->size() > p_pvs_b->size() ? p_pvs_b : p_pvs_a;

    // Start as a copy of the larger one
    PropVarSetPtr p_res = PropVarSet::make_pvs(*p_large);
    DEBUG(1) << "Biasing of " << p_pvs_a << " is " << p_pvs_a->m_biased << std::endl;
    DEBUG(1) << "Biasing of " << p_pvs_b << " is " << p_pvs_b->m_biased << std::endl;

    p_res->m_biased = true;
    const var_t p = p_large->m_biased ? ONE : solver->new_var();
    const var_t q = p_small->m_biased ? ONE : solver->new_var();
    p_large->m_biased = false;
    p_small->m_biased = false;

    // Go through all elements of the smaller set
    for (const auto& var_it_small : p_small->m_vars)
    {
        const lidx_t label = var_it_small.first;
        const var_t b = var_it_small.second;

        const auto& p_var_it_res = p_res->m_vars.find(label);
        if (p_var_it_res != p_res->m_vars.end())
        {
            // Found same index in result and small set, therefore constrain the variables
            var_t &a = p_var_it_res->second;

            const var_t v = solver->new_var();
            DEBUG(2) << "Entered adding part with " << p << " " << a << " " << q << " " << b << " " << v << std::endl;

            // Encodes that v = (a & p) ^ (b & q)
            solver->add_clause(p, q, -v);
            solver->add_clause(p, b, -v);
            solver->add_clause(a, q, -v);
            solver->add_clause(a, b, -v);
            solver->add_clause(-p, -a, -q, -b, -v);

            solver->add_clause(-p, -a, q, v);
            solver->add_clause(-p, -a, b, v);
            solver->add_clause(-q, -b, p, v);
            solver->add_clause(-q, -b, a, v);
            // TODO: Check if this is arc complete
            // Update the variable in the result
            a = v;
        }
        else
        {
            // Insert new variable that can be false at this label
            p_res->m_vars[label] = solver->make_and(b, q);
        }
    }

    for (auto& var_it_large : p_res->m_vars)
    {
        const lidx_t label = var_it_large.first;
        if (p_small->m_vars.find(label) != p_small->m_vars.end()) continue;
        const var_t a = var_it_large.second;
        var_it_large.second = solver->make_and(a, p);
    }
    #else
    // Use encoding through XOR of biased inputs
    PropVarSetPtr p_res = +p_pvs_a ^ +p_pvs_b;
    #endif
    DEBUG(1) << "CALC " << *p_pvs_a << " & " << *p_pvs_b << " = " << *p_res << std::endl;
    return p_res;
}

PropVarSetPtr operator|(const PropVarSetPtr& p_pvs_a, const PropVarSetPtr& p_pvs_b)
{
    assert(p_pvs_a.get() != nullptr);
    assert(p_pvs_b.get() != nullptr);
    assert(p_pvs_a->m_solver == p_pvs_b->m_solver);

    // In case the sets are trivially the same, return biased copy
    if (p_pvs_a.get() == p_pvs_b.get() || *p_pvs_a == *p_pvs_b)
    {
        DEBUG(1) << "CALC " << *p_pvs_a << " | " << *p_pvs_b << " = " << *p_pvs_a << std::endl;
        return p_pvs_a;
    }

    SatSolver* solver = p_pvs_a->m_solver;

    // Separate into larger and smaller for insertion later
    const PropVarSetPtr& p_large = p_pvs_a->size() > p_pvs_b->size() ? p_pvs_a : p_pvs_b;
    const PropVarSetPtr& p_small = p_pvs_a->size() > p_pvs_b->size() ? p_pvs_b : p_pvs_a;

    // Start as a copy of the larger one
    PropVarSetPtr p_res = PropVarSet::make_pvs(*p_large);
    DEBUG(1) << "Biasing of " << p_pvs_a << " is " << p_pvs_a->m_biased << std::endl;
    DEBUG(1) << "Biasing of " << p_pvs_b << " is " << p_pvs_b->m_biased << std::endl;

    const var_t p = solver->new_var();
    p_large->m_biased = false;
    p_small->m_biased = false;

    // Go through all elements of the smaller set
    for (const auto& var_it_small : p_small->m_vars)
    {
        const lidx_t label = var_it_small.first;
        const var_t b = var_it_small.second;

        const auto& p_var_it_res = p_res->m_vars.find(label);
        if (p_var_it_res != p_res->m_vars.end())
        {
            // Found same index in result and small set, therefore constrain the variables
            var_t &a = p_var_it_res->second;

            // In this case, we actually have no choice and the result is already set to a
            if (a == b) continue;

            const var_t v = solver->new_var();
            DEBUG(2) << "Entered adding part with " << p << " " << a << " " << b << " " << v << std::endl;

            // Encodes that v = p ? a : b
            solver->add_clause(-p,  a, -v);
            solver->add_clause(-p, -a,  v);
            solver->add_clause( p,  b, -v);
            solver->add_clause( p, -b,  v);
            solver->add_clause( a,  b, -v);
            solver->add_clause(-a, -b,  v);

            // Update the variable in the result
            a = v;
        }
        else
        {
            // Insert new variable that can be false at this label
            p_res->m_vars[label] = solver->make_and(b, -p);
        }
    }

    for (auto& var_it_large : p_res->m_vars)
    {
        const lidx_t label = var_it_large.first;
        if (p_small->m_vars.find(label) != p_small->m_vars.end()) continue;
        const var_t a = var_it_large.second;
        var_it_large.second = solver->make_and(a, p);
    }
    DEBUG(1) << "CALC " << *p_pvs_a << " | " << *p_pvs_b << " = " << *p_res << std::endl;
    return p_res;
}

std::ostream& operator<<(std::ostream &stream, const PropVarSet& pvs)
{
    stream << "{";
    bool first = true;
    for (const auto& var_it : pvs.m_vars)
    {
        const lidx_t label = var_it.first;
        const var_t solver_var = var_it.second;

        if (pvs.m_solver != nullptr && pvs.m_solver->state() == SatSolver::state_t::STATE_SAT)
        {
            if (solver_var == ONE || pvs.m_solver->value(solver_var))
            {
                if (!first) stream << ",";
                stream << label;
                first = false;
            }
        }
        else
        {
            if (!first) stream << ",";
            stream << label << ":";
            if (solver_var == ONE)
                { stream << "T"; }
            else
                { stream << solver_var; }
            first = false;
        }
    }
    stream << "}";
    return stream;
}

bool operator==(const PropVarSet& pvs_a, const PropVarSet& pvs_b)
{
    #ifdef OPT_FRESH_BIASED
    if (pvs_a.m_biased != pvs_b.m_biased) return false;
    #endif
    return pvs_a.m_solver == pvs_b.m_solver && pvs_a.size() == pvs_b.size() && pvs_a.m_vars == pvs_b.m_vars;
}
