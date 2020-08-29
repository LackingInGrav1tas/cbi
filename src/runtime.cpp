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

static bool valueToBool(Value value) {
    if (IS_NUM(value)) {
        return (bool) value.storage.number;
    } else if (IS_STRING(value)) {
        if (value.string == R"("")") {
            return false;
        } else {
            return true;
        }
    } else if (IS_BOOL(value)) {
        return value.storage.boolean;
    } else return false;
}

ErrorCode Machine::run() { // executes the program
    #define GET_TOP() \
        if (value_pool.size() < 2) { std::cerr << " Run-time Error: Stack underflow." << std::endl; return EXIT_RT; } \
        Value rhs = value_pool.top(); \
        value_pool.pop(); \
        Value lhs = value_pool.top(); \
        value_pool.pop()


    for (auto op = opcode.begin(); op < opcode.end(); op++) {
        #define OP (*op)
        switch (OP) {
            case OP_PRINT_TOP: { // prints top of stack
                std::cout << getPrintable(value_pool.top());
                value_pool.pop();
                break;
            }
            case OP_CONSTANT: { // adds constant
                op++;
                value_pool.push(constants[(int)OP]);
                break;
            }
            case OP_CONCATENATE: { // pops the top 2 strings off the value stack, then pushes a concatenated string
                GET_TOP();
                value_pool.push(stringValue('"' + getPrintable(lhs) + getPrintable(rhs) + '"'));
                break;
            }
            case OP_ADD: { // pops the top 2 numbers off the value stack, then pushes the sum
                GET_TOP();
                if (IS_NUM(rhs) && IS_NUM(lhs))
                    value_pool.push(numberValue(lhs.storage.number + rhs.storage.number));
                else {
                    std::cerr << "\nRun-time Error: Could not add non-number value in line " << lines[op-opcode.begin()] << "."; // need better
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
                if (IS_BOOL(top)) {
                    value_pool.push(boolValue(!top.storage.boolean));
                } else if (top.type == TYPE_NULL) {
                    value_pool.push(boolValue(true));
                } else if (IS_STRING(top)) {
                    if (top.string == R"("")") {
                        value_pool.push(boolValue(true));
                    } else {
                        value_pool.push(boolValue(false));
                    }
                } else if (IS_NUM(top)) {
                    if (top.storage.number == 0) value_pool.push(boolValue(true));
                    else value_pool.push(boolValue(false));
                } else {
                    std::cerr << "Run-time Error: Cannot negate unreferenced variable." << std::endl;
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
                value_pool.pop();
                break;
            }
            case OP_JUMP_FALSE_IFv: { // goto
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
                value_pool.pop();
                break;
            }
            case OP_JUMP: { // goto
                op++;
                op = opcode.begin() + (int) OP-1;
                break;
            }
            case OP_BREAK: { // breaks out of the nearest thing
                int position = op-opcode.begin();
                for (; OP != OP_JUMP_FALSE; op--)
                    if (op <= opcode.begin()) {
                        std::cerr << "\nRun-time Error: Misplaced break.";
                        return EXIT_RT;
                    }
                op++;
                if ((int) OP-1 < position) { // so while(...); break; doesnt work
                    std::cerr << "\nRun-time Error: Misplaced break.";
                    return EXIT_RT;
                }
                op = opcode.begin() + (int) OP-1;
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
                if (value_pool.size() > 0) value_pool.pop(); // the if statement is for function calls as exp stmts
                break;
            }
            case OP_VARIABLE: {
                Value gl_value = value_pool.top();
                value_pool.pop();
                if (!IS_ID(value_pool.top())) {
                    std::cerr << "Run-time Error: Expected an identifier." << std::endl;
                    return EXIT_RT;
                }
                std::string id = value_pool.top().string;
                value_pool.pop();
                scopes.back().variables[id] = gl_value;
                break;
            }
            case OP_VARIABLE_MUT: {
                Value gl_value = value_pool.top();
                value_pool.pop();
                if (!IS_ID(value_pool.top())) {
                    std::cerr << "Run-time Error:  Expected an identifier." << std::endl;
                    return EXIT_RT;
                }
                std::string id = value_pool.top().string;
                value_pool.pop();
                scopes.back().variables[id] = gl_value;
                scopes.back().mutables.push_back(id);
                break;
            }
            case OP_DECL_FN: { // format: identifier constant, decl, size
                if (!IS_ID(value_pool.top())) {
                    std::cerr << "Run-time Error: Expected an identifier." << std::endl;
                    return EXIT_RT;
                }
                std::string id = value_pool.top().string;
                value_pool.pop();
                op++;
                fn_pool[(int)OP].scopes = scopes;
                fn_scopes.back()[id] = fn_pool[(int)OP];
                break;
            }
            case OP_RETRIEVE: {
                op++;

                std::map<std::string, Value>::iterator found;
                for (int i = scopes.size()-1; i >= 0; i--) {
                    found = scopes[i].variables.find(constants[(int)OP].string);
                    if (found == scopes[i].variables.end()) {
                        if (i == 0) {
                            std::cerr << "Run-time Error: Cannot access variable out of scope, " << constants[(int)OP].string << std::endl;
                            return EXIT_RT;
                        }
                        continue;
                    }
                    value_pool.push(found->second);
                    break;
                }
                break;
            }
            case OP_IMUT: break;
            case OP_SET_VARIABLE: {
                Value replacement = value_pool.top();
                value_pool.pop();
                
                if (!IS_ID(value_pool.top())) {
                    std::cerr << "Run-time Error: Expected an identifier." << std::endl;
                    return EXIT_RT;
                }

                std::map<std::string, Value>::iterator found;
                for (int i = scopes.size()-1; i >= 0; i--) {
                    std::string a = value_pool.top().string;
                    found = scopes[i].variables.find(a);
                    if (found == scopes[i].variables.end()) {
                        if (i == 0) {
                            std::cerr << "Run-time Error: Cannot access variable out of scope, " << value_pool.top().string << std::endl;
                            return EXIT_RT;
                        }
                        continue;
                    }

                    if (std::find(scopes[i].mutables.begin(), scopes[i].mutables.end(), found->first) == scopes[i].mutables.end()) { // if it's immutable
                        std::cerr << "Run-time Error: Cannot mutate immutable value " << found->first << ". Use syntax:\nset mut <name>;";
                        return EXIT_RT;
                    }
                    found->second = replacement;
                    break;
                }
                break;
            }
            case OP_BEGIN_SCOPE: {
                scopes.push_back(Scope());
                fn_scopes.push_back(std::map<std::string, Function>());
                break;
            }
            case OP_END_SCOPE: {
                scopes.pop_back();
                fn_scopes.pop_back();
                break;
            }
            case OP_AND: {
                GET_TOP();
                value_pool.push(boolValue(valueToBool(lhs) && valueToBool(rhs)));
                break;
            }
            case OP_OR: {
                GET_TOP();
                value_pool.push(boolValue(valueToBool(lhs) || valueToBool(rhs)));
                break;
            }
            case OP_CALL: {
                if (!IS_ID(value_pool.top())) {
                    std::cerr << "Run-time Error: Expected an identifier." << std::endl;
                    return EXIT_RT;
                }
                std::string id = value_pool.top().string;
                value_pool.pop();
                std::map<std::string, Function>::iterator found;
                for (int i = fn_scopes.size()-1; i >= 0; i--) {
                    found = fn_scopes[i].find(id);
                    if (found == fn_scopes[i].end()) {
                        if (i == 0) {
                            std::cerr << "Run-time Error: Cannot call function out of scope, " << id << std::endl;
                            return EXIT_RT;
                        }
                        continue;
                    }
                    Function fn = found->second;
                    Machine call;
                    call.opcode = fn.opcode;
                    call.lines = fn.lines;
                    call.constants = fn.constants;
                    call.scopes = scopes;
                    call.fn_pool = fn_pool; // the best i can do atm
                    call.fn_scopes = std::vector<std::map<std::string, Function>>{fn_scopes}; // ^
                    if (value_pool.size() < fn.param_ids.size()) {
                        std::cerr << "Run-time Error: Expected more parameters during call of function " << id << ". got: " << value_pool.size() << ", rec: " << fn.param_ids.size() << std::endl;
                        return EXIT_RT;
                    }
                    for (int p = fn.param_ids.size()-1; p >= 0; p--) {
                        call.scopes.back().variables[fn.param_ids[p]] = value_pool.top();
                        value_pool.pop();
                        call.scopes.back().mutables.push_back(fn.param_ids[p]);
                    }
                    if (call.run() == EXIT_RT) return EXIT_RT;
                    scopes = call.scopes;
                    break;
                }
                break;
            }
            case OP_EMPTY_STACK: {
                value_pool.empty();
                break;
            }
            default: { // error
                std::cerr << "Run-time Error: Could not identify opcode in line " << lines[op-opcode.begin()] << ", " << (int)OP << "." << std::endl;
                return EXIT_RT;
            }
        }
    }
    #undef OP
    #undef GET_TOP
    return EXIT_OK;
}