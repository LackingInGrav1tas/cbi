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
    #define DBCOL(str) COLOR(str, DISPLAY_AQUA)
    switch (*op) {
        case OP_CONSTANT:
            op++;
            DBCOL("OP_CONSTANT");
            std::cout << "  position: " << (int) *op << "  value: " << getPrintable(constants[*op]);
            break;
        case OP_PRINT_TOP: DBCOL("OP_PRINT_TOP"); break;
        case OP_NEGATE: DBCOL("OP_NEGATE"); break;
        case OP_NOT: DBCOL("OP_NOT"); break;
        case OP_ADD: DBCOL("OP_ADD"); break;
        case OP_SUB: DBCOL("OP_SUB"); break;
        case OP_MUL: DBCOL("OP_MUL"); break;
        case OP_DIV: DBCOL("OP_DIV"); break;
        case OP_CONCATENATE: DBCOL("OP_CONCATENATE"); break;
        case OP_JUMP_FALSE_IFv:
        case OP_JUMP_FALSE:
            DBCOL("OP_JUMP_FALSE");
            std::cout << " to " << (int) *(op+1);
            op++;
            break;
        case OP_BREAK: DBCOL("OP_BREAK"); break;
        case OP_JUMP:
            DBCOL("OP_JUMP");
            std::cout << " to " << (int) *(op+1);
            op++;
            break;
        case OP_EQUALITY: DBCOL("OP_EQUALITY"); break;
        case OP_LESS: DBCOL("OP_LESS"); break;
        case OP_GREATER: DBCOL("OP_GREATER"); break;
        case OP_LESS_EQ: DBCOL("OP_LESS_EQ"); break;
        case OP_GREATER_EQ: DBCOL("OP_GREATER_EQ"); break;
        case OP_NOT_EQ: DBCOL("OP_NOT_EQ"); break;
        case OP_POP_TOP: DBCOL("OP_POP_TOP"); break;
        case OP_DECL_FN: {
            op++;
            DBCOL("OP_DECL_FN");
            std::cout << " with " << (int)*op;
            break;
        }
        case OP_VARIABLE: DBCOL("OP_VARIABLE"); break;
        case OP_VARIABLE_MUT: DBCOL("OP_VARIABLE_MUT"); break;
        case OP_RETRIEVE:
            op++;
            DBCOL("OP_RETRIEVE");
            std::cout << "  position: " << (int) *op << "  lexeme: " << constants[*op].string;
            break;
        case OP_SET_VARIABLE: DBCOL("OP_SET_VARIABLE"); break;
        case OP_IMUT: DBCOL("OP_IMUT"); break;
        case OP_BEGIN_SCOPE: DBCOL("OP_BEGIN_SCOPE"); break;
        case OP_END_SCOPE: DBCOL("OP_END_SCOPE"); break;
        case OP_CALL: DBCOL("OP_CALL"); break;
        case OP_EMPTY_STACK: DBCOL("OP_EMPTY_STACK"); break;
        case OP_RETURN_TOP: DBCOL("OP_RETURN_TOP"); break;
        case OP_DISASSEMBLE_CONSTANTS: DBCOL("OP_DISASSEMBLE_CONSTANTS"); break;
        case OP_DISASSEMBLE_SCOPES: DBCOL("OP_DISASSEMBLE_SCOPES"); break;
        case OP_DISASSEMBLE_STACK: DBCOL("OP_DISASSEMBLE_STACK"); break;
        case OP_AT: DBCOL("OP_AT"); break;
        case OP_REQUIRE_BOOL: DBCOL("OP_REQUIRE_BOOL"); break;
        case OP_REQUIRE_VOID: DBCOL("OP_REQUIRE_VOID"); break;
        case OP_REQUIRE_NUM: DBCOL("OP_REQUIRE_NUM"); break;
        case OP_REQUIRE_STR: DBCOL("OP_REQUIRE_STR"); break;
        case OP_REQUIRE_LIST: DBCOL("OP_REQUIRE_LIST"); break;
        case OP_CONVERT:
            op++;
            DBCOL("OP_CONVERT");
            std::cout << " to " << (int)*op;
        case OP_DECL_LIST: DBCOL("OP_DECL_LIST"); break;
        case OP_PUSH_LIST: DBCOL("OP_PUSH_LIST"); break;
        case OP_POP_LIST: DBCOL("OP_POP_LIST"); break;
        case OP_BACK_LIST: DBCOL("OP_BACK_LIST"); break;
        case OP_FRONT_LIST: DBCOL("OP_FRONT_LIST"); break;
        case OP_INDEX_LIST: DBCOL("OP_INDEX_LIST"); break;
        case OP_SIZEOF: DBCOL("OP_SIZEOF"); break;
        case OP_DECL_LIST_INDEX: DBCOL("OP_DECL_LIST_INDEX"); break;
        case OP_CONVERT_ASCII: DBCOL("OP_CONVERT_ASCII"); break;
        case OP_GETCH: DBCOL("OP_GETCH"); break;
        case OP_GETS: DBCOL("OP_GETS"); break;
        case OP_THROW: DBCOL("OP_THROW"); break;
        case OP_CONSOLE: DBCOL("OP_CONSOLE"); break;
        case OP_LIST_FN: DBCOL("OP_LIST_FN"); break;
        default:
            COLOR("bug in opcode, could not identify command: " + (int)*op, DISPLAY_RED);
            break;
    }
    #undef DBCOL
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
    std::stack<Value> copy = value_stack;
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