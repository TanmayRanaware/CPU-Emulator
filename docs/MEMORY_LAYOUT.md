# Memory Layout & Function Call Documentation

## Overview

This document shows how `factorial(5)` is laid out in memory on the Software CPU,
how function calls work using CALL/RET, and how recursion is carried out on the stack.

---

## 1. Executable Memory Layout

```
Address Range     Region          Contents
─────────────────────────────────────────────────────────────────
0x0000 - 0x00FF   Code Segment    Assembled instructions (main + factorial)
0x0100 - 0x01FF   Data Segment    Result stored here (MEM[0x0100] = 120)
0x0200 - 0xFCFF   Heap / Free     Unused RAM
0xFD00 - 0xFEFF   Save Area       Caller-saved 'n' values during recursion
0xFF00 - 0xFFFF   I/O + Stack     MMIO at 0xFF00; stack grows DOWN from 0xFFFF
─────────────────────────────────────────────────────────────────
```

### Code Segment Detail (factorial.asm)
```
0x0000  main:       LDI R0, #5          ; load argument n=5
0x0002              LDI R2, #1
0x0004              SHL R2, R2, #4      ; R2 = 0x0010 = address of factorial
0x0006              CALL R2             ; call factorial(5)
0x0008              LDI R1, #1
0x000A              SHL R1, R1, #8      ; R1 = 0x0100
0x000C              ST  R0, R1, #0      ; MEM[0x0100] = result
0x000E              HLT

0x0010  factorial:  LDI R1, #1          ; base-case check
        ...         (recursive body)
        ...         CALL R2             ; recursive call
        ...         RET                 ; return to caller

        base_case:  LDI R0, #1
                    RET
```

---

## 2. Function Call Mechanics (CALL / RET)

### CALL instruction
```
CALL RS1

Pseudocode:
    SP = SP - 2               ; grow stack downward
    MEM[SP] = PC + 2          ; push return address (next instruction)
    PC = RS1                  ; jump to function
```

### RET instruction
```
RET

Pseudocode:
    PC = MEM[SP]              ; pop return address
    SP = SP + 2               ; shrink stack
```

### Stack frame for one call
```
Before CALL:          After CALL:           After RET:
                      ┌─────────────┐
SP → 0xFFFF           │ return addr │ ← SP   SP → 0xFFFF (restored)
                      └─────────────┘
                        0xFFFD
```

---

## 3. Recursion: factorial(5) Call Stack

Each call to `factorial(n)` pushes a return address via CALL.
`n` is saved in a dedicated save area (0xFD00 downward).

```
Call Depth    n     Stack (SP →)          Save Area
──────────────────────────────────────────────────────
main calls    5     SP=0xFFFF             —
  factorial(5)      SP=0xFFFD  [0x0008]  [0xFDFF]=5
    factorial(4)    SP=0xFFFB  [ret]     [0xFDFD]=4
      factorial(3)  SP=0xFFF9  [ret]     [0xFDFB]=3
        factorial(2)SP=0xFFF7  [ret]     [0xFDF9]=2
          factorial(1) → BASE CASE: returns R0=1
        unwind: R0 = 2*1 = 2,  RET → SP=0xFFF9
      unwind:   R0 = 3*2 = 6,  RET → SP=0xFFFB
    unwind:     R0 = 4*6 = 24, RET → SP=0xFFFD
  unwind:       R0 = 5*24=120, RET → SP=0xFFFF
main: MEM[0x0100] = 120, HLT
```

### Stack contents at maximum depth (5 frames deep)
```
0xFFFF  ← initial SP (empty)
0xFFFE  ]
0xFFFD  ] return address from main → factorial(5) call      = 0x0008
0xFFFC  ]
0xFFFB  ] return address from factorial(5) → factorial(4)
0xFFF9  ] return address from factorial(4) → factorial(3)
0xFFF7  ] return address from factorial(3) → factorial(2)
0xFFF5  ] return address from factorial(2) → factorial(1)
         ← SP at deepest point
```

---

## 4. Fetch / Compute / Store for CALL and RET

### CALL R2 (calling factorial)
```
FETCH:   PC=0x0006 → load instruction word from MEM[0x0006]
COMPUTE: Decode CALL R2 → target = R2 = 0x0010
         return_addr = PC + 2 = 0x0008
STORE:   SP -= 2 → SP = 0xFFFD
         MEM[0xFFFD] = 0x0008  (return address written to stack)
         PC = 0x0010           (jump to factorial)
```

### RET (returning from factorial)
```
FETCH:   PC=addr of RET → load RET instruction word
COMPUTE: Decode RET → pop MEM[SP] = MEM[0xFFFD] = 0x0008
STORE:   SP += 2 → SP = 0xFFFF
         PC = 0x0008           (jump back to instruction after CALL)
```

---

## 5. Running It

```bash
make
./cpu_emulator programs/factorial.asm run
```

To inspect the result:
```
> load programs/factorial.asm
> trace on
> run
> dec 0x0100 1        ; should show: 120
> spr                 ; SP should be back at 0xFFFF
```

To watch the stack grow and shrink during recursion:
```
> load programs/factorial.asm
> trace on
> step                ; step through each instruction
> spr                 ; watch SP decrease with each CALL
```
