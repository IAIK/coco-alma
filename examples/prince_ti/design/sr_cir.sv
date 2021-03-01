
module sr_cir(
    in,
    out
);

input wire [0 : 63] in;
output wire [0 : 63] out;

genvar  i;
generate
  for  ( i  =  0 ;  i  <  16 ;  i  =  i + 1 )
    begin  :  mix
       assign  out[((52*i) % 64):((52*i) % 64) + 3] = in[i*4 : i*4 + 3];
    end
endgenerate

endmodule
