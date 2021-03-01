module s_bit1_sh4 (
  y2,
  w1,
  z2,
  out4
);

    input wire y2;
    input wire w1;
    input wire z2;

    output wire out4;

    assign out4 = z2 & w1 ^ y2 & z2 ^ y2 & z2 & w1 ;

endmodule
