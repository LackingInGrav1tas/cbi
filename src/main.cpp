#include "vm.hpp"
#include "types.hpp"
#include "lexer.hpp"

#include <iostream>
#include <vector>

int main() {
    /*bool b = true;
    auto a = lex(getLines("file.txt"), "file.txt", b);

    for (auto it = a.begin(); it < a.end(); it++) std::cout << (*it).lexeme << " -> " << (*it).type << " | ";
    a.back().error("error!");
    std::cout << "\nEND OF PROGRAM";
    /**/
    Machine vm;
    
    vm.writeConstant(0, numberValue(4));
    vm.writeConstant(0, stringValue("abc"));
    vm.writeOp(0, OP_LESS);

    vm.writeOp(0, OP_PRINT_TOP);

    vm.disassembleOpcode();
    std::cout << std::endl;

    switch (vm.run(NORMAL)) {
        case EXIT_OK: std::cout << "EXIT_OK"; break;
        case EXIT_RT: std::cout << "EXIT_RT"; break;
        case EXIT_CT: std::cout << "EXIT_CT"; break;
        default: std::cout << "bug: unknown error code."; break;
    }

    std::cout << "\n" << std::endl;
    vm.disassembleConstants();

    std::cout << std::endl;
    vm.disassembleStack();

    std::cout << "\nEND OF PROGRAM";//*/
}