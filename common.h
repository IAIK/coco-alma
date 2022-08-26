#ifndef COMMON_H
#define COMMON_H

#include <cstdint>

/// Hacky debugging utility
#define VERBOSITY 2L
#define DEBUG(l) (l <= VERBOSITY) && std::cout

/// Type for solver variables and literals
using var_t = int32_t;
/// Type used for label indexing
using lidx_t = uint32_t;

/// There are only two illegal int values for solver literals.
/// These are 0 and INT_MIN, whose negation is themselves.
inline bool is_legal(var_t x) { return (-x != x); }

/// We sacrifice one variable to use as true(ONE) and false(ZERO).
/// Most solvers do not support so many variables anyway.
constexpr var_t ZERO = -INT32_MAX;
constexpr var_t ONE  = +INT32_MAX;

#endif // COMMON_H