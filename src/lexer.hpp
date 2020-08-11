#ifndef _lexer_h
#define _lexer_h

#include "token.hpp"
#include "types.hpp"

std::vector<Token> lex(std::vector<std::string> lines, const char* filename, bool &sucess, Mode mode);

#endif