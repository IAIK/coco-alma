#pragma once

#include "verilated.h"
#include "verilated_vcd_c.h"
#include <iostream>
template<class Module> struct Testbench
{
    unsigned long m_tickcount;
    Module        *m_core;
    VerilatedVcdC *m_trace;
    
    Testbench(void)
    {
        Verilated::traceEverOn(true);
        m_core = new Module;
        m_tickcount = 0ul;
    }

    virtual ~Testbench(void)
    {
        delete m_core;
        m_core = NULL;
    }
    
    virtual void opentrace(const char *vcdname) 
    {
        //This is very dangerous...
        //if (!m_trace) 
        //{
        
        if(m_trace)
          std::cerr << "WARNING! m_trace non-zero - are you overwriting the object?" << std::endl;

        m_trace = new VerilatedVcdC;
        m_core->trace(m_trace, 99);
        m_trace->open(vcdname);    
        //}
        
    }
    
    virtual void closetrace(void) 
    {
        if (m_trace) 
        {
            m_trace->close();
            m_trace = NULL;
        }
    }
    
    virtual void reset(void) 
    {
        m_core->rst_sys_n = 0;
        this->tick();
        this->tick();
        m_core->rst_sys_n = 1;
    }

    virtual void tick(void) 
    {   
        // Falling edge
        m_core->clk_sys = 0;
        m_core->eval();
        if(m_trace) m_trace->dump(20*m_tickcount);
        
        // Rising edge
        m_core->clk_sys = 1;
        m_core->eval();
        if(m_trace) m_trace->dump(20*m_tickcount + 10);
        
        // Falling edge settle eval
        m_core->clk_sys = 0;
        m_core->eval();
        
        if(m_trace) m_trace->flush();
        m_tickcount++;
    }

    virtual bool done(void) { return (Verilated::gotFinish()); }
};
