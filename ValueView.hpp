#ifdef VALUEVIEW_H

#include "ValueView.h"

template<verif_mode_t mode>
ValueView<mode>& ValueView<mode>::operator=(const Value<mode>& other)
{
    other.sanity();
    m_value.sanity();

    Value<mode> result(false);

    // Perform the update for the stable computation
    result.m_symbolic_value = other.m_symbolic_value;
    // It is only stable if both parents are stable and equivalent
    result.m_stability = m_value.m_stability && other.m_stability &&
                         (m_value.m_symbolic_value == other.m_symbolic_value);

    if constexpr (has_stable(mode))
    {
        result.m_stable_pvs = other.m_stable_pvs;
    }

    if constexpr (has_glitches(mode))
    {

        // Perform the case distinction for the glitch computation
        if (m_value.is_glitch_const() && other.is_glitch_const())
        {
            // It is a constant in both cases, should be covered already
            assert(result.m_symbolic_value == other.m_symbolic_value);
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

/*
template <verif_mode_t mode>
ValueView<mode>& ValueView<mode>::operator=(uint64_t val)
{
    if (((val & 1) != val))
    { std::cout << "Warning: ValueView assignment overflow" << std::endl; }
    return operator=(Value<mode>((bool)val));
}
*/

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
    { res |= ((uint64_t)m_views[i].get().const_val()) << i; }
    return res;
}

template <verif_mode_t mode>
std::ostream& operator<<(std::ostream& out, const ValueViewVector<mode>& view_vector)
{
    for(uint32_t i = 0; i < view_vector.size(); i++)
    { out << view_vector.m_views[i].get() << " "; }
    return out;
}

#endif