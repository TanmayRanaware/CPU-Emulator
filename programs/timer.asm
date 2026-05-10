; timer.asm
; Timer Example - Demonstrates Fetch/Compute/Store Cycles
; Counts down from 10 to 0
;
; Each iteration explicitly shows the 3 pipeline stages:
;   FETCH:   PC points to instruction, Control Unit loads it from memory
;   COMPUTE: ALU executes the operation (e.g. SUB, ADD, JNZ)
;   STORE:   Result is written back to register or memory
;
; After running, use: dec 0x0040 11
; to see the stored countdown values: 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0

start:
    ; --- Cycle 1: Initialize counter ---
    ; FETCH:   Load LDI instruction from PC=0x0000
    ; COMPUTE: Sign-extend immediate #10
    ; STORE:   R0 = 10
    LDI R0, #10             ; R0 = 10 (countdown start)

    ; --- Cycle 2: Initialize step ---
    ; FETCH:   Load LDI instruction
    ; COMPUTE: Sign-extend immediate #1
    ; STORE:   R1 = 1
    LDI R1, #1              ; R1 = 1 (decrement amount)

    ; --- Cycle 3: Initialize memory pointer to 0x0040 ---
    ; FETCH:   Load LDI instruction
    ; COMPUTE: Sign-extend immediate #1, then shift left 6
    ; STORE:   R2 = 64 = 0x0040
    LDI R2, #1
    SHL R2, R2, #6          ; R2 = 64 = 0x0040

    ; Store initial value (10) before first decrement
    ST R0, R2, #0           ; MEM[0x0040] = 10
    LDI R3, #2
    ADD R2, R2, R3          ; advance pointer by 2 bytes

loop:
    ; --- FETCH:   Control Unit reads instruction at current PC
    ; --- COMPUTE: ALU subtracts R1 from R0
    ; --- STORE:   Result written back to register R0
    SUB R0, R0, R1          ; R0 = R0 - 1  <-- core Fetch/Compute/Store

    ; Store current counter value to memory (STORE phase)
    ST R0, R2, #0           ; MEM[R2] = R0

    ; Advance memory pointer
    LDI R3, #2
    ADD R2, R2, R3          ; R2 += 2

    ; Branch back if counter not zero
    JNZ R0, loop            ; if R0 != 0, repeat

    HLT                     ; Halt - countdown complete
