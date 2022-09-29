//
// Created by vhadzic on 30.08.22.
//

#include "Value.h"

// This is the constructor from pvs when in stable mode
template<>
Value<verif_mode_t::MODE_STABLE>::Value(PropVarSetPtr p_pvs)
        : m_const_value(false), m_const_stable(true), m_stable_pvs(std::move(p_pvs)), m_glitch_pvs()
{ }

// This is the constructor from pvs when in glitchy mode
template<>
Value<verif_mode_t::MODE_GLITCH>::Value(PropVarSetPtr p_pvs)
        : m_const_value(false), m_const_stable(true), m_stable_pvs(p_pvs), m_glitch_pvs(std::move(p_pvs))
{ }
