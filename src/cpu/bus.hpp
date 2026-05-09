#pragma once

#include <cstdint>
#include <string>

namespace cpu {

// Bus system for CPU communication
// Instruction Bus: Two-way communication for instruction fetch
// Info Bus: Data transfer between components
// Control Bus: One-way control signals from Control Unit

struct InstructionBus {
    uint16_t address = 0;  // Address to fetch instruction from
    uint16_t data = 0;     // Instruction data
    bool read_enable = false;
    bool write_enable = false;
    
    void reset() {
        address = 0;
        data = 0;
        read_enable = false;
        write_enable = false;
    }
    
    std::string to_string() const {
        return "IBUS[addr=0x" + std::to_string(address) + 
               ", data=0x" + std::to_string(data) + 
               ", read=" + (read_enable ? "1" : "0") + "]";
    }
};

struct InfoBus {
    uint16_t data = 0;
    bool valid = false;
    uint8_t source = 0;  // Component ID that put data on bus
    uint8_t dest = 0;    // Component ID that should read data
    
    void reset() {
        data = 0;
        valid = false;
        source = 0;
        dest = 0;
    }
    
    std::string to_string() const {
        return "INFOBUS[data=0x" + std::to_string(data) + 
               ", valid=" + (valid ? "1" : "0") + 
               ", src=" + std::to_string(source) + 
               ", dst=" + std::to_string(dest) + "]";
    }
};

struct ControlBus {
    // Control signals
    bool mem_read = false;
    bool mem_write = false;
    bool reg_read = false;
    bool reg_write = false;
    bool alu_enable = false;
    bool pc_increment = false;
    bool pc_load = false;
    bool sp_increment = false;
    bool sp_decrement = false;
    uint8_t alu_op = 0;      // ALU operation code
    uint8_t reg_select = 0;  // Register selection
    
    void reset() {
        mem_read = false;
        mem_write = false;
        reg_read = false;
        reg_write = false;
        alu_enable = false;
        pc_increment = false;
        pc_load = false;
        sp_increment = false;
        sp_decrement = false;
        alu_op = 0;
        reg_select = 0;
    }
    
    std::string to_string() const {
        std::string s = "CTRLBUS[";
        if (mem_read) s += "MEM_R ";
        if (mem_write) s += "MEM_W ";
        if (reg_read) s += "REG_R ";
        if (reg_write) s += "REG_W ";
        if (alu_enable) s += "ALU_EN ";
        if (pc_increment) s += "PC++ ";
        if (pc_load) s += "PC_LD ";
        if (sp_increment) s += "SP++ ";
        if (sp_decrement) s += "SP-- ";
        s += "ALU_OP=" + std::to_string(alu_op) + "]";
        return s;
    }
};

// Combined bus system
struct BusSystem {
    InstructionBus instruction_bus;
    InfoBus info_bus;
    ControlBus control_bus;
    
    void reset() {
        instruction_bus.reset();
        info_bus.reset();
        control_bus.reset();
    }
    
    void print() const {
        std::cout << instruction_bus.to_string() << std::endl;
        std::cout << info_bus.to_string() << std::endl;
        std::cout << control_bus.to_string() << std::endl;
    }
};

} // namespace cpu

