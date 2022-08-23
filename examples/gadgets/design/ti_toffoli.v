//https://tches.iacr.org/index.php/TCHES/article/view/7270/6448, Sec 3.1
module ti_toffoli (clk_i, rst_i, 
    X1_i, X2_i, X3_i, 
    Y1_i, Y2_i, Y3_i, 
    Z1_i, Z2_i, Z3_i,
    C1_o, C2_o, C3_o
     );
    input clk_i; 
    input rst_i;
    input [7:0] X1_i, X2_i, X3_i;
    input [7:0] Y1_i, Y2_i, Y3_i;
    input [7:0] Z1_i, Z2_i, Z3_i;
    output [7:0] C1_o, C2_o, C3_o; 

    assign C1_o = (X2_i & Y2_i ) ^ (X2_i & Y3_i) ^ (X3_i & Y2_i) ^ Z2_i;
    assign C2_o = (X3_i & Y3_i ) ^ (X3_i & Y1_i) ^ (X1_i & Y3_i) ^ Z3_i;
    assign C3_o = (X1_i & Y1_i ) ^ (X1_i & Y2_i) ^ (X2_i & Y1_i) ^ Z1_i;
endmodule