//---------------------------------------------------

module dom_and_1storder (clk_i, rst_i, 
    X0_i, X1_i, 
    Y0_i, Y1_i, 
    Z_i, 
    Q0_o, Q1_o
     );
    input clk_i; 
    input rst_i;
    input [7:0] X0_i, X1_i;
    input [7:0] Y0_i, Y1_i;
    input [7:0] Z_i;
    output [7:0] Q0_o, Q1_o; 

    //Same domain
    wire [7:0] X0_Y0,X1_Y1;

    assign X0_Y0 = X0_i & Y0_i;
    assign X1_Y1 = X1_i & Y1_i;

    //Cross domain + resharing
    wire [7:0] X0_Y1,X1_Y0;

    assign X0_Y1 = X0_i & Y1_i;
    assign X1_Y0 = X1_i & Y0_i;

    reg [7:0] X0_Y1_Z_q, X1_Y0_Z_q;

    always @(posedge clk_i) begin
        if(rst_i) begin
            X0_Y1_Z_q <= 8'b0;
            X1_Y0_Z_q <= 8'b0;
        end else begin
            X0_Y1_Z_q <= X0_Y1 ^ Z_i;
            X1_Y0_Z_q <= X1_Y0 ^ Z_i;
        end
    end

    assign Q0_o = X0_Y1_Z_q ^ X0_Y0;
    assign Q1_o = X1_Y0_Z_q ^ X1_Y1;

endmodule




//---------------------------------------------------

module dom_and_1storder_broken (clk_i, rst_i, 
    X0_i, X1_i, 
    Y0_i, Y1_i, 
    Z_i, 
    Q0_o, Q1_o
     );
    input clk_i; 
    input rst_i;
    input [7:0] X0_i, X1_i;
    input [7:0] Y0_i, Y1_i;
    input [7:0] Z_i;
    output [7:0] Q0_o, Q1_o; 

    //Same domain
    wire [7:0] X0_Y0;
    wire [7:0] X1_Y1;

    assign X0_Y0 = X0_i & Y0_i;
    assign X1_Y1 = X1_i & Y1_i;

    //Cross domain + resharing
    wire [7:0] X0_Y1;
    wire [7:0] X1_Y0;

    assign X0_Y1 = X0_i & Y1_i;
    assign X1_Y0 = X1_i & Y0_i;

    wire [7:0] X0_Y1_Z;
    wire [7:0] X1_Y0_Z;    

    assign X0_Y1_Z = X0_Y1 ^ Z_i;
    assign X1_Y0_Z = X1_Y0 ^ Z_i;
 
    assign Q0_o = X0_Y1_Z ^ X0_Y0;
    assign Q1_o = X1_Y0_Z ^ X1_Y1;

endmodule


//---------------------------------------------------




module dom_and_2ndorder (clk_i, rst_i, 
        X0_i, X1_i, X2_i,
        Y0_i, Y1_i, Y2_i,
        Z0_i, Z1_i, Z2_i,
        Q0_o, Q1_o, Q2_o
     );
    input clk_i; 
    input rst_i;
    input [7:0] X0_i, X1_i, X2_i;
    input [7:0] Y0_i, Y1_i, Y2_i;
    input [7:0] Z0_i, Z1_i, Z2_i;
    output [7:0] Q0_o, Q1_o, Q2_o; 

    //Same domain
    wire [7:0] AX_AY, BX_BY, CX_CY;

    assign AX_AY = X0_i & Y0_i;
    assign BX_BY = X1_i & Y1_i;
    assign CX_CY = X2_i & Y2_i;

    //Cross domain + resharing
    wire [7:0] AX_BY, AX_CY;

    wire [7:0] BX_AY, BX_CY;

    wire [7:0] CX_AY, CX_BY;

    assign AX_BY = X0_i & Y1_i;
    assign AX_CY = X0_i & Y2_i;
    
    assign BX_AY = X1_i & Y0_i;
    assign BX_CY = X1_i & Y2_i;

    assign CX_AY = X2_i & Y0_i;
    assign CX_BY = X2_i & Y1_i;


    reg [7:0] AX_BY_Z0_q, AX_CY_Z1_q;

    reg [7:0] BX_AY_Z0_q, BX_CY_Z2_q;

    reg [7:0] CX_AY_Z1_q, CX_BY_Z2_q;

    always @(posedge clk_i) begin
        if(rst_i) begin
            AX_BY_Z0_q <= 8'b0;
            AX_CY_Z1_q <= 8'b0;
            BX_AY_Z0_q <= 8'b0;
            BX_CY_Z2_q <= 8'b0;
            CX_AY_Z1_q <= 8'b0;
            CX_BY_Z2_q <= 8'b0;
        end else begin
            AX_BY_Z0_q <= AX_BY ^ Z0_i;
            AX_CY_Z1_q <= AX_CY ^ Z1_i;

            BX_AY_Z0_q <= BX_AY ^ Z0_i;
            BX_CY_Z2_q <= BX_CY ^ Z2_i;

            CX_AY_Z1_q <= CX_AY ^ Z1_i;
            CX_BY_Z2_q <= CX_BY ^ Z2_i;
        end
    end

    assign Q0_o = AX_BY_Z0_q ^ AX_CY_Z1_q ^ AX_AY;
    assign Q1_o = BX_AY_Z0_q ^ BX_CY_Z2_q ^ BX_BY;
    assign Q2_o = CX_AY_Z1_q ^ CX_BY_Z2_q ^ CX_CY;


endmodule

//---------------------------------------------------


module dom_and_3rdorder (clk_i, rst_i, 
        X0_i, X1_i, X2_i, X3_i,
        Y0_i, Y1_i, Y2_i, Y3_i,
        Z0_i, Z1_i, Z2_i, Z3_i, Z4_i, Z5_i,
        Q0_o, Q1_o, Q2_o, Q3_o);

    input clk_i; 
    input rst_i;
    input [7:0] X0_i, X1_i, X2_i, X3_i;
    input [7:0] Y0_i, Y1_i, Y2_i, Y3_i;
    input [7:0] Z0_i, Z1_i, Z2_i, Z3_i, Z4_i, Z5_i;
    output [7:0] Q0_o, Q1_o, Q2_o, Q3_o; 

    //Same domain
    wire [7:0] AX_AY;
    wire [7:0] BX_BY;
    wire [7:0] CX_CY;
    wire [7:0] DX_DY;

    assign AX_AY = X0_i & Y0_i;
    assign BX_BY = X1_i & Y1_i;
    assign CX_CY = X2_i & Y2_i;
    assign DX_DY = X3_i & Y3_i;

    //Cross domain + resharing
    wire [7:0] AX_BY;
    wire [7:0] AX_CY;
    wire [7:0] AX_DY;

    wire [7:0] BX_AY;
    wire [7:0] BX_CY;
    wire [7:0] BX_DY;

    wire [7:0] CX_AY;
    wire [7:0] CX_BY;
    wire [7:0] CX_DY;

    wire [7:0] DX_AY;
    wire [7:0] DX_BY;
    wire [7:0] DX_CY;

    assign AX_BY = X0_i & Y1_i;
    assign AX_CY = X0_i & Y2_i;
    assign AX_DY = X0_i & Y3_i;
    
    assign BX_AY = X1_i & Y0_i;
    assign BX_CY = X1_i & Y2_i;
    assign BX_DY = X1_i & Y3_i;

    assign CX_AY = X2_i & Y0_i;
    assign CX_BY = X2_i & Y1_i;
    assign CX_DY = X2_i & Y3_i;

    assign DX_AY = X3_i & Y0_i;
    assign DX_BY = X3_i & Y1_i;
    assign DX_CY = X3_i & Y2_i;


    reg [7:0] AX_BY_Z0_q;
    reg [7:0] AX_CY_Z1_q;
    reg [7:0] AX_DY_Z3_q;

    reg [7:0] BX_AY_Z0_q;
    reg [7:0] BX_CY_Z2_q;
    reg [7:0] BX_DY_Z4_q;

    reg [7:0] CX_AY_Z1_q;
    reg [7:0] CX_BY_Z2_q;
    reg [7:0] CX_DY_Z5_q;

    reg [7:0] DX_AY_Z3_q;
    reg [7:0] DX_BY_Z4_q;
    reg [7:0] DX_CY_Z5_q;

    

    always @(posedge clk_i) begin
        if(rst_i) begin
            AX_BY_Z0_q <= 8'b0;
            AX_CY_Z1_q <= 8'b0;
            AX_DY_Z3_q <= 8'b0;

            BX_AY_Z0_q <= 8'b0;
            BX_CY_Z2_q <= 8'b0;
            BX_DY_Z4_q <= 8'b0;

            CX_AY_Z1_q <= 8'b0;
            CX_BY_Z2_q <= 8'b0;
            CX_DY_Z5_q <= 8'b0;

            DX_AY_Z3_q <= 8'b0;
            DX_BY_Z4_q <= 8'b0;
            DX_CY_Z5_q <= 8'b0;
        end else begin
            AX_BY_Z0_q <= AX_BY ^ Z0_i;
            AX_CY_Z1_q <= AX_CY ^ Z1_i;
            AX_DY_Z3_q <= AX_DY ^ Z3_i;

            BX_AY_Z0_q <= BX_AY ^ Z0_i;
            BX_CY_Z2_q <= BX_CY ^ Z2_i;
            BX_DY_Z4_q <= BX_DY ^ Z4_i;

            CX_AY_Z1_q <= CX_AY ^ Z1_i;
            CX_BY_Z2_q <= CX_BY ^ Z2_i;
            CX_DY_Z5_q <= CX_DY ^ Z5_i;

            DX_AY_Z3_q <= DX_AY ^ Z3_i;
            DX_BY_Z4_q <= DX_BY ^ Z4_i;
            DX_CY_Z5_q <= DX_CY ^ Z5_i;
        end
    end

    assign Q0_o = AX_BY_Z0_q ^ AX_CY_Z1_q ^ AX_DY_Z3_q ^ AX_AY;
    assign Q1_o = BX_AY_Z0_q ^ BX_CY_Z2_q ^ BX_DY_Z4_q ^ BX_BY;
    assign Q2_o = CX_AY_Z1_q ^ CX_BY_Z2_q ^ CX_DY_Z5_q ^ CX_CY;
    assign Q3_o = DX_AY_Z3_q ^ DX_BY_Z4_q ^ DX_CY_Z5_q ^ DX_DY;


endmodule