module out_key_share_flip(
    in_key,
    out_key
);

input wire [63 : 0] in_key;
output wire [63 : 0] out_key;

assign out_key = {in_key[0], in_key[63 : 2], in_key[1] ^ in_key[63]};

endmodule
