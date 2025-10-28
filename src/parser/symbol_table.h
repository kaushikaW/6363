#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H
#include "AST/ast.h"

typedef enum { SYM_CLASS, SYM_FUNCTION, SYM_ATTRIBUTE, SYM_VARIABLE } SymbolKind;

typedef struct Symbol {
    char *name;
    SymbolKind kind;
    char *type;
    char *scope;
    char *visibility;
    int line;
    int column;
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
                  int line, int column);
void addNestedTable(SymbolTable *parent, SymbolTable *child);
void printSymbolTable(SymbolTable *table);
void freeSymbolTable(SymbolTable *table);
Symbol* lookupSymbol(SymbolTable *table, const char *name, const char *scope);


// symbol table genartion
// --- Find child node by kind ---
ASTNode* findChild(ASTNode *node, const char *kind);

// --- Handle attribute declarations ---
void handleAttributeDecl(ASTNode *node, SymbolTable *currentTable, const char *scope, const char *visibility);

// --- Handle function declarations ---
void handleFuncDecl(ASTNode *node, SymbolTable *currentTable, const char *scope, const char *visibility);

// --- Handle class declarations ---
void handleClassDecl(ASTNode *node, SymbolTable *currentTable);

// --- Main symbol table builder ---
void buildSymbolTable(ASTNode *root, SymbolTable *currentTable, const char *scope, const char *visibility);




#endif
