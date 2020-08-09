#ifndef _vm_h
#define _vm_h

#include <vector>
#include <string>
#include <iostream>
#include <stack>

#include "types.hpp"

class Machine {
    public:
    std::vector<uint8_t> opcode;
    std::vector<int> lines;
    std::stack<Value> value_pool;
    std::vector<Value> constants;

    //for ease of access
    void writeOp(int line, uint8_t command) {
        opcode.push_back(command);
        lines.push_back(line);
    }

    void writeConstant(int line, Value value) {
        writeOp(line, OP_CONSTANT);
        constants.push_back(value);
        writeOp(line, constants.size()-1);
    }

    //in runtime
    ErrorCode run(RunType mode);

    //debug functions
    void disassembleConstants();
    void disassembleOpcode();
    void disassembleStack();
};

//for debug, dont want to make a debug.h file
void disassembleOp(std::vector<uint8_t>::iterator &op, std::vector<Value> constants, std::vector<int> lines, int position);

#endif