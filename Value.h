#ifndef VALUE_H
#define VALUE_H

#include <utility>
#include <cassert>
#include <vector>

#include "PropVarSet.h"
#include "NoPVS.h"

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
    bool m_const_value;
    /// The stable PVS representing this value
    [[no_unique_address]] StablePVSPtr  m_stable_pvs;
    /// The glitchy PVS representing this value
    [[no_unique_address]] GlitchyPVSPtr m_glitch_pvs;
    /// Flag indicating whether the constant is stable during computation
    bool m_const_stability;

    /// The ValueView must be able to access and modify values
    friend ValueView<mode>;
public:
    /// Default constructor instantiating object in global context
    constexpr Value();

    /// Simple boolean constructor in global context
    explicit constexpr Value(bool val) : Value() { m_const_value = val; m_const_stability = true; }

    /// Simple boolean constructor in global context
    explicit constexpr Value(int val) : Value() { assert(val >= 0 & val <= 1); m_const_value = val; m_const_stability = true; }

    /// Constructor from PVS which is different in different verification modes
    explicit Value(PropVarSetPtr p_pvs);

    /// Explicitly declared default assignment operator
    Value<mode>& operator=(const Value<mode>& other) = default;

    /// Explicitly declared number assignment operator
    Value<mode>& operator=(uint64_t val);

    /// Explicitly declared ValueView assignment operator
    Value<mode>& operator=(const ValueView<mode>& other_view);

    /// Returns whether the value is a constant in the stable verification mode
    bool is_stable_const() const
    { return m_stable_pvs.get() == nullptr; }

    /// Returns whether the value is a constant in the glitchy verification mode
    bool is_glitch_const() const
    { return m_glitch_pvs.get() == nullptr; }

    /// Collection of sanity checks that ensure the object is well-formed
    constexpr void sanity() const;

    /// Safe getters for the different fields the value can have
    bool stable_val() const;
    bool glitch_val() const;
    bool is_signal_stable() const;
    const PropVarSetPtr& stable_pvs() const;
    const GlitchyPVSPtr& glitch_pvs() const;

    /// Stabilizing operator PLUS (+)
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
class ValueView
{
private:
    Value<mode>& m_value;
public:
    ValueView() = delete;
    ValueView(const ValueView& other) = default;
    explicit ValueView(Value<mode>& value) : m_value(value) {};
    ValueView& operator=(const Value<mode>& other);
    ValueView& operator=(uint64_t val);
    Value<mode>& get() const { return m_value; };
};

template <verif_mode_t mode>
class ValueRef
{
private:
    Value<mode>& m_value;
public:
    ValueRef() = delete;
    ValueRef(const ValueRef& other) = default;
    explicit ValueRef(Value<mode>& value) : m_value(value) {};
    ValueRef& operator=(const Value<mode>& other) { m_value = other; return *this; };
    ValueRef& operator=(uint64_t val) { m_value = val; return *this; };
    Value<mode>& get() const { return m_value; };
};

template<verif_mode_t mode>
using ValueVector = std::vector<Value<mode>>;

template <verif_mode_t mode>
class ValueViewVector
{
public:
    // using Value = ValueView<mode>;
    using ValueT = ValueRef<mode>;
private:
    std::vector<ValueT> m_views;
    ValueViewVector(const ValueViewVector& other, size_t up, size_t down);
public:
    ValueViewVector() = default;
    ValueT& operator[](const size_t id) { return m_views.at(id); }
    ValueViewVector operator[](const Range range)
    { return ValueViewVector(*this, range.first, range.second); }
    void push_back(ValueT elem) { m_views.push_back(elem); }
    void pop_back() { m_views.pop_back(); }
    ValueT& back() { m_views.back(); }
    ValueT& front() { m_views.front(); }
    size_t size() const { return m_views.size(); }
    ValueViewVector& operator=(uint64_t vals);
    ValueViewVector& operator=(const ValueVector<mode>& vals);
    bool is_const();
    uint64_t as_uint64_t();
    template <verif_mode_t M>
    friend std::ostream& operator<<(std::ostream& out, const ValueViewVector<M>& view_vector);
};

#include "Value.hpp"

#endif // VALUE_H