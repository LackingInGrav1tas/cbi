#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <stdio.h>

#include "vm.hpp"
#include "types.hpp"

void disassembleOp(std::vector<uint8_t>::iterator &op, std::vector<Value> constants, std::vector<int> lines, int position) {
    printf("\n| byte %04d lines %04d | ", position, lines[position]);
    //std::cout << "\n| byte " << position << " lines " << lines[position]<< "|  ";
    switch (*op) {
        case OP_BLANK: {
            std::cout << "OP_BLANK";
            break;
        }
        case OP_CONSTANT: {
            op++;
            std::cout << "OP_CONSTANT  position: " << (int) *op << "  value: " << getPrintable(constants[*op]);
            break;
        }
        case OP_PRINT_TOP: {
            std::cout << "OP_PRINT_TOP";
            break;
        }
        case OP_NEGATE: {
            std::cout << "OP_NEGATE";
            break;
        }
        case OP_NOT: {
            std::cout << "OP_NOT";
            break;
        }
        case OP_ADD: {
            std::cout << "OP_ADD";
            break;
        }
        case OP_SUB: {
            std::cout << "OP_SUB";
            break;
        }
        case OP_MUL: {
            std::cout << "OP_MUL";
            break;
        }
        case OP_DIV: {
            std::cout << "OP_DIV";
            break;
        }
        case OP_CONCATENATE: {
            std::cout << "OP_CONCATENATE";
            break;
        }
        default: {
            std::cout << "bug in opcode, could not identify command.";
            break;
        }
    }
    std::cerr << "   ";
}

void Machine::disassembleOpcode() {
    std::cout << "== opcode ==";
    for (auto op = opcode.begin(); op < opcode.end(); op++) {
        disassembleOp(op, constants, lines, op-opcode.begin());
    }
    std::cout << "\n== end ==" << std::endl;
}

void Machine::disassembleConstants() {
    std::cout << "== constants ==" << std::endl;
    for (int i = 0; i < constants.size(); i++) std::cout << i << ": " << getPrintable(constants[i]) << std::endl;
    std::cout << "== end ==" << std::endl;
}

void Machine::disassembleStack() {
    std::cout << "== stack ==" << std::endl;
    std::stack<Value> copy = value_pool;
    for (int count = copy.size()-1; !copy.empty(); count--) {
        std::cout << count << ": " << getPrintable(copy.top()) << std::endl;
        copy.pop();
    }
    std::cout << "== end ==" << std::endl;
}