#ifndef GLITCHYPVS_H
#define GLITCHYPVS_H

#include <cstdint>
#include <type_traits>
#include "common.h"
#include "PropVarSet.h"

/// Definition of empty class compatible with PropVarSet
struct NoPVS
{
    NoPVS() = default;
    NoPVS(PropVarSetPtr pvs) { }

    friend const NoPVS& operator&(const NoPVS& a, const NoPVS& b) { return a; }
    friend const NoPVS& operator^(const NoPVS& a, const NoPVS& b) { return a; }
    friend const NoPVS& operator|(const NoPVS& a, const NoPVS& b) { return a; }
    friend const NoPVS& operator+(const NoPVS& a) { return a; }

    constexpr void* get() const { return nullptr; }
    constexpr uint32_t operator*() const { return 0; }
};

#endif // GLITCHYPVS_H
