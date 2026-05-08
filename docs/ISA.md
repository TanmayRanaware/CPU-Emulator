# Instruction Set Architecture (ISA)

## Overview

This CPU implements a 16-bit instruction set architecture with 8 general-purpose registers and a simple, RISC-like instruction format.

## Instruction Format

All instructions are 16 bits wide, encoded as follows:

### Register Format
```
[15:12] - Opcode (4 bits)
[11:9]  - Destination Register (RD) (3 bits, 0-7)
[8:6]   - Source Register 1 (RS1) (3 bits, 0-7)
[5:3]   - Source Register 2 (RS2) (3 bits, 0-7)
[2:0]   - Unused
```

### Immediate Format
```
[15:12] - Opcode (4 bits)
[11:9]  - Destination Register (RD) (3 bits, 0-7)
[8:6]   - Source Register 1 (RS1) (3 bits, 0-7)
[5:0]   - Immediate Value (6 bits, signed -32 to 31)
```

## Registers

### General Purpose Registers (GPRs)
- **R0 - R7**: 8 general-purpose 16-bit registers
- All registers are 16-bit signed integers

### Special Purpose Registers (SPRs)
- **PC (Program Counter)**: 16-bit, points to current instruction
- **SP (Stack Pointer)**: 16-bit, points to top of stack (initialized to 0xFFFF)
- **FLAGS**: 8-bit status register
  - **Z (Zero)**: Set when result is zero
  - **N (Negative)**: Set when result is negative
  - **C (Carry)**: Set on arithmetic carry/borrow
  - **V (Overflow)**: Set on arithmetic overflow

## Instructions

### Arithmetic and Logic Operations

| Opcode | Mnemonic | Format | Description |
|--------|----------|--------|-------------|
| 0x1 | ADD | ADD RD, RS1, RS2/IMM | RD = RS1 + RS2/IMM |
| 0x2 | SUB | SUB RD, RS1, RS2/IMM | RD = RS1 - RS2/IMM |
| 0x3 | AND | AND RD, RS1, RS2/IMM | RD = RS1 & RS2/IMM |
| 0x4 | OR  | OR RD, RS1, RS2/IMM  | RD = RS1 \| RS2/IMM |
| 0x5 | XOR | XOR RD, RS1, RS2/IMM | RD = RS1 ^ RS2/IMM |
| 0x6 | NOT | NOT RD, RS1 | RD = ~RS1 |
| 0x7 | SHL | SHL RD, RS1, IMM | RD = RS1 << IMM |
| 0x8 | SHR | SHR RD, RS1, IMM | RD = RS1 >> IMM |

### Memory Operations

| Opcode | Mnemonic | Format | Description |
|--------|----------|--------|-------------|
| 0x9 | LD | LD RD, RS1, IMM | RD = MEM[RS1 + IMM] |
| 0xA | ST | ST RD, RS1, IMM | MEM[RS1 + IMM] = RD |

### Immediate Load

| Opcode | Mnemonic | Format | Description |
|--------|----------|--------|-------------|
| 0xB | LDI | LDI RD, IMM | RD = IMM (sign-extended) |

### Control Flow

| Opcode | Mnemonic | Format | Description |
|--------|----------|--------|-------------|
| 0xC | JMP | JMP RS1, IMM | PC = RS1 + IMM |
| 0xD | JZ  | JZ RS1, IMM  | If Z flag: PC = RS1 + IMM |
| 0xE | JNZ | JNZ RS1, IMM | If !Z flag: PC = RS1 + IMM |
| 0xF | HLT | HLT | Halt execution |

### Special

| Opcode | Mnemonic | Format | Description |
|--------|----------|--------|-------------|
| 0x0 | NOP | NOP | No operation |

## Addressing Modes

1. **Register Direct**: Operand is in a register (R0-R7)
2. **Immediate**: Operand is a constant value (-32 to 31)
3. **Register Indirect with Offset**: Memory address = Register + Immediate offset
4. **Direct**: Absolute memory address (for jumps, computed as RS1 + IMM)

## Flag Semantics

Flags are updated by arithmetic and logic operations:

- **Z (Zero)**: Set to 1 if result equals zero, else 0
- **N (Negative)**: Set to 1 if result is negative (MSB = 1), else 0
- **C (Carry)**: 
  - For ADD: Set if result exceeds 16-bit signed range
  - For SUB: Set if borrow occurs
  - For shifts: Set to the bit shifted out
- **V (Overflow)**: Set to 1 if signed arithmetic overflow occurs

## Memory Map

```
0x0000 - 0xFEFF: RAM (65,280 bytes)
0xFF00:         Memory-mapped I/O - STDOUT (character output)
0xFF01:         Memory-mapped I/O - STDIN (character input)
0xFF02:         Memory-mapped I/O - Status register
0xFF03 - 0xFFFF: Reserved
```

### Memory-Mapped I/O

- **0xFF00 (STDOUT)**: Writing a byte to this address outputs the character
- **0xFF01 (STDIN)**: Reading from this address gets input (currently returns 0)
- **0xFF02 (STATUS)**: Status register (bit 0 = ready)

## Instruction Encoding Examples

### ADD R1, R2, R3
```
Opcode: 0x1 (ADD)
RD: 1
RS1: 2
RS2: 3
Encoding: 0x1243
```

### LDI R0, #10
```
Opcode: 0xB (LDI)
RD: 0
RS1: 0 (unused)
IMM: 10
Encoding: 0xB00A
```

### LD R1, R0, #0
```
Opcode: 0x9 (LD)
RD: 1
RS1: 0
IMM: 0
Encoding: 0x9000
```

### JMP R0, #0
```
Opcode: 0xC (JMP)
RD: 0 (unused)
RS1: 0
IMM: 0
Encoding: 0xC000
```

## Assembly Syntax

### Labels
Labels are defined with a colon:
```
start:
    LDI R0, #10
```

### Immediate Values
Immediate values can be:
- Decimal: `#10`
- Hexadecimal: `#0x0A` or `#0xFF00`
- Labels: `start` (for jumps)

### Register Names
Registers are named R0 through R7 (case-insensitive).
