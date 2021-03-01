module M0_Share1(
    i_in1,
    o_out1
);

input wire [0:15] i_in1;
output wire [0:15] o_out1;

assign o_out1[ 0] = i_in1[4] ^ i_in1[ 8] ^ i_in1[12];
assign o_out1[ 1] = i_in1[1] ^ i_in1[ 9] ^ i_in1[13];
assign o_out1[ 2] = i_in1[2] ^ i_in1[ 6] ^ i_in1[14];
assign o_out1[ 3] = i_in1[3] ^ i_in1[ 7] ^ i_in1[11];
assign o_out1[ 4] = i_in1[0] ^ i_in1[ 4] ^ i_in1[ 8];
assign o_out1[ 5] = i_in1[5] ^ i_in1[ 9] ^ i_in1[13];
assign o_out1[ 6] = i_in1[2] ^ i_in1[10] ^ i_in1[14];
assign o_out1[ 7] = i_in1[3] ^ i_in1[ 7] ^ i_in1[15];
assign o_out1[ 8] = i_in1[0] ^ i_in1[ 4] ^ i_in1[12];
assign o_out1[ 9] = i_in1[1] ^ i_in1[ 5] ^ i_in1[ 9];
assign o_out1[10] = i_in1[6] ^ i_in1[10] ^ i_in1[14];
assign o_out1[11] = i_in1[3] ^ i_in1[11] ^ i_in1[15];
assign o_out1[12] = i_in1[0] ^ i_in1[ 8] ^ i_in1[12];
assign o_out1[13] = i_in1[1] ^ i_in1[ 5] ^ i_in1[13];
assign o_out1[14] = i_in1[2] ^ i_in1[ 6] ^ i_in1[10];
assign o_out1[15] = i_in1[7] ^ i_in1[11] ^ i_in1[15];

endmodule

