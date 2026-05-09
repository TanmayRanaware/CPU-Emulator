#pragma once

#include "cpu/isa.hpp"
#include <vector>
#include <string>
#include <map>
#include <sstream>
#include <cctype>
#include <algorithm>

namespace assembler {

// Assembler for converting assembly code to machine code
class Assembler {
private:
    std::map<std::string, uint16_t> labels;  // Label -> address mapping
    std::vector<std::string> lines;
    uint16_t current_address;
    
    // Tokenize a line
    std::vector<std::string> tokenize(const std::string& line) {
        std::vector<std::string> tokens;
        std::stringstream ss(line);
        std::string token;
        
        while (ss >> token) {
            // Remove commas
            token.erase(std::remove(token.begin(), token.end(), ','), token.end());
            // Remove # for immediate values
            if (token[0] == '#') {
                token = token.substr(1);
            }
            tokens.push_back(token);
        }
        
        return tokens;
    }
    
    // Parse register (R0-R7)
    uint8_t parse_register(const std::string& reg) {
        if (reg.length() >= 2 && (reg[0] == 'R' || reg[0] == 'r')) {
            int num = std::stoi(reg.substr(1));
            if (num >= 0 && num <= 7) {
                return static_cast<uint8_t>(num);
            }
        }
        throw std::runtime_error("Invalid register: " + reg);
    }
    
    // Parse immediate value or label
    int16_t parse_immediate(const std::string& imm, const std::map<std::string, uint16_t>& labels, uint16_t current_addr) {
        // Check if it's a label
        if (labels.find(imm) != labels.end()) {
            int16_t offset = static_cast<int16_t>(labels.at(imm)) - static_cast<int16_t>(current_addr);
            return offset;
        }
        
        // Try to parse as number
        try {
            if (imm.substr(0, 2) == "0x" || imm.substr(0, 2) == "0X") {
                return static_cast<int16_t>(std::stoul(imm, nullptr, 16));
            }
            return static_cast<int16_t>(std::stoi(imm));
        } catch (...) {
            throw std::runtime_error("Invalid immediate value: " + imm);
        }
    }
    
    // Convert opcode string to enum
    cpu::Opcode parse_opcode(const std::string& op) {
        std::string op_upper = op;
        std::transform(op_upper.begin(), op_upper.end(), op_upper.begin(), ::toupper);
        
        if (op_upper == "NOP") return cpu::Opcode::NOP;
        if (op_upper == "ADD") return cpu::Opcode::ADD;
        if (op_upper == "SUB") return cpu::Opcode::SUB;
        if (op_upper == "AND") return cpu::Opcode::AND;
        if (op_upper == "OR") return cpu::Opcode::OR;
        if (op_upper == "XOR") return cpu::Opcode::XOR;
        if (op_upper == "NOT") return cpu::Opcode::NOT;
        if (op_upper == "SHL") return cpu::Opcode::SHL;
        if (op_upper == "SHR") return cpu::Opcode::SHR;
        if (op_upper == "LD") return cpu::Opcode::LD;
        if (op_upper == "ST") return cpu::Opcode::ST;
        if (op_upper == "LDI") return cpu::Opcode::LDI;
        if (op_upper == "JMP") return cpu::Opcode::JMP;
        if (op_upper == "JZ") return cpu::Opcode::JZ;
        if (op_upper == "JNZ") return cpu::Opcode::JNZ;
        if (op_upper == "HLT") return cpu::Opcode::HLT;
        
        throw std::runtime_error("Unknown opcode: " + op);
    }
    
public:
    Assembler() : current_address(0) {}
    
    // Assemble source code to machine code
    std::vector<uint16_t> assemble(const std::string& source) {
        labels.clear();
        lines.clear();
        current_address = 0;
        
        // First pass: collect labels
        std::stringstream ss(source);
        std::string line;
        uint16_t addr = 0;
        
        while (std::getline(ss, line)) {
            // Remove comments
            size_t comment_pos = line.find(';');
            if (comment_pos != std::string::npos) {
                line = line.substr(0, comment_pos);
            }
            
            // Trim whitespace
            line.erase(0, line.find_first_not_of(" \t"));
            line.erase(line.find_last_not_of(" \t") + 1);
            
            if (line.empty()) continue;
            
            // Check for label (ends with :)
            size_t colon_pos = line.find(':');
            if (colon_pos != std::string::npos) {
                std::string label = line.substr(0, colon_pos);
                // Trim label
                label.erase(0, label.find_first_not_of(" \t"));
                label.erase(label.find_last_not_of(" \t") + 1);
                labels[label] = addr;
                
                // Check if there's an instruction on the same line
                std::string rest = line.substr(colon_pos + 1);
                rest.erase(0, rest.find_first_not_of(" \t"));
                if (!rest.empty()) {
                    lines.push_back(rest);
                    addr += 2;
                }
            } else {
                lines.push_back(line);
                addr += 2;
            }
        }
        
        // Second pass: assemble instructions
        std::vector<uint16_t> program;
        addr = 0;
        
        for (const auto& line : lines) {
            auto tokens = tokenize(line);
            if (tokens.empty()) continue;
            
            cpu::Instruction instr;
            instr.opcode = parse_opcode(tokens[0]);
            
            // Handle different instruction formats
            switch (instr.opcode) {
                case cpu::Opcode::NOP:
                case cpu::Opcode::HLT:
                    // No operands
                    instr.rd = 0;
                    instr.rs1 = 0;
                    instr.rs2 = 0;
                    instr.imm = 0;
                    instr.is_immediate = false;
                    break;
                    
                case cpu::Opcode::NOT:
                    // NOT RD, RS1
                    if (tokens.size() < 3) throw std::runtime_error("NOT requires 2 operands");
                    instr.rd = parse_register(tokens[1]);
                    instr.rs1 = parse_register(tokens[2]);
                    instr.rs2 = 0;
                    instr.imm = 0;
                    instr.is_immediate = false;
                    break;
                    
                case cpu::Opcode::LDI:
                    // LDI RD, IMM
                    if (tokens.size() < 3) throw std::runtime_error("LDI requires 2 operands");
                    instr.rd = parse_register(tokens[1]);
                    instr.rs1 = 0;
                    instr.rs2 = 0;
                    instr.imm = static_cast<int8_t>(parse_immediate(tokens[2], labels, addr));
                    instr.is_immediate = true;
                    break;
                    
                case cpu::Opcode::SHL:
                case cpu::Opcode::SHR:
                case cpu::Opcode::LD:
                case cpu::Opcode::ST:
                    // OP RD, RS1, IMM
                    if (tokens.size() < 4) throw std::runtime_error("Instruction requires 3 operands");
                    instr.rd = parse_register(tokens[1]);
                    instr.rs1 = parse_register(tokens[2]);
                    instr.rs2 = 0;
                    instr.imm = static_cast<int8_t>(parse_immediate(tokens[3], labels, addr));
                    instr.is_immediate = true;
                    break;
                    
                case cpu::Opcode::JMP:
                case cpu::Opcode::JZ:
                case cpu::Opcode::JNZ: {
                    // JMP/JZ/JNZ RS1, IMM (target address = RS1 + IMM)
                    // For labels, compute relative offset from current PC
                    if (tokens.size() < 3) throw std::runtime_error("Jump instruction requires 2 operands");
                    instr.rd = 0;
                    instr.rs1 = parse_register(tokens[1]);
                    
                    // Check if second operand is a label
                    std::string imm_str = tokens[2];
                    if (labels.find(imm_str) != labels.end()) {
                        // Label: compute relative offset
                        uint16_t target = labels.at(imm_str);
                        int16_t offset = static_cast<int16_t>(target) - static_cast<int16_t>(addr + 2);
                        // Since immediate is only 6 bits, we can only jump within -32 to +31
                        // For larger jumps, we'd need to load address into register first
                        if (offset < -32 || offset > 31) {
                            throw std::runtime_error("Jump offset out of range (-32 to 31): " + std::to_string(offset) + 
                                                    " (target: 0x" + std::to_string(target) + ", current: 0x" + std::to_string(addr) + ")");
                        }
                        instr.imm = static_cast<int8_t>(offset);
                    } else {
                        // Immediate value
                        instr.imm = static_cast<int8_t>(parse_immediate(imm_str, labels, addr));
                    }
                    instr.rs2 = 0;
                    instr.is_immediate = true;
                    break;
                }
                    
                default:
                    // Register or immediate format: OP RD, RS1, RS2/IMM
                    if (tokens.size() < 4) throw std::runtime_error("Instruction requires 3 operands");
                    instr.rd = parse_register(tokens[1]);
                    instr.rs1 = parse_register(tokens[2]);
                    
                    // Check if third operand is register or immediate
                    if (tokens[3][0] == 'R' || tokens[3][0] == 'r') {
                        instr.rs2 = parse_register(tokens[3]);
                        instr.is_immediate = false;
                        instr.imm = 0;
                    } else {
                        instr.imm = static_cast<int8_t>(parse_immediate(tokens[3], labels, addr));
                        instr.is_immediate = true;
                        instr.rs2 = 0;
                    }
                    break;
            }
            
            program.push_back(instr.encode());
            addr += 2;
        }
        
        return program;
    }
    
    // Get label addresses
    const std::map<std::string, uint16_t>& get_labels() const {
        return labels;
    }
};

} // namespace assembler

