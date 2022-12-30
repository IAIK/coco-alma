#include "Symbol.h"

SymbolManager Symbol::s_manager;

Symbol create_symbol()
{
    return Symbol(Symbol::s_manager.new_var());
}

Symbol operator!(const Symbol& a)
{
    return Symbol(-a.m_variable);
}

Symbol operator&(const Symbol& a, const Symbol& b)
{
    return Symbol(Symbol::s_manager.make_and(a.m_variable, b.m_variable));
}

Symbol operator^(const Symbol& a, const Symbol& b)
{
    return Symbol(Symbol::s_manager.make_xor(a.m_variable, b.m_variable));
}

Symbol operator|(const Symbol& a, const Symbol& b)
{
    return Symbol(Symbol::s_manager.make_or(a.m_variable, b.m_variable));
}

Symbol mux(const Symbol& s, const Symbol& t, const Symbol& e)
{
    return Symbol(Symbol::s_manager.make_mux(s.m_variable, t.m_variable, e.m_variable));
}

bool operator<(const Symbol& a, const Symbol& b)
{
    return a.m_variable < b.m_variable;
}

bool operator==(const Symbol& a, const Symbol& b)
{
    return a.m_variable == b.m_variable;
}

bool operator!=(const Symbol& a, const Symbol& b)
{
    return !(a == b);
}