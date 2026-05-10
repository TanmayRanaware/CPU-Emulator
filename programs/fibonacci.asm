; Computes first 10 Fibonacci numbers and stores them in memory

start:
    LDI R0, #0         ; F(0) = 0
    LDI R1, #1         ; F(1) = 1
    ; Build address 0x0040 (64 decimal) since LDI can only load -32 to 31
    ; 64 = 31 + 1 + 32, but we can't load 32 either
    ; Alternative: 64 = 31 + 31 + 2
    LDI R2, #31        ; Start with 31 (max positive immediate)
    LDI R4, #31        ; Load 31 again
    ADD R2, R2, R4     ; R2 = 31 + 31 = 62
    LDI R4, #2         ; Load 2
    ADD R2, R2, R4     ; R2 = 62 + 2 = 64 = 0x0040
    LDI R3, #8         ; Counter: 8 more numbers
    LDI R4, #2         ; Address increment (reuse R4, now R4 = 2)
    LDI R5, #1         ; Counter decrement
    LDI R7, #0         ; Zero register
    
    ; Verify R2 is correct before storing
    ST R0, R2, #0      ; Store F(0) at address in R2 (should be 0x0040)
    ADD R2, R2, R4     ; R2 = R2 + 2 (should be 0x0042)
    ST R1, R2, #0      ; Store F(1) at address in R2 (should be 0x0042)
    ADD R2, R2, R4     ; R2 = R2 + 2 (should be 0x0044)
    
loop:
    ADD R6, R0, R1     ; Compute F(n) = F(n-1) + F(n-2)
    ST R6, R2, #0      ; Store F(n)
    LDI R0, #0         ; Update F(n-2) = F(n-1)
    ADD R0, R1, R0
    LDI R1, #0         ; Update F(n-1) = F(n)
    ADD R1, R6, R1
    ADD R2, R2, R4     ; Increment address
    SUB R3, R3, R5     ; Decrement counter, sets Z when R3 == 0
    JNZ R7, loop       ; If NOT zero (R3 != 0), loop back
    HLT                ; If zero (R3 == 0), halt
