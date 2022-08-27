#include <popcntintrin.h>
#include <iostream>
#include "SatSolver.h"

SatSolver::SatSolver() :
        m_num_vars(0), m_solver(ipasir_init()), m_state(STATE_INPUT)
{ }

SatSolver::~SatSolver()
{
    ipasir_release(m_solver);
}

int SatSolver::make_xor(var_t a, var_t b)
{
    // Standard rules for xor with constant
    if (a == ZERO) return b;
    if (b == ZERO) return a;

    if (a == ONE && b == ONE) return ZERO; // necessary
    if (a == ONE) return -b;
    if (b == ONE) return -a;

    // Simplification rules for xor
    if (a == b) return ZERO;
    if (a == -b) return ONE;

    // Create canonical form
    bool negate = (a < 0) ^ (b < 0);
    a = std::abs(a);
    b = std::abs(b);

    #ifdef OPT_EXPR_CACHING
    // See if we already have a variable for this
    const gate_key_t key = make_key(a < b ? a : b, a < b ? b : a);
    auto res = m_xor_cache.find(key);
    if (res != m_xor_cache.end())
        return negate ? -(res->second) : res->second;
    #endif

    var_t c = new_var();
    add_clause(-a, -b, -c);
    add_clause(+a, +b, -c);
    add_clause(-a, +b, +c);
    add_clause(+a, -b, +c);
    m_state = STATE_INPUT;

    #ifdef OPT_EXPR_CACHING
    // Register variable in the cache
    m_xor_cache.emplace(key, c);
    // Also register implied relations
    m_xor_cache.emplace(make_key(a, c), b);
    m_xor_cache.emplace(make_key(b, c), a);
    #endif

    return negate ? -c : c;
}

int SatSolver::make_and(const var_t a, const var_t b)
{
    // Standard rules for and with constant
    if (a == ZERO || b == ZERO) return ZERO;
    if (a == ONE) return b;
    if (b == ONE) return a;
    // Simplification rules for and
    if (a == b) return a;
    if (a == -b) return ZERO;

    #ifdef OPT_EXPR_CACHING
    // See if we already have a variable for this
    const gate_key_t key = make_key(a < b ? a : b, a < b ? b : a);
    auto res = m_and_cache.find(key);
    if (res != m_and_cache.end()) return res->second;
    #endif

    // Add the clauses for constraining the variables
    const var_t c = new_var();
    add_clause(+a, -c);
    add_clause(+b, -c);
    add_clause(-a, -b, +c);
    m_state = STATE_INPUT;

    #ifdef OPT_EXPR_CACHING
    // Register variable in the cache
    m_and_cache.emplace(key, c);
    #endif
    return c;
}

SatSolver::state_t SatSolver::check() noexcept
{
    m_state = static_cast<state_t>(ipasir_solve(m_solver));
    return m_state;
}

bool SatSolver::value(var_t a)
{
    Assert(m_state == STATE_SAT, REQUIRE_SAT);
    return ipasir_val(m_solver, a) > 0;
}