#include "test_util.h"
#include "Cell.h"
#include <unordered_map>
#include <iostream>

bool mux(bool cond, bool t_val, bool e_val)
{
    return cond ? t_val : e_val;
}

#define ignored __attribute__((unused))

bool and_gate(bool a, bool b, ignored bool s = false) { return a & b; }
bool or_gate(bool a, bool b, ignored bool s = false) { return a | b; }
bool xor_gate(bool a, bool b, ignored bool s = false) { return a ^ b; }

bool nand_gate(bool a, bool b, ignored bool s = false) { return !(a & b); }
bool nor_gate(bool a, bool b, ignored bool s = false) { return !(a | b); }
bool xnor_gate(bool a, bool b, ignored bool s = false) { return !(a ^ b); }

bool andnot_gate(bool a, bool b, ignored bool s = false) { return a & !b; }
bool ornot_gate(bool a, bool b, ignored bool s = false) { return a | !b; }

bool not_gate(bool a, ignored bool b = false, ignored bool s = false) { return !a; }
bool buf_gate(bool a, ignored bool b = false, ignored bool s = false) { return a; }

bool mux_gate(bool a, bool b, bool s) { return s ? b : a; }
bool nmux_gate(bool a, bool b, bool s) { return s ? !b : !a; }

#define DEFINE_GATE_TEST(CELL_TYPE, FUNCTION, PORTS) \
void test_ ## FUNCTION() \
{ \
    std::unordered_map<signal_id_t, bool> prev_vals; \
    std::unordered_map<signal_id_t, bool> curr_vals; \
    auto a = static_cast<signal_id_t>(2); \
    auto b = static_cast<signal_id_t>(3); \
    auto s = static_cast<signal_id_t>(4); \
    auto y = static_cast<signal_id_t>(5); \
    curr_vals[a] = false; \
    curr_vals[b] = false; \
    curr_vals[s] = false; \
    Cell cell(# FUNCTION, cell_type_t::CELL_TYPE, PORTS); \
    for (int input = 0; input < 8; input++) \
    { \
        bool val_a = (input >> 0) & 1; \
        bool val_b = (input >> 1) & 1; \
        bool val_s = (input >> 2) & 1; \
        bool val_y = FUNCTION(val_a, val_b, val_s); \
        curr_vals[a] = val_a; \
        curr_vals[b] = val_b; \
        curr_vals[s] = val_s; \
        curr_vals.erase(y); \
        cell.eval<bool, bool&>(prev_vals, curr_vals); \
        std::cout << #FUNCTION << " " << val_a << " " << val_b << " " << val_s << " = "; \
        std::cout << curr_vals[y] << " should be (" << val_y << ")" << std::endl; \
        assert(curr_vals[y] == val_y); \
    } \
}

DEFINE_GATE_TEST(CELL_AND,    and_gate,    BinaryPorts(a, b, y))
DEFINE_GATE_TEST(CELL_OR,     or_gate,     BinaryPorts(a, b, y))
DEFINE_GATE_TEST(CELL_XOR,    xor_gate,    BinaryPorts(a, b, y))
DEFINE_GATE_TEST(CELL_NAND,   nand_gate,   BinaryPorts(a, b, y))
DEFINE_GATE_TEST(CELL_NOR,    nor_gate,    BinaryPorts(a, b, y))
DEFINE_GATE_TEST(CELL_XNOR,   xnor_gate,   BinaryPorts(a, b, y))
DEFINE_GATE_TEST(CELL_ANDNOT, andnot_gate, BinaryPorts(a, b, y))
DEFINE_GATE_TEST(CELL_ORNOT,  ornot_gate,  BinaryPorts(a, b, y))

DEFINE_GATE_TEST(CELL_NOT, not_gate, UnaryPorts(a, y))
DEFINE_GATE_TEST(CELL_BUF, buf_gate, UnaryPorts(a, y))

DEFINE_GATE_TEST(CELL_MUX,  mux_gate,  MultiplexerPorts(a, b, s, y))
DEFINE_GATE_TEST(CELL_NMUX, nmux_gate, MultiplexerPorts(a, b, s, y))

bool dff_p_reg(bool d, ignored bool q, ignored bool r = false, ignored bool e = false) { return d; }
#define dff_n_reg dff_p_reg

bool dff_pp0_reg(bool d, ignored bool q, bool r, ignored bool e = false) { return r == true  ? false : d; }
bool dff_pp1_reg(bool d, ignored bool q, bool r, ignored bool e = false) { return r == true  ? true  : d; }
bool dff_pn0_reg(bool d, ignored bool q, bool r, ignored bool e = false) { return r == false ? false : d; }
bool dff_pn1_reg(bool d, ignored bool q, bool r, ignored bool e = false) { return r == false ? true  : d; }
#define dff_np0_reg dff_pp0_reg
#define dff_np1_reg dff_pp1_reg
#define dff_nn0_reg dff_pn0_reg
#define dff_nn1_reg dff_pn1_reg

bool dffe_pp_reg(bool d, bool q, ignored bool r, bool e) { return e == true ? d : q; }
bool dffe_pn_reg(bool d, bool q, ignored bool r, bool e) { return e == false ? d : q; }
#define dffe_np_reg dffe_pp_reg
#define dffe_nn_reg dffe_pn_reg

bool dffe_pp0p_reg(bool d, bool q, bool r, bool e) { return r == true  ? false : (e == true ? d : q); }
bool dffe_pp1p_reg(bool d, bool q, bool r, bool e) { return r == true  ? true  : (e == true ? d : q); }
bool dffe_pn0p_reg(bool d, bool q, bool r, bool e) { return r == false ? false : (e == true ? d : q); }
bool dffe_pn1p_reg(bool d, bool q, bool r, bool e) { return r == false ? true  : (e == true ? d : q); }
bool dffe_pp0n_reg(bool d, bool q, bool r, bool e) { return r == true  ? false : (e == false ? d : q); }
bool dffe_pp1n_reg(bool d, bool q, bool r, bool e) { return r == true  ? true  : (e == false ? d : q); }
bool dffe_pn0n_reg(bool d, bool q, bool r, bool e) { return r == false ? false : (e == false ? d : q); }
bool dffe_pn1n_reg(bool d, bool q, bool r, bool e) { return r == false ? true  : (e == false ? d : q); }
#define dffe_np0p_reg dffe_pp0p_reg
#define dffe_np1p_reg dffe_pp1p_reg
#define dffe_nn0p_reg dffe_pn0p_reg
#define dffe_nn1p_reg dffe_pn1p_reg
#define dffe_np0n_reg dffe_pp0n_reg
#define dffe_np1n_reg dffe_pp1n_reg
#define dffe_nn0n_reg dffe_pn0n_reg
#define dffe_nn1n_reg dffe_pn1n_reg

#define DEFINE_REGISTER_TEST(CELL_TYPE, FUNCTION, PORTS) \
void test_ ## FUNCTION() \
{ \
    std::unordered_map<signal_id_t, bool> prev_vals; \
    std::unordered_map<signal_id_t, bool> curr_vals; \
    auto c = static_cast<signal_id_t>(2); \
    auto d = static_cast<signal_id_t>(3); \
    auto q = static_cast<signal_id_t>(4); \
    auto r = static_cast<signal_id_t>(5); \
    auto e = static_cast<signal_id_t>(6); \
                                          \
    prev_vals[c] = false; \
    prev_vals[d] = false; \
    prev_vals[q] = false; \
    prev_vals[r] = false; \
    prev_vals[e] = false; \
    Cell cell(# FUNCTION, cell_type_t::CELL_TYPE, PORTS); \
    for (int input = 0; input < 32; input++) \
    { \
        bool val_c = (input >> 0) & 1; \
        bool val_d = (input >> 1) & 1; \
        bool val_q = (input >> 2) & 1; \
        bool val_r = (input >> 3) & 1; \
        bool val_e = (input >> 4) & 1; \
        bool val_nq = FUNCTION(val_d, val_q, val_r, val_e); \
        prev_vals[c] = val_c; \
        prev_vals[d] = val_d; \
        prev_vals[q] = val_q; \
        prev_vals[r] = val_r; \
        prev_vals[e] = val_e; \
        curr_vals.erase(q); \
        cell.eval<bool, bool&>(prev_vals, curr_vals); \
        std::cout << #FUNCTION << " " << val_c << " " << val_d << " " << val_q << " " << val_r << " " << val_e << " = "; \
        std::cout << curr_vals[q] << " should be (" << val_nq << ")" << std::endl; \
        assert(curr_vals[q] == val_nq); \
    } \
}

DEFINE_REGISTER_TEST(CELL_DFF_P, dff_p_reg, DffPorts(c, d, q))
DEFINE_REGISTER_TEST(CELL_DFF_N, dff_n_reg, DffPorts(c, d, q))

DEFINE_REGISTER_TEST(CELL_DFF_PP0, dff_pp0_reg, DffrPorts(c, d, q, r))
DEFINE_REGISTER_TEST(CELL_DFF_PP1, dff_pp1_reg, DffrPorts(c, d, q, r))
DEFINE_REGISTER_TEST(CELL_DFF_PN0, dff_pn0_reg, DffrPorts(c, d, q, r))
DEFINE_REGISTER_TEST(CELL_DFF_PN1, dff_pn1_reg, DffrPorts(c, d, q, r))
DEFINE_REGISTER_TEST(CELL_DFF_NP0, dff_np0_reg, DffrPorts(c, d, q, r))
DEFINE_REGISTER_TEST(CELL_DFF_NP1, dff_np1_reg, DffrPorts(c, d, q, r))
DEFINE_REGISTER_TEST(CELL_DFF_NN0, dff_nn0_reg, DffrPorts(c, d, q, r))
DEFINE_REGISTER_TEST(CELL_DFF_NN1, dff_nn1_reg, DffrPorts(c, d, q, r))

DEFINE_REGISTER_TEST(CELL_DFFE_PP, dffe_pp_reg, DffePorts(c, d, q, e))
DEFINE_REGISTER_TEST(CELL_DFFE_PN, dffe_pn_reg, DffePorts(c, d, q, e))
DEFINE_REGISTER_TEST(CELL_DFFE_NP, dffe_np_reg, DffePorts(c, d, q, e))
DEFINE_REGISTER_TEST(CELL_DFFE_NN, dffe_nn_reg, DffePorts(c, d, q, e))

DEFINE_REGISTER_TEST(CELL_DFFE_PP0P, dffe_pp0p_reg, DfferPorts(c, d, q, r, e))
DEFINE_REGISTER_TEST(CELL_DFFE_PP1P, dffe_pp1p_reg, DfferPorts(c, d, q, r, e))
DEFINE_REGISTER_TEST(CELL_DFFE_PN0P, dffe_pn0p_reg, DfferPorts(c, d, q, r, e))
DEFINE_REGISTER_TEST(CELL_DFFE_PN1P, dffe_pn1p_reg, DfferPorts(c, d, q, r, e))
DEFINE_REGISTER_TEST(CELL_DFFE_PP0N, dffe_pp0n_reg, DfferPorts(c, d, q, r, e))
DEFINE_REGISTER_TEST(CELL_DFFE_PP1N, dffe_pp1n_reg, DfferPorts(c, d, q, r, e))
DEFINE_REGISTER_TEST(CELL_DFFE_PN0N, dffe_pn0n_reg, DfferPorts(c, d, q, r, e))
DEFINE_REGISTER_TEST(CELL_DFFE_PN1N, dffe_pn1n_reg, DfferPorts(c, d, q, r, e))
DEFINE_REGISTER_TEST(CELL_DFFE_NP0P, dffe_np0p_reg, DfferPorts(c, d, q, r, e))
DEFINE_REGISTER_TEST(CELL_DFFE_NP1P, dffe_np1p_reg, DfferPorts(c, d, q, r, e))
DEFINE_REGISTER_TEST(CELL_DFFE_NN0P, dffe_nn0p_reg, DfferPorts(c, d, q, r, e))
DEFINE_REGISTER_TEST(CELL_DFFE_NN1P, dffe_nn1p_reg, DfferPorts(c, d, q, r, e))
DEFINE_REGISTER_TEST(CELL_DFFE_NP0N, dffe_np0n_reg, DfferPorts(c, d, q, r, e))
DEFINE_REGISTER_TEST(CELL_DFFE_NP1N, dffe_np1n_reg, DfferPorts(c, d, q, r, e))
DEFINE_REGISTER_TEST(CELL_DFFE_NN0N, dffe_nn0n_reg, DfferPorts(c, d, q, r, e))
DEFINE_REGISTER_TEST(CELL_DFFE_NN1N, dffe_nn1n_reg, DfferPorts(c, d, q, r, e))

int main(int argc, char* argv[])
{
    if (argc != 2) return 1;
    const std::string test_Name = argv[1];
    if (test_Name == STR(test_and_gate))
        test_and_gate();
    else if (test_Name == STR(test_or_gate))
        test_or_gate();
    else if (test_Name == STR(test_xor_gate))
        test_xor_gate();
    else if (test_Name == STR(test_nand_gate))
        test_nand_gate();
    else if (test_Name == STR(test_nor_gate))
        test_nor_gate();
    else if (test_Name == STR(test_xnor_gate))
        test_xnor_gate();
    else if (test_Name == STR(test_andnot_gate))
        test_andnot_gate();
    else if (test_Name == STR(test_ornot_gate))
        test_ornot_gate();
    else if (test_Name == STR(test_not_gate))
        test_not_gate();
    else if (test_Name == STR(test_buf_gate))
        test_buf_gate();
    else if (test_Name == STR(test_mux_gate))
        test_mux_gate();
    else if (test_Name == STR(test_nmux_gate))
        test_nmux_gate();
    else if (test_Name == STR(test_dff_p_reg))
        test_dff_p_reg();
    else if (test_Name == STR(test_dff_n_reg))
        test_dff_n_reg();
    else if (test_Name == STR(test_dff_pp0_reg))
        test_dff_pp0_reg();
    else if (test_Name == STR(test_dff_np0_reg))
        test_dff_np0_reg();
    else if (test_Name == STR(test_dff_pp1_reg))
        test_dff_pp1_reg();
    else if (test_Name == STR(test_dff_np1_reg))
        test_dff_np1_reg();
    else if (test_Name == STR(test_dff_pn0_reg))
        test_dff_pn0_reg();
    else if (test_Name == STR(test_dff_nn0_reg))
        test_dff_nn0_reg();
    else if (test_Name == STR(test_dff_pn1_reg))
        test_dff_pn1_reg();
    else if (test_Name == STR(test_dff_nn1_reg))
        test_dff_nn1_reg();
    else if (test_Name == STR(test_dffe_pp_reg))
        test_dffe_pp_reg();
    else if (test_Name == STR(test_dffe_pn_reg))
        test_dffe_pn_reg();
    else if (test_Name == STR(test_dffe_np_reg))
        test_dffe_np_reg();
    else if (test_Name == STR(test_dffe_nn_reg))
        test_dffe_nn_reg();
    else if (test_Name == STR(test_dffe_pp0p_reg))
        test_dffe_pp0p_reg();
    else if (test_Name == STR(test_dffe_pp1p_reg))
        test_dffe_pp1p_reg();
    else if (test_Name == STR(test_dffe_pn0p_reg))
        test_dffe_pn0p_reg();
    else if (test_Name == STR(test_dffe_pn1p_reg))
        test_dffe_pn1p_reg();
    else if (test_Name == STR(test_dffe_pp0n_reg))
        test_dffe_pp0n_reg();
    else if (test_Name == STR(test_dffe_pp1n_reg))
        test_dffe_pp1n_reg();
    else if (test_Name == STR(test_dffe_pn0n_reg))
        test_dffe_pn0n_reg();
    else if (test_Name == STR(test_dffe_pn1n_reg))
        test_dffe_pn1n_reg();
    else if (test_Name == STR(test_dffe_np0p_reg))
        test_dffe_np0p_reg();
    else if (test_Name == STR(test_dffe_np1p_reg))
        test_dffe_np1p_reg();
    else if (test_Name == STR(test_dffe_nn0p_reg))
        test_dffe_nn0p_reg();
    else if (test_Name == STR(test_dffe_nn1p_reg))
        test_dffe_nn1p_reg();
    else if (test_Name == STR(test_dffe_np0n_reg))
        test_dffe_np0n_reg();
    else if (test_Name == STR(test_dffe_np1n_reg))
        test_dffe_np1n_reg();
    else if (test_Name == STR(test_dffe_nn0n_reg))
        test_dffe_nn0n_reg();
    else if (test_Name == STR(test_dffe_nn1n_reg))
        test_dffe_nn1n_reg();
    else
        return 1;
    return 0;
}