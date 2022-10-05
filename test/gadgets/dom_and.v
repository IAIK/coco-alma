module dom_and(ClkCI, RstRI, LeftDI, RightDI, RandomDI, OutDO);
    parameter NUM_SHARES = 2;
    localparam NUM_MASKS = (NUM_SHARES - 1) * (NUM_SHARES) / 2;

    input ClkCI;
    input RstRI;
    input [NUM_SHARES-1:0] LeftDI;
    input [NUM_SHARES-1:0] RightDI;
    input [NUM_MASKS-1:0]  RandomDI;

    output [NUM_SHARES-1:0] OutDO;
    genvar i, j;
    
    // First layer (and gates) as a 2d map
    wire [NUM_SHARES*NUM_SHARES-1:0] CrossD;
    generate
        for (i = 0; i < NUM_SHARES; i++)
            begin
                for (j = 0; j < NUM_SHARES; j++)
                begin
                    assign CrossD[i*NUM_SHARES + j] = LeftDI[i] & RightDI[j];
                end
            end
    endgenerate

    // Create the 2d map of the randomness
    wire [NUM_SHARES*NUM_SHARES-1:0] RandomMapD;
    generate
        for (i = 0; i < NUM_SHARES; i++)
            begin
                for (j = 0; j < i; j++)
                    assign RandomMapD[i*NUM_SHARES + j] = RandomDI[j + i * (i - 1)/2];
                assign RandomMapD[i*NUM_SHARES + i] = 1'b0;
                for (j = i+1; j < NUM_SHARES; j++)
                    assign RandomMapD[i*NUM_SHARES + j] = RandomDI[i + j * (j - 1)/2];
            end
    endgenerate

    wire [NUM_SHARES*NUM_SHARES-1:0] BlindedD;
    assign BlindedD = CrossD ^ RandomMapD;

    // Assign everything into registers
    wire [NUM_SHARES*NUM_SHARES-1:0] BufferedDN;
    reg  [NUM_SHARES*NUM_SHARES-1:0] BufferedDP;
    
    assign BufferedDN = BlindedD;
    
    always_ff @(posedge ClkCI, negedge RstRI) begin
        if (~RstRI) begin
            BufferedDP <= 'b0;
        end else begin
            BufferedDP <= BufferedDN;
        end
    end


    // Collapse everything into correct number of output shares
    generate
        for (i = 0; i < NUM_SHARES; i++)
        begin
            assign OutDO[i] = ^BufferedDP[NUM_SHARES*(i+1)-1:NUM_SHARES * i];
        end
    endgenerate

endmodule
