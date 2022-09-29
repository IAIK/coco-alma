#include "test_util.h"
#include "Cell.h"
#include <unordered_map>
#include <iostream>

bool _and(bool a, bool b) { return a & b; }
bool _or(bool a, bool b) { return a | b; }
bool _xor(bool a, bool b) { return a ^ b; }

bool _nand(bool a, bool b) { return !(a & b); }
bool _nor(bool a, bool b) { return !(a | b); }
bool _xnor(bool a, bool b) { return !(a ^ b); }

bool _andnot(bool a, bool b) { return a & !b; }
bool _ornot(bool a, bool b) { return a | !b; }

bool _not(bool a) { return !a; }
bool _buf(bool a) { return a; }

#define DEFINE_BIN_TEST(CELL_TYPE, FUNCTION) \
void test ## FUNCTION() \
{ \
    std::unordered_map<signal_id_t, bool> prev_vals; \
    std::unordered_map<signal_id_t, bool> curr_vals; \
    auto a = static_cast<signal_id_t>(2); \
    auto b = static_cast<signal_id_t>(3); \
    auto y = static_cast<signal_id_t>(4); \
    curr_vals[a] = false; \
    curr_vals[b] = false; \
    Cell binary_cell(# FUNCTION, cell_type_t::CELL_TYPE, BinaryPorts(a, b, y)); \
    for (int input = 0; input < 4; input++) \
    { \
        bool val_a = (input >> 0) & 1; \
        bool val_b = (input >> 1) & 1; \
        bool val_y = FUNCTION(val_a, val_b); \
        curr_vals[a] = val_a; \
        curr_vals[b] = val_b; \
        curr_vals.erase(y); \
        binary_cell.eval<bool, bool&>(prev_vals, curr_vals); \
        std::cout << val_a << " " << #FUNCTION << " " << val_b << " = "; \
        std::cout << curr_vals[y] << " should be (" << val_y << ")" << std::endl; \
        assert(curr_vals[y] == val_y); \
    } \
}

DEFINE_BIN_TEST(CELL_AND,  _and);
DEFINE_BIN_TEST(CELL_OR,   _or);
DEFINE_BIN_TEST(CELL_XOR,  _xor);
DEFINE_BIN_TEST(CELL_NAND, _nand);
DEFINE_BIN_TEST(CELL_NOR,  _nor);
DEFINE_BIN_TEST(CELL_XNOR, _xnor);
DEFINE_BIN_TEST(CELL_ANDNOT, _andnot);
DEFINE_BIN_TEST(CELL_ORNOT,  _ornot);

#define DEFINE_UNR_TEST(CELL_TYPE, FUNCTION) \
void test ## FUNCTION() \
{ \
    std::unordered_map<signal_id_t, bool> prev_vals; \
    std::unordered_map<signal_id_t, bool> curr_vals; \
    auto a = static_cast<signal_id_t>(2); \
    auto y = static_cast<signal_id_t>(3); \
    curr_vals[a] = false; \
    Cell unary_cell(# FUNCTION, cell_type_t::CELL_TYPE, UnaryPorts(a, y)); \
    for (int input = 0; input < 2; input++) \
    { \
        bool val_a = (input >> 0) & 1; \
        bool val_y = FUNCTION(val_a); \
        curr_vals[a] = val_a; \
        curr_vals.erase(y); \
        unary_cell.eval<bool, bool&>(prev_vals, curr_vals); \
        std::cout << #FUNCTION << " " << val_a << " = "; \
        std::cout << curr_vals[y] << " should be (" << val_y << ")" << std::endl; \
        assert(curr_vals[y] == val_y); \
    } \
}

DEFINE_UNR_TEST(CELL_NOT, _not);
DEFINE_UNR_TEST(CELL_BUF, _buf);

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
    else
        return 1;
    return 0;
}