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
            if (!CHECK(RIGHT_BRACKET)) ERROR("Compile-time Error: Expected '}' after block."); \
        } while (false)\

    std::function<void(int)> expression = [&](int p)->void { // this notation used because c++ has no nested functions
        switch (TOKEN.type) {
            case LEFT_PAREN: { // group
                token++;
                expression(1);
                token++;
                if (!CHECK(RIGHT_PAREN)) ERROR("Compile-time Error: Expected ')' after expression.");
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
            case TRUE: { // true
                vm.writeConstant(TOKEN.line, boolValue(true));
                break;
            }
            case FALSE: { // false
                vm.writeConstant(TOKEN.line, boolValue(false));
                break;
            }
            case IDENTIFIER: {
                vm.writeConstant(TOKEN.line, idLexeme(TOKEN.lexeme));
                if (NEXT.type != EQUAL) ERROR("Compile-time Error: Stray identifier."); 
                break;
            }
            case _EOF: break;
            default: {
                ERROR("Compile-time Error: Expected an expression.");
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

    std::function<void()> declaration = [&]()->void {
        if (CHECK(SET)) { // setting scoped variable
            token++;
            bool mut = true;
            //checking mut
            if (!CHECK(MUT))
                mut = false;
            else
                token++;

            if (!CHECK(IDENTIFIER)) {
                token--;
                ERROR("Compile-time Error: Expected an identifier.");
            }

            vm.writeConstant(TOKEN.line, idLexeme(TOKEN.lexeme)); // note: this won't show up in debug if the lexeme
                                                                     // is >= 2 because of TRIM()
            token++;
            if (CHECK(EQUAL)) {
                token++;
                expression(1);
                token++;
            } else vm.writeConstant(TOKEN.line, nullValue());

            if (!CHECK(SEMICOLON)) ERROR("Compile-time Error: Expected a semicolon.");

            if (mut) vm.writeOp(TOKEN.line, OP_VARIABLE_MUT);
            else vm.writeOp(TOKEN.line, OP_VARIABLE);
        } else if (CHECK(PRINT)) { // printing
            token++;
            expression(1);
            token++;

            if (!CHECK(SEMICOLON)) ERROR("Compile-time Error: Expected a semicolon.");

            vm.writeOp(TOKEN.line, OP_PRINT_TOP);
        } else if (CHECK(IF)) { // if statement
            token++;

            if (!CHECK(LEFT_PAREN)) ERROR("Compile-time Error: Expected '(' after if.");

            token++;
            expression(1);
            token++;

            if (!CHECK(RIGHT_PAREN)) ERROR("Compile-time Error: Expected ')' after if condition.");

            vm.writeOp(TOKEN.line, OP_JUMP_FALSE);
            int size = vm.opcode.size();
            int line = TOKEN.line;

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
                vm.opcode.insert(vm.opcode.begin() + size, vm.opcode.size()+1);
                vm.lines.insert(vm.lines.begin() + size, line);
                token--;
            } else { // "if" <flexible-block> else <flexible-block>
                vm.writeOp(TOKEN.line, OP_JUMP);
                int elsesize = vm.opcode.size();
                int elseline = TOKEN.line;
                vm.opcode.insert(vm.opcode.begin() + size, vm.opcode.size()+2);
                vm.lines.insert(vm.lines.begin() + size, line);
                token++;
                if (!CHECK(LEFT_BRACKET))
                    declaration();
                else {
                    vm.writeOp(TOKEN.line, OP_BEGIN_SCOPE);
                    HANDLE_BLOCK();
                    vm.writeOp(TOKEN.line, OP_END_SCOPE);
                }
                vm.opcode.insert(vm.opcode.begin() + elsesize+1, vm.opcode.size()+1);
                vm.lines.insert(vm.lines.begin() + elsesize+1, elseline);
            }
        } else if (CHECK(WHILE)) { // while statement
            token++;

            if (!CHECK(LEFT_PAREN)) ERROR("Compile-time Error: Expected '(' after while.");
            token++;
            expression(1);
            token++;
            if (!CHECK(RIGHT_PAREN)) ERROR("Compile-time Error: Expected ')' after while condition.");

            int presize = vm.opcode.size();
            vm.writeOp(TOKEN.line, OP_JUMP_FALSE);
            int size = vm.opcode.size();
            int line = TOKEN.line;

            token++;
            if (!CHECK(LEFT_BRACKET))
                declaration();
            else {
                vm.writeOp(TOKEN.line, OP_BEGIN_SCOPE);
                HANDLE_BLOCK();
                vm.writeOp(TOKEN.line, OP_END_SCOPE);
            }

            vm.writeOp(TOKEN.line, OP_JUMP); // jumping the beginning
            vm.writeOp(TOKEN.line, presize-2);

            vm.opcode.insert(vm.opcode.begin() + size, vm.opcode.size()+1); // skipping past the bytecodes
            vm.lines.insert(vm.lines.begin() + size, line);
        } else if (CHECK(LEFT_BRACKET)) { // block
            vm.writeOp(TOKEN.line, OP_BEGIN_SCOPE);
            HANDLE_BLOCK();
            vm.writeOp(TOKEN.line, OP_END_SCOPE);
        } else {
            expression(1);
            token++;
            if (TOKEN.type != SEMICOLON) ERROR("Compile-time Error: Expected a semicolon.");
            vm.writeOp(TOKEN.line, OP_POP_TOP);
        }
    };

    vm.writeOp(-1, OP_BEGIN_SCOPE);
    for (; !CHECK(_EOF) && token < tokens.end(); token++) {
        declaration();
        if (panicking) {
            for (; !CHECK(_EOF) && !CHECK(SEMICOLON) && token < tokens.end(); token++);
            panicking = false;
        }
    }
    vm.writeOp(-1, OP_END_SCOPE);

    #undef TOKEN
    #undef PREV
    #undef NEXT

    return vm;
}