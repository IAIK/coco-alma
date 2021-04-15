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

#define acc(ptr, i, j) (ptr + i * (N+1) + j)

int main(int argc, char **argv) 
{
	Verilated::commandArgs(argc, argv);
	Testbench<Vcircuit> *tb = new Testbench<Vcircuit>();
    tb->opentrace("tmp.vcd");
    
    uint8_t key[16] = {0x3d, 0xc6, 0xa6, 0xa4, 0x30, 0x19, 0xb1, 0xa4, 
                       0xd5, 0x31, 0x0f, 0x1f, 0x00, 0x4d, 0x8d, 0x2d};
    uint8_t  pt[16] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 
                       0x38, 0x39, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66};
    uint8_t ect[16] = {0x68, 0x4d, 0x58, 0x3b, 0xa3, 0x24, 0x41, 0x6f, 
                       0xad, 0xe4, 0xa4, 0xa6, 0x53, 0xe8, 0x13, 0xfc};
    srand(42);
    const int BLOCK_SIZE = 16 * (N + 1) / sizeof(WData);
    WData masked_key[BLOCK_SIZE];
    WData masked_pt[BLOCK_SIZE];
    uint8_t* u8_key = (uint8_t*)masked_key;
    uint8_t* u8_pt = (uint8_t*)masked_pt;
    
    
    for (int i = 0; i < 16; i++)
    {
        *acc(u8_key, i, 0) = key[i];
        *acc(u8_pt, i, 0) = pt[i];
        
        for (int j = 1; j <= N; j++)
        {
            *acc(u8_key, i, j) = rand() & 0xff;
            *acc(u8_pt, i, j) = rand() & 0xff;
            
            *acc(u8_key, i, 0) ^= *acc(u8_key, i, j);
            *acc(u8_pt, i, 0) ^= *acc(u8_pt, i, j);
        }
    }
    
    tb->reset();
    
    tb->m_core->StartxSI = 1;
    for (int i = 0; i < BLOCK_SIZE; i++)
    {
        tb->m_core->KxDI[i] = masked_key[i];
        tb->m_core->PTxDI[i] = masked_pt[i];
    }
    tb->tick();
    
    tb->m_core->StartxSI = 0;
    for (int i = 0; i < BLOCK_SIZE; i++)
    {
        tb->m_core->KxDI[i] = 0;
        tb->m_core->PTxDI[i] = 0;
    }
    tb->tick();
    
    while (tb->m_core->DonexSO != 1 && tb->m_tickcount < 300)
    { tb->tick(); }
    
    WData masked_ct[BLOCK_SIZE];
    uint8_t* u8_ct = (uint8_t*)masked_ct;
    
    for (int i = 0; i < BLOCK_SIZE; i++)
        masked_ct[i] = tb->m_core->CxDO[i];
    
    uint8_t rct[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    
    for (int i = 0; i < 16; i++)
        for (int j = 0; j <= N; j++) 
            rct[i] ^= *acc(u8_ct, i, j);
    
    printf("rct:  ");
    for(int i = 0; i < 16; i++) printf("%02x", rct[i]);
    printf("\n");
      
    printf("ect:  ");
    for(int i = 0; i < 16; i++) printf("%02x", ect[i]);
    printf("\n");
    
    tb->closetrace();
}
