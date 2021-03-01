module sr_2sh(
    in1,
    in2,
    out1,
    out2
);

    parameter N = 64;

    input wire [N - 1 : 0] in1;
    input wire [N - 1 : 0] in2;

    output wire [N - 1 : 0] out1;
    output wire [N - 1 : 0] out2;

    sr_cir sh1(
        .in   (in1),
        .out (out1)
    );

    sr_cir sh2(
        .in   (in2),
        .out (out2)
    );


endmodule

