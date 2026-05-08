# CPU Architecture Schematic

## Overview

This document describes the architecture and organization of the software CPU emulator.

## Component Details

### Control Unit
- **Function**: Orchestrates CPU operations
- **Responsibilities**:
  - Instruction fetch
  - Instruction decode
  - Execution control
  - Program counter management
  - Cycle counting

### ALU (Arithmetic Logic Unit)
- **Function**: Performs arithmetic and logical operations
- **Operations**:
  - Arithmetic: ADD, SUB
  - Logic: AND, OR, XOR, NOT
  - Shifts: SHL, SHR
- **Output**: Result + flags (Z, N, C, V)

### Registers

#### General Purpose Registers (GPRs)
- **R0 - R7**: 8 registers, 16-bit each
- Used for data storage and computation

#### Special Purpose Registers (SPRs)
- **PC (Program Counter)**: Points to current instruction
- **SP (Stack Pointer)**: Points to stack top (initialized to 0xFFFF)
- **FLAGS**: Status flags (Z, N, C, V)

### Bus System

#### Instruction Bus (Two-way)
- **Purpose**: Fetch instructions from memory
- **Signals**:
  - `address`: Memory address to read/write
  - `data`: Instruction data
  - `read_enable`: Enable instruction fetch
  - `write_enable`: Enable instruction write (for self-modifying code)

#### Info Bus (Data Bus)
- **Purpose**: Transfer data between components
- **Signals**:
  - `data`: 16-bit data value
  - `valid`: Data valid signal
  - `source`: Source component ID
  - `dest`: Destination component ID

#### Control Bus (One-way)
- **Purpose**: Control signals from Control Unit
- **Signals**:
  - `mem_read`: Enable memory read
  - `mem_write`: Enable memory write
  - `reg_read`: Enable register read
  - `reg_write`: Enable register write
  - `alu_enable`: Enable ALU operation
  - `pc_increment`: Increment program counter
  - `pc_load`: Load program counter
  - `sp_increment`: Increment stack pointer
  - `sp_decrement`: Decrement stack pointer
  - `alu_op`: ALU operation code
  - `reg_select`: Register selection

### Memory
- **Size**: 64KB (65,536 bytes)
- **Organization**: Byte-addressable, word-aligned
- **Endianness**: Little-endian (LSB first)
- **Memory-Mapped I/O**: I/O devices mapped to high memory addresses

## Execution Cycle

### Fetch Phase
1. Control Unit places PC value on Instruction Bus address
2. Instruction Bus read_enable asserted
3. Memory returns instruction word
4. Instruction stored in Control Unit

### Decode Phase
1. Control Unit parses instruction opcode
2. Extracts register indices and immediate values
3. Determines addressing mode
4. Sets up control signals

### Execute Phase
1. Control Unit asserts appropriate control signals
2. Operands fetched from registers or memory
3. ALU performs operation (if needed)
4. Result written to destination register or memory
5. Flags updated based on result

### Store Phase
1. Program counter updated (incremented or loaded)
2. Control signals deasserted
3. Cycle complete

## Data Flow Example: ADD R1, R2, R3

```
1. FETCH:
   PC → Instruction Bus → Memory → Instruction Word

2. DECODE:
   Instruction Word → Control Unit
   - Opcode: ADD
   - RD: R1
   - RS1: R2
   - RS2: R3

3. EXECUTE:
   - Control Unit: Assert reg_read for R2 and R3
   - Registers: Place R2 and R3 values on Info Bus
   - ALU: Perform ADD operation
   - ALU: Place result and flags on Info Bus

4. STORE:
   - Control Unit: Assert reg_write for R1
   - Registers: Store result in R1
   - Registers: Update FLAGS register
   - Control Unit: Increment PC
```

## Component Interaction

```
Control Unit
    │
    ├──→ Control Bus ──→ All Components (control signals)
    │
    ├──→ Instruction Bus ──→ Memory (instruction fetch)
    │
    └──→ Info Bus ──→ Registers, ALU, Memory (data transfer)

ALU
    │
    └──→ Info Bus ──→ Registers (results and flags)

Registers
    │
    ├──→ Info Bus ──→ ALU, Memory (operands)
    └──→ Info Bus ←── ALU, Memory (results)

Memory
    │
    ├──→ Instruction Bus ←── Control Unit (instructions)
    └──→ Info Bus ←──→ Registers, ALU (data)
```
