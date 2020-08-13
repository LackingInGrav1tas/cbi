#include "vm.hpp"
#include "types.hpp"
#include "lexer.hpp"
#include "compiler.hpp"

#include <iostream>
#include <vector>

int main(int argc, char **argv) {
    if (argc == 2) {
        bool b = true;
        auto a = lex(getLines(argv[1]), argv[1], b, NORM);
        if (!b) {
            std::cerr << "\nFatal error during scanning." << std::endl;
            return EXIT_FAILURE;
        }

        Machine vm = compile(a, b);
        if (!b) {
            std::cerr << "\nFatal error during compile time." << std::endl;
            return EXIT_FAILURE;
        }

        vm.disassembleOpcode();
        std::cout << std::endl;

        switch (vm.run(NORMAL)) {
            case EXIT_OK: std::cout << "\nEXIT_OK"; break;
            case EXIT_RT: std::cout << "\nEXIT_RT"; break;
            case EXIT_CT: std::cout << "\nEXIT_CT"; break;
            default: std::cout << "\nbug: unknown error code."; break;
        }

        std::cout << "\n" << std::endl;
        vm.disassembleConstants();

        std::cout << std::endl;
        vm.disassembleStack();
        
        std::cout << std::endl;
        vm.disassembleGlobalMap();

        std::cout << "\nEND OF PROGRAM";
    }
}