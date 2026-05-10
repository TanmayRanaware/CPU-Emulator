; factorial.asm
; Recursive Factorial using CALL/RET + software stack
;
; C equivalent:
;   int factorial(int n) {
;       if (n <= 1) return 1;
;       return n * factorial(n - 1);
;   }
;   int main() {
;       int result = factorial(5);   // expected: 120
;   }
;
; Register convention:
;   R0 = n (argument in, result out)
;   R1 = scratch / saved n after pop
;   R2 = factorial address for CALL (= 0x0014 = 20)
;   R3 = multiply accumulator
;   R4 = multiply counter
;   R5 = SOFTWARE stack pointer (init once in main at 0xFDFF, NEVER rebuilt)
;   R7 = scratch for comparisons
;   SP = hardware stack (CALL/RET manage return addresses automatically)
;
; Memory layout:
;   0x0000-0x004E  Code segment
;   0x0100         Result stored here after run (MEM[0x0100] = 120)
;   0xFDFF down    Software stack: saved n per recursive frame
;   0xFFFF down    Hardware stack: return addresses pushed by CALL
;
; Jump range: 6-bit signed = -32 to +31 bytes only.
; Layout carefully places base_case between the JZ checks and recursive body
; so all jumps stay within range. Verified offsets:
;   JZ  0x0018 -> base_case 0x0022 : offset = +8   OK
;   JZ  0x001E -> base_case 0x0022 : offset = +2   OK
;   JMP 0x0020 -> recurse   0x0026 : offset = +4   OK
;   JNZ 0x0046 -> multiply_loop    : offset = -10  OK

; ── main (0x0000) ─────────────────────────────────────────────────────────────
main:
    ; Build software stack base: R5 = NOT(0x0200) = 0xFDFF
    LDI R5, #1
    SHL R5, R5, #9       ; R5 = 0x0200
    NOT R5, R5           ; R5 = 0xFDFF  <-- init ONCE here, never touch in factorial

    LDI R0, #5           ; argument n = 5
    LDI R2, #20          ; R2 = 0x0014 = address of factorial label
    CALL R2              ; call factorial(5); return addr pushed to HW stack

    ; Store result at 0x0100
    LDI R1, #1
    SHL R1, R1, #8       ; R1 = 0x0100
    ST  R0, R1, #0       ; MEM[0x0100] = result (120)
    HLT

; ── factorial (0x0014) ────────────────────────────────────────────────────────
; Input:  R0 = n
; Output: R0 = n!
; Note: base_case is placed immediately after the two JZ checks so that
;       the forward jump offsets are small (+8 and +2 bytes respectively).
factorial:
    ; Check n == 1
    LDI R1, #1
    SUB R7, R0, R1       ; R7 = n - 1  (Z set if n==1)
    JZ  R7, base_case    ; offset = +8, jumps to 0x0022

    ; Check n == 0
    LDI R7, #0
    SUB R7, R0, R7       ; R7 = n      (Z set if n==0)
    JZ  R7, base_case    ; offset = +2, jumps to 0x0022

    ; Jump over base_case to recursive body
    JMP R0, recurse      ; offset = +4, jumps to 0x0026

; ── base_case (0x0022) ────────────────────────────────────────────────────────
base_case:
    LDI R0, #1           ; return value = 1
    RET                  ; pop return addr from HW stack, jump back to caller

; ── recurse (0x0026) ─────────────────────────────────────────────────────────
recurse:
    ; PUSH n onto software stack
    ; MEM[R5] = n;  R5 -= 2
    ST  R0, R5, #0       ; save n at current frame slot
    LDI R1, #2
    SUB R5, R5, R1       ; R5 -= 2  (R5 persists across all recursive calls)

    ; Set R0 = n - 1 for recursive call
    LDI R1, #1
    SUB R0, R0, R1       ; R0 = n - 1

    ; Reload factorial address (R2 may be clobbered)
    LDI R2, #20          ; R2 = 0x0014 = factorial
    CALL R2              ; recursive call: factorial(n-1), result in R0

    ; POP saved n from software stack
    ; R5 += 2;  R1 = MEM[R5]
    LDI R1, #2
    ADD R5, R5, R1       ; R5 += 2  (back to this frame's slot)
    LD  R1, R5, #0       ; R1 = saved n

    ; Multiply: R3 = R1 * R0  (n * factorial(n-1))
    ; Uses repeated addition: add R0 exactly R1 times
    LDI R3, #0           ; accumulator = 0
    LDI R4, #0           ; counter = 0

multiply_loop:
    ADD R3, R3, R0       ; R3 += factorial(n-1)
    LDI R7, #1
    ADD R4, R4, R7       ; R4++
    SUB R7, R4, R1       ; R7 = counter - n  (zero when done)
    JNZ R7, multiply_loop ; offset = -10, jumps back to 0x003E

    ; Move result to return register
    LDI R0, #0
    ADD R0, R0, R3       ; R0 = n!
    RET                  ; return to caller
