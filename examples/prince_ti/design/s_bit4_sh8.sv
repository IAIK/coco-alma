module s_bit4_sh8 (
  x2,
  y2,
  w2,
  z2,
  out8
);

    input wire x2;
    input wire y2;
    input wire w2;
    input wire z2;

    output wire out8;

    assign out8 = y2 & z2 & w2 ^ x2 & z2 & w2 ^ x2 & y2 & w2 ;

endmodule
