#ifndef CELL_H
#define CELL_H

#include <cstring>
#include <string>
#include "cell_types.h"
#include <unordered_map>
#include <iostream>

constexpr const char* ILLEGAL_SIGNAL_STRING      = "Could not parse string signal";
constexpr const char* ILLEGAL_SIGNAL_TYPE        = "Could not parse signal from illegal type";
constexpr const char* ILLEGAL_CELL_TYPE          = "Could not parse cell type from string";
constexpr const char* ILLEGAL_PORT_DIRECTION     = "Port direction must be \"input\" or \"output\"";
constexpr const char* ILLEGAL_SIGNAL_LIST        = "Detected non-array type for signal list";
constexpr const char* ILLEGAL_NAME_REDECLARATION = "Redeclaration of existing signal name";
constexpr const char* ILLEGAL_CELL_CYCLE         = "Cell definition produces cycle";
constexpr const char* ILLEGAL_MISSING_SIGNALS    = "Found signals that are used but undefined";
constexpr const char* ILLEGAL_CLOCK_EDGE         = "Found both posedge and negedge flip-flops";
constexpr const char* ILLEGAL_CLOCK_SIGNAL       = "Found illegal clock signal";
constexpr const char* ILLEGAL_MULTIPLE_CLOCKS    = "Found multiple clocks driving different registers";

constexpr const char* ILLEGAL_SIGNAL_OVERWRITE   = "Overwriting value of signal during evaluation";
constexpr const char* ILLEGAL_VALUE_NOT_CONST    = "Attempted getting constant from non-const value";
constexpr const char* ILLEGAL_VALUE_NOT_SYMBOLIC = "Attempted getting pvs from non-pvs value";


enum class signal_id_t : uint32_t {S_0 = 0, S_1 = 1, S_X = UINT32_MAX, S_Z = UINT32_MAX - 1};

signal_id_t signal_from_str(const std::string& s);
std::string vcd_identifier(signal_id_t sig);

struct UnaryPorts
{
    signal_id_t m_in_a;
    signal_id_t m_out_y;
    constexpr UnaryPorts(signal_id_t a, signal_id_t y) :
            m_in_a(a), m_out_y(y) {}
};

struct BinaryPorts : public UnaryPorts
{
    signal_id_t m_in_b;
    constexpr BinaryPorts(signal_id_t a, signal_id_t b, signal_id_t y) :
            UnaryPorts(a, y), m_in_b(b) {}
};

struct MultiplexerPorts : public BinaryPorts
{
    signal_id_t m_in_s;
    constexpr MultiplexerPorts(signal_id_t a, signal_id_t b, signal_id_t s, signal_id_t y) :
            BinaryPorts(a, b, y), m_in_s(s) {}
};

struct DffPorts
{
    signal_id_t m_in_c;
    signal_id_t m_in_d;
    signal_id_t m_out_q;
    constexpr DffPorts(signal_id_t c, signal_id_t d, signal_id_t q) :
            m_in_c(c), m_in_d(d), m_out_q(q) {}
};

struct DffrPorts : public DffPorts
{
    signal_id_t m_in_r;
    constexpr DffrPorts(signal_id_t c, signal_id_t d, signal_id_t q, signal_id_t r) :
            DffPorts(c, d, q), m_in_r(r) {};
};

struct DffePorts : public DffPorts
{
    signal_id_t m_in_e;
    constexpr DffePorts(signal_id_t c, signal_id_t d, signal_id_t q, signal_id_t e) :
            DffPorts(c, d, q), m_in_e(e) {};
};

struct DfferPorts : public DffrPorts
{
    signal_id_t m_in_e;
    constexpr DfferPorts(signal_id_t c, signal_id_t d, signal_id_t q, signal_id_t r, signal_id_t e) :
            DffrPorts(c, d, q, r), m_in_e(e) {};
};

union Ports
{
public:
    BinaryPorts      m_bin;
    UnaryPorts       m_unr;
    DffPorts         m_dff;
    DffrPorts        m_dffr;
    DffePorts        m_dffe;
    DfferPorts       m_dffer;
    MultiplexerPorts m_mux;
    inline Ports() { std::memset(this, 0, sizeof(Ports)); };
};

class Cell
{
protected:
    std::string m_name;
    cell_type_t m_type;
    Ports m_ports;
    friend class Circuit;
public:
    const std::string& name() const { return m_name; }
    cell_type_t type() const { return m_type; }
    Cell(std::string c_name, cell_type_t c_type, UnaryPorts c_ports);
    Cell(std::string c_name, cell_type_t c_type, BinaryPorts c_ports);
    Cell(std::string c_name, cell_type_t c_type, DffPorts c_ports);
    Cell(std::string c_name, cell_type_t c_type, DffrPorts c_ports);
    Cell(std::string c_name, cell_type_t c_type, DffePorts c_ports);
    Cell(std::string c_name, cell_type_t c_type, DfferPorts c_ports);
    Cell(std::string c_name, cell_type_t c_type, MultiplexerPorts c_ports);

    template <typename V, typename R>
    void eval(const std::unordered_map<signal_id_t, V>& prev_signals, std::unordered_map<signal_id_t, V>& curr_signals) const;
};

extern bool mux(bool cond, bool t_val, bool e_val);

#define USE_MUX

template <typename V, typename R>
void Cell::eval(const std::unordered_map<signal_id_t, V>& prev_signals, std::unordered_map<signal_id_t, V>& curr_signals) const
{
    if (is_unary(m_type))
    {
        const signal_id_t out_y = m_ports.m_unr.m_out_y;
        const signal_id_t in_a = m_ports.m_unr.m_in_a;

        if(curr_signals.find(out_y) != curr_signals.end())
            { throw std::logic_error(ILLEGAL_SIGNAL_OVERWRITE); }
        assert(curr_signals.find(in_a) != curr_signals.end());
        const V& val_a = curr_signals.at(in_a);

        auto prev_it_a = prev_signals.find(in_a);
        auto prev_it_y = prev_signals.find(out_y);
        if (prev_it_a != prev_signals.end() &&
            prev_it_y != prev_signals.end() &&
            val_a == prev_it_a->second)
        { curr_signals[out_y] = prev_it_y->second; }
        else
        {
            V val_y = val_a;
            if (is_out_negated(m_type)) val_y = !val_y;
            curr_signals[out_y] = val_y;
        }
    }
    else if (is_binary(m_type))
    {
        const signal_id_t out_y = m_ports.m_bin.m_out_y;
        const signal_id_t in_a = m_ports.m_bin.m_in_a;
        const signal_id_t in_b = m_ports.m_bin.m_in_b;

        if(curr_signals.find(out_y) != curr_signals.end())
        { throw std::logic_error(ILLEGAL_SIGNAL_OVERWRITE); }

        const V& val_a = curr_signals.at(in_a);
        V val_b = curr_signals.at(in_b);

        auto prev_it_a = prev_signals.find(in_a);
        auto prev_it_b = prev_signals.find(in_b);
        auto prev_it_y = prev_signals.find(out_y);

        if (prev_it_a != prev_signals.end() &&
            prev_it_b != prev_signals.end() &&
            prev_it_y != prev_signals.end() &&
            val_a == prev_it_a->second && val_b == prev_it_b->second)
        { curr_signals[out_y] = prev_it_y->second; }
        else
        {
            V val_y;
            if (is_second_negated(m_type)) val_b = !val_b;

            if (gate_is_like_and(m_type))
            { val_y = val_a & val_b; }
            else if (gate_is_like_xor(m_type))
            { val_y = val_a ^ val_b; }
            else
            {
                assert(gate_is_like_or(m_type));
                val_y = val_a | val_b;
            }

            if (is_out_negated(m_type)) val_y = !val_y;
            curr_signals[out_y] = val_y;
        }
    }
    else if (is_multiplexer(m_type))
    {
        const signal_id_t out_y = m_ports.m_mux.m_out_y;
        const signal_id_t in_a = m_ports.m_mux.m_in_a;
        const signal_id_t in_b = m_ports.m_mux.m_in_b;
        const signal_id_t in_s = m_ports.m_mux.m_in_s;

        if(curr_signals.find(out_y) != curr_signals.end())
        { throw std::logic_error(ILLEGAL_SIGNAL_OVERWRITE); }

        const V& val_a = curr_signals.at(in_a);
        const V& val_b = curr_signals.at(in_b);
        const V& val_s = curr_signals.at(in_s);

        auto prev_it_a = prev_signals.find(in_a);
        auto prev_it_b = prev_signals.find(in_b);
        auto prev_it_s = prev_signals.find(in_s);
        auto prev_it_y = prev_signals.find(out_y);

        if (prev_it_a != prev_signals.end() &&
            prev_it_b != prev_signals.end() &&
            prev_it_s != prev_signals.end() &&
            prev_it_y != prev_signals.end() &&
            val_a == prev_it_a->second &&
            val_b == prev_it_b->second &&
            val_s == prev_it_s->second)
        { curr_signals[out_y] = prev_it_y->second; }
        else
        {
            #ifndef USE_MUX
            V val_y = (!val_s & val_a) ^ (val_s & val_b); // plain variant
            #else
            V val_y = mux(val_s, val_b, val_a); // mux variant
            #endif // USE_MUX
            if (is_out_negated(m_type)) val_y = !val_y;
            curr_signals[out_y] = val_y;
        }
    }
    else
    {
        assert(is_register(m_type));

        const signal_id_t out_q = m_ports.m_dff.m_out_q;
        // const signal_id_t in_c = m_ports.m_dff.m_in_c;
        const signal_id_t in_d = m_ports.m_dff.m_in_d;

        if(curr_signals.find(out_q) != curr_signals.end())
        { throw std::logic_error(ILLEGAL_SIGNAL_OVERWRITE); }

        const V& val_q = +prev_signals.at(out_q);
        // const V& val_c = curr_signals.at(in_c);
        const V& val_d = +prev_signals.at(in_d);

        V val_non_reset_wb; // value written back in non-reset case

        if (dff_has_enable(m_type))
        {
            bool e_only = test_is_reg_with_enable(m_type);
            const signal_id_t in_e = e_only ? m_ports.m_dffe.m_in_e : m_ports.m_dffer.m_in_e;
            V val_e = +prev_signals.at(in_e);
            if (!dff_enable_trigger(m_type)) val_e = !val_e;
            #ifndef USE_MUX
            val_non_reset_wb = (val_e & val_d) ^ (!val_e & val_q); // plain variant
            #else
            val_non_reset_wb = mux(val_e, val_d, val_q); // mux variant
            #endif // USE_MUX
        }
        else
        {
            val_non_reset_wb = val_d;
        }

        V val_fresh_q = +prev_signals.at(out_q);
        R val_res_q(val_fresh_q);

        if (dff_has_reset(m_type))
        {
            bool r_only = test_is_reg_with_reset(m_type);
            const signal_id_t in_r = r_only ? m_ports.m_dffr.m_in_r : m_ports.m_dffer.m_in_r;
            V val_r = +prev_signals.at(in_r);
            if (!dff_reset_trigger(m_type)) val_r = !val_r;
            V val_reset_wb = V(dff_reset_value(m_type));
            #ifndef USE_MUX
            val_res_q = (val_r & val_reset_wb) ^ (!val_r & val_non_reset_wb); // plain variant
            #else
            val_res_q = mux(val_r, val_reset_wb, val_non_reset_wb); // mux variant
            #endif // USE_MUX
        }
        else
        {
            val_res_q = val_non_reset_wb;
        }

        curr_signals[out_q] = val_res_q;
    }
}


#endif // CELL_H
