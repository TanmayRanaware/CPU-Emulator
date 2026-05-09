#pragma once

#include "cpu/registers.hpp"
#include "cpu/alu.hpp"
#include "cpu/bus.hpp"
#include "cpu/memory.hpp"
#include "cpu/isa.hpp"
#include "cpu/control_unit.hpp"
#include <vector>
#include <string>
#include <iomanip>
#include <iostream>

namespace emulator {

// Main CPU Emulator class
class CPUEmulator {
private:
    cpu::GPRs gprs;
    cpu::SPRs sprs;
    cpu::Memory memory;
    cpu::ControlUnit control_unit;
    cpu::BusSystem buses;
    
    bool running;
    uint16_t program_start;
    
public:
    CPUEmulator(bool trace = false) 
        : control_unit(trace), running(false), program_start(0x0000) {}
    
    // Load program into memory
    void load_program(const std::vector<uint16_t>& program, uint16_t start_address = 0x0000) {
        program_start = start_address;
        sprs.PC = start_address;
        memory.load_program(start_address, program);
    }
    
    // Run program until halt
    void run() {
        running = true;
        while (running && !control_unit.is_halted()) {
            running = control_unit.execute_cycle(memory, gprs, sprs, buses);
        }
        // Flush any remaining output in the buffer
        std::string remaining = memory.get_output();
        if (!remaining.empty()) {
            std::cout << remaining << std::endl;
            memory.clear_output();
        }
    }
    
    // Step one instruction
    void step() {
        if (!control_unit.is_halted()) {
            control_unit.execute_cycle(memory, gprs, sprs, buses);
        }
    }
    
    // Reset CPU state
    void reset() {
        gprs = cpu::GPRs();
        sprs = cpu::SPRs();
        buses.reset();
        running = false;
        sprs.PC = program_start;
    }
    
    // Print CPU state
    void print_state() const {
        std::cout << "\n=== CPU State ===" << std::endl;
        std::cout << "Cycle: " << control_unit.get_cycle_count() << std::endl;
        gprs.print();
        std::cout << std::endl;
        sprs.print();
    }
    
    // Print GPRs
    void print_gprs() const {
        gprs.print();
    }
    
    // Print SPRs
    void print_sprs() const {
        sprs.print();
    }
    
    // Print RAM
    void print_ram(uint16_t start = 0, uint16_t length = 256) const {
        memory.print_dump(start, length);
    }
    
    // Print memory values as decimal numbers
    void print_decimal(uint16_t start = 0, uint16_t count = 10) const {
        std::cout << "\n=== Memory as Decimal Numbers ===" << std::endl;
        std::cout << "Starting address: 0x" << std::hex << std::setw(4) << std::setfill('0') 
                  << start << std::dec << std::endl;
        for (uint16_t i = 0; i < count; i++) {
            uint16_t addr = start + (i * 2);  // Each value is 2 bytes (16-bit word)
            uint16_t value = memory.read_word(addr);
            std::cout << "[" << std::setw(2) << std::setfill('0') << i << "] "
                      << "0x" << std::hex << std::setw(4) << std::setfill('0') << addr 
                      << std::dec << ": " << static_cast<int16_t>(value) << std::endl;
        }
    }
    
    // Print memory as instructions
    void print_instructions(uint16_t start, uint16_t count) const {
        memory.print_instructions(start, count);
    }
    
    // Enable/disable trace
    void enable_trace(bool enable) {
        control_unit.enable_trace(enable);
    }
    
    // Get current PC
    uint16_t get_pc() const {
        return sprs.PC;
    }
    
    // Check if halted
    bool is_halted() const {
        return control_unit.is_halted();
    }
    
    // Get cycle count
    uint64_t get_cycle_count() const {
        return control_unit.get_cycle_count();
    }
    
    // Memory dump to file
    void memory_dump(const std::string& filename, uint16_t start = 0, uint16_t length = 0xFFFF) const {
        // This would write memory to file - simplified for now
        (void)start;  // Suppress unused parameter warning
        (void)length; // Suppress unused parameter warning
        std::cout << "Memory dump to " << filename << " (not implemented)" << std::endl;
    }
};

} // namespace emulator

