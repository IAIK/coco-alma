#ifdef VALUE_H

#include "Cell.h"
#include "Value.h"


template <verif_mode_t mode>
constexpr void Value<mode>::sanity() const
{
    assert((m_const_value & 0x01) == m_const_value);
    assert((m_const_stability & 0x01) == m_const_stability);
    assert(implies(has_glitches(mode) && is_glitch_const(), is_stable_const()));
    assert(implies(!is_stable_const(), m_const_value == false));
    assert(implies(has_glitches(mode) && !is_glitch_const(), m_const_stability == false));
    assert(implies(!has_glitches(mode), m_const_stability == true));
}

template <verif_mode_t mode>
bool Value<mode>::stable_val() const
{
    if (!is_stable_const())
        throw std::logic_error(ILLEGAL_VALUE_NOT_CONST);
    return m_const_value;
}

template <verif_mode_t mode>
bool Value<mode>::glitch_val() const
{
    if (!is_glitch_const())
        throw std::logic_error(ILLEGAL_VALUE_NOT_CONST);
    return m_const_value;
}

template <verif_mode_t mode>
bool Value<mode>::is_signal_stable() const
{
    if (!is_glitch_const()) return false;
    return m_const_stability;
}

template <verif_mode_t mode>
const PropVarSetPtr& Value<mode>::stable_pvs() const
{
    if (is_stable_const())
        throw std::logic_error(ILLEGAL_VALUE_NOT_CONST);
    return m_stable_pvs;
}

template <verif_mode_t mode>
const typename Value<mode>::GlitchyPVSPtr& Value<mode>::glitch_pvs() const
{
    if (is_glitch_const())
        throw std::logic_error(ILLEGAL_VALUE_NOT_CONST);
    return m_glitch_pvs;
}

template<verif_mode_t mode>
constexpr Value<mode>::Value()
        : m_const_value(false), m_const_stability(true), m_stable_pvs(), m_glitch_pvs()
{}

template<verif_mode_t M>
Value<M> operator!(const Value<M>& a)
{
    a.sanity();

    Value<M> result = a;
    if (a.is_stable_const())
        { result.m_const_value = !a.m_const_value; }

    result.sanity();
    return result;
}

template<verif_mode_t M>
Value<M> operator+(const Value<M>& a)
{
    a.sanity();

    Value<M> result = a;
    result.m_glitch_pvs = result.m_stable_pvs;

    result.sanity();
    return result;
}

template<verif_mode_t mode>
Value<mode> operator&(const Value<mode>& a, const Value<mode>& b)
{
    a.sanity();
    b.sanity();

    Value<mode> result(false);
    assert(result.m_const_value == false);
    assert(result.is_glitch_const());
    assert(result.is_stable_const());

    if constexpr (has_glitches(mode))
    { // Perform the case distinction for the glitch computation
        const bool order = a.is_glitch_const();
        const Value<mode>& left =  order ? a : b;
        const Value<mode>& right = order ? b : a;

        // What is not covered is the stability computation
        result.m_const_stability = (left.is_signal_stable() && right.is_signal_stable()) ||
                                   (left.is_signal_stable() && left.m_const_value == false) ||
                                   (right.is_signal_stable() && right.m_const_value == false);

        if (left.is_glitch_const() && right.is_glitch_const())
        {
            // It is a constant in both cases, result is a constant
            result.m_const_value = left.m_const_value & right.m_const_value;
            assert(result.is_glitch_const());
        }
        else if (!left.is_glitch_const() && !right.is_glitch_const())
        {
            // It is symbolic in both cases, result is symbolic
            result.m_glitch_pvs = left.m_glitch_pvs & right.m_glitch_pvs;
            assert(!result.is_glitch_const());
        }
        else
        {
            // It is constant in left and symbolic in right
            assert(left.is_glitch_const() && !right.is_glitch_const());
            assert(left.is_signal_stable() == left.m_const_stability);
            if (left.m_const_stability)
            {
                // The constant is stable, so we can simplify
                if(left.m_const_value == false)
                {
                    // If the constant value is false, then result is constant
                    assert(result.is_glitch_const());
                    assert(result.m_const_value == false);
                    assert(result.m_const_stability == true);
                }
                else
                {
                    // If the constant value is true, then result is symbolic
                    result.m_glitch_pvs = right.m_glitch_pvs;
                    assert(!result.is_glitch_const());
                    assert(result.m_const_stability == false);
                }
            }
            else
            {
                // The constant is not stable, and could make the result false
                // In other words, the result is the biased symbolic argument
                result.m_glitch_pvs = +right.m_glitch_pvs;
                assert(!result.is_glitch_const());
            }
        }
    }

    { // Perform the case distinction for the stable computation
        const bool order = a.is_stable_const();
        const Value<mode>& left =  order ? a : b;
        const Value<mode>& right = order ? b : a;

        if (left.is_stable_const() && right.is_stable_const())
        {
            // It is a constant in both cases, already computed if glitches are present
            assert(implies(has_glitches(mode) && a.is_glitch_const() && b.is_glitch_const(),
                           result.m_const_value == (left.m_const_value & right.m_const_value)));
            result.m_const_value = left.m_const_value & right.m_const_value;
            assert(result.is_stable_const());
        }
        else if (!left.is_stable_const() && !right.is_stable_const())
        {
            // It is symbolic in both cases, result is symbolic
            result.m_stable_pvs = left.m_stable_pvs & right.m_stable_pvs;
            assert(!result.is_stable_const());
        }
        else
        {
            // It is constant in left and symbolic in right
            assert(left.is_stable_const() && !right.is_stable_const());
            if (left.m_const_value == false)
            {
                // If the constant value is false, then result is constant
                assert(result.is_stable_const());
                assert(result.m_const_value == false);
            }
            else
            {
                // If the constant value is true, then result is symbolic
                result.m_stable_pvs = right.m_stable_pvs;
                assert(!result.is_stable_const());
            }
        }
    }

    result.sanity();

    return result;
}

template<verif_mode_t mode>
Value<mode> operator^(const Value<mode>& a, const Value<mode>& b)
{
    a.sanity();
    b.sanity();

    Value<mode> result(false);
    assert(result.m_const_value == false);
    assert(result.is_glitch_const());
    assert(result.is_stable_const());

    if constexpr (has_glitches(mode))
    { // Perform the case distinction for the glitch computation
        const bool order = a.is_glitch_const();
        const Value<mode>& left = order ? a : b;
        const Value<mode>& right = order ? b : a;

        // It is only stable if both parents are stable (and constants)
        result.m_const_stability = left.is_signal_stable() && right.is_signal_stable();

        if (left.is_glitch_const() && right.is_glitch_const())
        {
            // It is a constant in both cases, result is a constant
            result.m_const_value = (left.m_const_value ^ right.m_const_value);
            assert(result.is_glitch_const());
        } else if (!left.is_glitch_const() && !right.is_glitch_const())
        {
            // It is symbolic in both cases, result is symbolic
            result.m_glitch_pvs = left.m_glitch_pvs ^ right.m_glitch_pvs;
            assert(!result.is_glitch_const());
        } else
        {
            // It is constant in left and symbolic in right
            assert(left.is_glitch_const() && !right.is_glitch_const());
            // Whether the constant is stable or not does not matter
            // In all cases, the symbolic argument is forwarded
            result.m_glitch_pvs = right.m_glitch_pvs;
            assert(!result.is_glitch_const());
        }
    }

    { // Perform the case distinction for the stable computation
        const bool order = a.is_stable_const();
        const Value<mode>& left =  order ? a : b;
        const Value<mode>& right = order ? b : a;

        if (left.is_stable_const() && right.is_stable_const())
        {
            // It is a constant in both cases, already computed if glitches are present
            assert(implies(has_glitches(mode) && a.is_glitch_const() && b.is_glitch_const(),
                           result.m_const_value == (left.m_const_value ^ right.m_const_value)));
            result.m_const_value = left.m_const_value ^ right.m_const_value;
            assert(result.is_stable_const());
        }
        else if (!left.is_stable_const() && !right.is_stable_const())
        {
            // It is symbolic in both cases, result is symbolic
            result.m_stable_pvs = left.m_stable_pvs ^ right.m_stable_pvs;
            assert(!result.is_stable_const());
        }
        else
        {
            // It is constant in left and symbolic in right
            assert(left.is_stable_const() && !right.is_stable_const());
            // Inherit from right because xor with constants does not change the set
            result.m_stable_pvs = right.m_stable_pvs;
            assert(!result.is_stable_const());
        }
    }

    result.sanity();

    return result;
}

template<verif_mode_t mode>
Value<mode> operator|(const Value<mode>& a, const Value<mode>& b)
{
    return !(!a & !b);
}

template<verif_mode_t mode>
Value<mode> mux(const Value<mode>& cond, const Value<mode>& t_val, const Value<mode>& e_val)
{
    cond.sanity();
    t_val.sanity();
    e_val.sanity();

    Value<mode> result(false);
    assert(result.m_const_value == false);
    assert(result.is_glitch_const());
    assert(result.is_stable_const());

    if constexpr (has_glitches(mode))
    { // Perform the case distinction for the glitch computation

        if (cond.is_glitch_const())
        {
            // The selection signal is a constant in the glitchy domain
            if (cond.is_signal_stable())
            {
                // The selection signal is stable, so just mux the inputs
                if (cond.glitch_val())
                {
                    // Take everything from the then value
                    result.m_glitch_pvs = t_val.m_glitch_pvs;
                    result.m_const_value = t_val.m_const_value;
                    result.m_const_stability = t_val.m_const_stability;
                } else
                {
                    // Take everything from the else value
                    result.m_glitch_pvs = e_val.m_glitch_pvs;
                    result.m_const_value = e_val.m_const_value;
                    result.m_const_stability = e_val.m_const_stability;
                }
            }
            else
            {
                // The selection signal is unstable, result is both inputs simultaneously
                const bool order = t_val.is_glitch_const();
                const Value<mode>& left = order ? t_val : e_val;
                const Value<mode>& right = order ? e_val : t_val;

                result.m_const_stability = left.is_glitch_const() && right.is_glitch_const() &&
                                           left.m_const_value == right.m_const_value &&
                                           left.m_const_stability && right.m_const_stability;

                if (left.is_glitch_const() && right.is_glitch_const())
                {
                    result.m_const_value = cond.m_const_value ? t_val.m_const_value : e_val.m_const_value;
                    assert(result.is_glitch_const());
                } else if (!left.is_glitch_const() && !right.is_glitch_const())
                {
                    result.m_glitch_pvs = left.m_glitch_pvs | right.m_glitch_pvs;
                    assert(!result.is_glitch_const());
                    assert(result.m_const_stability == false);
                } else
                {
                    assert(left.is_glitch_const() && !right.is_glitch_const());
                    result.m_glitch_pvs = +right.m_glitch_pvs;
                    assert(!result.is_glitch_const());
                    assert(result.m_const_stability == false);
                }
            }
        } else
        {
            // The selection signal is not constant, mix it with the inputs
            const bool order = t_val.is_glitch_const();
            const Value<mode>& left = order ? t_val : e_val;
            const Value<mode>& right = order ? e_val : t_val;

            result.m_const_stability = t_val.is_glitch_const() && e_val.is_glitch_const() &&
                    left.m_const_value == right.m_const_value && left.m_const_stability && right.m_const_stability;

            if (left.is_glitch_const() && right.is_glitch_const())
            {
                // Both then and else are constants
                if (left.m_const_value == right.m_const_value)
                {
                    // We are selecting symbolically between the same constant
                    result.m_const_value = left.m_const_value;
                    assert(result.is_glitch_const());
                    assert(result.m_const_stability == left.m_const_stability && right.m_const_stability);
                }
                else
                {
                    // The result is influenced by the symbolic selection
                    result.m_glitch_pvs = cond.m_glitch_pvs;
                    assert(!result.is_glitch_const());
                    assert(result.m_const_stability == false);
                }
            }
            else if (!left.is_glitch_const() && !right.is_glitch_const())
            {
                // Both then and else are symbolic, create multiplexer pvs
                result.m_glitch_pvs = +cond.m_glitch_pvs ^ (t_val.m_glitch_pvs | e_val.m_glitch_pvs);
                assert(!result.is_glitch_const());
                assert(result.m_const_stability == false);
            }
            else
            {
                // Simplification of the case where both then and else are symbolic
                assert(left.is_glitch_const() && !right.is_glitch_const());
                result.m_glitch_pvs = cond.m_glitch_pvs & right.m_glitch_pvs;
                assert(!result.is_glitch_const());
                assert(result.m_const_stability == false);
            }
        }
    }

    { // Perform the case distinction for the stable computation
        if (cond.is_stable_const())
        {
            // The selection signal is stable, so just mux the inputs
            if (cond.stable_val())
            {
                // Take everything from the then value
                assert(implies(has_glitches(mode) && cond.is_glitch_const() && cond.is_signal_stable(),
                               result.m_const_value == t_val.m_const_value));
                result.m_stable_pvs = t_val.m_stable_pvs;
                result.m_const_value = t_val.m_const_value;
            } else
            {
                // Take everything from the else value
                assert(implies(has_glitches(mode) && cond.is_glitch_const() && cond.is_signal_stable(),
                               result.m_const_value == e_val.m_const_value));
                result.m_stable_pvs = e_val.m_stable_pvs;
                result.m_const_value = e_val.m_const_value;
            }
        } else
        {
            // The selection signal is not constant, mix it with the inputs
            const bool order = t_val.is_stable_const();
            const Value<mode>& left = order ? t_val : e_val;
            const Value<mode>& right = order ? e_val : t_val;

            if (left.is_stable_const() && right.is_stable_const())
            {
                // Both then and else are constants
                if (left.m_const_value == right.m_const_value)
                {
                    // We are selecting symbolically between the same constant
                    assert(implies(has_glitches(mode) && !cond.is_glitch_const() &&
                                        left.is_glitch_const() && right.is_glitch_const(),
                                   result.m_const_value == left.m_const_value));
                    result.m_const_value = left.m_const_value;
                    assert(result.is_stable_const());
                }
                else
                {
                    // The result is influenced by the symbolic selection
                    result.m_stable_pvs = cond.m_stable_pvs;
                    assert(!result.is_stable_const());
                }
            }
            else if (!left.is_stable_const() && !right.is_stable_const())
            {
                // Both then and else are symbolic, create multiplexer pvs
                result.m_stable_pvs = +cond.m_stable_pvs ^ (t_val.m_stable_pvs | e_val.m_stable_pvs);
                assert(!result.is_stable_const());
            }
            else
            {
                // Simplification of the case where both then and else are symbolic
                assert(left.is_stable_const() && !right.is_stable_const());
                result.m_stable_pvs = cond.m_stable_pvs & right.m_stable_pvs;
                assert(!result.is_stable_const());
            }
        }
    }

    result.sanity();

    return result;
}

template<verif_mode_t mode>
bool operator<(const Value<mode>& a, const Value<mode>& b)
{
    a.sanity();
    b.sanity();
    if (a.m_glitch_pvs.get() != b.m_glitch_pvs.get())
        { return a.m_glitch_pvs.get() < b.m_glitch_pvs.get(); }
    if (a.m_stable_pvs.get() != b.m_stable_pvs.get())
        { return a.m_stable_pvs.get() < b.m_stable_pvs.get(); }
    if (a.m_const_stability != b.m_const_stability)
        { return a.m_const_stability < b.m_const_stability; }
    if (a.m_const_value != b.m_const_value)
        { return a.m_const_value < b.m_const_value; }
    return false;
}

template<verif_mode_t mode>
bool operator==(const Value<mode>& a, const Value<mode>& b)
{
    a.sanity();
    b.sanity();
    if (a.m_glitch_pvs.get() != b.m_glitch_pvs.get()) return false;
    if (a.m_stable_pvs.get() != b.m_stable_pvs.get()) return false;
    if (a.m_const_stability != b.m_const_stability) return false;
    if (a.m_const_value != b.m_const_value) return false;
    return true;
}

template<verif_mode_t mode>
bool operator!=(const Value<mode>& a, const Value<mode>& b)
{
    return !(a == b);
}

template<verif_mode_t mode>
std::ostream& operator<<(std::ostream& out, const Value<mode>& val)
{
    bool display_const = false;
    switch(Value<mode>::s_display)
    {
        case value_display_t::DISPLAY_STABLE:
            display_const = val.is_stable_const();
            break;
        case value_display_t::DISPLAY_GLITCH: // fallthrough
            display_const = has_glitches(mode) && val.is_glitch_const();
    }

    if (display_const)
    { out << val.m_const_value; }
    else switch (Value<mode>::s_display)
    {
        case value_display_t::DISPLAY_STABLE:
            out << *val.m_stable_pvs;
            break;
        case value_display_t::DISPLAY_GLITCH:
            out << *val.m_glitch_pvs;
            break;
        default: assert(false);
    }
    return out;
}

template<verif_mode_t mode>
Value<mode>& Value<mode>::operator=(const ValueView<mode>& other_view)
{
    const Value<mode>& other = other_view.get();
    m_const_value = other.m_const_value;
    m_const_stability = other.m_const_stability;
    m_stable_pvs = other.m_stable_pvs;
    m_glitch_pvs = other.m_glitch_pvs;
    return *this;
}

template <verif_mode_t mode>
Value<mode>& Value<mode>::operator=(uint64_t val)
{
    if (((val & 1) != val))
    { std::cout << "Warning: Value assignment overflow" << std::endl; }
    return operator=(Value<mode>((bool)val));
}

template<verif_mode_t mode>
ValueView<mode>& ValueView<mode>::operator=(const Value<mode>& other)
{
    other.sanity();
    m_value.sanity();

    Value<mode> result(false);

    // Perform the update for the stable computation
    result.m_const_value = other.m_const_value;
    result.m_stable_pvs = other.m_stable_pvs;

    if constexpr (has_glitches(mode))
    {
        // It is only stable if both parents are stable and equivalent
        result.m_const_stability = (m_value.is_signal_stable() && other.is_signal_stable() &&
                                    (m_value.m_const_value == other.m_const_value));

        // Perform the case distinction for the glitch computation
        if (m_value.is_glitch_const() && other.is_glitch_const())
        {
            // It is a constant in both cases, should be covered already
            assert(result.m_const_value == other.m_const_value);
            assert(result.is_glitch_const());
        }
        else if (!m_value.is_glitch_const() && !other.is_glitch_const())
        {
            // It is symbolic in both cases, result is symbolic
            result.m_glitch_pvs = m_value.m_glitch_pvs | other.m_glitch_pvs;
            assert(!result.is_glitch_const());
        }
        else if (m_value.is_stable_const() && !other.is_glitch_const())
        {
            // The new glitchy value is the biased overwrite value
            result.m_glitch_pvs = +other.m_glitch_pvs;
            assert(!result.is_glitch_const());
        }
        else
        {
            assert(!m_value.is_glitch_const() && other.is_glitch_const());
            // The new glitchy value is the biased previous value
            result.m_glitch_pvs = +m_value.m_glitch_pvs;
            assert(!result.is_glitch_const());
        }
    }
    result.sanity();
    m_value = result;
    return *this;
}

template <verif_mode_t mode>
ValueView<mode>& ValueView<mode>::operator=(uint64_t val)
{
    if (((val & 1) != val))
    { std::cout << "Warning: ValueView assignment overflow" << std::endl; }
    return operator=(Value<mode>((bool)val));
}

template <verif_mode_t mode>
ValueViewVector<mode>::ValueViewVector(const ValueViewVector& other, const size_t up, const size_t down)
{
    if (up >= down)
    {
        const auto begin = other.m_views.cbegin() + down;
        const auto end = other.m_views.cbegin() + up + 1;
        m_views = std::vector<ValueT>(begin, end);
    }
    else
    {
        const auto begin = other.m_views.crbegin() + (other.m_views.size() - down - 1);
        const auto end = other.m_views.crbegin() + (other.m_views.size() - up);
        m_views = std::vector<ValueT>(begin, end);
    }
    assert(m_views.size() == (std::abs((int64_t)up - (int64_t)down) + 1L));
}

template <verif_mode_t mode>
ValueViewVector<mode>& ValueViewVector<mode>::operator=(uint64_t vals)
{
    if (size() < 64 && (vals >> size()) != 0)
    { std::cout << "Warning: ValueViewVector assignment overflow " << size() << std::endl; }

    for (size_t i = 0; i < size(); i++)
    { m_views.at(i) = Value<mode>((bool)((vals >> i) & 1)); }

    return *this;
}

template <verif_mode_t mode>
ValueViewVector<mode>& ValueViewVector<mode>::operator=(const ValueVector<mode>& vals)
{
    if (vals.size() > size())
    { std::cout << "Warning: ValueViewVector assignment overflow " << vals.size() << std::endl; }

    for (size_t i = 0; i < size() && i < vals.size(); i++)
    { m_views.at(i) = vals[i]; }

    if (vals.size() < size())
    { std::cout << "Warning: Assigning " << size() - vals.size() << "upper ValueViewVector bits to zero" << std::endl; }

    for (size_t i = vals.size(); i < size(); i++)
    { m_views.at(i) = Value<mode>(false); }

    return *this;
}

template <verif_mode_t mode>
bool ValueViewVector<mode>::is_const()
{
    for (uint32_t i = 0; i < size(); i++)
    {
        if (!m_views[i].get().is_stable_const()) return false;
    }
    return true;
}

template <verif_mode_t mode>
uint64_t ValueViewVector<mode>::as_uint64_t()
{
    if (!is_const())
        { throw std::logic_error(ILLEGAL_VALUE_NOT_CONST); }

    uint64_t res = 0;
    for (uint32_t i = 0; i < size() && i < 64; i++)
        { res |= ((uint64_t)m_views[i].get().stable_val()) << i; }
    return res;
}

template <verif_mode_t mode>
std::ostream& operator<<(std::ostream& out, const ValueViewVector<mode>& view_vector)
{
    for(uint32_t i = 0; i < view_vector.size(); i++)
        { out << view_vector.m_views[i].get() << " "; }
    return out;
}

#endif // VALUE_H