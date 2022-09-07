#ifndef VALUE_H
#define VALUE_H

#include <utility>
#include <cassert>
#include <vector>

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

    /// Safe getters for the different fields the value can have
    bool stable_val() const;
    bool glitch_val() const;
    const PropVarSetPtr& stable_pvs() const;
    const GlitchyPVS<mode>& glitch_pvs() const;

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
    ValueView(const ValueView& other) = default;
    explicit ValueView(Value<mode>& value) : m_value(value) {};
    ValueView& operator=(const Value<mode>& other);
    ValueView& operator=(uint64_t val);
    Value<mode>& operator*() const { return m_value; };
};

template <verif_mode_t mode>
class ValueViewVector
{
private:
    std::vector<ValueView<mode>> m_views;
    using Range = std::pair<size_t, size_t>;

    ValueViewVector(const ValueViewVector& other, size_t up, size_t down);
public:
    ValueViewVector() = default;
    ValueView<mode>& operator[](const size_t id) { return m_views.at(id); }
    ValueViewVector operator[](const Range range)
    { return ValueViewVector(*this, range.first, range.second); }
    void push_back(ValueView<mode> elem) { m_views.push_back(elem); }
    void pop_back() { m_views.pop_back(); }
    ValueView<mode>& back() { m_views.back(); }
    ValueView<mode>& front() { m_views.front(); }
    size_t size() { return m_views.size(); }
    ValueViewVector& operator=(uint64_t val);
};

#include "Value.hpp"

#endif // VALUE_H