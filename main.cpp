#include "src/emulator.hpp"
#include "src/assembler.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// Helper function to read file
std::string read_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Interactive command interface
void print_help() {
    std::cout << "\n=== CPU Emulator Commands ===" << std::endl;
    std::cout << "load <file>      - Load and assemble program from file" << std::endl;
    std::cout << "run              - Run program until halt" << std::endl;
    std::cout << "step             - Execute one instruction" << std::endl;
    std::cout << "gpr              - Print General Purpose Registers" << std::endl;
    std::cout << "spr              - Print Special Purpose Registers" << std::endl;
    std::cout << "ram [addr] [len] - Print RAM dump (default: 0x0000, 256 bytes)" << std::endl;
    std::cout << "dump [addr] [len]- Alias for RAM dump" << std::endl;
    std::cout << "dec [addr] [cnt] - Print memory as decimal numbers (default: 0x0040, 10 words)" << std::endl;
    std::cout << "state            - Print complete CPU state" << std::endl;
    std::cout << "trace on/off     - Enable/disable instruction tracing" << std::endl;
    std::cout << "reset            - Reset CPU to initial state" << std::endl;
    std::cout << "help             - Show this help message" << std::endl;
    std::cout << "quit/exit        - Exit emulator" << std::endl;
}

int main(int argc, char* argv[]) {
    emulator::CPUEmulator emu(false);
    assembler::Assembler asm_assembler;
    bool program_loaded = false;

    std::cout << "=== Simple CPU Emulator ===" << std::endl;
    std::cout << "Type 'help' for commands" << std::endl;

    if (argc > 1) {
        try {
            std::string source = read_file(argv[1]);
            auto program = asm_assembler.assemble(source);
            emu.load_program(program);
            program_loaded = true;

            std::cout << "Program loaded: " << program.size() << " instructions" << std::endl;

            if (argc > 2 && std::string(argv[2]) == "run") {
                emu.enable_trace(false);
                emu.run();
                emu.print_state();
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1;
        }
    }

    std::string command;

    while (true) {
        std::cout << "\n> ";
        std::getline(std::cin, command);

        if (command.empty()) continue;

        std::stringstream ss(command);
        std::string cmd;
        ss >> cmd;

        if (cmd == "quit" || cmd == "exit") {
            break;

        } else if (cmd == "help") {
            print_help();

        } else if (cmd == "load") {
            std::string filename;
            ss >> filename;

            if (filename.empty()) {
                std::cout << "Usage: load <filename>" << std::endl;
                continue;
            }

            try {
                std::string source = read_file(filename);
                auto program = asm_assembler.assemble(source);
                emu.load_program(program);
                program_loaded = true;

                std::cout << "Program loaded: " << program.size() << " instructions" << std::endl;

                auto labels = asm_assembler.get_labels();
                if (!labels.empty()) {
                    std::cout << "Labels:" << std::endl;
                    for (const auto& label : labels) {
                        std::cout << "  " << label.first << ": 0x"
                                  << std::hex << label.second << std::dec << std::endl;
                    }
                }
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << std::endl;
            }

        } else if (cmd == "run") {
            if (!program_loaded) {
                std::cout << "No program loaded. Use 'load <file>' first." << std::endl;
                continue;
            }

            emu.run();
            emu.print_state();

        } else if (cmd == "step") {
            if (!program_loaded) {
                std::cout << "No program loaded. Use 'load <file>' first." << std::endl;
                continue;
            }

            if (emu.is_halted()) {
                std::cout << "CPU is halted. Reset to continue." << std::endl;
                continue;
            }

            emu.step();
            emu.print_state();

        } else if (cmd == "gpr") {
            emu.print_gprs();

        } else if (cmd == "spr") {
            emu.print_sprs();

        } else if (cmd == "ram") {
            uint16_t addr = 0x0000;
            uint16_t len = 256;
            std::string addr_str, len_str;
            ss >> addr_str >> len_str;

            if (!addr_str.empty()) {
                if (addr_str.substr(0, 2) == "0x") {
                    addr = static_cast<uint16_t>(std::stoul(addr_str, nullptr, 16));
                } else {
                    addr = static_cast<uint16_t>(std::stoul(addr_str));
                }
            }

            if (!len_str.empty()) {
                len = static_cast<uint16_t>(std::stoul(len_str));
            }

            emu.print_ram(addr, len);

        } else if (cmd == "filedump") {
            // filedump [filename] [start_addr] [length]
            std::string filename = "memdump.txt";
            uint16_t addr = 0x0000;
            uint16_t len = 0xFFFF;
            std::string fname_str, addr_str, len_str;
            ss >> fname_str >> addr_str >> len_str;
 
            if (!fname_str.empty()) filename = fname_str;
 
            if (!addr_str.empty()) {
                if (addr_str.substr(0, 2) == "0x") {
                    addr = static_cast<uint16_t>(std::stoul(addr_str, nullptr, 16));
                } else {
                    addr = static_cast<uint16_t>(std::stoul(addr_str));
                }
            }
            if (!len_str.empty()) {
                len = static_cast<uint16_t>(std::stoul(len_str));
            }
            emu.memory_dump(filename, addr, len);
        } else if (cmd == "dec" || cmd == "decimal" || cmd == "print") {
            uint16_t addr = 0x0040;
            uint16_t count = 10;
            std::string addr_str, count_str;
            ss >> addr_str >> count_str;

            if (!addr_str.empty()) {
                if (addr_str.substr(0, 2) == "0x") {
                    addr = static_cast<uint16_t>(std::stoul(addr_str, nullptr, 16));
                } else {
                    addr = static_cast<uint16_t>(std::stoul(addr_str));
                }
            }

            if (!count_str.empty()) {
                count = static_cast<uint16_t>(std::stoul(count_str));
            }

            emu.print_decimal(addr, count);

        } else if (cmd == "state") {
            emu.print_state();

        } else if (cmd == "trace") {
            std::string on_off;
            ss >> on_off;

            if (on_off == "on") {
                emu.enable_trace(true);
                std::cout << "Trace enabled" << std::endl;
            } else if (on_off == "off") {
                emu.enable_trace(false);
                std::cout << "Trace disabled" << std::endl;
            } else {
                std::cout << "Usage: trace on|off" << std::endl;
            }

        } else if (cmd == "reset") {
            emu.reset();
            program_loaded = false;
            std::cout << "CPU reset" << std::endl;

        } else {
            std::cout << "Unknown command: " << cmd << ". Type 'help' for commands." << std::endl;
        }
    }

    return 0;
}
