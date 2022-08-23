#include "Vcircuit.h"
#include "testbench_keccak.h"
#include <iostream>


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

    //Testcase: for functional purposes only!
    tb->m_core->A0_i = 0x849b83f6;
    tb->m_core->A1_i = 0xa091a781;
    tb->m_core->B0_i = 0xf4e969d5;
    tb->m_core->B1_i = 0x26029b6b;
    tb->m_core->C0_i = 0xc220e3a4;
    tb->m_core->C1_i = 0x398e6453;
    tb->m_core->D0_i = 0x2b171c97;
    tb->m_core->D1_i = 0x78b73e09;
    tb->m_core->E0_i = 0x6e7dce4;
    tb->m_core->E1_i = 0x5f35d56c;
    tb->m_core->rand_i = 0x6e617f0d;
    tb->tick();
    tb->tick();
    tb->tick();

    int q;

    q = tb->m_core->A0_o ^ tb->m_core->A1_o;
    if (q != 0xd0e2136)
        std::cout << "Error." <<"\n" << std::endl;
    else
        std::cout << "OK." <<"\n" << std::endl;

    q = tb->m_core->B0_o ^ tb->m_core->B1_o;
    if (q != 0xd2ebd2b6)
        std::cout << "Error." <<"\n" << std::endl;
    else
        std::cout << "OK." <<"\n" << std::endl;

    q = tb->m_core->C0_o ^ tb->m_core->C1_o;
    if (q != 0xf3fc8ef7)
        std::cout << "Error." <<"\n" << std::endl;
    else
        std::cout << "OK." <<"\n" << std::endl;

    q = tb->m_core->D0_o ^ tb->m_core->D1_o;
    if (q != 0x77a806e9)
        std::cout << "Error." <<"\n" << std::endl;
    else
        std::cout << "OK." <<"\n" << std::endl;

    q = tb->m_core->E0_o ^ tb->m_core->E1_o;
    if (q != 0x8b33db00)
        std::cout << "Error." <<"\n" << std::endl;
    else
        std::cout << "OK." <<"\n" << std::endl;
       

    tb->tick();
    tb->tick();
    tb->tick();
    tb->tick();
    tb->tick();
    tb->tick();

    
    tb->closetrace();
}
