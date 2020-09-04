#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <stdio.h>

#include "color.hpp"
#include "vm.hpp"
#include "types.hpp"

void disassembleOp(std::vector<uint8_t>::iterator &op, std::vector<Value> constants, std::vector<int> lines, int position) {
    printf("\n| byte %04d lines %04d | ", position, lines[position]);
    switch (*op) {
        case OP_CONSTANT:
            op++;
            COLOR("OP_CONSTANT", DISPLAY_AQUA);
            std::cout << "  position: " << (int) *op << "  value: " << getPrintable(constants[*op]);
            break;
        case OP_PRINT_TOP: COLOR("OP_PRINT_TOP", DISPLAY_AQUA); break;
        case OP_NEGATE: COLOR("OP_NEGATE", DISPLAY_AQUA); break;
        case OP_NOT: COLOR("OP_NOT", DISPLAY_AQUA); break;
        case OP_ADD: COLOR("OP_ADD", DISPLAY_AQUA); break;
        case OP_SUB: COLOR("OP_SUB", DISPLAY_AQUA); break;
        case OP_MUL: COLOR("OP_MUL", DISPLAY_AQUA); break;
        case OP_DIV: COLOR("OP_DIV", DISPLAY_AQUA); break;
        case OP_CONCATENATE: COLOR("OP_CONCATENATE", DISPLAY_AQUA); break;
        case OP_JUMP_FALSE_IFv:
        case OP_JUMP_FALSE:
            COLOR("OP_JUMP_FALSE", DISPLAY_AQUA);
            std::cout << " to " << (int) *(op+1);
            op++;
            break;
        case OP_BREAK: COLOR("OP_BREAK", DISPLAY_AQUA); break;
        case OP_JUMP:
            COLOR("OP_JUMP", DISPLAY_AQUA);
            std::cout << " to " << (int) *(op+1);
            op++;
            break;
        case OP_EQUALITY: COLOR("OP_EQUALITY", DISPLAY_AQUA); break;
        case OP_LESS: COLOR("OP_LESS", DISPLAY_AQUA); break;
        case OP_GREATER: COLOR("OP_GREATER", DISPLAY_AQUA); break;
        case OP_LESS_EQ: COLOR("OP_LESS_EQ", DISPLAY_AQUA); break;
        case OP_GREATER_EQ: COLOR("OP_GREATER_EQ", DISPLAY_AQUA); break;
        case OP_NOT_EQ: COLOR("OP_NOT_EQ", DISPLAY_AQUA); break;
        case OP_POP_TOP: COLOR("OP_POP_TOP", DISPLAY_AQUA); break;
        case OP_DECL_FN: {
            op++;
            COLOR("OP_DECL_FN", DISPLAY_AQUA);
            std::cout << " with " << (int)*op;
            break;
        }
        case OP_VARIABLE: COLOR("OP_VARIABLE", DISPLAY_AQUA); break;
        case OP_VARIABLE_MUT: COLOR("OP_VARIABLE_MUT", DISPLAY_AQUA); break;
        case OP_RETRIEVE:
            op++;
            COLOR("OP_RETRIEVE", DISPLAY_AQUA);
            std::cout << "  position: " << (int) *op << "  lexeme: " << constants[*op].string;
            break;
        case OP_SET_VARIABLE: COLOR("OP_SET_VARIABLE", DISPLAY_AQUA); break;
        case OP_IMUT: COLOR("OP_IMUT", DISPLAY_AQUA); break;
        case OP_BEGIN_SCOPE: COLOR("OP_BEGIN_SCOPE", DISPLAY_AQUA); break;
        case OP_END_SCOPE: COLOR("OP_END_SCOPE", DISPLAY_AQUA); break;
        case OP_CALL: COLOR("OP_CALL", DISPLAY_AQUA); break;
        case OP_EMPTY_STACK: COLOR("OP_EMPTY_STACK", DISPLAY_AQUA); break;
        case OP_RETURN_TOP: COLOR("OP_RETURN_TOP", DISPLAY_AQUA); break;
        case OP_DISASSEMBLE_CONSTANTS: COLOR("OP_DISASSEMBLE_CONSTANTS", DISPLAY_AQUA); break;
        case OP_DISASSEMBLE_SCOPES: COLOR("OP_DISASSEMBLE_SCOPES", DISPLAY_AQUA); break;
        case OP_DISASSEMBLE_STACK: COLOR("OP_DISASSEMBLE_STACK", DISPLAY_AQUA); break;
        case OP_AT: COLOR("OP_AT", DISPLAY_AQUA); break;
        case OP_REQUIRE_BOOL: COLOR("OP_REQUIRE_BOOL", DISPLAY_AQUA); break;
        case OP_REQUIRE_VOID: COLOR("OP_REQUIRE_VOID", DISPLAY_AQUA); break;
        case OP_REQUIRE_NUM: COLOR("OP_REQUIRE_NUM", DISPLAY_AQUA); break;
        case OP_REQUIRE_STR: COLOR("OP_REQUIRE_STR", DISPLAY_AQUA); break;
        case OP_CONVERT:
            op++;
            COLOR("OP_CONVERT", DISPLAY_AQUA);
            std::cout << " to " << (int)*op;
        case OP_DECL_LIST: COLOR("OP_DECL_LIST", DISPLAY_AQUA); break;
        case OP_PUSH_LIST: COLOR("OP_PUSH_LIST", DISPLAY_AQUA); break;
        case OP_POP_LIST: COLOR("OP_POP_LIST", DISPLAY_AQUA); break;
        case OP_BACK_LIST: COLOR("OP_BACK_LIST", DISPLAY_AQUA); break;
        case OP_FRONT_LIST: COLOR("OP_FRONT_LIST", DISPLAY_AQUA); break;
        case OP_INDEX_LIST: COLOR("OP_INDEX_LIST", DISPLAY_AQUA); break;
        case OP_SIZEOF: COLOR("OP_SIZEOF", DISPLAY_AQUA); break;
        case OP_DECL_LIST_INDEX: COLOR("OP_DECL_LIST_INDEX", DISPLAY_AQUA); break;
        case OP_CONVERT_ASCII: COLOR("OP_CONVERT_ASCII", DISPLAY_AQUA); break;
        case OP_GETCH: COLOR("OP_GETCH", DISPLAY_AQUA); break;
        case OP_GETS: COLOR("OP_GETS", DISPLAY_AQUA); break;
        default:
            COLOR("bug in opcode, could not identify command.", DISPLAY_RED);
            break;
    }
    std::cerr << "   ";
}

void Machine::disassembleOpcode() {
    COLOR("\n== opcode ==", DISPLAY_YELLOW);
    for (auto op = opcode.begin(); op < opcode.end(); op++) {
        disassembleOp(op, constants, lines, op-opcode.begin());
    }
    COLOR("\n== end ==\n", DISPLAY_YELLOW);
}

void Machine::disassembleConstants() {
    COLOR("\n== constants ==\n", DISPLAY_YELLOW);
    for (int i = 0; i < constants.size(); i++) std::cout << i << ": " << getPrintable(constants[i]) << std::endl;
    COLOR("== end ==\n", DISPLAY_YELLOW);
}

void Machine::disassembleStack() {
    COLOR("\n== stack ==\n", DISPLAY_YELLOW);
    std::stack<Value> copy = value_pool;
    for (int count = copy.size()-1; !copy.empty(); count--) {
        std::cout << count << ": " << getPrintable(copy.top()) << std::endl;
        copy.pop();
    }
    COLOR("== end ==\n", DISPLAY_YELLOW);
}

void Machine::disassembleScopes() {
    #define SCOPE_AT() scope-scopes.rbegin()
    #define SCOPE (*scope)
    #define VARS SCOPE.variables

    for (auto scope = scopes.rbegin(); scope < scopes.rend(); scope++) {
        COLOR(std::string("\n== scope ") + std::to_string(SCOPE_AT()) + " map ==\n", DISPLAY_YELLOW);
        for (std::map<std::string, Value>::iterator it = VARS.begin(); it != VARS.end(); it++) {
            std::cout << it->first << " : " << getPrintable(it->second) << std::endl;
        }
        COLOR("== end ==\n", DISPLAY_YELLOW);
    }

    #undef SCOPE_AT
    #undef SCOPE
    #undef VARS
}