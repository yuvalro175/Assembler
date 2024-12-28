.extern fn1
lolipop	: add r3, LIST
;------------------------------to much space after label 
jsr fn1
.entry K 
LOOP: prn #r8
;---------------------- ERR: cant be an #r8
 lea STR, r16
 inc r6
 r1: mov *r6, L3
;------------------- r1 cant be a label 
 sub r9, r4
;------------------- r9 doesnt exist
 cmp r3, #-6
 bne END
 *r7: add r7, *r6
;------------------- *r7 cant be a label
 clr K 
sub L3, L3
.entry MAIN
jmp LOOP
END: stop
STR: .string "abcd"
LIST: .data rtt,3,2,rr
;------------------- illigle data input
.data -88
K: .data 31
.extern L3 
.extern L3
