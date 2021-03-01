module affine_2sh(
    in1,
    in2,
    out1,
    out2
);

parameter N=64;

input wire [N - 1 : 0] in1;
input wire [N - 1 : 0] in2;

output wire [N - 1 : 0] out1;
output wire [N - 1 : 0] out2;

genvar i;
generate
for (i=0; i<16; i= i + 1) begin : SHARE1
    affine_share1 sh1(
        .i_in1 (in1[3 + i * 4  : i * 4]),
        .o_out1 (out1[3 + i * 4  : i * 4])
    );
end
endgenerate

generate
for (i=0; i<16; i= i + 1) begin : SHARE2
    affine_share2 sh2(
        .i_in2 (in2[3 + i * 4  : i * 4]),
        .o_out2 (out2[3 + i * 4  : i * 4])
    );
end
endgenerate

endmodule


