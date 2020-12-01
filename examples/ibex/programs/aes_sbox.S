.macro dom_and a0, a1, b0, b1, R, c0, c1
      and x0, x0, x0
      and x31, \a0, \b1
      xor x31, x31, \R
      and x0, x0, x0
      and \c0, \a0, \b0
      xor \c0, \c0, x31
      and x31, x31, x0
      and x31, \a1, \b1
      xor x31, x31, \R
      and x0, x0, x0
      and \c1, \a1, \b0
      xor \c1, \c1, x31
      and x31, x31, x0      
.endm

.org 0x80
li x18, 0x40
li x31, 0x0
jal x30, top_linear
jal x30, clear_all
li x18, 0x44
li x31, 0x4
jal x30, top_linear
jal x30, clear_all
#######################
#//Part1
li x30, 0x38
lw x15, (x30)
lw x16, 4(x30)
lw x3, 48(x30)
lw x4, 52(x30)
lw x8, 64(x30)
lw x9, 68(x30)
lw x1, 104(x30)
lw x2, 108(x30)
lw x11, 112(x30)
lw x12, 116(x30)
li x30, 0xd0
lw x13, (x30)
lw x14, 4(x30)
lw x6, 32(x30)
lw x7, 36(x30)
lw x18, 40(x30)
lw x19, 44(x30)
li x30, 0x160
lw x5, (x30)
lw x10, 4(x30)
lw x17, 8(x30)
dom_and x1, x2, x3, x4, x5, x20, x21
dom_and x6, x7, x8, x9, x10, x22, x23
xor x11, x11, x20
xor x11, x11, x22
and x0, x0, x0
xor x12, x12, x21
xor x12, x12, x23
dom_and x13, x14, x15, x16, x17, x24, x25
and x0, x0, x0
xor x24, x24, x20
and x0, x0, x0
xor x24, x24, x18
and x0, x0, x0
xor x25, x25, x21
and x0, x0, x0
xor x25, x25, x19
and x0, x0, x0
sw x24, 4(x30)
and x0, x0, x0
sw x25, 8(x30)
#//Part2
li x30, 0x40
lw x18, (x30)
lw x19, 4(x30)
lw x1, 16(x30)
lw x2, 20(x30)
lw x23, 24(x30)
lw x24, 28(x30)
lw x28, 64(x30)
lw x29, 68(x30)
lw x20, 112(x30)
lw x21, 116(x30)
lw x3, 120(x30)
lw x4, 124(x30)
li x30, 0xc0
lw x15, (x30)
lw x16, 4(x30)
lw x13, 24(x30)
lw x14, 28(x30)
lw x8, 40(x30)
lw x9, 44(x30)
lw x6, 72(x30)
lw x7, 76(x30)
lw x25, 80(x30)
lw x26, 84(x30)
li x30, 0x16c
lw x5, (x30)
lw x10, 4(x30)
lw x17, 8(x30)
lw x22, 12(x30)
lw x27, 16(x30)
dom_and x1, x2, x3, x4, x5, x1, x2
and x3, x0, x0 
and x0, x0, x0
xor x3, x6, x1
and x0, x0, x0
xor x4, x7, x2
dom_and x8, x9, x28, x29, x10, x5, x30
and x0, x0, x0
xor x3, x3, x5
and x0, x0, x0
xor x4, x4, x30
dom_and x13, x14, x15, x16, x17, x5, x6
and x0, x0, x0
xor x5, x5, x1
and x0, x0, x0
xor x6, x6, x2
dom_and x18, x19, x20, x21, x22, x7, x8
dom_and x23, x24, x25, x26, x27, x9, x10
and x0, x0, x0
xor x9, x9, x7
xor x11, x11, x9
xor x3, x3, x9
and x0, x0, x0
xor x10, x10, x8
xor x12, x12, x10
xor x4, x4, x10
#//Part3
li x30, 0x48
lw x9, (x30)
lw x10, 4(x30)
lw x13, 64(x30)
lw x14, 68(x30)
li x30, 0x100
and x16, x16, x0
lw x16, (x30)
lw x17, 4(x30)
lw x1, 100(x30)
lw x2, 104(x30)
li x30, 0x180
lw x15, (x30)
lw x22, 4(x30)
dom_and x9, x10, x13, x14, x15, x18, x19
and x0, x0, x0
xor x18, x18, x7
xor x1, x1, x18
xor x5, x5, x18
xor x5, x5, x16
xor x20, x3, x5
and x0, x0, x0
xor x19, x19, x8
xor x2, x2, x19
xor x6, x6, x19
xor x6, x6, x17
xor x21, x4, x6
dom_and x3, x4, x11, x12, x22, x9, x10
and x0, x0, x0
xor x13, x1, x9
xor x15, x11, x1
and x22, x0, x0
and x0, x0, x0
xor x22, x5, x9
and x0, x0, x0
xor x24, x20, x9
and x0, x0, x0
xor x14, x2, x10
xor x16, x12, x2
xor x23, x6, x10
xor x25, x21, x10
#//Part4
lw x29, 8(x30)
lw x28, 12(x30)
lw x27, 16(x30)
lw x26, 20(x30)
lw x18, 24(x30)
lw x19, 28(x30)
dom_and x22, x23, x15, x16, x29, x30, x17
dom_and x13, x14, x20, x21, x28, x7, x8
and x0, x0, x0
xor x7, x5, x7
and x0, x0, x0
xor x8, x6, x8
dom_and x11, x12, x5, x6, x27, x29, x28
dom_and x15, x16, x29, x28, x26, x5, x6
and x0, x0, x0
xor x15, x15, x9
xor x15, x5, x15
and x0, x0, x0
xor x16, x16, x10
xor x16, x6, x16
dom_and x1, x2, x3, x4, x18, x27, x26
and x0, x0, x0
xor x3, x1, x30
and x0, x0, x0
and x4, x0, x0
xor x4, x2, x17
and x0, x0, x0
and x17, x0, x0
xor x17, x3, x7
and x0, x0, x0
xor x18, x4, x8
dom_and x20, x21, x27, x26, x19, x29, x22
and x0, x0, x0
xor x20, x29, x24
xor x27, x15, x20
xor x19, x3, x15
xor x5, x7, x20
xor x9, x17, x27
and x0, x0, x0
xor x21, x22, x25
xor x28, x16, x21
xor x26, x4, x16
xor x6, x8, x21
xor x10, x18, x28
#//Part5
li x30, 0x68
lw x1, (x30)
lw x2, 4(x30)
lw x12, 56(x30)
lw x13, 60(x30)
li x30, 0x1a0
lw x11, (x30)
lw x14, 36(x30)
dom_and x5, x6, x1, x2, x11, x22, x23
dom_and x5, x6, x12, x13, x14, x24, x25
sw x0, (x30)
sw x0, 36(x30)
and x0, x0, x0
li x30, 0x100
and x0, x0, x0
sw x22, (x30)
and x0, x0, x0
sw x24, 72(x30)
sw x23, 4(x30)
sw x25, 76(x30)
li x30, 0x78
lw x11, (x30)
lw x14, 4(x30)
lw x24, 120(x30)
lw x22, 124(x30)
li x30, 0x1a4
lw x25, (x30)
lw x23, 36(x30)
dom_and x20, x21, x11, x14, x25, x12, x13
dom_and x20, x21, x24, x22, x23, x25, x14
and x0, x0, x0
li x30, 0x108
sw x12, (x30)
sw x25, 72(x30)
sw x13, 4(x30)
sw x14, 76(x30)
li x30, 0x38
lw x1, (x30)
lw x2, 4(x30)
lw x22, 24(x30)
lw x20, 28(x30)
li x30, 0xb8
lw x13, (x30)
lw x14, 4(x30)
lw x6, 24(x30)
lw x11, 28(x30)
lw x25, 88(x30)
lw x23, 92(x30)
li x30, 0x1a8
lw x12, (x30)
dom_and x7, x8, x1, x2, x12, x24, x5
and x12, x0, x0
lw x12, 36(x30)
dom_and x7, x8, x6, x11, x12, x1, x2
lw x29, 4(x30)
dom_and x19, x26, x13, x14, x29, x7, x8
lw x30, 40(x30)
dom_and x19, x26, x22, x20, x30, x6, x21
and x0, x0, x0
li x30, 0x110
sw x24, (x30)
and x0, x0, x0
sw x5, 4(x30)
and x0, x0, x0
sw x7, 8(x30)
and x0, x0, x0
sw x8, 12(x30)
and x0, x0, x0
sw x1, 72(x30)
and x0, x0, x0
sw x2, 76(x30)
and x0, x0, x0
sw x6, 80(x30)
and x0, x0, x0
sw x21, 84(x30)
li x30, 0x80
lw x1, (x30)
lw x2, 4(x30)
lw x11, 64(x30)
lw x12, 68(x30)
lw x14, 88(x30)
lw x29, 92(x30)
lw x6, 104(x30)
lw x7, 108(x30)
li x30, 0x1b0
lw x5, (x30)
lw x13, 4(x30)
lw x8, 36(x30)
lw x26, 40(x30)
dom_and x15, x16, x1, x2, x5, x22, x21
dom_and x15, x16, x6, x7, x8, x1, x2
dom_and x3, x4, x11, x12, x13, x15, x16
dom_and x3, x4, x14, x29, x26, x5, x8
and x0, x0, x0
li x30, 0x120
sw x22, (x30)
sw x1, 72(x30)
sw x15, 8(x30)
sw x5, 80(x30)
and x0, x0, x0
sw x21, 4(x30)
sw x2, 76(x30)
sw x16, 12(x30)
sw x8, 84(x30)
li x30, 0x40
lw x22, (x30)
lw x21, 4(x30)
lw x19, 112(x30)
lw x20, 116(x30)
li x30, 0x1b8
lw x1, (x30)
lw x30, 36(x30)
dom_and x17, x18, x19, x20, x1, x11, x12
dom_and x17, x18, x22, x21, x30, x1, x24
and x0, x0, x0
li x30, 0x1b8
sw x0, 36(x30)
li x30, 0x130
sw x11, (x30)
and x0, x0, x0
sw x1, 72(x30)
sw x12, 4(x30)
sw x24, 76(x30)
li x30, 0x48
lw x11, (x30)
lw x12, 4(x30)
lw x4, 16(x30)
lw x5, 20(x30)
lw x7, 64(x30)
lw x8, 68(x30)
li x30, 0x1bc
lw x3, (x30)
sw x0, (x30)
lw x14, 4(x30)
sw x0, 4(x30)
lw x6, 36(x30)
sw x0, 36(x30)
lw x13, 40(x30)
sw x0, 40(x30)
dom_and x9, x10, x25, x23, x3, x15, x16
dom_and x9, x10, x4, x5, x6, x17, x18
dom_and x27, x28, x7, x8, x14, x19, x20
dom_and x27, x28, x11, x12, x13, x21, x22
and x0, x0, x0
li x30, 0x138
sw x15, (x30)
sw x17, 72(x30)
sw x19, 8(x30)
sw x21, 80(x30)
and x0, x0, x0
sw x16, 4(x30)
sw x18, 76(x30)
sw x20, 12(x30)
sw x22, 84(x30)
###############
jal x30, clear_all
li x22, 0x100
li x31, 0x0
li x24, 1
jal x30, bottom_linear
jal x30, clear_all
li x22, 0x104
li x31, 0x4
jal x30, bottom_linear
jal x30, clear_all

#######################
trap:
nop
j trap
nop
top_linear:
lw x10, 0(x31)
lw x11, 8(x31)
lw x12, 16(x31)
lw x13, 24(x31)
lw x14, 32(x31)
lw x15, 40(x31)
lw x16, 48(x31)
lw x17, 56(x31)

xor x1, x10, x13
xor x2, x10, x15
xor x10, x10, x16
xor x3, x13, x15
xor x29, x14, x16
xor x4, x1, x29
xor x5, x11, x12
xor x7, x17, x4
xor x8, x17, x5
xor x9, x4, x5
xor x11, x11, x15
xor x15, x12, x15
xor x14, x10, x3
xor x19, x4, x11
xor x20, x29, x11
xor x21, x29, x15
xor x22, x8, x21
xor x23, x13, x17
xor x24, x5, x23
xor x25, x1, x24
xor x26, x16, x17
xor x27, x5, x26
xor x28, x2, x27
xor x17, x2, x9
xor x16, x25, x22
xor x13, x10, x21
xor x12, x1, x15


sw x1, 0(x18)
sw x2, 8(x18)
sw x10, 16(x18)
sw x3, 24(x18)
sw x29, 32(x18)
sw x4, 40(x18)
sw x5, 48(x18)
sw x7, 56(x18)
sw x8, 64(x18)
sw x9, 72(x18)
sw x11, 80(x18)
sw x15, 88(x18)
sw x14, 96(x18)
sw x19, 104(x18)
sw x20, 112(x18)
sw x21, 120(x18)
sw x22, 128(x18)
sw x23, 136(x18)
sw x24, 144(x18)
sw x25, 152(x18)
sw x26, 160(x18)
sw x27, 168(x18)
sw x28, 176(x18)
sw x17, 184(x18)
sw x16, 192(x18)
sw x13, 200(x18)
sw x12, 208(x18)

jalr x0, x30


bottom_linear: 

lw x1, 0(x22)
lw x3, 8(x22)
lw x2, 16(x22)
lw x10, 24(x22)
lw x13, 32(x22)
lw x5, 40(x22)
lw x15, 48(x22)
lw x23, 56(x22)
lw x8, 64(x22)
lw x4, 72(x22)
lw x14, 80(x22)
lw x16, 88(x22)
lw x9, 96(x22)
lw x6, 104(x22)
lw x7, 112(x22)
lw x11, 120(x22)
lw x12, 128(x22)
lw x21, 136(x22)
xor x3, x3, x4
xor x8, x8, x9
xor x17, x1, x2
xor x6, x5, x6
xor x1, x1, x3
xor x7, x7, x17
xor x18, x23, x8
xor x10, x10, x11
xor x19, x10, x12
xor x20, x13, x14
xor x10, x15, x23
xor x4, x19, x4
xor x4, x4, x20
xor x4, x4, x10
xor x29, x19, x20
xor x29, x29, x1
xor x28, x19, x18
xor x28, x28, x16
xor x28, x28, x20
xor x28, x28, x7
xor x9, x19, x9
xor x9, x9, x6
xor x9, x9, x17
beqz x24, skip1
not x9, x9
skip1:
sw x4, 0(x31)
sw x29, 24(x31)
sw x28, 40(x31)
sw x9, 56(x31)
xor x12, x11, x12
xor x14, x14, x12
xor x14, x14, x1
xor x14, x14, x10
beqz x24, skip2
not x14, x14
skip2:
xor x21, x21, x8
xor x21, x21, x7
xor x21, x21, x15
xor x21, x21, x11
beqz x24, skip3
not x21, x21
skip3:
xor x20, x12, x20
xor x20, x20, x2
xor x20, x20, x5
xor x20, x20, x3
xor x13, x13, x12
xor x13, x13, x6
xor x13, x13, x18
beqz x24, skip4
not x13, x13
skip4:
sw x14, 8(x31)
sw x21, 16(x31)
sw x20, 32(x31)
sw x13, 48(x31)

jalr x0, x30

clear_all:
  andi x1, x1, 0
  and x0, x0, x0
  andi x2, x2, 0
  and x0, x0, x0
  andi x3, x3, 0
  and x0, x0, x0
  andi x4, x4, 0
  and x0, x0, x0
  andi x5, x5, 0
  and x0, x0, x0
  andi x6, x6, 0
  andi x7, x7, 0
  and x0, x0, x0
  andi x8, x8, 0
  and x0, x0, x0
  andi x9, x9, 0
  and x0, x0, x0
  andi x10, x10, 0
  and x0, x0, x0
  andi x11, x11, 0
  and x0, x0, x0
  andi x12, x12, 0
  and x0, x0, x0
  andi x13, x13, 0
  and x0, x0, x0
  andi x14, x14, 0
  and x0, x0, x0
  andi x15, x15, 0
  and x0, x0, x0
  andi x16, x16, 0
  and x0, x0, x0
  andi x17, x17, 0
  and x0, x0, x0
  andi x18, x18, 0
  and x0, x0, x0
  andi x19, x19, 0
  and x0, x0, x0
  andi x20, x20, 0
  and x0, x0, x0
  andi x21, x21, 0
  and x0, x0, x0
  andi x22, x22, 0
  and x0, x0, x0
  andi x23, x23, 0
  and x0, x0, x0
  andi x24, x24, 0
and x0, x0, x0  
  andi x25, x25, 0
  and x0, x0, x0
  andi x26, x26, 0
  and x0, x0, x0
  andi x27, x27, 0
  and x0, x0, x0
  andi x28, x28, 0
  and x0, x0, x0
  andi x29, x29, 0
  and x0, x0, x0
  andi x31, x31, 0
  jalr x0, x30

