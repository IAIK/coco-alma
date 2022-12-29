#include <popcntintrin.h>
#include <iostream>
#include "SatSolver.h"

SatSolver::SatSolver() :
        m_state(STATE_INPUT), m_num_clauses(0), m_solver(ipasir_init())
{ }

SatSolver::~SatSolver()
{
    DEBUG(0) << "Solver xor cache has " << m_xor_cache.size() << " entries." << std::endl;
    DEBUG(0) << "Solver and cache has " << m_and_cache.size() << " entries." << std::endl;

    ipasir_release(m_solver);
}

var_t SatSolver::make_xor(var_t a, var_t b)
{
    Assert(is_legal(a), ILLEGAL_LITERAL);
    Assert(is_legal(b), ILLEGAL_LITERAL);
    Assert(is_known(a), UNKNOWN_LITERAL);
    Assert(is_known(b), UNKNOWN_LITERAL);

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

    // See if we already have a variable for this
    const gate_key_t key = make_key(a < b ? a : b, a < b ? b : a);
    auto res = m_xor_cache.find(key);
    if (res != m_xor_cache.end())
        return negate ? -(res->second) : res->second;

    var_t c = new_var();
    add_clause(-a, -b, -c);
    add_clause(+a, +b, -c);
    add_clause(-a, +b, +c);
    add_clause(+a, -b, +c);
    m_state = STATE_INPUT;

    // Register variable in the cache
    m_xor_cache.emplace(key, c);
    // Also register implied relations
    m_xor_cache.emplace(make_key(a, c), b);
    m_xor_cache.emplace(make_key(b, c), a);

    return negate ? -c : c;
}

var_t SatSolver::make_and(const var_t a, const var_t b)
{
    var_t c = simplify_and(a, b);
    if (c != 0) return c;

    // Add the clauses for constraining the variables
    c = new_var();
    add_clause(+a, -c);
    add_clause(+b, -c);
    add_clause(-a, -b, +c);
    m_state = STATE_INPUT;

    const gate_key_t key = make_key(a < b ? a : b, a < b ? b : a);
    register_and(key, c);
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