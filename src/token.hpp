#ifndef _token_h
#define _token_h

#include <iostream>
#include <string>
#include <vector>

//std::vector<std::string> line_lookup;

enum Type {
    STRING, NUMBER, TRUE, FALSE, IDENTIFIER,

    SET, MUT, // for variables

    EQUAL, LESS, GREATER, NOT, PLUS, MINUS, STAR, SLASH, // single length operators

    // multiple length operators
    EQUAL_EQUAL, NOT_EQUAL, LESS_EQUAL, GREATER_EQUAL, CONCATENATE, PLUS_PLUS, MINUS_MINUS, PLUS_EQUALS,
    MINUS_EQUALS, STAR_EQUALS, SLASH_EQUALS,

    // keywords
    AND, OR, ELSE, FOR, IF, TOKEN_NULL, FUN, PRINT
};

class Token {
    public:
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
    void error(const char *message) {
        //std::cout << line << "| " << line_lookup[line] << "\n" << message << std::endl;
    }
};

#endif