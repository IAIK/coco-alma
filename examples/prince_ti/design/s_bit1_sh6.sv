module s_bit1_sh6 (
  x2,
  w1,
  y1,
  z2,
  out6
);

    input wire x2;
    input wire w1;
    input wire y1;
    input wire z2;

    output wire out6;

    assign out6 = x2 & w1 ^ y1 & z2 & w1 ;

endmodule
