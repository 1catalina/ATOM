#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

#include "parser.h"
#include "lexer.h"

bool structDef(void);
bool fnDef(void);
bool varDef(void);
bool arrayDecl(void);
bool fnParam(void);
bool stm(void);
bool stmCompound(void);
bool expr(void);
bool exprAssign(void);
bool exprOr(void);
bool exprOrPrim(void);
bool exprAnd(void);
bool exprAndPrim(void);
bool exprEq(void);
bool exprEqPrim(void);
bool exprRel(void);
bool exprRelPrim(void);
bool exprAdd(void);
bool exprAddPrim(void);
bool exprMul(void);
bool exprMulPrim(void);
bool exprCast(void);
bool exprUnary(void);
bool exprPostfix(void);
bool exprPostfixPrim(void);
bool exprPrimary(void);

Token *iTk;				// the iterator in the tokens list
Token *consumedTk;		// the last consumed token

void tkerr(const char *fmt,...){
	fprintf(stderr,"error in line %d: ",iTk->line);
	va_list va;
	va_start(va,fmt);
	vfprintf(stderr,fmt,va);
	va_end(va);
	fprintf(stderr,"\n");
	exit(EXIT_FAILURE);
	}

bool consume(int code){
	if(iTk->code==code){
		consumedTk=iTk;
		iTk=iTk->next;
		return true;
		}
	return false;
	}

// typeBase: TYPE_INT | TYPE_DOUBLE | TYPE_CHAR | STRUCT ID
bool typeBase(){
	Token *start = iTk;
	if(consume(TYPE_INT)){
		return true;
		}
	if(consume(TYPE_DOUBLE)){
		return true;
		}
	if(consume(TYPE_CHAR)){
		return true;
		}
	if(consume(STRUCT)){
		if(consume(ID))
		{
			return true;
		}
		iTk = start;
	}
	return false;
	}

// structDef: STRUCT ID LACC varDef* RACC SEMICOLON
bool structDef() {
    Token *start = iTk;
    if (consume(STRUCT)) {
        if (consume(ID)) {
            if (consume(LACC)) {
                while (varDef()) {} 
                if (consume(RACC)) {
                    if (consume(SEMICOLON)) {
                        return true;
                    } else tkerr("Missing ; after struct definition");
                } else tkerr("Missing } in struct definition");
			} 
		} 
		else tkerr("Missing ID after struct");
    }
    iTk = start;
    return false;
}

// varDef: typeBase ID arrayDecl? SEMICOLON
bool varDef() {
    Token *start = iTk;
    if (typeBase()) {
        if (consume(ID)) {
            arrayDecl();
            if (consume(SEMICOLON)) {
                return true;
            } else tkerr("Missing ; after variable definition");
        }
		else tkerr("Missing ID after type or invalid struct/function declaration");
    }
    iTk = start;
    return false;
}

// arrayDecl: LBRACKET INT? RBRACKET
bool arrayDecl() {
    Token *start = iTk;
    if (consume(LBRACKET)) {
        consume(INT);
        if (consume(RBRACKET)) {
            return true;
        } else tkerr("Missing ] in array declaration");
    }
    iTk = start;
    return false;
}

// fnDef: (typeBase | VOID) ID LPAR (fnParam (COMMA fnParam)*)? RPAR stmCompound
bool fnDef() {
    Token *start = iTk;
    if (typeBase() || consume(VOID)) {
        if (consume(ID)) {
            if (consume(LPAR)) {
                if (fnParam()) {
                    while (consume(COMMA)) {
                        if (!fnParam()) tkerr("Syntax error in function parameter: missing/invalid parameter after ,");
                    }
                }
                if (consume(RPAR)) {
                    if (stmCompound()) {
                        return true;
                    } else tkerr("Missing { in function definition}");
                } else tkerr("Missing ) in function definition");
            }
        }
    }
    iTk = start;
    return false;
}

// fnParam: typeBase ID arrayDecl?
bool fnParam() {
    Token *start = iTk;
    if (typeBase()) {
        if (consume(ID)) {
            arrayDecl(); 
            return true;
        } else tkerr("Missing ID in function parameter");
    }
    iTk = start;
    return false;
}

// stm: stmCompound | IF LPAR expr RPAR stm (ELSE stm )? | WHILE LPAR expr RPAR stm | RETURN expr? SEMICOLON | expr? SEMICOLON
bool stm() {
    Token *start = iTk;

    if (stmCompound()) return true;

    if (consume(IF)) {
        if (consume(LPAR)) {
            if (expr()) {
                if (consume(RPAR)) {
                    if (stm()) {
                        if (consume(ELSE)) {
                            if (!stm()) tkerr("Missing statement after else");
                        }
                        return true;
                    } else tkerr("Missing statement after if");
                } else tkerr("Missing ) after if expression");
            } else tkerr("Missing condition expression in if");
        } else tkerr("Missing ( after if");
    }

    if (consume(WHILE)) {
        if (consume(LPAR)) {
            if (expr()) {
                if (consume(RPAR)) {
                    if (stm()) {
                        return true;
                    } else tkerr("Missing statement after while");
                } else tkerr("Missing ) after while expression");
            } else tkerr("Missing condition expression in while");
        } else tkerr("Missing ( after while");
    }

    if (consume(RETURN)) {
        expr(); 
        if (consume(SEMICOLON)) {
            return true;
        } else tkerr("Missing ; after return");
    }

    iTk = start; 
    if (expr()) {
        if (consume(SEMICOLON)) 
			return true;
		else 
			tkerr("Missing ; after expression statement");
    }
	else 
	{
        if (consume(SEMICOLON)) return true;
    }

    iTk = start;
    return false;
}

// stmCompound: LACC (varDef | stm)* RACC
bool stmCompound() {
    Token *start = iTk;
    if (consume(LACC)) {
        for (;;) {
            if (varDef()) {}
            else if (stm()) {}
            else break;
        }
        if (consume(RACC)) {
            return true;
        } else tkerr("Missing } at end of compound statement");
    }
    iTk = start;
    return false;
}

// expr: exprAssign
bool expr() {
    return exprAssign();
}

// exprAssign: exprUnary ASSIGN exprAssign | exprOr
bool exprAssign() {
    Token *start = iTk;
    if (exprUnary()) {
        if (consume(ASSIGN)) {
            if (exprAssign()) {
                return true;
            } else tkerr("Missing expression after =");
        }
    }
    iTk = start;
    if (exprOr()) return true;
    iTk = start;
    return false;
}

//exprOr: exprOr OR exprAnd | exprAnd
// exprOr: exprAnd exprOrPrim

bool exprOr() {
    Token *start = iTk;
    if (exprAnd()) {
        if (exprOrPrim()) {
            return true;
        }
    }
    iTk = start;
    return false;
}

// exprOrPrim: OR exprAnd exprOrPrim | epsilon

bool exprOrPrim() {
    if (consume(OR)) {
        if (exprAnd()) {
            if (exprOrPrim()) {
                return true;
            }
        } else tkerr("Missing expression after ||");
    }
    return true; // epsilon
}

// exprAnd: exprAnd AND exprEq | exprEq
// exprAnd: exprEq exprAndPrim

bool exprAnd() {
    Token *start = iTk;
    if (exprEq()) {
        if (exprAndPrim()) {
            return true;
        }
    }
    iTk = start;
    return false;
}

// exprAndPrim: AND exprEq exprAndPrim | epsilon

bool exprAndPrim() {
	if (consume(AND)) {
		if (exprEq()) {
			if (exprAndPrim()) {
				return true;
			}
		} else tkerr("Missing expression after &&");
	}
	return true; // epsilon
}

// exprEq: exprEq ( EQUAL | NOTEQ ) exprRel | exprRel
// exprEq: exprRel exprEqPrim

bool exprEq() {
    Token *start = iTk;
	if (exprRel()) {
		if (exprEqPrim()) {
			return true;
		}
	}
	iTk = start;
	return false;
}

// exprEqPrim: ( EQUAL | NOTEQ ) exprRel exprEqPrim | epsilon

bool exprEqPrim() {
	if (consume(EQUAL)) {
		if (exprRel()) {
			if (exprEqPrim()) {
				return true;
			}
		} else tkerr("Missing expression after ==");
	}
	else if (consume(NOTEQ)) {
		if (exprRel()) {
			if (exprEqPrim()) {
				return true;
			}
		} else tkerr("Missing expression after !=");
	}
	return true; // epsilon
}

//exprRel: exprRel ( LESS | LESSEQ | GREATER | GREATEREQ ) exprAdd | exprAdd
// exprRel: exprAdd exprRelPrim

bool exprRel() {
	Token *start = iTk;
	if (exprAdd()) {
		if (exprRelPrim()) {
			return true;
		}
	}
	iTk = start;
	return false;
}

// exprRelPrim: ( LESS | LESSEQ | GREATER | GREATEREQ ) exprAdd exprRelPrim | epsilon

bool exprRelPrim() {	
	if (consume(LESS)) {
		if (exprAdd()) {
			if (exprRelPrim()) {
				return true;
			}
		} else tkerr("Missing expression after <");
	}
	else if (consume(LESSEQ)) {
		if (exprAdd()) {
			if (exprRelPrim()) {
				return true;
			}
		} else tkerr("Missing expression after <=");
	}
	else if (consume(GREATER)) {
		if (exprAdd()) {
			if (exprRelPrim()) {
				return true;
			}
		} else tkerr("Missing expression after >");
	}
	else if (consume(GREATEREQ)) {
		if (exprAdd()) {
			if (exprRelPrim()) {
				return true;
			}
		} else tkerr("Missing expression after >=");
	}
	return true; // epsilon
}

//exprAdd: exprAdd ( ADD | SUB ) exprMul | exprMul
// exprAdd: exprMul exprAddPrim

bool exprAdd() {
	Token *start = iTk;
	if (exprMul()) {
		if (exprAddPrim()) {
			return true;
		}
	}
	iTk = start;
	return false;
}

// exprAddPrim: ( ADD | SUB ) exprMul exprAddPrim | epsilon

bool exprAddPrim() {
	if (consume(ADD)) {
		if (exprMul()) {
			if (exprAddPrim()) {
				return true;
			}
		} else tkerr("Missing expression after +");
	}
	else if (consume(SUB)) {
		if (exprMul()) {
			if (exprAddPrim()) {
				return true;
			}
		} else tkerr("Missing expression after -");
	}
	return true; // epsilon
}

//exprMul: exprMul ( MUL | DIV ) exprCast | exprCast
// exprMul: exprCast exprMulPrim

bool exprMul() {
	Token *start = iTk;
	if (exprCast()) {
		if (exprMulPrim()) {
			return true;
		}
	}
	iTk = start;
	return false;
}

// exprMulPrim: ( MUL | DIV ) exprCast exprMulPrim | epsilon

bool exprMulPrim() {
	if (consume(MUL)) {
		if (exprCast()) {
			if (exprMulPrim()) {
				return true;
			}
		} else tkerr("Missing expression after * ");
	}
	else if (consume(DIV)) {
		if (exprCast()) {
			if (exprMulPrim()) {
				return true;
			}
		} else tkerr("Missing expression after /");
	}
	return true; // epsilon
}

//exprCast: LPAR typeBase arrayDecl? RPAR exprCast | exprUnary

bool exprCast() {
	Token *start = iTk;
	if (consume(LPAR)) {
		if (typeBase()) {
			arrayDecl();
			if (consume(RPAR)) {
				if (exprCast()) {
					return true;
				} else tkerr("Missing expression after cast");
			} else tkerr("Missing ) in cast");
		}
	}
	iTk = start;
	return exprUnary();
}

//exprUnary: ( SUB | NOT ) exprUnary | exprPostfix

bool exprUnary() {
	Token *start = iTk;
	if (consume(SUB)) {
		if (exprUnary()) {
			return true;
		} else tkerr("Missing expression after -");
	}
	else if (consume(NOT)) {
		if (exprUnary()) {
			return true;
		} else tkerr("Missing expression after !");
	}
	iTk = start;
	return exprPostfix();
}

//exprPostfix: exprPostfix LBRACKET expr RBRACKET
//| exprPostfix DOT ID
//| exprPrimary
//
//exprPostfix: exprPrimary exprPostfixPrim


bool exprPostfix() {
	Token *start = iTk;
	if (exprPrimary()) {
		if (exprPostfixPrim()) {
			return true;
		}
	}
	iTk = start;
	return false;
}

// exprPostfixPrim: LBRACKET expr RBRACKET exprPostfixPrim | DOT ID exprPostfixPrim | epsilon
bool exprPostfixPrim() {
	if (consume(LBRACKET)) {
		if (expr()) {
			if (consume(RBRACKET)) {
				if (exprPostfixPrim()) {
					return true;
				}
			} else tkerr("Missing ] in array access");
		} else tkerr("Missing expression in array access");
	} else if (consume(DOT)) {
		if (consume(ID)) {
			if (exprPostfixPrim()) {
				return true;
			}
		} else tkerr("Missing ID in struct access");
	}
	return true; // epsilon
}


//exprPrimary: ID ( LPAR ( expr ( COMMA expr )* )? RPAR )?
//| INT | DOUBLE | CHAR | STRING | LPAR expr RPAR

bool exprPrimary() {
	Token *start = iTk;
	if (consume(ID)) {
		if (consume(LPAR)) {
			if (expr()) {
				while (consume(COMMA)) {
					if (!expr()) tkerr("Missing expression after , in function call");
				}
			}
			if (consume(RPAR)) {
				return true;
			} else tkerr("Missing ) in function call");
		}
		return true;
	}
	iTk = start;
	if (consume(INT)) return true;
	else if (consume(DOUBLE)) return true;
	else if (consume(CHAR)) return true;
	else if (consume(STRING)) return true;

	iTk = start;
	if (consume(LPAR)) {
		if (expr()) {
			if (consume(RPAR)) {
				return true;
			} else tkerr("Missing ) in parenthesized expression");
		} 
	}
	iTk = start;
	return false;
}

// unit: ( structDef | fnDef | varDef )* END

bool unit(){
	for(;;){
		if(structDef()){}
		else if(fnDef()){}
		else if(varDef()){}
		else break;
		}
	if(consume(END)){
		return true;
		}

	tkerr("Unexpected token after end of program");
	return false;
}

void parse(Token *tokens){
	iTk=tokens;
	if(!unit())tkerr("syntax error");
	}
