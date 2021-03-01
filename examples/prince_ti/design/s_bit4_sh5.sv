module s_bit4_sh5 (
  x2,
  z1,
  y1,
  w2,
  out5
);

    input wire x2;
    input wire z1;
    input wire y1;
    input wire w2;

    output wire out5;

    assign out5 = x2 ^ x2 & y1 ^ y1 & z1 & w2 ^ x2 & z1 & w2 ^ x2 & y1 & w2 ;

endmodule
