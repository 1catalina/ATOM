#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

#include "parser.h"
#include "utils.h"
#include "ad.h"

Token* iTk;
Token* consumedTk;

// variabila globala owner: functia sau structura in interiorul careia suntem
Symbol* owner = NULL;

bool structDef();
bool fnDef();
bool varDef();
bool stmCompound(bool newDomain);
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
bool typeBase(Type* t);
bool arrayDecl(Type* t);
bool fnParam();

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
// AD: seteaza campurile din *t
bool typeBase(Type* t) {
	t->n = -1;
	if (consume(TYPE_INT)) { t->tb = TB_INT; return true; }
	if (consume(TYPE_DOUBLE)) { t->tb = TB_DOUBLE; return true; }
	if (consume(TYPE_CHAR)) { t->tb = TB_CHAR; return true; }
	if (consume(STRUCT)) {
		if (consume(ID)) {
			Token* tkName = consumedTk;
			t->tb = TB_STRUCT;
			t->s = findSymbol(tkName->text);
			if (!t->s) tkerr("structura nedefinita: %s", tkName->text);
			return true;
		}
		tkerr("identifier missing after struct");
	}
	return false;
}

// arrayDecl: LBRACKET INT? RBRACKET
// AD: seteaza t->n (dimensiunea sau 0 daca lipseste)
bool arrayDecl(Type* t) {
	Token* start = iTk;
	if (consume(LBRACKET)) {
		if (consume(INT)) {
			Token* tkSize = consumedTk;
			t->n = tkSize->i;
		}
		else {
			t->n = 0;
		}
		if (consume(RBRACKET)) {
			return true;
		}
		else tkerr("Missing ] in array declaration");
	}
	iTk = start;
	return false;
}

// varDef: typeBase ID arrayDecl? SEMICOLON
// AD: verifica redefinire, adauga simbol in TS
bool varDef() {
	Token* start = iTk;
	Type t;
	if (typeBase(&t)) {
		if (consume(ID)) {
			Token* tkName = consumedTk;
			if (arrayDecl(&t)) {
				if (t.n == 0) tkerr("a vector variable must have a specified dimension");
			}
			if (consume(SEMICOLON)) {
				Symbol* var = findSymbolInDomain(symTable, tkName->text);
				if (var) tkerr("symbol redefinition: %s", tkName->text);
				var = newSymbol(tkName->text, SK_VAR);
				var->type = t;
				var->owner = owner;
				addSymbolToDomain(symTable, var);
				if (owner) {
					switch (owner->kind) {
					case SK_FN:
						var->varIdx = symbolsLen(owner->fn.locals);
						addSymbolToList(&owner->fn.locals, dupSymbol(var));
						break;
					case SK_STRUCT:
						var->varIdx = typeSize(&owner->type);
						addSymbolToList(&owner->structMembers, dupSymbol(var));
						break;
					default: break;
					}
				}
				else {
					var->varMem = safeAlloc(typeSize(&t));
				}
				return true;
			}
			else tkerr("Missing ; after variable definition");
		}
		else tkerr("Missing Identificator after type or invalid struct/function declaration");
	}
	iTk = start;
	return false;
}

// arrayDecl: LBRACKET INT? RBRACKET
// structDef: STRUCT ID LACC varDef* RACC SEMICOLON
// AD: verifica redefinire, adauga SK_STRUCT, pushDomain, owner=s
bool structDef() {
	Token* start = iTk;
	if (consume(STRUCT)) {
		if (consume(ID)) {
			Token* tkName = consumedTk;
			if (consume(LACC)) {
				Symbol* s = findSymbolInDomain(symTable, tkName->text);
				if (s) tkerr("symbol redefinition: %s", tkName->text);
				s = addSymbolToDomain(symTable, newSymbol(tkName->text, SK_STRUCT));
				s->type.tb = TB_STRUCT;
				s->type.s = s;
				s->type.n = -1;
				pushDomain();
				owner = s;

				while (varDef()) {}

				if (consume(RACC)) {
					if (consume(SEMICOLON)) {
						owner = NULL;
						dropDomain();
						return true;
					}
					else tkerr("Missing ; after struct definition");
				}
				else tkerr("Missing } in struct definition");
			}
		}
		else tkerr("Missing ID after struct");
	}
	iTk = start;
	return false;
}

// fnParam: typeBase ID arrayDecl?
// AD: verifica redefinire, adauga SK_PARAM in domeniu si in fn.params
bool fnParam() {
	Token* start = iTk;
	Type t;
	if (typeBase(&t)) {
		if (consume(ID)) {
			Token* tkName = consumedTk;
			if (arrayDecl(&t)) {
				t.n = 0;	// parametrii vectori devin fara dimensiune specificata
			}
			Symbol* param = findSymbolInDomain(symTable, tkName->text);
			if (param) tkerr("symbol redefinition: %s", tkName->text);
			param = newSymbol(tkName->text, SK_PARAM);
			param->type = t;
			param->owner = owner;
			param->paramIdx = symbolsLen(owner->fn.params);
			addSymbolToDomain(symTable, param);
			addSymbolToList(&owner->fn.params, dupSymbol(param));
			return true;
		}
		tkerr("identifier missing in function parameter");
	}
	iTk = start;
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
					if (!expr()) tkerr("expression missing after ,");
				}
			}
			if (consume(RPAR)) return true;
			tkerr(") missing in function call");
		}
		return true;
	}
	if (consume(INT))    return true;
	if (consume(DOUBLE)) return true;
	if (consume(CHAR))   return true;
	if (consume(STRING)) return true;
	if (consume(LPAR)) {
		if (expr()) {
			if (consume(RPAR)) return true;
			tkerr(") missing after expression");
		}
		tkerr("invalid expression after (");
	}
	iTk = start;
	return false;
}

// exprPostfixPrim: LBRACKET expr RBRACKET exprPostfixPrim | DOT ID exprPostfixPrim | epsilon
bool exprPostfixPrim() {
	if (consume(LBRACKET)) {
		if (expr()) {
			if (consume(RBRACKET)) return exprPostfixPrim();
			tkerr("] missing in indexing");
		}
		tkerr("invalid expression in indexing");
	}
	if (consume(DOT)) {
		if (consume(ID)) return exprPostfixPrim();
		tkerr("field name missing after .");
	}
	return true;
}

// exprPostfix: exprPrimary exprPostfixPrim
bool exprPostfix() {
	if (exprPrimary()) return exprPostfixPrim();
	return false;
}

// exprUnary: ( SUB | NOT ) exprUnary | exprPostfix
bool exprUnary() {
	if (consume(SUB)) {
		if (exprUnary()) return true;
		tkerr("expression missing after unary operator (SUB)");
	}
	else if (consume(NOT)) {
		if (exprUnary()) return true;
		tkerr("expression missing after unary operator(NOT)");
	}
	return exprPostfix();
}

// exprCast: LPAR typeBase arrayDecl? RPAR exprCast | exprUnary
// AD: typeBase si arrayDecl au nevoie de un Type t local
bool exprCast() {
	Token* start = iTk;
	if (consume(LPAR)) {
		Type t;
		if (typeBase(&t)) {
			arrayDecl(&t);
			if (consume(RPAR)) {
				if (exprCast()) return true;
				tkerr("expression missing after cast");
			}
			tkerr(") missing in cast expression");
		}
	}
	iTk = start;
	return exprUnary();
}

// exprMulPrim: ( MUL | DIV ) exprCast exprMulPrim | epsilon
bool exprMulPrim() {
	if (consume(MUL)) {
		if (exprCast()) return exprMulPrim();
		tkerr("expression missing after *");
	}
	else if (consume(DIV)) {
		if (exprCast()) return exprMulPrim();
		tkerr("expression missing after /");
	}
	return true;
}

// exprMul: exprCast exprMulPrim
bool exprMul() {
	if (exprCast()) return exprMulPrim();
	return false;
}

// exprAddPrim: ( ADD | SUB ) exprMul exprAddPrim | epsilon
bool exprAddPrim() {
	if (consume(ADD)) {
		if (exprMul()) return exprAddPrim();
		tkerr("expression missing after +");
	}
	else if (consume(SUB)) {
		if (exprMul()) return exprAddPrim();
		tkerr("expression missing after -");
	}
	return true;
}

// exprAdd: exprMul exprAddPrim
bool exprAdd() {
	if (exprMul()) return exprAddPrim();
	return false;
}

// exprRelPrim: ( LESS | LESSEQ | GREATER | GREATEREQ ) exprAdd exprRelPrim | epsilon
bool exprRelPrim() {
	if (consume(LESS)) {
		if (exprAdd()) return exprRelPrim();
		tkerr("expression missing after relational operator");
	}
	else if (consume(LESSEQ)) {
		if (exprAdd()) return exprRelPrim();
		tkerr("expression missing after relational operator");
	}
	else if (consume(GREATER)) {
		if (exprAdd()) return exprRelPrim();
		tkerr("expression missing after relational operator");
	}
	else if (consume(GREATEREQ)) {
		if (exprAdd()) return exprRelPrim();
		tkerr("expression missing after relational operator");
	}
	return true;
}

// exprRel: exprAdd exprRelPrim
bool exprRel() {
	if (exprAdd()) return exprRelPrim();
	return false;
}

// exprEqPrim: ( EQUAL | NOTEQ ) exprRel exprEqPrim | epsilon
bool exprEqPrim() {
	if (consume(EQUAL)) {
		if (exprRel()) return exprEqPrim();
		tkerr("expression missing after ==");
	}
	else if (consume(NOTEQ)) {
		if (exprRel()) return exprEqPrim();
		tkerr("expression missing after !=");
	}
	return true;
}

// exprEq: exprRel exprEqPrim
bool exprEq() {
	if (exprRel()) return exprEqPrim();
	return false;
}

// exprAndPrim: AND exprEq exprAndPrim | epsilon
bool exprAndPrim() {
	if (consume(AND)) {
		if (exprEq()) return exprAndPrim();
		tkerr("expression missing after &&");
	}
	return true;
}

// exprAnd: exprEq exprAndPrim
bool exprAnd() {
	if (exprEq()) return exprAndPrim();
	return false;
}

// exprOrPrim: OR exprAnd exprOrPrim | epsilon
bool exprOrPrim() {
	if (consume(OR)) {
		if (exprAnd()) return exprOrPrim();
		tkerr("expression missing after ||");
	}
	return true;
}

// exprOr: exprAnd exprOrPrim
bool exprOr() {
	if (exprAnd()) return exprOrPrim();
	return false;
}

// exprAssign: exprUnary ASSIGN exprAssign | exprOr
bool exprAssign() {
	Token* start = iTk;
	if (exprUnary()) {
		if (consume(ASSIGN)) {
			if (exprAssign()) return true;
			tkerr("expression missing after =");
		}
	}
	iTk = start;
	return exprOr();
}

// expr: exprAssign
bool expr() {
	return exprAssign();
}

// stmCompound: LACC ( varDef | stm )* RACC
// AD: parametru newDomain controleaza daca se creeaza un domeniu nou
bool stmCompound(bool newDomain) {
	Token* start = iTk;
	if (consume(LACC)) {
		if (newDomain) pushDomain();
		for (;;) {
			if (varDef()) {}
			else if (stm()) {}
			else break;
		}
		if (consume(RACC)) {
			if (newDomain) dropDomain();
			return true;
		}
		tkerr("} missing at end of block");
	}
	iTk = start;
	return false;
}

// stm: stmCompound | IF LPAR expr RPAR stm ( ELSE stm )? | WHILE LPAR expr RPAR stm | RETURN expr? SEMICOLON | expr? SEMICOLON
// AD: stmCompound apelat cu true (blocurile if/while creeaza domenii noi)
bool stm() {
	Token* start = iTk;

	if (stmCompound(true)) return true;

	if (consume(IF)) {
		if (consume(LPAR)) {
			if (expr()) {
				if (consume(RPAR)) {
					if (stm()) {
						if (consume(ELSE)) {
							if (!stm()) tkerr("statement missing after else");
						}
						return true;
					}
					tkerr("statement missing for if body");
				}
				tkerr(") missing after if condition");
			}
			tkerr("invalid or missing condition in if");
		}
		tkerr("( missing after if");
	}

	if (consume(WHILE)) {
		if (consume(LPAR)) {
			if (expr()) {
				if (consume(RPAR)) {
					if (stm()) return true;
					tkerr("statement missing for while body");
				}
				tkerr(") missing after while condition");
			}
			tkerr("invalid or missing condition in while");
		}
		tkerr("( missing after while");
	}

	if (consume(RETURN)) {
		expr();
		if (consume(SEMICOLON)) return true;
		tkerr("; missing after return");
	}

	expr();
	if (consume(SEMICOLON)) return true;

	iTk = start;
	return false;
}

// fnDef: ( typeBase | VOID ) ID LPAR ( fnParam ( COMMA fnParam )* )? RPAR stmCompound
// AD: verifica redefinire, adauga SK_FN, owner=fn, pushDomain
//     stmCompound apelat cu false (corpul fn nu adauga domeniu nou)
bool fnDef() {
	Token* start = iTk;
	Type t;
	bool hasType = typeBase(&t);
	if (!hasType) {
		if (consume(VOID)) {
			t.tb = TB_VOID;
			t.n = -1;
			t.s = NULL;
		}
		else {
			iTk = start;
			return false;
		}
	}
	if (consume(ID)) {
		Token* tkName = consumedTk;
		if (consume(LPAR)) {
			Symbol* fn = findSymbolInDomain(symTable, tkName->text);
			if (fn) tkerr("symbol redefinition: %s", tkName->text);
			fn = newSymbol(tkName->text, SK_FN);
			fn->type = t;
			addSymbolToDomain(symTable, fn);
			owner = fn;
			pushDomain();

			if (fnParam()) {
				while (consume(COMMA)) {
					if (!fnParam()) tkerr("parameter missing or invalid after ,");
				}
			}
			// else{
			// 	tkerr("invalid or missing parameter");
			// }
			if (consume(RPAR)) {
				if (stmCompound(false)) {
					dropDomain();
					owner = NULL;
					return true;
				}
				tkerr("function body { } missing");
			}
			tkerr(") missing in function definition");
		}
		iTk = start;
		return false;
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
	if (consume(END)) return true;
	tkerr("unexpected token at end of file");
	return false;
}

void parse(Token* tokens) {
	iTk = tokens;
	pushDomain();
	if (!unit()) tkerr("syntax error");
	showDomain(symTable, "global");
	dropDomain();
}