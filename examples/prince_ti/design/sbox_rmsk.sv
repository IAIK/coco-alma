module sbox_rmsk (
    in1,
    in2,
    in3,
    in4,
    in_rand,
    out1,
    out2,
    out3,
    out4
);

input wire [7 : 0] in1;
input wire [7 : 0] in2;
input wire [7 : 0] in3;
input wire [7 : 0] in4;
//input wire [27 : 0] in_rand;
input wire [11 : 0] in_rand;

output wire [7 : 0] out1;
output wire [7 : 0] out2;
output wire [7 : 0] out3;
output wire [7 : 0] out4;

//wire [6 : 0] r1;
//wire [6 : 0] r2;
//wire [6 : 0] r3;
//wire [6 : 0] r4;
wire [2 : 0] r1;
wire [2 : 0] r2;
wire [2 : 0] r3;
wire [2 : 0] r4;
wire [2 : 0] r1_inv;
wire [2 : 0] r2_inv;
wire [2 : 0] r3_inv;
wire [2 : 0] r4_inv;

//assign r1 = in_rand[6 : 0];
//assign r2 = in_rand[13 : 7];
//assign r3 = in_rand[20 : 14];
//assign r4 = in_rand[27 : 21];
assign r1 = in_rand[2 : 0];
assign r2 = in_rand[5 : 3];
assign r3 = in_rand[8 : 6];
assign r4 = in_rand[11 : 9];
// assign r1_inv = {<<{r1}};
assign r1_inv = {r1[0],r1[1],r1[2]};
// assign r2_inv = {<<{r2}};
assign r2_inv = {r2[0],r2[1],r2[2]};
// assign r3_inv = {<<{r3}};
assign r3_inv = {r3[0],r3[1],r3[2]};
// assign r4_inv = {<<{r4}};
assign r4_inv = {r4[0],r4[1],r4[2]};

//First version
//assign out1[6 : 0] = in1[6 : 0] ^ r1[6 : 0];
//assign out1[7] = in1[7] ^ r1[6] ^ r1[5] ^ r1[4] ^ r1[3] ^ r1[2] ^ r1[1] ^ r1[0];

//assign out2[6 : 0] = in2[6 : 0] ^ r2[6 : 0];
//assign out2[7] = in2[7] ^ r2[6] ^ r2[5] ^ r2[4] ^ r2[3] ^ r2[2] ^ r2[1] ^ r2[0];

//assign out3[6 : 0] = in3[6 : 0] ^ r3[6 : 0];
//assign out3[7] = in3[7] ^ r3[6] ^ r3[5] ^ r3[4] ^ r3[3] ^ r3[2] ^ r3[1] ^ r3[0];

//assign out4[6 : 0] = in4[6 : 0] ^ r4[6 : 0];
//assign out4[7] = in4[7] ^ r4[6] ^ r4[5] ^ r4[4] ^ r4[3] ^ r4[2] ^ r4[1] ^ r4[0];
  assign out1 = {in1[7], in1[6:4] ^ r1, in1[3:1] ^ r1_inv, in1[0]};
  assign out2 = {in2[7], in2[6:4] ^ r2, in2[3:1] ^ r2_inv, in2[0]};
  assign out3 = {in3[7], in3[6:4] ^ r3, in3[3:1] ^ r3_inv, in3[0]};
  assign out4 = {in4[7], in4[6:4] ^ r4, in4[3:1] ^ r4_inv, in4[0]};

endmodule
