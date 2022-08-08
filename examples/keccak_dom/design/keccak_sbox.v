module DOM_and (clk_i, rst_i, 
    X0_i, X1_i, 
    Y0_i, Y1_i, 
    Z_i, 
    Q0_o, Q1_o
     );
    input clk_i; 
    input rst_i;
    input [31:0] X0_i;
    input [31:0] X1_i;
    input [31:0] Y0_i; 
    input [31:0] Y1_i; 
    input [31:0] Z_i;
    output [31:0] Q0_o; 
    output [31:0] Q1_o; 
    //Same domain
    wire [31:0] X0_Y0;
    wire [31:0] X1_Y1;

    assign X0_Y0 = X0_i & Y0_i;
    assign X1_Y1 = X1_i & Y1_i;

    //Cross domain + resharing
    wire [31:0] X0_Y1;
    wire [31:0] X1_Y0;

    assign X0_Y1 = X0_i & Y1_i;
    assign X1_Y0 = X1_i & Y0_i;

    reg [31:0] X0_Y1_Z;
    reg [31:0] X1_Y0_Z;    

    always @(posedge clk_i) begin
        if(rst_i) begin
            X0_Y1_Z <= 32'b0;
            X1_Y0_Z <= 32'b0;
        end else begin
            X0_Y1_Z <= X0_Y1 ^ Z_i;
            X1_Y0_Z <= X1_Y0 ^ Z_i;
        end
    end

    assign Q0_o = X0_Y1_Z ^ X0_Y0;
    assign Q1_o = X1_Y0_Z ^ X1_Y1;

endmodule


module keccak_sbox (clk_i, rst_i, 
    A0_i, A1_i, B0_i, B1_i, C0_i, C1_i, D0_i, D1_i, E0_i, E1_i,
    rand_i,
    A0_o, A1_o, B0_o, B1_o, C0_o, C1_o, D0_o, D1_o, E0_o, E1_o);

    input clk_i;
    input rst_i;
    input [31:0] rand_i;

    input [31:0]  A0_i, A1_i, B0_i, B1_i, C0_i, C1_i, D0_i, D1_i, E0_i, E1_i;
    output [31:0] A0_o, A1_o, B0_o, B1_o, C0_o, C1_o, D0_o, D1_o, E0_o, E1_o;

    wire [31:0] n_A0, n_A1, n_B0, n_B1, n_C0, n_C1, n_D0, n_D1, n_E0, n_E1;
    
    assign n_A0 = ~A0_i;
    assign n_B0 = ~B0_i;
    assign n_C0 = ~C0_i;
    assign n_D0 = ~D0_i;
    assign n_E0 = ~E0_i;

    assign n_A1 = A1_i;
    assign n_B1 = B1_i;
    assign n_C1 = C1_i;
    assign n_D1 = D1_i;
    assign n_E1 = E1_i;

   
    wire [31:0] AB0, AB1;
    wire [31:0] BC0, BC1;
    wire [31:0] CD0, CD1;
    wire [31:0] DE0, DE1;
    wire [31:0] EA0, EA1;


    DOM_and dom_and0 (clk_i, rst_i, n_A0, n_A1, B0_i, B1_i, rand_i, AB0, AB1);
    DOM_and dom_and1 (clk_i, rst_i, n_B0, n_B1, C0_i, C1_i, rand_i, BC0, BC1);
    DOM_and dom_and2 (clk_i, rst_i, n_C0, n_C1, D0_i, D1_i, rand_i, CD0, CD1);
    DOM_and dom_and3 (clk_i, rst_i, n_D0, n_D1, E0_i, E1_i, rand_i, DE0, DE1);
    DOM_and dom_and4 (clk_i, rst_i, n_E0, n_E1, A0_i, A1_i, rand_i, EA0, EA1);

  
    assign A0_o = BC0 ^ A0_i;
    assign B0_o = CD0 ^ B0_i;
    assign C0_o = DE0 ^ C0_i;
    assign D0_o = EA0 ^ D0_i;
    assign E0_o = AB0 ^ E0_i;

    assign A1_o = BC1 ^ A1_i;
    assign B1_o = CD1 ^ B1_i;
    assign C1_o = DE1 ^ C1_i;
    assign D1_o = EA1 ^ D1_i;
    assign E1_o = AB1 ^ E1_i;



endmodule
