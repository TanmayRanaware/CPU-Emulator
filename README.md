# Simple CPU Emulator

A complete CPU emulator written in C++ with a modular architecture.  
The project includes a custom instruction set, a full assembler, and sample assembly programs.

## Features

- **Modular architecture**: CPU parts are organized into dedicated classes/structs.
- **Complete ISA**: 16-bit instruction set covering arithmetic, logic, memory, and control flow.
- **Assembler support**: Label handling and literal parsing are built in.
- **Debug tooling**: Instruction tracing and CPU state inspection are available.
- **Memory-mapped I/O**: Character output support via mapped memory range.
- **Example programs**: Includes Timer, Hello World, and Fibonacci demos.

## Build

```bash
make
```

Build output: `cpu_emulator`

## Usage

### Interactive Mode

```bash
./cpu_emulator
```

Available commands:

- `load <file>` - Assemble and load a program file
- `run` - Execute continuously until halt
- `step` - Execute one instruction cycle
- `gpr` - Show General Purpose Registers
- `spr` - Show Special Purpose Registers
- `ram [addr] [len]` - Dump RAM
- `state` - Print full CPU state
- `trace on/off` - Toggle instruction trace output
- `reset` - Reset emulator state
- `help` - Show command list
- `quit` / `exit` - Leave the emulator

### Direct Execution

Run and execute a program in one command:

```bash
./cpu_emulator programs/hello.asm run
./cpu_emulator programs/fibonacci.asm run
./cpu_emulator programs/timer.asm run
```

## Example Session

```text
$ ./cpu_emulator
> load programs/fibonacci.asm
Program loaded: 25 instructions
Labels:
  start: 0x0
  loop: 0x1a
  done: 0x3a

> trace on
Trace enabled

> step
=== Cycle 1 ===
PC: 0x0000
[FETCH] Instruction at PC: 0xb000
[DECODE] LDI R0, #0
[EXECUTE] R0 = 0
[STORE] PC updated to 0x0002

=== CPU State ===
Cycle: 1
=== General Purpose Registers ===
R0: 0x0000 (0)
R1: 0x0000 (0)
...

> run
> state
```

## CPU Components

### Registers

- **GPRs (General Purpose Registers)**: 8 registers (`R0`-`R7`), each 16-bit
- **SPRs (Special Purpose Registers)**: `PC` (Program Counter), `SP` (Stack Pointer), `FLAGS`

### ALU

Handles arithmetic and logical operations, and updates status flags:

- Overflow
- Carry
- Zero
- Negative

### Control Unit

Coordinates the fetch-decode-execute cycle, controls instruction flow, and updates the program counter.

### Bus System

- **Instruction Bus**: Bidirectional instruction fetch communication
- **Info Bus**: Data movement between components
- **Control Bus**: One-way control signaling from the Control Unit

### Memory

- 64KB address space
- Byte-addressable, word-aligned
- Memory-mapped I/O at `0xFF00-0xFFFF`

## Instruction Set

See `docs/ISA.md` for the full instruction set reference.

Core instruction groups:

- Arithmetic: `ADD`, `SUB`
- Logic: `AND`, `OR`, `XOR`, `NOT`
- Shifts: `SHL`, `SHR`
- Memory: `LD`, `ST`
- Immediate: `LDI`
- Control Flow: `JMP`, `JZ`, `JNZ`, `HLT`

## Example Programs

### Hello World (`programs/hello.asm`)

Prints `Hello, World!` via memory-mapped output.

### Fibonacci (`programs/fibonacci.asm`)

Calculates the first 10 Fibonacci values and stores results in memory.

### Timer (`programs/timer.asm`)

Shows Fetch/Compute/Store style execution by counting from 10 down to 0.

## Architecture

For detailed architecture notes, see `docs/CPU_SCHEMATIC.md`.

### Architecture Diagram

CPU Architecture Diagram

## Design Principles

- **Modularity**: Components are isolated and clearly defined
- **Traceability**: Control flow is visible in trace output
- **Debuggability**: Registers, memory, and complete state are easy to inspect
- **Extensibility**: Instruction set and components can be expanded easily

## Videos

- Fibonacci Walk Through: Fibonacci Walkthrough
- Architecture: Architecture

## Notes

- Instructions are 16 bits wide
- Memory uses little-endian format
- Immediate values are 6-bit signed (`-32` to `31`)
- Program Counter increments by 2 bytes per instruction
- Stack Pointer initializes at `0xFFFF` (top of memory)

## License

This project is part of a course assignment.
