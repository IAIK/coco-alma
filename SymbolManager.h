#ifndef SYMBOLMANAGER_H
#define SYMBOLMANAGER_H

#include "VarManager.h"

class SymbolManager : public VarManager
{
public:
    /// Creates a new variable representing the and of \a a and \a b
    var_t make_and(var_t a, var_t b) override;
    /// Creates a new variable representing the or of \a a and \a b
    var_t make_or(var_t a, var_t b);
    /// Creates a new variable representing the xor of \a a and \a b
    var_t make_xor(var_t a, var_t b);
    /// Creates a new variable representing the mux of \a s, \a t and \a e
    var_t make_mux(var_t s, var_t t, var_t e);
};


#endif // SYMBOLMANAGER_H
