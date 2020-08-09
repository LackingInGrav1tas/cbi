#include "vm.hpp"
#include "types.hpp"

#include <iostream>
#include <vector>

int main() {
    Machine vm;
    
    vm.writeConstant(0, stringValue("this is string 1.\n"));
    vm.writeConstant(0, stringValue("this is string 2."));
    vm.writeOp(0, OP_CONCATENATE);

    vm.writeOp(1, OP_PRINT_TOP);

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

    std::cout << "\nEND OF PROGRAM";
}