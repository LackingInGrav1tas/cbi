#include "vm.hpp"
#include "types.hpp"
#include "lexer.hpp"
#include "compiler.hpp"

#include <iostream>
#include <vector>
#include <fstream>

int main(int argc, char **argv) {
    try {
        if (2 <= argc <= 3) {
            bool debugmode = false;
            bool success = true;
            if (argc == 3)
            if ((std::string)argv[2] == "-debug")
                debugmode = true;

            std::vector<std::string> lines = getLines(argv[1], success); // getting the contents of the file
            if (!success) {
                std::cerr << "\nCould not access file." << std::endl;
                return EXIT_FAILURE;
            }

            auto tokens = lex(lines, argv[1], success); // lexing the file into tokens
            if (!success) {
                std::cerr << "\nFatal error(s) during scanning." << std::endl;
                return EXIT_FAILURE;
            }

            Machine vm = compile(tokens, success); // compiling the tokens to bytecode
            if (!success) {
                std::cerr << "\nFatal error(s) during compile time." << std::endl;
                return EXIT_FAILURE;
            }

            if (debugmode) {
                vm.disassembleOpcode();
                std::cout << std::endl;
            }

            if (debugmode) {
                switch (vm.run()) { // running the opcode
                    case EXIT_OK: std::cout << "\nEXIT_OK"; break;
                    case EXIT_RT: std::cout << "\nEXIT_RT"; break;
                    case EXIT_CT: std::cout << "\nEXIT_CT"; break;
                    default: std::cout << "\nbug: unknown error code."; break;
                }
            } else {
                vm.run();
            }

            if (debugmode) {
                std::cout << "\n" << std::endl;
                vm.disassembleConstants();
                std::cout << "\nEND OF PROGRAM";
            }
        } else {
            std::cerr << "The accepted format for cbi is: " << argv[0] << " d:/path/to/file.cbi";
            return EXIT_FAILURE;
        }
    } catch (...) {
        std::cerr << "\nAn unexpected fatal error has occurred." << std::endl;
        return EXIT_FAILURE;
    }
}