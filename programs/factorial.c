/*
 * factorial.c
 * Recursive factorial function — C reference implementation
 *
 * This is the C code that factorial.asm implements on the Software CPU.
 * It shows:
 *   1. A recursive function (factorial)
 *   2. A driver / main program that calls it
 *
 * Memory layout when running on the Software CPU (see docs/MEMORY_LAYOUT.md):
 *   0x0000 - 0x00FF  Code segment  (assembled instructions)
 *   0x0100           Result stored here after execution
 *   0xFFFE downward  Stack segment (grows downward, SP starts at 0xFFFF)
 *
 * Function call mechanics on the Software CPU:
 *   CALL RS1  =>  SP -= 2; MEM[SP] = PC+2; PC = RS1
 *   RET       =>  PC = MEM[SP]; SP += 2
 */

#include <stdio.h>

/* ── Recursive factorial ─────────────────────────────────────────────────── */
int factorial(int n) {
    /* Base case */
    if (n <= 1) return 1;

    /* Recursive case: n * factorial(n-1)
     * On each call the CPU:
     *   1. PUSHes the return address via CALL
     *   2. Saves 'n' on the stack (caller-save convention)
     *   3. Calls factorial(n-1)
     *   4. Multiplies result by saved n
     *   5. RETs to the caller
     */
    return n * factorial(n - 1);
}

/* ── Driver / main ───────────────────────────────────────────────────────── */
int main(void) {
    int n      = 5;
    int result = factorial(n);

    printf("factorial(%d) = %d\n", n, result);   /* expected: 120 */

    return 0;
}
