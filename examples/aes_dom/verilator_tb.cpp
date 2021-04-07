#include "Vcircuit.h"
#include "testbench_aes.h"
#include <stdio.h>
#include <stdlib.h>

#define N 1
#define N2 (N * (N+1) / 2)

#define ZMULSIZE (N2 * 4)
#define ZINVSIZE (N2 * 2)

#define BMULSIZE (N * 4)
#define BINVSIZE (N * 2)

int main(int argc, char **argv) 
{
	Verilated::commandArgs(argc, argv);
	Testbench<Vcircuit> *tb = new Testbench<Vcircuit>();
    tb->opentrace("/tmp/tmp.vcd");
    
    uint8_t key[16] = {0x3d, 0xc6, 0xa6, 0xa4, 0x30, 0x19, 0xb1, 0xa4, 
                       0xd5, 0x31, 0x0f, 0x1f, 0x00, 0x4d, 0x8d, 0x2d};
    uint8_t  pt[16] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 
                       0x38, 0x39, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66};
    uint8_t ect[16] = {0x68, 0x4d, 0x58, 0x3b, 0xa3, 0x24, 0x41, 0x6f, 
                       0xad, 0xe4, 0xa4, 0xa6, 0x53, 0xe8, 0x13, 0xfc};
    srand(42);
    
    uint8_t key_a[16]; uint8_t key_b[16];
    uint8_t  pt_a[16]; uint8_t  pt_b[16];
    
    for (int i = 0; i < 16; i++)
    {
        key_a[i] = rand() & 0xff;
        key_b[i] = key[i] ^ key_a[i];
        
        pt_a[i] = rand() & 0xff;
        pt_b[i] = pt[i] ^ pt_a[i];
    }
    
    tb->reset();
    tb->m_core->StartxSI = 1;
    tb->tick();
    tb->m_core->StartxSI = 0;
    
    for (int i = 0; i < 16; i++)
    {
        tb->m_core->KxDI = (key_a[i] << 8) + key_b[i];
        tb->m_core->PTxDI = (pt_a[i] << 8) + pt_b[i];
        tb->tick();
    }
    
    while (tb->m_core->DonexSO != 1 && tb->m_tickcount < 300)
    { tb->tick(); }
    
    uint8_t ct_a[16]; uint8_t ct_b[16];
    uint8_t rct[16];
    
    for (int i = 0; i < 16; i++)
    {
        uint16_t c_ab = tb->m_core->CxDO;
        ct_a[i] = (c_ab >> 8) & 0xff;
        ct_b[i] = c_ab & 0xff;
        rct[i] = ct_a[i] ^ ct_b[i];
        tb->tick();
    }
    
    printf("ct_a: ");
    for(int i = 0; i < 16; i++) printf("%02x", ct_a[i]);
    printf("\n");
    
    printf("ct_b: ");
    for(int i = 0; i < 16; i++) printf("%02x", ct_b[i]);
    printf("\n");
    
    printf("rct:  ");
    for(int i = 0; i < 16; i++) printf("%02x", rct[i]);
    printf("\n");
      
    printf("ect:  ");
    for(int i = 0; i < 16; i++) printf("%02x", ect[i]);
    printf("\n");
    
    tb->closetrace();
}
