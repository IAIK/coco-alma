module Mprime_cir(
    in,
    out
);

input wire [0:63] in;
output wire [0:63] out;

M0_Share1 m00_Share1(
    .i_in1 (in[0:15]),
    .o_out1 (out[0:15])
);

M1_Share1 m10_Share1(
    .i_in1 (in[16:31]),
    .o_out1 (out[16:31])
);

M1_Share1 m11_Share1(
    .i_in1 (in[32:47]),
    .o_out1 (out[32:47])
);

M0_Share1 m01_Share1(
    .i_in1 (in[48:63]),
    .o_out1 (out[48:63])
);

endmodule

