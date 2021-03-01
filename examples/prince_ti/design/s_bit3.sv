module s_bit3 (
  in1,
  in2,
  out
);

    input wire [3:0] in1;
    input wire [3:0] in2;
    output wire [7:0] out;

    wire x1, y1, z1, w1;
    wire x2, y2, z2, w2;

    assign x1 = in1[3];
    assign x2 = in2[3];

    assign y1 = in1[2];
    assign y2 = in2[2];

    assign z1 = in1[1];
    assign z2 = in2[1];

    assign w1 = in1[0];
    assign w2 = in2[0];

    s_bit3_sh1 s_bit31_inst (
      .w1 (w1),
      .z1 (z1),
      .y1 (y1),
      .x1 (x1),
      .out1 (out[0])
    );

    s_bit3_sh2 s_bit32_inst (
      .w2 (w2),
      .z2 (z2),
      .y1 (y1),
      .x1 (x1),
      .out2 (out[1])
    );

    s_bit3_sh3 s_bit33_inst (
      .y2 (y2),
      .z1 (z1),
      .w2 (w2),
      .x1 (x1),
      .out3 (out[2])
    );

    s_bit3_sh4 s_bit34_inst (
      .y2 (y2),
      .w1 (w1),
      .z2 (z2),
      .x1 (x1),
      .out4 (out[3])
    );

    s_bit3_sh5 s_bit35_inst (
      .x2 (x2),
      .w2 (w2),
      .z1 (z1),
      .y1 (y1),
      .out5 (out[4])
    );

    s_bit3_sh6 s_bit36_inst (
      .x2 (x2),
      .w1 (w1),
      .z2 (z2),
      .y1 (y1),
      .out6 (out[5])
    );

    s_bit3_sh7 s_bit37_inst (
      .x2 (x2),
      .y2 (y2),
      .w1 (w1),
      .z1 (z1),
      .out7 (out[6])
    );

    s_bit3_sh8 s_bit38_inst (
      .x2 (x2),
      .y2 (y2),
      .w2 (w2),
      .z2 (z2),
      .out8 (out[7])
    );

endmodule
