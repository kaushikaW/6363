#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

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

#endif
