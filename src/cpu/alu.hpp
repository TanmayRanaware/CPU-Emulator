#pragma once

#include <cstdint>

namespace cpu {

// ALU result structure
struct ALUResult {
    int16_t output;
    bool overflow;
    bool carry;
    bool zero;
    bool negative;
};

// Arithmetic Logic Unit
// Performs arithmetic and logical operations
class ALU {
public:
    // Add two 16-bit signed integers
    static ALUResult add(int16_t a, int16_t b) {
        ALUResult result;
        int32_t sum = static_cast<int32_t>(a) + static_cast<int32_t>(b);
        
        result.output = static_cast<int16_t>(sum);
        result.carry = (sum > 32767) || (sum < -32768);
        result.overflow = (a > 0 && b > 0 && result.output < 0) || 
                          (a < 0 && b < 0 && result.output > 0);
        result.zero = (result.output == 0);
        result.negative = (result.output < 0);
        
        return result;
    }

    // Subtract b from a
    static ALUResult subtract(int16_t a, int16_t b) {
        return add(a, -b);
    }

    // Bitwise AND
    static ALUResult and_op(int16_t a, int16_t b) {
        ALUResult result;
        result.output = a & b;
        result.carry = false;
        result.overflow = false;
        result.zero = (result.output == 0);
        result.negative = (result.output < 0);
        return result;
    }

    // Bitwise OR
    static ALUResult or_op(int16_t a, int16_t b) {
        ALUResult result;
        result.output = a | b;
        result.carry = false;
        result.overflow = false;
        result.zero = (result.output == 0);
        result.negative = (result.output < 0);
        return result;
    }

    // Bitwise XOR
    static ALUResult xor_op(int16_t a, int16_t b) {
        ALUResult result;
        result.output = a ^ b;
        result.carry = false;
        result.overflow = false;
        result.zero = (result.output == 0);
        result.negative = (result.output < 0);
        return result;
    }

    // Shift left
    static ALUResult shift_left(int16_t a, int16_t shift) {
        ALUResult result;
        if (shift < 0 || shift > 15) {
            result.output = 0;
            result.carry = false;
        } else {
            result.carry = (a & (1 << (15 - shift))) != 0;
            result.output = static_cast<int16_t>(a << shift);
        }
        result.overflow = false;
        result.zero = (result.output == 0);
        result.negative = (result.output < 0);
        return result;
    }

    // Shift right (arithmetic)
    static ALUResult shift_right(int16_t a, int16_t shift) {
        ALUResult result;
        if (shift < 0 || shift > 15) {
            result.output = 0;
            result.carry = false;
        } else {
            result.carry = (a & (1 << (shift - 1))) != 0;
            result.output = static_cast<int16_t>(a >> shift);
        }
        result.overflow = false;
        result.zero = (result.output == 0);
        result.negative = (result.output < 0);
        return result;
    }

    // Compare (subtract without storing result)
    static ALUResult compare(int16_t a, int16_t b) {
        return subtract(a, b);
    }
};

} // namespace cpu

