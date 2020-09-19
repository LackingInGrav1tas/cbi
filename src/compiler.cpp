#include "types.hpp"
#include "token.hpp"
#include "compiler.hpp"
#include "vm.hpp"
#include "lexer.hpp"
#include "color.hpp"

#include <algorithm>
#include <string>
#include <vector>
#include <iostream>
#include <functional>

struct CompilingEnvironment {
    std::map<std::string, int> custom_infix_ops; // for getting precedence
    std::map<std::string, int> custom_prefix_ops;
} environment;

static int getInfixOp(std::string opname) {
    auto found = environment.custom_infix_ops.find(opname);
    if (found == environment.custom_infix_ops.end()) return -1;
    return found->second;
}

static int getPrefixOp(std::string opname) {
    auto found = environment.custom_prefix_ops.find(opname);
    if (found == environment.custom_prefix_ops.end()) return -1;
    return found->second;
}

static int getPrecedence(Type type, std::string lexeme = "") {
    switch (type) {
        case PUSH:
        case CONCAT_EQUALS:
        case STAR_EQUALS:
        case SLASH_EQUALS:
        case PLUS_EQUALS:
        case MINUS_EQUALS:
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

        case PERCENT:
        case SLASH:
        case STAR: return 7;

        case AS:
        case AT_KEYWORD: return 8;

        //case DOT:
        //case LEFT_PAREN: return 9;
        
        case LEFT_BRACKET: return 9;

        case IDENTIFIER: {
            int infix = getInfixOp(lexeme);
            if (infix != -1) return infix;
            return getPrefixOp(lexeme);
        }

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

    #define SEMICOLON() if (!CHECK(SEMICOLON)) ERROR("Expected a semicolon.")

    #define IDEN() if (!CHECK(IDENTIFIER)) ERROR("Expected an identifier.")

    #define CHECK(t) (TOKEN.type == t)

    #define HANDLE_BLOCK() \
        /* begins with LEFT_BRACE ends with RIGHT_BRACE */\
        do { \
            token++; \
            for (; !CHECK(_EOF) && !CHECK(RIGHT_BRACE) && token < tokens.end(); token++) { \
                declaration(); \
                if (panicking) { \
                    for (; !CHECK(_EOF) && !CHECK(SEMICOLON) && token < tokens.end(); token++); \
                    panicking = false; \
                } \
            } \
            if (!CHECK(RIGHT_BRACE)) ERROR("Expected '}' after block."); \
        } while (false)\

    std::function<void(int)> expression = [&](int p)->void { // this notation used because c++ has no nested functions
        switch (TOKEN.type) { // prefix
            case LEFT_PAREN: { // group
                token++;
                expression(1);
                token++;
                if (!CHECK(RIGHT_PAREN)) ERROR("Expected ')' after expression.");
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
            case DOLLAR: { // prefix retrieve $IDENTIFIER
                token++;
                if (!CHECK(IDENTIFIER)) ERROR("Cannot retrieve non-identifier.");
                vm.writeOp(TOKEN.line, OP_RETRIEVE);
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
            case TOKEN_NULL: {
                vm.writeConstant(TOKEN.line, nullValue());
                break;
            }
            case LIST: {
                if (NEXT.type == LEFT_PAREN) {
                    token += 2;
                    int arity = -1; // -1 because of )
                    while (true) {
                        arity++;
                        if (CHECK(RIGHT_PAREN)) break;
                        expression(1);
                        token++;
                        if (CHECK(COMMA)) {
                            token++;
                            if (CHECK(RIGHT_PAREN)) ERROR("Expected an expression.");
                        }
                    }
                    if (!CHECK(RIGHT_PAREN)) ERROR("Expected ')'.");
                    vm.writeConstant(TOKEN.line, numberValue(arity));
                    vm.writeOp(TOKEN.line, OP_LIST_FN);
                } else
                    vm.writeConstant(TOKEN.line, listValue());
                break;
            }
            case IDENTIFIER: {
                std::string id = TOKEN.lexeme;
                int prec = getPrefixOp(id);
                if (prec != -1) {
                    token++;
                    expression(prec+1);
                    vm.writeConstant(TOKEN.line, idLexeme(id));
                    vm.writeOp(TOKEN.line, OP_CALL);
                    break;
                }
                vm.writeConstant(TOKEN.line, idLexeme(TOKEN.lexeme));
                if (NEXT.type != EQUAL && NEXT.type != PLUS_EQUALS && NEXT.type != MINUS_EQUALS
                && NEXT.type != SLASH_EQUALS && NEXT.type != STAR_EQUALS && NEXT.type != CONCAT_EQUALS &&
                NEXT.type != PUSH && NEXT.type != LEFT_BRACKET && PREV.type != BACK && PREV.type != FRONT && PREV.type != SIZEOF)
                    ERROR("Stray identifier. If you wanted to access the variable's value, use $" + TOKEN.lexeme + ".");
                break;
            }
            case AT: { // change to prefix @ so params work
                token++;
                IDEN();
                std::string id = TOKEN.lexeme;
                token++;
                if (!CHECK(LEFT_PAREN)) ERROR("Expected '('.");
                token++;
                while (true) {
                    if (CHECK(RIGHT_PAREN)) break;
                    expression(1);
                    token++;
                    if (CHECK(COMMA)) {
                        token++;
                        if (CHECK(RIGHT_PAREN)) ERROR("Expected an expression.");
                    }
                }

                if (!CHECK(RIGHT_PAREN)) ERROR("Expected ')'.");

                vm.writeConstant(TOKEN.line, idLexeme(id));
                vm.writeOp(TOKEN.line, OP_CALL);
                break;
            }
            case BACK: {
                token++;
                expression(8);
                vm.writeOp(TOKEN.line, OP_BACK_LIST);
                break;
            }
            case FRONT: {
                token++;
                expression(8);
                vm.writeOp(TOKEN.line, OP_FRONT_LIST);
                break;
            }
            case SIZEOF: {
                token++;
                expression(8);
                vm.writeOp(TOKEN.line, OP_SIZEOF);
                break;
            }
            case ASCII: {
                token++;
                expression(8);
                vm.writeOp(TOKEN.line, OP_CONVERT_ASCII);
                break;
            }
            case RAND: {
                token++;
                expression(8);
                vm.writeOp(TOKEN.line, OP_RAND);
                break;
            }
            case FLOOR: {
                token++;
                expression(8);
                vm.writeOp(TOKEN.line, OP_FLOOR);
                break;
            }
            case _EOF: break;
            default: {
                ERROR("Expected an expression.");
            }
        }
        while (p <= getPrecedence(NEXT.type, NEXT.lexeme)) { // infix/postfix
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
                case PERCENT: { // /
                    token++;
                    expression(getPrecedence(PERCENT)+1);
                    vm.writeOp(TOKEN.line, OP_MODULO);
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
                case PLUS_EQUALS: {
                    uint8_t first = vm.opcode.back();

                    vm.writeOp(TOKEN.line, OP_RETRIEVE);
                    vm.writeOp(TOKEN.line, (int) first);
                    
                    token++;
                    expression(1);
                    vm.writeOp(TOKEN.line, OP_ADD);
                    vm.writeOp(TOKEN.line, OP_SET_VARIABLE);
                    break;
                }
                case MINUS_EQUALS: {
                    uint8_t first = vm.opcode.back();

                    vm.writeOp(TOKEN.line, OP_RETRIEVE);
                    vm.writeOp(TOKEN.line, (int) first);
                    
                    token++;
                    expression(1);
                    vm.writeOp(TOKEN.line, OP_SUB);
                    vm.writeOp(TOKEN.line, OP_SET_VARIABLE);
                    break;
                }
                case STAR_EQUALS: {
                    uint8_t first = vm.opcode.back();

                    vm.writeOp(TOKEN.line, OP_RETRIEVE);
                    vm.writeOp(TOKEN.line, (int) first);
                    
                    token++;
                    expression(1);
                    vm.writeOp(TOKEN.line, OP_MUL);
                    vm.writeOp(TOKEN.line, OP_SET_VARIABLE);
                    break;
                }
                case SLASH_EQUALS: {
                    uint8_t first = vm.opcode.back();

                    vm.writeOp(TOKEN.line, OP_RETRIEVE);
                    vm.writeOp(TOKEN.line, (int) first);
                    
                    token++;
                    expression(1);
                    vm.writeOp(TOKEN.line, OP_DIV);
                    vm.writeOp(TOKEN.line, OP_SET_VARIABLE);
                    break;
                }
                case CONCAT_EQUALS: {
                    uint8_t first = vm.opcode.back();

                    vm.writeOp(TOKEN.line, OP_RETRIEVE);
                    vm.writeOp(TOKEN.line, (int) first);
                    
                    token++;
                    expression(1);
                    vm.writeOp(TOKEN.line, OP_CONCATENATE);
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
                case AS: {
                    token++;
                    vm.writeOp(TOKEN.line, OP_CONVERT);
                    switch (TOKEN.type) {
                        case NUM:
                            vm.writeOp(TOKEN.line, 0);
                            break;
                        case STR:
                            vm.writeOp(TOKEN.line, 1);
                            break;
                        case _BOOL:
                            vm.writeOp(TOKEN.line, 2);
                            break;
                        case _VOID:
                            vm.writeOp(TOKEN.line, 3);
                            break;
                        case _LIST:
                            vm.writeOp(TOKEN.line, 4);
                            break;
                        default: ERROR("Expected either 'NUM', 'STR', 'BOOL', 'LIST', or 'VOID' as type specifier.");
                    }
                    break;
                }
                case AT_KEYWORD: {
                    token++;
                    expression(getPrecedence(AT_KEYWORD));
                    vm.writeOp(TOKEN.line, OP_AT);
                    break;
                }
                case IDENTIFIER: {
                    std::string id = TOKEN.lexeme;
                    int prec = getInfixOp(TOKEN.lexeme);
                    if (prec != -1) {
                        token++;
                        expression(prec+1);
                        vm.writeConstant(TOKEN.line, idLexeme(id));
                        vm.writeOp(TOKEN.line, OP_CALL);
                    } else ERROR("No operator by name " + id + ".");
                    break;
                }
                case PUSH: {
                    token++;
                    expression(getPrecedence(PUSH)+1);
                    vm.writeOp(TOKEN.line, OP_PUSH_LIST);
                    break;
                }
                case LEFT_BRACKET: {
                    token++;
                    expression(1);
                    token++;
                    if (!CHECK(RIGHT_BRACKET)) ERROR("Expected a ']'");
                    vm.writeOp(TOKEN.line, OP_INDEX_LIST);
                    break;
                }
                default: break;
            }
        }
    };

    auto expStatement = [&]() {
        expression(1);
        token++;
        SEMICOLON();
        vm.writeOp(TOKEN.line, OP_EMPTY_STACK); 
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
                ERROR("Expected an identifier.");
            }
            std::string id = TOKEN.lexeme;

            token++;
            if (CHECK(LEFT_BRACKET)) {
                token++;
                expression(1);
                token++;
                if (!CHECK(RIGHT_BRACKET)) ERROR("Expected a ']'");
                token++;
                Type expected_type = _EOF;
                if (CHECK(COLON)) {
                    token++;
                    if (!CHECK(NUM) && !CHECK(STR) && !CHECK(_VOID) && !CHECK(_BOOL) && !CHECK(_LIST)) ERROR("Expected a type specifier ('NUM', 'STR', 'BOOL', 'LIST', 'VOID').");
                    expected_type = TOKEN.type;
                    token++;
                }
                if (!CHECK(EQUAL)) ERROR("Expected '='.");
                token++;
                expression(1);
                token++;
                SEMICOLON();
                switch (expected_type) {
                    case NUM:
                        vm.writeOp(TOKEN.line, OP_REQUIRE_NUM);
                        break;
                    case STR:
                        vm.writeOp(TOKEN.line, OP_REQUIRE_STR);
                        break;
                    case _BOOL:
                        vm.writeOp(TOKEN.line, OP_REQUIRE_BOOL);
                        break;
                    case _VOID:
                        vm.writeOp(TOKEN.line, OP_REQUIRE_VOID);
                        break;
                    case _LIST:
                        vm.writeOp(TOKEN.line, OP_REQUIRE_LIST);
                        break;
                }
                vm.writeConstant(TOKEN.line, idLexeme(id));
                vm.writeOp(TOKEN.line, OP_DECL_LIST_INDEX);
            } else {
                Type expected = _EOF;
                if (CHECK(COLON)) {
                    token++;
                    if (!CHECK(NUM) && !CHECK(STR) && !CHECK(_VOID) && !CHECK(_BOOL) && !CHECK(_LIST)) ERROR("Expected a type specifier ('NUM', 'STR', 'BOOL', 'LIST', 'VOID').");
                    expected = TOKEN.type;
                } else token--;

                vm.writeConstant(TOKEN.line, idLexeme(id)); // note: this won't show up in debug if the lexeme
                                                                            // is >= 2 because of TRIM()
                token++;
                bool found_equal = false;
                if (CHECK(EQUAL)) {
                    found_equal = true;
                    token++;
                    expression(1);
                    token++;
                } else vm.writeConstant(TOKEN.line, nullValue());

                if (found_equal) {
                    switch (expected) {
                        case NUM:
                            vm.writeOp(TOKEN.line, OP_REQUIRE_NUM);
                            break;
                        case STR:
                            vm.writeOp(TOKEN.line, OP_REQUIRE_STR);
                            break;
                        case _BOOL:
                            vm.writeOp(TOKEN.line, OP_REQUIRE_BOOL);
                            break;
                        case _VOID:
                            vm.writeOp(TOKEN.line, OP_REQUIRE_VOID);
                            break;
                        case _LIST:
                            vm.writeOp(TOKEN.line, OP_REQUIRE_LIST);
                            break;
                    }
                }

                SEMICOLON();

                if (mut) vm.writeOp(TOKEN.line, OP_VARIABLE_MUT);
                else vm.writeOp(TOKEN.line, OP_VARIABLE);
            }
        } else if (CHECK(THROW)) {
            token++;
            expression(1);
            vm.writeOp(TOKEN.line, OP_THROW);
            token++;
            SEMICOLON();
        } else if (CHECK(SLEEP)) {
            token++;
            expression(1);
            vm.writeOp(TOKEN.line, OP_SLEEP);
            token++;
            SEMICOLON();
        } else if (CHECK(CONSOLE)) {
            token++;
            expression(1);
            vm.writeOp(TOKEN.line, OP_CONSOLE);
            token++;
            SEMICOLON();
        } else if (CHECK(POP)) {
            token++;
            IDEN();
            vm.writeConstant(TOKEN.line, idLexeme(TOKEN.lexeme));
            vm.writeOp(TOKEN.line, OP_POP_LIST);
            token++;
            SEMICOLON();
        } else if (CHECK(FUN)) {
            Function fn;
            token++;

            if (CHECK(AWARE)) {
                token++;
                fn.type = FN_AWARE;
            } else if (CHECK(BLIND)) {
                token++;
                fn.type = FN_BLIND;
            } else {
                fn.type = FN_NORMAL;
            }
            IDEN();
            std::string id = TOKEN.lexeme;
            vm.writeConstant(TOKEN.line, idLexeme(id));

            token++;
            if (TOKEN.type != LEFT_PAREN) ERROR("Expected a '('.");

            token++;

            while (true) {
                if (CHECK(IDENTIFIER)) {
                    fn.param_ids.push_back(TOKEN.lexeme);
                    token++;
                    if (!CHECK(COLON)) ERROR("Expected a type specifier ('ANY', 'NUM', 'STR', 'BOOL', 'LIST', 'VOID').\nCorrect method == fn foo(param: ANY)");
                    token++;
                    if (!CHECK(NUM) && !CHECK(STR) && !CHECK(_BOOL) && !CHECK(_VOID) && !CHECK(ANY) && !CHECK(_LIST)) ERROR("Expected a type specifier ('ANY', 'NUM', 'STR', 'BOOL', 'LIST', 'VOID').\nCorrect method == fn foo(param: ANY)");
                    fn.param_types.push_back(TOKEN.lexeme);
                    token++;
                    if (CHECK(COMMA)) {
                        token++;
                        IDEN();
                    }
                } else if (CHECK(RIGHT_PAREN)) break;
                else ERROR("Expected valid parameters.");
            }

            if (!CHECK(RIGHT_PAREN)) ERROR("Expected a ')'.");
            if (NEXT.type != LEFT_BRACE) ERROR("Expected a '{'.");

            vm.writeOp(TOKEN.line, OP_DECL_FN);
            vm.writeOp(TOKEN.line, vm.fn_pool.size());

            int nests = 0;
            std::vector<Token> function_body;
            while (true) {
                token++;

                if (CHECK(FUN)) ERROR("cbi does not support nested functions.");
                function_body.push_back(TOKEN);
                if (CHECK(LEFT_BRACE))
                    nests++;
                else if (CHECK(RIGHT_BRACE)) {
                    if (nests == 1) break;
                    nests--;
                }
            }

            Machine body_as_M = compile(function_body, success);
            if (!success) {
                success = false;
                std::cerr.setstate(std::ios_base::failbit);
                panicking = true;
                return;
            }

            fn.opcode = body_as_M.opcode;
            fn.lines = body_as_M.lines;
            fn.constants = body_as_M.constants;
            vm.fn_pool.push_back(fn);
        } else if (CHECK(PRINT)) { // printing
            token++;
            expression(1);
            token++;

            SEMICOLON();

            vm.writeOp(TOKEN.line, OP_PRINT_TOP);
        } else if (CHECK(RETURN)) {
            token++;
            expression(1);
            token++;
            SEMICOLON();
            vm.writeOp(TOKEN.line, OP_RETURN_TOP);
        } else if (CHECK(IF)) { // if statement
            token++;

            if (!CHECK(LEFT_PAREN)) ERROR("Expected '(' after if.");

            token++;
            expression(1);
            token++;

            if (!CHECK(RIGHT_PAREN)) ERROR("Expected ')' after if condition.");

            vm.writeOp(TOKEN.line, OP_JUMP_FALSE_IFv);
            int size = vm.opcode.size();
            vm.opcode.push_back(uint8_t());
            vm.lines.push_back(TOKEN.line);

            token++;
            if (!CHECK(LEFT_BRACE))
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
                if (!CHECK(LEFT_BRACE))
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

            if (!CHECK(LEFT_PAREN)) ERROR("Expected '(' after while.");
            int presize = vm.opcode.size(); // before the condition
            token++;
            expression(1);
            token++;
            if (!CHECK(RIGHT_PAREN)) ERROR("Expected ')' after while condition.");

            vm.writeOp(TOKEN.line, OP_JUMP_FALSE);
            int size = vm.opcode.size();
            vm.opcode.push_back(uint8_t());
            vm.lines.push_back(TOKEN.line);

            token++;
            if (!CHECK(LEFT_BRACE))
                declaration();
            else
                HANDLE_BLOCK();

            vm.writeOp(TOKEN.line, OP_JUMP); // jumping the beginning
            vm.writeOp(TOKEN.line, presize);

            vm.opcode[size] = vm.opcode.size(); // skipping past the bytecode

            vm.writeOp(TOKEN.line, OP_END_SCOPE);
        } else if (CHECK(INFIX)) {
            Function fn;
            fn.type = FN_BLIND;
            
            token++;

            IDEN();
            std::string id = TOKEN.lexeme;
            vm.writeConstant(TOKEN.line, idLexeme(id));

            token++;
            if (TOKEN.type != LEFT_PAREN) ERROR("Expected a '('.");

            token++;

            while (true) {
                if (CHECK(IDENTIFIER)) {
                    fn.param_ids.push_back(TOKEN.lexeme);
                    token++;
                    if (!CHECK(COLON)) ERROR("Expected a type specifier ('ANY', 'NUM', 'STR', 'BOOL', 'VOID').\nCorrect method == fn foo(param: ANY)");
                    token++;
                    if (!CHECK(NUM) && !CHECK(STR) && !CHECK(_BOOL) && !CHECK(_VOID) && !CHECK(ANY)) ERROR("Expected a type specifier ('ANY', 'NUM', 'STR', 'BOOL', 'VOID').\nCorrect method == fn foo(param: ANY)");
                    fn.param_types.push_back(TOKEN.lexeme);
                    token++;
                    if (CHECK(COMMA)) {
                        token++;
                        IDEN();
                    }
                } else if (CHECK(RIGHT_PAREN)) break;
                else ERROR("Expected valid parameters.");
            }
            if (fn.param_ids.size() != 2) ERROR("Expected two params.");

            if (!CHECK(RIGHT_PAREN)) ERROR("Expected a ')'.");
            token++;
            if (!CHECK(PRECEDENCE)) ERROR("Expected 'precedence'.");
            token++;
            if (!CHECK(NUMBER)) ERROR("Expected a number literal.");
            environment.custom_infix_ops[id] = std::stoi(TOKEN.lexeme);
            
            if (NEXT.type != LEFT_BRACE) {
                token++;
                ERROR("Expected a '{'.");
            }

            vm.writeOp(TOKEN.line, OP_DECL_FN);
            vm.writeOp(TOKEN.line, vm.fn_pool.size());

            int nests = 0;
            std::vector<Token> function_body;
            while (true) {
                token++;

                if (CHECK(FUN)) ERROR("cbi does not support nested functions.");
                function_body.push_back(TOKEN);
                if (CHECK(LEFT_BRACE))
                    nests++;
                else if (CHECK(RIGHT_BRACE)) {
                    if (nests == 1) break;
                    nests--;
                }
            }

            Machine body_as_M = compile(function_body, success);
            if (!success) {
                success = false;
                std::cerr.setstate(std::ios_base::failbit);
                panicking = true;
                return;
            }

            fn.opcode = body_as_M.opcode;
            fn.lines = body_as_M.lines;
            fn.constants = body_as_M.constants;
            vm.fn_pool.push_back(fn);
        } else if (CHECK(PREFIX)) {
            Function fn;
            fn.type = FN_BLIND;
            
            token++;

            IDEN();
            std::string id = TOKEN.lexeme;
            vm.writeConstant(TOKEN.line, idLexeme(id));

            token++;
            if (TOKEN.type != LEFT_PAREN) ERROR("Expected a '('.");

            token++;

            while (true) {
                if (CHECK(IDENTIFIER)) {
                    fn.param_ids.push_back(TOKEN.lexeme);
                    token++;
                    if (!CHECK(COLON)) ERROR("Expected a type specifier ('ANY', 'NUM', 'STR', 'BOOL', 'VOID').\nCorrect method == fn foo(param: ANY)");
                    token++;
                    if (!CHECK(NUM) && !CHECK(STR) && !CHECK(_BOOL) && !CHECK(_VOID) && !CHECK(ANY)) ERROR("Expected a type specifier ('ANY', 'NUM', 'STR', 'BOOL', 'VOID').\nCorrect method == fn foo(param: ANY)");
                    fn.param_types.push_back(TOKEN.lexeme);
                    token++;
                    if (CHECK(COMMA)) {
                        token++;
                        IDEN();
                    }
                } else if (CHECK(RIGHT_PAREN)) break;
                else ERROR("Expected valid parameters.");
            }
            if (fn.param_ids.size() != 1) ERROR("Expected one params.");

            if (!CHECK(RIGHT_PAREN)) ERROR("Expected a ')'.");
            token++;
            if (!CHECK(PRECEDENCE)) ERROR("Expected 'precedence'.");
            token++;
            if (!CHECK(NUMBER)) ERROR("Expected a number literal.");
            environment.custom_prefix_ops[id] = std::stoi(TOKEN.lexeme);
            
            if (NEXT.type != LEFT_BRACE) {
                token++;
                ERROR("Expected a '{'.");
            }

            vm.writeOp(TOKEN.line, OP_DECL_FN);
            vm.writeOp(TOKEN.line, vm.fn_pool.size());

            int nests = 0;
            std::vector<Token> function_body;
            while (true) {
                token++;

                if (CHECK(FUN)) ERROR("cbi does not support nested functions.");
                function_body.push_back(TOKEN);
                if (CHECK(LEFT_BRACE))
                    nests++;
                else if (CHECK(RIGHT_BRACE)) {
                    if (nests == 1) break;
                    nests--;
                }
            }

            Machine body_as_M = compile(function_body, success);
            if (!success) {
                success = false;
                std::cerr.setstate(std::ios_base::failbit);
                panicking = true;
                return;
            }

            fn.opcode = body_as_M.opcode;
            fn.lines = body_as_M.lines;
            fn.constants = body_as_M.constants;
            vm.fn_pool.push_back(fn);
        } else if (CHECK(BREAK)) {
            vm.writeOp(TOKEN.line, OP_BREAK);
            token++;
            SEMICOLON();
        } else if (CHECK(DIS_C)) {
            token++;
            SEMICOLON();
            vm.writeOp(TOKEN.line, OP_DISASSEMBLE_CONSTANTS);
        } else if (CHECK(DIS_SC)) {
            token++;
            SEMICOLON();
            vm.writeOp(TOKEN.line, OP_DISASSEMBLE_SCOPES);
        } else if (CHECK(DIS_ST)) {
            token++;
            SEMICOLON();
            vm.writeOp(TOKEN.line, OP_DISASSEMBLE_STACK);
        } else if (CHECK(GETS)) {
            token++;
            IDEN();
            std::string id = TOKEN.lexeme;
            token++;
            SEMICOLON();
            vm.writeConstant(TOKEN.line, idLexeme(id));
            vm.writeOp(TOKEN.line, OP_GETS);
        } else if (CHECK(GETCH)) {
            token++;
            IDEN();
            std::string id = TOKEN.lexeme;
            token++;
            SEMICOLON();
            vm.writeConstant(TOKEN.line, idLexeme(id));
            vm.writeOp(TOKEN.line, OP_GETCH);
        } else if (CHECK(LEFT_BRACE)) { // block
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
            std::cerr.clear();
            panicking = false;
        }
    }
    vm.writeOp(-1, OP_END_SCOPE);

    #undef TOKEN
    #undef PREV
    #undef NEXT
    #undef ERROR
    #undef CHECK

    return vm;
}