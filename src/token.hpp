#ifndef _token_h
#define _token_h

#include "types.hpp"

#include <iostream>
#include <string>
#include <vector>


std::vector<std::string> getLines(const char *filename);

enum Type {
    STRING, NUMBER, TRUE, FALSE, IDENTIFIER,

    SET, MUT, // for variables

    // single length operators
    EQUAL, LESS, GREATER, NOT, PLUS, MINUS, STAR, SLASH, DOT, LEFT_PAREN, RIGHT_PAREN,
    LEFT_BRACKET, RIGHT_BRACKET, SEMICOLON, DOLLAR,

    // multiple length operators
    EQUAL_EQUAL, NOT_EQUAL, LESS_EQUAL, GREATER_EQUAL, CONCATENATE, INCREMENT, DECREMENT, PLUS_EQUALS,
    MINUS_EQUALS, STAR_EQUALS, SLASH_EQUALS,

    // keywords
    AND, OR, ELSE, FOR, IF, TOKEN_NULL, FUN, PRINT, /*<-this*/

    _EOF
};

class Token {
    public:
    Type type;
    Mode mode;
    std::string lexeme;
    const char *filename;
    int line;
    Token(Type init_type, std::string init_lexeme, const char *init_filename, int init_line, Mode init_mode) {
        type = init_type;
        lexeme = init_lexeme;
        filename = init_filename;
        line = init_line;
        mode = init_mode;
    }
    void error(std::string message) {
        std::vector<std::string> lines = getLines(filename);
        if (line == -1) {
            std::cerr << "\n" << lines.size()+1 << "| _EOF\n" << message << std::endl;
        } else {
            std::cerr << "\n" << line+1 << "| ";
            for (auto it = lines.begin(); it < lines.end(); it++) {
                if (it-lines.begin() == line) {
                    std::cerr << *it;
                    break;
                }
            }
            std::cerr << "\n" << message << std::endl;
        }
    }
};

#endif