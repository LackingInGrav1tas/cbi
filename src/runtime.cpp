#include <vector>
#include <string>
#include <iostream>
#include <stack>
#include <cstring>
#include <algorithm>
#include <conio.h>
#include <time.h>
#include <math.h>
#include <fstream>

#include "color.hpp"
#include "vm.hpp"
#include "types.hpp"
#include "token.hpp"

static bool valueEquals(Value lhs, Value rhs) {
    if (lhs.type != rhs.type) return false;
    if (IS_NUM(lhs)) return lhs.storage.number == rhs.storage.number;
    if (IS_STRING(lhs)) return lhs.string == rhs.string;
    if (IS_BOOL(lhs)) return lhs.storage.boolean == rhs.storage.boolean;
    return true; // this would be if both are null
}

static bool checkTypes(std::string type, Tag tag) {
    if (type == "ANY") return true;
    if (tag == TYPE_DOUBLE) {
        if (type == "NUM") return true;
        return false;
    } else if (tag == TYPE_BOOL) {
        if (type == "BOOL") return true;
        return false;
    } else if (tag == TYPE_STRING) {
        if (type == "STR") return true;
        return false;
    } else if (tag == TYPE_NULL) {
        if (type == "VOID") return true;
        return false;
    } else if (tag == TYPE_LIST) {
        if (type == "LIST") return true;
        return false;
    } else return true;
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

#define ADD_NAMESPACES(id) \
    do { for (int i = namespaces.size()-1; i >= 0; i--) { \
        id = namespaces[i] + "::" + id; \
    } } while (0)

Value Machine::run() { // executes the program
    #define ERROR(message) \
        do { \
            COLOR("Run-time Error", DISPLAY_RED); \
            std::cerr << " in line " << lines[op-opcode.begin()]-9 << ": " << message; \
            std::cerr.flush(); \
            while (!scopes.empty()) { \
                for (auto it = scopes.back().variables.begin(); it != scopes.back().variables.end(); it++) \
                    delete_list(it->second.list); \
                scopes.pop_back(); \
            } \
            return exitRT(); \
        } while (false)

    #define GET_TOP() \
        if (value_stack.size() < 2) ERROR("Stack underflow."); \
        Value rhs = value_stack.top(); \
        value_stack.pop(); \
        Value lhs = value_stack.top(); \
        value_stack.pop()

    #define TOP() \
        if (value_stack.size() < 1) ERROR("Stack underflow."); \
        Value top = value_stack.top(); \
        value_stack.pop()

    for (auto op = opcode.begin(); op < opcode.end(); op++) {
        #define OP (*op)
        switch (OP) {
            case OP_PRINT_TOP: { // prints top of stack
                std::cout << getPrintable(value_stack.top());
                value_stack.pop();
                break;
            }
            case OP_CONSTANT: { // adds constant
                op++;
                value_stack.push(constants[(int)OP]);
                break;
            }
            case OP_CONCATENATE: { // pops the top 2 strings off the value stack, then pushes a concatenated string
                GET_TOP();
                value_stack.push(stringValue('"' + getPrintable(lhs) + getPrintable(rhs) + '"'));
                break;
            }
            case OP_ADD: { // pops the top 2 numbers off the value stack, then pushes the sum
                GET_TOP();
                if (IS_NUM(rhs) && IS_NUM(lhs))
                    value_stack.push(numberValue(lhs.storage.number + rhs.storage.number));
                else {
                    ERROR("Could not add non-number value.");
                }
                break;
            }
            case OP_SUB: { // pops the top 2 numbers off the value stack, then pushes the difference
                GET_TOP();
                if (IS_NUM(rhs) && IS_NUM(lhs))
                    value_stack.push(numberValue(lhs.storage.number - rhs.storage.number));
                else {
                    ERROR("Could not subtract non-number value.");
                }
                break;
            }
            case OP_MUL: { // pops the top 2 numbers off the value stack, then pushes the product
                GET_TOP();
                if (IS_NUM(rhs) && IS_NUM(lhs))
                    value_stack.push(numberValue(lhs.storage.number * rhs.storage.number));
                else {
                    ERROR("Could not multiply non-number value.");
                }
                break;
            }
            case OP_DIV: { // pops the top 2 numbers off the value stack, then pushes the quotient
                GET_TOP();
                if (IS_NUM(rhs) && IS_NUM(lhs))
                    value_stack.push(numberValue(lhs.storage.number / rhs.storage.number));
                else {
                    ERROR("Run-time Error: Could not divide non-number value.");
                }
                break;
            }
            case OP_MODULO: { // pops the top 2 numbers off the value stack, then pushes the quotient
                GET_TOP();
                if (IS_NUM(rhs) && IS_NUM(lhs))
                    value_stack.push(numberValue((int)lhs.storage.number % (int)rhs.storage.number));
                else {
                    ERROR("Run-time Error: Could not mod non-number value.");
                }
                break;
            }
            case OP_NEGATE: { // -top
                TOP();
                if (IS_NUM(top))
                    value_stack.push(numberValue(-top.storage.number));
                else {
                    ERROR("Could not negate non-number value.");
                }
                break;
            }
            case OP_NOT: { // !top
                TOP();
                if (IS_BOOL(top)) {
                    value_stack.push(boolValue(!top.storage.boolean));
                } else if (IS_NULL(top)) {
                    value_stack.push(boolValue(true));
                } else if (IS_STRING(top)) {
                    if (top.string == R"("")") {
                        value_stack.push(boolValue(true));
                    } else {
                        value_stack.push(boolValue(false));
                    }
                } else if (IS_NUM(top)) {
                    if (top.storage.number == 0) value_stack.push(boolValue(true));
                    else value_stack.push(boolValue(false));
                } else {
                    ERROR("Cannot negate unreferenced variable.");
                }
                break;
            }
            case OP_JUMP_FALSE:
            case OP_JUMP_FALSE_IFv: { // goto
                if (IS_BOOL(value_stack.top())) {
                    if (value_stack.top().storage.boolean) {
                        value_stack.pop();
                        op++;
                        break;
                    }
                } else if (IS_NUM(value_stack.top())) {
                    if (value_stack.top().storage.number) {
                        value_stack.pop();
                        op++;
                        break;
                    }
                } else ERROR("Expected a boolean expression. You can use 'as BOOL' to convert a value.");
                op++;
                op = opcode.begin() + (int) OP-1;
                value_stack.pop();
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
                        ERROR("Misplaced break.");
                    }
                op++;
                if ((int) OP-1 < position) { // so while(...); break; doesnt work
                    ERROR("Misplaced break.");
                }
                op = opcode.begin() + (int) OP-1;
            }
            case OP_BEGIN: break; // because of jump quirk
            case OP_EQUALITY: { // ==
                GET_TOP();
                value_stack.push(boolValue(valueEquals(lhs, rhs)));
                break;
            }
            case OP_NOT_EQ: { // !=
                GET_TOP();
                value_stack.push(boolValue(!valueEquals(lhs, rhs)));
                break;
            }
            case OP_LESS: { // <
                GET_TOP();
                if (IS_NUM(rhs) && IS_NUM(lhs))
                    value_stack.push(boolValue(lhs.storage.number < rhs.storage.number));
                else {
                    ERROR("Could not solve with non-number value.");
                }
                break;
            }
            case OP_GREATER: { // >
                GET_TOP();
                if (IS_NUM(rhs) && IS_NUM(lhs))
                    value_stack.push(boolValue(lhs.storage.number > rhs.storage.number));
                else {
                    ERROR("Could not solve with non-number value.");
                }
                break;
            }
            case OP_LESS_EQ: { // <=
                GET_TOP();
                if (IS_NUM(rhs) && IS_NUM(lhs))
                    value_stack.push(boolValue(lhs.storage.number <= rhs.storage.number));
                else {
                    ERROR("Could not solve with non-number value.");
                }
                break;
            }
            case OP_GREATER_EQ: { // >=
                GET_TOP();
                if (IS_NUM(rhs) && IS_NUM(lhs))
                    value_stack.push(boolValue(lhs.storage.number >= rhs.storage.number));
                else {
                    ERROR("Could not solve with non-number value.");
                }
                break;
            }
            case OP_POP_TOP: {
                if (value_stack.size() > 0) value_stack.pop(); // the if statement is for function calls as exp stmts
                break;
            }
            case OP_VARIABLE: {
                Value gl_value = value_stack.top();
                value_stack.pop();
                if (!IS_ID(value_stack.top())) {
                    ERROR("Expected an identifier.");
                }
                std::string id = value_stack.top().string;
                ADD_NAMESPACES(id);

                value_stack.pop();
                scopes.back().variables[id] = gl_value;
                break;
            }
            case OP_VARIABLE_MUT: {
                TOP();
                if (!IS_ID(value_stack.top())) {
                    ERROR("Expected an identifier.");
                }
                std::string id = value_stack.top().string;
                ADD_NAMESPACES(id);

                value_stack.pop();
                scopes.back().variables[id] = top;
                scopes.back().mutables.push_back(id);
                break;
            }
            case OP_DECL_FN: { // format: identifier constant, decl, size
                if (!IS_ID(value_stack.top())) {
                    ERROR("Expected an identifier.");
                }
                std::string id = value_stack.top().string;
                ADD_NAMESPACES(id);

                value_stack.pop();
                op++;
                fn_pool[(int)OP].scopes = scopes;
                fn_scopes.back()[id] = fn_pool[(int)OP];
                break;
            }
            case OP_RETRIEVE: {
                op++;
                bool b = false;
                std::map<std::string, Value>::iterator found;
                for (int o = scopes.size()-1; o >= 0; o--) {
                    found = scopes[o].variables.find(constants[(int)OP].string);
                    if (found == scopes[o].variables.end())
                        continue;
                    value_stack.push(found->second);
                    b = true;
                    break;
                }
                if (!b) {
                    bool in_scope = false;
                    for (int i = using_namespaces.size()-1; i >= 0; i--) {
                        for (int ns = 0; ns < using_namespaces[i].size(); ns++) {
                            for (int o = scopes.size()-1; o >= 0; o--) {
                                found = scopes[o].variables.find(using_namespaces[i][ns] + "::" + constants[(int)OP].string);
                                if (found == scopes[o].variables.end())
                                    continue;
                                value_stack.push(found->second);
                                in_scope = true;
                                break;
                            }
                        }
                    }
                    if (!in_scope) ERROR("Cannot access variable out of scope, " << constants[(int)OP].string << ".");
                }
                break;
            }
            case OP_IMUT: break;
            case OP_SET_VARIABLE: {
                TOP();
                
                if (!IS_ID(value_stack.top())) {
                    ERROR("Expected an identifier.");
                }

                std::map<std::string, Value>::iterator found;
                for (int i = scopes.size()-1; i >= 0; i--) {
                    std::string a = value_stack.top().string;
                    found = scopes[i].variables.find(a);
                    if (found == scopes[i].variables.end()) {
                        if (i == 0) {
                            ERROR("Cannot access variable out of scope, " << value_stack.top().string << ".");
                        }
                        continue;
                    }

                    if (std::find(scopes[i].mutables.begin(), scopes[i].mutables.end(), found->first) == scopes[i].mutables.end()) { // if it's immutable
                        ERROR("Cannot mutate immutable value " << found->first << ". Use syntax: set mut <name>;");
                    }
                    found->second = top;
                    break;
                }
                break;
            }
            case OP_BEGIN_SCOPE: {
                scopes.push_back(Scope());
                fn_scopes.push_back(std::map<std::string, Function>());
                using_namespaces.push_back(std::vector<std::string>());
                break;
            }
            case OP_END_SCOPE: {
                for (auto it = scopes.back().variables.begin(); it != scopes.back().variables.end(); it++)
                    delete_list(it->second.list);
                scopes.pop_back();
                fn_scopes.pop_back();
                using_namespaces.pop_back();
                break;
            }
            case OP_AND: {
                GET_TOP();
                value_stack.push(boolValue(valueToBool(lhs) && valueToBool(rhs)));
                break;
            }
            case OP_OR: {
                GET_TOP();
                value_stack.push(boolValue(valueToBool(lhs) || valueToBool(rhs)));
                break;
            }
            case OP_CALL: {
                if (!IS_ID(value_stack.top())) {
                    ERROR("Expected an identifier.");
                }
                std::string id = value_stack.top().string;
                value_stack.pop();
                std::map<std::string, Function>::iterator found;
                Function fn;
                bool fnd = false;
                for (int i = fn_scopes.size()-1; i >= 0; i--) {
                    found = fn_scopes[i].find(id);
                    if (found == fn_scopes[i].end()) {
                        continue;
                    }
                    fn = found->second;
                    fnd = true;
                    break;
                }
                if (!fnd) {
                    bool in_scope = false;
                    for (int i = using_namespaces.size()-1; i >= 0; i--) {
                        for (int ns = 0; ns < using_namespaces[i].size(); ns++) {
                            for (int o = fn_scopes.size()-1; o >= 0; o--) {
                                found = fn_scopes[o].find(using_namespaces[i][ns] + "::" + id);
                                if (found == fn_scopes[o].end()) {
                                    continue;
                                }
                                fn = found->second;
                                in_scope = true;
                                break;
                            }
                        }
                    }
                    if (!in_scope) ERROR("Cannot access variable out of scope, " << id << ".");
                }
                Machine call = Machine::from(fn); // setting opcode/constant pool/etc.
                if (value_stack.size() < fn.param_ids.size()) { // checking params
                    ERROR("Expected more parameters during call of function " << id << ". Received: " << value_stack.size() << ", Expected: " << fn.param_ids.size());
                }
                for (int p = fn.param_ids.size()-1; p >= 0; p--) { // setting params
                    call.scopes.back().variables[fn.param_ids[p]] = value_stack.top();
                    if (!checkTypes(fn.param_types[p], value_stack.top().type)) ERROR("Param specifiers do not match. Expected " << fn.param_types[p] << ".");
                    value_stack.pop();
                    call.scopes.back().mutables.push_back(fn.param_ids[p]);
                }
                Value call_run = call.run();
                if (call_run.type == TYPE_RT_ERROR) return exitRT();
                else if (call_run.type != TYPE_OK) value_stack.push(call_run);
                else value_stack.push(nullValue());
                switch (fn.type) {
                    case FN_AWARE:
                        scopes = call.scopes;
                        break;
                    case FN_NORMAL:
                        scopes[0] = call.scopes[0];
                        scopes[1] = call.scopes[1];
                        break;
                }
                break;
            }
            case OP_EMPTY_STACK: {
                value_stack.empty();
                break;
            }
            case OP_RETURN_TOP: {
                TOP();
                return top;
            }
            case OP_THROW: {
                TOP();
                ERROR(getPrintable(top));
            }
            case OP_GETS: {
                TOP();
                if (!IS_ID(top)) ERROR("Expected an identifier.");
                std::map<std::string, Value>::iterator found;
                for (int i = scopes.size()-1; i >= 0; i--) {
                    std::string a = top.string;
                    found = scopes[i].variables.find(a);
                    if (found == scopes[i].variables.end()) {
                        if (i == 0) {
                            ERROR("Cannot access variable out of scope, " << top.string << ".");
                        }
                        continue;
                    }

                    if (std::find(scopes[i].mutables.begin(), scopes[i].mutables.end(), found->first) == scopes[i].mutables.end()) { // if it's immutable
                        ERROR("Cannot mutate immutable value " << found->first << ". Use syntax: set mut <name>;");
                    }
                    std::string input;
                    std::getline(std::cin, input);
                    found->second = stringValue(std::string("\"") + input + "\"");
                    break;
                }
                break;
            }
            case OP_GETCH: {
                TOP();
                if (!IS_ID(top)) ERROR("Expected an identifier.");
                std::map<std::string, Value>::iterator found;
                for (int i = scopes.size()-1; i >= 0; i--) {
                    std::string a = top.string;
                    found = scopes[i].variables.find(a);
                    if (found == scopes[i].variables.end()) {
                        if (i == 0) {
                            ERROR("Cannot access variable out of scope, " << top.string << ".");
                        }
                        continue;
                    }

                    if (std::find(scopes[i].mutables.begin(), scopes[i].mutables.end(), found->first) == scopes[i].mutables.end()) { // if it's immutable
                        ERROR("Cannot mutate immutable value " << found->first << ". Use syntax: set mut <name>;");
                    }
                    found->second = stringValue(std::string("\"") + (char)getch() + "\"");
                    break;
                }
                break;
            }
            case OP_CONVERT: {
                TOP();
                op++;
                switch ((int)OP) { // 0 -> num, 1 -> string, 2 -> bool, 3 -> void, 4 -> list
                    case 0: { // x to num
                        if (top.type == TYPE_STRING) {
                            try {
                                value_stack.push(numberValue(std::stod(TRIM(top.string))));
                            } catch (...) {
                                ERROR("Cannot convert this string value to number.");
                            }
                        } else if (top.type == TYPE_DOUBLE) value_stack.push(top);
                        else if (top.type == TYPE_BOOL) value_stack.push(numberValue(top.storage.boolean));
                        else if (top.type == TYPE_LIST) value_stack.push(numberValue(top.list.size()));
                        else value_stack.push(numberValue(0));
                        break;
                    }
                    case 1: { // x to string
                        if (top.type == TYPE_DOUBLE) value_stack.push(stringValue("\"" + shorten(std::to_string(top.storage.number)) + "\""));
                        else if (top.type == TYPE_STRING) value_stack.push(top);
                        else if (top.type == TYPE_BOOL) {
                            if (top.storage.boolean) value_stack.push(stringValue("\"true\""));
                            else value_stack.push(stringValue("\"false\""));
                        }
                        else if (top.type == TYPE_LIST) value_stack.push(stringValue("\"" + getPrintable(top) + "\""));
                        else value_stack.push(stringValue("\"\""));
                        break;
                    }
                    case 2: { // x to bool
                        if (top.type == TYPE_DOUBLE) {
                            if (top.storage.number == 0) value_stack.push(boolValue(false));
                            else value_stack.push(boolValue(true));
                        }
                        else if (top.type == TYPE_BOOL) value_stack.push(top);
                        else if (top.type == TYPE_STRING) {
                            if (TRIM(top.string) == "" || TRIM(top.string) == "false" || TRIM(top.string) == "0")
                                value_stack.push(boolValue(false));
                            else value_stack.push(boolValue(true));
                        }
                        else if (IS_LIST(top))
                            value_stack.push(boolValue(top.list.size()));
                        else value_stack.push(boolValue(false));
                        break;
                    }
                    case 3: { // x to null
                        value_stack.push(nullValue());
                        break;
                    }
                    case 4: { // x to list
                        if (IS_LIST(top)) {
                            value_stack.push(top);
                        } else {
                            value_stack.push(listValue(top));
                        }
                        break;
                    }
                    default: ERROR("Expected a type specifier.");
                }
                break;
            }
            case OP_REQUIRE_BOOL: {
                if (value_stack.top().type != TYPE_BOOL) ERROR("Expected a boolean value.");
                break;
            }
            case OP_REQUIRE_NUM: {
                if (value_stack.top().type != TYPE_DOUBLE) ERROR("Expected a num value.");
                break;
            }
            case OP_REQUIRE_STR: {
                if (value_stack.top().type != TYPE_STRING) ERROR("Expected a string value.");
                break;
            }
            case OP_REQUIRE_VOID: {
                if (value_stack.top().type != TYPE_NULL) ERROR("Expected a void value.");
                break;
            }
            case OP_REQUIRE_LIST: {
                if (value_stack.top().type != TYPE_LIST) ERROR("Expected a list value.");
                break;
            }
            case OP_AT: {
                GET_TOP(); // rhs = position, lhs = subject
                if (!IS_NUM(rhs)) ERROR("Expected a number.");
                if (!IS_STRING(lhs)) ERROR("Expected a string value.");
                if (TRIM(lhs.string).length() <= rhs.storage.number) ERROR("Index out of range.");
                value_stack.push(stringValue(std::string("\"") + TRIM(lhs.string).at(rhs.storage.number) + "\""));
                break;
            }
            case OP_PUSH_LIST: {
                Value *rhs = new Value;
                *rhs = value_stack.top();
                value_stack.pop();
                Value lhs = value_stack.top();
                value_stack.pop();
                // lhs = id, rhs = value
                if (!IS_ID(lhs)) ERROR("Expected an identifier.");
                std::map<std::string, Value>::iterator found;
                for (int i = scopes.size()-1; i >= 0; i--) {
                    found = scopes[i].variables.find(lhs.string);
                    if (found == scopes[i].variables.end()) {
                        if (i == 0) {
                            ERROR("Cannot access variable out of scope, " << lhs.string << ".");
                        }
                        continue;
                    }

                    if (std::find(scopes[i].mutables.begin(), scopes[i].mutables.end(), found->first) == scopes[i].mutables.end()) { // if it's immutable
                        ERROR("Cannot mutate immutable value " << found->first << ". Use syntax: set mut <name>;");
                    }
                    if (found->second.type != TYPE_LIST) ERROR("Expected a list variable.");
                    found->second.list.push_back(rhs);
                    break;
                }
                break;
            }
            case OP_POP_LIST: {
                TOP();
                if (!IS_ID(top)) ERROR("Expected an identifier.");
                std::map<std::string, Value>::iterator found;
                for (int i = scopes.size()-1; i >= 0; i--) {
                    found = scopes[i].variables.find(top.string);
                    if (found == scopes[i].variables.end()) {
                        if (i == 0) {
                            ERROR("Cannot access variable out of scope, " << top.string << ".");
                        }
                        continue;
                    }

                    if (std::find(scopes[i].mutables.begin(), scopes[i].mutables.end(), found->first) == scopes[i].mutables.end()) { // if it's immutable
                        ERROR("Cannot mutate immutable value " << found->first << ". Use syntax: set mut <name>;");
                    }
                    if (found->second.type != TYPE_LIST) ERROR("Expected a list variable.");
                    found->second.list.pop_back();
                    break;
                }
                break;
            }
            case OP_BACK_LIST: {
                TOP();
                if (top.type != TYPE_LIST) ERROR("Expected a list variable.");
                if (top.list.size() == 0)
                    value_stack.push(nullValue());
                else value_stack.push(*top.list.back());
                break;
            }
            case OP_FRONT_LIST: {
                TOP();
                if (top.type != TYPE_LIST) ERROR("Expected a list variable.");
                if (top.list.size() == 0)
                    value_stack.push(nullValue());
                else value_stack.push(*top.list.front());
                break;
            }
            case OP_INDEX_LIST: {
                GET_TOP();
                if (!IS_LIST(lhs)) ERROR("Expected a list.");
                if (!IS_NUM(rhs)) ERROR("Expected a number, rhs = " << rhs.type );
                if (rhs.storage.number < 0 || rhs.storage.number >= lhs.list.size())
                    ERROR("Index out of range.");
                value_stack.push(*lhs.list[rhs.storage.number]);
                break;
            }
            case OP_SIZEOF: {
                TOP();
                if (IS_LIST(top)) { // it should be a list
                    value_stack.push(numberValue(top.list.size()));
                } else if (IS_STRING(top)) { // string
                    value_stack.push(numberValue(top.string.size()-2));
                } else ERROR("Expected either a string value or list.");
                break;
            }
            case OP_DECL_LIST_INDEX: {
                Value listname = value_stack.top();
                value_stack.pop();
                Value *toassign = new Value;
                *toassign = value_stack.top();
                value_stack.pop();
                Value index = value_stack.top();
                value_stack.pop();
                if (!IS_NUM(index)) ERROR("Expected a number.");
                if (!IS_ID(listname)) ERROR("Expected an identifier.");
                std::map<std::string, Value>::iterator found;
                for (int i = scopes.size()-1; i >= 0; i--) {
                    found = scopes[i].variables.find(listname.string);
                    if (found == scopes[i].variables.end()) {
                        if (i == 0) {
                            ERROR("Cannot access variable out of scope, " << listname.string << ".");
                        }
                        continue;
                    }

                    if (std::find(scopes[i].mutables.begin(), scopes[i].mutables.end(), found->first) == scopes[i].mutables.end()) { // if it's immutable
                        ERROR("Cannot mutate immutable value " << found->first << ". Use syntax: set mut <name>;");
                    }
                    if (!IS_LIST(found->second)) ERROR("Expected a list variable.");
                    if (index.storage.number < 0 || index.storage.number >= found->second.list.size()) {
                        ERROR("Index out of range.");
                    }
                    found->second.list[index.storage.number] = toassign;
                    break;
                }
                break;
            }
            case OP_CONVERT_ASCII: {
                TOP();
                if (IS_NUM(top)) value_stack.push(stringValue(std::string("\"") + (char)(int)top.storage.number + "\""));
                else if (IS_STRING(top)) {
                    if (top.string.length() != 3) ERROR("Expected a string with sizeof 1, " << top.string.length() << "  " << top.string);
                    value_stack.push(numberValue(top.string.at(1)));
                }
                else ERROR("Expected a string or number.");
                break;
            }
            case OP_CONSOLE: {
                TOP();
                if (!IS_STRING(top)) ERROR("Expected a string");
                system(TRIM(top.string).c_str());
                break;
            }
            case OP_SLEEP: {
                TOP();
                if (!IS_NUM(top)) ERROR("Expected a number.");
                Sleep(top.storage.number);
                break;
            }
            case OP_RAND: {
                TOP();
                if (!IS_NUM(top)) ERROR("Expected a number.");
                srand(time(NULL));
                value_stack.push(numberValue(rand() % (int)top.storage.number));
                break;
            }
            case OP_FLOOR: {
                TOP();
                if (!IS_NUM(top)) ERROR("Expected a number.");
                value_stack.push(numberValue(floor(top.storage.number)));
                break;
            }
            case OP_LIST_FN: {
                TOP();
                if (!IS_NUM(top)) ERROR("Expected a number.");

                Value list = listValue();

                if (top.storage.number > value_stack.size()) ERROR("Stack underflow.");
                for (int i = 0; i < top.storage.number; i++) {
                    Value *item = new Value;
                    *item = value_stack.top();
                    value_stack.pop();
                    list.list.insert(list.list.begin(), item);
                }

                value_stack.push(list);
                break;
            }
            case OP_WRITE_FILE: {
                std::cout << "WF B" << std::endl;
                // stack: BOT ..., a/w, text, file TOP
                if (value_stack.size() < 3) ERROR("Stack underflow.");
                Value file = value_stack.top();
                value_stack.pop();

                if (!IS_STRING(file)) ERROR("Expected a string path.");
                Value text = value_stack.top();
                value_stack.pop();
                if (!IS_STRING(text)) ERROR("Expected text. You can use 'as STR' to convert.");

                Value mode = value_stack.top();
                value_stack.pop();
                if (!IS_STRING(file)) ERROR("Expected a string mode, either 'a' or 'w'.");

                std::ofstream f;
                if (TRIM(mode.string).at(0) == 'a') f.open(TRIM(file.string), std::ios_base::app);
                else f.open(TRIM(file.string));

                if (!f) value_stack.push(boolValue(false));
                else {
                    f << TRIM(text.string);
                    f.flush();
                    value_stack.push(boolValue(true));
                }
                std::cout << "WF E" << std::endl;

                break;
            }
            case OP_BEGIN_NAMESPACE: {
                TOP();
                namespaces.push_back(top.string);
                break;
            }
            case OP_END_NAMESPACE: {
                namespaces.pop_back();
                break;
            }
            case OP_USE_NAMESPACE: {
                TOP();
                using_namespaces.back().push_back(top.string);
                break;
            }
            case OP_FETCH_CLIP: {
                std::string text;
                if (OpenClipboard(NULL)) {
                    HANDLE clip;
                    clip = GetClipboardData(CF_TEXT);
                    text = (LPSTR)GlobalLock(clip);
                    GlobalUnlock(clip);
                    CloseClipboard();
                }
                value_stack.push(stringValue("\"" + text + "\""));
                break;
            }
            
            case OP_DISASSEMBLE_CONSTANTS: {
                disassembleConstants();
                break;
            }
            case OP_DISASSEMBLE_SCOPES: {
                disassembleScopes();
                break;
            }
            case OP_DISASSEMBLE_STACK: {
                disassembleStack();
                break;
            }

            default: {
                ERROR("Could not identify opcode in line " << lines[op-opcode.begin()] << ", " << (int)OP << ", " << op-opcode.begin() << ".");
            }
        }
    }
    #undef OP
    #undef GET_TOP
    #undef TOP
    #undef ERROR
    return exitOK();
}