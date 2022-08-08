#include "Vcircuit.h"
#include "testbench_dom_and.h"
#include <iostream>

#define dom_and_1storder 0
#define dom_and_2ndorder 2
#define dom_and_3rdorder 3
#define isw_and_1storder 4
#define ti_toffoli 5


//This should be replaced by resp. define
#define TC TC_NAME



int main(int argc, char **argv) 
{
	Verilated::commandArgs(argc, argv);
	Testbench<Vcircuit> *tb = new Testbench<Vcircuit>();
    tb->opentrace("tmp.vcd");
    
    tb->reset();
    
    
    tb->tick();
    tb->tick();
    tb->tick();
    tb->tick();
    tb->tick();
    tb->tick();

    int X, Y, Q;
    X = rand() & 0xFF;
    Y = rand() & 0xFF;
    Q = X & Y;


    //Testcase: for functional purposes only!
    #if TC == dom_and_1storder
    int X0, X1, Y0, Y1, Z;
    X0 = rand()  & 0xFF;
    X1 = X^X0;
    Y0 = rand()  & 0xFF;
    Y1 = Y^Y0;

    tb->m_core->X0_i = X0;
    tb->m_core->X1_i = X1;
    tb->m_core->Y0_i = Y0;
    tb->m_core->Y1_i = Y1;
    tb->m_core->Z_i = rand() & 0xFF;
    #elif TC == dom_and_2ndorder 
    int X0, X1, X2, Y0, Y1, Y2, Z0, Z1, Z2;
    X0 = rand()  & 0xFF;
    X2 = rand()  & 0xFF;
    X1 = X^X0^X2;
    Y0 = rand()  & 0xFF;
    Y2 = rand()  & 0xFF;
    Y1 = Y^Y0^Y2;

    tb->m_core->X0_i = X0;
    tb->m_core->X1_i = X1;
    tb->m_core->X2_i = X2;
    tb->m_core->Y0_i = Y0;
    tb->m_core->Y1_i = Y1;
    tb->m_core->Y2_i = Y2;
    tb->m_core->Z0_i = rand() & 0xFF;
    tb->m_core->Z1_i = rand() & 0xFF;
    tb->m_core->Z2_i = rand() & 0xFF;
    #elif TC == dom_and_3rdorder
    int X0, X1, X2, X3, Y0, Y1, Y2, Y3, Z0, Z1, Z2, Z3, Z4, Z5;
    X0 = rand()  & 0xFF;
    X2 = rand()  & 0xFF;
    X3 = rand()  & 0xFF;
    X1 = X^X0^X2^X3;
    Y0 = rand()  & 0xFF;
    Y2 = rand()  & 0xFF;
    Y3 = rand()  & 0xFF;
    Y1 = Y^Y0^Y2^Y3;

    tb->m_core->X0_i = X0;
    tb->m_core->X1_i = X1;
    tb->m_core->X2_i = X2;
    tb->m_core->X3_i = X3;
    tb->m_core->Y0_i = Y0;
    tb->m_core->Y1_i = Y1;
    tb->m_core->Y2_i = Y2;
    tb->m_core->Y3_i = Y3;
    tb->m_core->Z0_i = rand() & 0xFF;
    tb->m_core->Z1_i = rand() & 0xFF;
    tb->m_core->Z2_i = rand() & 0xFF;
    tb->m_core->Z3_i = rand() & 0xFF;
    tb->m_core->Z4_i = rand() & 0xFF;
    tb->m_core->Z5_i = rand() & 0xFF;
    #elif TC == isw_and_1storder
    int X0, X1, Y0, Y1, Z;
    X0 = rand()  & 0xFF;
    X1 = X^X0;
    Y0 = rand()  & 0xFF;
    Y1 = Y^Y0;

    tb->m_core->X0_i = X0;
    tb->m_core->X1_i = X1;
    tb->m_core->Y0_i = Y0;
    tb->m_core->Y1_i = Y1;
    tb->m_core->R01_i = 0; //rand() & 0xFF;
    #elif TC == ti_toffoli
    int Z = rand() & 0xff;

    Q = Q ^ Z;

    int X1, X2, X3, Y1, Y2, Y3, Z1, Z2, Z3;

    X1 = rand() & 0xff;
    X2 = rand() & 0xff;
    X3 = X ^ X1 ^ X2;
    Y1 = rand() & 0xff;
    Y2 = rand() & 0xff;
    Y3 = Y ^ Y1 ^ Y2;
    Z1 = rand() & 0xff;
    Z2 = rand() & 0xff;
    Z3 = Z ^ Z1 ^ Z2;

    tb->m_core->X1_i = X1;
    tb->m_core->X2_i = X2;
    tb->m_core->X3_i = X3;

    tb->m_core->Y1_i = Y1;
    tb->m_core->Y2_i = Y2;
    tb->m_core->Y3_i = Y3;

    tb->m_core->Z1_i = Z1;
    tb->m_core->Z2_i = Z2;
    tb->m_core->Z3_i = Z3;

    #endif
    tb->tick();
    tb->tick();
    tb->tick();

    int q;
    #if TC == dom_and_1storder
    q = tb->m_core->Q0_o ^ tb->m_core->Q1_o;
    #elif TC == dom_and_2ndorder
    q = tb->m_core->Q0_o ^ tb->m_core->Q1_o ^ tb->m_core->Q2_o;
    #elif TC == dom_and_3rdorder
    q = tb->m_core->Q0_o ^ tb->m_core->Q1_o ^ tb->m_core->Q2_o ^ tb->m_core->Q3_o;
    #elif TC == isw_and_1storder
    q = tb->m_core->Q0_o ^ tb->m_core->Q1_o;
    #elif TC == ti_toffoli
    q = tb->m_core->C1_o ^ tb->m_core->C2_o ^ tb->m_core->C3_o;
    #endif

    if (q != Q)
        std::cout << "Error." <<"\n" << std::endl;
    else
        std::cout << "OK." <<"\n" << std::endl;
    assert(q==Q);

    tb->tick();
    tb->tick();
    tb->tick();
    tb->tick();
    tb->tick();
    tb->tick();
    
    tb->closetrace();
}
