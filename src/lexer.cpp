#include "lexer.hpp"
#include "token.hpp"
#include "types.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

// getting the lines of the file.
std::vector<std::string> getLines(const char *filename, bool &success) {
    std::vector<std::string> lines;
    std::string line;
    std::ifstream file(filename);
    if (!file) {
        success = false;
        return lines;
    }
    while (std::getline(file, line)) // also checks to see if the file was successfully found
        lines.push_back(line); // pushes the line to the vector
    return lines;
}

std::vector<Token> lex(std::vector<std::string> lines, const char* filename, bool &sucess) {
    std::vector<Token> tokens;

    // for verbosity
    #define c (*character)
    #define LINE (*line)
    for (auto line = lines.begin(); line < lines.end(); line++) { // for each line
        std::string lexeme = "";

        for (auto character = LINE.begin(); character < LINE.end(); character++) { // for each character
            switch (c) {
                #define ERROR(message) \
                    do { std::cerr << "\n" << (line-lines.begin())+1 << "| " << LINE << message << std::endl; \
                    sucess = false; \
                    character = LINE.end(); } while (false)
                #define PUSH_TOKEN(lexeme) \
                    do { if (lexeme.empty()); \
                    else if (lexeme == "print") tokens.push_back(Token(PRINT, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "and") tokens.push_back(Token(AND, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "or") tokens.push_back(Token(OR, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "set") tokens.push_back(Token(SET, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "mut") tokens.push_back(Token(MUT, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "null") tokens.push_back(Token(TOKEN_NULL, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "true") tokens.push_back(Token(TOKEN_TRUE, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "false") tokens.push_back(Token(TOKEN_FALSE, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "fn") tokens.push_back(Token(FUN, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "if") tokens.push_back(Token(IF, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "else") tokens.push_back(Token(ELSE, lexeme, filename, line-lines.begin())); \
                    /*else if (lexeme == "for") tokens.push_back(Token(FOR, lexeme, filename, line-lines.begin()));*/ \
                    else if (lexeme == "while") tokens.push_back(Token(WHILE, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "break") tokens.push_back(Token(BREAK, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "use") tokens.push_back(Token(USE, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "return") tokens.push_back(Token(RETURN, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "disassemble_constants") tokens.push_back(Token(DIS_C, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "disassemble_stack") tokens.push_back(Token(DIS_ST, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "disassemble_scopes") tokens.push_back(Token(DIS_SC, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "gets") tokens.push_back(Token(GETS, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "getc") tokens.push_back(Token(GETCH, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "aware") tokens.push_back(Token(AWARE, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "blind") tokens.push_back(Token(BLIND, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "namespace") tokens.push_back(Token(NAMESPACE, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "NUM") tokens.push_back(Token(NUM, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "STR") tokens.push_back(Token(STR, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "VOID") tokens.push_back(Token(_VOID, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "BOOL") tokens.push_back(Token(_BOOL, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "LIST") tokens.push_back(Token(_LIST, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "ANY") tokens.push_back(Token(ANY, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "as") tokens.push_back(Token(AS, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "infix") tokens.push_back(Token(INFIX, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "prefix") tokens.push_back(Token(PREFIX, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "precedence") tokens.push_back(Token(PRECEDENCE, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "null") tokens.push_back(Token(TOKEN_NULL, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "list") tokens.push_back(Token(LIST, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "push") tokens.push_back(Token(PUSH, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "pop") tokens.push_back(Token(POP, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "back") tokens.push_back(Token(BACK, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "front") tokens.push_back(Token(FRONT, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "at") tokens.push_back(Token(AT_KEYWORD, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "sizeof") tokens.push_back(Token(SIZEOF, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "ascii") tokens.push_back(Token(ASCII, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "throw") tokens.push_back(Token(THROW, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "console") tokens.push_back(Token(CONSOLE, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "sleep") tokens.push_back(Token(SLEEP, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "rand") tokens.push_back(Token(RAND, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "floor") tokens.push_back(Token(FLOOR, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "clipboard") tokens.push_back(Token(CLIPBOARD, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "write") tokens.push_back(Token(WRITE, lexeme, filename, line-lines.begin())); \
                    else tokens.push_back(Token(IDENTIFIER, lexeme, filename, line-lines.begin())); \
                    lexeme.clear(); } while (false)

                // alphabet
                case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
                case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
                case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
                case 's': case 't': case 'u': case 'v': case 'w': case 'x':
                case 'y': case 'z': case 'A': case 'B': case 'C': case 'D':
                case 'E': case 'F': case 'G': case 'H': case 'I': case 'J':
                case 'K': case 'L': case 'M': case 'N': case 'O': case 'P':
                case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V':
                case 'W': case 'X': case 'Y': case 'Z': case '_': {
                    lexeme += c;
                    break;
                }
                // characters which signify the end of a token
                case '#': {
                    PUSH_TOKEN(lexeme);
                    character++;
                    for (; c != '#' && character < LINE.end(); character++);
                    break;
                }
                case '"':
                case "'"[0]: {
                    char current = c;
                    PUSH_TOKEN(lexeme);
                    lexeme += c;
                    character++;
                    for (; c != current && character < LINE.end(); character++) {
                        if (lexeme.length() > 1) {
                            if (lexeme.back() == '\\' && c == 'n') {
                                lexeme.pop_back();
                                lexeme.push_back('\n');
                                continue;
                            }
                            if (lexeme.back() == '\\' && c == 'r') {
                                lexeme.pop_back();
                                lexeme.push_back('\r');
                                continue;
                            }
                        }
                        lexeme += c;
                    }
                    if (character == LINE.end()) {
                        ERROR("\nSyntax Error: Unending string.");
                        break;
                    }
                    lexeme.push_back(current);
                    tokens.push_back(Token(STRING, lexeme, filename, line - lines.begin()));
                    lexeme.clear();
                    break;
                }
                case '0': case '1': case '2': case '3': case '4': case '5': 
                case '6': case '7': case '8': case '9': {
                    PUSH_TOKEN(lexeme);
                    for (; c == '0' || c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6' || 
                           c == '7' || c == '8' || c == '9'; character++) {
                        lexeme += c;
                    }
                    if (c == '.') {
                        lexeme += c;
                        character++;
                        for (; c == '0' || c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6' || 
                               c == '7' || c == '8' || c == '9'; character++) {
                               lexeme += c;
                        }
                    }
                    tokens.push_back(Token(NUMBER, lexeme, filename, line-lines.begin()));
                    lexeme.clear();
                    character--;
                    break;
                }
                case '.':
                    PUSH_TOKEN(lexeme);
                    if (character+1 == LINE.end()) tokens.push_back(Token(DOT, ".", filename, line-lines.begin()));
                    else {
                        character++;
                        if (c == '0' || c == '1' || c == '2' || c == '3' || c == '4' ||
                            c == '5' || c == '6' || c == '7' || c == '8' || c == '9') {
                            lexeme += ".";
                            for (; c == '0' || c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6' || 
                               c == '7' || c == '8' || c == '9'; character++) {
                               lexeme += c;
                            }
                            tokens.push_back(Token(NUMBER, "0" + lexeme, filename, line-lines.begin()));
                            lexeme.clear();
                        } else {
                            tokens.push_back(Token(DOT, ".", filename, line-lines.begin()));
                            character--;
                        }
                    }
                    break;
                case ' ': {
                    PUSH_TOKEN(lexeme);
                    break;
                }
                // necessarily one character operators
                case '%': {
                    PUSH_TOKEN(lexeme);
                    tokens.push_back(Token(PERCENT, "%", filename, line-lines.begin()));
                    break;
                }
                case ';': {
                    PUSH_TOKEN(lexeme);
                    tokens.push_back(Token(SEMICOLON, ";", filename, line-lines.begin()));
                    break;
                }
                case ':': {
                    if (*(character+1) == ':') {
                        character++;
                        lexeme += "::";
                    } else {
                        PUSH_TOKEN(lexeme);
                        tokens.push_back(Token(COLON, ":", filename, line-lines.begin()));
                    }
                    break;
                }
                case '$': {
                    PUSH_TOKEN(lexeme);
                    tokens.push_back(Token(DOLLAR, "$", filename, line-lines.begin()));
                    break;
                }
                case '@': {
                    PUSH_TOKEN(lexeme);
                    tokens.push_back(Token(AT, "@", filename, line-lines.begin()));
                    break;
                }
                case ',': {
                    PUSH_TOKEN(lexeme);
                    tokens.push_back(Token(COMMA, ",", filename, line-lines.begin()));
                    break;
                }
                case '(': {
                    PUSH_TOKEN(lexeme);
                    tokens.push_back(Token(LEFT_PAREN, "(", filename, line-lines.begin()));
                    break;
                }
                case ')': {
                    PUSH_TOKEN(lexeme);
                    tokens.push_back(Token(RIGHT_PAREN, ")", filename, line-lines.begin()));
                    break;
                }
                case '{': {
                    PUSH_TOKEN(lexeme);
                    tokens.push_back(Token(LEFT_BRACE, "{", filename, line-lines.begin()));
                    break;
                }
                case '}': {
                    PUSH_TOKEN(lexeme);
                    tokens.push_back(Token(RIGHT_BRACE, "}", filename, line-lines.begin()));
                    break;
                }
                case '[': {
                    PUSH_TOKEN(lexeme);
                    tokens.push_back(Token(LEFT_BRACKET, "{", filename, line-lines.begin()));
                    break;
                }
                case ']': {
                    PUSH_TOKEN(lexeme);
                    tokens.push_back(Token(RIGHT_BRACKET, "}", filename, line-lines.begin()));
                    break;
                }
                // necessarily two character operators
                case '|': {
                    PUSH_TOKEN(lexeme);
                    if (character == LINE.end()-1) {
                        ERROR("\nSyntax Error: Expected '||' character but only found '|'");
                        break;
                    } else if (*(character+1) != '|') {
                        ERROR("\nSyntax Error: Expected '||' character but only found '|'");
                        break;
                    }
                    character++;
                    if (character == LINE.end()-1) {
                    } else if (*(character+1) == '=') {
                        tokens.push_back(Token(CONCAT_EQUALS, "||=", filename, line-lines.begin()));
                        character++;
                        break;
                    }
                    character--;
                    tokens.push_back(Token(CONCATENATE, "||", filename, line-lines.begin()));
                    character++;
                    break;
                }
                // one or two character operators
                case '=': { // =, ==
                    PUSH_TOKEN(lexeme);
                    if (character == LINE.end()-1)
                        tokens.push_back(Token(EQUAL, "=", filename, line-lines.begin()));
                    else if (*(character+1) == '=') {
                        tokens.push_back(Token(EQUAL_EQUAL, "==", filename, line-lines.begin()));
                        character++;
                    } else
                        tokens.push_back(Token(EQUAL, "=", filename, line-lines.begin()));
                    break;
                }
                case '<': { // <, <=
                    PUSH_TOKEN(lexeme);
                    if (character == LINE.end()-1)
                        tokens.push_back(Token(LESS, "<", filename, line-lines.begin()));
                    else if (*(character+1) == '=') {
                        tokens.push_back(Token(LESS_EQUAL, "==", filename, line-lines.begin()));
                        character++;
                    } else
                        tokens.push_back(Token(LESS, "=", filename, line-lines.begin()));
                    break;
                }
                case '>': { // >, >=
                    PUSH_TOKEN(lexeme);
                    if (character == LINE.end()-1)
                        tokens.push_back(Token(GREATER, ">", filename, line-lines.begin()));
                    else if (*(character+1) == '=') {
                        tokens.push_back(Token(GREATER_EQUAL, ">=", filename, line-lines.begin()));
                        character++;
                    } else
                        tokens.push_back(Token(GREATER, ">", filename, line-lines.begin()));
                    break;
                }
                case '!': { // !, !=
                    PUSH_TOKEN(lexeme);
                    if (character == LINE.end()-1)
                        tokens.push_back(Token(NOT, "!", filename, line-lines.begin()));
                    else if (*(character+1) == '=') {
                        tokens.push_back(Token(NOT_EQUAL, "!=", filename, line-lines.begin()));
                        character++;
                    } else
                        tokens.push_back(Token(NOT, "!", filename, line-lines.begin()));
                    break;
                }
                case '+': { // +, +=
                    PUSH_TOKEN(lexeme);
                    if (character == LINE.end()-1)
                        tokens.push_back(Token(PLUS, "+", filename, line-lines.begin()));
                    else if (*(character+1) == '=') {
                        tokens.push_back(Token(PLUS_EQUALS, "+=", filename, line-lines.begin()));
                        character++;
                    } else
                        tokens.push_back(Token(PLUS, "+", filename, line-lines.begin()));
                    break;
                }
                case '-': { // -, -=, --
                    PUSH_TOKEN(lexeme);
                    if (character == LINE.end()-1)
                        tokens.push_back(Token(MINUS, "-", filename, line-lines.begin()));
                    else if (*(character+1) == '=') {
                        tokens.push_back(Token(MINUS_EQUALS, "-=", filename, line-lines.begin()));
                        character++;
                    } else
                        tokens.push_back(Token(MINUS, "-", filename, line-lines.begin()));
                    break;
                }
                case '*': { // *, *=
                    PUSH_TOKEN(lexeme);
                    if (character == LINE.end()-1)
                        tokens.push_back(Token(STAR, "*", filename, line-lines.begin()));
                    else if (*(character+1) == '=') {
                        tokens.push_back(Token(STAR_EQUALS, "*=", filename, line-lines.begin()));
                        character++;
                    } else
                        tokens.push_back(Token(STAR, "*", filename, line-lines.begin()));
                    break;
                }
                case '/': { // /, /=
                    PUSH_TOKEN(lexeme);
                    if (character == LINE.end()-1)
                        tokens.push_back(Token(SLASH, "/", filename, line-lines.begin()));
                    else if (*(character+1) == '=') {
                        tokens.push_back(Token(SLASH_EQUALS, "/=", filename, line-lines.begin()));
                        character++;
                    } else
                        tokens.push_back(Token(SLASH, "/", filename, line-lines.begin()));
                    break;
                }
                default: {
                    ERROR("\nSyntax Error: Unrecognized character: " + c);
                    break;
                }
            }
        }
        PUSH_TOKEN(lexeme);
    }
    #undef c
    #undef rline
    #undef PUSH_TOKEN
    tokens.push_back(Token(_EOF, "_EOF", filename, -1));
    return tokens;
}