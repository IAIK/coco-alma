#pragma once
#include <cstdint>

/// Hacky debugging utility
#define DEBUG std::cout
// #define DEBUG 0 && std::cout

/// Type used for solver variables
using var_t = int32_t;

/// Type used for label indexing
using lidx_t = uint32_t;

/// There are only two illegal int values for solver literals
/// We use them here to represent the constants 0(false) and 1(true)
constexpr var_t ZERO = 0;
constexpr var_t ONE  = INT32_MIN;
