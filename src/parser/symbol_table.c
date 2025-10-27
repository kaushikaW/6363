#include "symbol_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

SymbolTable* createSymbolTable(const char *scopeName) {
    SymbolTable *table = malloc(sizeof(SymbolTable));
    table->head = NULL;
    table->scopeName = scopeName ? strdup(scopeName) : strdup("GLOBAL");
    table->children = NULL;
    table->childCount = 0;
    return table;
}

void insertSymbol(SymbolTable *table, const char *name, SymbolKind kind,
                  const char *type, const char *scope, const char *visibility,
                  int line, int column) {
    Symbol *sym = malloc(sizeof(Symbol));
    sym->name = strdup(name);
    sym->kind = kind;
    sym->type = type ? strdup(type) : NULL;
    sym->scope = scope ? strdup(scope) : NULL;
    sym->visibility = visibility ? strdup(visibility) : NULL;
    sym->line = line;
    sym->column = column;
    sym->next = table->head;
    table->head = sym;
}

void addNestedTable(SymbolTable *parent, SymbolTable *child) {
    parent->children = realloc(parent->children, sizeof(SymbolTable*) * (parent->childCount + 1));
    parent->children[parent->childCount++] = child;
}

Symbol* lookupSymbol(SymbolTable *table, const char *name, const char *scope) {
    for (Symbol *s = table->head; s; s = s->next) {
        if (strcmp(s->name, name) == 0 &&
            (!scope || (s->scope && strcmp(s->scope, scope) == 0))) {
            return s;
        }
    }
    return NULL;
}

void printSymbolTable(SymbolTable *table) {
    if (!table) return;

    printf("\n===== SYMBOL TABLE for scope: %s =====\n", table->scopeName);
    printf("%-12s %-12s %-12s %-12s %-10s %-5s %-5s\n",
           "Name", "Kind", "Type", "Scope", "Visibility", "Line", "Col");
    printf("---------------------------------------------------------------------------\n");

    for (Symbol *s = table->head; s; s = s->next) {
        const char *kindStr =
            (s->kind == SYM_CLASS) ? "Class" :
            (s->kind == SYM_FUNCTION) ? "Function" :
            (s->kind == SYM_ATTRIBUTE) ? "Attribute" : "Variable";
        printf("%-12s %-12s %-12s %-12s %-10s %-5d %-5d\n",
               s->name, kindStr,
               s->type ? s->type : "-",
               s->scope ? s->scope : "-",
               s->visibility ? s->visibility : "-",
               s->line, s->column);
    }

    printf("=============================================================================\n");

    for (int i = 0; i < table->childCount; i++) {
        printSymbolTable(table->children[i]);
    }
}

void freeSymbolTable(SymbolTable *table) {
    if (!table) return;

    Symbol *cur = table->head;
    while (cur) {
        Symbol *next = cur->next;
        free(cur->name);
        free(cur->type);
        free(cur->scope);
        free(cur->visibility);
        free(cur);
        cur = next;
    }

    for (int i = 0; i < table->childCount; i++) {
        freeSymbolTable(table->children[i]);
    }

    free(table->children);
    free(table->scopeName);
    free(table);
}
