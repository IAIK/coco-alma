#include "test_util.h"

// #define private public
#include "Value.h"
#include "ValueView.h"
// #undef private

#include "SatSolver.h"
#include "PropVarSet.h"

using ValueS = Value<verif_mode_t::MODE_STABLE>;
using ValueG = Value<verif_mode_t::MODE_GLITCH>;

void test_constructor()
{
    SatSolver solver;
    PropVarSetPtr p_a = std::make_shared<PropVarSet>(&solver, 0);
    PropVarSetPtr p_b = std::make_shared<PropVarSet>(&solver, 1);
    PropVarSetPtr p_c = p_a & p_b;
    Symbol s = create_symbol();

    { // Check the normal constructor from boolean
        ValueS val(false);
        val.sanity();
        assert(val.is_stable_const());
        assert(val.const_val() == false);
        assert(val.is_signal_stable() == true);
    }

    { // Check the normal constructor from simple pvs
        ValueS val(s, p_a);
        val.sanity();
        assert(!val.is_stable_const());
    }

    { // Check the normal constructor from complex pvs
        ValueS val(s, p_c);
        val.sanity();
        assert(!val.is_stable_const());
    }

    { // Check the normal constructor from boolean
        ValueG val(true);
        val.sanity();
        assert(val.is_stable_const());
        assert(val.is_glitch_const());
        assert(val.const_val() == true);
        assert(val.is_signal_stable() == true);
    }

    { // Check the normal constructor from simple pvs
        ValueG val(s, p_a);
        val.sanity();
        assert(!val.is_stable_const());
        assert(!val.is_glitch_const());
    }

    { // Check the normal constructor from complex pvs
        ValueG val(s, p_c);
        val.sanity();
        assert(!val.is_stable_const());
        assert(!val.is_glitch_const());
    }
}

template <verif_mode_t mode>
void test_xor()
{
    using ValueM = Value<mode>;
    using ViewM = ValueView<mode>;

    SatSolver solver;
    PropVarSetPtr p_a = std::make_shared<PropVarSet>(&solver, 0);
    PropVarSetPtr p_b = std::make_shared<PropVarSet>(&solver, 1);

    ValueM val_f(false);
    ValueM val_t(true);
    Symbol a = create_symbol();
    Symbol b = create_symbol();
    ValueM val_a(a, p_a);
    ValueM val_b(b, p_b);

    // Test combinations of val_t and val_f

    {
        ValueM res = val_f ^ val_f;
        assert(res.is_stable_const());
        assert(res.is_glitch_const());
        assert(res == val_f);
    }

    {
        ValueM res = val_f ^ val_t;
        assert(res.is_stable_const());
        assert(res.is_glitch_const());
        assert(res == val_t);
    }

    {
        ValueM res = val_t ^ val_f;
        assert(res.is_stable_const());
        assert(res.is_glitch_const());
        assert(res == val_t);
    }

    {
        ValueM res = val_t ^ val_t;
        assert(res.is_stable_const());
        assert(res.is_glitch_const());
        assert(res == val_f);
    }

    // Create unstable signal variants

    ValueM unstable_f(false);
    ValueM unstable_t(false);

    {
        ValueM copy(val_t);
        assert(copy == val_t);
        ViewM view(copy);
        view = val_f;
        unstable_f = view.get();
    }

    {
        ValueM copy(val_f);
        assert(copy == val_f);
        ViewM view(copy);
        view = val_t;
        unstable_t = view.get();
    }

    assert(unstable_f.is_stable_const());
    assert(unstable_t.is_stable_const());

    assert(unstable_f.is_glitch_const());
    assert(unstable_t.is_glitch_const());

    assert(unstable_f.const_val() == val_f.const_val());
    assert(unstable_t.const_val() == val_t.const_val());
    assert(unstable_f.is_signal_stable() == false);
    assert(unstable_t.is_signal_stable() == false);

    // Test unstable_t vs stable values

    {
        ValueM res = unstable_t ^ val_f;
        assert(res.is_stable_const());
        assert(res.is_glitch_const());
        assert(res == unstable_t);
    }

    {
        ValueM res = val_f ^ unstable_t;
        assert(res.is_stable_const());
        assert(res.is_glitch_const());
        assert(res == unstable_t);
    }

    {
        ValueM res = unstable_t ^ val_t;
        assert(res.is_stable_const());
        assert(res.is_glitch_const());
        assert(res == unstable_f);
    }

    {
        ValueM res = val_t ^ unstable_t;
        assert(res.is_stable_const());
        assert(res.is_glitch_const());
        assert(res == unstable_f);
    }

    // Test unstable_f vs stable values

    {
        ValueM res = unstable_f ^ val_f;
        assert(res.is_stable_const());
        assert(res.is_glitch_const());
        assert(res == unstable_f);
    }

    {
        ValueM res = val_f ^ unstable_f;
        assert(res.is_stable_const());
        assert(res.is_glitch_const());
        assert(res == unstable_f);
    }

    {
        ValueM res = unstable_f ^ val_t;
        assert(res.is_stable_const());
        assert(res.is_glitch_const());
        assert(res == unstable_t);
    }

    {
        ValueM res = val_t ^ unstable_f;
        assert(res.is_stable_const());
        assert(res.is_glitch_const());
        assert(res == unstable_t);
    }

    // Test combinations of unstable_t and unstable_f

    {
        ValueM res = unstable_f ^ unstable_f;
        assert(res.is_stable_const());
        assert(res.is_glitch_const());
        assert(res == unstable_f);
    }

    {
        ValueM res = unstable_t ^ unstable_f;
        assert(res.is_stable_const());
        assert(res.is_glitch_const());
        assert(res == unstable_t);
    }

    {
        ValueM res = unstable_f ^ unstable_t;
        assert(res.is_stable_const());
        assert(res.is_glitch_const());
        assert(res == unstable_t);
    }

    {
        ValueM res = unstable_t ^ unstable_t;
        assert(res.is_stable_const());
        assert(res.is_glitch_const());
        assert(res == unstable_f);
    }

}

inline void test_stable_xor()
{ return test_xor<verif_mode_t::MODE_STABLE>(); }

inline void test_glitch_xor()
{ return test_xor<verif_mode_t::MODE_GLITCH>(); }

/*
void test_ptr_xor()
{
    SatSolver solver;
    PropVarSetPtr a = std::make_shared<PropVarSet>(&solver, 0);

    // Test if the same index cancels
    {
        PropVarSetPtr b = std::make_shared<PropVarSet>(&solver, 0);
        PropVarSetPtr c = a ^ b;
        assert(c->size() == 0);
    }

    // Test if different indexes remain
    {
        PropVarSetPtr b = std::make_shared<PropVarSet>(&solver, 1);
        PropVarSetPtr c = a ^ b;
        assert(c->size() == 2);
        assert((*c)[0] == (*a)[0]);
        assert((*c)[1] == (*b)[1]);
    }
}

void test_ptr_plus_bias()
{
    SatSolver solver;
    PropVarSetPtr a = std::make_shared<PropVarSet>(&solver, 0);

    // Test with overloaded operator for biasing
    {
        PropVarSetPtr a_biased= +a;
        var_t x = (*a_biased)[0];
        assert(x != ONE && x != ZERO);
        assert(is_legal(x));
    }

    // Test biasing with function, extracting biasing solver var
    {
        var_t biasing_var;
        PropVarSetPtr a_biased = bias(a, &biasing_var);
        var_t x = (*a_biased)[0];
        assert(x == biasing_var);
    }

    #ifdef OPT_FRESH_BIASED
    // Test double biasing
    {
        PropVarSetPtr a_biased_1 = +a;
        PropVarSetPtr a_biased_2 = +a_biased_1;

        var_t x = (*a_biased_1)[0];
        var_t y = (*a_biased_2)[0];
        assert(x == y);
        assert(x != ONE && x != ZERO);
        assert(is_legal(x));
    }
    #endif
}

void test_ptr_and()
{
    SatSolver solver;
    PropVarSetPtr a = std::make_shared<PropVarSet>(&solver, 0);

    // Test with overloaded operator for biasing
    {
        PropVarSetPtr a_and = a & a;
        assert(a_and->size() == 1);
        var_t x = (*a_and)[0];
        assert(x != ONE && x != ZERO);
        assert(is_legal(x));
        #ifdef OPT_FRESH_BIASED
        PropVarSetPtr a_bias = +a_and;
        var_t y = (*a_and)[0];
        assert(x == y);
        #endif
    }
}
*/

int main(int argc, char* argv[])
{
    if (argc != 2) return 1;
    const std::string test_name = argv[1];
    if (test_name == STR(test_constructor))
        test_constructor();
    else if (test_name == STR(test_stable_xor))
        test_stable_xor();
    else if (test_name == STR(test_glitch_xor))
        test_glitch_xor();
    else
        return 1;
    return 0;
}