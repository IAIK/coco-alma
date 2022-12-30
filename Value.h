#ifndef VALUE_H
#define VALUE_H

#include <utility>
#include <cassert>

#include "PropVarSet.h"
#include "Symbol.h"

using Range = std::pair<size_t, size_t>;

template<verif_mode_t mode>
class Simulator;

template<verif_mode_t mode>
class ValueView;

enum class value_display_t : uint8_t
{ DISPLAY_STABLE = 0x1, DISPLAY_GLITCH = 0x2 };

struct Empty {};

template<verif_mode_t mode>
class Value
{
    using StablePVSPtr = std::conditional_t<has_stable(mode), PropVarSetPtr, Empty>;
    /// Defines the type used for Glitch tracking PVS
    using GlitchyPVSPtr = std::conditional_t<has_glitches(mode), PropVarSetPtr, Empty>;

    /// Constant value of object stored as a boolean
    Symbol m_symbolic_value;
    /// Flag indicating whether the constant is stable during computation
    bool m_stability;
    /// The stable PVS representing this value
    [[no_unique_address]] StablePVSPtr  m_stable_pvs;
    /// The glitchy PVS representing this value
    [[no_unique_address]] GlitchyPVSPtr m_glitch_pvs;

    using gate_key_t = std::tuple<var_t, PropVarSetPtr, var_t, PropVarSetPtr>;
    static std::map<gate_key_t, PropVarSetPtr> and_cache;
    static std::map<gate_key_t, PropVarSetPtr> xor_cache;

    /// The ValueView must be able to access and modify values
    friend ValueView<mode>;
public:
    /// Default constructor instantiating object in global context
    constexpr Value();

    /// Simple boolean constructor in global context
    explicit constexpr Value(bool val) : Value() { m_symbolic_value = val; m_stability = true; }

    /// Simple boolean constructor in global context TODO: Why was this here
    /// explicit constexpr Value(int val) : Value() { assert(val >= 0 & val <= 1); m_const_value = val; m_stability = true; }

    /// Constructor from PVS which is different in different verification modes
    Value(Symbol sym, PropVarSetPtr p_pvs);

    /// Explicitly declared default assignment operator
    Value<mode>& operator=(const Value<mode>& other) = default;

    /// Explicitly declared number assignment operator
    Value<mode>& operator=(bool val);

    Value<mode>& operator=(uint8_t val) = delete;
    Value<mode>& operator=(uint16_t val) = delete;
    Value<mode>& operator=(uint32_t val) = delete;

    /// Explicitly declared ValueView assignment operator
    Value<mode>& operator=(const ValueView<mode>& other_view);

    /// Returns whether the value is a constant in the stable verification mode
    bool is_stable_const() const
    { return m_stable_pvs.get() == nullptr; }

    /// Returns whether the value is a constant in the glitchy verification mode
    bool is_glitch_const() const
    {
        if constexpr(!has_glitches(mode)) return true;
        else return m_glitch_pvs.get() == nullptr;
    }

    /// Collection of sanity checks that ensure the object is well-formed
    constexpr void sanity() const;

    /// Safe getters for the different fields the value can have
    bool const_val() const;
    bool is_signal_stable() const;
    const StablePVSPtr& stable_pvs() const;
    const GlitchyPVSPtr& glitch_pvs() const;

    static PropVarSetPtr make_and(var_t a_var, PropVarSetPtr a_pvs,
                                  var_t b_var, PropVarSetPtr b_pvs);
    static PropVarSetPtr make_xor(var_t a_var, PropVarSetPtr a_pvs,
                                  var_t b_var, PropVarSetPtr b_pvs);


    /// Stabilizing operator PLUS (+) TODO: weird?
    template<verif_mode_t M>
    friend Value<M> operator+(const Value<M>& a);

    /// Simplifying operator NOT (!)
    template<verif_mode_t M>
    friend Value<M> operator!(const Value<M>& a);

    /// Simplifying operator AND (&)
    template<verif_mode_t M>
    friend Value<M> operator&(const Value<M>& a, const Value<M>& b);

    /// Simplifying operator XOR (^)
    template<verif_mode_t M>
    friend Value<M> operator^(const Value<M>& a, const Value<M>& b);

    /// Simplifying operator OR (|)
    template<verif_mode_t M>
    friend Value<M> operator|(const Value<M>& a, const Value<M>& b);

    /// Simplifying operator MUX (?:)
    template<verif_mode_t M>
    friend Value<M> mux(const Value<M>& cond, const Value<M>& t_val, const Value<M>& e_val);

    /// Compares two Values lexicographically
    template<verif_mode_t M>
    friend bool operator<(const Value<M>& a, const Value<M>& b);

    /// Compares two Values lexicographically
    template<verif_mode_t M>
    friend bool operator==(const Value<M>& a, const Value<M>& b);

    template<verif_mode_t M>
    friend bool operator!=(const Value<M>& a, const Value<M>& b);

    /// What is to be displayed in an output stream
    static value_display_t s_display;

    /// Write Value onto output stream
    template<verif_mode_t M>
    friend std::ostream& operator<<(std::ostream& out, const Value<M>& val);
};

template <verif_mode_t mode>
value_display_t Value<mode>::s_display = value_display_t::DISPLAY_STABLE;

template <verif_mode_t mode>
std::map<typename Value<mode>::gate_key_t, PropVarSetPtr> Value<mode>::and_cache;

template <verif_mode_t mode>
std::map<typename Value<mode>::gate_key_t, PropVarSetPtr> Value<mode>::xor_cache;


#include "Value.hpp"

#endif // VALUE_H