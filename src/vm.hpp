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
    std::vector<uint8_t> opcode = { OP_BEGIN };
    std::vector<int> lines = { 0 };
    std::stack<Value> value_pool;
    std::vector<Value> constants;
    std::map<std::string, Value> globals;

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
        writeOp(line, OP_JUMP);
        writeOp(line, index);
    }
    // </helper>

    //in runtime
    ErrorCode run(RunType mode);

    // <debug>
    void disassembleConstants();
    void disassembleOpcode();
    void disassembleStack();
    void disassembleGlobalMap();
    // </debug>
};

//for debug, dont want to make a debug.h file
void disassembleOp(std::vector<uint8_t>::iterator &op, std::vector<Value> constants, std::vector<int> lines, int position);

#endif