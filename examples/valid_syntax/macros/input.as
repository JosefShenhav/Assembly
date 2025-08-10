mcro a
prn r1
mov #1, r2
mcroend
mcro b
jmp W
clr r5
mcroend
a ; Call Macro
W: .data 1, 2,3
inc r5
b
b
stop

