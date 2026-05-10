#pragma once

#include <cstdint>
#include <string>
#include <map>

namespace cpu {

// Instruction Set Architecture Definition

// Instruction format: 16-bit
// Format: [OPCODE:4][RD:3][RS1:3][RS2:3][IMM:3] or [OPCODE:4][RD:3][RS1:3][IMM:6]
// For immediate instructions, IMM is 6 bits (signed -32 to 31)
// For register instructions, RS2 is used and IMM is ignored
//
// CALL/RET are encoded as NOP sub-opcodes (opcode=0x0, rd field used as sub-type):
//   NOP:  opcode=0x0, rd=0
//   CALL: opcode=0x0, rd=1, rs1=target register (PC = rs1; push return addr to stack)
//   RET:  opcode=0x0, rd=2  (PC = pop from stack)

// Opcodes (4 bits = 16 possible instructions)
enum class Opcode : uint8_t {
    NOP  = 0x0,  // No operation (rd=0) / CALL (rd=1) / RET (rd=2)
    ADD  = 0x1,  // Add: RD = RS1 + RS2
    SUB  = 0x2,  // Subtract: RD = RS1 - RS2
    AND  = 0x3,  // Bitwise AND: RD = RS1 & RS2
    OR   = 0x4,  // Bitwise OR: RD = RS1 | RS2
    XOR  = 0x5,  // Bitwise XOR: RD = RS1 ^ RS2
    NOT  = 0x6,  // Bitwise NOT: RD = ~RS1
    SHL  = 0x7,  // Shift left: RD = RS1 << IMM
    SHR  = 0x8,  // Shift right: RD = RS1 >> IMM
    LD   = 0x9,  // Load: RD = MEM[RS1 + IMM]
    ST   = 0xA,  // Store: MEM[RS1 + IMM] = RD
    LDI  = 0xB,  // Load immediate: RD = IMM (sign extended)
    JMP  = 0xC,  // Jump: PC = RS1 + IMM
    JZ   = 0xD,  // Jump if zero: if (Z flag) PC = RS1 + IMM
    JNZ  = 0xE,  // Jump if not zero: if (!Z flag) PC = RS1 + IMM
    HLT  = 0xF   // Halt
};

// NOP sub-opcodes (encoded in the rd field when opcode == NOP)
enum class NopSubOp : uint8_t {
    NOP  = 0,  // True no-op
    CALL = 1,  // Call: push (PC+2) to stack, jump to RS1
    RET  = 2   // Return: pop return address from stack, jump to it
};

// Addressing modes
enum class AddressingMode {
    REGISTER,    // Register-register operation
    IMMEDIATE,   // Immediate value
    INDIRECT,    // Memory indirect (for LD/ST)
    DIRECT       // Direct address
};

// Instruction structure
struct Instruction {
    Opcode opcode;
    uint8_t rd;       // Destination register (0-7), or NopSubOp for NOP
    uint8_t rs1;      // Source register 1 (0-7)
    uint8_t rs2;      // Source register 2 (0-7) or unused
    int8_t  imm;      // Immediate value (-32 to 31) or unused
    bool    is_immediate;

    // Encode instruction to 16-bit word
    uint16_t encode() const {
        uint16_t word = 0;
        word |= (static_cast<uint8_t>(opcode) & 0x0F) << 12;
        word |= (rd  & 0x07) << 9;
        word |= (rs1 & 0x07) << 6;

        if (is_immediate) {
            word |= (static_cast<uint8_t>(imm) & 0x3F);
        } else {
            word |= (rs2 & 0x07) << 3;
        }

        return word;
    }

    // Decode 16-bit word to instruction
    static Instruction decode(uint16_t word) {
        Instruction instr;
        instr.opcode = static_cast<Opcode>((word >> 12) & 0x0F);
        instr.rd     = (word >> 9) & 0x07;
        instr.rs1    = (word >> 6) & 0x07;

        // Determine if immediate or register based on opcode
        instr.is_immediate = (instr.opcode == Opcode::LDI ||
                              instr.opcode == Opcode::LD  ||
                              instr.opcode == Opcode::ST  ||
                              instr.opcode == Opcode::JMP ||
                              instr.opcode == Opcode::JZ  ||
                              instr.opcode == Opcode::JNZ ||
                              instr.opcode == Opcode::SHL ||
                              instr.opcode == Opcode::SHR);

        if (instr.is_immediate) {
            // Sign-extend 6-bit immediate
            int8_t imm_raw = word & 0x3F;
            instr.imm = (imm_raw & 0x20) ? (imm_raw | 0xC0) : imm_raw;
            instr.rs2 = 0;
        } else {
            instr.rs2 = (word >> 3) & 0x07;
            instr.imm = 0;
        }

        return instr;
    }

    // Helper: is this a CALL instruction?
    bool is_call() const {
        return opcode == Opcode::NOP && rd == static_cast<uint8_t>(NopSubOp::CALL);
    }

    // Helper: is this a RET instruction?
    bool is_ret() const {
        return opcode == Opcode::NOP && rd == static_cast<uint8_t>(NopSubOp::RET);
    }

    // Helper: is this a true NOP?
    bool is_nop() const {
        return opcode == Opcode::NOP && rd == static_cast<uint8_t>(NopSubOp::NOP);
    }

    // Get instruction mnemonic
    std::string mnemonic() const {
        // Handle NOP sub-opcodes first
        if (opcode == Opcode::NOP) {
            if (rd == static_cast<uint8_t>(NopSubOp::CALL))
                return "CALL R" + std::to_string(rs1);
            if (rd == static_cast<uint8_t>(NopSubOp::RET))
                return "RET";
            return "NOP";
        }

        static const std::map<Opcode, std::string> opcode_names = {
            {Opcode::ADD, "ADD"}, {Opcode::SUB, "SUB"},
            {Opcode::AND, "AND"}, {Opcode::OR,  "OR" }, {Opcode::XOR, "XOR"},
            {Opcode::NOT, "NOT"}, {Opcode::SHL, "SHL"}, {Opcode::SHR, "SHR"},
            {Opcode::LD,  "LD" }, {Opcode::ST,  "ST" }, {Opcode::LDI, "LDI"},
            {Opcode::JMP, "JMP"}, {Opcode::JZ,  "JZ" }, {Opcode::JNZ, "JNZ"},
            {Opcode::HLT, "HLT"}
        };

        std::string name = opcode_names.at(opcode);

        if (opcode == Opcode::HLT) {
            return name;
        } else if (opcode == Opcode::NOT) {
            return name + " R" + std::to_string(rd) + ", R" + std::to_string(rs1);
        } else if (is_immediate) {
            return name + " R" + std::to_string(rd) + ", R" + std::to_string(rs1)
                        + ", #" + std::to_string(static_cast<int>(imm));
        } else {
            return name + " R" + std::to_string(rd) + ", R" + std::to_string(rs1)
                        + ", R" + std::to_string(rs2);
        }
    }
};

} // namespace cpu
