#include <iostream>
#include <cassert>
#include <iostream>

#include "SatSolver.h"

int main() {
    SatSolver solver;

    var_t a = solver.new_var();
    var_t b = solver.new_var();
    var_t c = solver.make_and(a, b);
    var_t d = solver.make_xor(a, b);
    var_t e = solver.make_xor(-a, b);
    var_t f = solver.make_xor(a, -b);
    var_t g = solver.make_xor(-a, -b);
    var_t h = solver.make_xor(-a, d);



    SatSolver::state_t res = solver.check();
    assert(res == SatSolver::STATE_SAT);

    std::cout << "a: " << a << " " << solver.value(a) << std::endl;
    std::cout << "b: " << b << " " << solver.value(b) << std::endl;
    std::cout << "c: " << c << " " << solver.value(c) << std::endl;
    std::cout << "d: " << d << " " << solver.value(d) << std::endl;
    std::cout << "e: " << e << " " << solver.value(e) << std::endl;
    std::cout << "f: " << f << " " << solver.value(f) << std::endl;
    std::cout << "g: " << g << " " << solver.value(g) << std::endl;
    std::cout << "h: " << h << " " << solver.value(h) << std::endl;

    return 0;
}
