.extern fn1
;---------------------- empty line number one
lolipop:  add r3, LIST
jsr fn1
LOOP: prn #48
 lea STR, r6
 inc r6
 mov *r6, r6, L3
;-------------------too much operands 
 sub r1, r4
 cmp r3, #-6
 bne END
 add r7, *r6
 clr K
;-------------- empty number two 
sub L3, L3
.entry MAIN
jmp #12
; ------------ opcode jmp cant get an IMMIDIATE dest methud 
END: stop
STR: .string "abcd"
LIST: .data 6, -9
.data -100
K: .data 3$1
; -------- .data cant get nothing but numbers or letters
.extern L3 
.entry END
