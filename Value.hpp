#ifdef VALUE_H

#include "Cell.h"
#include "Value.h"


template <verif_mode_t mode>
constexpr void Value<mode>::sanity() const
{
    assert((m_stability & 0x01) == m_stability);
    assert(implies(m_stability, is_const(m_symbolic_value.var())));
    assert(implies(has_glitches(mode) && is_glitch_const(), is_stable_const()));
    assert(implies(has_glitches(mode) && !is_glitch_const(), m_stability == false));
    // assert(implies(!has_glitches(mode), m_stability == true));
}

template <verif_mode_t mode>
bool Value<mode>::const_val() const
{
    assert(is_const(m_symbolic_value.var()));
    return m_symbolic_value.var() == ONE;
}

template <verif_mode_t mode>
bool Value<mode>::is_signal_stable() const
{
    if (!is_glitch_const()) return false;
    return m_stability;
}

template <verif_mode_t mode>
const typename Value<mode>::StablePVSPtr& Value<mode>::stable_pvs() const
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
        : m_symbolic_value(false), m_stability(true), m_stable_pvs(), m_glitch_pvs()
{}

template<verif_mode_t M>
Value<M> operator!(const Value<M>& a)
{
    a.sanity();

    Value<M> result = a;
    result.m_symbolic_value = !result.m_symbolic_value;

    result.sanity();
    return result;
}

// TODO: Weird?
template<verif_mode_t mode>
Value<mode> operator+(const Value<mode>& a)
{
    a.sanity();

    Value<mode> result = a;
    if constexpr (has_glitches(mode))
    {
        result.m_glitch_pvs = result.m_stable_pvs;
    }

    result.sanity();
    return result;
}

template<verif_mode_t mode>
PropVarSetPtr Value<mode>::make_and(var_t a_var, PropVarSetPtr a_pvs,
                                    var_t b_var, PropVarSetPtr b_pvs)
{
    const typename Value<mode>::gate_key_t key(a_var, a_pvs, b_var, b_pvs);
    const auto it = Value<mode>::and_cache.find(key);
    if (it != Value<mode>::and_cache.end())
        { return it->second; }
    PropVarSetPtr result_pvs = a_pvs & b_pvs;
    Value<mode>::and_cache.emplace_hint(it, key, result_pvs);
    return result_pvs;
}

template<verif_mode_t mode>
PropVarSetPtr Value<mode>::make_xor(var_t a_var, PropVarSetPtr a_pvs,
                                    var_t b_var, PropVarSetPtr b_pvs)
{
    const typename Value<mode>::gate_key_t key(a_var, a_pvs, b_var, b_pvs);
    const auto it = Value<mode>::xor_cache.find(key);
    if (it != Value<mode>::xor_cache.end())
    { return it->second; }
    PropVarSetPtr result_pvs = a_pvs ^ b_pvs;
    Value<mode>::xor_cache.emplace_hint(it, key, result_pvs);
    return result_pvs;
}

template<verif_mode_t mode>
Value<mode> operator&(const Value<mode>& a, const Value<mode>& b)
{
    a.sanity();
    b.sanity();

    Value<mode> result;
    result.m_symbolic_value = a.m_symbolic_value & b.m_symbolic_value;
    // it is stable if both inputs are stable or one is stable false
    result.m_stability = (a.m_stability && b.m_stability) ||
                         (a.m_stability && a.m_symbolic_value.var() == ZERO) ||
                         (b.m_stability && b.m_symbolic_value.var() == ZERO);
    assert(implies(result.m_stability, is_const(result.m_symbolic_value.var())));

    if constexpr (has_stable(mode))
    { // Perform the case distinction for the stable computation
        const bool order = a.is_stable_const() &&
                (!b.is_stable_const() || a.m_symbolic_value.var() < b.m_symbolic_value.var());
        const Value<mode>& left =  order ? a : b;
        const Value<mode>& right = order ? b : a;

        if (left.is_stable_const() && right.is_stable_const())
        {
            // It is a constant in both cases, result is a constant
            assert(result.is_stable_const());
            assert(is_const(result.m_symbolic_value.var()));
        }
        else if (!left.is_stable_const() && !right.is_stable_const())
        {
            // It is symbolic in both cases, result is symbolic
            result.m_stable_pvs = Value<mode>::make_and(left.m_symbolic_value.var(), left.m_stable_pvs,
                                                        right.m_symbolic_value.var(), right.m_stable_pvs);
            assert(!result.is_stable_const());
            assert(!is_const(result.m_symbolic_value.var()));
        }
        else
        {
            // It is constant in left and symbolic in right
            assert(left.is_stable_const() && !right.is_stable_const());
            if (left.m_symbolic_value.var() == ZERO)
            {
                // If the constant value is false, then result is constant
                assert(result.is_stable_const());
                assert(result.m_symbolic_value.var() == ZERO);
            }
            else
            {
                assert(left.m_symbolic_value.var() == ONE);
                // If the constant value is true, then result is symbolic
                result.m_stable_pvs = right.m_stable_pvs;
                assert(!result.is_stable_const());
            }
        }
    }

    if constexpr (has_glitches(mode))
    { // Perform the case distinction for the glitch computation
        const bool order = a.is_glitch_const() &&
                           (!b.is_glitch_const() || a.m_symbolic_value.var() < b.m_symbolic_value.var());
        const Value<mode>& left =  order ? a : b;
        const Value<mode>& right = order ? b : a;

        if (left.is_glitch_const() && right.is_glitch_const())
        {
            // It is a constant in both cases, result is a constant
            assert(result.is_glitch_const());
            assert(is_const(result.m_symbolic_value.var()));
        }
        else if (!left.is_glitch_const() && !right.is_glitch_const())
        {
            // It is symbolic in both cases, result is symbolic
            result.m_glitch_pvs = Value<mode>::make_and(left.m_symbolic_value.var(), left.m_glitch_pvs,
                                                        right.m_symbolic_value.var(), right.m_glitch_pvs);
            assert(!result.is_glitch_const());
        }
        else
        {
            // It is constant in left and symbolic in right
            assert(left.is_glitch_const() && !right.is_glitch_const());
            assert(left.is_signal_stable() == left.m_stability);
            if (left.m_stability)
            {
                // The constant is stable, so we can simplify
                if(left.m_symbolic_value.var() == ZERO)
                {
                    // If the constant value is false, then result is constant
                    assert(result.is_glitch_const());
                    assert(result.m_stability == true);
                }
                else
                {
                    assert(left.m_symbolic_value.var() == ONE);
                    // If the constant value is true, then result is symbolic
                    result.m_glitch_pvs = right.m_glitch_pvs;
                    assert(!result.is_glitch_const());
                    assert(result.m_stability == false);
                }
            }
            else
            {
                // The constant is not stable, and could make the result false
                // In other words, the result is the biased symbolic argument
                result.m_glitch_pvs = +right.m_glitch_pvs;
                assert(!result.is_glitch_const());
                assert(result.m_stability == false);
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

    Value<mode> result;
    result.m_symbolic_value = a.m_symbolic_value ^ b.m_symbolic_value;
    // It is only stable if both parents are stable (and constants)
    result.m_stability = a.m_stability && b.m_stability;
    assert(implies(result.m_stability, is_const(result.m_symbolic_value.var())));

    if constexpr (has_stable(mode))
    { // Perform the case distinction for the stable computation
        const bool order = a.is_stable_const() &&
                           (!b.is_stable_const() || a.m_symbolic_value.var() < b.m_symbolic_value.var());
        const Value<mode>& left =  order ? a : b;
        const Value<mode>& right = order ? b : a;

        if (is_const(result.m_symbolic_value.var()))
        {
            assert(result.is_stable_const());
        }
        else if (left.is_stable_const() && right.is_stable_const())
        {
            // It is a constant in both cases, result is a constant
            assert(result.is_stable_const());
            assert(is_const(result.m_symbolic_value.var()));
        }
        else if (!left.is_stable_const() && !right.is_stable_const())
        {
            // It is symbolic in both cases, result is symbolic
            result.m_stable_pvs = Value<mode>::make_xor(left.m_symbolic_value.var(), left.m_stable_pvs,
                                                        right.m_symbolic_value.var(), right.m_stable_pvs);
            assert(!result.is_stable_const());
            /// TODO: investigate the assertion a bit more
            assert(!is_const(result.m_symbolic_value.var()));
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

    if constexpr (has_glitches(mode))
    { // Perform the case distinction for the glitch computation
        const bool order = a.is_glitch_const() &&
                           (!b.is_glitch_const() || a.m_symbolic_value.var() < b.m_symbolic_value.var());
        const Value<mode>& left = order ? a : b;
        const Value<mode>& right = order ? b : a;

        if (left.is_glitch_const() && right.is_glitch_const())
        {
            // It is a constant in both cases, result is a constant
            assert(result.is_glitch_const());
            assert(is_const(result.m_symbolic_value.var()));
        } else if (!left.is_glitch_const() && !right.is_glitch_const())
        {
            // It is symbolic in both cases, result is symbolic
            result.m_glitch_pvs = Value<mode>::make_xor(left.m_symbolic_value.var(), left.m_glitch_pvs,
                                                        right.m_symbolic_value.var(), right.m_glitch_pvs);
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

    result.sanity();

    return result;
}

template<verif_mode_t mode>
Value<mode> operator|(const Value<mode>& a, const Value<mode>& b)
{
    return !(!a & !b);
}

//TODO: remove extern int MUX_COUNT;

template<verif_mode_t mode>
Value<mode> mux(const Value<mode>& cond, const Value<mode>& t_val, const Value<mode>& e_val)
{
    cond.sanity();
    t_val.sanity();
    e_val.sanity();

    Value<mode> result;

    result.m_symbolic_value = mux(cond.m_symbolic_value, t_val.m_symbolic_value, e_val.m_symbolic_value);
    // complicated logic to determine if the signal is stable
    result.m_stability = (cond.m_stability && ((cond.m_symbolic_value.var() == ONE && t_val.m_stability) || (cond.m_symbolic_value.var() == ZERO && e_val.m_stability))) ||
            (t_val.m_stability && e_val.m_stability && t_val.m_symbolic_value.var() == e_val.m_symbolic_value.var());
    assert(implies(result.m_stability, is_const(result.m_symbolic_value.var())));

    if constexpr (has_stable(mode))
    { // Perform the case distinction for the stable computation
        if (is_const(result.m_symbolic_value.var()))
        {
            assert(result.is_stable_const());
        }
        else if (cond.is_stable_const())
        {
            // The selection signal is stable, so just mux the inputs
            if (cond.m_symbolic_value.var() == ONE)
            { // Take everything from the then value
                result.m_stable_pvs = t_val.m_stable_pvs;
            } else
            { // Take everything from the else value
                result.m_stable_pvs = e_val.m_stable_pvs;
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
                if (left.m_symbolic_value == right.m_symbolic_value)
                {
                    // We are selecting symbolically between the same constant
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
                //TODO: remove MUX_COUNT += 1;
                std::cout << "here! 1" << std::endl;
                assert(!result.is_stable_const());
            }
            else
            {
                // Simplification of the case where one is symbolic
                assert(left.is_stable_const() && !right.is_stable_const());
                result.m_stable_pvs = cond.m_stable_pvs & right.m_stable_pvs;
                assert(!result.is_stable_const());
            }
        }
    }

    if constexpr (has_glitches(mode))
    { // Perform the case distinction for the glitch computation
        if (cond.is_glitch_const())
        {
            // The selection signal is a constant in the glitchy domain
            if (cond.m_stability)
            {
                // The selection signal is stable, so just mux the inputs
                if (cond.m_symbolic_value.var() == ONE)
                {
                    // Take everything from the then value
                    result.m_glitch_pvs = t_val.m_glitch_pvs;
                } else
                {
                    // Take everything from the else value
                    result.m_glitch_pvs = e_val.m_glitch_pvs;
                }
            }
            else
            {
                // The selection signal is unstable, result is both inputs simultaneously
                const bool order = t_val.is_glitch_const();
                const Value<mode>& left = order ? t_val : e_val;
                const Value<mode>& right = order ? e_val : t_val;

                if (left.is_glitch_const() && right.is_glitch_const())
                {
                    assert(result.is_glitch_const());
                } else if (!left.is_glitch_const() && !right.is_glitch_const())
                {
                    result.m_glitch_pvs = left.m_glitch_pvs | right.m_glitch_pvs;
                    assert(!result.is_glitch_const());
                    assert(result.m_stability == false);
                } else
                {
                    assert(left.is_glitch_const() && !right.is_glitch_const());
                    result.m_glitch_pvs = +right.m_glitch_pvs;
                    assert(!result.is_glitch_const());
                    assert(result.m_stability == false);
                }
            }
        } else
        {
            // The selection signal is not constant, mix it with the inputs
            const bool order = t_val.is_glitch_const();
            const Value<mode>& left = order ? t_val : e_val;
            const Value<mode>& right = order ? e_val : t_val;

            if (left.is_glitch_const() && right.is_glitch_const())
            {
                // Both then and else are constants
                if (left.m_symbolic_value == right.m_symbolic_value)
                {
                    // We are selecting symbolically between the same constant
                    assert(result.is_glitch_const());
                    assert(result.m_stability == left.m_stability && right.m_stability);
                }
                else
                {
                    // The result is influenced by the symbolic selection
                    result.m_glitch_pvs = cond.m_glitch_pvs;
                    assert(!result.is_glitch_const());
                    assert(result.m_stability == false);
                }
            }
            else if (!left.is_glitch_const() && !right.is_glitch_const())
            {
                // Both then and else are symbolic, create multiplexer pvs
                result.m_glitch_pvs = +cond.m_glitch_pvs ^ (t_val.m_glitch_pvs | e_val.m_glitch_pvs);
                //TODO: remove MUX_COUNT += 1;
                std::cout << "here! 2" << std::endl;
                assert(!result.is_glitch_const());
                assert(result.m_stability == false);
            }
            else
            {
                // Simplification of the case where one is symbolic
                assert(left.is_glitch_const() && !right.is_glitch_const());
                result.m_glitch_pvs = cond.m_glitch_pvs & right.m_glitch_pvs;
                assert(!result.is_glitch_const());
                assert(result.m_stability == false);
            }
        }
    }

    result.sanity();

    return result;

    // alternative dumb implementation:
    // return (cond & t_val) ^ (!cond & e_val);
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
    if (a.m_stability != b.m_stability)
        { return a.m_stability < b.m_stability; }
    if (a.m_const_value != b.m_const_value)
        { return a.m_const_value < b.m_const_value; }
    return false;
}

template<verif_mode_t mode>
bool operator==(const Value<mode>& a, const Value<mode>& b)
{
    a.sanity();
    b.sanity();
    if constexpr (has_glitches(mode))
        if (a.m_glitch_pvs.get() != b.m_glitch_pvs.get()) return false;
    if constexpr (has_stable(mode))
        if (a.m_stable_pvs.get() != b.m_stable_pvs.get()) return false;
    if (a.m_stability != b.m_stability) return false;
    if (a.m_symbolic_value != b.m_symbolic_value) return false;
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
    {
        assert(is_const(val.m_symbolic_value.var()));
        out << (val.m_symbolic_value.var() == ONE);
    }
    else switch (Value<mode>::s_display)
    {
        case value_display_t::DISPLAY_STABLE:
            if constexpr (has_stable(mode))
            { out << *val.m_stable_pvs; }
            break;
        case value_display_t::DISPLAY_GLITCH:
            if constexpr (has_glitches(mode))
            { out << *val.m_glitch_pvs; }
            break;
        default: assert(false);
    }
    return out;
}

template<verif_mode_t mode>
Value<mode>& Value<mode>::operator=(const ValueView<mode>& other_view)
{
    const Value<mode>& other = other_view.get();
    m_symbolic_value = other.m_symbolic_value;
    m_stability = other.m_stability;
    m_stable_pvs = other.m_stable_pvs;
    m_glitch_pvs = other.m_glitch_pvs;
    return *this;
}


template <verif_mode_t mode>
Value<mode>& Value<mode>::operator=(bool val)
{
    return operator=(Value<mode>((bool)val));
}

#endif // VALUE_H