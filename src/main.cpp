#include "vm.hpp"
#include "types.hpp"
#include "lexer.hpp"
#include "compiler.hpp"
#include "color.hpp"

#include <iostream>
#include <vector>
#include <fstream>
#include <chrono>

int main(int argc, char **argv) {
    try {
        if (2 <= argc <= 4) {
#define NOW() std::chrono::steady_clock::now()
#define PASSED_TIME() std::chrono::duration_cast<std::chrono::microseconds>(NOW() - saved_time).count()
            bool debugmode = false;
            bool success = true;
            int total_relevant_time = 0;

            if ((std::string)argv[1] == "-h" || (std::string)argv[1] == "-help") {
                std::cout << "Documentation: https://github.com/LackingInGrav1tas/cbi" << std::endl;
                return 0;
            }

            for (int i = 2; i < argc; i++) {
                if ((std::string)argv[i] == "-h" || (std::string)argv[i] == "-help") {
                    std::cout << "Unknown command '" << argv[i] << "'.\nDocumentation: https://github.com/LackingInGrav1tas/cbi" << std::endl;
                    return 0;
                } else if ((std::string)argv[i] == "-b" || (std::string)argv[i] == "-build") {
                    std::ofstream build("build.bat");
                    build << "@ECHO off\n";
                    for (int a = 0; a < argc; a++) {
                        build << argv[a] << " ";
                    }
                    build << "\npause";
                } else if ((std::string)argv[i] == "-d" || (std::string)argv[i] == "-debug")
                    debugmode = true;
                else {
                    std::cout << "Unknown command '" << argv[i] << "'.\nDocumentation: https://github.com/LackingInGrav1tas/cbi" << std::endl;
                    return 0;
                }
            }
            
    
            std::chrono::steady_clock::time_point saved_time;
            if (debugmode) {
                saved_time = NOW();
                std::cout << "Reading file... ";
            }
            std::vector<std::string> lines = getLines(argv[1], success); // getting the contents of the file
            lines.insert(lines.begin(), "prefix println(txt: ANY) precedence 1 { print $txt || \"\\n\"; } } {");
            lines.insert(lines.begin(), "infix exp(lhs: NUM, rhs: NUM) precedence 5 { set mut i = 1; set base = $lhs; while ($i < $rhs) { lhs *= $base; i += 1; } return $lhs; }");
            lines.insert(lines.begin(), "fn input(text: ANY) { print $text; set mut s; gets s; return $s; }");
            lines.insert(lines.begin(), "fn assert(expr: BOOL) { if (!$expr) throw \"assertion failed.\"; }");
            lines.insert(lines.begin(), "set EXIT_FAILURE = 1; set EXIT_SUCCESS = 0;");
#ifdef _WIN32
            lines.insert(lines.begin(), "set OS_WIN = true;");
#else
            lines.insert(lines.begin(), "set OS_WIN = false;");
#endif
#if defined(unix) || defined(__unix__) || defined(__unix)
            lines.insert(lines.begin(), "set OS_UNIX = true;");
#else
            lines.insert(lines.begin(), "set OS_UNIX = false;");
#endif
#ifdef __APPLE__
            lines.insert(lines.begin(), "set OS_MAC = true;");
#else
            lines.insert(lines.begin(), "set OS_MAC = false;");
#endif
#ifdef __Free_BSD__
            lines.insert(lines.begin(), "set OS_FBSD = true;");
#else
            lines.insert(lines.begin(), "set OS_FBSD = false;");
#endif
#ifdef __ANDROID__
            lines.insert(lines.begin(), "set OS_ANDR = true;");
#else
            lines.insert(lines.begin(), "namespace std { set OS_ANDR = false;");
#endif
            lines.push_back("}");
            if (!success) {
                std::cerr << "\nCould not access file" << std::endl;
                return EXIT_FAILURE;
            }

            if (debugmode) {
                int pt = PASSED_TIME();
                total_relevant_time += pt;
                std::cout << pt << " microseconds\nLexing file... ";
                saved_time = NOW();
            }
            auto tokens = lex(lines, argv[1], success); // lexing the file into tokens
            if (!success) {
                COLOR("\n\nFatal error(s) during scanning.\n", DISPLAY_RED);
                return EXIT_FAILURE;
            }
            /*for (int b = 0; b < tokens.size(); b++) {
                std::cout << tokens[b].lexeme << " ";
                if (tokens[b].type == SEMICOLON) std::cout << std::endl;
                if (tokens[b].type == RIGHT_BRACE) std::cout << std::endl;
                if (tokens[b].type == LEFT_BRACE) std::cout << std::endl;
            }*/

            if (debugmode) {
                int pt = PASSED_TIME();
                total_relevant_time += pt;
                std::cout << pt << " microseconds\nCompiling opcode... ";
                saved_time = NOW();
            }
            Machine vm = compile(tokens, success); // compiling the tokens to bytecode
            if (!success) {
                COLOR("\n\nFatal error(s) during compile time.\n", DISPLAY_RED);
                if (debugmode) std::cout << "EXIT_CT" << std::endl;
                return EXIT_FAILURE;
            }

            if (debugmode) {
                int pt = PASSED_TIME();
                total_relevant_time += pt;
                std::cout << pt << " microseconds\nDisassembling opcode... ";
                vm.disassembleOpcode();
                vm.disassembleConstants();
            }

            if (debugmode) {
                COLOR("\n== runtime ==\n", DISPLAY_GREEN);
                saved_time = NOW();
                Tag result = vm.run().type;
                switch (result) { // running the opcode
                    case TYPE_OK: std::cout << "\nEXIT_OK"; break;
                    case TYPE_RT_ERROR: std::cout << "\nEXIT_RT"; break;
                    default: std::cout << "\nbug: unknown error code."; break;
                }
                int pt = PASSED_TIME();
                total_relevant_time += pt;
                COLOR("\n== end ==", DISPLAY_GREEN);
                std::cout << "\nTime spent in runtime: " << pt << " microseconds.\n";
                vm.disassembleStack(); // this and
                vm.disassembleScopes(); // this should be empty if everything goes right
                std::cout << "Total relevant time taken: \n" << total_relevant_time << " microseconds\n" << total_relevant_time*0.000001 << " seconds";
                return result;
            } else {
                return vm.run().type;
            }
#undef NOW
#undef PASSED_TIME
        } else {
            std::cout << "The accepted format for cbi is: " + (std::string) argv[0] + " d:/path/to/file.cbi" << std::endl;
            return EXIT_FAILURE;
        }
    } catch (...) {
        COLOR("\nAn unexpected fatal error has occurred.", DISPLAY_RED);
        return EXIT_FAILURE;
    }
}