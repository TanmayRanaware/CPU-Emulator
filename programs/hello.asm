; Hello, World program
; Outputs "Hello, World!" using memory-mapped I/O

start:
    ; Build I/O address 0xFF00 in R7: 0xFFFF XOR 0x00FF
    LDI R7, #0
    NOT R7, R7          ; R7 = 0xFFFF
    LDI R6, #1
    SHL R6, R6, #8      ; R6 = 256
    LDI R5, #1
    SUB R6, R6, R5      ; R6 = 255 = 0x00FF
    XOR R7, R7, R6      ; R7 = 0xFF00
    
    ; Output 'H' (72)
    LDI R1, #31
    LDI R2, #31
    ADD R1, R1, R2      ; R1 = 62
    LDI R2, #10
    ADD R1, R1, R2      ; R1 = 72
    ST R1, R7, #0
    
    ; Output 'e' (101)
    LDI R1, #31
    LDI R2, #31
    ADD R1, R1, R2      ; R1 = 62
    LDI R2, #31
    ADD R1, R1, R2      ; R1 = 93
    LDI R2, #8
    ADD R1, R1, R2      ; R1 = 101
    ST R1, R7, #0
    
    ; Output 'l' (108)
    LDI R1, #31
    LDI R2, #31
    ADD R1, R1, R2      ; R1 = 62
    LDI R2, #31
    ADD R1, R1, R2      ; R1 = 93
    LDI R2, #15
    ADD R1, R1, R2      ; R1 = 108
    ST R1, R7, #0
    
    ; Output 'l' (108)
    ST R1, R7, #0
    
    ; Output 'o' (111)
    LDI R1, #31
    LDI R2, #31
    ADD R1, R1, R2      ; R1 = 62
    LDI R2, #31
    ADD R1, R1, R2      ; R1 = 93
    LDI R2, #18
    ADD R1, R1, R2      ; R1 = 111
    ST R1, R7, #0
    
    ; Output ',' (44)
    LDI R1, #31
    LDI R2, #13
    ADD R1, R1, R2      ; R1 = 44
    ST R1, R7, #0
    
    ; Output ' ' (32)
    LDI R1, #31
    LDI R2, #1
    ADD R1, R1, R2      ; R1 = 32
    ST R1, R7, #0
    
    ; Output 'W' (87)
    LDI R1, #31
    LDI R2, #31
    ADD R1, R1, R2      ; R1 = 62
    LDI R2, #25
    ADD R1, R1, R2      ; R1 = 87
    ST R1, R7, #0
    
    ; Output 'o' (111)
    LDI R1, #31
    LDI R2, #31
    ADD R1, R1, R2      ; R1 = 62
    LDI R2, #31
    ADD R1, R1, R2      ; R1 = 93
    LDI R2, #18
    ADD R1, R1, R2      ; R1 = 111
    ST R1, R7, #0
    
    ; Output 'r' (114)
    LDI R1, #31
    LDI R2, #31
    ADD R1, R1, R2      ; R1 = 62
    LDI R2, #31
    ADD R1, R1, R2      ; R1 = 93
    LDI R2, #21
    ADD R1, R1, R2      ; R1 = 114
    ST R1, R7, #0
    
    ; Output 'l' (108)
    LDI R1, #31
    LDI R2, #31
    ADD R1, R1, R2      ; R1 = 62
    LDI R2, #31
    ADD R1, R1, R2      ; R1 = 93
    LDI R2, #15
    ADD R1, R1, R2      ; R1 = 108
    ST R1, R7, #0
    
    ; Output 'd' (100)
    LDI R1, #31
    LDI R2, #31
    ADD R1, R1, R2      ; R1 = 62
    LDI R2, #31
    ADD R1, R1, R2      ; R1 = 93
    LDI R2, #7
    ADD R1, R1, R2      ; R1 = 100
    ST R1, R7, #0
    
    ; Output '!' (33)
    LDI R1, #31
    LDI R2, #2
    ADD R1, R1, R2      ; R1 = 33
    ST R1, R7, #0
    
    ; Output newline
    LDI R1, #10
    ST R1, R7, #0
    
    HLT