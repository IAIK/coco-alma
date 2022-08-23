.org 0x80
nop
xor x15, x15, x17 # r15 = {share 1, static_random}
xor x17, x15, x17 # r17 = {share 1}
xor x15, x17, x16 # r15 = {share 1, share 1}
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
