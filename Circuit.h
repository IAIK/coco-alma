#ifndef CIRCUIT_H
#define CIRCUIT_H

#include <string>
#include <cstdint>
#include <utility>
#include <cassert>
#include <cstring>
#include <unordered_set>
#include <unordered_map>
#include <vector>

enum class signal_id_t : uint32_t {S_0 = 0, S_1 = 1, S_X = UINT32_MAX, S_Z = UINT32_MAX - 1};

enum class cell_type_t : uint32_t {
    CELL_AND = 0x100, CELL_OR = 0x101, CELL_XOR = 0x102, CELL_XNOR = 103,
    CELL_NOT = 0x200,
    CELL_DFF = 0x400,
    CELL_MUX = 0x800};

inline bool is_binary(cell_type_t x)
{ return static_cast<uint32_t>(x) & 0x100; }

inline bool is_unary(cell_type_t x)
{ return static_cast<uint32_t>(x) & 0x200; }

inline bool is_register(cell_type_t x)
{ return static_cast<uint32_t>(x) & 0x400; }

inline bool is_multiplexer(cell_type_t x)
{ return static_cast<uint32_t>(x) & 0x800; }



struct UnaryPorts
{
    signal_id_t m_in_a;
    signal_id_t m_out_y;
    UnaryPorts(signal_id_t a, signal_id_t y) :
            m_in_a(a), m_out_y(y) {}
};

struct BinaryPorts
{
    signal_id_t m_in_a;
    signal_id_t m_in_b;
    signal_id_t m_out_y;
    BinaryPorts(signal_id_t a, signal_id_t b, signal_id_t y) :
        m_in_a(a), m_in_b(b), m_out_y(y) {}
};

struct MultiplexerPorts
{
    signal_id_t m_in_a;
    signal_id_t m_in_b;
    signal_id_t m_in_s;
    signal_id_t m_out_y;
    MultiplexerPorts(signal_id_t a, signal_id_t b, signal_id_t s, signal_id_t y) :
            m_in_a(a), m_in_b(b), m_in_s(s), m_out_y(y) {}
};

struct RegisterPorts
{
    signal_id_t m_in_c;
    signal_id_t m_in_d;
    signal_id_t m_out_q;
    RegisterPorts(signal_id_t c, signal_id_t d, signal_id_t q) :
            m_in_c(c), m_in_d(d), m_out_q(q) {}
};

class Cell
{
protected:
    std::string m_name;
    cell_type_t m_type;
    union Ports
    {
    public:
        BinaryPorts      m_bin;
        UnaryPorts       m_unr;
        RegisterPorts    m_dff;
        MultiplexerPorts m_mux;
        inline Ports();
    } m_ports;
    template<typename... Ts>
    Cell(std::string c_name, cell_type_t c_type, Ts... c_ports);
public:
    const std::string& name() const { return m_name; }
};

inline Cell::Ports::Ports()
{
    std::memset(this, 0, sizeof(m_ports));
}

template<typename... Ts>
Cell::Cell(std::string c_name, cell_type_t c_type, Ts... c_ports) :
    m_name(std::move(c_name)), m_type(c_type)
{
    if(is_binary(m_type))
        m_ports.m_bin = BinaryPorts(c_ports...);
    else if(is_unary(m_type))
        m_ports.m_unr = UnaryPorts(c_ports...);
    else if(is_register(m_type))
        m_ports.m_dff = RegisterPorts(c_ports...);
    else if(is_multiplexer(m_type))
        m_ports.m_mux = MultiplexerPorts(c_ports...);
    else
        assert(false);
}

class CircuitParsingError : public std::logic_error
{
public:
    explicit CircuitParsingError(const char* message) : std::logic_error(message) {};
};

constexpr const char* ILLEGAL_SIGNAL_STRING      = "Could not parse string signal";
constexpr const char* ILLEGAL_SIGNAL_TYPE        = "Could not parse signal from illegal type";
constexpr const char* ILLEGAL_PORT_DIRECTION     = "Port direction must be \"input\" or \"output\"";
constexpr const char* ILLEGAL_SIGNAL_LIST        = "Detected non-array type for signal list";
constexpr const char* ILLEGAL_NAME_REDECLARATION = "Redeclaration of existing signal name";

signal_id_t signal_from_str(const std::string& s);

class Circuit
{
private:
    std::unordered_set<signal_id_t> m_in_ports;
    std::unordered_set<signal_id_t> m_out_ports;
    std::unordered_set<signal_id_t> m_signals;
    std::unordered_map<std::string, std::vector<signal_id_t>> m_name_bits;
    template <typename T> signal_id_t get_signal_any(T& bit);
public:
    Circuit(const std::string& json_file_path, const std::string& top_module_name);
};

template <typename T> signal_id_t Circuit::get_signal_any(T& bit)
{
    if(bit.is_number_unsigned())
        return static_cast<signal_id_t>((uint32_t)bit);
    else if(bit.is_string()) // it must be a constant
        return signal_from_str(bit);
    throw CircuitParsingError(ILLEGAL_SIGNAL_TYPE);
}

#endif // CIRCUIT_H
