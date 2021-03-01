module affine_share2 (

    i_in2,
    o_out2
);         

    input wire [0:3] i_in2;
    output wire [0:3] o_out2;

    wire x2, w2, v2, u2;

    assign x2 = i_in2[0];

    assign w2 = i_in2[1];

    assign v2 = i_in2[2];

    assign u2 = i_in2[3];

    assign o_out2[3] = v2 ;

    assign o_out2[2] = u2 ^ v2 ^ w2 ;

    assign o_out2[1] = x2 ;

    assign o_out2[0] = w2 ;

endmodule

