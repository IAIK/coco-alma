module s_bit1_sh5 (
  x2,
  w2,
  y1,
  z1,
  out5
);

    input wire x2;
    input wire w2;
    input wire y1;
    input wire z1;

    output wire out5;

    assign out5 = x2 ^ x2 & w2 ^ x2 & y1 ^ y1 & z1 & w2 ;

endmodule
