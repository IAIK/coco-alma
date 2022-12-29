#include "VarManager.h"

VarManager::VarManager() : m_num_vars(0) { }

var_t VarManager::simplify_and(var_t a, var_t b)
{
    Assert(is_legal(a), ILLEGAL_LITERAL);
    Assert(is_legal(b), ILLEGAL_LITERAL);
    Assert(is_known(a), UNKNOWN_LITERAL);
    Assert(is_known(b), UNKNOWN_LITERAL);

    // Standard rules for and with constant
    if (a == ZERO || b == ZERO) return ZERO;
    if (a == ONE) return b;
    if (b == ONE) return a;
    // Simplification rules for and
    if (a == b) return a;
    if (a == -b) return ZERO;

    // See if we already have a variable for this
    const gate_key_t key = make_key(a < b ? a : b, a < b ? b : a);
    auto res = m_and_cache.find(key);
    if (res != m_and_cache.end()) return res->second;
    return 0;
}

void VarManager::register_and(gate_key_t key, var_t c)
{
    const auto a = (var_t)(key & UINT32_MAX);
    const auto b = (var_t)(key >> 32);

    Assert(is_legal(a), ILLEGAL_LITERAL);
    Assert(is_legal(b), ILLEGAL_LITERAL);
    Assert(is_legal(c), ILLEGAL_LITERAL);

    Assert(is_known(a), UNKNOWN_LITERAL);
    Assert(is_known(b), UNKNOWN_LITERAL);
    Assert(is_known(c), UNKNOWN_LITERAL);

    m_and_cache.emplace(key, c);
}
