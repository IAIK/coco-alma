#ifndef GLITCHYPVS_H
#define GLITCHYPVS_H

#include <cstdint>
#include <type_traits>
#include "common.h"
#include "PropVarSet.h"

/// Definition of empty class, should not take up space
template<verif_mode_t mode, typename enable = void>
struct GlitchyPVS
{
    friend const GlitchyPVS& operator&(const GlitchyPVS& a, const GlitchyPVS& b) { return a; }
    friend const GlitchyPVS& operator^(const GlitchyPVS& a, const GlitchyPVS& b) { return a; }
    friend const GlitchyPVS& operator|(const GlitchyPVS& a, const GlitchyPVS& b) { return a; }
    friend const GlitchyPVS& operator+(const GlitchyPVS& a) { return a; }

    constexpr void* get() const { return nullptr; }
    constexpr uint32_t operator*() const { return 0; }
};

/// In case the mode requires glitches, it is just a PropVarSetPtr
template<verif_mode_t mode>
struct GlitchyPVS<mode, std::enable_if_t<has_glitches(mode)>> : public PropVarSetPtr
{
    GlitchyPVS() : PropVarSetPtr() {}
    GlitchyPVS(PropVarSetPtr pvs) : PropVarSetPtr(pvs) { }

    friend GlitchyPVS operator&(const GlitchyPVS& a, const GlitchyPVS& b)
    { return ((PropVarSetPtr)a) & ((PropVarSetPtr)b); }

    friend GlitchyPVS operator^(const GlitchyPVS& a, const GlitchyPVS& b)
    { return ((PropVarSetPtr)a) ^ ((PropVarSetPtr)b); }

    friend GlitchyPVS operator|(const GlitchyPVS& a, const GlitchyPVS& b)
    { return ((PropVarSetPtr)a) | ((PropVarSetPtr)b); }

    friend GlitchyPVS operator+(const GlitchyPVS& a)
    { return +((PropVarSetPtr)a); }
};

#endif // GLITCHYPVS_H
