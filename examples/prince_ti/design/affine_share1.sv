module affine_share1 (

    i_in1,
    o_out1
);        
    input wire [0:3] i_in1;
    output wire [0:3] o_out1;

    wire x1, w1, v1, u1;

    assign x1 = i_in1[0];

    assign w1 = i_in1[1];

    assign v1 = i_in1[2];

    assign u1 = i_in1[3];
    assign o_out1[3] = 1 ^ v1 ;

    assign o_out1[2] = u1 ^ v1 ^ w1 ;

    assign o_out1[1] = 1 ^ x1 ;

    assign o_out1[0] = w1 ;

endmodule

