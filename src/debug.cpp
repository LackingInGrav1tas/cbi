#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <stdio.h>

#include "vm.hpp"
#include "types.hpp"
#include "color.hpp"

void disassembleOp(std::vector<uint8_t>::iterator &op, std::vector<Value> constants, std::vector<int> lines, int position) {
    printf("\n| byte %04d lines %04d | ", position, lines[position]);
    switch (*op) {
        case OP_BLANK: {
            //std::cout << "OP_BLANK";
            COLOR("OP_BLANK", 9);
            break;
        }
        case OP_CONSTANT: {
            op++;
            COLOR("OP_CONSTANT", 9);
            std::cout << "  position: " << (int) *op << "  value: " << getPrintable(constants[*op]);
            break;
        }
        case OP_PRINT_TOP: {
            COLOR("OP_PRINT_TOP", 9);
            break;
        }
        case OP_NEGATE: {
            COLOR("OP_NEGATE", 9);
            break;
        }
        case OP_NOT: {
            COLOR("OP_NOT", 9);
            break;
        }
        case OP_ADD: {
            COLOR("OP_ADD", 9);
            break;
        }
        case OP_SUB: {
            COLOR("OP_SUB", 9);
            break;
        }
        case OP_MUL: {
            COLOR("OP_MUL", 9);
            break;
        }
        case OP_DIV: {
            COLOR("OP_DIV", 9);
            break;
        }
        case OP_CONCATENATE: {
            COLOR("OP_CONCATENATE", 9);
            break;
        }
        case OP_JUMP_FALSE: {
            COLOR("OP_JUMP_FALSE", 9);
            std::cout << " to " << (int) *(op+1);
            op++;
            break;
        }
        case OP_JUMP: {
            COLOR("OP_JUMP", 9);
            std::cout << " to " << (int) *(op+1);
            op++;
            break;
        }
        case OP_EQUALITY: {
            COLOR("OP_EQUALITY", 9);
            break;
        }
        case OP_LESS: {
            COLOR("OP_LESS", 9);
            break;
        }
        case OP_GREATER: {
            COLOR("OP_GREATER", 9);
            break;
        }
        case OP_LESS_EQ: {
            COLOR("OP_LESS_EQ", 9);
            break;
        }
        case OP_GREATER_EQ: {
            COLOR("OP_GREATER_EQ", 9);
            break;
        }
        case OP_NOT_EQ: {
            COLOR("OP_NOT_EQ", 9);
            break;
        }
        case OP_POP_TOP: {
            COLOR("OP_POP_TOP", 9);
            break;
        }
        case OP_VARIABLE: {
            COLOR("OP_VARIABLE", 9);
            break;
        }
        case OP_VARIABLE_MUT: {
            COLOR("OP_VARIABLE_MUT", 9);
            break;
        }
        case OP_RETRIEVE: {
            op++;
            COLOR("OP_RETRIEVE", 9);
            std::cout << "  position: " << (int) *op << "  lexeme: " << constants[*op].string;
            break;
        }
        case OP_SET_VARIABLE: {
            COLOR("OP_SET_VARIABLE", 9);
            break;
        }
        case OP_IMUT: {
            COLOR("OP_IMUT", 9);
            break;
        }
        case OP_BEGIN_SCOPE: {
            COLOR("OP_BEGIN_SCOPE", 9);
            break;
        }
        case OP_END_SCOPE: {
            COLOR("OP_END_SCOPE", 9);
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

void Machine::disassembleScopes() {
    #define SCOPE_AT() scope-scopes.rbegin()
    #define SCOPE (*scope)
    #define VARS SCOPE.variables

    for (auto scope = scopes.rbegin(); scope < scopes.rend(); scope++) {
        std::cout << "== scope " << SCOPE_AT() << " map ==" << std::endl;
        for (std::map<std::string, Value>::iterator it = VARS.begin(); it != VARS.end(); it++) {
            std::cout << it->first << " : " << getPrintable(it->second) << std::endl;
        }
        std::cout << "== end ==" << std::endl;
    }

    #undef SCOPE_AT
    #undef VARS
}