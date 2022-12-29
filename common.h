#ifndef COMMON_H
#define COMMON_H

#include <cstdint>
#include <iostream>

/// Hacky debugging utility
#define VERBOSITY 0L
#define DEBUG(l) ((l) <= VERBOSITY) && std::cout

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

/// Type for verification modes
enum class verif_mode_t : uint8_t
{ MODE_SYMBOLIC = 0x0, MODE_STABLE = 0x1, MODE_GLITCH = 0x3, MODE_HAMMING = 0x5,
  MODE_TRANSIENT = 0x7 };

/// Returns whether glitches are considered in the verification mode
constexpr bool has_stable(verif_mode_t mode)
{ return static_cast<uint8_t>(mode) & 0x1; }

/// Returns whether glitches are considered in the verification mode
constexpr bool has_glitches(verif_mode_t mode)
{ return static_cast<uint8_t>(mode) & 0x2; }

/// Returns whether transitions are considered in the verification mode
constexpr bool has_hamming(verif_mode_t mode)
{ return static_cast<uint8_t>(mode) & 0x4; }

/// Simple implication utility function
constexpr bool implies(const bool a, const bool b)
{ return !a || b; }

#endif // COMMON_H