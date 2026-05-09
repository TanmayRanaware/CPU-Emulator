; Timer example program
; Demonstrates Fetch/Compute/Store cycles
; Counts down from 9 to 0, outputting each value

start:
    ; Initialize counter
    LDI R0, #9         ; Counter = 9
    
    ; Build I/O address 0xFF00 in R1: 0xFFFF XOR 0x00FF
    LDI R1, #0
    NOT R1, R1         ; R1 = 0xFFFF
    LDI R2, #1
    SHL R2, R2, #8     ; R2 = 256
    LDI R3, #1
    SUB R2, R2, R3     ; R2 = 255 = 0x00FF
    XOR R1, R1, R2     ; R1 = 0xFF00
    
    ; Output "10" first (before the loop)
    LDI R2, #49        ; ASCII '1'
    ST R2, R1, #0      ; Output '1'
    LDI R2, #48        ; ASCII '0'
    ST R2, R1, #0      ; Output '0'
    LDI R2, #10        ; Newline
    ST R2, R1, #0      ; Output newline
    
loop:
    ; Convert number to ASCII and output
    LDI R2, #48        ; ASCII '0'
    ADD R4, R0, R2     ; R4 = R0 + 48 (ASCII value)
    ST R4, R1, #0      ; Output digit
    
    ; Output newline
    LDI R4, #10        ; Newline
    ST R4, R1, #0      ; Output newline
    
    ; Check if zero BEFORE decrementing (so we output 0, then exit)
    LDI R7, #0         ; Set R7 to 0 for jump
    LDI R2, #0
    SUB R3, R0, R2     ; R3 = R0 - 0, sets Z when R0 == 0
    JZ R7, output_done ; If zero, output "Done"
    
    ; Decrement counter
    LDI R2, #1
    SUB R0, R0, R2     ; R0 = R0 - 1
    
    ; Continue loop
    LDI R7, #0
    JMP R7, loop       ; Jump to loop
    
output_done:
    ; Output "Done"
    ; 'D' = 68
    LDI R0, #31
    LDI R2, #31
    ADD R0, R0, R2     ; R0 = 62
    LDI R2, #6
    ADD R0, R0, R2     ; R0 = 68
    ST R0, R1, #0      ; Output 'D'
    
    ; 'o' = 111
    LDI R0, #31
    LDI R2, #31
    ADD R0, R0, R2     ; R0 = 62
    LDI R2, #31
    ADD R0, R0, R2     ; R0 = 93
    LDI R2, #18
    ADD R0, R0, R2     ; R0 = 111
    ST R0, R1, #0      ; Output 'o'
    
    ; 'n' = 110
    LDI R0, #31
    LDI R2, #31
    ADD R0, R0, R2     ; R0 = 62
    LDI R2, #31
    ADD R0, R0, R2     ; R0 = 93
    LDI R2, #17
    ADD R0, R0, R2     ; R0 = 110
    ST R0, R1, #0      ; Output 'n'
    
    ; 'e' = 101
    LDI R0, #31
    LDI R2, #31
    ADD R0, R0, R2     ; R0 = 62
    LDI R2, #31
    ADD R0, R0, R2     ; R0 = 93
    LDI R2, #8
    ADD R0, R0, R2     ; R0 = 101
    ST R0, R1, #0      ; Output 'e'
    
    ; Output newline
    LDI R0, #10
    ST R0, R1, #0      ; Output newline
    
    HLT