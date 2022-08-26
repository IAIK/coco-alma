#include <iostream>
#include <cassert>

#include "SatSolver.h"
#include "PropVarSet.h"

int main()
{
    SatSolver solver;

    PropVarSetPtr p_a = std::make_shared<PropVarSet>(&solver, 0);
    PropVarSetPtr p_b = std::make_shared<PropVarSet>(&solver, 1);
    PropVarSetPtr p_c = +p_a ^ +p_b;
    PropVarSetPtr p_d = std::make_shared<PropVarSet>(&solver, 2);
    PropVarSetPtr p_e = p_c ^ p_d;
    PropVarSetPtr p_f = p_a & p_e;

    std::cout << "a: " << *p_a << std::endl;
    std::cout << "b: " << *p_b << std::endl;
    std::cout << "c: " << *p_c << std::endl;
    std::cout << "d: " << *p_d << std::endl;
    std::cout << "e: " << *p_e << std::endl;
    std::cout << "f: " << *p_f << std::endl;
    return 0;
}