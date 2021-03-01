module s_bit1_sh7 (
  x2,
  y2,
  w1,
  z1,
  out7
);

    input wire x2;
    input wire y2;
    input wire w1;
    input wire z1;

    output wire out7;

    assign out7 = x2 & y2 ^ y2 & z1 & w1 ;

endmodule
