#pragma once

#include <cstdint>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>

namespace cpu {

// Memory class with memory-mapped I/O
class Memory {
public:
    static constexpr size_t MEMORY_SIZE = 65536;  // 64KB
    static constexpr uint16_t IO_BASE = 0xFF00;     // I/O region starts at 0xFF00
    
    // Memory-mapped I/O addresses
    static constexpr uint16_t IO_STDOUT = 0xFF00;   // Character output
    static constexpr uint16_t IO_STDIN = 0xFF01;    // Character input
    static constexpr uint16_t IO_STATUS = 0xFF02;   // Status register
    
private:
    std::vector<uint8_t> mem;
    std::string output_buffer;  // For capturing stdout
    
public:
    Memory() : mem(MEMORY_SIZE, 0) {
        // Initialize I/O status register
        mem[IO_STATUS] = 0x01;  // Ready
    }
    
    // Read byte from memory
    uint8_t read_byte(uint16_t address) const {
        if (static_cast<size_t>(address) >= MEMORY_SIZE) return 0;
        
        // Memory-mapped I/O read
        if (address == IO_STDIN) {
            // For now, return 0 (no input)
            return 0;
        }
        
        return mem[address];
    }
    
    // Write byte to memory
    void write_byte(uint16_t address, uint8_t value) {
        if (static_cast<size_t>(address) >= MEMORY_SIZE) return;
        
        // Memory-mapped I/O write
        if (address == IO_STDOUT) {
            // Output character
            char c = static_cast<char>(value);
            if (c == '\n') {
                std::cout << output_buffer << std::endl;
                output_buffer.clear();
            } else if (c >= 32 && c < 127) {
                output_buffer += c;
            }
            return;
        }
        
        mem[address] = value;
    }
    
    // Read 16-bit word (little-endian)
    uint16_t read_word(uint16_t address) const {
        if (static_cast<size_t>(address) >= MEMORY_SIZE - 1) return 0;
        uint16_t low = read_byte(address);
        uint16_t high = read_byte(address + 1);
        return low | (high << 8);
    }
    
    // Write 16-bit word (little-endian)
    void write_word(uint16_t address, uint16_t value) {
        if (static_cast<size_t>(address) >= MEMORY_SIZE - 1) return;
        write_byte(address, value & 0xFF);
        write_byte(address + 1, (value >> 8) & 0xFF);
    }
    
    // Load program into memory starting at address
    void load_program(uint16_t start_address, const std::vector<uint16_t>& program) {
        for (size_t i = 0; i < program.size(); i++) {
            write_word(start_address + (i * 2), program[i]);
        }
    }
    
    // Clear output buffer
    void clear_output() {
        output_buffer.clear();
    }
    
    // Get output buffer
    std::string get_output() const {
        return output_buffer;
    }
    
    // Print memory dump (hex format)
    void print_dump(uint16_t start = 0, uint16_t length = 256) const {
        std::cout << "=== Memory Dump (0x" << std::hex << std::setw(4) 
                  << std::setfill('0') << start << " - 0x" 
                  << std::setw(4) << (start + length - 1) << ") ===" << std::endl;
        
        for (uint16_t addr = start; addr < start + length && static_cast<size_t>(addr) < MEMORY_SIZE; addr += 16) {
            std::cout << std::hex << std::setw(4) << std::setfill('0') << addr << ": ";
            
            // Print hex bytes
            for (int i = 0; i < 16 && static_cast<size_t>(addr + i) < MEMORY_SIZE; i++) {
                std::cout << std::hex << std::setw(2) << std::setfill('0') 
                          << static_cast<int>(read_byte(addr + i)) << " ";
            }
            
            // Print ASCII representation
            std::cout << " |";
            for (int i = 0; i < 16 && static_cast<size_t>(addr + i) < MEMORY_SIZE; i++) {
                uint8_t byte = read_byte(addr + i);
                char c = (byte >= 32 && byte < 127) ? static_cast<char>(byte) : '.';
                std::cout << c;
            }
            std::cout << "|" << std::endl;
        }
        std::cout << std::dec;
    }
    
    // Print memory region as instructions
    void print_instructions(uint16_t start, uint16_t count) const {
        std::cout << "=== Instructions at 0x" << std::hex << std::setw(4) 
                  << std::setfill('0') << start << " ===" << std::endl;
        
        for (uint16_t i = 0; i < count; i++) {
            uint16_t addr = start + (i * 2);
            uint16_t instruction = read_word(addr);
            std::cout << "0x" << std::hex << std::setw(4) << std::setfill('0') << addr 
                      << ": 0x" << std::setw(4) << instruction << std::endl;
        }
        std::cout << std::dec;
    }
};

} // namespace cpu

