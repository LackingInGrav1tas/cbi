#include <vector>
#include <string>
#include <iostream>
#include <stack>
#include <cstring>

#include "vm.hpp"
#include "types.hpp"

ErrorCode Machine::run(RunType mode) { // executes the program

    if (mode == DEBUG) std::cout << "== runtime debug mode ==";

    for (auto op = opcode.begin(); op < opcode.end(); op++) {
        switch (*op) {
            case OP_BLANK: { // n/a
                std::cout << "blank!" << std::endl;
                break;
            }
            case OP_PRINT_TOP: { // for debug, print top of stack
                std::cout << getPrintable(value_pool.top()) << std::endl;
                break;
            }
            case OP_CONSTANT: { // adds constant
                op++;
                value_pool.push(constants[*op]);
                break;
            }
            case OP_CONCATENATE: { // pops the top 2 strings off the value stack, then pushes a concatenated string
                Value rhs = value_pool.top();
                value_pool.pop();
                Value lhs = value_pool.top();
                value_pool.pop();
                if (IS_STRING(rhs) && IS_STRING(lhs)) {
                    std::string l = std::string(lhs.storage.string);
                    std::string r = std::string(rhs.storage.string);
                    char *a = (char*)(l+r).c_str();
                    value_pool.push(stringValue(a));
                } else {
                    std::cerr << "Runtime Error: Could not concatenate non-string value in line " << lines[op-opcode.begin()] << "."; // need better
                    return EXIT_RT;
                }
                break;
            }
            case OP_ADD: { // pops the top 2 numbers off the value stack, then pushes the sum
                Value rhs = value_pool.top();
                value_pool.pop();
                Value lhs = value_pool.top();
                value_pool.pop();
                if (IS_NUM(rhs) && IS_NUM(lhs))
                    value_pool.push(numberValue(lhs.storage.number + rhs.storage.number));
                else {
                    std::cerr << "Runtime Error: Could not add non-number value in line " << lines[op-opcode.begin()] << "."; // need better
                    return EXIT_RT;
                }
                break;
            }
            case OP_SUB: { // pops the top 2 numbers off the value stack, then pushes the difference
                Value rhs = value_pool.top();
                value_pool.pop();
                Value lhs = value_pool.top();
                value_pool.pop();
                if (IS_NUM(rhs) && IS_NUM(lhs))
                    value_pool.push(numberValue(lhs.storage.number - rhs.storage.number));
                else {
                    std::cerr << "Runtime Error: Could not subtract non-number value in line " << lines[op-opcode.begin()] << "."; // need better
                    return EXIT_RT;
                }
                break;
            }
            case OP_MUL: { // pops the top 2 numbers off the value stack, then pushes the product
                Value rhs = value_pool.top();
                value_pool.pop();
                Value lhs = value_pool.top();
                value_pool.pop();
                if (IS_NUM(rhs) && IS_NUM(lhs))
                    value_pool.push(numberValue(lhs.storage.number * rhs.storage.number));
                else {
                    std::cerr << "Runtime Error: Could not multiply non-number value in line " << lines[op-opcode.begin()] << "."; // need better
                    return EXIT_RT;
                }
                break;
            }
            case OP_DIV: { // pops the top 2 numbers off the value stack, then pushes the quotient
                Value rhs = value_pool.top();
                value_pool.pop();
                Value lhs = value_pool.top();
                value_pool.pop();
                if (IS_NUM(rhs) && IS_NUM(lhs))
                    value_pool.push(numberValue(lhs.storage.number / rhs.storage.number));
                else {
                    std::cerr << "Runtime Error: Could not divide non-number value in line " << lines[op-opcode.begin()] << "."; // need better
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
                    std::cerr << "Runtime Error: Could not negate non-number value in line " << lines[op-opcode.begin()] << "."; // need better
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
                    std::cerr << "Runtime Error: Could not negate non-boolean value in line " << lines[op-opcode.begin()] << "."; // need better
                    return EXIT_RT;
                }
                break;
            }
            default: { // error
                std::cerr << "Runtime Error: Could not identify opcode in line " << lines[op-opcode.begin()] << "." << std::endl;
                return EXIT_RT;
                break;
            }
        }
        if (mode == DEBUG) { //printing out OP info
            disassembleOp(op, constants, lines, op-opcode.begin());
        }
    }
    if (mode == DEBUG) std::cout << "\n== end ==" << std::endl;
    return EXIT_OK;
}