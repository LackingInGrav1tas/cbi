#include <vector>
#include <string>
#include <iostream>
#include <stack>
#include <cstring>
#include <algorithm>

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
        if (value_pool.size() < 2) { std::cerr << "Run-time Error: Stack underflow." << std::endl; return EXIT_RT; } \
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
                    value_pool.push(stringValue('"' + TRIM(lhs.string) + TRIM(rhs.string) + '"'));
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
            case OP_JUMP_FALSE: { // goto
                if (IS_BOOL(value_pool.top())) {
                    if (value_pool.top().storage.boolean) {
                        value_pool.pop();
                        op++;
                        break;
                    }
                } else if (value_pool.top().type != TYPE_NULL) {
                    value_pool.pop();
                    op++;
                    break;
                }
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
            case OP_POP_TOP: {
                value_pool.pop();
                break;
            }
            case OP_GLOBAL: {
                Value gl_value = value_pool.top();
                value_pool.pop();
                if (!IS_ID(value_pool.top())) {
                    std::cerr << "Run-time Error:  Expected an identifier." << std::endl;
                    return EXIT_RT;
                }
                std::string id = value_pool.top().string;
                value_pool.pop();
                if (!(*(op-1) == OP_IMUT && *(op-2) != OP_CONSTANT && *(op-2) != OP_RETRIEVE && *(op-2) != OP_JUMP_FALSE)) {
                    mutables.push_back(id);
                }
                globals[id] = gl_value;
                break;
            }
            case OP_RETRIEVE: {
                op++;
                value_pool.push(globals[constants[(int)OP].string]);
                break;
            }
            case OP_IMUT: break;
            case OP_SET_GLOBAL: {
                Value replacement = value_pool.top();
                value_pool.pop();
                
                if (!IS_ID(value_pool.top())) {
                    std::cerr << "Run-time Error:  Expected an identifier." << std::endl;
                    return EXIT_RT;
                }

                std::map<std::string, Value>::iterator found = globals.find(value_pool.top().string);

                if (found == globals.end()) { // checking if the variable exists
                    std::cerr << "Run-time Error: Cannot access variable out of scope, " << value_pool.top().string << std::endl;
                    return EXIT_RT;
                }

                if (std::find(mutables.begin(), mutables.end(), found->first) == mutables.end()) { // if it's immutable
                    std::cerr << "Run-time Error: Cannot mutate immutable value " << found->first << ". Use syntax:\nset mut <name>;";
                    return EXIT_RT;
                }

                found->second = replacement; // setting new value
                break;
            }
            default: { // error
                std::cerr << "Run-time Error: Could not identify opcode in line " << lines[op-opcode.begin()] << "." << std::endl;
                return EXIT_RT;
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