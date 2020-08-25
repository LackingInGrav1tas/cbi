#include "vm.hpp"
#include "types.hpp"
#include "lexer.hpp"
#include "compiler.hpp"

#include <iostream>
#include <vector>
#include <fstream>

std::vector<Function> functions;

int main(int argc, char **argv) {
    //try {
        if (2 <= argc <= 3) {
            bool debugmode = false;
            bool success = true;
            if (argc == 3)
            if ((std::string)argv[2] == "-debug")
                debugmode = true;
    
            if (debugmode) {
                std::cout << "Reading file..." << std::endl;
            }
            std::vector<std::string> lines = getLines(argv[1], success); // getting the contents of the file
            if (!success) {
                std::cerr << "\nCould not access file" << std::endl;
                return EXIT_FAILURE;
            }

            if (debugmode) std::cout << "Lexing file..." << std::endl;
            auto tokens = lex(lines, argv[1], success); // lexing the file into tokens
            if (!success) {
                std::cout << "\n\nFatal error(s) during scanning.\n" << std::endl;
                return EXIT_FAILURE;
            }

            if (debugmode) std::cout << "Compiling opcode..." << std::endl;
            Machine vm = compile(tokens, success); // compiling the tokens to bytecode
            if (!success) {
                std::cout << "\n\nFatal error(s) during compile time.\n" << std::endl;
                if (debugmode) std::cout << "EXIT_CT" << std::endl;
                return EXIT_FAILURE;
            }

            if (debugmode) {
                if (debugmode) std::cout << "Disassembling opcode..." << std::endl;
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
                std::cout << "\n== end ==\n\n";
                vm.disassembleStack(); // this and
                std::cout << std::endl;
                vm.disassembleScopes(); // this should be empty if everything goes right
            } else {
                vm.run();
            }

        } else {
            std::cout << "The accepted format for cbi is: " + (std::string) argv[0] + " d:/path/to/file.cbi" << std::endl;
            return EXIT_FAILURE;
        }
    //} catch (...) {
    //    std::cout << "\nAn unexpected fatal error has occurred." << std::endl;
    //    return EXIT_FAILURE;
    //}
}