#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

#include "lexer.h"
#include "utils.h"
#include "parser.h"


Token* iTk;         // the iterator in the tokens list
Token* consumedTk;  // the last consumed token

void tkerr(const char* fmt, ...) {
    fprintf(stderr, "error in line %d: ", iTk->line);
    va_list va;
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

bool consume(int code) {
    if (iTk->code == code) {
        consumedTk = iTk;
        iTk = iTk->next;
        return true;
    }
    return false;
}


// typeBase: TYPE_INT | TYPE_DOUBLE | TYPE_CHAR | STRUCT ID
bool typeBase() {
    if (consume(TYPE_INT)) return true;
    if (consume(TYPE_DOUBLE)) return true;
    if (consume(TYPE_CHAR)) return true;
    if (consume(STRUCT)) {
        if (consume(ID)) return true;
        tkerr("expected identifier after 'struct'");
    }
    return false;
}

// arrayDecl: LBRACKET INT? RBRACKET
bool arrayDecl() {
    Token* start = iTk;
    if (consume(LBRACKET)) {
        consume(INT); // optional
        if (consume(RBRACKET)) return true;
        tkerr("missing ] in array declaration");
    }
    iTk = start;
    return false;
}

// varDef: typeBase ID arrayDecl? SEMICOLON
bool varDef() {
    Token* start = iTk;
    if (typeBase()) {
        if (consume(ID)) {
            if (arrayDecl()) {}
            if (consume(SEMICOLON)) return true;
            tkerr("missing ; after variable definition");
        }
        // typeBase was consumed but no ID - restore
        iTk = start;
        return false;
    }
    return false;
}

// structDef: STRUCT ID LACC varDef* RACC SEMICOLON
bool structDef() {
    Token* start = iTk;
    if (consume(STRUCT)) {
        if (consume(ID)) {
            if (consume(LACC)) {
                while (varDef()) {}
                if (consume(RACC)) {
                    if (consume(SEMICOLON)) return true;
                    tkerr("missing ; after struct definition");
                }
                tkerr("missing } in struct definition");
            }
        }
        iTk = start;
        return false;
    }
    return false;
}

// fnParam: typeBase ID arrayDecl?
bool fnParam() {
    Token* start = iTk;
    if (typeBase()) {
        if (consume(ID)) {
            if (arrayDecl()) {}
            return true;
        }
        tkerr("missing parameter name");
    }
    iTk = start;
    return false;
}

// fnDef: ( typeBase | VOID ) ID LPAR ( fnParam ( COMMA fnParam )* )? RPAR stmCompound
bool fnDef() {
    Token* start = iTk;
    bool hasType = false;
    if (typeBase()) {
        hasType = true;
    }
    else if (consume(VOID)) {
        hasType = true;
    }
    if (hasType) {
        if (consume(ID)) {
            if (consume(LPAR)) {
                if (fnParam()) {
                    while (consume(COMMA)) {
                        if (!fnParam()) tkerr("missing parameter after ,");
                    }
                }
                if (consume(RPAR)) {
                    if (stmCompound()) return true;
                    tkerr("missing function body");
                }
                tkerr("missing ) in function definition");
            }
            // Had type + ID but no LPAR => could be varDef, restore
            iTk = start;
            return false;
        }
        iTk = start;
        return false;
    }
    return false;
}

// stmCompound: LACC ( varDef | stm )* RACC
bool stmCompound() {
    Token* start = iTk;
    if (consume(LACC)) {
        for (;;) {
            if (varDef()) {}
            else if (stm()) {}
            else break;
        }
        if (consume(RACC)) return true;
        tkerr("missing } in compound statement");
    }
    iTk = start;
    return false;
}

// stm: stmCompound
//    | IF LPAR expr RPAR stm ( ELSE stm )?
//    | WHILE LPAR expr RPAR stm
//    | RETURN expr? SEMICOLON
//    | expr? SEMICOLON
bool stm() {
    Token* start = iTk;

    if (stmCompound()) return true;

    if (consume(IF)) {
        if (consume(LPAR)) {
            if (expr()) {
                if (consume(RPAR)) {
                    if (stm()) {
                        if (consume(ELSE)) {
                            if (!stm()) tkerr("missing statement after else");
                        }
                        return true;
                    }
                    tkerr("missing statement after if");
                }
                tkerr("missing ) after if condition");
            }
            tkerr("invalid condition in if or missing )");
        }
        tkerr("missing ( after if");
    }

    if (consume(WHILE)) {
        if (consume(LPAR)) {
            if (expr()) {
                if (consume(RPAR)) {
                    if (stm()) return true;
                    tkerr("missing statement after while");
                }
                tkerr("missing ) after while condition");
            }
            tkerr("invalid condition in while or missing )");
        }
        tkerr("missing ( after while");
    }

    if (consume(RETURN)) {
        expr(); // optional
        if (consume(SEMICOLON)) return true;
        tkerr("missing ; after return");
    }

    // expr? SEMICOLON
    expr(); // optional
    if (consume(SEMICOLON)) return true;

    iTk = start;
    return false;
}

// expr: exprAssign
bool expr() {
    return exprAssign();
}

// exprAssign: exprUnary ASSIGN exprAssign | exprOr
bool exprAssign() {
    Token* start = iTk;
    if (exprUnary()) {
        if (consume(ASSIGN)) {
            if (exprAssign()) return true;
            tkerr("missing expression after =");
        }
        iTk = start;
    }
    return exprOr();
}

// exprOr: exprAnd exprOrPrim
// exprOrPrim: OR exprAnd exprOrPrim | epsilon
bool exprOrPrim() {
    if (consume(OR)) {
        if (exprAnd()) {
            if (exprOrPrim()) return true;
            tkerr("missing expression after ||");
        }
        tkerr("missing expression after ||");
    }
    return true; 
}

bool exprOr() {
    if (exprAnd()) {
        return exprOrPrim();
    }
    return false;
}

// exprAnd: exprEq exprAndPrim
// exprAndPrim: AND exprEq exprAndPrim | epsilon
bool exprAndPrim() {
    if (consume(AND)) {
        if (exprEq()) {
            if (exprAndPrim()) return true;
            tkerr("missing expression after &&");
        }
        tkerr("missing expression after &&");
    }
    return true;
}

bool exprAnd() {
    if (exprEq()) {
        return exprAndPrim();
    }
    return false;
}

// exprEq: exprRel exprEqPrim
// exprEqPrim: ( EQUAL | NOTEQ ) exprRel exprEqPrim | epsilon
bool exprEqPrim() {
    if (consume(EQUAL) || consume(NOTEQ)) {
        if (exprRel()) {
            return exprEqPrim();
        }
        tkerr("missing expression after == or !=");
    }
    return true; // epsilon
}

bool exprEq() {
    if (exprRel()) {
        return exprEqPrim();
    }
    return false;
}

// exprRel: exprAdd exprRelPrim
// exprRelPrim: ( LESS | LESSEQ | GREATER | GREATEREQ ) exprAdd exprRelPrim | epsilon
bool exprRelPrim() {
    if (consume(LESS) || consume(LESSEQ) || consume(GREATER) || consume(GREATEREQ)) {
        if (exprAdd()) {
            return exprRelPrim();
        }
        tkerr("missing expression after relational operator");
    }
    return true; 
}

bool exprRel() {
    if (exprAdd()) {
        return exprRelPrim();
    }
    return false;
}

// exprAdd: exprMul exprAddPrim
// exprAddPrim: ( ADD | SUB ) exprMul exprAddPrim | epsilon
bool exprAddPrim() {
    if (consume(ADD) || consume(SUB)) {
        if (exprMul()) {
            return exprAddPrim();
        }
        tkerr("missing expression after + or -");
    }
    return true; 
}

bool exprAdd() {
    if (exprMul()) {
        return exprAddPrim();
    }
    return false;
}

// exprMul: exprCast exprMulPrim
// exprMulPrim: ( MUL | DIV ) exprCast exprMulPrim | epsilon
bool exprMulPrim() {
    if (consume(MUL) || consume(DIV)) {
        if (exprCast()) {
            return exprMulPrim();
        }
        tkerr("missing expression after * or /");
    }
    return true; 
}

bool exprMul() {
    if (exprCast()) {
        return exprMulPrim();
    }
    return false;
}

// exprCast: LPAR typeBase arrayDecl? RPAR exprCast | exprUnary
bool exprCast() {
    Token* start = iTk;
    if (consume(LPAR)) {
        if (typeBase()) {
            if (arrayDecl()) {}
            if (consume(RPAR)) {
                if (exprCast()) return true;
                tkerr("missing expression after cast");
            }
        }
        iTk = start;
    }
    return exprUnary();
}

// exprUnary: ( SUB | NOT ) exprUnary | exprPostfix
bool exprUnary() {
    if (consume(SUB) || consume(NOT)) {
        if (exprUnary()) return true;
        tkerr("missing expression after unary operator");
    }
    return exprPostfix();
}

// exprPostfix: exprPrimary exprPostfixPrim
// exprPostfixPrim: LBRACKET expr RBRACKET exprPostfixPrim
//               | DOT ID exprPostfixPrim
//               | epsilon
bool exprPostfixPrim() {
    if (consume(LBRACKET)) {
        if (expr()) {
            if (consume(RBRACKET)) return exprPostfixPrim();
            tkerr("missing ] in array index");
        }
        tkerr("missing expression in array index");
    }
    if (consume(DOT)) {
        if (consume(ID)) return exprPostfixPrim();
        tkerr("missing identifier after .");
    }
    return true; 
}

bool exprPostfix() {
    if (exprPrimary()) {
        return exprPostfixPrim();
    }
    return false;
}

// exprPrimary: ID ( LPAR ( expr ( COMMA expr )* )? RPAR )?
//            | INT | DOUBLE | CHAR | STRING | LPAR expr RPAR
bool exprPrimary() {
    Token* start = iTk;

    if (consume(ID)) {
        if (consume(LPAR)) {
            if (expr()) {
                while (consume(COMMA)) {
                    if (!expr()) tkerr("missing expression after ,");
                }
            }
            if (consume(RPAR)) return true;
            tkerr("missing ) in function call");
        }
        return true; // just an ID (variable)
    }

    if (consume(INT)) return true;
    if (consume(DOUBLE)) return true;
    if (consume(CHAR)) return true;
    if (consume(STRING)) return true;

    if (consume(LPAR)) {
        if (expr()) {
            if (consume(RPAR)) return true;
            tkerr("missing ) after expression");
        }
        tkerr("missing expression after (");
    }

    iTk = start;
    return false;
}

// unit: ( structDef | fnDef | varDef )* END
bool unit() {
    for (;;) {
        if (structDef()) {}
        else if (fnDef()) {}
        else if (varDef()) {}
        else break;
    }
    if (consume(END)) {
        return true;
    }
    return false;
}

void parse(Token* tokens) {
    iTk = tokens;
    if (!unit()) tkerr("syntax error");
}