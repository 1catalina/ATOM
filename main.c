#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"
#include "utils.h"
#include "parser.h"

int main() {
    FILE* fout = fopen("out.txt", "w");
    if (!fout) {
        printf("Unable to open file to write\n");
        return 1;
    }

    char* buffer = loadFile("testat.c");
    Token* tks = tokenize(buffer);

    //printTokens(tks);
    //writeTokens(tks, fout);
    //printf("Tokenize process successful\n");

    parse(tks);
    printf("Parsing successful\n");
	writeTokens(tks, fout);
	

    free(buffer);
    fclose(fout);

    return 0;
}
