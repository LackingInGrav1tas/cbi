#include <vector>
#include <string>
#include <iostream>
#include <stack>
#include <cstring>

#include "vm.hpp"
#include "types.hpp"

static bool valueEquals(Value lhs, Value rhs) {
    if (lhs.type != rhs.type) return false;
    if (IS_NUM(lhs)) return lhs.storage.number == rhs.storage.number;
    if (IS_STRING(lhs)) return lhs.string == rhs.string;
    if (IS_BOOL(lhs)) return lhs.storage.boolean == rhs.storage.boolean;
    return true; // this would be if both are null
}

ErrorCode Machine::run(RunType mode) { // executes the program

    #define GET_TOP() \
        Value rhs = value_pool.top(); \
        value_pool.pop(); \
        Value lhs = value_pool.top(); \
        value_pool.pop()
    if (mode == DEBUG) std::cout << "== runtime debug mode ==";

    for (auto op = opcode.begin(); op < opcode.end(); op++) {
        #define OP (*op)
        switch (OP) {
            case OP_BLANK: { // n/a
                break;
            }
            case OP_PRINT_TOP: { // prints top of stack
                std::cout << getPrintable(value_pool.top());
                break;
            }
            case OP_CONSTANT: { // adds constant
                op++;
                value_pool.push(constants[OP]);
                break;
            }
            case OP_CONCATENATE: { // pops the top 2 strings off the value stack, then pushes a concatenated string
                GET_TOP();
                if (IS_STRING(rhs) && IS_STRING(lhs)) {
                    value_pool.push(stringValue(lhs.string + rhs.string));
                } else {
                    std::cerr << "Run-time Error: Could not concatenate non-string value in line " << lines[op-opcode.begin()] << "."; // need better
                    return EXIT_RT;
                }
                break;
            }
            case OP_ADD: { // pops the top 2 numbers off the value stack, then pushes the sum
                GET_TOP();
                if (IS_NUM(rhs) && IS_NUM(lhs))
                    value_pool.push(numberValue(lhs.storage.number + rhs.storage.number));
                else {
                    std::cerr << "Run-time Error: Could not add non-number value in line " << lines[op-opcode.begin()] << "."; // need better
                    return EXIT_RT;
                }
                break;
            }
            case OP_SUB: { // pops the top 2 numbers off the value stack, then pushes the difference
                GET_TOP();
                if (IS_NUM(rhs) && IS_NUM(lhs))
                    value_pool.push(numberValue(lhs.storage.number - rhs.storage.number));
                else {
                    std::cerr << "Run-time Error: Could not subtract non-number value in line " << lines[op-opcode.begin()] << "."; // need better
                    return EXIT_RT;
                }
                break;
            }
            case OP_MUL: { // pops the top 2 numbers off the value stack, then pushes the product
                GET_TOP();
                if (IS_NUM(rhs) && IS_NUM(lhs))
                    value_pool.push(numberValue(lhs.storage.number * rhs.storage.number));
                else {
                    std::cerr << "Run-time Error: Could not multiply non-number value in line " << lines[op-opcode.begin()] << "."; // need better
                    return EXIT_RT;
                }
                break;
            }
            case OP_DIV: { // pops the top 2 numbers off the value stack, then pushes the quotient
                GET_TOP();
                if (IS_NUM(rhs) && IS_NUM(lhs))
                    value_pool.push(numberValue(lhs.storage.number / rhs.storage.number));
                else {
                    std::cerr << "Run-time Error: Could not divide non-number value in line " << lines[op-opcode.begin()] << "."; // need better
                    return EXIT_RT;
                }
                break;
            }
            case OP_NEGATE: { // -top
                Value num = value_pool.top();
                value_pool.pop();
                if (IS_NUM(num))
                    value_pool.push(numberValue(-num.storage.number));
                else {
                    std::cerr << "Run-time Error: Could not negate non-number value in line " << lines[op-opcode.begin()] << "."; // need better
                    return EXIT_RT;
                }
                break;
            }
            case OP_NOT: { // !top
                Value top = value_pool.top();
                value_pool.pop();
                if (IS_BOOL(top))
                    value_pool.push(boolValue(!top.storage.boolean));
                else {
                    std::cerr << "Run-time Error: Could not negate non-boolean value in line " << lines[op-opcode.begin()] << "."; // need better
                    return EXIT_RT;
                }
                break;
            }
            case OP_JUMP: { // goto
                op++;
                op = opcode.begin() + (int) OP-1;
                break;
            }
            case OP_BEGIN: break; // because of jump quirk
            case OP_EQUALITY: { // ==
                GET_TOP();
                value_pool.push(boolValue(valueEquals(lhs, rhs)));
                break;
            }
            case OP_NOT_EQ: { // !=
                GET_TOP();
                value_pool.push(boolValue(!valueEquals(lhs, rhs)));
                break;
            }
            case OP_LESS: { // <
                GET_TOP();
                if (IS_NUM(rhs) && IS_NUM(lhs))
                    value_pool.push(boolValue(lhs.storage.number < rhs.storage.number));
                else {
                    std::cerr << "Run-time Error: Could not solve with non-number value in line " << lines[op-opcode.begin()] << "."; // need better
                    return EXIT_RT;
                }
                break;
            }
            case OP_GREATER: { // >
                GET_TOP();
                if (IS_NUM(rhs) && IS_NUM(lhs))
                    value_pool.push(boolValue(lhs.storage.number > rhs.storage.number));
                else {
                    std::cerr << "Run-time Error: Could not solve with non-number value in line " << lines[op-opcode.begin()] << "."; // need better
                    return EXIT_RT;
                }
                break;
            }
            case OP_LESS_EQ: { // <=
                GET_TOP();
                if (IS_NUM(rhs) && IS_NUM(lhs))
                    value_pool.push(boolValue(lhs.storage.number <= rhs.storage.number));
                else {
                    std::cerr << "Run-time Error: Could not solve with non-number value in line " << lines[op-opcode.begin()] << "."; // need better
                    return EXIT_RT;
                }
                break;
            }
            case OP_GREATER_EQ: { // >=
                GET_TOP();
                if (IS_NUM(rhs) && IS_NUM(lhs))
                    value_pool.push(boolValue(lhs.storage.number >= rhs.storage.number));
                else {
                    std::cerr << "Run-time Error: Could not solve with non-number value in line " << lines[op-opcode.begin()] << "."; // need better
                    return EXIT_RT;
                }
                break;
            }
            default: { // error
                std::cerr << "Run-time Error: Could not identify opcode in line " << lines[op-opcode.begin()] << "." << std::endl;
                return EXIT_RT;
                break;
            }
        }
        if (mode == DEBUG) { //printing out OP info
            disassembleOp(op, constants, lines, op-opcode.begin());
        }
    }
    #undef OP
    #undef GET_TOP
    if (mode == DEBUG) std::cout << "\n== end ==" << std::endl;
    return EXIT_OK;
}