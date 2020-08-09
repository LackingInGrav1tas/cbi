#ifndef _lexer_h
#define _lexer_h

#include "token.hpp"

std::vector<std::string> getLines(const char *filename);

std::vector<Token> lex(std::vector<std::string> lines, const char* filename, bool &sucess);

#endif