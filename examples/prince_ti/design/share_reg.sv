module share_reg(
    i_clk,
    i_d,
    o_q
);

input wire i_clk;
input wire i_d;

output wire o_q;

reg state;

always @(posedge i_clk )
begin
        state <= i_d;
end

assign o_q = state;
endmodule

