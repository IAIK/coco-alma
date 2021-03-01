module s (
  in1,
  in2,
  out1,
  out2,
  out3,
  out4
);

    input wire [3:0] in1;
    input wire [3:0] in2;
    output wire [7:0] out1;
    output wire [7:0] out2;
    output wire [7:0] out3;
    output wire [7:0] out4;

    s_bit1 sb1_inst (
      .in1 (in1),
      .in2 (in2),
      .out (out1)
    );

    s_bit2 sb2_inst (
      .in1 (in1),
      .in2 (in2),
      .out (out2)
    );

    s_bit3 sb3_inst (
      .in1 (in1),
      .in2 (in2),
      .out (out3)
    );

    s_bit4 sb4_inst (
      .in1 (in1),
      .in2 (in2),
      .out (out4)
    );

endmodule
