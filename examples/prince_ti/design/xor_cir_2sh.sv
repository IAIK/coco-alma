module xor_cir_2sh(
    a1,
    a2,
    b1,
    b2,
    out1,
    out2
);

    parameter N = 64;

    input wire [N - 1 : 0] a1;
    input wire [N - 1 : 0] a2;
    input wire [N - 1 : 0] b1;
    input wire [N - 1 : 0] b2;

    output wire [N - 1 : 0] out1;
    output wire [N - 1 : 0] out2;

    xor_cir sh1(
        .a   (a1),
        .b   (b1),
        .out (out1)
    );

    xor_cir sh2(
        .a   (a2),
        .b   (b2),
        .out (out2)
    );

endmodule


