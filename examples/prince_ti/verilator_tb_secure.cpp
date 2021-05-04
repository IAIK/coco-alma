#include "Vcircuit.h"
#include "testbench_prince.h"
#include <stdio.h>
#include <cstdlib>

void assign_random(Testbench<Vcircuit> *tb)
{
    printf("Assigning randomness\n");
    // tb->m_core->i_r = rand();
    tb->m_core->i_r[0] = rand();
    tb->m_core->i_r[1] = rand();
    tb->m_core->i_r[2] = rand();
    printf("Assigning randomness mid\n");
    tb->m_core->i_r[3] = rand();
    tb->m_core->i_r[4] = rand();
    tb->m_core->i_r[5] = rand();
    printf("Assigning randomness end\n");
}
int main(int argc, char **argv) 
{
	Verilated::commandArgs(argc, argv);
	Testbench<Vcircuit> *tb = new Testbench<Vcircuit>();
    tb->opentrace("tmp.vcd");
    
    tb->reset();
    
    tb->m_core->i_enc_dec = 0;
    assign_random(tb);
    // i_pt =  0000 0000 0000 0000
    // i_key = 0000 0000 0000 0000 0000 0000 0000 0000
    // k0 [2, 3] = 0000 0000 0000 0000
    // k1 [0, 1] = 0000 0000 0000 0000
    // o_ct =  8186 65aa 0d02 dfda
    
    /*
    tb->m_core->i_pt1 = 0xf2ab9aeb45079458;
    tb->m_core->i_pt2 = 0xf2ab9aeb45079458;
    
    tb->m_core->i_key1[0] = 0xb101598e;
    tb->m_core->i_key1[1] = 0x29cfa5cc;
    tb->m_core->i_key1[2] = 0x2fa459bd;
    tb->m_core->i_key1[3] = 0xfe7e5441;
    
    tb->m_core->i_key2[0] = 0xb101598e;
    tb->m_core->i_key2[1] = 0x29cfa5cc;
    tb->m_core->i_key2[2] = 0x2fa459bd;
    tb->m_core->i_key2[3] = 0xfe7e5441;
    */
    
    // i_pt =  ffff ffff ffff ffff
    // i_key = 0000 0000 0000 0000 0000 0000 0000 0000
    // k0 [2, 3] = 0000 0000 0000 0000
    // k1 [0, 1] = 0000 0000 0000 0000
    // o_ct =  604a e6ca 03c2 0ada
    
    /*
    tb->m_core->i_pt1 = 0x4634779fd4cefc5c;
    tb->m_core->i_pt2 = 0xb9cb88602b3103a3;
    
    tb->m_core->i_key1[0] = 0xb101598e;
    tb->m_core->i_key1[1] = 0x29cfa5cc;
    tb->m_core->i_key1[2] = 0x2fa459bd;
    tb->m_core->i_key1[3] = 0xfe7e5441;
    
    tb->m_core->i_key2[0] = 0xb101598e;
    tb->m_core->i_key2[1] = 0x29cfa5cc;
    tb->m_core->i_key2[2] = 0x2fa459bd;
    tb->m_core->i_key2[3] = 0xfe7e5441;
    */
    
    // i_pt =  0000 0000 0000 0000
    // i_key = ffff ffff ffff ffff 0000 0000 0000 0000  
    // k0 [2, 3] = ffff ffff ffff ffff
    // k1 [0, 1] = 0000 0000 0000 0000
    // o_ct =  9fb5 1935 fc3d f524
    
    /*
    tb->m_core->i_pt1 = 0x5ed0f5a4657740e0; 
    tb->m_core->i_pt2 = 0x5ed0f5a4657740e0;
    
    tb->m_core->i_key1[0] = 0xba800c41;
    tb->m_core->i_key1[1] = 0x6a0dea65;
    tb->m_core->i_key1[2] = 0x7c72d767;
    tb->m_core->i_key1[3] = 0xab8a2996;
    
    tb->m_core->i_key2[0] = 0xba800c41;
    tb->m_core->i_key2[1] = 0x6a0dea65;
    tb->m_core->i_key2[2] = 0x838d2898;
    tb->m_core->i_key2[3] = 0x5475d669;
    */
    
    // i_pt =  0000 0000 0000 0000
    // i_key = 0000 0000 0000 0000 ffff ffff ffff ffff
    // k0 [2, 3] = 0000 0000 0000 0000
    // k1 [0, 1] = ffff ffff ffff ffff
    // o_ct =  78a5 4cbe 737b b7ef
    
    /*
    tb->m_core->i_pt1 = 0x5fec9d9c714ed9c9; 
    tb->m_core->i_pt2 = 0x5fec9d9c714ed9c9;
    
    tb->m_core->i_key1[0] = 0xeadc3c9d;
    tb->m_core->i_key1[1] = 0x9c9269c3;
    tb->m_core->i_key1[2] = 0xacbfd2c4;
    tb->m_core->i_key1[3] = 0xc4880f31;
    
    tb->m_core->i_key2[0] = 0x1523c362;
    tb->m_core->i_key2[1] = 0x636d963c;
    tb->m_core->i_key2[2] = 0xacbfd2c4;
    tb->m_core->i_key2[3] = 0xc4880f31;
    */
    
    // i_pt =  0123 4567 89ab cdef
    // i_key = 0000 0000 0000 0000 fedc ba98 7654 3210
    // k0 [2, 3] = 0000 0000 0000 0000
    // k1 [0, 1] = fedc ba98 7654 3210
    // o_ct =  ae25 ad3c a8fa 9ccf
    
    // /*
    tb->m_core->i_pt1 = 0x4a3d8ed7beb23314; 
    tb->m_core->i_pt2 = 0x4b1ecbb03719fefb;
    
    tb->m_core->i_key1[0] = 0x56b4cf12;
    tb->m_core->i_key1[1] = 0x93d7168d;
    tb->m_core->i_key1[2] = 0xddabce8e;
    tb->m_core->i_key1[3] = 0xa8366ca7;
    
    tb->m_core->i_key2[0] = 0x20e0fd02;
    tb->m_core->i_key2[1] = 0x6d0bac15;
    tb->m_core->i_key2[2] = 0xddabce8e;
    tb->m_core->i_key2[3] = 0xa8366ca7;
    // */
    
    tb->m_core->i_load = 1;
    tb->m_core->i_start = 0;
    assign_random(tb);
    tb->tick();
    tb->m_core->i_load = 0;
    tb->m_core->i_start = 1;
    assign_random(tb);
    tb->tick();
    
    while (tb->m_core->o_done != 1)
    { 
      assign_random(tb);
      tb->tick(); 
    }
    
    unsigned long long ct1 = tb->m_core->o_ct1;
    unsigned long long ct2 = tb->m_core->o_ct2;
    unsigned long long res = ct1 ^ ct2;
    
    printf("ct1: %016lx\nct2: %016lx\nres: %016lx\n", ct1, ct2, res);
    
    tb->closetrace();
}
