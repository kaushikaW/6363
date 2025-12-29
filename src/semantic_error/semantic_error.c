#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "semantic_error.h"

/* ---------- ANSI Colors ---------- */
#define COLOR_RESET   "\x1b[0m"
#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_BOLD    "\x1b[1m"
/* -------------------------------- */

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
        printf(COLOR_GREEN COLOR_BOLD
               "Semantic analysis completed successfully.\n"
               COLOR_RESET);
        return;
    }

    // Sort errors by line and column
    qsort(errorList, errorCount, sizeof(SemanticError), compareErrors);

    printf(COLOR_RED COLOR_BOLD
           "\nSemantic Errors Found (%d)\n"
           COLOR_RESET, errorCount);

    // Print all errors
    for (int i = 0; i < errorCount; i++) {
        printf(
            COLOR_BLUE "Line %d, Col %d: " COLOR_RESET
            COLOR_YELLOW "[%s] " COLOR_RESET
            COLOR_RED "%s\n" COLOR_RESET,
            errorList[i].line,
            errorList[i].col,
            errorList[i].type,
            errorList[i].message
        );
    }
}
