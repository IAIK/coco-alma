#ifndef CELL_TYPES_H
#define CELL_TYPES_H


/// Enum class for cell types
/// The cell types are encoded as follows:
/// [11] is multiplexer
/// [10] is register
///  [9] is gate with single input
///  [8] is gate with two inputs
///  [7] is output of gate negated
///  [6] is B (second) input port negated
///  [5] is R (reset)  input port enabled
///  [4] is E (enable) input port enabled
/// Gates: [1:0] gate type
/// Registers: [3] clock edge that triggers advance
///            [2] value that triggers reset
///            [1] value given on reset
///            [0] value that enables register write
///
enum class cell_type_t : uint32_t
{
    /* standard gates    */    CELL_AND = 0x100,    CELL_OR = 0x101,  CELL_XOR = 0x102,
    /* output negated    */   CELL_NAND = 0x180,   CELL_NOR = 0x181, CELL_XNOR = 0x182,
    /* second negated    */ CELL_ANDNOT = 0x140, CELL_ORNOT = 0x141, /* no such xor */
    /* single input      */   CELL_BUF = 0x200,   CELL_NOT = 0x280,
    /* normal register   */ CELL_DFF_N = 0x400, CELL_DFF_P = 0x408,
    /* normal registers that have a reset signal */
    CELL_DFF_NN0 = 0x420, CELL_DFF_NN1 = 0x422, CELL_DFF_NP0 = 0x424, CELL_DFF_NP1 = 0x426,
    CELL_DFF_PN0 = 0x428, CELL_DFF_PN1 = 0x42a, CELL_DFF_PP0 = 0x42c, CELL_DFF_PP1 = 0x42e,
    /* normal registers that have a write-enable signal */
    CELL_DFFE_NN = 0x410, CELL_DFFE_NP = 0x411, CELL_DFFE_PN = 0x418, CELL_DFFE_PP = 0x419,
    /* registers that have both reset and write-enable signals */
    CELL_DFFE_NN0N = 0x430, CELL_DFFE_NN1N = 0x432, CELL_DFFE_NP0N = 0x434, CELL_DFFE_NP1N = 0x436,
    CELL_DFFE_PN0N = 0x438, CELL_DFFE_PN1N = 0x43a, CELL_DFFE_PP0N = 0x43c, CELL_DFFE_PP1N = 0x43e,
    CELL_DFFE_NN0P = 0x431, CELL_DFFE_NN1P = 0x433, CELL_DFFE_NP0P = 0x435, CELL_DFFE_NP1P = 0x437,
    CELL_DFFE_PN0P = 0x439, CELL_DFFE_PN1P = 0x43b, CELL_DFFE_PP0P = 0x43d, CELL_DFFE_PP1P = 0x43f,
    /* multiplexer gates */ CELL_MUX = 0x800, CELL_NMUX = 0x880
};

constexpr bool is_multiplexer(cell_type_t x)
{ return static_cast<uint32_t>(x) & 0x800; }

constexpr bool is_register(cell_type_t x)
{ return static_cast<uint32_t>(x) & 0x400; }

constexpr bool is_unary(cell_type_t x)
{ return static_cast<uint32_t>(x) & 0x200; }

constexpr bool is_binary(cell_type_t x)
{ return static_cast<uint32_t>(x) & 0x100; }

constexpr bool is_out_negated(cell_type_t x)
{ return static_cast<uint32_t>(x) & 0x080; }

constexpr bool is_second_negated(cell_type_t x)
{ return static_cast<uint32_t>(x) & 0x040; }

constexpr bool dff_has_reset(cell_type_t x)
{ return static_cast<uint32_t>(x) & 0x020; }

constexpr bool dff_has_enable(cell_type_t x)
{ return static_cast<uint32_t>(x) & 0x010; }

constexpr bool dff_enable_trigger(cell_type_t x)
{ return static_cast<uint32_t>(x) & 0x001; }

constexpr bool dff_reset_value(cell_type_t x)
{ return static_cast<uint32_t>(x) & 0x002; }

constexpr bool dff_reset_trigger(cell_type_t x)
{ return static_cast<uint32_t>(x) & 0x004; }

constexpr bool dff_clock_trigger(cell_type_t x)
{ return static_cast<uint32_t>(x) & 0x008; }

constexpr bool gate_is_like_and(cell_type_t x)
{ return (static_cast<uint32_t>(x) & 0x3) == 0U; }

constexpr bool gate_is_like_or(cell_type_t x)
{ return (static_cast<uint32_t>(x) & 0x3) == 1U; }

constexpr bool gate_is_like_xor(cell_type_t x)
{ return (static_cast<uint32_t>(x) & 0x3) == 2U; }

constexpr bool is_well_formed(cell_type_t x)
{
    // only one of these is active
    const bool onehot = (1 == ((uint32_t)is_binary(x) + (uint32_t)is_unary(x) +
                               (uint32_t)is_register(x) + (uint32_t)is_multiplexer(x)));
    const auto u_x = static_cast<uint32_t>(x);
    // if it is a register, 0xf3f mask must hold
    const bool dff_ok = (!is_register(x)    || ((u_x & 0xf3f) == u_x));
    // if it is a binary gate, 0xfc3 mask must hold
    const bool bin_ok = (!is_binary(x)      || ((u_x & 0xfc3) == u_x));
    // if it is a unary gate, 0xf80 mask must hold
    const bool unr_ok = (!is_unary(x)       || ((u_x & 0xf80) == u_x));
    // if it is a multiplexer, 0xf80 mask must hold
    const bool mux_ok = (!is_multiplexer(x) || ((u_x & 0xf80) == u_x));
    return onehot && dff_ok && bin_ok && unr_ok && mux_ok;
}

#define like_and_text(gate_type) #gate_type " must be like an AND"
#define like_or_text(gate_type)  #gate_type " must be like an OR"
#define like_xor_text(gate_type) #gate_type " must be like an XOR"

constexpr bool test_is_binary_normal_gate(cell_type_t x)
{
    return is_well_formed(x) && is_binary(x) && !is_out_negated(x) && !is_second_negated(x);
}

#define binary_normal_text(gate_type) #gate_type " must be binary normal gate"

// Static tests for standard gates
static_assert(test_is_binary_normal_gate(cell_type_t::CELL_AND), binary_normal_text(cell_type_t::CELL_AND));
static_assert(gate_is_like_and(cell_type_t::CELL_AND), like_and_text(cell_type_t::CELL_AND));
static_assert(test_is_binary_normal_gate(cell_type_t::CELL_OR), binary_normal_text(cell_type_t::CELL_OR));
static_assert(gate_is_like_or(cell_type_t::CELL_OR), like_or_text(cell_type_t::CELL_OR));
static_assert(test_is_binary_normal_gate(cell_type_t::CELL_XOR), binary_normal_text(cell_type_t::CELL_XOR));
static_assert(gate_is_like_xor(cell_type_t::CELL_XOR), like_xor_text(cell_type_t::CELL_XOR));

constexpr bool test_is_binary_output_negated_gate(cell_type_t x)
{
    return is_well_formed(x) && is_binary(x) && is_out_negated(x) && !is_second_negated(x);
}

#define binary_output_negated_text(gate_type) #gate_type " must be binary gate with negated output"

// Static tests for standard gates with negated outputs
static_assert(test_is_binary_output_negated_gate(cell_type_t::CELL_NAND), binary_output_negated_text(cell_type_t::CELL_NAND));
static_assert(gate_is_like_and(cell_type_t::CELL_NAND), like_and_text(cell_type_t::CELL_NAND));
static_assert(test_is_binary_output_negated_gate(cell_type_t::CELL_NOR), binary_output_negated_text(cell_type_t::CELL_NOR));
static_assert(gate_is_like_or(cell_type_t::CELL_NOR), like_or_text(cell_type_t::CELL_NOR));
static_assert(test_is_binary_output_negated_gate(cell_type_t::CELL_XNOR), binary_output_negated_text(cell_type_t::CELL_XNOR));
static_assert(gate_is_like_xor(cell_type_t::CELL_XNOR), like_xor_text(cell_type_t::CELL_XNOR));

constexpr bool test_is_binary_second_negated_gate(cell_type_t x)
{
    return is_well_formed(x) && is_binary(x) && !is_out_negated(x) && is_second_negated(x);
}

#define binary_second_negated_text(gate_type) #gate_type " must be binary gate with negated second input"

// Static tests for standard gates with negated second operand
static_assert(test_is_binary_second_negated_gate(cell_type_t::CELL_ANDNOT), binary_second_negated_text(cell_type_t::CELL_ANDNOT));
static_assert(gate_is_like_and(cell_type_t::CELL_ANDNOT), like_and_text(cell_type_t::CELL_ANDNOT));
static_assert(test_is_binary_second_negated_gate(cell_type_t::CELL_ORNOT), binary_second_negated_text(cell_type_t::CELL_ORNOT));
static_assert(gate_is_like_or(cell_type_t::CELL_ORNOT), like_or_text(cell_type_t::CELL_ORNOT));

constexpr bool test_is_unary(cell_type_t x)
{
    return is_well_formed(x) && is_unary(x);
}

// Static tests for single input gates
static_assert(test_is_unary(cell_type_t::CELL_BUF) && !is_out_negated(cell_type_t::CELL_BUF),
              "cell_type_t::CELL_BUF must be unary gate that does not negate output");
static_assert(test_is_unary(cell_type_t::CELL_NOT) && is_out_negated(cell_type_t::CELL_NOT),
              "cell_type_t::CELL_NOT must be unary gate that negates its output");

constexpr bool test_is_multiplexer(cell_type_t x)
{
    return is_well_formed(x) && is_multiplexer(x);
}

// Static tests for multiplexer gates
static_assert(test_is_multiplexer(cell_type_t::CELL_MUX) && !is_out_negated(cell_type_t::CELL_MUX),
              "cell_type_t::CELL_MUX must be a multiplexer that does not negate output");
static_assert(test_is_multiplexer(cell_type_t::CELL_NMUX) && is_out_negated(cell_type_t::CELL_NMUX),
              "cell_type_t::CELL_NMUX must be a multiplexer that negates its output");

constexpr bool test_is_normal_reg(cell_type_t x)
{
    return is_well_formed(x) && is_register(x) && !dff_has_enable(x) && !dff_has_reset(x);
}

// Static tests for normal registers
static_assert(test_is_normal_reg(cell_type_t::CELL_DFF_N) && !dff_clock_trigger(cell_type_t::CELL_DFF_N),
        "cell_type_t::CELL_DFF_N must be normal register triggered with negative clock");
static_assert(test_is_normal_reg(cell_type_t::CELL_DFF_P) &&  dff_clock_trigger(cell_type_t::CELL_DFF_P),
        "cell_type_t::CELL_DFF_P must be normal register triggered with positive clock");

constexpr bool test_is_reg_with_enable(cell_type_t x)
{
    return is_well_formed(x) && is_register(x) && dff_has_enable(x) && !dff_has_reset(x);
}

#define reg_with_enable_text(reg_type, c, e) #reg_type " is a register with enable that is triggered with " \
    c " clock and written when enable is " e

// Static tests for registers with enable
static_assert(test_is_reg_with_enable(cell_type_t::CELL_DFFE_NN) &&
              !dff_clock_trigger(cell_type_t::CELL_DFFE_NN) &&
              !dff_enable_trigger(cell_type_t::CELL_DFFE_NN),
              reg_with_enable_text(cell_type_t::CELL_DFFE_NN, "negative", "low"));
static_assert(test_is_reg_with_enable(cell_type_t::CELL_DFFE_NP) &&
              !dff_clock_trigger(cell_type_t::CELL_DFFE_NP) &&
              dff_enable_trigger(cell_type_t::CELL_DFFE_NP),
              reg_with_enable_text(cell_type_t::CELL_DFFE_NP, "negative", "high"));
static_assert(test_is_reg_with_enable(cell_type_t::CELL_DFFE_PN) &&
              dff_clock_trigger(cell_type_t::CELL_DFFE_PN) &&
              !dff_enable_trigger(cell_type_t::CELL_DFFE_PN),
              reg_with_enable_text(cell_type_t::CELL_DFFE_PN, "positive", "low"));
static_assert(test_is_reg_with_enable(cell_type_t::CELL_DFFE_PP) &&
              dff_clock_trigger(cell_type_t::CELL_DFFE_PP) &&
              dff_enable_trigger(cell_type_t::CELL_DFFE_PP),
              reg_with_enable_text(cell_type_t::CELL_DFFE_PP, "positive", "high"));

constexpr bool test_is_reg_with_reset(cell_type_t x)
{
    return is_well_formed(x) && is_register(x) && dff_has_reset(x) && !dff_has_enable(x);
}

#define reg_with_reset_text(reg_type, c, r, v) #reg_type " is a register with reset that is triggered with " \
    c " clock, reset to " #v " when reset is " r

// Static tests for registers with resets
static_assert(test_is_reg_with_reset(cell_type_t::CELL_DFF_NN0) &&
              !dff_clock_trigger(cell_type_t::CELL_DFF_NN0) &&
              !dff_reset_trigger(cell_type_t::CELL_DFF_NN0) &&
              !dff_reset_value(cell_type_t::CELL_DFF_NN0),
              reg_with_reset_text(cell_type_t::CELL_DFF_NN0, "negative", "low", 0));
static_assert(test_is_reg_with_reset(cell_type_t::CELL_DFF_NN1) &&
              !dff_clock_trigger(cell_type_t::CELL_DFF_NN1) &&
              !dff_reset_trigger(cell_type_t::CELL_DFF_NN1) &&
              dff_reset_value(cell_type_t::CELL_DFF_NN1),
              reg_with_reset_text(cell_type_t::CELL_DFF_NN1, "negative", "low", 1));
static_assert(test_is_reg_with_reset(cell_type_t::CELL_DFF_NP0) &&
              !dff_clock_trigger(cell_type_t::CELL_DFF_NP0) &&
              dff_reset_trigger(cell_type_t::CELL_DFF_NP0) &&
              !dff_reset_value(cell_type_t::CELL_DFF_NP0),
              reg_with_reset_text(cell_type_t::CELL_DFF_NP0, "negative", "high", 0));
static_assert(test_is_reg_with_reset(cell_type_t::CELL_DFF_NP1) &&
              !dff_clock_trigger(cell_type_t::CELL_DFF_NP1) &&
              dff_reset_trigger(cell_type_t::CELL_DFF_NP1) &&
              dff_reset_value(cell_type_t::CELL_DFF_NP1),
              reg_with_reset_text(cell_type_t::CELL_DFF_NP1, "negative", "high", 1));
static_assert(test_is_reg_with_reset(cell_type_t::CELL_DFF_PN0) &&
              dff_clock_trigger(cell_type_t::CELL_DFF_PN0) &&
              !dff_reset_trigger(cell_type_t::CELL_DFF_PN0) &&
              !dff_reset_value(cell_type_t::CELL_DFF_PN0),
              reg_with_reset_text(cell_type_t::CELL_DFF_PN0, "positive", "low", 0));
static_assert(test_is_reg_with_reset(cell_type_t::CELL_DFF_PN1) &&
              dff_clock_trigger(cell_type_t::CELL_DFF_PN1) &&
              !dff_reset_trigger(cell_type_t::CELL_DFF_PN1) &&
              dff_reset_value(cell_type_t::CELL_DFF_PN1),
              reg_with_reset_text(cell_type_t::CELL_DFF_PN1, "positive", "low", 1));
static_assert(test_is_reg_with_reset(cell_type_t::CELL_DFF_PP0) &&
              dff_clock_trigger(cell_type_t::CELL_DFF_PP0) &&
              dff_reset_trigger(cell_type_t::CELL_DFF_PP0) &&
              !dff_reset_value(cell_type_t::CELL_DFF_PP0),
              reg_with_reset_text(cell_type_t::CELL_DFF_PP0, "positive", "high", 0));
static_assert(test_is_reg_with_reset(cell_type_t::CELL_DFF_PP1) &&
              dff_clock_trigger(cell_type_t::CELL_DFF_PP1) &&
              dff_reset_trigger(cell_type_t::CELL_DFF_PP1) &&
              dff_reset_value(cell_type_t::CELL_DFF_PP1),
              reg_with_reset_text(cell_type_t::CELL_DFF_PP1, "positive", "high", 1));

constexpr bool test_is_reg_with_reset_and_enable(cell_type_t x)
{
    return is_well_formed(x) && is_register(x) && dff_has_reset(x) && dff_has_enable(x);
}

#define reg_with_reset_and_enable_text(reg_type, c, r, v, e) #reg_type " is a register with reset and enable "\
    "that is triggered with " c " clock, reset to " #v " when reset is " r " and otherwise only written if"   \
    "enable is " e

// Static tests for registers with reset and enable
static_assert(test_is_reg_with_reset_and_enable(cell_type_t::CELL_DFFE_NN0N) &&
              !dff_clock_trigger(cell_type_t::CELL_DFFE_NN0N) &&
              !dff_reset_trigger(cell_type_t::CELL_DFFE_NN0N) &&
              !dff_reset_value(cell_type_t::CELL_DFFE_NN0N) &&
              !dff_enable_trigger(cell_type_t::CELL_DFFE_NN0N),
              reg_with_reset_and_enable_text(cell_type_t::CELL_DFFE_NN0N, "negative", "low", 0, "low"));
static_assert(test_is_reg_with_reset_and_enable(cell_type_t::CELL_DFFE_NN1N) &&
              !dff_clock_trigger(cell_type_t::CELL_DFFE_NN1N) &&
              !dff_reset_trigger(cell_type_t::CELL_DFFE_NN1N) &&
              dff_reset_value(cell_type_t::CELL_DFFE_NN1N) &&
              !dff_enable_trigger(cell_type_t::CELL_DFFE_NN1N),
              reg_with_reset_and_enable_text(cell_type_t::CELL_DFFE_NN1N, "negative", "low", 1, "low"));
static_assert(test_is_reg_with_reset_and_enable(cell_type_t::CELL_DFFE_NP0N) &&
              !dff_clock_trigger(cell_type_t::CELL_DFFE_NP0N) &&
              dff_reset_trigger(cell_type_t::CELL_DFFE_NP0N) &&
              !dff_reset_value(cell_type_t::CELL_DFFE_NP0N) &&
              !dff_enable_trigger(cell_type_t::CELL_DFFE_NP0N),
              reg_with_reset_and_enable_text(cell_type_t::CELL_DFFE_NP0N, "negative", "high", 0, "low"));
static_assert(test_is_reg_with_reset_and_enable(cell_type_t::CELL_DFFE_NP1N) &&
              !dff_clock_trigger(cell_type_t::CELL_DFFE_NP1N) &&
              dff_reset_trigger(cell_type_t::CELL_DFFE_NP1N) &&
              dff_reset_value(cell_type_t::CELL_DFFE_NP1N) &&
              !dff_enable_trigger(cell_type_t::CELL_DFFE_NP1N),
              reg_with_reset_and_enable_text(cell_type_t::CELL_DFFE_NP1N, "negative", "high", 1, "low"));
static_assert(test_is_reg_with_reset_and_enable(cell_type_t::CELL_DFFE_PN0N) &&
              dff_clock_trigger(cell_type_t::CELL_DFFE_PN0N) &&
              !dff_reset_trigger(cell_type_t::CELL_DFFE_PN0N) &&
              !dff_reset_value(cell_type_t::CELL_DFFE_PN0N) &&
              !dff_enable_trigger(cell_type_t::CELL_DFFE_PN0N),
              reg_with_reset_and_enable_text(cell_type_t::CELL_DFFE_PN0N, "positive", "low", 0, "low"));
static_assert(test_is_reg_with_reset_and_enable(cell_type_t::CELL_DFFE_PN1N) &&
              dff_clock_trigger(cell_type_t::CELL_DFFE_PN1N) &&
              !dff_reset_trigger(cell_type_t::CELL_DFFE_PN1N) &&
              dff_reset_value(cell_type_t::CELL_DFFE_PN1N) &&
              !dff_enable_trigger(cell_type_t::CELL_DFFE_PN1N),
              reg_with_reset_and_enable_text(cell_type_t::CELL_DFFE_PN1N, "positive", "low", 1, "low"));
static_assert(test_is_reg_with_reset_and_enable(cell_type_t::CELL_DFFE_PP0N) &&
              dff_clock_trigger(cell_type_t::CELL_DFFE_PP0N) &&
              dff_reset_trigger(cell_type_t::CELL_DFFE_PP0N) &&
              !dff_reset_value(cell_type_t::CELL_DFFE_PP0N) &&
              !dff_enable_trigger(cell_type_t::CELL_DFFE_PP0N),
              reg_with_reset_and_enable_text(cell_type_t::CELL_DFFE_PP0N, "positive", "high", 0, "low"));
static_assert(test_is_reg_with_reset_and_enable(cell_type_t::CELL_DFFE_PP1N) &&
              dff_clock_trigger(cell_type_t::CELL_DFFE_PP1N) &&
              dff_reset_trigger(cell_type_t::CELL_DFFE_PP1N) &&
              dff_reset_value(cell_type_t::CELL_DFFE_PP1N) &&
              !dff_enable_trigger(cell_type_t::CELL_DFFE_PP1N),
              reg_with_reset_and_enable_text(cell_type_t::CELL_DFFE_PP1N, "positive", "high", 1, "low"));

static_assert(test_is_reg_with_reset_and_enable(cell_type_t::CELL_DFFE_NN0P) &&
              !dff_clock_trigger(cell_type_t::CELL_DFFE_NN0P) &&
              !dff_reset_trigger(cell_type_t::CELL_DFFE_NN0P) &&
              !dff_reset_value(cell_type_t::CELL_DFFE_NN0P) &&
              dff_enable_trigger(cell_type_t::CELL_DFFE_NN0P),
              reg_with_reset_and_enable_text(cell_type_t::CELL_DFFE_NN0P, "negative", "low", 0, "high"));
static_assert(test_is_reg_with_reset_and_enable(cell_type_t::CELL_DFFE_NN1P) &&
              !dff_clock_trigger(cell_type_t::CELL_DFFE_NN1P) &&
              !dff_reset_trigger(cell_type_t::CELL_DFFE_NN1P) &&
              dff_reset_value(cell_type_t::CELL_DFFE_NN1P) &&
              dff_enable_trigger(cell_type_t::CELL_DFFE_NN1P),
              reg_with_reset_and_enable_text(cell_type_t::CELL_DFFE_NN1P, "negative", "low", 1, "high"));
static_assert(test_is_reg_with_reset_and_enable(cell_type_t::CELL_DFFE_NP0P) &&
              !dff_clock_trigger(cell_type_t::CELL_DFFE_NP0P) &&
              dff_reset_trigger(cell_type_t::CELL_DFFE_NP0P) &&
              !dff_reset_value(cell_type_t::CELL_DFFE_NP0P) &&
              dff_enable_trigger(cell_type_t::CELL_DFFE_NP0P),
              reg_with_reset_and_enable_text(cell_type_t::CELL_DFFE_NP0P, "negative", "high", 0, "high"));
static_assert(test_is_reg_with_reset_and_enable(cell_type_t::CELL_DFFE_NP1P) &&
              !dff_clock_trigger(cell_type_t::CELL_DFFE_NP1P) &&
              dff_reset_trigger(cell_type_t::CELL_DFFE_NP1P) &&
              dff_reset_value(cell_type_t::CELL_DFFE_NP1P) &&
              dff_enable_trigger(cell_type_t::CELL_DFFE_NP1P),
              reg_with_reset_and_enable_text(cell_type_t::CELL_DFFE_NP1P, "negative", "high", 1, "high"));
static_assert(test_is_reg_with_reset_and_enable(cell_type_t::CELL_DFFE_PN0P) &&
              dff_clock_trigger(cell_type_t::CELL_DFFE_PN0P) &&
              !dff_reset_trigger(cell_type_t::CELL_DFFE_PN0P) &&
              !dff_reset_value(cell_type_t::CELL_DFFE_PN0P) &&
              dff_enable_trigger(cell_type_t::CELL_DFFE_PN0P),
              reg_with_reset_and_enable_text(cell_type_t::CELL_DFFE_PN0P, "positive", "low", 0, "high"));
static_assert(test_is_reg_with_reset_and_enable(cell_type_t::CELL_DFFE_PN1P) &&
              dff_clock_trigger(cell_type_t::CELL_DFFE_PN1P) &&
              !dff_reset_trigger(cell_type_t::CELL_DFFE_PN1P) &&
              dff_reset_value(cell_type_t::CELL_DFFE_PN1P) &&
              dff_enable_trigger(cell_type_t::CELL_DFFE_PN1P),
              reg_with_reset_and_enable_text(cell_type_t::CELL_DFFE_PN1P, "positive", "low", 1, "high"));
static_assert(test_is_reg_with_reset_and_enable(cell_type_t::CELL_DFFE_PP0P) &&
              dff_clock_trigger(cell_type_t::CELL_DFFE_PP0P) &&
              dff_reset_trigger(cell_type_t::CELL_DFFE_PP0P) &&
              !dff_reset_value(cell_type_t::CELL_DFFE_PP0P) &&
              dff_enable_trigger(cell_type_t::CELL_DFFE_PP0P),
              reg_with_reset_and_enable_text(cell_type_t::CELL_DFFE_PP0P, "positive", "high", 0, "high"));
static_assert(test_is_reg_with_reset_and_enable(cell_type_t::CELL_DFFE_PP1P) &&
              dff_clock_trigger(cell_type_t::CELL_DFFE_PP1P) &&
              dff_reset_trigger(cell_type_t::CELL_DFFE_PP1P) &&
              dff_reset_value(cell_type_t::CELL_DFFE_PP1P) &&
              dff_enable_trigger(cell_type_t::CELL_DFFE_PP1P),
              reg_with_reset_and_enable_text(cell_type_t::CELL_DFFE_PP1P, "positive", "high", 1, "high"));



#endif // CELL_TYPES_H
