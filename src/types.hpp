#ifndef _types_h
#define _types_h

#include <string>
#include <vector>
#include <stack>
#include <map>

enum Command {
    OP_BEGIN, OP_PRINT_TOP, OP_CONSTANT,

    //prefix operators
    OP_NEGATE, OP_NOT,

    //postfix
    OP_INC, OP_DEC,

    //infix operators
    OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_CONCATENATE, OP_AND, OP_OR,

    OP_EQUALITY, OP_LESS, OP_GREATER, OP_LESS_EQ, OP_GREATER_EQ, OP_NOT_EQ,
    OP_JUMP_FALSE, OP_IF, OP_ELSE, OP_POP_TOP, OP_VARIABLE, OP_SET_VARIABLE, OP_IMUT, OP_RETRIEVE, OP_VARIABLE_MUT,
    OP_BEGIN_SCOPE, OP_END_SCOPE, OP_JUMP
};

enum RunType {
    NORMAL, DEBUG
};

enum ErrorCode {
    EXIT_OK, EXIT_RT, EXIT_CT
};

enum Tag {
    TYPE_DOUBLE, TYPE_BOOL, TYPE_NULL, TYPE_STRING, TYPE_ID_LEXEME
};

struct Value {
    Tag type;
    std::string string;
    union {
        double number;
        bool boolean;
    } storage;
};

struct Scope {
    std::map<std::string, Value> variables;
    std::vector<std::string> mutables;
};

Value numberValue(double num);

Value stringValue(std::string str);

Value idLexeme(std::string str);

Value boolValue(bool boolean);

Value nullValue();

std::string getPrintable(Value value);

#define IS_BOOL(value) ((value).type == TYPE_BOOL)

#define IS_NUM(value) ((value).type == TYPE_DOUBLE)

#define IS_STRING(value) ((value).type == TYPE_STRING)

#define IS_ID(value) ((value).type == TYPE_ID_LEXEME)

#define TRIM(string) string.substr(1, string.length()-2)

#endif