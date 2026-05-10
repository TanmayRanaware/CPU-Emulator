#pragma once

#include "cpu/isa.hpp"
#include <vector>
#include <string>
#include <map>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <stdexcept>

namespace assembler {

// Assembler for converting assembly code to machine code
class Assembler {
private:
    std::map<std::string, uint16_t> labels;
    std::vector<std::string>        lines;
    uint16_t                        current_address;

    // Tokenize a line
    std::vector<std::string> tokenize(const std::string& line) {
        std::vector<std::string> tokens;
        std::stringstream ss(line);
        std::string token;

        while (ss >> token) {
            // Remove commas
            token.erase(std::remove(token.begin(), token.end(), ','), token.end());
            // Strip leading # for immediates
            if (!token.empty() && token[0] == '#')
                token = token.substr(1);
            tokens.push_back(token);
        }
        return tokens;
    }

    // Parse register (R0–R7, case-insensitive)
    uint8_t parse_register(const std::string& reg) {
        if (reg.length() >= 2 && (reg[0] == 'R' || reg[0] == 'r')) {
            int num = std::stoi(reg.substr(1));
            if (num >= 0 && num <= 7)
                return static_cast<uint8_t>(num);
        }
        throw std::runtime_error("Invalid register: " + reg);
    }

    // Parse immediate value or label reference
    int16_t parse_immediate(const std::string& imm,
                            const std::map<std::string, uint16_t>& lbl_map,
                            uint16_t current_addr) {
        if (lbl_map.find(imm) != lbl_map.end()) {
            return static_cast<int16_t>(lbl_map.at(imm))
                 - static_cast<int16_t>(current_addr);
        }
        try {
            if (imm.size() > 2 && (imm.substr(0, 2) == "0x" || imm.substr(0, 2) == "0X"))
                return static_cast<int16_t>(std::stoul(imm, nullptr, 16));
            return static_cast<int16_t>(std::stoi(imm));
        } catch (...) {
            throw std::runtime_error("Invalid immediate value: " + imm);
        }
    }

    // Convert opcode string to enum
    cpu::Opcode parse_opcode(const std::string& op) {
        std::string up = op;
        std::transform(up.begin(), up.end(), up.begin(), ::toupper);

        if (up == "NOP")  return cpu::Opcode::NOP;
        if (up == "ADD")  return cpu::Opcode::ADD;
        if (up == "SUB")  return cpu::Opcode::SUB;
        if (up == "AND")  return cpu::Opcode::AND;
        if (up == "OR")   return cpu::Opcode::OR;
        if (up == "XOR")  return cpu::Opcode::XOR;
        if (up == "NOT")  return cpu::Opcode::NOT;
        if (up == "SHL")  return cpu::Opcode::SHL;
        if (up == "SHR")  return cpu::Opcode::SHR;
        if (up == "LD")   return cpu::Opcode::LD;
        if (up == "ST")   return cpu::Opcode::ST;
        if (up == "LDI")  return cpu::Opcode::LDI;
        if (up == "JMP")  return cpu::Opcode::JMP;
        if (up == "JZ")   return cpu::Opcode::JZ;
        if (up == "JNZ")  return cpu::Opcode::JNZ;
        if (up == "HLT")  return cpu::Opcode::HLT;
        // CALL and RET are handled specially — they use NOP opcode
        throw std::runtime_error("Unknown opcode: " + op);
    }

public:
    Assembler() : current_address(0) {}

    // Assemble source code to machine code
    std::vector<uint16_t> assemble(const std::string& source) {
        labels.clear();
        lines.clear();
        current_address = 0;

        // ── First pass: collect labels ─────────────────────────────────────────
        std::stringstream ss(source);
        std::string       line;
        uint16_t          addr = 0;

        while (std::getline(ss, line)) {
            // Strip comments
            size_t comment = line.find(';');
            if (comment != std::string::npos)
                line = line.substr(0, comment);

            // Trim whitespace
            auto trim = [](std::string& s) {
                s.erase(0, s.find_first_not_of(" \t"));
                auto pos = s.find_last_not_of(" \t");
                if (pos != std::string::npos) s.erase(pos + 1);
                else s.clear();
            };
            trim(line);
            if (line.empty()) continue;

            size_t colon = line.find(':');
            if (colon != std::string::npos) {
                std::string label = line.substr(0, colon);
                trim(label);
                labels[label] = addr;

                std::string rest = line.substr(colon + 1);
                trim(rest);
                if (!rest.empty()) {
                    lines.push_back(rest);
                    addr += 2;
                }
            } else {
                lines.push_back(line);
                addr += 2;
            }
        }

        // ── Second pass: encode instructions ───────────────────────────────────
        std::vector<uint16_t> program;
        addr = 0;

        for (const auto& ln : lines) {
            auto tokens = tokenize(ln);
            if (tokens.empty()) continue;

            // Normalize mnemonic to uppercase
            std::string mnem = tokens[0];
            std::transform(mnem.begin(), mnem.end(), mnem.begin(), ::toupper);

            cpu::Instruction instr;
            instr.rd           = 0;
            instr.rs1          = 0;
            instr.rs2          = 0;
            instr.imm          = 0;
            instr.is_immediate = false;

            // ── CALL RS1 ──────────────────────────────────────────────────────
            if (mnem == "CALL") {
                if (tokens.size() < 2)
                    throw std::runtime_error("CALL requires a register operand");
                instr.opcode       = cpu::Opcode::NOP;
                instr.rd           = static_cast<uint8_t>(cpu::NopSubOp::CALL);
                instr.rs1          = parse_register(tokens[1]);
                instr.is_immediate = false;

            // ── RET ───────────────────────────────────────────────────────────
            } else if (mnem == "RET") {
                instr.opcode       = cpu::Opcode::NOP;
                instr.rd           = static_cast<uint8_t>(cpu::NopSubOp::RET);
                instr.is_immediate = false;

            // ── All other instructions ─────────────────────────────────────────
            } else {
                instr.opcode = parse_opcode(mnem);

                switch (instr.opcode) {
                    case cpu::Opcode::NOP:
                    case cpu::Opcode::HLT:
                        break;

                    case cpu::Opcode::NOT:
                        if (tokens.size() < 3)
                            throw std::runtime_error("NOT requires 2 operands");
                        instr.rd  = parse_register(tokens[1]);
                        instr.rs1 = parse_register(tokens[2]);
                        break;

                    case cpu::Opcode::LDI:
                        if (tokens.size() < 3)
                            throw std::runtime_error("LDI requires 2 operands");
                        instr.rd           = parse_register(tokens[1]);
                        instr.imm          = static_cast<int8_t>(
                                                parse_immediate(tokens[2], labels, addr));
                        instr.is_immediate = true;
                        break;

                    case cpu::Opcode::SHL:
                    case cpu::Opcode::SHR:
                    case cpu::Opcode::LD:
                    case cpu::Opcode::ST:
                        if (tokens.size() < 4)
                            throw std::runtime_error(mnem + " requires 3 operands");
                        instr.rd           = parse_register(tokens[1]);
                        instr.rs1          = parse_register(tokens[2]);
                        instr.imm          = static_cast<int8_t>(
                                                parse_immediate(tokens[3], labels, addr));
                        instr.is_immediate = true;
                        break;

                    case cpu::Opcode::JMP:
                    case cpu::Opcode::JZ:
                    case cpu::Opcode::JNZ: {
                        if (tokens.size() < 3)
                            throw std::runtime_error("Jump instruction requires 2 operands");
                        instr.rs1          = parse_register(tokens[1]);
                        instr.is_immediate = true;

                        std::string imm_str = tokens[2];
                        if (labels.find(imm_str) != labels.end()) {
                            uint16_t target = labels.at(imm_str);
                            int16_t  offset = static_cast<int16_t>(target)
                                            - static_cast<int16_t>(addr + 2);
                            if (offset < -32 || offset > 31)
                                throw std::runtime_error(
                                    "Jump offset out of range (-32 to 31): "
                                    + std::to_string(offset));
                            instr.imm = static_cast<int8_t>(offset);
                        } else {
                            instr.imm = static_cast<int8_t>(
                                            parse_immediate(imm_str, labels, addr));
                        }
                        break;
                    }

                    default: {
                        // OP RD, RS1, RS2  or  OP RD, RS1, #IMM
                        if (tokens.size() < 4)
                            throw std::runtime_error(mnem + " requires 3 operands");
                        instr.rd  = parse_register(tokens[1]);
                        instr.rs1 = parse_register(tokens[2]);

                        if (tokens[3][0] == 'R' || tokens[3][0] == 'r') {
                            instr.rs2          = parse_register(tokens[3]);
                            instr.is_immediate = false;
                        } else {
                            instr.imm          = static_cast<int8_t>(
                                                    parse_immediate(tokens[3], labels, addr));
                            instr.is_immediate = true;
                        }
                        break;
                    }
                }
            }

            program.push_back(instr.encode());
            addr += 2;
        }

        return program;
    }

    // Get label addresses
    const std::map<std::string, uint16_t>& get_labels() const { return labels; }
};

} // namespace assembler
