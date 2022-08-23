//---------------------------------------------------

module isw_and_1storder_broken (clk_i, rst_i, 
    X0_i, X1_i, 
    Y0_i, Y1_i, 
    R01_i, 
    Q0_o, Q1_o
     );
    input clk_i; 
    input rst_i;
    input [7:0] X0_i, X1_i;
    input [7:0] Y0_i, Y1_i;
    input [7:0] R01_i;
    output [7:0] Q0_o, Q1_o; 


    wire [7:0] R10;
    assign R10 = (R01_i ^ (X0_i&Y1_i)) ^ (X1_i&Y0_i);

    wire [7:0] C0, C1;
    assign C0 = X0_i & Y0_i;
    assign C1 = X1_i & Y1_i;

    assign Q0_o = C0 ^ R01_i;
    assign Q1_o = C1 ^ R10;

endmodule

module isw_and_1storder (clk_i, rst_i, 
    X0_i, X1_i, 
    Y0_i, Y1_i, 
    R01_i, 
    Q0_o, Q1_o
     );
    input clk_i; 
    input rst_i;
    input [7:0] X0_i, X1_i;
    input [7:0] Y0_i, Y1_i;
    input [7:0] R01_i;
    output [7:0] Q0_o, Q1_o; 

    reg [7:0] tmp0_q;
    reg [7:0] tmp1_q;
    reg [7:0] C0_q, C1_q;
    reg [7:0] R10_q;


    always @(posedge clk_i) begin
        if(rst_i) begin
            tmp0_q <= 8'b0;
            tmp1_q <= 8'b0;

            C0_q <= 8'b0;
            C1_q <= 8'b0;

            R10_q <= 8'b0;
        end else begin
            tmp0_q <= (R01_i ^ (X0_i&Y1_i));
            tmp1_q <= (X1_i&Y0_i);
    
            C0_q <= X0_i & Y0_i;
            C1_q <= X1_i & Y1_i;
             R10_q <= tmp0_q ^ tmp1_q;
        end
    end

    assign Q0_o = C0_q ^ R01_i;
    assign Q1_o = C1_q ^ R10_q;

endmodule




//---------------------------------------------------
