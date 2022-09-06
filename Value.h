#ifndef VALUE_H
#define VALUE_H

#include <utility>
#include <cassert>

#include "PropVarSet.h"
#include "GlitchyPVS.h"

template<verif_mode_t mode>
class Simulator;

template<verif_mode_t mode>
class ValueView;


template<verif_mode_t mode>
class Value
{
    /// Constant value of object stored as a boolean
    bool             m_const_value;
    /// Flag indicating whether the constant is stable during computation
    bool             m_const_stable;
    /// The stable PVS representing this value
    PropVarSetPtr    m_stable_pvs;
    /// The glitchy PVS representing this value
    GlitchyPVS<mode> m_glitch_pvs;
    friend ValueView<mode>;
public:

    /// Simple constructor from boolean value, available in all verification modes
    explicit constexpr Value(bool val);

    /// Default constructor based on boolean constructor
    constexpr Value() : Value(false) {};

    /// Constructor from PVS which is different in different verification modes
    explicit Value(PropVarSetPtr p_pvs);

    /// Returns whether the value is a constant in the stable verification mode
    bool is_stable_const() const
    { return m_stable_pvs.get() == nullptr; }

    /// Returns whether the value is a constant in the glitchy verification mode
    bool is_glitch_const() const
    { return m_glitch_pvs.get() == nullptr; }

    /// Collection of sanity checks that ensure the object is well-formed
    constexpr void sanity() const;

    /// Simplifying operator NOT (~)
    template<verif_mode_t M>
    friend Value<M> operator~(const Value<M>& a);

    /// Simplifying operator AND (&)
    template<verif_mode_t M>
    friend Value<M> operator&(const Value<M>& a, const Value<M>& b);

    /// Simplifying operator XOR (^)
    template<verif_mode_t M>
    friend Value<M> operator^(const Value<M>& a, const Value<M>& b);

    /// Simplifying operator OR (|)
    template<verif_mode_t M>
    friend Value<M> operator|(const Value<M>& a, const Value<M>& b);
};

template <verif_mode_t mode>
class ValueView
{
private:
    Value<mode>& m_value;
public:
    ValueView() = delete;
    ValueView(const ValueView& other) = delete;
    explicit ValueView(Value<mode>& value) : m_value(value) {};
    ValueView& operator=(Value<mode>& other);
};

#include "Value.hpp"

#endif // VALUE_H