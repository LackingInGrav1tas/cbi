#include "color.hpp"
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
    
            if (debugmode) {
                COLOR("Reading file...\n", 9);
            }
            std::vector<std::string> lines = getLines(argv[1], success); // getting the contents of the file
            if (!success) {
                COLOR("\nCould not access file\n", 4);
                return EXIT_FAILURE;
            }

            if (debugmode) COLOR("Lexing file...\n", 9);
            auto tokens = lex(lines, argv[1], success); // lexing the file into tokens
            if (!success) {
                COLOR("\n\nFatal error(s) during scanning.\n", 4);
                return EXIT_FAILURE;
            }

            if (debugmode) COLOR("Compiling opcode...\n", 9);
            Machine vm = compile(tokens, success); // compiling the tokens to bytecode
            if (!success) {
                COLOR("\n\nFatal error(s) during compile time.\n", 4);
                if (debugmode) COLOR("EXIT_CT", 4);
                return EXIT_FAILURE;
            }

            if (debugmode) {
                if (debugmode) COLOR("Disassembling opcode...\n", 9);
                vm.disassembleOpcode();
                std::cout << "\n" << std::endl;
                vm.disassembleConstants();
            }

            if (debugmode) {
                std::cout << "\n== runtime ==" << std::endl;
                switch (vm.run()) { // running the opcode
                    case EXIT_OK: std::cout << "\nEXIT_OK"; break;
                    case EXIT_RT: std::cout << "\nEXIT_RT"; break;
                    case EXIT_CT: std::cout << "\nEXIT_CT"; break;
                    default: std::cout << "\nbug: unknown error code."; break;
                }
                std::cout << "\n== end ==";
            } else {
                vm.run();
            }
        } else {
            COLOR("The accepted format for cbi is: " + (std::string) argv[0] + " d:/path/to/file.cbi", 4);
            return EXIT_FAILURE;
        }
    } catch (...) {
        COLOR("\nAn unexpected fatal error has occurred.", 4);
        return EXIT_FAILURE;
    }
}