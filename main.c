#include <stdio.h>
#include "lexer.h"
#include "utils.h"

int main(int argc, char* argv[]) {
    FILE* fout = fopen("out.txt", "w");
    if (!fout) {
        printf("Unable to open file to write\n");
        return 1;
    }

    char* buffer = loadFile("testlex.c");
    Token* tks = tokenize(buffer);

    //printTokens(tks);
    writeTokens(tks, fout);
    printf("Tokenize process is done!\n");

    free(buffer);
    fclose(fout);

    return 0;
}
