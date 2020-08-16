#include "vm.hpp"
#include "types.hpp"
#include "lexer.hpp"
#include "compiler.hpp"

#include <iostream>
#include <vector>

int main(int argc, char **argv) {
    try {
        if (argc == 2) {
            bool success = true;
            std::vector<std::string> lines = getLines(argv[1], success); // getting the contents of the file
            if (!success) {
                std::cerr << "\nCould not access file." << std::endl;
                return EXIT_FAILURE;
            }

            auto tokens = lex(lines, argv[1], success, NORM); // lexing the file into tokens
            if (!success) {
                std::cerr << "\nFatal error during scanning." << std::endl;
                return EXIT_FAILURE;
            }

            Machine vm = compile(tokens, success); // compiling the tokens to bytecode
            if (!success) {
                std::cerr << "\nFatal error during compile time." << std::endl;
                return EXIT_FAILURE;
            }

            vm.disassembleOpcode();
            std::cout << std::endl;

            switch (vm.run(NORMAL)) { // running the opcode
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
            vm.disassembleScopes();

            std::cout << "\nEND OF PROGRAM";
        } else {
            std::cerr << "The accepted format for cbi is: " << argv[0] << " d:/path/to/file.cbi";
            return EXIT_FAILURE;
        }
    } catch (...) {
        std::cerr << "\nAn unexpected fatal error has occurred." << std::endl;
        return EXIT_FAILURE;
    }
}