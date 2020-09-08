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

    //infix operators
    OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_CONCATENATE, OP_AND, OP_OR,

    OP_EQUALITY, OP_LESS, OP_GREATER, OP_LESS_EQ, OP_GREATER_EQ, OP_NOT_EQ,

    OP_JUMP_FALSE, OP_JUMP_FALSE_IFv,
    
    OP_POP_TOP,

    OP_VARIABLE, OP_SET_VARIABLE, OP_IMUT, OP_RETRIEVE, OP_VARIABLE_MUT,

    OP_BEGIN_SCOPE, OP_END_SCOPE,

    OP_JUMP, OP_BREAK,

    OP_CALL, OP_DECL_FN,
    
    OP_EMPTY_STACK,

    OP_RETURN_TOP,
    
    OP_DISASSEMBLE_STACK, OP_DISASSEMBLE_SCOPES, OP_DISASSEMBLE_CONSTANTS,
    
    OP_GETS, OP_GETCH,
    
    OP_CONVERT, OP_REQUIRE_NUM, OP_REQUIRE_STR, OP_REQUIRE_BOOL, OP_REQUIRE_VOID, OP_REQUIRE_LIST,
    
    OP_AT, OP_DECL_LIST, OP_PUSH_LIST,
    
    OP_POP_LIST, OP_BACK_LIST, OP_FRONT_LIST, OP_INDEX_LIST, OP_SIZEOF, OP_DECL_LIST_INDEX,
    
    OP_CONVERT_ASCII, OP_THROW, OP_CONSOLE, OP_SLEEP, OP_RAND, OP_FLOOR
};

enum Tag {
    TYPE_OK, TYPE_RT_ERROR, TYPE_DOUBLE, TYPE_BOOL, TYPE_NULL, TYPE_STRING, TYPE_ID_LEXEME, TYPE_LIST
};

struct Value {
    Tag type;
    std::string string;
    std::vector<Value *> list;
    union {
        double number;
        bool boolean;
    } storage;
};

template <typename T>
void delete_list(std::vector<T*> &list) {
    for (auto it = list.begin(); it < list.end(); it++)
        delete *it;
}

struct Scope {
    std::map<std::string, Value> variables;
    std::vector<std::string> mutables;
};

enum FunctionType {
  FN_AWARE,
  FN_BLIND,
  FN_NORMAL
};

struct Function {
    FunctionType type;
    std::vector<uint8_t> opcode;
    std::vector<int> lines;
    std::vector<Value> constants;
    std::vector<Scope> scopes;
    std::vector<std::string> param_ids;
    std::vector<std::string> param_types;
};

struct Struct {
    Scope scope;
    std::map<std::string, Function> fns;
    std::vector<Function> fn_pool;
};

std::string shorten(std::string str);

Value numberValue(double num);

Value stringValue(std::string str);

Value idLexeme(std::string str);

Value boolValue(bool boolean);

Value nullValue();

Value listValue();

Value listValue(Value v);

Value exitRT();

Value exitOK();

std::string getPrintable(Value value);

#define IS_BOOL(value) ((value).type == TYPE_BOOL)

#define IS_NUM(value) ((value).type == TYPE_DOUBLE)

#define IS_STRING(value) ((value).type == TYPE_STRING)

#define IS_ID(value) ((value).type == TYPE_ID_LEXEME)

#define IS_LIST(value) ((value).type == TYPE_LIST)

#define IS_NULL(value) ((value).type == TYPE_NULL)

#define TRIM(string) string.substr(1, string.length()-2)

#endif