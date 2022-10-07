#ifndef SATSOLVER_H
#define SATSOLVER_H
#include "common.h"

extern "C" {
#include "ipasir.h"
}

#define OPT_EXPR_CACHING

#ifdef OPT_EXPR_CACHING
#include <unordered_map>

using gate_key_t = uint64_t;
constexpr gate_key_t make_key(var_t a, var_t b) { return ((gate_key_t)(a) << 32) | b; }
#endif

// If we are in debugging mode, enable exception throwing
// Otherwise ignore the checks like with assert
#ifndef NDEBUG
class SatSolverException : public std::logic_error
{
public:
    explicit SatSolverException(const char* message) : std::logic_error(message) {};
};
#define Assert(NCOND, MESSAGE) do { if (!(NCOND)) throw SatSolverException(MESSAGE); } while (0)
constexpr const char* ILLEGAL_LITERAL = "Found illegal literal when adding clause";
constexpr const char* REQUIRE_SAT     = "Solver must be in STATE_SAT state";
#else
#define Assert(NCOND, MESSAGE)
#endif

class SatSolver {
public:
    enum state_t {STATE_SAT = 10, STATE_UNSAT = 20, STATE_INPUT = 30};
private:
    /// Current state of the solver
    state_t m_state;
    /// The number of currently allocated solver variables
    int m_num_vars;
    /// The number of currently added solver clauses
    int m_num_clauses;
    /// IPASIR solver object with generic (void*) type
    void* m_solver;

    #ifdef OPT_EXPR_CACHING
    /// Cache for AND gates
    std::unordered_map<gate_key_t, var_t> m_and_cache;
    /// Cache for XOR gates
    std::unordered_map<gate_key_t, var_t> m_xor_cache;
    #endif

    /// Internal ipasir_add forwarding
    inline void add(var_t x);

    /// Performs checks whether the clause is a tautology, or contains illegal literals
    template<typename... Ts>
    bool check_clause_inner(var_t head, Ts... tail);

    /// Recursive template function that simplifies and adds a clause into the solver
    template<typename... Ts>
    void add_clause_inner(var_t head, Ts... tail);

public:
    /// Allocates and returns a new solver variable
    inline var_t new_var() noexcept;
    /// Allocates \a number many solver variables and returns the first one
    inline var_t new_vars(int number) noexcept;
    /// Returns the number of currently used variables
    inline int num_vars() noexcept;
    /// Returns the number of currently added clauses
    inline int num_clauses() noexcept;
    /// Returns the current state of the solver
    inline state_t state() { return m_state; }

    /// Creates a new variable representing the xor of \a a and \a b
    var_t make_xor(var_t a, var_t b);
    /// Creates a new variable representing the and of \a a and \a b
    var_t make_and(var_t a, var_t b);

    /// Public template function for adding clauses into the solver
    template<typename... Ts>
    void add_clause(var_t head, Ts... tail);

    /// Main satisfiability checking routine
    state_t check() noexcept;
    /// Return the value assigned to variable \a a
    bool value(var_t a);

    /// Only default constructor is implemented
    SatSolver();
    /// Destructor destroying the internal IPASIR object
    ~SatSolver();
};

inline void SatSolver::add(var_t x)
{
    ipasir_add(m_solver, x);
    DEBUG(2) << x << " ";
}

inline var_t SatSolver::new_vars(const int number) noexcept
{
    const var_t var = m_num_vars;
    m_num_vars += number;
    return var + 1;
}

inline var_t SatSolver::new_var() noexcept
{
    return new_vars(1);
}

inline int SatSolver::num_vars() noexcept
{
    return m_num_vars;
}

inline int SatSolver::num_clauses() noexcept
{
    return m_num_clauses;
}

template<>
inline void SatSolver::add_clause_inner(var_t head)
{
    if (head != ZERO) add(head);
    add(0); // terminate the clause
    m_num_clauses += 1;
    m_state = STATE_INPUT;
}

template<typename... Ts>
inline void SatSolver::add_clause_inner(var_t head, Ts... tail)
{
    if (head != ZERO) add(head);
    add_clause_inner(tail...);
}

template<typename... Ts>
inline void SatSolver::add_clause(var_t head, Ts... tail)
{
    if(!check_clause_inner(head, tail...))
    {
        DEBUG(2) << "Eliminated clause" << std::endl;
        return;
    }
    DEBUG(2) << "Adding clause: ";
    add_clause_inner(head,tail...);
    DEBUG(2) << std::endl;
}

template<>
inline bool SatSolver::check_clause_inner(var_t head)
{
    Assert(is_legal(head), ILLEGAL_LITERAL);
    return (head != ONE);
}

template<typename... Ts>
inline bool SatSolver::check_clause_inner(var_t head, Ts... tail) {
    Assert(is_legal(head), ILLEGAL_LITERAL);
    return (head != ONE) && check_clause_inner(tail...);
}

#endif // SATSOLVER_H