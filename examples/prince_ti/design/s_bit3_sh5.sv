module s_bit3_sh5 (
  x2,
  w2,
  z1,
  y1,
  out5
);

    input wire x2;
    input wire w2;
    input wire z1;
    input wire y1;

    output wire out5;

    assign out5 = x2 ^ x2 & w2 ^ x2 & z1 ^ x2 & z1 & w2 ^ x2 & y1 & z1 ;

endmodule
