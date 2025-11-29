#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"

SymbolTable* symbolTable = NULL; // global table

SymbolTable* createSymbolTable() {
    SymbolTable* table = malloc(sizeof(SymbolTable));
    table->head = NULL;
    return table;
}

void addSymbol(char *name, char *type, char *scope) {
    Symbol *sym = malloc(sizeof(Symbol));
    sym->name = strdup(name);
    sym->type = strdup(type);
    sym->scope = strdup(scope);
    sym->offset = -1; // offset will be calculated later
    sym->next = NULL;

    if (!symbolTable->head) {
        symbolTable->head = sym;
    } else {
        // insert at tail
        Symbol* curr = symbolTable->head;
        while (curr->next) curr = curr->next;
        curr->next = sym;
    }
}


void printSymbolTable() {
    printf("\n--- Symbol Table ---\n");
    printf("%-10s %-10s %-10s %-6s\n", "Name", "Type", "Scope", "Offset");
    Symbol *curr = symbolTable->head;
    while (curr) {
        printf("%-10s %-10s %-10s %-6d\n", curr->name, curr->type, curr->scope, curr->offset);
        curr = curr->next;
    }
}

void freeSymbolTable() {
    Symbol *curr = symbolTable->head;
    while (curr) {
        Symbol *tmp = curr;
        curr = curr->next;
        free(tmp->name);
        free(tmp->type);
        free(tmp->scope);
        free(tmp);
    }
    free(symbolTable);
}

void calculateOffsets() {
    if (!symbolTable) return;

    Symbol *curr = symbolTable->head;
    int offset = -8;  // start from -8 for the first local/temporary

    while (curr) {
        curr->offset = offset;
        offset -= 8; // next variable gets next 8-byte slot
        curr = curr->next;
    }
}

// Returns the total number of bytes needed to store all local variables
// by finding the largest negative offset in the symbol table.
int totalLocalBytes() {
    Symbol *curr = symbolTable->head;
    int maxBytes = 0;
    while (curr) {
        if (-curr->offset > maxBytes) maxBytes = -curr->offset;
        curr = curr->next;
    }
    return maxBytes;
}


