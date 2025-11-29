#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

typedef struct Symbol {
    char *name;      // variable or temporary name
    char *type;      // data type (e.g., "integer")
    char *scope;     // scope name (e.g., "main")
    int offset;      // memory offset (not generated yet)
    struct Symbol *next;
} Symbol;

// Symbol table as a linked list
typedef struct {
    Symbol *head;
} SymbolTable;

SymbolTable* createSymbolTable();
void addSymbol(char *name, char *type, char *scope);
Symbol* lookupSymbol(char *name, char *scope);
void printSymbolTable();
void freeSymbolTable();
void calculateOffsets();
int totalLocalBytes();
#endif
