#pragma once

#include <cstdint>
#include <iostream>
#include <iomanip>

namespace cpu {

// General Purpose Registers (8 registers)
struct GPRs {
    int16_t r0 = 0;
    int16_t r1 = 0;
    int16_t r2 = 0;
    int16_t r3 = 0;
    int16_t r4 = 0;
    int16_t r5 = 0;
    int16_t r6 = 0;
    int16_t r7 = 0;

    // Access register by index (0-7)
    int16_t& operator[](int index) {
        switch(index) {
            case 0: return r0;
            case 1: return r1;
            case 2: return r2;
            case 3: return r3;
            case 4: return r4;
            case 5: return r5;
            case 6: return r6;
            case 7: return r7;
            default: return r0;
        }
    }

    const int16_t& operator[](int index) const {
        switch(index) {
            case 0: return r0;
            case 1: return r1;
            case 2: return r2;
            case 3: return r3;
            case 4: return r4;
            case 5: return r5;
            case 6: return r6;
            case 7: return r7;
            default: return r0;
        }
    }

    void print() const {
        std::cout << "=== General Purpose Registers ===" << std::endl;
        for (int i = 0; i < 8; i++) {
            std::cout << "R" << i << ": 0x" << std::hex << std::setw(4) << std::setfill('0') 
                      << static_cast<uint16_t>((*this)[i]) << " (" << std::dec << (*this)[i] << ")" << std::endl;
        }
        std::cout << std::dec;
    }
};

// Special Purpose Registers
struct SPRs {
    uint16_t PC = 0;      // Program Counter
    uint16_t SP = 0xFFFF; // Stack Pointer (starts at top of memory)
    
    // Flags register (8 bits)
    struct Flags {
        bool Z = false;  // Zero flag
        bool N = false;  // Negative flag
        bool C = false;  // Carry flag
        bool V = false;  // Overflow flag
        
        uint8_t to_byte() const {
            return (Z ? 0x01 : 0) | (N ? 0x02 : 0) | (C ? 0x04 : 0) | (V ? 0x08 : 0);
        }
        
        void from_byte(uint8_t byte) {
            Z = (byte & 0x01) != 0;
            N = (byte & 0x02) != 0;
            C = (byte & 0x04) != 0;
            V = (byte & 0x08) != 0;
        }
    } flags;

    void print() const {
        std::cout << "=== Special Purpose Registers ===" << std::endl;
        std::cout << "PC:  0x" << std::hex << std::setw(4) << std::setfill('0') << PC << std::endl;
        std::cout << "SP:  0x" << std::hex << std::setw(4) << std::setfill('0') << SP << std::endl;
        std::cout << "FLAGS: Z=" << flags.Z << " N=" << flags.N 
                  << " C=" << flags.C << " V=" << flags.V << std::endl;
        std::cout << std::dec;
    }
};

} // namespace cpu

