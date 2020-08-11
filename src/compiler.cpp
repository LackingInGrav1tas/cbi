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
        case PLUS_EQUALS:
        case MINUS_EQUALS:
        case STAR_EQUALS:
        case SLASH_EQUALS:
        case INCREMENT:
        case DECREMENT:
        case EQUAL: return 1;

        case OR: return 2;

        case EQUAL_EQUAL:
        case NOT_EQUAL: return 3;

        case LESS:
        case LESS_EQUAL:
        case GREATER:
        case GREATER_EQUAL: return 4;

        case MINUS:
        case PLUS: return 5;

        case STAR:
        case SLASH: return 6;

        case NOT: return 7;

        case DOT: return 8;

        default: return 0;
    }
}

Machine compile(std::vector<Token> tokens, bool &success) { // preps bytecode
    Machine vm;
    auto token = tokens.begin();
    #define TOKEN (*token)
    #define PREV (*std::prev(token))
    #define NEXT (*std::next(token))

    std::function<void(int)> expression = [&](int p)->void { // lambdas
        switch (TOKEN.type) {
            case LEFT_PAREN: {
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
            case MINUS: {
                token++;
                expression(7);
                vm.writeOp(TOKEN.line, OP_NEGATE);
                break;
            }
            case NUMBER: {
                vm.writeConstant(TOKEN.line, numberValue(std::stod(TOKEN.lexeme)));
                break;
            }
            case STRING: {
                vm.writeConstant(TOKEN.line, stringValue(TOKEN.lexeme));
                break;
            }
            case TRUE: {
                vm.writeConstant(TOKEN.line, boolValue(true));
                break;
            }
            case FALSE: {
                vm.writeConstant(TOKEN.line, boolValue(false));
                break;
            }
            case _EOF: break;
            default: {
                TOKEN.error("Expexted an expression with token " + TOKEN.lexeme + ".");
                success = false;
                return;
            }
        }
        while (p <= getPrecedence(NEXT.type)) { // fix this
            token++;
            switch (TOKEN.type) {
                case MINUS: {
                    token++;
                    expression(getPrecedence(MINUS)+1);
                    vm.writeOp(TOKEN.line, OP_SUB);
                    break;
                }
                case PLUS: {
                    token++;
                    expression(getPrecedence(PLUS)+1);
                    vm.writeOp(TOKEN.line, OP_ADD);
                    break;
                }
                case STAR: {
                    token++;
                    expression(getPrecedence(STAR)+1);
                    vm.writeOp(TOKEN.line, OP_MUL);
                    break;
                }
                case SLASH: {
                    token++;
                    expression(getPrecedence(SLASH)+1);
                    vm.writeOp(TOKEN.line, OP_DIV);
                    break;
                }
                default: break;
            }
        }
    };

    for (; token < tokens.end(); token++) {
        /*
        expression finds infix which it calls
        then finds postfix if it exists
        and recurses until it finds number, string, or bool
        */
        expression(1);
    }

    #undef TOKEN
    #undef PREV
    #undef NEXT

    return vm;
}