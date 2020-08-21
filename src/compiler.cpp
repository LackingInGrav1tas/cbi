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
    bool panicking = false;
    auto token = tokens.begin();
    #define TOKEN (*token)
    #define PREV (*std::prev(token))
    #define NEXT (*std::next(token))

    #define ERROR(message) \
        do { \
            TOKEN.error(message); \
            success = false; \
            std::cerr.setstate(std::ios_base::failbit); \
            panicking = true; \
            return; \
        } while (false)

    #define CHECK(t) (TOKEN.type == t)

    #define HANDLE_BLOCK() \
        /* begins with LEFT_BRACKET ends with RIGHT_BRACKET */\
        do { \
            token++; \
            for (; !CHECK(_EOF) && !CHECK(RIGHT_BRACKET) && token < tokens.end(); token++) { \
                declaration(); \
                if (panicking) { \
                    for (; !CHECK(_EOF) && !CHECK(SEMICOLON) && token < tokens.end(); token++); \
                    panicking = false; \
                } \
            } \
            if (!CHECK(RIGHT_BRACKET)) ERROR(" Expected '}' after block."); \
        } while (false)\

    std::function<void(int)> expression = [&](int p)->void { // this notation used because c++ has no nested functions
        switch (TOKEN.type) {
            case LEFT_PAREN: { // group
                token++;
                expression(1);
                token++;
                if (!CHECK(RIGHT_PAREN)) ERROR(" Expected ')' after expression.");
                break;
            }
            case MINUS: { // prefix negation -
                token++;
                expression(7);
                vm.writeOp(TOKEN.line, OP_NEGATE);
                break;
            }
            case NOT: { // prefix negation -
                token++;
                expression(7);
                vm.writeOp(TOKEN.line, OP_NOT);
                break;
            }
            case DOLLAR: { // prefix retrieve &<identifier>
                vm.writeOp(TOKEN.line, OP_RETRIEVE);
                token++;
                if (TOKEN.type != IDENTIFIER) ERROR("Cannot retrieve non-identifier.");
                vm.constants.push_back(idLexeme(TOKEN.lexeme));
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
            case TOKEN_TRUE: { // true
                vm.writeConstant(TOKEN.line, boolValue(true));
                break;
            }
            case TOKEN_FALSE: { // false
                vm.writeConstant(TOKEN.line, boolValue(false));
                break;
            }
            case IDENTIFIER: {
                vm.writeConstant(TOKEN.line, idLexeme(TOKEN.lexeme));
                if (NEXT.type != EQUAL) ERROR(" Stray identifier."); 
                break;
            }
            case _EOF: break;
            default: {
                ERROR(" Expected an expression.");
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
                case NOT_EQUAL: {
                    token++;
                    expression(getPrecedence(NOT_EQUAL)+1);
                    vm.writeOp(TOKEN.line, OP_NOT_EQ);
                    break;
                }
                case EQUAL: {
                    token++;
                    expression(1);
                    vm.writeOp(TOKEN.line, OP_SET_VARIABLE);
                    break;
                }
                case AND: {
                    token++;
                    expression(getPrecedence(AND)+1);
                    vm.writeOp(TOKEN.line, OP_AND);
                    break;
                }
                case OR: {
                    token++;
                    expression(getPrecedence(OR)+1);
                    vm.writeOp(TOKEN.line, OP_OR);
                    break;
                }
                default: break;
            }
        }
    };

    auto expStatement = [&]() {
        expression(1);
        token++;
        if (TOKEN.type != SEMICOLON) ERROR(" Expected a semicolon.");
        vm.writeOp(TOKEN.line, OP_POP_TOP);
    };

    auto setVariable = [&]() {
        token++;
        bool mut = true;
        //checking mut
        if (!CHECK(MUT))
            mut = false;
        else
            token++;

        if (!CHECK(IDENTIFIER)) {
            token--;
            ERROR(" Expected an identifier.");
        }

        vm.writeConstant(TOKEN.line, idLexeme(TOKEN.lexeme)); // note: this won't show up in debug if the lexeme
                                                                    // is >= 2 because of TRIM()
        token++;
        if (CHECK(EQUAL)) {
            token++;
            expression(1);
            token++;
        } else vm.writeConstant(TOKEN.line, nullValue());

        if (!CHECK(SEMICOLON)) ERROR(" Expected a semicolon.");

        if (mut) vm.writeOp(TOKEN.line, OP_VARIABLE_MUT);
        else vm.writeOp(TOKEN.line, OP_VARIABLE);
    };

    std::function<void()> declaration = [&]()->void {
        if (CHECK(SET)) { // setting scoped variable
            setVariable();
        }else if (CHECK(PRINT)) { // printing
            token++;
            expression(1);
            token++;

            if (!CHECK(SEMICOLON)) ERROR(" Expected a semicolon.");

            vm.writeOp(TOKEN.line, OP_PRINT_TOP);
        } else if (CHECK(IF)) { // if statement
            token++;

            if (!CHECK(LEFT_PAREN)) ERROR(" Expected '(' after if.");

            token++;
            expression(1);
            token++;

            if (!CHECK(RIGHT_PAREN)) ERROR(" Expected ')' after if condition.");

            vm.writeOp(TOKEN.line, OP_JUMP_FALSE_IFv);
            int size = vm.opcode.size();
            vm.opcode.push_back(uint8_t());
            vm.lines.push_back(TOKEN.line);

            token++;
            if (!CHECK(LEFT_BRACKET))
                declaration();
            else {
                vm.writeOp(TOKEN.line, OP_BEGIN_SCOPE);
                HANDLE_BLOCK();
                vm.writeOp(TOKEN.line, OP_END_SCOPE);
            }
            token++;
            if (!CHECK(ELSE)) { // "if" <flexible-block>
                vm.opcode[size] = vm.opcode.size();
                token--;
            } else { // "if" <flexible-block> else <flexible-block>
                vm.writeOp(TOKEN.line, OP_JUMP);
                int elsesize = vm.opcode.size();
                vm.opcode.push_back(uint8_t());
                vm.lines.push_back(TOKEN.line);
                vm.opcode[size] = vm.opcode.size();
                token++;
                if (!CHECK(LEFT_BRACKET))
                    declaration();
                else {
                    vm.writeOp(TOKEN.line, OP_BEGIN_SCOPE);
                    HANDLE_BLOCK();
                    vm.writeOp(TOKEN.line, OP_END_SCOPE);
                }
                vm.opcode[elsesize] = vm.opcode.size();
            }
        } else if (CHECK(WHILE)) { // while statement
            vm.writeOp(TOKEN.line, OP_BEGIN_SCOPE);
            token++;

            if (!CHECK(LEFT_PAREN)) ERROR(" Expected '(' after while.");
            int presize = vm.opcode.size(); // before the condition
            token++;
            expression(1);
            token++;
            if (!CHECK(RIGHT_PAREN)) ERROR(" Expected ')' after while condition.");

            vm.writeOp(TOKEN.line, OP_JUMP_FALSE);
            int size = vm.opcode.size();
            vm.opcode.push_back(uint8_t());
            vm.lines.push_back(TOKEN.line);

            token++;
            if (!CHECK(LEFT_BRACKET))
                declaration();
            else
                HANDLE_BLOCK();

            vm.writeOp(TOKEN.line, OP_JUMP); // jumping the beginning
            vm.writeOp(TOKEN.line, presize);

            vm.opcode[size] = vm.opcode.size(); // skipping past the bytecode

            vm.writeOp(TOKEN.line, OP_END_SCOPE);
        } else if (CHECK(BREAK)) {
            vm.writeOp(TOKEN.line, OP_BREAK);
            token++;
            if (TOKEN.type != SEMICOLON) ERROR(" Expected a semicolon.");
        } else if (CHECK(LEFT_BRACKET)) { // block
            vm.writeOp(TOKEN.line, OP_BEGIN_SCOPE);
            HANDLE_BLOCK();
            vm.writeOp(TOKEN.line, OP_END_SCOPE);
        } else {
            expStatement();
        }
    };

    vm.writeOp(-1, OP_BEGIN_SCOPE);
    for (; !CHECK(_EOF) && token < tokens.end(); token++) {
        declaration();
        if (panicking) {
            //for (; !CHECK(_EOF) && !CHECK(SEMICOLON) && token < tokens.end(); token++);
            std::cerr.clear();
            panicking = false;
        }
    }
    vm.writeOp(-1, OP_END_SCOPE);

    #undef TOKEN
    #undef PREV
    #undef NEXT

    return vm;
}