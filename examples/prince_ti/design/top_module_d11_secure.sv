module top_module_d11(
    i_clk,
    i_reset,
    i_start,
    i_load,
    i_enc_dec,
    i_pt1,
    i_pt2,
    i_key1,
    i_key2,
    i_r,
    o_ct1,
    o_ct2,
    o_done
);

input  i_start;
input  i_clk;
input  i_reset;
input  i_enc_dec;
input  i_load;

//input  [111 : 0] i_r;
input  [191 : 0] i_r;
// input  [47 : 0] i_r;

input  [63 : 0] i_pt1;
input  [63 : 0] i_pt2;

input  [127 : 0] i_key1;
input  [127 : 0] i_key2;

output wire [63 : 0] o_ct1;
output wire [63 : 0] o_ct2;
output wire o_done;
parameter IDLE = 2'b0, ROUND_FW = 2'b10, ROUND_BW = 2'b11, HALT = 2'b01;

reg [3 : 0] rnd_cnt;
reg [3 : 0] rnd_next;
reg  subrnd_cnt;
reg  subrnd_next;
reg [1 : 0] state;
reg [1 : 0] next_state;
reg done_reg;
reg done_next;

reg sel1, sel2, sel3, sel4;
reg [3 : 0] rc_sel;
wire rk_sel;
wire [3 : 0] rc_in_xor;
reg enc_dec_reg;
reg enc_dec_next;

wire  [63:0] RCarray [15 : 0];
reg [63 : 0] RC_in;
assign  RCarray[0]  =  64'h0;
assign  RCarray[1]  =  64'h0;
assign  RCarray[2]  =  64'h0;
assign  RCarray[3]  =  64'h13198a2e03707344;
assign  RCarray[4]  =  64'ha4093822299f31d0;
assign  RCarray[5]  =  64'h082efa98ec4e6c89;
assign  RCarray[6]  =  64'h452821e638d01377;
assign  RCarray[7]  =  64'hbe5466cf34e90c6c;
assign  RCarray[8]  =  64'h7ef84f78fd955cb1;
assign  RCarray[9]  =  64'h85840851f1ac43aa;
assign  RCarray[10] =  64'hc882d32f25323c54;
assign  RCarray[11] =  64'h64a51195e0e3610d;
assign  RCarray[12] =  64'hd3b5a399ca0c2399;
assign  RCarray[13] =  64'hc0ac29b7c97c50dd;
assign  RCarray[14]  =  64'h0;
assign  RCarray[15]  =  64'h0;


always @(rc_sel or RCarray[12] or RCarray[13] or RCarray[2] or
         RCarray[3] or RCarray[4] or RCarray[5] or RCarray[6] or 
         RCarray[7] or RCarray[8] or RCarray[9] or RCarray[10] or 
         RCarray[11] or RCarray[0] or RCarray[1] or RCarray[14] or 
         RCarray[15])
    RC_in = RCarray[rc_sel];


//Sbox input register and next value
wire [63 : 0] in_reg_next1;
wire [63 : 0] in_reg_next2;

wire [63 : 0] in_reg_out1;
wire [63 : 0] in_reg_out2;


//Sbox output register and next value

wire [15 : 0] sbox_out1_b1;
wire [15 : 0] sbox_out2_b1;
wire [15 : 0] sbox_out3_b1;
wire [15 : 0] sbox_out4_b1;
wire [15 : 0] sbox_out5_b1;
wire [15 : 0] sbox_out6_b1;
wire [15 : 0] sbox_out7_b1;
wire [15 : 0] sbox_out8_b1;
                       
wire [15 : 0] sbox_out1_b2;
wire [15 : 0] sbox_out2_b2;
wire [15 : 0] sbox_out3_b2;
wire [15 : 0] sbox_out4_b2;
wire [15 : 0] sbox_out5_b2;
wire [15 : 0] sbox_out6_b2;
wire [15 : 0] sbox_out7_b2;
wire [15 : 0] sbox_out8_b2;
                       
wire [15 : 0] sbox_out1_b3;
wire [15 : 0] sbox_out2_b3;
wire [15 : 0] sbox_out3_b3;
wire [15 : 0] sbox_out4_b3;
wire [15 : 0] sbox_out5_b3;
wire [15 : 0] sbox_out6_b3;
wire [15 : 0] sbox_out7_b3;
wire [15 : 0] sbox_out8_b3;
                       
wire [15 : 0] sbox_out1_b4;
wire [15 : 0] sbox_out2_b4;
wire [15 : 0] sbox_out3_b4;
wire [15 : 0] sbox_out4_b4;
wire [15 : 0] sbox_out5_b4;
wire [15 : 0] sbox_out6_b4;
wire [15 : 0] sbox_out7_b4;
wire [15 : 0] sbox_out8_b4;

//Remasked values
wire [15 : 0] out_b1_reg_next1;
wire [15 : 0] out_b1_reg_next2;
wire [15 : 0] out_b1_reg_next3;
wire [15 : 0] out_b1_reg_next4;
wire [15 : 0] out_b1_reg_next5;
wire [15 : 0] out_b1_reg_next6;
wire [15 : 0] out_b1_reg_next7;
wire [15 : 0] out_b1_reg_next8;

wire [15 : 0] out_b2_reg_next1;
wire [15 : 0] out_b2_reg_next2;
wire [15 : 0] out_b2_reg_next3;
wire [15 : 0] out_b2_reg_next4;
wire [15 : 0] out_b2_reg_next5;
wire [15 : 0] out_b2_reg_next6;
wire [15 : 0] out_b2_reg_next7;
wire [15 : 0] out_b2_reg_next8;

wire [15 : 0] out_b3_reg_next1;
wire [15 : 0] out_b3_reg_next2;
wire [15 : 0] out_b3_reg_next3;
wire [15 : 0] out_b3_reg_next4;
wire [15 : 0] out_b3_reg_next5;
wire [15 : 0] out_b3_reg_next6;
wire [15 : 0] out_b3_reg_next7;
wire [15 : 0] out_b3_reg_next8;

wire [15 : 0] out_b4_reg_next1;
wire [15 : 0] out_b4_reg_next2;
wire [15 : 0] out_b4_reg_next3;
wire [15 : 0] out_b4_reg_next4;
wire [15 : 0] out_b4_reg_next5;
wire [15 : 0] out_b4_reg_next6;
wire [15 : 0] out_b4_reg_next7;
wire [15 : 0] out_b4_reg_next8;


wire [15 : 0] out_b1_reg_out1;
wire [15 : 0] out_b1_reg_out2;
wire [15 : 0] out_b1_reg_out3;
wire [15 : 0] out_b1_reg_out4;
wire [15 : 0] out_b1_reg_out5;
wire [15 : 0] out_b1_reg_out6;
wire [15 : 0] out_b1_reg_out7;
wire [15 : 0] out_b1_reg_out8;

wire [15 : 0] out_b2_reg_out1;
wire [15 : 0] out_b2_reg_out2;
wire [15 : 0] out_b2_reg_out3;
wire [15 : 0] out_b2_reg_out4;
wire [15 : 0] out_b2_reg_out5;
wire [15 : 0] out_b2_reg_out6;
wire [15 : 0] out_b2_reg_out7;
wire [15 : 0] out_b2_reg_out8;

wire [15 : 0] out_b3_reg_out1;
wire [15 : 0] out_b3_reg_out2;
wire [15 : 0] out_b3_reg_out3;
wire [15 : 0] out_b3_reg_out4;
wire [15 : 0] out_b3_reg_out5;
wire [15 : 0] out_b3_reg_out6;
wire [15 : 0] out_b3_reg_out7;
wire [15 : 0] out_b3_reg_out8;

wire [15 : 0] out_b4_reg_out1;
wire [15 : 0] out_b4_reg_out2;
wire [15 : 0] out_b4_reg_out3;
wire [15 : 0] out_b4_reg_out4;
wire [15 : 0] out_b4_reg_out5;
wire [15 : 0] out_b4_reg_out6;
wire [15 : 0] out_b4_reg_out7;
wire [15 : 0] out_b4_reg_out8;



//key registers
reg [63 : 0] key0_next1;
reg [63 : 0] key0_next2;
                  
wire [63 : 0] key0_reg_out1;
wire [63 : 0] key0_reg_out2;
                  
                  
reg [63 : 0] key1_next1;
reg [63 : 0] key1_next2;
                 
wire [63 : 0] key1_reg_out1;
wire [63 : 0] key1_reg_out2;

wire [63 : 0] comp_sh1;
wire [63 : 0] comp_sh2;


wire [63 : 0] fwd_rnd_mux1_in1;
wire [63 : 0] fwd_rnd_mux1_in2;

wire [63 : 0] outaff_out1;
wire [63 : 0] outaff_out2;

wire [63 : 0] rk_out1;
wire [63 : 0] rk_out2;

wire [63 : 0] k1_out1;
wire [63 : 0] k1_out2;

wire [63 : 0] invkeyadd_out1;
wire [63 : 0] invkeyadd_out2;

wire [63 : 0] inv_sr_out1;
wire [63 : 0] inv_sr_out2;

wire [63 : 0] bkwd_rnd_mux1_in1;
wire [63 : 0] bkwd_rnd_mux1_in2;

wire [63 : 0] mux1_out1;
wire [63 : 0] mux1_out2;

wire [63 : 0] mprime_out1;
wire [63 : 0] mprime_out2;

wire [63 : 0] mux2_in1 [1 : 0];
wire [63 : 0] mux2_in2 [1 : 0];

wire [63 : 0] in_key1;
wire [63 : 0] in_key2;


wire [63 : 0] in_wkey1;
wire [63 : 0] in_wkey2;

wire [63 : 0] in_wkey_out1;
wire [63 : 0] in_wkey_out2;

wire [63 : 0] wkey_inv_out1;
wire [63 : 0] wkey_inv_out2;

wire [63 : 0] mux2_out1;
wire [63 : 0] mux2_out2;

wire [63 : 0] srows_out1;
wire [63 : 0] srows_out2;

wire [63 : 0] fwdkey_out1;
wire [63 : 0] fwdkey_out2;

wire [63 : 0] inaff_out1;
wire [63 : 0] inaff_out2;

wire [63 : 0] mux3_in1 [1 : 0];
wire [63 : 0] mux3_in2 [1 : 0];

wire [63 : 0] mux3_out1;
wire [63 : 0] mux3_out2;

wire [63 : 0] mux4_in1 [1 : 0];
wire [63 : 0] mux4_in2 [1 : 0];

wire [63 : 0] mux4_out1;
wire [63 : 0] mux4_out2;




wire [63 : 0] key0_out1;
wire [63 : 0] key0_out2;

wire [63 : 0] out_wkey1;
wire [63 : 0] out_wkey2;





genvar i;

generate
for(i=0; i<64; i = i + 1) begin: KEY0_REG_INST1
    share_reg in_reg1(
        .i_clk (i_clk),
        .i_d   (key0_next1[i]),
        .o_q   (key0_reg_out1[i])
    );
end
endgenerate

generate
for(i=0; i<64; i = i + 1) begin: KEY0_REG_INST2
    share_reg in_reg2(
        .i_clk (i_clk),
        .i_d   (key0_next2[i]),
        .o_q   (key0_reg_out2[i])
    );
end
endgenerate


generate
for(i=0; i<64; i = i + 1) begin: KEY1_REG_INST1
    share_reg in_reg1(
        .i_clk (i_clk),
        .i_d   (key1_next1[i]),
        .o_q   (key1_reg_out1[i])
    );
end
endgenerate

generate
for(i=0; i<64; i = i + 1) begin: KEY1_REG_INST2
    share_reg in_reg2(
        .i_clk (i_clk),
        .i_d   (key1_next2[i]),
        .o_q   (key1_reg_out2[i])
    );
end
endgenerate


generate
for(i=0; i<64; i = i + 1) begin: IN_REG_INST1
    share_reg in_reg1(
        .i_clk (i_clk),
        .i_d   (in_reg_next1[i]),
        .o_q   (in_reg_out1[i])
    );
end
endgenerate

generate
for(i=0; i<64; i = i + 1) begin: IN_REG_INST2
    share_reg in_reg2(
        .i_clk (i_clk),
        .i_d   (in_reg_next2[i]),
        .o_q   (in_reg_out2[i])
    );
end
endgenerate




generate
for(i=0; i<16; i = i + 1) begin: OUT_REG_INST_B1_S1
    share_reg out_reg_b1_s1(
        .i_clk (i_clk),
        .i_d   (out_b1_reg_next1[i]),
        .o_q   (out_b1_reg_out1[i])
    );
end
endgenerate

generate
for(i=0; i<16; i = i + 1) begin: OUT_REG_INST_B1_S2
    share_reg out_reg_b1_s2(
        .i_clk (i_clk),
        .i_d   (out_b1_reg_next2[i]),
        .o_q   (out_b1_reg_out2[i])
    );
end
endgenerate

generate
for(i=0; i<16; i = i + 1) begin: OUT_REG_INST_B1_S3
    share_reg out_reg_b1_s3(
        .i_clk (i_clk),
        .i_d   (out_b1_reg_next3[i]),
        .o_q   (out_b1_reg_out3[i])
    );
end
endgenerate

generate
for(i=0; i<16; i = i + 1) begin: OUT_REG_INST_B1_S4
    share_reg out_reg_b1_s4(
        .i_clk (i_clk),
        .i_d   (out_b1_reg_next4[i]),
        .o_q   (out_b1_reg_out4[i])
    );
end
endgenerate

generate
for(i=0; i<16; i = i + 1) begin: OUT_REG_INST_B1_S5
    share_reg out_reg_b1_s5(
        .i_clk (i_clk),
        .i_d   (out_b1_reg_next5[i]),
        .o_q   (out_b1_reg_out5[i])
    );
end
endgenerate

generate
for(i=0; i<16; i = i + 1) begin: OUT_REG_INST_B1_S6
    share_reg out_reg_b1_s6(
        .i_clk (i_clk),
        .i_d   (out_b1_reg_next6[i]),
        .o_q   (out_b1_reg_out6[i])
    );
end
endgenerate

generate
for(i=0; i<16; i = i + 1) begin: OUT_REG_INST_B1_S7
    share_reg out_reg_b1_s7(
        .i_clk (i_clk),
        .i_d   (out_b1_reg_next7[i]),
        .o_q   (out_b1_reg_out7[i])
    );
end
endgenerate

generate
for(i=0; i<16; i = i + 1) begin: OUT_REG_INST_B1_S8
    share_reg out_reg_b1_s8(
        .i_clk (i_clk),
        .i_d   (out_b1_reg_next8[i]),
        .o_q   (out_b1_reg_out8[i])
    );
end
endgenerate

//Bit 2 reg instantiation
generate
for(i=0; i<16; i = i + 1) begin: OUT_REG_INST_B2_S1
    share_reg out_reg_b2_s1(
        .i_clk (i_clk),
        .i_d   (out_b2_reg_next1[i]),
        .o_q   (out_b2_reg_out1[i])
    );
end
endgenerate

generate
for(i=0; i<16; i = i + 1) begin: OUT_REG_INST_B2_S2
    share_reg out_reg_b2_s2(
        .i_clk (i_clk),
        .i_d   (out_b2_reg_next2[i]),
        .o_q   (out_b2_reg_out2[i])
    );
end
endgenerate

generate
for(i=0; i<16; i = i + 1) begin: OUT_REG_INST_B2_S3
    share_reg out_reg_b2_s3(
        .i_clk (i_clk),
        .i_d   (out_b2_reg_next3[i]),
        .o_q   (out_b2_reg_out3[i])
    );
end
endgenerate

generate
for(i=0; i<16; i = i + 1) begin: OUT_REG_INST_B2_S4
    share_reg out_reg_b2_s4(
        .i_clk (i_clk),
        .i_d   (out_b2_reg_next4[i]),
        .o_q   (out_b2_reg_out4[i])
    );
end
endgenerate

generate
for(i=0; i<16; i = i + 1) begin: OUT_REG_INST_B2_S5
    share_reg out_reg_b2_s5(
        .i_clk (i_clk),
        .i_d   (out_b2_reg_next5[i]),
        .o_q   (out_b2_reg_out5[i])
    );
end
endgenerate

generate
for(i=0; i<16; i = i + 1) begin: OUT_REG_INST_B2_S6
    share_reg out_reg_b2_s6(
        .i_clk (i_clk),
        .i_d   (out_b2_reg_next6[i]),
        .o_q   (out_b2_reg_out6[i])
    );
end
endgenerate

generate
for(i=0; i<16; i = i + 1) begin: OUT_REG_INST_B2_S7
    share_reg out_reg_b2_s7(
        .i_clk (i_clk),
        .i_d   (out_b2_reg_next7[i]),
        .o_q   (out_b2_reg_out7[i])
    );
end
endgenerate

generate
for(i=0; i<16; i = i + 1) begin: OUT_REG_INST_B2_S8
    share_reg out_reg_b2_s8(
        .i_clk (i_clk),
        .i_d   (out_b2_reg_next8[i]),
        .o_q   (out_b2_reg_out8[i])
    );
end
endgenerate

//Bit 3 instantiation
generate
for(i=0; i<16; i = i + 1) begin: OUT_REG_INST_B3_S1
    share_reg out_reg_b3_s1(
        .i_clk (i_clk),
        .i_d   (out_b3_reg_next1[i]),
        .o_q   (out_b3_reg_out1[i])
    );
end
endgenerate

generate
for(i=0; i<16; i = i + 1) begin: OUT_REG_INST_B3_S2
    share_reg out_reg_b3_s2(
        .i_clk (i_clk),
        .i_d   (out_b3_reg_next2[i]),
        .o_q   (out_b3_reg_out2[i])
    );
end
endgenerate

generate
for(i=0; i<16; i = i + 1) begin: OUT_REG_INST_B3_S3
    share_reg out_reg_b3_s3(
        .i_clk (i_clk),
        .i_d   (out_b3_reg_next3[i]),
        .o_q   (out_b3_reg_out3[i])
    );
end
endgenerate

generate
for(i=0; i<16; i = i + 1) begin: OUT_REG_INST_B3_S4
    share_reg out_reg_b3_s4(
        .i_clk (i_clk),
        .i_d   (out_b3_reg_next4[i]),
        .o_q   (out_b3_reg_out4[i])
    );
end
endgenerate

generate
for(i=0; i<16; i = i + 1) begin: OUT_REG_INST_B3_S5
    share_reg out_reg_b3_s5(
        .i_clk (i_clk),
        .i_d   (out_b3_reg_next5[i]),
        .o_q   (out_b3_reg_out5[i])
    );
end
endgenerate

generate
for(i=0; i<16; i = i + 1) begin: OUT_REG_INST_B3_S6
    share_reg out_reg_b3_s6(
        .i_clk (i_clk),
        .i_d   (out_b3_reg_next6[i]),
        .o_q   (out_b3_reg_out6[i])
    );
end
endgenerate

generate
for(i=0; i<16; i = i + 1) begin: OUT_REG_INST_B3_S7
    share_reg out_reg_b3_s7(
        .i_clk (i_clk),
        .i_d   (out_b3_reg_next7[i]),
        .o_q   (out_b3_reg_out7[i])
    );
end
endgenerate

generate
for(i=0; i<16; i = i + 1) begin: OUT_REG_INST_B3_S8
    share_reg out_reg_b3_s8(
        .i_clk (i_clk),
        .i_d   (out_b3_reg_next8[i]),
        .o_q   (out_b3_reg_out8[i])
    );
end
endgenerate

//Bit 4 instantiation
generate
for(i=0; i<16; i = i + 1) begin: OUT_REG_INST_B4_S1
    share_reg out_reg_b4_s1(
        .i_clk (i_clk),
        .i_d   (out_b4_reg_next1[i]),
        .o_q   (out_b4_reg_out1[i])
    );
end
endgenerate

generate
for(i=0; i<16; i = i + 1) begin: OUT_REG_INST_B4_S2
    share_reg out_reg_b4_s2(
        .i_clk (i_clk),
        .i_d   (out_b4_reg_next2[i]),
        .o_q   (out_b4_reg_out2[i])
    );
end
endgenerate

generate
for(i=0; i<16; i = i + 1) begin: OUT_REG_INST_B4_S3
    share_reg out_reg_b4_s3(
        .i_clk (i_clk),
        .i_d   (out_b4_reg_next3[i]),
        .o_q   (out_b4_reg_out3[i])
    );
end
endgenerate

generate
for(i=0; i<16; i = i + 1) begin: OUT_REG_INST_B4_S4
    share_reg out_reg_b4_s4(
        .i_clk (i_clk),
        .i_d   (out_b4_reg_next4[i]),
        .o_q   (out_b4_reg_out4[i])
    );
end
endgenerate

generate
for(i=0; i<16; i = i + 1) begin: OUT_REG_INST_B4_S5
    share_reg out_reg_b4_s5(
        .i_clk (i_clk),
        .i_d   (out_b4_reg_next5[i]),
        .o_q   (out_b4_reg_out5[i])
    );
end
endgenerate

generate
for(i=0; i<16; i = i + 1) begin: OUT_REG_INST_B4_S6
    share_reg out_reg_b4_s6(
        .i_clk (i_clk),
        .i_d   (out_b4_reg_next6[i]),
        .o_q   (out_b4_reg_out6[i])
    );
end
endgenerate

generate
for(i=0; i<16; i = i + 1) begin: OUT_REG_INST_B4_S7
    share_reg out_reg_b4_s7(
        .i_clk (i_clk),
        .i_d   (out_b4_reg_next7[i]),
        .o_q   (out_b4_reg_out7[i])
    );
end
endgenerate

generate
for(i=0; i<16; i = i + 1) begin: OUT_REG_INST_B4_S8
    share_reg out_reg_b4_s8(
        .i_clk (i_clk),
        .i_d   (out_b4_reg_next8[i]),
        .o_q   (out_b4_reg_out8[i])
    );
end
endgenerate






//HERE we start with connecting
assign rk_sel = (rnd_cnt == 4'h2)? 1'b1 : 1'b0;

mux_cir_2sh rk_mux(
    .sel (rk_sel),
    .a1  (i_key1[63 : 0]),
    .a2  (i_key2[63 : 0]),
    .b1  (key1_reg_out1),
    .b2  (key1_reg_out2),
    .out1 (k1_out1),
    .out2 (k1_out2)
);




xor_cir rc_add(
    .a (k1_out1),
    .b (RC_in),
    .out (rk_out1)
);

assign rk_out2 = k1_out2;

assign rc_in_xor = (rnd_cnt == 4'h2)? {i_enc_dec, i_enc_dec, i_enc_dec, i_enc_dec} : {enc_dec_reg, enc_dec_reg, enc_dec_reg, enc_dec_reg};

// assign rc_sel = rnd_cnt ^ rc_in_xor;

always @(rnd_cnt or rc_in_xor) begin
    rc_sel = rnd_cnt ^ rc_in_xor;
end

generate 
for (i=0; i<16; i = i+1)begin : COMPRESS11
    xor_cir4 #(.N(1)) comp_b1_s1(
        .in1 (out_b1_reg_out1[i]),
        .in2 (out_b1_reg_out2[i]),
        .in3 (out_b1_reg_out3[i]),
        .in4 (out_b1_reg_out4[i]),
        .out (comp_sh1[0 + 4*i])
    );
end
endgenerate
generate 
for (i=0; i<16; i = i+1)begin : COMPRESS12
    xor_cir4 #(.N(1)) comp_b1_s2(
        .in1 (out_b1_reg_out5[i]),
        .in2 (out_b1_reg_out6[i]),
        .in3 (out_b1_reg_out7[i]),
        .in4 (out_b1_reg_out8[i]),
        .out (comp_sh2[0 + 4*i])
    );
end
endgenerate

generate 
for (i=0; i<16; i = i+1)begin : COMPRESS2
    xor_cir4 #(.N(1)) comp_b2_s1(
        .in1 (out_b2_reg_out1[i]),
        .in2 (out_b2_reg_out2[i]),
        .in3 (out_b2_reg_out3[i]),
        .in4 (out_b2_reg_out4[i]),
        .out (comp_sh1[1 + 4*i])
    );
    xor_cir4 #(.N(1)) comp_b2_s2(
        .in1 (out_b2_reg_out5[i]),
        .in2 (out_b2_reg_out6[i]),
        .in3 (out_b2_reg_out7[i]),
        .in4 (out_b2_reg_out8[i]),
        .out (comp_sh2[1 + 4*i])
    );
end
endgenerate

generate 
for (i=0; i<16; i = i+1)begin : COMPRESS3
    xor_cir4 #(.N(1)) comp_b3_s1(
        .in1 (out_b3_reg_out1[i]),
        .in2 (out_b3_reg_out2[i]),
        .in3 (out_b3_reg_out3[i]),
        .in4 (out_b3_reg_out4[i]),
        .out (comp_sh1[2 + 4*i])
    );
    xor_cir4 #(.N(1)) comp_b3_s2(
        .in1 (out_b3_reg_out5[i]),
        .in2 (out_b3_reg_out6[i]),
        .in3 (out_b3_reg_out7[i]),
        .in4 (out_b3_reg_out8[i]),
        .out (comp_sh2[2 + 4*i])
    );
end
endgenerate

generate 
for (i=0; i<16; i = i+1)begin : COMPRESS4
    xor_cir4 #(.N(1)) comp_b4_s1(
        .in1 (out_b4_reg_out1[i]),
        .in2 (out_b4_reg_out2[i]),
        .in3 (out_b4_reg_out3[i]),
        .in4 (out_b4_reg_out4[i]),
        .out (comp_sh1[3 + 4*i])
    );
    xor_cir4 #(.N(1)) comp_b4_s2(
        .in1 (out_b4_reg_out5[i]),
        .in2 (out_b4_reg_out6[i]),
        .in3 (out_b4_reg_out7[i]),
        .in4 (out_b4_reg_out8[i]),
        .out (comp_sh2[3 + 4*i])
    );
end
endgenerate




assign fwd_rnd_mux1_in1 = comp_sh1;
assign fwd_rnd_mux1_in2 = comp_sh2;

affine_2sh outaff(
    .in1  (comp_sh1),
    .in2  (comp_sh2),
    .out1 (outaff_out1),
    .out2 (outaff_out2)
);

xor_cir_2sh inv_keyadd(
    .a1   (outaff_out1),
    .a2   (outaff_out2),
    .b1   (rk_out1),
    .b2   (rk_out2),
    .out1 (invkeyadd_out1),
    .out2 (invkeyadd_out2)
);

inv_sr_2sh invsr(
    .in1  (invkeyadd_out1),
    .in2  (invkeyadd_out2),
    .out1 (inv_sr_out1),
    .out2 (inv_sr_out2)
);






assign bkwd_rnd_mux1_in1 = inv_sr_out1;
assign bkwd_rnd_mux1_in2 = inv_sr_out2;

//Input to mprime
mux_cir_2sh mux1(
    .sel (sel1),
    .a1 (fwd_rnd_mux1_in1),
    .a2 (fwd_rnd_mux1_in2),
    .b1 (bkwd_rnd_mux1_in1),
    .b2 (bkwd_rnd_mux1_in2),
    .out1 (mux1_out1),
    .out2 (mux1_out2)
);

mprime_cir_2sh mprime(
    .in1 (mux1_out1),
    .in2 (mux1_out2),
    .out1 (mprime_out1),
    .out2 (mprime_out2)
);

assign mux2_in1[1] = mprime_out1;
assign mux2_in2[1] = mprime_out2;

out_key_share_flip in_ok1(
    .in_key (i_key1[127 : 64]),
    .out_key (in_key1)
);

out_key_share_flip in_ok2(
    .in_key (i_key2[127 : 64]),
    .out_key (in_key2)
);


mux_cir_2sh in_key(
    .sel (i_enc_dec),
    .a1  (in_key1),
    .a2  (in_key2),
    .b1  (i_key1[127 : 64]),
    .b2  (i_key2[127 : 64]),
    .out1 (in_wkey1),
    .out2 (in_wkey2)
);

xor_cir_2sh inwkey_add(
    .a1   (i_pt1),
    .a2   (i_pt2),
    .b1   (in_wkey1),
    .b2   (in_wkey2),
    .out1 (in_wkey_out1),
    .out2 (in_wkey_out2)
);

inv_sr_2sh wkey_invsr(
    .in1  (in_wkey_out1),
    .in2  (in_wkey_out2),
    .out1 (wkey_inv_out1),
    .out2 (wkey_inv_out2)
);

assign mux2_in1[0] = wkey_inv_out1;
assign mux2_in2[0] = wkey_inv_out2;

mux_cir_2sh mux2(
    .sel (sel2),
    .a1 (mux2_in1[1]),
    .a2 (mux2_in2[1]),
    .b1 (mux2_in1[0]),
    .b2 (mux2_in2[0]),
    .out1 (mux2_out1),
    .out2 (mux2_out2)
);


sr_2sh srows(
    .in1  (mux2_out1),
    .in2  (mux2_out2),
    .out1 (srows_out1),
    .out2 (srows_out2)
);

xor_cir_2sh fwdkey_add(
    .a1   (srows_out1),
    .a2   (srows_out2),
    .b1   (rk_out1),
    .b2   (rk_out2),
    .out1 (fwdkey_out1),
    .out2 (fwdkey_out2)
);

affine_2sh inaff(
    .in1  (mux2_out1),
    .in2  (mux2_out2),
    .out1 (inaff_out1),
    .out2 (inaff_out2)
);

assign mux3_in1[1] = fwdkey_out1;
assign mux3_in2[1] = fwdkey_out2;

assign mux3_in1[0] = inaff_out1;
assign mux3_in2[0] = inaff_out2;

mux_cir_2sh mux3(
    .sel (sel3),
    .a1 (mux3_in1[1]),
    .a2 (mux3_in2[1]),
    .b1 (mux3_in1[0]),
    .b2 (mux3_in2[0]),
    .out1 (mux3_out1),
    .out2 (mux3_out2)
);

assign mux4_in1[1] = mux3_out1;
assign mux4_in2[1] = mux3_out2;

assign mux4_in1[0] = in_reg_out1;
assign mux4_in2[0] = in_reg_out2;

mux_cir_2sh mux4(
    .sel (sel4),
    .a1 (mux4_in1[1]),
    .a2 (mux4_in2[1]),
    .b1 (mux4_in1[0]),
    .b2 (mux4_in2[0]),
    .out1 (mux4_out1),
    .out2 (mux4_out2)
);

assign in_reg_next1 = mux4_out1;
assign in_reg_next2 = mux4_out2;





//Sbox instantiation

generate
for (i=0; i<16; i= i+1) begin: SBOX_INST
    s SBOX_ITK(
        .in1 (in_reg_out1[4*i + 3: 4*i]),
        .in2 (in_reg_out2[4*i + 3: 4*i]),
        .out1({sbox_out8_b1[i],sbox_out7_b1[i],sbox_out6_b1[i],sbox_out5_b1[i],sbox_out4_b1[i],sbox_out3_b1[i],sbox_out2_b1[i],sbox_out1_b1[i]}),
        .out2({sbox_out8_b2[i],sbox_out7_b2[i],sbox_out6_b2[i],sbox_out5_b2[i],sbox_out4_b2[i],sbox_out3_b2[i],sbox_out2_b2[i],sbox_out1_b2[i]}),
        .out3({sbox_out8_b3[i],sbox_out7_b3[i],sbox_out6_b3[i],sbox_out5_b3[i],sbox_out4_b3[i],sbox_out3_b3[i],sbox_out2_b3[i],sbox_out1_b3[i]}),
        .out4({sbox_out8_b4[i],sbox_out7_b4[i],sbox_out6_b4[i],sbox_out5_b4[i],sbox_out4_b4[i],sbox_out3_b4[i],sbox_out2_b4[i],sbox_out1_b4[i]})
    );
end
endgenerate

generate
for (i=0; i<16; i= i+1) begin: REMASKING
    sbox_rmsk srmsk(
        .in1({sbox_out8_b1[i],sbox_out7_b1[i],sbox_out6_b1[i],sbox_out5_b1[i],sbox_out4_b1[i],sbox_out3_b1[i],sbox_out2_b1[i],sbox_out1_b1[i]}),
        .in2({sbox_out8_b2[i],sbox_out7_b2[i],sbox_out6_b2[i],sbox_out5_b2[i],sbox_out4_b2[i],sbox_out3_b2[i],sbox_out2_b2[i],sbox_out1_b2[i]}),
        .in3({sbox_out8_b3[i],sbox_out7_b3[i],sbox_out6_b3[i],sbox_out5_b3[i],sbox_out4_b3[i],sbox_out3_b3[i],sbox_out2_b3[i],sbox_out1_b3[i]}),
        .in4({sbox_out8_b4[i],sbox_out7_b4[i],sbox_out6_b4[i],sbox_out5_b4[i],sbox_out4_b4[i],sbox_out3_b4[i],sbox_out2_b4[i],sbox_out1_b4[i]}),
        // .in_rand (i_r[(11 + 12*i)%48: (12*i)%48]),
        .in_rand (i_r[(11 + 12*i): (12*i)]),
        .out1({out_b1_reg_next8[i],out_b1_reg_next7[i],out_b1_reg_next6[i],out_b1_reg_next5[i],out_b1_reg_next4[i],out_b1_reg_next3[i],out_b1_reg_next2[i],out_b1_reg_next1[i]}),
        .out2({out_b2_reg_next8[i],out_b2_reg_next7[i],out_b2_reg_next6[i],out_b2_reg_next5[i],out_b2_reg_next4[i],out_b2_reg_next3[i],out_b2_reg_next2[i],out_b2_reg_next1[i]}),
        .out3({out_b3_reg_next8[i],out_b3_reg_next7[i],out_b3_reg_next6[i],out_b3_reg_next5[i],out_b3_reg_next4[i],out_b3_reg_next3[i],out_b3_reg_next2[i],out_b3_reg_next1[i]}),
        .out4({out_b4_reg_next8[i],out_b4_reg_next7[i],out_b4_reg_next6[i],out_b4_reg_next5[i],out_b4_reg_next4[i],out_b4_reg_next3[i],out_b4_reg_next2[i],out_b4_reg_next1[i]})
    );
end
endgenerate



//Output generation

out_key_share_flip ok1(
    .in_key (key0_reg_out1),
    .out_key (key0_out1)
);

out_key_share_flip ok2(
    .in_key (key0_reg_out2),
    .out_key (key0_out2)
);


mux_cir_2sh mux_out(
    .sel (enc_dec_reg),
    .a1 (key0_reg_out1),
    .a2 (key0_reg_out2),
    .b1 (key0_out1),
    .b2 (key0_out2),
    .out1 (out_wkey1),
    .out2 (out_wkey2)
);

xor_cir_2sh outkey_add(
    .a1 (out_wkey1),
    .a2 (out_wkey2),
    .b1 (invkeyadd_out1),
    .b2 (invkeyadd_out2),
    .out1 (o_ct1),
    .out2 (o_ct2)
);

assign o_done = done_reg;
//Next state logic
always @(i_start or i_load or state or i_pt1 or i_pt2 or i_key1 or i_key2 or rnd_cnt or subrnd_cnt or key0_reg_out1 or key0_reg_out2 or key1_reg_out1 or key1_reg_out2 or in_reg_out1 or in_reg_out2 or out_b1_reg_out1 or out_b1_reg_out2 or out_b1_reg_out3 or out_b1_reg_out4 or out_b1_reg_out5 or out_b1_reg_out6 or out_b1_reg_out7 or out_b1_reg_out8 or out_b2_reg_out1 or out_b2_reg_out2 or out_b2_reg_out3 or out_b2_reg_out4 or out_b2_reg_out5 or out_b2_reg_out6 or out_b2_reg_out7 or out_b2_reg_out8 or out_b3_reg_out1 or out_b3_reg_out2 or out_b3_reg_out3 or out_b3_reg_out4 or out_b3_reg_out5 or out_b3_reg_out6 or out_b3_reg_out7 or out_b3_reg_out8 or out_b4_reg_out1 or out_b4_reg_out2 or out_b4_reg_out3 or out_b4_reg_out4 or out_b4_reg_out5 or out_b4_reg_out6 or out_b4_reg_out7 or out_b4_reg_out8)
//always @(*)
begin
    next_state = state;
    sel1 = 1'b1;
    sel2 = 2'b1;
    sel3 = 1'b1;
    sel4 = 2'b1;
    key0_next1 = key0_reg_out1;
    key0_next2 = key0_reg_out2;
    key1_next1 = key1_reg_out1;
    key1_next2 = key1_reg_out2;
    rnd_next = rnd_cnt;
    subrnd_next = subrnd_cnt + 1'b1;
    enc_dec_next = enc_dec_reg;
    done_next = 1'b0;
    case(state) 
        IDLE :  begin
        if (i_load == 1'b1) begin
            key0_next1 = i_key1[127 : 64];
            key0_next2 = i_key2[127 : 64];
            key1_next1 = i_key1[63 : 0];
            key1_next2 = i_key2[63 : 0];
            next_state = IDLE;
            //rnd_next = 4'h2;
            //subrnd_next = 1'b1;
            enc_dec_next = i_enc_dec;
            sel2  = 1'b0;
            sel3 = 1'b1;
            sel4 = 1'b1;
        end else if (i_start == 1'b1) begin
            next_state = ROUND_FW;
            rnd_next = 4'h2;
            subrnd_next = 1'b1;
            sel4 = 1'b0;
        end else begin
            sel4 = 1'b0;
            next_state = IDLE;
        end
    end
        ROUND_FW: begin
            sel1 = 1'b1;
            sel2 = 1'b1;
            sel3 = 1'b1;
            sel4 = 1'b1;
            next_state = ROUND_FW;
            subrnd_next = subrnd_cnt + 1'b1;
            rnd_next = rnd_cnt;
            if (subrnd_cnt == 1'b1) begin
                sel4 = 1'b0;
                if (rnd_cnt == 4'h7 ) begin
                    rnd_next = rnd_cnt;
                    next_state = ROUND_BW;
                end else begin
                    rnd_next = rnd_cnt + 4'h1;
                    sel3 = 1'b0;
                end
            end else begin
                sel4 = 1'b1;
                if (rnd_cnt == 4'h7) begin
                end
            end
        end
        ROUND_BW: begin
            sel1 = 1'b0;
            sel2 = 1'b1;
            sel3 = 1'b0;
            sel4 = 1'b1;
            subrnd_next = subrnd_cnt + 1'b1;
            rnd_next = rnd_cnt;
            next_state = ROUND_BW;
            if (rnd_cnt == 4'h7) begin
                sel1 = 1'b1;
                sel2 = 1'b1;
                sel3 = 1'b0;
                sel4 = 1'b1;
            end
            if (subrnd_cnt == 1'b1) begin
                sel4 = 1'b0;
                rnd_next = rnd_cnt + 4'h1;
                if (rnd_cnt == 4'hC) begin
                    next_state = HALT;
                    done_next = 1'b1;
                end
            end else begin
                sel4 = 1'b1;
            end
        end
        HALT: begin
            next_state = HALT;
            done_next = 1'b0;
            if (i_start == 0) begin
                next_state = IDLE;
                done_next = 1'b0;
                rnd_next = 4'h2;
                subrnd_next = 2'h0;
            end
        end
        default: begin
            next_state = IDLE;
        end
    endcase
end

always @(posedge i_clk or posedge i_reset)
begin
    if (i_reset == 1'b1) begin
        state <= IDLE;
        rnd_cnt <= 4'h2;
        subrnd_cnt <= 2'h0;
        done_reg <= 1'h0;
        enc_dec_reg <= 1'h0;
    end else begin
        state <= next_state;
        rnd_cnt <= rnd_next;
        subrnd_cnt <= subrnd_next;
        done_reg <= done_next;
        enc_dec_reg <= enc_dec_next;
    end
end

//Debugging purposes

// wire [63 : 0] comp_unm;
// wire [63 : 0] mprime_unm;
// wire [63 : 0] srows_unm;
// wire [63 : 0] rk_add_unm;
// wire [63 : 0] in_reg_unm; 
// wire [63 : 0] sbox_unm; 
// wire [63 : 0] aff_out_unm; 
// wire [63 : 0] aff_in_unm; 
// wire [63 : 0] invkeyadd_out_unm;
// wire [63 : 0] inv_sr_unm;
// wire [63 : 0] output_unm;
// wire [63 : 0] mux2_out_unm;
// wire [63 : 0] mux3_out_unm;

// wire [15 : 0] sbox_bit1_unm;
// wire [15 : 0] sbox_bit2_unm;
// wire [15 : 0] sbox_bit3_unm;
// wire [15 : 0] sbox_bit4_unm;

// assign out_reg_unm = comp_sh1 ^ comp_sh2;

// assign mprime_unm = mprime_out1 ^ mprime_out2;
// assign srows_unm = srows_out1 ^ srows_out2;
// assign rk_add_unm = fwdkey_out1 ^ fwdkey_out2; 
// assign in_reg_unm = in_reg_out1 ^ in_reg_out2;
// assign sbox_bit1_unm = sbox_out1_b1 ^ sbox_out2_b1 ^ sbox_out3_b1 ^ sbox_out4_b1 ^ sbox_out5_b1 ^ sbox_out6_b1 ^ sbox_out7_b1 ^ sbox_out8_b1;
// assign sbox_bit2_unm = sbox_out1_b2 ^ sbox_out2_b2 ^ sbox_out3_b2 ^ sbox_out4_b2 ^ sbox_out5_b2 ^ sbox_out6_b2 ^ sbox_out7_b2 ^ sbox_out8_b2;
// assign sbox_bit3_unm = sbox_out1_b3 ^ sbox_out2_b3 ^ sbox_out3_b3 ^ sbox_out4_b3 ^ sbox_out5_b3 ^ sbox_out6_b3 ^ sbox_out7_b3 ^ sbox_out8_b3;
// assign sbox_bit4_unm = sbox_out1_b4 ^ sbox_out2_b4 ^ sbox_out3_b4 ^ sbox_out4_b4 ^ sbox_out5_b4 ^ sbox_out6_b4 ^ sbox_out7_b4 ^ sbox_out8_b4;

// generate
// for (i=0; i<16; i=i+1)begin : UNMASKING_SBOX
//     assign sbox_unm[3 + 4*i : 4*i] = {sbox_bit4_unm[i],sbox_bit3_unm[i],sbox_bit2_unm[i],sbox_bit1_unm[i]};
// end
// endgenerate

// assign aff_out_unm = outaff_out1 ^ outaff_out2;
// assign aff_in_unm = inaff_out1 ^ inaff_out2;
// assign invkeyadd_out_unm = invkeyadd_out1 ^ invkeyadd_out2;
// assign inv_sr_unm = inv_sr_out1 ^ inv_sr_out2;

// assign output_unm = o_ct1 ^ o_ct2;
// assign mux2_out_unm = mux2_out1 ^ mux2_out2;
// assign mux3_out_unm = mux3_out1 ^ mux3_out2;


endmodule




