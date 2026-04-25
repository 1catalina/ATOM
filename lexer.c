#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "lexer.h"
#include "utils.h"

Token* tokens;   // single linked list of tokens
Token* lastTk;   // the last token in list
int line = 1;    // the current line in the input file

// adds a token to the end of the tokens list and returns it
// sets its code and line
Token* addTk(int code) {
    Token* tk = safeAlloc(sizeof(Token));
    tk->code = code;
    tk->line = line;
    tk->next = NULL;
    if (lastTk) {
        lastTk->next = tk;
    }
    else {
        tokens = tk;
    }
    lastTk = tk;
    return tk;
}

char* extract(const char* begin, const char* end) {
    size_t len = end - begin;
    char* res = (char*)safeAlloc(len + 1);
    memcpy(res, begin, len);
    res[len] = '\0';
    return res;
}

Token* tokenize(const char* pch) {
    const char* start;
    Token* tk;
    for (;;) {
        switch (*pch) {
        case ' ': case '\t':
            pch++;
            break;
        case '\r':
            // handles different kinds of newlines (Windows: \r\n, Linux: \n, MacOS/OS X: \r or \n)
            if (pch[1] == '\n') pch++;
            // fallthrough to \n
        case '\n':
            line++;
            pch++;
            break;
        case '\0':
            addTk(END);
            return tokens;
        case ',':
            addTk(COMMA); pch++;
            break;
        case ';':
            addTk(SEMICOLON); pch++;
            break;
        case '-':
            addTk(SUB); pch++;
            break;
        case '+':
            addTk(ADD); pch++;
            break;
        case '*':
            addTk(MUL); pch++;
            break;
        case '/':
            if (pch[1] == '/') {
                // skip single-line comment
                pch += 2;
                while (*pch != '\n' && *pch != '\r' && *pch != '\0' && *pch) pch++;
            }
            else {
                addTk(DIV); pch++;
            }
            break;
        case '.':
            addTk(DOT); pch++;
            break;
        case '(':
            addTk(LPAR); pch++;
            break;
        case ')':
            addTk(RPAR); pch++;
            break;
        case '[':
            addTk(LBRACKET); pch++;
            break;
        case ']':
            addTk(RBRACKET); pch++;
            break;
        case '{':
            addTk(LACC); pch++;
            break;
        case '}':
            addTk(RACC); pch++;
            break;
        case '=':
            if (pch[1] == '=') {
                addTk(EQUAL); pch += 2;
            }
            else {
                addTk(ASSIGN); pch++;
            }
            break;
        case '<':
            if (pch[1] == '=') {
                addTk(LESSEQ); pch += 2;
            }
            else {
                addTk(LESS); pch++;
            }
            break;
        case '>':
            if (pch[1] == '=') {
                addTk(GREATEREQ); pch += 2;
            }
            else {
                addTk(GREATER); pch++;
            }
            break;
        case '|':
            if (pch[1] == '|') {
                addTk(OR); pch += 2;
            }
            else {
                err("invalid char: %c (%d)", *pch, *pch);
            }
            break;
        case '&':
            if (pch[1] == '&') {
                addTk(AND); pch += 2;
            }
            else {
                err("invalid char: %c (%d)", *pch, *pch);
            }
            break;
        case '!':
            if (pch[1] == '=') {
                addTk(NOTEQ); pch += 2;
            }
            else {
                addTk(NOT); pch++;
            }
            break;

            // CHAR: ['] ( [^'\\] | [\\] [abfnrtv\\'\"0] ) [']
        case '\'': {
            pch++; // skip opening quote
            char cval;
            if (*pch == '\\') {
                pch++; // skip backslash
                switch (*pch) {
                case 'a':  cval = '\a'; break;
                case 'b':  cval = '\b'; break;
                case 'f':  cval = '\f'; break;
                case 'n':  cval = '\n'; break;
                case 'r':  cval = '\r'; break;
                case 't':  cval = '\t'; break;
                case 'v':  cval = '\v'; break;
                case '\\': cval = '\\'; break;
                case '\'': cval = '\''; break;
                case '"':  cval = '"';  break;
                case '0':  cval = '\0'; break;
                default:
                    err("invalid escape sequence in char constant: \\%c", *pch);
                }
                pch++;
            }
            else if (*pch != '\'' && *pch != '\0') {
                cval = *pch;
                pch++;
            }
            else {
                err("invalid char constant");
            }
            if (*pch != '\'') err("missing closing ' in char constant");
            pch++; // skip closing quote
            tk = addTk(CHAR);
            tk->c = cval;
            break;
        }

                 // STRING: ["] ( [^"\\] | [\\] [abfnrtv\\'\"0] )* ["]
        case '"': {
            pch++; // skip opening quote
            // string value -> dynamic buffer
            size_t capacity = 64;
            size_t len = 0;
            char* buf = (char*)safeAlloc(capacity);
            while (*pch != '"' && *pch != '\0') {
                char ch;
                if (*pch == '\\') {
                    pch++;
                    switch (*pch) {
                    case 'a':  ch = '\a'; break;
                    case 'b':  ch = '\b'; break;
                    case 'f':  ch = '\f'; break;
                    case 'n':  ch = '\n'; break;
                    case 'r':  ch = '\r'; break;
                    case 't':  ch = '\t'; break;
                    case 'v':  ch = '\v'; break;
                    case '\\': ch = '\\'; break;
                    case '\'': ch = '\''; break;
                    case '"':  ch = '"'; break;
                    case '0':  ch = '\0'; break;
                    default:
                        err("invalid escape sequence in string: \\%c", *pch);
                    }
                    pch++;
                }
                else {
                    ch = *pch++;
                }
                // grow buffer if needed
                if (len + 1 >= capacity) {
                    capacity *= 2;
                    char* newbuf = (char*)safeAlloc(capacity);
                    memcpy(newbuf, buf, len);
                    free(buf);
                    buf = newbuf;
                }
                buf[len++] = ch;
            }
            if (*pch != '"') err("missing closing \" in string constant");
            pch++; // skip closing quote
            buf[len] = '\0';
            tk = addTk(STRING);
            tk->text = buf;
            break;
        }

        default:
            if (isalpha(*pch) || *pch == '_') {
                // identifier or keyword
                for (start = pch++; isalnum(*pch) || *pch == '_'; pch++) {}
                char* text = extract(start, pch);
                if (strcmp(text, "char") == 0) { addTk(TYPE_CHAR); free(text); }
                else if (strcmp(text, "double") == 0) { addTk(TYPE_DOUBLE); free(text); }
                else if (strcmp(text, "int") == 0) { addTk(TYPE_INT); free(text); }
                else if (strcmp(text, "else") == 0) { addTk(ELSE); free(text); }
                else if (strcmp(text, "if") == 0) { addTk(IF); free(text); }
                else if (strcmp(text, "return") == 0) { addTk(RETURN); free(text); }
                else if (strcmp(text, "struct") == 0) { addTk(STRUCT); free(text); }
                else if (strcmp(text, "void") == 0) { addTk(VOID); free(text); }
                else if (strcmp(text, "while") == 0) { addTk(WHILE); free(text); }
                else {
                    tk = addTk(ID);
                    tk->text = text;
                }
            }
            // DOUBLE: [0-9]+ ( '.' [0-9]+ ([eE][+-]?[0-9]+)? | ([eE][+-]?[0-9]+) )
            // INT:    [0-9]+
            else if (isdigit(*pch)) {
                start = pch;
                while (isdigit(*pch)) pch++;

                int isDouble = 0;

                // fraction
                if (*pch == '.')
                    if (isdigit(pch[1]))
                    {
                        isDouble = 1;
                        pch++; // consume
                        while (isdigit(*pch)) pch++;
                    }
                    else {
                        if (!isdigit(*pch))
                            err("expected digits after point");
                    }

                // exponent
                if (*pch == 'e' || *pch == 'E') {
                    isDouble = 1;
                    pch++; // consume 
                    if (*pch == '+' || *pch == '-') pch++;
                    if (!isdigit(*pch)) err("expected digits after exponent");
                    while (isdigit(*pch)) pch++;
                }

                char* text = extract(start, pch);
                if (isDouble) {
                    tk = addTk(DOUBLE);
                    tk->d = atof(text);
                }
                else {
                    tk = addTk(INT);
                    tk->i = atoi(text);
                }
                free(text);
            }
            else {
                err("invalid char: %c (%d)", *pch, *pch);
            }
            break;
        }
    }
}

void showTokens(const Token* tokens) {
    for (const Token* tk = tokens; tk; tk = tk->next) {
        printf("%d\n", tk->code);
    }
}

void printTokens(const Token* tokens) {
    for (const Token* tk = tokens; tk; tk = tk->next) {
        switch (tk->code) {
        case ID:         printf("%d\tID:%s\n", tk->line, tk->text); break;
        case TYPE_CHAR:  printf("%d\tTYPE_CHAR\n", tk->line); break;
        case TYPE_DOUBLE:printf("%d\tTYPE_DOUBLE\n", tk->line); break;
        case TYPE_INT:   printf("%d\tTYPE_INT\n", tk->line); break;
        case STRUCT:     printf("%d\tSTRUCT\n", tk->line); break;
        case VOID:       printf("%d\tVOID\n", tk->line); break;
        case IF:         printf("%d\tIF\n", tk->line); break;
        case ELSE:       printf("%d\tELSE\n", tk->line); break;
        case WHILE:      printf("%d\tWHILE\n", tk->line); break;
        case RETURN:     printf("%d\tRETURN\n", tk->line); break;
        case COMMA:      printf("%d\tCOMMA\n", tk->line); break;
        case SEMICOLON:  printf("%d\tSEMICOLON\n", tk->line); break;
        case LPAR:       printf("%d\tLPAR\n", tk->line); break;
        case RPAR:       printf("%d\tRPAR\n", tk->line); break;
        case LBRACKET:   printf("%d\tLBRACKET\n", tk->line); break;
        case RBRACKET:   printf("%d\tRBRACKET\n", tk->line); break;
        case LACC:       printf("%d\tLACC\n", tk->line); break;
        case RACC:       printf("%d\tRACC\n", tk->line); break;
        case ADD:        printf("%d\tADD\n", tk->line); break;
        case SUB:        printf("%d\tSUB\n", tk->line); break;
        case MUL:        printf("%d\tMUL\n", tk->line); break;
        case DIV:        printf("%d\tDIV\n", tk->line); break;
        case DOT:        printf("%d\tDOT\n", tk->line); break;
        case AND:        printf("%d\tAND\n", tk->line); break;
        case OR:         printf("%d\tOR\n", tk->line); break;
        case NOT:        printf("%d\tNOT\n", tk->line); break;
        case ASSIGN:     printf("%d\tASSIGN\n", tk->line); break;
        case EQUAL:      printf("%d\tEQUAL\n", tk->line); break;
        case NOTEQ:      printf("%d\tNOTEQ\n", tk->line); break;
        case LESS:       printf("%d\tLESS\n", tk->line); break;
        case LESSEQ:     printf("%d\tLESSEQ\n", tk->line); break;
        case GREATER:    printf("%d\tGREATER\n", tk->line); break;
        case GREATEREQ:  printf("%d\tGREATEREQ\n", tk->line); break;
        case INT:        printf("%d\tINT:%d\n", tk->line, tk->i); break;
        case DOUBLE:     printf("%d\tDOUBLE:%g\n", tk->line, tk->d); break;
        case CHAR:       printf("%d\tCHAR:%c\n", tk->line, tk->c); break;
        case STRING:     printf("%d\tSTRING:%s\n", tk->line, tk->text); break;
        case END:        printf("%d\tEND\n", tk->line); break;
        default:         printf("%d\tUNKNOWN\n", tk->line); break;
        }
    }
}

void writeTokens(const Token* tokens, FILE* fout) {
    for (const Token* tk = tokens; tk; tk = tk->next) {
        switch (tk->code) {
        case ID:         fprintf(fout, "%d\tID:%s\n", tk->line, tk->text); break;
        case TYPE_CHAR:  fprintf(fout, "%d\tTYPE_CHAR\n", tk->line); break;
        case TYPE_DOUBLE:fprintf(fout, "%d\tTYPE_DOUBLE\n", tk->line); break;
        case TYPE_INT:   fprintf(fout, "%d\tTYPE_INT\n", tk->line); break;
        case STRUCT:     fprintf(fout, "%d\tSTRUCT\n", tk->line); break;
        case VOID:       fprintf(fout, "%d\tVOID\n", tk->line); break;
        case IF:         fprintf(fout, "%d\tIF\n", tk->line); break;
        case ELSE:       fprintf(fout, "%d\tELSE\n", tk->line); break;
        case WHILE:      fprintf(fout, "%d\tWHILE\n", tk->line); break;
        case RETURN:     fprintf(fout, "%d\tRETURN\n", tk->line); break;
        case COMMA:      fprintf(fout, "%d\tCOMMA\n", tk->line); break;
        case SEMICOLON:  fprintf(fout, "%d\tSEMICOLON\n", tk->line); break;
        case LPAR:       fprintf(fout, "%d\tLPAR\n", tk->line); break;
        case RPAR:       fprintf(fout, "%d\tRPAR\n", tk->line); break;
        case LBRACKET:   fprintf(fout, "%d\tLBRACKET\n", tk->line); break;
        case RBRACKET:   fprintf(fout, "%d\tRBRACKET\n", tk->line); break;
        case LACC:       fprintf(fout, "%d\tLACC\n", tk->line); break;
        case RACC:       fprintf(fout, "%d\tRACC\n", tk->line); break;
        case ADD:        fprintf(fout, "%d\tADD\n", tk->line); break;
        case SUB:        fprintf(fout, "%d\tSUB\n", tk->line); break;
        case MUL:        fprintf(fout, "%d\tMUL\n", tk->line); break;
        case DIV:        fprintf(fout, "%d\tDIV\n", tk->line); break;
        case DOT:        fprintf(fout, "%d\tDOT\n", tk->line); break;
        case AND:        fprintf(fout, "%d\tAND\n", tk->line); break;
        case OR:         fprintf(fout, "%d\tOR\n", tk->line); break;
        case NOT:        fprintf(fout, "%d\tNOT\n", tk->line); break;
        case ASSIGN:     fprintf(fout, "%d\tASSIGN\n", tk->line); break;
        case EQUAL:      fprintf(fout, "%d\tEQUAL\n", tk->line); break;
        case NOTEQ:      fprintf(fout, "%d\tNOTEQ\n", tk->line); break;
        case LESS:       fprintf(fout, "%d\tLESS\n", tk->line); break;
        case LESSEQ:     fprintf(fout, "%d\tLESSEQ\n", tk->line); break;
        case GREATER:    fprintf(fout, "%d\tGREATER\n", tk->line); break;
        case GREATEREQ:  fprintf(fout, "%d\tGREATEREQ\n", tk->line); break;
        case INT:        fprintf(fout, "%d\tINT:%d\n", tk->line, tk->i); break;
        case DOUBLE:     fprintf(fout, "%d\tDOUBLE:%.4f\n", tk->line, tk->d); break;
        case CHAR:       fprintf(fout, "%d\tCHAR:%c\n", tk->line, tk->c); break;
        case STRING:     fprintf(fout, "%d\tSTRING:%s\n", tk->line, tk->text); break;
        case END:        fprintf(fout, "%d\tEND\n", tk->line); break;
        default:         fprintf(fout, "%d\tUNKNOWN\n", tk->line); break;
        }
    }
}