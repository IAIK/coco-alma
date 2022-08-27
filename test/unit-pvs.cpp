#include "test_util.h"
#include "SatSolver.h"
#include "PropVarSet.h"

void test_constructor()
{
    SatSolver solver;
    PropVarSet a(&solver, 0);
    assert(a.size() == 1);
    assert(a[0] == ONE);
    assert(a[1] == ZERO);
}

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


int main(int argc, char* argv[])
{
    if (argc != 2) return 1;
    const std::string test_name = argv[1];
    if (test_name == STR(test_constructor))
        test_constructor();
    else if (test_name == STR(test_ptr_xor))
        test_ptr_xor();
    else if (test_name == STR(test_ptr_plus_bias))
        test_ptr_plus_bias();
    else if (test_name == STR(test_ptr_and))
        test_ptr_and();
    else
        return 1;
    return 0;
}