#ifndef VARMANAGER_H
#define VARMANAGER_H

#include "common.h"
#include <unordered_map>

using gate_key_t = uint64_t;
constexpr gate_key_t make_key(var_t a, var_t b) { return ((gate_key_t)(a) << 32) | b; }

// If we are in debugging mode, enable exception throwing
// Otherwise ignore the checks like with assert
#ifndef NDEBUG
#define Assert(NCOND, MESSAGE) do { if (!(NCOND)) throw std::logic_error(MESSAGE); } while (0)
constexpr const char* ILLEGAL_LITERAL = "Found illegal literal when adding clause";
constexpr const char* UNKNOWN_LITERAL = "Found unknown literal when adding clause";
#else
#define Assert(NCOND, MESSAGE)
#endif

class VarManager {
private:
    /// The number of currently allocated solver variables
    int m_num_vars;
protected:
    /// Cache for AND gates
    std::unordered_map<gate_key_t, var_t> m_and_cache;
    /// Checks for simplification for the and of \a a and \a b
    var_t simplify_and(var_t a, var_t b);
    /// Checks for simplification for the and of \a a and \a b
    void register_and(gate_key_t key, var_t c);
public:
    /// Allocates \a number many solver variables and returns the first one
    inline var_t new_vars(int number) noexcept;
    /// Allocates and returns a new solver variable
    inline var_t new_var() noexcept { return new_vars(1); };
    /// Returns the number of currently used variables
    inline int num_vars() const noexcept { return m_num_vars; };
    /// Returns true if provided variable is known
    inline bool is_known(var_t a) const { return (a <= m_num_vars && a >= -m_num_vars) || a == ZERO || a == ONE; }

    /// Creates a new variable representing the and of \a a and \a b
    virtual var_t make_and(var_t a, var_t b) = 0;

    /// Only default constructor is implemented
    VarManager();
    /// We use the default destructor since there are no directly allocated members
    ~VarManager() = default;
};

inline var_t VarManager::new_vars(const int number) noexcept
{
    const var_t var = m_num_vars;
    m_num_vars += number;
    return var + 1;
}


#endif // VARMANAGER_H
