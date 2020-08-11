#ifndef _com_h
#define _com_h

#include "token.hpp"
#include "types.hpp"

Machine compile(std::vector<Token> tokens, bool &sucess);

#endif