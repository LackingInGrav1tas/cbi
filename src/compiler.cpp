#include "token.hpp"
#include "compiler.hpp"
#include "vm.hpp"
#include "types.hpp"
#include "lexer.hpp"

#include <string>
#include <vector>
#include <iostream>
#include <functional>

static int getPrecedence(Type type) {
    switch (type) {
        case EQUAL: return 1;

        case OR: return 2;

        case AND: return 3;

        case EQUAL_EQUAL:
        case NOT_EQUAL: return 4;

        case LESS:
        case LESS_EQUAL:
        case GREATER:
        case GREATER_EQUAL: return 5;

        case CONCATENATE:
        case MINUS:
        case PLUS: return 6;

        case SLASH:
        case STAR: return 7;

        //case DOT:
        //case LEFT_PAREN: return 8;
        
        default: return 0;
    }
}

Machine compile(std::vector<Token> tokens, bool &success) { // preps bytecode
    Machine vm;
    auto token = tokens.begin();
    #define TOKEN (*token)
    #define PREV (*std::prev(token))
    #define NEXT (*std::next(token))

    #define CHECK(t) (TOKEN.type == t)

    std::function<void(int)> expression = [&](int p)->void { // this notation used because c++ has no nested functions
        switch (TOKEN.type) {
            case LEFT_PAREN: { // group
                token++;
                expression(1);
                if (NEXT.type != RIGHT_PAREN) {
                    NEXT.error("Compile-time Error: Expected ')' after expression.");
                    success = false;
                    return;
                }
                token++;
                break;
            }
            case MINUS: { // prefix negation -
                token++;
                expression(7);
                vm.writeOp(TOKEN.line, OP_NEGATE);
                break;
            }
            case DOLLAR: { // prefix retrieve &<identifier>
                vm.writeOp(TOKEN.line, OP_RETRIEVE);
                token++;
                vm.constants.push_back(stringValue(TOKEN.lexeme));
                vm.writeOp(TOKEN.line, vm.constants.size()-1);
                break;
            }
            case NUMBER: { // number literal
                vm.writeConstant(TOKEN.line, numberValue(std::stod(TOKEN.lexeme)));
                break;
            }
            case STRING: { // string literal
                vm.writeConstant(TOKEN.line, stringValue(TOKEN.lexeme));
                break;
            }
            case TRUE: { // true
                vm.writeConstant(TOKEN.line, boolValue(true));
                break;
            }
            case FALSE: { // false
                vm.writeConstant(TOKEN.line, boolValue(false));
                break;
            }
            case _EOF: break;
            default: {
                TOKEN.error("Compile-time Error: Expexted an expression with token " + TOKEN.lexeme + " in line " + std::to_string(TOKEN.line) + ".");
                success = false;
                return;
            }
        }
        while (p <= getPrecedence(NEXT.type)) {
            token++;
            switch (TOKEN.type) {
                case MINUS: { // infix subtraction - 
                    token++;
                    expression(getPrecedence(MINUS)+1);
                    vm.writeOp(TOKEN.line, OP_SUB);
                    break;
                }
                case PLUS: { // +
                    token++;
                    expression(getPrecedence(PLUS)+1);
                    vm.writeOp(TOKEN.line, OP_ADD);
                    break;
                }
                case STAR: { // *
                    token++;
                    expression(getPrecedence(STAR)+1);
                    vm.writeOp(TOKEN.line, OP_MUL);
                    break;
                }
                case SLASH: { // /
                    token++;
                    expression(getPrecedence(SLASH)+1);
                    vm.writeOp(TOKEN.line, OP_DIV);
                    break;
                }
                case CONCATENATE: { // ||
                    token++;
                    expression(getPrecedence(CONCATENATE)+1);
                    vm.writeOp(TOKEN.line, OP_CONCATENATE);
                    break;
                }
                case LESS: { // <
                    token++;
                    expression(getPrecedence(LESS)+1);
                    vm.writeOp(TOKEN.line, OP_LESS);
                    break;
                }
                case LESS_EQUAL: { // <=
                    token++;
                    expression(getPrecedence(LESS_EQUAL)+1);
                    vm.writeOp(TOKEN.line, OP_LESS_EQ);
                    break;
                }
                case GREATER: { // >
                    token++;
                    expression(getPrecedence(GREATER)+1);
                    vm.writeOp(TOKEN.line, OP_GREATER);
                    break;
                }
                case GREATER_EQUAL: { // >=
                    token++;
                    expression(getPrecedence(GREATER_EQUAL)+1);
                    vm.writeOp(TOKEN.line, OP_GREATER_EQ);
                    break;
                }
                case EQUAL_EQUAL: { // ==
                    token++;
                    expression(getPrecedence(EQUAL_EQUAL)+1);
                    vm.writeOp(TOKEN.line, OP_EQUALITY);
                    break;
                }
                default: break;
            }
        }
    };

    std::function<void()> declaration = [&]()->void {
        if (CHECK(SET)) { // setting variable
            token++;
            if (!CHECK(IDENTIFIER)) {
                TOKEN.error("Compile-time Error: Expected an identifier.");
                success = false;
                return;
            }

            vm.writeConstant(TOKEN.line, stringValue(TOKEN.lexeme)); // note: this won't show up in debug if the lexeme
                                                                     // is >= 2 because of TRIM()
            token++;
            if (CHECK(EQUAL)) {
                token++;
                expression(1);
                token++;
            } else {
                vm.writeConstant(TOKEN.line, nullValue());
            }
            vm.writeOp(TOKEN.line, OP_GLOBAL);
        } else if (CHECK(PRINT)) { // printing
            token++;
            expression(1);
            token++;
            if (TOKEN.type != SEMICOLON) {
                TOKEN.error("Compile-time Error: Expected a semicolon.");
                success = false;
                return;
            }
            vm.writeOp(TOKEN.line, OP_PRINT_TOP);
        } else {
            expression(1);
            token++;
            if (TOKEN.type != SEMICOLON) {
                TOKEN.error("Compile-time Error: Expected a semicolon.");
                success = false;
                return;
            }
            vm.writeOp(TOKEN.line, OP_POP_TOP);
        }
    };

    for (; TOKEN.type != _EOF && token < tokens.end(); token++) // this is where the compiling starts
        declaration();
    // and finishes

    #undef TOKEN
    #undef PREV
    #undef NEXT

    return vm;
}