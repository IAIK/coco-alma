module s_bit3_sh2 (
  w2,
  z2,
  y1,
  x1,
  out2
);

    input wire w2;
    input wire z2;
    input wire y1;
    input wire x1;

    output wire out2;

    assign out2 = w2 ^ z2 & w2 ^ x1 & w2 ^ x1 & z2 ^ x1 & z2 & w2 ^ x1 & y1 & z2 ;

endmodule
