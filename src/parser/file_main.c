#include <stdio.h>
#include "parser.h"

int main() {
    derivation = fopen("derivation.txt", "w");
    if (!derivation) {
        perror("derivation.txt");
        return 1;
    }

    nextToken(); // read first token
    prog();

    if (lookahead != NULL) {
        fprintf(stderr, "Syntax error: extra tokens at end starting at '%s' (line %d, col %d)\n",
                lookahead->lexeme, lookahead->line, lookahead->column);
        fclose(derivation);
        return 1;
    }

    printf("Parsing successful! Derivation written to derivation.txt\n");
    fclose(derivation);
    return 0;
}
