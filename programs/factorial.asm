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
;   R2 = factorial address for CALL (0x0014 = 20)
;   R3 = multiply accumulator
;   R4 = multiply counter
;   R5 = SOFTWARE stack pointer (init once in main at 0xFDFF, NEVER rebuilt)
;   R7 = scratch for comparisons
;   SP = hardware stack (CALL/RET manage return addresses automatically)
;
; Memory layout:
;   0x0000-0x004A  Code segment
;   0x0100         Result stored here (MEM[0x0100] = 120)
;   0xFDFF down    Software stack: saved n per frame (grows downward)
;   0xFFFF down    Hardware stack: return addresses (grows downward)
;
; Verified jump offsets (6-bit signed, must be -32 to +31):
;   JNZ 0x0018 -> check_zero  0x001E : offset = +4   OK
;   JNZ 0x001E -> recurse     0x0024 : offset = +4   OK
;   JNZ 0x0044 -> multiply_loop      : offset = -10  OK

; ── main (0x0000) ─────────────────────────────────────────────────────────────
main:
    LDI R5, #1
    SHL R5, R5, #9       ; R5 = 0x0200
    NOT R5, R5           ; R5 = 0xFDFF  software stack base, init ONCE

    LDI R0, #5           ; n = 5
    LDI R2, #20          ; R2 = 0x0014 = address of factorial
    CALL R2              ; call factorial(5)

    LDI R1, #1
    SHL R1, R1, #8       ; R1 = 0x0100
    ST  R0, R1, #0       ; MEM[0x0100] = result (120)
    HLT

; ── factorial (0x0014) ────────────────────────────────────────────────────────
; Logic: if n==1, fall through to base_case.
;        if n!=1, jump to check_zero.
;        if n==0, fall through to base_case.
;        if n!=0, jump to recurse.
; This avoids any forward jump longer than +4 bytes.
factorial:
    LDI R1, #1
    SUB R7, R0, R1       ; R7 = n - 1
    JNZ R7, check_zero   ; if n != 1, skip to check_zero  (+4 bytes)
    ; fall through: n == 1

; ── base_case (0x001A) ────────────────────────────────────────────────────────
base_case:
    LDI R0, #1           ; return 1
    RET

; ── check_zero (0x001E) ───────────────────────────────────────────────────────
check_zero:
    JNZ R0, recurse      ; if n != 0, go to recurse        (+4 bytes)
    ; fall through: n == 0
    LDI R0, #1           ; return 1
    RET

; ── recurse (0x0024) ─────────────────────────────────────────────────────────
recurse:
    ; PUSH n: MEM[R5] = n;  R5 -= 2
    ST  R0, R5, #0       ; save n for this frame
    LDI R1, #2
    SUB R5, R5, R1       ; R5 -= 2  (software stack grows down)

    ; R0 = n - 1
    LDI R1, #1
    SUB R0, R0, R1

    ; Reload factorial address and recurse
    LDI R2, #20          ; R2 = 0x0014
    CALL R2              ; factorial(n-1), result in R0

    ; POP n: R5 += 2;  R1 = MEM[R5]
    LDI R1, #2
    ADD R5, R5, R1       ; R5 += 2
    LD  R1, R5, #0       ; R1 = saved n

    ; Multiply: R3 = n * factorial(n-1)
    LDI R3, #0           ; accumulator = 0
    LDI R4, #0           ; counter = 0

multiply_loop:
    ADD R3, R3, R0       ; R3 += factorial(n-1)
    LDI R7, #1
    ADD R4, R4, R7       ; R4++
    SUB R7, R4, R1       ; R7 = counter - n
    JNZ R7, multiply_loop ; loop until counter == n  (-10 bytes)

    LDI R0, #0
    ADD R0, R0, R3       ; R0 = n!
    RET
