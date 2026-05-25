#include <string.h>
#include "at.h"

bool canBeScalar(Ret* r) {
	Type* t = &r->type;
	// void is not scalar
	if (t->tb == TB_VOID) return false;
	// a struct value (not an array of struct) is not scalar
	if (t->tb == TB_STRUCT && t->n < 0) return false;
	// arrays are convertible to addresses and are considered scalar in conditions
	return true;
}

bool convTo(Type* src, Type* dst) {
	// arrays can be converted only to arrays
	if (src->n >= 0) {
		return (dst->n >= 0);
	}
	// cannot convert a non-array to an array
	if (dst->n >= 0) return false;

	// Strict scalar conversions:
	// - int <-> double allowed
	// - char only converts to char (no implicit promotion to int/double)
	switch (src->tb) {
	case TB_INT:
		if (dst->tb == TB_INT || dst->tb == TB_DOUBLE) return true;
		return false;
	case TB_DOUBLE:
		if (dst->tb == TB_DOUBLE || dst->tb == TB_INT) return true;
		return false;
	case TB_CHAR:
		if (dst->tb == TB_CHAR) return true;
		return false;
		// a struct can be converted only to itself
	case TB_STRUCT:
		if (dst->tb == TB_STRUCT && src->s == dst->s) return true;
		return false;
	default:
		return false;
	}
}

bool arithTypeTo(Type* t1, Type* t2, Type* dst) {
	// there are no arithmetic operations with pointers
	if (t1->n >= 0 || t2->n >= 0) return false;
	// the result of an arithmetic operation cannot be pointer or struct
	dst->s = NULL;
	dst->n = -1;
	switch (t1->tb) {
	case TB_INT:
		switch (t2->tb) {
		case TB_INT:
		case TB_CHAR:
			dst->tb = TB_INT; return true;
		case TB_DOUBLE: dst->tb = TB_DOUBLE; return true;
		default: return false;
		}
	case TB_DOUBLE:
		switch (t2->tb) {
		case TB_INT:
		case TB_DOUBLE:
		case TB_CHAR:
			dst->tb = TB_DOUBLE; return true;
		default: return false;
		}
	case TB_CHAR:
		switch (t2->tb) {
		case TB_INT:
		case TB_DOUBLE:
		case TB_CHAR:
			dst->tb = t2->tb; return true;
		default: return false;
		}
	default: return false;
	}
}

Symbol* findSymbolInList(Symbol* list, const char* name) {
	for (Symbol* s = list; s; s = s->next) {
		if (!strcmp(s->name, name)) return s;
	}
	return NULL;
}