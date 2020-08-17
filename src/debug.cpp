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
        case OP_JUMP_FALSE: {
            std::cout << "OP_JUMP_FALSE to " << (int) *(op+1);
            op++;
            break;
        }
        case OP_BEGIN: {
            std::cout << "OP_BEGIN";
            break;
        }
        case OP_EQUALITY: {
            std::cout << "OP_EQUALITY";
            break;
        }
        case OP_LESS: {
            std::cout << "OP_LESS";
            break;
        }
        case OP_GREATER: {
            std::cout << "OP_GREATER";
            break;
        }
        case OP_LESS_EQ: {
            std::cout << "OP_LESS_EQ";
            break;
        }
        case OP_GREATER_EQ: {
            std::cout << "OP_GREATER_EQ";
            break;
        }
        case OP_NOT_EQ: {
            std::cout << "OP_NOT_EQ";
            break;
        }
        case OP_POP_TOP: {
            std::cout << "OP_POP_TOP";
            break;
        }
        case OP_VARIABLE: {
            std::cout << "OP_VARIABLE";
            break;
        }
        case OP_VARIABLE_MUT: {
            std::cout << "OP_VARIABLE_MUT";
            break;
        }
        case OP_RETRIEVE: {
            op++;
            std::cout << "OP_RETRIEVE  position: " << (int) *op << "  lexeme: " << constants[*op].string;
            break;
        }
        case OP_SET_VARIABLE: {
            std::cout << "OP_SET_VARIABLE";
            break;
        }
        case OP_IMUT: {
            std::cout << "OP_IMUT";
            break;
        }
        case OP_BEGIN_SCOPE: {
            std::cout << "OP_BEGIN_SCOPE";
            break;
        }
        case OP_END_SCOPE: {
            std::cout << "OP_END_SCOPE";
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