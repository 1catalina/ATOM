#pragma once
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "lexer.h"

void parse(Token* tokens);
void tkerr(const char* fmt, ...);
bool consume(int code);

bool structDef();
bool varDef();
bool typeBase();
bool arrayDecl();
bool fnDef();
bool fnParam();
bool stmCompound();
bool stm();
bool expr();
bool exprAssign();
bool exprOr();
bool exprOrPrim();
bool exprAnd();
bool exprAndPrim();
bool exprEq();
bool exprEqPrim();
bool exprRel();
bool exprRelPrim();
bool exprAdd();
bool exprAddPrim();
bool exprMul();
bool exprMulPrim();
bool exprCast();
bool exprUnary();
bool exprPostfix();
bool exprPostfixPrim();
bool exprPrimary();

bool unit();
//bool typeBase();
//bool arrayDecl();
//bool varDef();
//bool structDef();
//bool fnDef();
//bool fnParam();
//bool stmCompound();
//bool stm();
//bool expr();
//bool exprAssign();
//bool exprOr();
//bool exprOrPrim();
//bool exprAnd();
//bool exprAndPrim();
//bool exprEq();
//bool exprEqPrim();
//bool exprRel();
//bool exprRelPrim();
//bool exprAdd();
//bool exprAddPrim();
//bool exprMul();
//bool exprMulPrim();
//bool exprCast();
//bool exprUnary();
//bool exprPostfix();
//bool exprPostfixPrim();
//bool exprPrimary();