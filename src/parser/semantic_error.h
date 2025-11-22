#ifndef SEMANTIC_ERROR_H
#define SEMANTIC_ERROR_H

#define MAX_ERRORS 500

typedef struct SemanticError {
    int line;
    int col;
    char type[50];      // "TYPE_MISMATCH", "UNDECLARED_VARIABLE", etc.
    char message[200];  // Full description
} SemanticError;

extern SemanticError errorList[MAX_ERRORS];
extern int errorCount;

// Add a semantic error
void addSemanticError(int line, int col, const char* type, const char* format, ...);

// Print all semantic errors (sorted by line and col)
void printSemanticErrors();

#endif // SEMANTIC_ERROR_H
