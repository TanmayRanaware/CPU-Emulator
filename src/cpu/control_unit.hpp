#pragma once

#include "isa.hpp"
#include "bus.hpp"
#include "registers.hpp"
#include "alu.hpp"
#include "memory.hpp"
#include <iostream>
#include <iomanip>

namespace cpu {

// Control Unit - orchestrates CPU operations
class ControlUnit {
private:
    bool trace_enabled;
    bool halted;
    uint64_t cycle_count;
    
public:
    ControlUnit(bool trace = false) 
        : trace_enabled(trace), halted(false), cycle_count(0) {}
    
    void enable_trace(bool enable) { trace_enabled = enable; }
    bool is_halted() const { return halted; }
    uint64_t get_cycle_count() const { return cycle_count; }
    
    // Execute one instruction cycle (Fetch-Decode-Execute)
    bool execute_cycle(Memory& memory, GPRs& gprs, SPRs& sprs, BusSystem& buses) {
        if (halted) return false;
        
        cycle_count++;
        
        if (trace_enabled) {
            std::cout << "\n=== Cycle " << cycle_count << " ===" << std::endl;
            std::cout << "PC: 0x" << std::hex << std::setw(4) << std::setfill('0') 
                      << sprs.PC << std::dec << std::endl;
        }
        
        // FETCH: Get instruction from memory
        buses.instruction_bus.address = sprs.PC;
        buses.instruction_bus.read_enable = true;
        uint16_t instruction_word = memory.read_word(sprs.PC);
        buses.instruction_bus.data = instruction_word;
        buses.instruction_bus.read_enable = false;
        
        if (trace_enabled) {
            std::cout << "[FETCH] Instruction at PC: 0x" << std::hex << std::setw(4) 
                      << std::setfill('0') << instruction_word << std::dec << std::endl;
        }
        
        // DECODE: Parse instruction
        Instruction instr = Instruction::decode(instruction_word);
        
        if (trace_enabled) {
            std::cout << "[DECODE] " << instr.mnemonic() << std::endl;
        }
        
        // EXECUTE: Perform operation
        bool pc_updated = false;
        
        switch (instr.opcode) {
            case Opcode::NOP:
                // No operation
                break;
                
            case Opcode::ADD:
            case Opcode::SUB:
            case Opcode::AND:
            case Opcode::OR:
            case Opcode::XOR: {
                // Arithmetic/Logic operations
                int16_t val1 = gprs[instr.rs1];
                int16_t val2 = instr.is_immediate ? instr.imm : gprs[instr.rs2];
                
                ALUResult result;
                switch (instr.opcode) {
                    case Opcode::ADD:
                        result = ALU::add(val1, val2);
                        break;
                    case Opcode::SUB:
                        result = ALU::subtract(val1, val2);
                        break;
                    case Opcode::AND:
                        result = ALU::and_op(val1, val2);
                        break;
                    case Opcode::OR:
                        result = ALU::or_op(val1, val2);
                        break;
                    case Opcode::XOR:
                        result = ALU::xor_op(val1, val2);
                        break;
                    default:
                        break;
                }
                
                gprs[instr.rd] = result.output;
                sprs.flags.Z = result.zero;
                sprs.flags.N = result.negative;
                sprs.flags.C = result.carry;
                sprs.flags.V = result.overflow;
                
                if (trace_enabled) {
                    std::cout << "[EXECUTE] R" << static_cast<int>(instr.rd) 
                              << " = " << val1 << " op " << val2 << " = " 
                              << result.output << std::endl;
                }
                break;
            }
            
            case Opcode::NOT: {
                gprs[instr.rd] = ~gprs[instr.rs1];
                sprs.flags.Z = (gprs[instr.rd] == 0);
                sprs.flags.N = (gprs[instr.rd] < 0);
                break;
            }
            
            case Opcode::SHL:
            case Opcode::SHR: {
                int16_t val = gprs[instr.rs1];
                int16_t shift = instr.imm;
                
                ALUResult result;
                if (instr.opcode == Opcode::SHL) {
                    result = ALU::shift_left(val, shift);
                } else {
                    result = ALU::shift_right(val, shift);
                }
                
                gprs[instr.rd] = result.output;
                sprs.flags.Z = result.zero;
                sprs.flags.N = result.negative;
                sprs.flags.C = result.carry;
                
                if (trace_enabled) {
                    std::cout << "[EXECUTE] R" << static_cast<int>(instr.rd) 
                              << " = R" << static_cast<int>(instr.rs1) 
                              << " shift " << static_cast<int>(shift) << std::endl;
                }
                break;
            }
            
            case Opcode::LDI: {
                // Load immediate (sign-extended)
                gprs[instr.rd] = static_cast<int16_t>(instr.imm);
                if (trace_enabled) {
                    std::cout << "[EXECUTE] R" << static_cast<int>(instr.rd) 
                              << " = " << static_cast<int>(instr.imm) << std::endl;
                }
                break;
            }
            
            case Opcode::LD: {
                // Load from memory: RD = MEM[RS1 + IMM]
                uint16_t addr = static_cast<uint16_t>(gprs[instr.rs1] + instr.imm);
                buses.control_bus.mem_read = true;
                buses.info_bus.data = addr;
                buses.info_bus.valid = true;
                uint16_t value = memory.read_word(addr);
                buses.control_bus.mem_read = false;
                buses.info_bus.valid = false;
                gprs[instr.rd] = static_cast<int16_t>(value);
                
                if (trace_enabled) {
                    std::cout << "[EXECUTE] R" << static_cast<int>(instr.rd) 
                              << " = MEM[0x" << std::hex << addr << "] = " 
                              << std::dec << gprs[instr.rd] << std::endl;
                }
                break;
            }
            
            case Opcode::ST: {
                // Store to memory: MEM[RS1 + IMM] = RD
                uint16_t addr = static_cast<uint16_t>(gprs[instr.rs1] + instr.imm);
                buses.control_bus.mem_write = true;
                buses.info_bus.data = static_cast<uint16_t>(gprs[instr.rd]);
                buses.info_bus.valid = true;
                memory.write_word(addr, buses.info_bus.data);
                buses.control_bus.mem_write = false;
                buses.info_bus.valid = false;
                
                if (trace_enabled) {
                    std::cout << "[EXECUTE] MEM[0x" << std::hex << addr << "] = R" 
                              << std::dec << static_cast<int>(instr.rd) 
                              << " = " << gprs[instr.rd] << std::endl;
                }
                break;
            }
            
            case Opcode::JMP: {
                // Jump: PC = RS1 + IMM (relative to current PC if RS1 is 0)
                // The assembler calculates offset as (target - current_PC - 2)
                // So we need: new_PC = current_PC + 2 + offset = current_PC + 2 + (target - current_PC - 2) = target
                // But the instruction format is PC = RS1 + IMM, so if RS1=0: PC = 0 + IMM (wrong!)
                // Fix: For relative jumps (RS1 == 0), use current PC as base
                uint16_t base = (gprs[instr.rs1] == 0) ? (sprs.PC + 2) : gprs[instr.rs1];
                uint16_t new_pc = static_cast<uint16_t>(base + static_cast<int16_t>(instr.imm));
                sprs.PC = new_pc;
                pc_updated = true;
                
                if (trace_enabled) {
                    std::cout << "[EXECUTE] Jump to 0x" << std::hex << new_pc << std::dec << std::endl;
                }
                break;
            }
            
            case Opcode::JZ: {
                // Jump if zero flag is set
                if (sprs.flags.Z) {
                    uint16_t base = (gprs[instr.rs1] == 0) ? (sprs.PC + 2) : gprs[instr.rs1];
                    uint16_t new_pc = static_cast<uint16_t>(base + static_cast<int16_t>(instr.imm));
                    sprs.PC = new_pc;
                    pc_updated = true;
                    if (trace_enabled) {
                        std::cout << "[EXECUTE] Jump (Z=1) to 0x" << std::hex << new_pc << std::dec << std::endl;
                    }
                } else {
                    if (trace_enabled) {
                        std::cout << "[EXECUTE] Jump skipped (Z=0)" << std::endl;
                    }
                }
                break;
            }
            
            case Opcode::JNZ: {
                // Jump if zero flag is not set
                if (!sprs.flags.Z) {
                    uint16_t base = (gprs[instr.rs1] == 0) ? (sprs.PC + 2) : gprs[instr.rs1];
                    uint16_t new_pc = static_cast<uint16_t>(base + static_cast<int16_t>(instr.imm));
                    sprs.PC = new_pc;
                    pc_updated = true;
                    if (trace_enabled) {
                        std::cout << "[EXECUTE] Jump (Z=0) to 0x" << std::hex << new_pc << std::dec << std::endl;
                    }
                } else {
                    if (trace_enabled) {
                        std::cout << "[EXECUTE] Jump skipped (Z=1)" << std::endl;
                    }
                }
                break;
            }
            
            case Opcode::HLT: {
                halted = true;
                if (trace_enabled) {
                    std::cout << "[EXECUTE] HALT" << std::endl;
                }
                return false;
            }
        }
        
        // Update program counter if not modified by instruction
        if (!pc_updated) {
            sprs.PC += 2;  // Instructions are 2 bytes
        }
        
        if (trace_enabled) {
            std::cout << "[STORE] PC updated to 0x" << std::hex << std::setw(4) 
                      << std::setfill('0') << sprs.PC << std::dec << std::endl;
        }
        
        return true;
    }
};

} // namespace cpu

