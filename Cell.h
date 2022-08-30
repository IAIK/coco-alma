#ifndef CELL_H
#define CELL_H

#include <cstring>
#include <string>
#include "cell_types.h"

constexpr const char* ILLEGAL_SIGNAL_STRING      = "Could not parse string signal";
constexpr const char* ILLEGAL_SIGNAL_TYPE        = "Could not parse signal from illegal type";
constexpr const char* ILLEGAL_CELL_TYPE          = "Could not parse cell type from string";
constexpr const char* ILLEGAL_PORT_DIRECTION     = "Port direction must be \"input\" or \"output\"";
constexpr const char* ILLEGAL_SIGNAL_LIST        = "Detected non-array type for signal list";
constexpr const char* ILLEGAL_NAME_REDECLARATION = "Redeclaration of existing signal name";
constexpr const char* ILLEGAL_CELL_CYCLE         = "Cell definition produces cycle";
constexpr const char* ILLEGAL_MISSING_SIGNALS    = "Found signals that are used but undefined";
constexpr const char* ILLEGAL_CLOCK_EDGE         = "Found both posedge and negedge flip-flops";


enum class signal_id_t : uint32_t {S_0 = 0, S_1 = 1, S_X = UINT32_MAX, S_Z = UINT32_MAX - 1};

signal_id_t signal_from_str(const std::string& s);

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
};

#endif // CELL_H
