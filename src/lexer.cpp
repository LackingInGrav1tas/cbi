#include "lexer.hpp"
#include "token.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

// getting the lines of the file.
std::vector<std::string> getLines(const char *filename) {
    std::vector<std::string> lines;
    std::string line;
    std::ifstream file(filename);
    while (std::getline(file, line) && !file) // also checks to see if the file was successfully found
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
            std::cout << c; // debugging
            switch (c) {
                #define FIND_KEYWORD(lexeme) \
                    {if (lexeme == "print") tokens.push_back(Token(PRINT, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "and") tokens.push_back(Token(AND, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "or") tokens.push_back(Token(OR, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "set") tokens.push_back(Token(SET, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "mut") tokens.push_back(Token(MUT, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "null") tokens.push_back(Token(TOKEN_NULL, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "true") tokens.push_back(Token(TRUE, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "false") tokens.push_back(Token(FALSE, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "fun") tokens.push_back(Token(FUN, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "if") tokens.push_back(Token(IF, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "else") tokens.push_back(Token(ELSE, lexeme, filename, line-lines.begin())); \
                    else if (lexeme == "for") tokens.push_back(Token(FOR, lexeme, filename, line-lines.begin())); \
                    else tokens.push_back(Token(IDENTIFIER, lexeme, filename, line-lines.begin())); \
                    lexeme.clear();}

                // alphabet
                case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
                case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
                case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
                case 's': case 't': case 'u': case 'v': case 'w': case 'x':
                case 'y': case 'z': case 'A': case 'B': case 'C': case 'D':
                case 'E': case 'F': case 'G': case 'H': case 'I': case 'J':
                case 'K': case 'L': case 'M': case 'N': case 'O': case 'P':
                case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V':
                case 'W': case 'X': case 'Y': case 'Z': {
                    lexeme += c;
                    break;
                }
                // characters which signify the end of a token
                case '#': {
                    if (!lexeme.empty()) FIND_KEYWORD(lexeme);
                    character++;
                    for (; c != '#' && character < LINE.end(); character++);
                    break;
                }
                case '"':
                case "'"[0]: {
                    char current = c;
                    if (!lexeme.empty()) FIND_KEYWORD(lexeme);
                    lexeme += c;
                    character++;
                    for (; c != current && character < LINE.end(); character++) lexeme += c;
                    if (character == LINE.end()) {
                        std::cerr << "Compile-time Error: Unending string.";
                        sucess = false;
                        return std::vector<Token>();
                    }
                    lexeme.push_back(current);
                    tokens.push_back(Token(STRING, lexeme, filename, line - lines.begin()));
                    lexeme.clear();
                    break;
                }
                case '0': case '1': case '2': case '3': case '4': case '5': 
                case '6': case '7': case '8': case '9': {
                    if (!lexeme.empty()) FIND_KEYWORD(lexeme);
                    //while () {

                    //}
                    break;
                }
                case ' ': {
                    if (!lexeme.empty()) FIND_KEYWORD(lexeme);
                    break;
                }
                default:
                    std::cerr << "Compile-time Error: Unrecognized character.";
                    sucess = false;
                    return std::vector<Token>();
            }
        }
        if (!lexeme.empty()) FIND_KEYWORD(lexeme);
        std::cout << " | tokens.s: " << tokens.size() << std::endl; // debugging
    }
    #undef c
    #undef rline

    return tokens;
}