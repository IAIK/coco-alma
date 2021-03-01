module mux_cir(
    sel,
    a,
    b,
    out
);

parameter N = 64;
input wire sel;

input wire [N - 1 : 0] a;
input wire [N - 1 : 0] b;

output wire [N - 1 : 0] out;

assign out = sel? a : b;

endmodule

