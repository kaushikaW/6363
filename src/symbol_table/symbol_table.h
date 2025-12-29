#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H
#include "../parser/ast.h"

typedef enum { SYM_CLASS, SYM_FUNCTION, SYM_ATTRIBUTE, SYM_VARIABLE } SymbolKind;

typedef struct Param {
    char *name;
    char *type;
    struct Param *next;
} Param;

typedef struct Symbol {
    char *name;
    SymbolKind kind;
    char *type;
    char *scope;
    char *visibility;
    int line;
    int column;
    int isDeclared;      // 1 if declared only, 0 if implemented
    Param *params;       // linked list of parameters (for functions)
    int offset;
    struct Symbol *next;
} Symbol;

typedef struct SymbolTable {
    char *scopeName;
    Symbol *head;
    struct SymbolTable **children;
    int childCount;
} SymbolTable;

// Symbol Table functions
SymbolTable* createSymbolTable(const char *scopeName);
void insertSymbol(SymbolTable *table, const char *name, SymbolKind kind,
                  const char *type, const char *scope, const char *visibility,
                  int line, int column, int isDeclared);
void addNestedTable(SymbolTable *parent, SymbolTable *child);
void printSymbolTable(SymbolTable *table);
void freeSymbolTable(SymbolTable *table);
Symbol* lookupSymbol(SymbolTable *table, const char *name, const char *scope);

// symbol table generation
ASTNode* findChild(ASTNode *node, const char *kind);
void handleAttributeDecl(ASTNode *node, SymbolTable *currentTable, const char *scope, const char *visibility);
void handleFuncDecl(ASTNode *node, SymbolTable *currentTable, const char *scope, const char *visibility, int isDeclaration);
void handleClassDecl(ASTNode *node, SymbolTable *currentTable);
void buildSymbolTable(ASTNode *root, SymbolTable *currentTable, const char *scope, const char *visibility);
void collectFuncParams(ASTNode *node, SymbolTable *funcTable, const char *funcName);
Symbol* lookupSymbolRecursive(SymbolTable *table, const char *name, const char *scope);
void handleFunctionDeclaration(ASTNode *node, SymbolTable *currentTable, FILE *errorLog);
Symbol* lookupSymbolRecursive(SymbolTable *table, const char *name, const char *scope);
void assignOffsets(SymbolTable *funcTable);
#endif
