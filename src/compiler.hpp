#ifndef _com_h
#define _com_h

#include "token.hpp"
#include "types.hpp"
#include "vm.hpp"

Machine compile(std::vector<Token> tokens, bool &success);

#endif