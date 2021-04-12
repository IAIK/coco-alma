module aes_wrapper(ClkxCI, RstxBI, PTxDI, KxDI, Zmul1xDI, Zmul2xDI, Zmul3xDI, Zinv1xDI, Zinv2xDI, Zinv3xDI, Bmul1xDI, Binv1xDI, Binv2xDI, Binv3xDI, StartxSI, DonexSO, CxDO);
  parameter N = 1;
  
  input ClkxCI;
  input RstxBI;
  input StartxSI;
  
  input [4*(N*(N+1)/2)-1:0] Zmul1xDI;
  input [4*(N*(N+1)/2)-1:0] Zmul2xDI;
  input [4*(N*(N+1)/2)-1:0] Zmul3xDI;
  input [2*(N*(N+1)/2)-1:0] Zinv1xDI;
  input [2*(N*(N+1)/2)-1:0] Zinv2xDI;
  input [2*(N*(N+1)/2)-1:0] Zinv3xDI;
  
  input [4*(N+1)-1:0] Bmul1xDI;
  input [4*(N+1)-1:0] Binv1xDI;
  input [4*(N+1)-1:0] Binv2xDI;
  input [2*(N+1)-1:0] Binv3xDI;
  
  // organized as (AF,BF), ..., (A0, B0)
  
  input [(N+1)*8*16-1:0] KxDI;
  input [(N+1)*8*16-1:0] PTxDI;
  output [(N+1)*8*16-1:0] CxDO;
  
  reg   [(N+1)*8*16-1:0] key;
  reg   [(N+1)*8*16-1:0] plain;
  reg   [(N+1)*8*16-1:0] cipher;
  wire  [(N+1)*8*16-1:0] next_key;
  wire  [(N+1)*8*16-1:0] next_plain;
  wire  [(N+1)*8*16-1:0] next_cipher;
  
  output DonexSO;
  
  parameter IDLE = 3'b000, LOAD = 3'b001, COMPUTE = 3'b010, STORE = 3'b011, DONE = 3'b100;
  
  reg [2:0] state;
  reg [3:0] counter;
  wire [2:0] next_state;
  wire [3:0] next_counter;
  
  
  wire e_done;
  wire [8*(N+1)-1:0] e_pt;
  wire [8*(N+1)-1:0] e_key;
  wire [8*(N+1)-1:0] e_ct;
  
  assign e_pt = plain[8*(N+1)*counter +: 8*(N+1)];
  assign e_key = key[8*(N+1)*counter +: 8*(N+1)];
  
  aes_top design(ClkxCI, RstxBI, e_pt, e_key, 
    Zmul1xDI, Zmul2xDI, Zmul3xDI, Zinv1xDI, 
    Zinv2xDI, Zinv3xDI, Bmul1xDI, Binv1xDI, 
    Binv2xDI, Binv3xDI, StartxSI, e_done, e_ct);
  
  always @(posedge ClkxCI or negedge RstxBI) begin
    if(!RstxBI) begin
        state <= IDLE;
        counter <= 0;
        key <= 0;
        plain <= 0;
        cipher <= 0;
    end else begin
        state <= next_state;
        counter <= next_counter;
        key <= next_key;
        plain <= next_plain;
        cipher <= next_cipher;
    end
  end  
  always @(*) begin
      next_state = state;
      next_counter = counter;
      next_key = key;
      next_plain = plain;
      next_cipher = cipher;
      
      case(state)
        IDLE: begin
            if (StartxSI) begin
                next_state = LOAD;
                next_key = KxDI;
                next_plain = PTxDI;
            end
        end
        LOAD: begin
            next_counter = counter + 1;
            if (counter == 4'hf) next_state = COMPUTE;
        end
        COMPUTE: begin
            next_counter = 0;
            if (e_done) begin
                next_state = STORE;
                next_counter = counter + 1;
            end
        end
        STORE: begin
            next_counter = counter + 1;
            if (counter == 4'hf) next_state = DONE;
        end
        DONE:
            next_state = IDLE;
      endcase
      
      if (state == STORE || next_state == STORE)
        next_cipher[8*(N+1)*counter +: 8*(N+1)] = e_ct;
  end
    
  assign CxDO = cipher;
  assign DonexSO = (state == DONE);
  
endmodule
