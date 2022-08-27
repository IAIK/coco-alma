#include "test_util.h"
#include "SatSolver.h"

void test_new_var()
{
    SatSolver solver;
    var_t a = solver.new_var();
    assert(is_legal(a));
    assert(a > 0);
    var_t b = solver.new_var();
    assert(is_legal(b));
    assert(b > 0);
    assert(a != b);
}

void test_new_vars()
{
    SatSolver solver;
    var_t a = solver.new_vars(20);
    assert(is_legal(a));
    var_t b = solver.new_var();
    assert(is_legal(b));
    assert(a + 20 == b);
}

void test_make_xor()
{
    SatSolver solver;
    var_t a = solver.new_var();
    var_t b = solver.new_var();
    var_t c = solver.make_xor(a, b);
    for (int i = 0; i < 4; i++)
    {
        SatSolver::state_t res = solver.check();
        assert(res == SatSolver::STATE_SAT);
        bool val_a = solver.value(a);
        bool val_b = solver.value(b);
        bool val_c = solver.value(c);
        std::cout << "a: " << val_a << " b: " << val_b << " c: " << val_c << std::endl;
        assert((val_a ^ val_b) == val_c);
        var_t lit_a = val_a ? a : -a;
        var_t lit_b = val_b ? b : -b;
        var_t lit_c = val_c ? c : -c;
        solver.add_clause(-lit_a, -lit_b, -lit_c);
    }
    SatSolver::state_t res = solver.check();
    assert(res == SatSolver::STATE_UNSAT);
}

void test_make_and()
{
    SatSolver solver;
    var_t a = solver.new_var();
    var_t b = solver.new_var();
    var_t c = solver.make_and(a, b);
    for (int i = 0; i < 4; i++)
    {
        SatSolver::state_t res = solver.check();
        assert(res == SatSolver::STATE_SAT);
        bool val_a = solver.value(a);
        bool val_b = solver.value(b);
        bool val_c = solver.value(c);
        std::cout << "a: " << val_a << " b: " << val_b << " c: " << val_c << std::endl;
        assert((val_a & val_b) == val_c);
        var_t lit_a = val_a ? a : -a;
        var_t lit_b = val_b ? b : -b;
        var_t lit_c = val_c ? c : -c;
        solver.add_clause(-lit_a, -lit_b, -lit_c);
    }
    SatSolver::state_t res = solver.check();
    assert(res == SatSolver::STATE_UNSAT);
}

void test_add_clause()
{
    SatSolver solver;
    var_t vars[5];
    for (var_t& v : vars)
        v = solver.new_var();
    var_t out = solver.new_var();
    for (const var_t v : vars)
        solver.add_clause(v, -out);
    solver.add_clause(-vars[0], -vars[1], -vars[2],
                      -vars[3], -vars[4], out);
    bool vals[5];
    var_t lits[5];
    for (int i = 0; i < 32; i++)
    {
        SatSolver::state_t res = solver.check();
        assert(res == SatSolver::STATE_SAT);
        bool total = true;
        for (int j = 0; j < 5; j++)
        {
            vals[j] = solver.value(vars[j]);
            total &= vals[j];
            std::cout << "v" << j << ": " << vals[j] << " ";
        }
        std::cout << std::endl;
        bool out_val = solver.value(out);
        assert(total == out_val);
        for (int j = 0; j < 5; j++)
            lits[j] = vals[j] ? vars[j] : -vars[j];

        var_t out_lit = out_val ? out : -out;
        solver.add_clause(-lits[0], -lits[1], -lits[2],
                          -lits[3], -lits[4], -out_lit);
    }

    SatSolver::state_t res = solver.check();
    assert(res == SatSolver::STATE_UNSAT);
}

void test_make_xor_simplify()
{
    SatSolver solver;

    // Test single variable simplifications
    var_t a = solver.new_var();
    var_t a_0 = solver.make_xor(a, ZERO);
    assert(a_0 == a);
    var_t a_1 = solver.make_xor(a, ONE);
    assert(a_1 == -a);
    var_t a_a = solver.make_xor(a, a);
    assert(a_a == ZERO);
    var_t a_na = solver.make_xor(a, -a);
    assert(a_na == ONE);

    #ifdef OPT_EXPR_CACHING
    // Test not simplifications
    var_t b = solver.new_var();
    var_t c = solver.make_xor(a, b);
    assert(c != a && c != b);
    var_t a_nb = solver.make_xor(a, -b);
    assert(a_nb == -c);
    var_t na_b = solver.make_xor(-a, b);
    assert(na_b == -c);
    var_t na_nb = solver.make_xor(-a, -b);
    assert(na_nb == c);

    // Test linearity left
    var_t a_c = solver.make_xor(a, c);
    assert(a_c == b);
    var_t a_nc = solver.make_xor(a, -c);
    assert(a_nc == -b);
    var_t na_c = solver.make_xor(-a, c);
    assert(na_c == -b);
    var_t na_nc = solver.make_xor(-a, -c);
    assert(na_nc == b);

    // Test linearity right
    var_t b_c = solver.make_xor(b, c);
    assert(b_c == a);
    var_t b_nc = solver.make_xor(b, -c);
    assert(b_nc == -a);
    var_t nb_c = solver.make_xor(-b, c);
    assert(nb_c == -a);
    var_t nb_nc = solver.make_xor(-b, -c);
    assert(nb_nc == a);

    // Test re-ordering
    var_t c_a = solver.make_xor(c, a);
    assert(c_a == a_c);
    var_t nc_a = solver.make_xor(-c, a);
    assert(nc_a == a_nc);
    var_t c_na = solver.make_xor(c, -a);
    assert(c_na == na_c);
    var_t nc_na = solver.make_xor(-c, -a);
    assert(nc_na == na_nc);
    #endif
}

void test_make_and_simplify()
{
    SatSolver solver;

    // Test single variable simplifications
    var_t a = solver.new_var();
    var_t a_0 = solver.make_and(a, ZERO);
    assert(a_0 == ZERO);
    var_t a_1 = solver.make_and(a, ONE);
    assert(a_1 == a);
    var_t a_a = solver.make_and(a, a);
    assert(a_a == a);
    var_t a_na = solver.make_and(a, -a);
    assert(a_na == ZERO);

    #ifdef OPT_EXPR_CACHING
    // Test wrong caching
    var_t b = solver.new_var();
    var_t a_b = solver.make_and(a, b);
    assert(a_b > 0);
    assert(a_b != a);
    assert(a_b != b);
    var_t a_nb = solver.make_and(a, -b);
    assert(a_nb > 0);
    assert(a_nb != a);
    assert(a_nb != b);
    assert(a_nb != a_b);
    var_t na_b = solver.make_and(-a, b);
    assert(na_b > 0);
    assert(na_b != a);
    assert(na_b != b);
    assert(na_b != a_b);
    assert(na_b != a_nb);
    var_t na_nb = solver.make_and(-a, -b);
    assert(na_nb > 0);
    assert(na_nb != a);
    assert(na_nb != b);
    assert(na_nb != a_b);
    assert(na_nb != a_nb);
    assert(na_nb != na_b);

    // Test wring order
    var_t b_a = solver.make_and(b, a);
    assert(b_a == a_b);
    var_t nb_a = solver.make_and(-b, a);
    assert(nb_a == a_nb);
    var_t b_na = solver.make_and(b, -a);
    assert(b_na == na_b);
    var_t nb_na = solver.make_and(-b, -a);
    assert(nb_na == na_nb);
    #endif
}

void test_add_clause_simplify()
{
    // Check that ZERO literal is eliminated
    {
        SatSolver solver;
        var_t a  = solver.new_var();
        solver.add_clause(a, ZERO);

        SatSolver::state_t res = solver.check();
        assert(res == SatSolver::STATE_SAT);
        bool a_val = solver.value(a);
        assert(a_val == true);
        solver.add_clause(-a);
        res = solver.check();
        assert(res == SatSolver::STATE_UNSAT);
    }
    // Check that clause with ONE is eliminated
    {
        SatSolver solver;
        var_t a = solver.new_var();
        var_t b = solver.new_var();
        solver.add_clause(a, ONE, -b);
        SatSolver::state_t res;
        for (int i = 0; i < 4; i++)
        {
            res = solver.check();
            assert(res == SatSolver::STATE_SAT);
            bool a_val = solver.value(a);
            bool b_val = solver.value(b);
            var_t a_lit = a_val ? a : -a;
            var_t b_lit = b_val ? b : -b;
            solver.add_clause(-a_lit, -b_lit);
        }
        res = solver.check();
        assert(res == SatSolver::STATE_UNSAT);
    }
}

void test_add_clause_illegal()
{
    // Check that 0 is detected
    SatSolver solver;
    var_t a = solver.new_var();
    constexpr var_t ILLEGAL_0 = 0;
    constexpr var_t ILLEGAL_1 = INT32_MIN;

    TEST_CRASH(solver.add_clause(ILLEGAL_0));
    TEST_CRASH(solver.add_clause(ILLEGAL_1));

    TEST_CRASH(solver.add_clause(+a, ILLEGAL_0));
    TEST_CRASH(solver.add_clause(-a, ILLEGAL_0));
    TEST_CRASH(solver.add_clause(ILLEGAL_0, +a));
    TEST_CRASH(solver.add_clause(ILLEGAL_0, -a));
    TEST_CRASH(solver.add_clause(+a, ILLEGAL_1));
    TEST_CRASH(solver.add_clause(-a, ILLEGAL_1));
    TEST_CRASH(solver.add_clause(ILLEGAL_1, +a));
    TEST_CRASH(solver.add_clause(ILLEGAL_1, -a));

}

int main(int argc, char* argv[])
{
    if (argc != 2) return 1;
    const std::string test_name = argv[1];
    if (test_name == STR(test_new_var))
        test_new_var();
    else if (test_name == STR(test_new_vars))
        test_new_vars();
    else if (test_name == STR(test_make_xor))
        test_make_xor();
    else if (test_name == STR(test_make_and))
        test_make_and();
    else if (test_name == STR(test_add_clause))
        test_add_clause();
    else if (test_name == STR(test_make_xor_simplify))
        test_make_xor_simplify();
    else if (test_name == STR(test_make_and_simplify))
        test_make_and_simplify();
    else if (test_name == STR(test_add_clause_simplify))
        test_add_clause_simplify();
    else if (test_name == STR(test_add_clause_illegal))
        test_add_clause_illegal();
    else
        return 1;
    return 0;
}