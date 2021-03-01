module s_bit4_sh1 (
  w1,
  z1,
  y1,
  x1,
  out1
);

    input wire w1;
    input wire z1;
    input wire y1;
    input wire x1;

    output wire out1;

    assign out1 = 1'b1 ^ z1 ^ x1 ^ y1 & z1 ^ x1 & y1 ^ y1 & z1 & w1 ^ x1 & z1 & w1 ^ x1 & y1 & w1 ;

endmodule
