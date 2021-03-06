#ifndef _vm_h
#define _vm_h

#include <vector>
#include <string>
#include <iostream>
#include <stack>
#include <map>

#include "types.hpp"

class Machine {
    public:
    std::vector<uint8_t> opcode;
    std::vector<Value> constants;
    std::vector<int> lines;

    std::stack<Value> value_stack;
    std::vector<Scope> scopes;

    // seperated functions from variables because its much easier to not have functions be included as values
    std::vector<std::map<std::string, Function>> fn_scopes;
    std::vector<Function> fn_pool;

    std::vector<std::string> namespaces;
    std::vector<std::vector<std::string>> using_namespaces;

    // <helper>
    void writeOp(int line, uint8_t command) {
        opcode.push_back(command);
        lines.push_back(line);
    }
    void writeConstant(int line, Value value) {
        writeOp(line, OP_CONSTANT);
        constants.push_back(value);
        writeOp(line, constants.size()-1);
    }
    void writeJump(int line, int index) {
        writeOp(line, OP_JUMP_FALSE);
        writeOp(line, index);
    }
    // </helper>

    //in runtime
    Value run();

    Machine from(Function fn) {
        Machine call;
        call.opcode = fn.opcode;
        call.lines = fn.lines;
        call.constants = fn.constants;
        switch (fn.type) {
            case FN_AWARE:
                call.scopes = scopes;
                call.fn_pool = fn_pool;
                call.fn_scopes = fn_scopes;
                break;
            case FN_NORMAL:
                call.scopes = std::vector<Scope>{scopes[0], scopes[1]};
                call.fn_pool = fn_pool;
                call.fn_scopes = std::vector<std::map<std::string, Function>>{fn_scopes[0], fn_scopes[1]};
                break;
            case FN_BLIND:
                call.scopes = std::vector<Scope>{scopes[0]};
                call.fn_pool = fn_pool;
                call.fn_scopes = std::vector<std::map<std::string, Function>>{fn_scopes[0]};
                break;
            default: break;
        }
        return call;
    }

    // <debug>
    void disassembleConstants();
    void disassembleOpcode();
    void disassembleStack();
    void disassembleScopes();
    // </debug>
};

//for debug, dont want to make a debug.h file
void disassembleOp(std::vector<uint8_t>::iterator &op, std::vector<Value> constants, std::vector<int> lines, int position);

#endif