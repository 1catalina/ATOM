#pragma once

enum {
    ID,
    // keywords
    TYPE_CHAR, TYPE_DOUBLE, TYPE_INT, ELSE, IF, RETURN, STRUCT, VOID, WHILE,
    // constants
    INT, DOUBLE, CHAR, STRING,
    // delimiters
    COMMA, SEMICOLON, LPAR, RPAR, LBRACKET, RBRACKET, LACC, RACC, END,
    // operators
    ADD, SUB, MUL, DIV, DOT, AND, OR, NOT, ASSIGN, EQUAL, NOTEQ,
    LESS, LESSEQ, GREATER, GREATEREQ
};

typedef struct Token {
    int code;       // ID, TYPE_CHAR etc
    int line;       // input file line
    union { //values for ID, STRING, INT, CHAR, DOUBLE
        char* text;
        int i;
        char c;
        double d;
    };
    struct Token* next;
} Token;

Token* tokenize(const char* pch);
void showTokens(const Token* tokens);
