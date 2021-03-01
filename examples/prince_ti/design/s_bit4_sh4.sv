module s_bit4_sh4 (
  y2,
  w1,
  z2,
  x1,
  out4
);

    input wire y2;
    input wire w1;
    input wire z2;
    input wire x1;

    output wire out4;

    assign out4 = y2 & z2 ^ y2 & z2 & w1 ^ x1 & z2 & w1 ^ x1 & y2 & w1 ;

endmodule
