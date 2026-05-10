; factorial.asm
; Recursive Factorial using CALL / RET
;
; Demonstrates:
;   - Function calls (CALL / RET)
;   - Stack-based recursion
;   - Memory layout during execution
;
; C equivalent:
;   int factorial(int n) {
;       if (n <= 1) return 1;
;       return n * factorial(n - 1);
;   }
;   int main() {
;       int result = factorial(5);   // expects 120
;   }
;
; Register Calling Convention:
;   R0 = argument n (input) and return value (output)
;   R1 = scratch / temp
;   R2 = address of factorial function (for CALL)
;   R7 = scratch register (used for jumps)
;   SP = stack pointer (auto-managed by CALL/RET)
;
; Memory Layout:
;   0x0000 - 0x00xx : Code segment (program instructions)
;   0xFFFE - downward: Stack (grows downward, managed by SP)
;   0x0100          : Result stored here after execution
;
; ── main ──────────────────────────────────────────────────────────────────────
main:
    ; Load argument: n = 5
    ; FETCH: load LDI instruction
    ; COMPUTE: sign-extend immediate #5
    ; STORE: R0 = 5
    LDI R0, #5              ; argument n = 5

    ; Load address of factorial function into R2
    ; We use LDI + SHL to build the address since it may exceed 6-bit immediate
    ; factorial label resolves to its byte address in the assembled program
    LDI R2, #1
    SHL R2, R2, #4          ; R2 = 16 (address of factorial, adjust if needed)

    ; CALL factorial
    ; FETCH: load CALL instruction
    ; COMPUTE: push return address onto stack, set PC = R2
    ; STORE: SP decremented, MEM[SP] = return address, PC = factorial
    CALL R2                 ; call factorial(5)

    ; After CALL returns, R0 holds the result (120)
    ; Store result at memory address 0x0100 for inspection
    LDI R1, #1
    SHL R1, R1, #8          ; R1 = 256 = 0x0100
    ST  R0, R1, #0          ; MEM[0x0100] = result

    HLT                     ; Done

; ── factorial(n) ──────────────────────────────────────────────────────────────
; Input:  R0 = n
; Output: R0 = n!
; Uses stack to save: return address (via CALL/RET), n, and partial results
factorial:
    ; ── Base case: if n <= 1, return 1 ──────────────────────────────────────
    LDI R1, #1
    SUB R7, R0, R1          ; R7 = n - 1, sets Z flag if n == 1
    JZ  R7, base_case       ; if n == 1, jump to base case

    ; Check n == 0 (also base case)
    LDI R7, #0
    SUB R1, R0, R7          ; R1 = n - 0
    JZ  R1, base_case       ; if n == 0, jump to base case

    ; ── Recursive case: factorial(n-1) ───────────────────────────────────────
    ; Save n onto the stack before recursive call
    ; STORE phase: push R0 (n) to stack
    LDI R1, #2
    SUB R7, R0, R1          ; R7 = n - 2 (check for n==2 shortcut, not used here)

    ; Push current n onto stack manually (caller-save convention)
    ; SP -= 2; MEM[SP] = R0
    LDI R1, #2
    NOT R7, R1              ; prepare to decrement SP
    ; Decrement SP by 2: use ST with pre-decrement trick
    ; We load SP-like address through R6 (dedicated frame pointer / SP mirror)
    ; Since SP is a special register and not a GPR, we track it via R6
    ; R6 = current stack frame top (mirrors SP after CALL adjustments)

    ; Strategy: use a fixed scratch area in high memory for saving n
    ; We use MEM[0xFE00 - depth*2] for saved n values
    ; For simplicity, we use R5 as a "frame depth" counter
    ; and store saved n at 0xFE00 - (R5 * 2)

    ; Save n to scratch stack area
    ; R5 = frame depth (starts at 0 in main, incremented each call)
    LDI R6, #1
    NOT R6, R6              ; R6 = 0xFFFF
    LDI R7, #1
    SHL R7, R7, #9          ; R7 = 512 = 0x0200
    SUB R6, R6, R7          ; R6 = 0xFDFF  (base of our save area)
    ; Offset by R5 (frame depth * 2)
    LDI R1, #1
    SHL R1, R1, #1          ; R1 = 2
    ; Save n (R0) to R6 + R5*2... simplified: ST R0, R6, #0
    ; For a clean demo we just save to a fixed slot using R5 as index
    ; R5 starts at 0 (initialized in main, not shown — see note below)
    ST  R0, R6, #0          ; MEM[R6] = n  (save n before recursion)

    ; Decrement n: R0 = n - 1
    LDI R1, #1
    SUB R0, R0, R1          ; R0 = n - 1

    ; Adjust save-area pointer for next frame
    LDI R1, #2
    SUB R6, R6, R1          ; R6 -= 2 (next frame saves at lower address)

    ; Load factorial address and recurse
    ; Reload factorial address into R2
    ; (same address as before — recalculate since R2 may be clobbered)
    LDI R2, #1
    SHL R2, R2, #4          ; R2 = 16 = address of factorial

    ; CALL factorial(n-1)
    ; FETCH: load CALL instruction
    ; COMPUTE: push PC+2 to stack, jump to factorial
    ; STORE: SP-=2, MEM[SP] = return_addr, PC = factorial
    CALL R2                 ; recursive call: factorial(n-1)

    ; ── Back from recursion: R0 = factorial(n-1) ────────────────────────────
    ; Restore saved n from memory
    ; Undo the R6 adjustment
    LDI R1, #2
    ADD R6, R6, R1          ; R6 += 2 (restore to this frame's slot)
    LD  R1, R6, #0          ; R1 = saved n

    ; result = n * factorial(n-1)
    ; Multiply R1 * R0 using repeated addition
    ; R3 = accumulator, R4 = loop counter
    LDI R3, #0              ; R3 = 0 (accumulator)
    LDI R4, #0              ; R4 = loop index

multiply_loop:
    ADD R3, R3, R0          ; R3 += factorial(n-1)
    LDI R7, #1
    ADD R4, R4, R7          ; R4++
    SUB R7, R4, R1          ; R7 = R4 - n
    JNZ R7, multiply_loop   ; loop until R4 == n

    ; R3 = n * factorial(n-1) = n!
    ; Move result to R0 (return value register)
    LDI R0, #0
    ADD R0, R0, R3          ; R0 = n!

    ; RETURN to caller
    ; FETCH: load RET instruction
    ; COMPUTE: pop return address from stack
    ; STORE: SP += 2, PC = popped return address
    RET

base_case:
    ; Base case: return 1
    LDI R0, #1              ; R0 = 1
    RET                     ; return to caller
