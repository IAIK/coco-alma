#include "SymbolManager.h"

var_t SymbolManager::make_and(const var_t a, const var_t b)
{
    var_t c = simplify_and(a, b);
    if (c != 0) return c;

    c = new_var();
    const gate_key_t key = make_key(a < b ? a : b, a < b ? b : a);
    register_and(key, c);
    return c;
}

var_t SymbolManager::make_or(var_t a, var_t b)
{
    return -make_and(-a, -b);
}

var_t SymbolManager::make_xor(var_t a, var_t b)
{
    var_t pos_neg = make_and(a, -b);
    var_t neg_pos = make_and(-a, b);
    return make_or(pos_neg, neg_pos);
}

var_t SymbolManager::make_mux(var_t s, var_t t, var_t e)
{
    Assert(is_known(s), UNKNOWN_LITERAL);
    Assert(is_known(t), UNKNOWN_LITERAL);
    Assert(is_known(e), UNKNOWN_LITERAL);

    // The formula representation is (s & t) | (-s & e)
    if (s == ONE) return t;  // ... = t | 0 = t
    if (s == ZERO) return e; // ... = 0 | e = e

    if (t == ONE) return make_or(s, e);    // ... = s | (-s & e) = s | e
    if (t == ZERO) return make_and(-s, e); // ... = 0 | (-s & e) = (-s & e)

    if (e == ONE) return make_or(-s, t);   // ... = (s & t) | -s  = -s | t
    if (e == ZERO) return make_and(s, t);  // ... = (s & t) | 0 = (s & t)


    if (t == e) return t;               // ... = t & (-s | s) = t
    if (t == -e) return make_xor(s, e); // ... = (s & -e) | (-s & e)

    if (t == s) return make_or(s, e);    // ... = s | (-s & e) = s | e
    if (t == -s) return make_and(-s, e); // ... = 0 | (-s & e) = (-s & e);

    if (e == s) return make_and(s, t);   // ... = (s & t) | 0  = (s & t)
    if (e == -s) return make_or(-s, t);  // ... = (s & t) | -s = -s | t

    // every simplification case should be handled by this point
    var_t s_and_t = make_and(s, t);
    var_t not_s_and_e = make_and(-s, e);
    return make_or(s_and_t, not_s_and_e);
}
