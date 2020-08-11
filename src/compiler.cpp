#include "compiler.hpp"
#include "token.hpp"
#include "vm.hpp"
#include "types.hpp"
#include "lexer.hpp"

#include <string>
#include <vector>
#include <iostream>

Machine compile(std::vector<Token> tokens, bool &sucess) { // preps bytecode
    Machine vm;

    #define TOKEN (*token)
    #define PREV (*std::prev(token))
    #define NEXT (*std::next(token))

    for (auto token = tokens.begin(); token < tokens.end(); token++) {
        /*
        expression finds infix which it calls
        then finds postfix if it exists
        and recurses until it finds number, string, or bool
        */
        
    }

    #undef TOKEN
    #undef PREV
    #undef NEXT

    return vm;
}