#ifndef SYMBOL_H
#define SYMBOL_H

#include "SymbolManager.h"

class Symbol
{
    /// Manager in charge of this symbol
    static SymbolManager s_manager;
    /// Variable identifier for this symbol
    var_t m_variable;

    /// Constructor from variable index
    explicit Symbol(var_t variable): m_variable(variable) {}

public:
    /// Default constructor creating an invalid symbol
    Symbol() : m_variable(0) {}

    /// Constructor from boolean
    explicit Symbol(bool value): m_variable(to_var(value)) {}

    /// We use the default assignment operator
    Symbol(const Symbol& other) = default;

    /// We use the default assignment operator
    Symbol& operator=(const Symbol& other) = default;

    /// We define a custom assignment operator from booleans
    Symbol& operator=(bool other_val) { m_variable = to_var(other_val); return *this; };
    Symbol& operator=(var_t other_val) = delete;
    Symbol& operator=(uint8_t other_val) = delete;
    Symbol& operator=(uint16_t other_val) = delete;
    Symbol& operator=(uint32_t other_val) = delete;
    Symbol& operator=(uint64_t other_val) = delete;

    /// Returns the variable index
    inline var_t var() const { return m_variable; }

    /// Creates a new symbolic value
    friend Symbol create_symbol();

    /// Simplifying operator NOT (!)
    friend Symbol operator!(const Symbol& a);

    /// Simplifying operator AND (&)
    friend Symbol operator&(const Symbol& a, const Symbol& b);

    /// Simplifying operator XOR (^)
    friend Symbol operator^(const Symbol& a, const Symbol& b);

    /// Simplifying operator OR (|)
    friend Symbol operator|(const Symbol& a, const Symbol& b);

    /// Simplifying operator MUX (?:)
    friend Symbol mux(const Symbol& cond, const Symbol& t_val, const Symbol& e_val);

    /// Compares two Symbols by variable index
    friend bool operator<(const Symbol& a, const Symbol& b);

    /// Compares two Symbols for equality
    friend bool operator==(const Symbol& a, const Symbol& b);
    friend bool operator!=(const Symbol& a, const Symbol& b);
};

Symbol create_symbol();
Symbol mux(const Symbol& s, const Symbol& t, const Symbol& e);

#endif // SYMBOL_H
