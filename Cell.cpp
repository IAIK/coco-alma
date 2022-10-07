#include <stdexcept>
#include <cassert>
#include "Cell.h"

cell_type_t cell_type_from_string(const std::string& x)
{
    // TODO: Think about replacing with constexpr map lookup
    if (x == "$_AND_") return cell_type_t::CELL_AND;
    else if (x == "$_OR_") return cell_type_t::CELL_OR;
    else if (x == "$_XOR_") return cell_type_t::CELL_XOR;
    else if (x == "$_NAND_") return cell_type_t::CELL_NAND;
    else if (x == "$_NOR_") return cell_type_t::CELL_NOR;
    else if (x == "$_XNOR_") return cell_type_t::CELL_XNOR;
    else if (x == "$_BUF_") return cell_type_t::CELL_BUF;
    else if (x == "$_NOT_") return cell_type_t::CELL_NOT;
    else if (x == "$_MUX_") return cell_type_t::CELL_MUX;
    else if (x == "$_NMUX_") return cell_type_t::CELL_NMUX;
    else if (x == "$_DFF_N_") return cell_type_t::CELL_DFF_N;
    else if (x == "$_DFF_P_") return cell_type_t::CELL_DFF_P;
    else if (x == "$_DFF_NN0_") return cell_type_t::CELL_DFF_NN0;
    else if (x == "$_DFF_NN1_") return cell_type_t::CELL_DFF_NN1;
    else if (x == "$_DFF_NP0_") return cell_type_t::CELL_DFF_NP0;
    else if (x == "$_DFF_NP1_") return cell_type_t::CELL_DFF_NP1;
    else if (x == "$_DFF_PN0_") return cell_type_t::CELL_DFF_PN0;
    else if (x == "$_DFF_PN1_") return cell_type_t::CELL_DFF_PN1;
    else if (x == "$_DFF_PP0_") return cell_type_t::CELL_DFF_PP0;
    else if (x == "$_DFF_PP1_") return cell_type_t::CELL_DFF_PP1;
    else if (x == "$_DFFE_NN_") return cell_type_t::CELL_DFFE_NN;
    else if (x == "$_DFFE_NP_") return cell_type_t::CELL_DFFE_NP;
    else if (x == "$_DFFE_PN_") return cell_type_t::CELL_DFFE_PN;
    else if (x == "$_DFFE_PP_") return cell_type_t::CELL_DFFE_PP;
    else if (x == "$_DFFE_NN0N_") return cell_type_t::CELL_DFFE_NN0N;
    else if (x == "$_DFFE_NN1N_") return cell_type_t::CELL_DFFE_NN1N;
    else if (x == "$_DFFE_NP0N_") return cell_type_t::CELL_DFFE_NP0N;
    else if (x == "$_DFFE_NP1N_") return cell_type_t::CELL_DFFE_NP1N;
    else if (x == "$_DFFE_PN0N_") return cell_type_t::CELL_DFFE_PN0N;
    else if (x == "$_DFFE_PN1N_") return cell_type_t::CELL_DFFE_PN1N;
    else if (x == "$_DFFE_PP0N_") return cell_type_t::CELL_DFFE_PP0N;
    else if (x == "$_DFFE_PP1N_") return cell_type_t::CELL_DFFE_PP1N;
    else if (x == "$_DFFE_NN0P_") return cell_type_t::CELL_DFFE_NN0P;
    else if (x == "$_DFFE_NN1P_") return cell_type_t::CELL_DFFE_NN1P;
    else if (x == "$_DFFE_NP0P_") return cell_type_t::CELL_DFFE_NP0P;
    else if (x == "$_DFFE_NP1P_") return cell_type_t::CELL_DFFE_NP1P;
    else if (x == "$_DFFE_PN0P_") return cell_type_t::CELL_DFFE_PN0P;
    else if (x == "$_DFFE_PN1P_") return cell_type_t::CELL_DFFE_PN1P;
    else if (x == "$_DFFE_PP0P_") return cell_type_t::CELL_DFFE_PP0P;
    else if (x == "$_DFFE_PP1P_") return cell_type_t::CELL_DFFE_PP1P;
    return cell_type_t::CELL_NONE;
}

signal_id_t signal_from_str(const std::string& s)
{
    if (s.length() != 1) throw std::logic_error(ILLEGAL_SIGNAL_STRING);
    const char c = s[0];
    if (c == '0' || c == '1') return static_cast<signal_id_t>(c - '0');
    else if (c == 'x' || c == 'X') return signal_id_t::S_X;
    else if (c == 'z' || c == 'Z') return signal_id_t::S_Z;
    throw std::logic_error(ILLEGAL_SIGNAL_STRING);
}

std::string vcd_identifier(signal_id_t sig)
{
    // The identifier code is composed of the printable characters which are in the
    // ASCII character set from ! to ~ (33 to 126).
    constexpr uint32_t MODULUS = (uint32_t)'~' - (uint32_t)'!' + 1;
    std::string result;
    result.reserve(6);
    auto sig_num = static_cast<uint32_t>(sig);
    do
    {
        const uint32_t remainder = (sig_num % MODULUS);
        result.push_back((char)('!' + remainder));
        sig_num = sig_num / MODULUS;
    } while (sig_num);
    return result;
}

Cell::Cell(std::string c_name, cell_type_t c_type, UnaryPorts c_ports) :
        m_name(std::move(c_name)), m_type(c_type)
{
    assert(is_unary(m_type));
    m_ports.m_unr = c_ports;
}

Cell::Cell(std::string c_name, cell_type_t c_type, BinaryPorts c_ports) :
        m_name(std::move(c_name)), m_type(c_type)
{
    assert(is_binary(m_type));
    m_ports.m_bin = c_ports;
}

Cell::Cell(std::string c_name, cell_type_t c_type, DffPorts c_ports) :
        m_name(std::move(c_name)), m_type(c_type)
{
    assert(is_register(m_type) && !dff_has_reset(m_type) && !dff_has_enable(m_type));
    m_ports.m_dff = c_ports;
}

Cell::Cell(std::string c_name, cell_type_t c_type, DffrPorts c_ports) :
        m_name(std::move(c_name)), m_type(c_type)
{
    assert(is_register(m_type) && dff_has_reset(m_type) && !dff_has_enable(m_type));
    m_ports.m_dffr = c_ports;
}

Cell::Cell(std::string c_name, cell_type_t c_type, DffePorts c_ports) :
        m_name(std::move(c_name)), m_type(c_type)
{
    assert(is_register(m_type) && !dff_has_reset(m_type) && dff_has_enable(m_type));
    m_ports.m_dffe = c_ports;
}

Cell::Cell(std::string c_name, cell_type_t c_type, DfferPorts c_ports) :
        m_name(std::move(c_name)), m_type(c_type)
{
    assert(is_register(m_type) && dff_has_reset(m_type) && dff_has_enable(m_type));
    m_ports.m_dffer = c_ports;
}

Cell::Cell(std::string c_name, cell_type_t c_type, MultiplexerPorts c_ports) :
        m_name(std::move(c_name)), m_type(c_type)
{
    assert(is_multiplexer(m_type));
    m_ports.m_mux = c_ports;
}