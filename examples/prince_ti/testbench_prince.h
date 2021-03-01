#pragma once

#include "verilated.h"
#include "verilated_vcd_c.h"

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
        if (!m_trace) 
        {
            m_trace = new VerilatedVcdC;
            m_core->trace(m_trace, 99);
            m_trace->open(vcdname);
        }
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
        m_core->i_reset = 1;
        this->tick();
        this->tick();
        m_core->i_reset = 0;
    }

    virtual void tick(void) 
    {   
        // Falling edge
        m_core->i_clk = 0;
        m_core->eval();
        if(m_trace) m_trace->dump(20*m_tickcount);
        
        // Rising edge
        m_core->i_clk = 1;
        m_core->eval();
        if(m_trace) m_trace->dump(20*m_tickcount + 10);
        
        // Falling edge settle eval
        m_core->i_clk = 0;
        m_core->eval();
        
        if(m_trace) m_trace->flush();
        m_tickcount++;
    }

    virtual bool done(void) { return (Verilated::gotFinish()); }
};
