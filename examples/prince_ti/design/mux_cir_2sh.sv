module mux_cir_2sh(
    sel,
    a1,
    a2,
    b1,
    b2,
    out1,
    out2
);

    parameter N = 64;

    input wire sel;
    input wire [N - 1 : 0] a1;
    input wire [N - 1 : 0] a2;
    input wire [N - 1 : 0] b1;
    input wire [N - 1 : 0] b2;

    output wire [N - 1 : 0] out1;
    output wire [N - 1 : 0] out2;

    mux_cir sh1(
        .sel (sel),
        .a   (a1),
        .b   (b1),
        .out (out1)
    );

    mux_cir sh2(
        .sel (sel),
        .a   (a2),
        .b   (b2),
        .out (out2)
    );


endmodule


