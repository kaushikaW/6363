#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "semantic_error.h"

SemanticError errorList[MAX_ERRORS];
int errorCount = 0;

// Add a semantic error
void addSemanticError(int line, int col, const char* type, const char* format, ...) {
    if (errorCount >= MAX_ERRORS) return;

    va_list args;
    va_start(args, format);

    errorList[errorCount].line = line;
    errorList[errorCount].col = col;
    strcpy(errorList[errorCount].type, type);
    vsprintf(errorList[errorCount].message, format, args);

    va_end(args);
    errorCount++;
}

// Comparator for sorting by line, then column
static int compareErrors(const void* a, const void* b) {
    SemanticError* e1 = (SemanticError*)a;
    SemanticError* e2 = (SemanticError*)b;

    if (e1->line != e2->line)
        return e1->line - e2->line;
    return e1->col - e2->col;
}

// Print all semantic errors
void printSemanticErrors() {
    if (errorCount == 0) {
        printf("Semantic analysis completed successfully.\n");
        return;
    }

    // Sort errors by line and column
    qsort(errorList, errorCount, sizeof(SemanticError), compareErrors);

    // Print all errors
    for (int i = 0; i < errorCount; i++) {
        printf("Line %d, Col %d: [%s] %s\n",
               errorList[i].line,
               errorList[i].col,
               errorList[i].type,
               errorList[i].message);
    }
}
