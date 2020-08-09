#ifndef _types_h
#define _types_h

enum Command {
    OP_BLANK, OP_PRINT_TOP, OP_CONSTANT,

    //postfix operators
    OP_NEGATE, OP_NOT,

    //infix operators
    OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_CONCATENATE,

    OP_EQUALITY, OP_LESS, OP_GREATER, OP_LESS_EQ, OP_GREATER_EQ, OP_NOT_EQ
};

enum RunType {
    NORMAL, DEBUG
};

enum ErrorCode {
    EXIT_OK, EXIT_RT, EXIT_CT
};

enum Tag {
    TYPE_DOUBLE, TYPE_BOOL, TYPE_NULL, TYPE_STRING
};

struct Value {
    Tag type;
    union StorageUnit {
        double number;
        bool boolean;
        char* string;
    } storage;
};

/*enum ObjTag {
    OBJ_STRING
};

struct Object {

};*/

Value numberValue(double num);

Value stringValue(char* str);

Value boolValue(bool boolean);

Value nullValue();

std::string getPrintable(Value value);

#define IS_BOOL(value) ((value).type == TYPE_BOOL)

#define IS_NUM(value) ((value).type == TYPE_DOUBLE)

#define IS_STRING(value) ((value).type == TYPE_STRING)

#endif