#include "test_util.h"
#include "Cell.h"
#include <unordered_map>
#include <iostream>

#define ignored __attribute__((unused))

bool _and(bool a, bool b, ignored bool s = false) { return a & b; }
bool _or(bool a, bool b, ignored bool s = false) { return a | b; }
bool _xor(bool a, bool b, ignored bool s = false) { return a ^ b; }

bool _nand(bool a, bool b, ignored bool s = false) { return !(a & b); }
bool _nor(bool a, bool b, ignored bool s = false) { return !(a | b); }
bool _xnor(bool a, bool b, ignored bool s = false) { return !(a ^ b); }

bool _andnot(bool a, bool b, ignored bool s = false) { return a & !b; }
bool _ornot(bool a, bool b, ignored bool s = false) { return a | !b; }

bool _not(bool a, ignored bool b = false, ignored bool s = false) { return !a; }
bool _buf(bool a, ignored bool b = false, ignored bool s = false) { return a; }

bool _mux(bool a, bool b, bool s) { return s ? b : a; }
bool _nmux(bool a, bool b, bool s) { return s ? !b : !a; }

#define DEFINE_GATE_TEST(CELL_TYPE, FUNCTION, PORTS) \
void test ## FUNCTION() \
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

DEFINE_GATE_TEST(CELL_AND,    _and,    BinaryPorts(a, b, y))
DEFINE_GATE_TEST(CELL_OR,     _or,     BinaryPorts(a, b, y))
DEFINE_GATE_TEST(CELL_XOR,    _xor,    BinaryPorts(a, b, y))
DEFINE_GATE_TEST(CELL_NAND,   _nand,   BinaryPorts(a, b, y))
DEFINE_GATE_TEST(CELL_NOR,    _nor,    BinaryPorts(a, b, y))
DEFINE_GATE_TEST(CELL_XNOR,   _xnor,   BinaryPorts(a, b, y))
DEFINE_GATE_TEST(CELL_ANDNOT, _andnot, BinaryPorts(a, b, y))
DEFINE_GATE_TEST(CELL_ORNOT,  _ornot,  BinaryPorts(a, b, y))

DEFINE_GATE_TEST(CELL_NOT, _not, UnaryPorts(a, y))
DEFINE_GATE_TEST(CELL_BUF, _buf, UnaryPorts(a, y))

DEFINE_GATE_TEST(CELL_MUX,  _mux,  MultiplexerPorts(a, b, s, y))
DEFINE_GATE_TEST(CELL_NMUX, _nmux, MultiplexerPorts(a, b, s, y))

bool _dff_p(bool d, ignored bool q, ignored bool r = false, ignored bool e = false) { return d; }
#define _dff_n _dff_p

bool _dff_pp0(bool d, ignored bool q, bool r, ignored bool e = false) { return r == true  ? false : d; }
bool _dff_pp1(bool d, ignored bool q, bool r, ignored bool e = false) { return r == true  ? true  : d; }
bool _dff_pn0(bool d, ignored bool q, bool r, ignored bool e = false) { return r == false ? false : d; }
bool _dff_pn1(bool d, ignored bool q, bool r, ignored bool e = false) { return r == false ? true  : d; }
#define _dff_np0 _dff_pp0
#define _dff_np1 _dff_pp1
#define _dff_nn0 _dff_pn0
#define _dff_nn1 _dff_pn1

bool _dffe_pp(bool d, bool q, ignored bool r, bool e) { return e == true ? d : q; }
bool _dffe_pn(bool d, bool q, ignored bool r, bool e) { return e == false ? d : q; }
#define _dffe_np _dffe_pp
#define _dffe_nn _dffe_pn

bool _dffe_pp0p(bool d, bool q, bool r, bool e) { return r == true  ? false : (e == true ? d : q); }
bool _dffe_pp1p(bool d, bool q, bool r, bool e) { return r == true  ? true  : (e == true ? d : q); }
bool _dffe_pn0p(bool d, bool q, bool r, bool e) { return r == false ? false : (e == true ? d : q); }
bool _dffe_pn1p(bool d, bool q, bool r, bool e) { return r == false ? true  : (e == true ? d : q); }
bool _dffe_pp0n(bool d, bool q, bool r, bool e) { return r == true  ? false : (e == false ? d : q); }
bool _dffe_pp1n(bool d, bool q, bool r, bool e) { return r == true  ? true  : (e == false ? d : q); }
bool _dffe_pn0n(bool d, bool q, bool r, bool e) { return r == false ? false : (e == false ? d : q); }
bool _dffe_pn1n(bool d, bool q, bool r, bool e) { return r == false ? true  : (e == false ? d : q); }
#define _dffe_np0p _dffe_pp0p
#define _dffe_np1p _dffe_pp1p
#define _dffe_nn0p _dffe_pn0p
#define _dffe_nn1p _dffe_pn1p
#define _dffe_np0n _dffe_pp0n
#define _dffe_np1n _dffe_pp1n
#define _dffe_nn0n _dffe_pn0n
#define _dffe_nn1n _dffe_pn1n

#define DEFINE_REGISTER_TEST(CELL_TYPE, FUNCTION, PORTS) \
void test ## FUNCTION() \
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

DEFINE_REGISTER_TEST(CELL_DFF_P, _dff_p, DffPorts(c, d, q))
DEFINE_REGISTER_TEST(CELL_DFF_N, _dff_n, DffPorts(c, d, q))

DEFINE_REGISTER_TEST(CELL_DFF_PP0, _dff_pp0, DffrPorts(c, d, q, r))
DEFINE_REGISTER_TEST(CELL_DFF_PP1, _dff_pp1, DffrPorts(c, d, q, r))
DEFINE_REGISTER_TEST(CELL_DFF_PN0, _dff_pn0, DffrPorts(c, d, q, r))
DEFINE_REGISTER_TEST(CELL_DFF_PN1, _dff_pn1, DffrPorts(c, d, q, r))
DEFINE_REGISTER_TEST(CELL_DFF_NP0, _dff_np0, DffrPorts(c, d, q, r))
DEFINE_REGISTER_TEST(CELL_DFF_NP1, _dff_np1, DffrPorts(c, d, q, r))
DEFINE_REGISTER_TEST(CELL_DFF_NN0, _dff_nn0, DffrPorts(c, d, q, r))
DEFINE_REGISTER_TEST(CELL_DFF_NN1, _dff_nn1, DffrPorts(c, d, q, r))

DEFINE_REGISTER_TEST(CELL_DFFE_PP, _dffe_pp, DffePorts(c, d, q, e))
DEFINE_REGISTER_TEST(CELL_DFFE_PN, _dffe_pn, DffePorts(c, d, q, e))
DEFINE_REGISTER_TEST(CELL_DFFE_NP, _dffe_np, DffePorts(c, d, q, e))
DEFINE_REGISTER_TEST(CELL_DFFE_NN, _dffe_nn, DffePorts(c, d, q, e))

DEFINE_REGISTER_TEST(CELL_DFFE_PP0P, _dffe_pp0p, DfferPorts(c, d, q, r, e))
DEFINE_REGISTER_TEST(CELL_DFFE_PP1P, _dffe_pp1p, DfferPorts(c, d, q, r, e))
DEFINE_REGISTER_TEST(CELL_DFFE_PN0P, _dffe_pn0p, DfferPorts(c, d, q, r, e))
DEFINE_REGISTER_TEST(CELL_DFFE_PN1P, _dffe_pn1p, DfferPorts(c, d, q, r, e))
DEFINE_REGISTER_TEST(CELL_DFFE_PP0N, _dffe_pp0n, DfferPorts(c, d, q, r, e))
DEFINE_REGISTER_TEST(CELL_DFFE_PP1N, _dffe_pp1n, DfferPorts(c, d, q, r, e))
DEFINE_REGISTER_TEST(CELL_DFFE_PN0N, _dffe_pn0n, DfferPorts(c, d, q, r, e))
DEFINE_REGISTER_TEST(CELL_DFFE_PN1N, _dffe_pn1n, DfferPorts(c, d, q, r, e))
DEFINE_REGISTER_TEST(CELL_DFFE_NP0P, _dffe_np0p, DfferPorts(c, d, q, r, e))
DEFINE_REGISTER_TEST(CELL_DFFE_NP1P, _dffe_np1p, DfferPorts(c, d, q, r, e))
DEFINE_REGISTER_TEST(CELL_DFFE_NN0P, _dffe_nn0p, DfferPorts(c, d, q, r, e))
DEFINE_REGISTER_TEST(CELL_DFFE_NN1P, _dffe_nn1p, DfferPorts(c, d, q, r, e))
DEFINE_REGISTER_TEST(CELL_DFFE_NP0N, _dffe_np0n, DfferPorts(c, d, q, r, e))
DEFINE_REGISTER_TEST(CELL_DFFE_NP1N, _dffe_np1n, DfferPorts(c, d, q, r, e))
DEFINE_REGISTER_TEST(CELL_DFFE_NN0N, _dffe_nn0n, DfferPorts(c, d, q, r, e))
DEFINE_REGISTER_TEST(CELL_DFFE_NN1N, _dffe_nn1n, DfferPorts(c, d, q, r, e))

int main(int argc, char* argv[])
{
    if (argc != 2) return 1;
    const std::string test_name = argv[1];
    if (test_name == STR(test_and))
        test_and();
    else if (test_name == STR(test_or))
        test_or();
    else if (test_name == STR(test_xor))
        test_xor();
    else if (test_name == STR(test_nand))
        test_nand();
    else if (test_name == STR(test_nor))
        test_nor();
    else if (test_name == STR(test_xnor))
        test_xnor();
    else if (test_name == STR(test_andnot))
        test_andnot();
    else if (test_name == STR(test_ornot))
        test_ornot();
    else if (test_name == STR(test_not))
        test_not();
    else if (test_name == STR(test_buf))
        test_buf();
    else if (test_name == STR(test_mux))
        test_mux();
    else if (test_name == STR(test_nmux))
        test_nmux();
    else if (test_name == STR(test_dff_p))
        test_dff_p();
    else if (test_name == STR(test_dff_n))
        test_dff_n();
    else if (test_name == STR(test_dff_pp0))
        test_dff_pp0();
    else if (test_name == STR(test_dff_np0))
        test_dff_np0();
    else if (test_name == STR(test_dff_pp1))
        test_dff_pp1();
    else if (test_name == STR(test_dff_np1))
        test_dff_np1();
    else if (test_name == STR(test_dff_pn0))
        test_dff_pn0();
    else if (test_name == STR(test_dff_nn0))
        test_dff_nn0();
    else if (test_name == STR(test_dff_pn1))
        test_dff_pn1();
    else if (test_name == STR(test_dff_nn1))
        test_dff_nn1();
    else if (test_name == STR(test_dffe_pp))
        test_dffe_pp();
    else if (test_name == STR(test_dffe_pn))
        test_dffe_pn();
    else if (test_name == STR(test_dffe_np))
        test_dffe_np();
    else if (test_name == STR(test_dffe_nn))
        test_dffe_nn();
    else if (test_name == STR(test_dffe_pp0p))
        test_dffe_pp0p();
    else if (test_name == STR(test_dffe_pp1p))
        test_dffe_pp1p();
    else if (test_name == STR(test_dffe_pn0p))
        test_dffe_pn0p();
    else if (test_name == STR(test_dffe_pn1p))
        test_dffe_pn1p();
    else if (test_name == STR(test_dffe_pp0n))
        test_dffe_pp0n();
    else if (test_name == STR(test_dffe_pp1n))
        test_dffe_pp1n();
    else if (test_name == STR(test_dffe_pn0n))
        test_dffe_pn0n();
    else if (test_name == STR(test_dffe_pn1n))
        test_dffe_pn1n();
    else if (test_name == STR(test_dffe_np0p))
        test_dffe_np0p();
    else if (test_name == STR(test_dffe_np1p))
        test_dffe_np1p();
    else if (test_name == STR(test_dffe_nn0p))
        test_dffe_nn0p();
    else if (test_name == STR(test_dffe_nn1p))
        test_dffe_nn1p();
    else if (test_name == STR(test_dffe_np0n))
        test_dffe_np0n();
    else if (test_name == STR(test_dffe_np1n))
        test_dffe_np1n();
    else if (test_name == STR(test_dffe_nn0n))
        test_dffe_nn0n();
    else if (test_name == STR(test_dffe_nn1n))
        test_dffe_nn1n();
    else
        return 1;
    return 0;
}