#ifndef _token_h
#define _token_h

#include "types.hpp"
#include "color.hpp"

#include <iostream>
#include <string>
#include <vector>

enum Type {
    STRING, NUMBER, TOKEN_TRUE, TOKEN_FALSE, IDENTIFIER,

    SET, MUT, // for variables

    // single length operators
    EQUAL, LESS, GREATER, NOT, PLUS, MINUS, STAR, SLASH, DOT, LEFT_PAREN, RIGHT_PAREN,
    LEFT_BRACKET, RIGHT_BRACKET, SEMICOLON, COLON, DOLLAR, COMMA, AT,
 
    // multiple length operators
    EQUAL_EQUAL, NOT_EQUAL, LESS_EQUAL, GREATER_EQUAL, CONCATENATE, PLUS_EQUALS,
    MINUS_EQUALS, STAR_EQUALS, SLASH_EQUALS, CONCAT_EQUALS,

    // keywords
    AND, OR, ELSE, FOR, WHILE, IF, TOKEN_NULL, FUN, PRINT, BREAK, C_SCOPE, NEW, RETURN,
    DIS_C, DIS_ST, DIS_SC, GETS, GETCH, AWARE, BLIND, AS, INFIX, PREFIX, POSTFIX, PRECEDENCE,

    NUM, STR, _VOID, _BOOL, ANY,

    _EOF
};

std::vector<std::string> getLines(const char *filename, bool &success);

struct Token {
    Type type;
    std::string lexeme;
    const char *filename;
    int line;
    Token(Type init_type, std::string init_lexeme, const char *init_filename, int init_line) {
        type = init_type;
        lexeme = init_lexeme;
        filename = init_filename;
        line = init_line;
    }
    void error(std::string message) {
        std::cerr << "\n";
        bool b = true;
        std::vector<std::string> lines = getLines(filename, b);
        if (!b | line == -1) {
            std::cerr << "\n" << lines.size()+2 << "| _EOF\n";
            COLOR("Compile-time Error", DISPLAY_RED);
            std::cerr << ": " << message;
        } else {
            std::cerr << "\n" << filename << ": ";
            COLOR("Compile-time Error", DISPLAY_RED);
            std::cerr << ": " << message << " TOKEN: " << lexeme;
            std::cerr << "\n" << line+1 << "| ";
            for (auto it = lines.begin(); it < lines.end(); it++) {
                if (it-lines.begin() == line) {
                    std::cerr << *it;
                    break;
                }
            }
        }
    }
};

#endif