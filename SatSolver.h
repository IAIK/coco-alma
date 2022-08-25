#pragma once
#include "common.h"

extern "C" {
#include "ipasir.h"
}

#define EXPR_CACHING

#ifdef EXPR_CACHING
#include <unordered_map>
using gate_key_t = uint64_t;
constexpr gate_key_t make_key(var_t a, var_t b) { return ((gate_key_t)(a) << 32) | b; }
#endif

class SatSolver {
public:
    enum state_t {STATE_SAT = 10, STATE_UNSAT = 20, STATE_INPUT = 30};
private:
    /// Current state of the solver
    state_t m_state;
    /// The number of currently allocated solver variables
    int m_num_vars;
    /// IPASIR solver object with generic (void*) type
    void* m_solver;

    /// Internal ipasir_add forwarding
    inline void add(var_t x) { ipasir_add(m_solver, x); }

    #ifdef EXPR_CACHING
    /// Cache for AND gates
    std::unordered_map<gate_key_t, var_t> m_and_cache;
    /// Cache for XOR gates
    std::unordered_map<gate_key_t, var_t> m_xor_cache;
    #endif

public:
    /// Allocates and returns a new solver variable
    inline var_t new_var();
    /// Allocates \a number many solver variables and returns the first one
    inline var_t new_vars(int number);
    /// Creates a new variable representing the xor of \a a and \a b
    var_t make_xor(var_t a, var_t b);
    /// Creates a new variable representing the and of \a a and \a b
    var_t make_and(var_t a, var_t b);

    /// Main satisfiability checking routine
    state_t check();
    /// Return the value assigned to variable \a a
    bool value(var_t a);

    /// Only default constructor is implemented
    SatSolver();
    /// Destructor destroying the internal IPASIR object
    ~SatSolver();
};

inline var_t SatSolver::new_vars(const int number)
{
    const var_t var = m_num_vars;
    m_num_vars += number;
    return var + 1;
}

inline var_t SatSolver::new_var()
{
    return new_vars(1);
}