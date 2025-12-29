#include <stdio.h>
#include <stdlib.h>

#include "parser/parser.h"
#include "parser/ast.h"
#include "symbol_table/symbol_table.h"
#include "semantics/semantic.h"
#include "semantic_error/semantic_error.h"

int main() {

    // Open derivation file
    derivation = fopen("derivation.txt", "w");
    if (!derivation) {
        perror("derivation.txt");
        return 1;
    }

    nextToken(); // read first token
    ASTNode * root = prog();

    // Check for extra tokens
    if (lookahead != NULL) {
        fprintf(stderr, "Syntax error: extra tokens at end starting at '%s' (line %d, col %d)\n",
          lookahead -> lexeme, lookahead -> line, lookahead -> column);
        fclose(derivation);
        return 1;
    }

    printf("Parsing successful! Derivation written to derivation.txt\n");
    printf("\nAbstract Syntax Tree (ast):\n");
    printAST(root, 0);

    // Create global table and build nested symbol tables
    SymbolTable * globalTable = createSymbolTable("GLOBAL");
    buildSymbolTable(root, globalTable, "GLOBAL", NULL);


    printSymbolTable(globalTable);

    // Open semantic error log file
    FILE * semanticErrorLog = fopen("semantic_errors.txt", "w");

    if (!semanticErrorLog) {
        perror("semantic_errors.txt");
        freeSymbolTable(globalTable);
        fclose(derivation);
        return 1;
    }

    // Semantic analysis with type checking and error reporting
    analyzeSemantics(root, globalTable);

    printSemanticErrors();


    fclose(semanticErrorLog);
    freeSymbolTable(globalTable);
    fclose(derivation);

    return 0;
}