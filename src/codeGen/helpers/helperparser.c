#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../lexer/token.h"
#include "helperparser.h"
#include "symbol_table.h"
#include "../codegen.h"

extern Token * yylex();

Token * lookahead; // current token
FILE * derivation; // optional derivation output
SymbolTable *symbolTable; // global table

int tempCount = 0;

Quadruple **quadArray = NULL;
int quadCount = 0;

// Generate a new temporary variable for 3AC
char* newTemp() {
    static char buffer[20];
    sprintf(buffer, "_t%d", tempCount++);
    return strdup(buffer);
}

// ---------------- AST Functions ----------------
ASTNode_n* createASTNode_n(char *kind, const char *value, int line, int column) {
    ASTNode_n* node = malloc(sizeof(ASTNode_n));
    if (!node) {
        fprintf(stderr, "Memory allocation failed for ASTNode_n\n");
        exit(EXIT_FAILURE);
    }
    node->kind = kind ? strdup(kind) : NULL;
    node->value = value ? strdup(value) : NULL;
    node->children = NULL;
    node->childCount = 0;
    node->line = line;
    node->column = column;
    node->type = NODE_EXPR;
    return node;
}

void addChild_n(ASTNode_n* parent, ASTNode_n* child) {
    if (!parent || !child) return;
    parent->childCount++;
    parent->children = (ASTNode_n**) realloc(parent->children, parent->childCount * sizeof(ASTNode_n*));
    parent->children[parent->childCount - 1] = child;
}

void freeAST_n(ASTNode_n* node) {
    if (!node) return;
    for (int i = 0; i < node->childCount; i++)
        freeAST_n(node->children[i]);
    free(node->children);
    if (node->kind) free(node->kind);
    if (node->value) free(node->value);
    free(node);
}

void printAST_n(ASTNode_n* node, int indent) {
    if (!node) return;
    for (int i = 0; i < indent; i++) printf("|   ");
    printf("|-- %s", node->kind ? node->kind : "NULL");
    if (node->value) printf(" (%s)", node->value);
    printf(" [line %d, col %d]\n", node->line, node->column);
    for (int i = 0; i < node->childCount; i++)
        printAST_n(node->children[i], indent + 1);
}


// ---------------- Token Functions ----------------
void nextToken_n() {
    if (lookahead) {
        free(lookahead->tokenType);
        free(lookahead->lexeme);
        free(lookahead);
    }
    lookahead = yylex();
    if (lookahead)
        printf("TOKEN: %-15s Lexeme: %-10s\n", lookahead->tokenType, lookahead->lexeme);
}

void syntax_error_n(const char *expected) {
    if (lookahead)
        fprintf(stderr, "Syntax error: expected %s but found %s (lexeme '%s') at line %d, col %d\n",
                expected, lookahead->tokenType, lookahead->lexeme, lookahead->line, lookahead->column);
    else
        fprintf(stderr, "Syntax error: expected %s\n", expected);
    exit(1);
}

void match_n(const char *expectedType) {
    if (lookahead && strcmp(lookahead->tokenType, expectedType) == 0)
        nextToken_n();
    else
        syntax_error_n(expectedType);
}





// ---------------- Forward Declarations ----------------
ASTNode_n* statement_list_n();
ASTNode_n* statement_n();
ASTNode_n* var_decl_n();
ASTNode_n* assignment_n();
ASTNode_n* expression_n();
ASTNode_n* term_n();
ASTNode_n* factor_n();
ASTNode_n* if_statement_n();
ASTNode_n* condition_n();
ASTNode_n* while_statement_n();
ASTNode_n* condition_n();
ASTNode_n* write_statement_n();

// ---------------- Parser Rules ----------------
// program ::= func main() => void { statement_list }
ASTNode_n* prog_n() {
    fprintf(derivation, "program -> FUNC IDENTIFIER LPAREN RPAREN ARROW VOID LBRACE statement_list RBRACE\n");
    ASTNode_n* node = createASTNode_n("program", NULL, 0, 0);
    match_n("FUNC");
    match_n("IDENTIFIER"); // main
    match_n("LPAREN");
    match_n("RPAREN");
    match_n("ARROW");
    match_n("VOID");
    match_n("LBRACE");
    ASTNode_n* stmts = statement_list_n();
    if (stmts) addChild_n(node, stmts);
    match_n("RBRACE");
    return node;
}

// statement_list ::= { statement ; }
ASTNode_n* statement_list_n() {
    ASTNode_n* node = createASTNode_n("statement_list", NULL, 0, 0);
    while (lookahead &&
    (strcmp(lookahead->tokenType, "LOCAL") == 0 ||
     strcmp(lookahead->tokenType, "IDENTIFIER") == 0 ||
     strcmp(lookahead->tokenType, "IF") == 0 ||
     strcmp(lookahead->tokenType, "WRITE") == 0 ||
     strcmp(lookahead->tokenType, "WHILE") == 0))
{
        ASTNode_n* stmt = statement_n();
        if (stmt) addChild_n(node, stmt);
        match_n("SEMICOLON");
    }
    return node;
}

// statement ::= var_decl | assignment | write
ASTNode_n* statement_n() {
    if (strcmp(lookahead->tokenType, "LOCAL") == 0)
        return var_decl_n();

    if (strcmp(lookahead->tokenType, "IDENTIFIER") == 0)
        return assignment_n();

    if (strcmp(lookahead->tokenType, "IF") == 0)
        return if_statement_n();

    if (strcmp(lookahead->tokenType, "WHILE") == 0)
        return while_statement_n();

    if (strcmp(lookahead->tokenType, "WRITE") == 0)
        return write_statement_n(); // NEW

    syntax_error_n("statement");
    return NULL;
}

// write_statement ::= WRITE ( expression | IDENT )
ASTNode_n* write_statement_n() {
    ASTNode_n* node = createASTNode_n("write", NULL, lookahead->line, lookahead->column);
    match_n("WRITE");
    match_n("LPAREN");

    ASTNode_n* arg = expression_n(); // handles INTEGER or IDENT or more complex expressions
    addChild_n(node, arg);

    match_n("RPAREN");
    return node;
}



ASTNode_n* if_statement_n() {
    ASTNode_n* node = createASTNode_n("if_statement", NULL, lookahead->line, lookahead->column);

    match_n("IF");
    match_n("LPAREN");
    ASTNode_n* cond = condition_n();
    addChild_n(node, cond);
    match_n("RPAREN");

    match_n("THEN");

    match_n("LBRACE");
    ASTNode_n* thenPart = statement_list_n();
    addChild_n(node, thenPart);
    match_n("RBRACE");

    // optional else
    if (lookahead && strcmp(lookahead->tokenType, "ELSE") == 0) {
        match_n("ELSE");
        match_n("LBRACE");
        ASTNode_n* elsePart = statement_list_n();
        addChild_n(node, elsePart);
        match_n("RBRACE");
    }

    return node;
}



// var_decl ::= local IDENT : (integer | float)
ASTNode_n* var_decl_n() {
    ASTNode_n* node = createASTNode_n("var_decl", NULL, lookahead->line, lookahead->column);

    match_n("LOCAL");
    node->value = strdup(lookahead->lexeme); // variable name
    match_n("IDENTIFIER");
    match_n("COLON");

    if (strcmp(lookahead->tokenType, "INTEGER_TYPE") == 0) {
        node->type = NODE_DECL;
        match_n("INTEGER_TYPE");
    } else if (strcmp(lookahead->tokenType, "FLOAT_TYPE") == 0) {
        node->type = NODE_DECL;
        match_n("FLOAT_TYPE");
    } else {
        syntax_error_n("integer | float");
    }

    return node;
}


// assignment ::= IDENT := expression
ASTNode_n* assignment_n() {
    ASTNode_n* node = createASTNode_n("assignment", lookahead->lexeme, lookahead->line, lookahead->column);
    match_n("IDENTIFIER");
    match_n("ASSIGN");
    ASTNode_n* expr = expression_n();
    addChild_n(node, expr);
    return node;
}

// expression ::= term { (+|-) term }
ASTNode_n* expression_n() {
    ASTNode_n* node = term_n();
    while (lookahead && (strcmp(lookahead->tokenType, "PLUS") == 0 || strcmp(lookahead->tokenType, "MINUS") == 0)) {
        ASTNode_n* op = createASTNode_n(lookahead->tokenType, lookahead->lexeme, lookahead->line, lookahead->column);
        match_n(lookahead->tokenType);
        addChild_n(op, node);
        addChild_n(op, term_n());
        node = op;
    }
    return node;
}

// term ::= factor
ASTNode_n* term_n() {
    ASTNode_n* node = factor_n();

    while (lookahead &&
          (strcmp(lookahead->tokenType, "TIMES") == 0 ||
           strcmp(lookahead->tokenType, "DIVIDE") == 0)) {

        ASTNode_n* op = createASTNode_n(lookahead->tokenType,
                                        lookahead->lexeme,
                                        lookahead->line,
                                        lookahead->column);

        match_n(lookahead->tokenType);

        addChild_n(op, node);
        addChild_n(op, factor_n());
        node = op;
    }

    return node;
}

ASTNode_n* condition_n() {
    ASTNode_n* left = expression_n();

    if (!(strcmp(lookahead->tokenType, "GT") == 0 ||
          strcmp(lookahead->tokenType, "LT") == 0)) {
        syntax_error_n("comparison operator");
    }

    ASTNode_n* op = createASTNode_n(lookahead->tokenType,
                                    lookahead->lexeme,
                                    lookahead->line,
                                    lookahead->column);

    match_n(lookahead->tokenType);

    ASTNode_n* right = expression_n();

    addChild_n(op, left);
    addChild_n(op, right);

    return op;
}

ASTNode_n* while_statement_n() {
    ASTNode_n* node = createASTNode_n("while_statement", NULL, lookahead->line, lookahead->column);

    match_n("WHILE");
    match_n("LPAREN");

    ASTNode_n* cond = condition_n();
    addChild_n(node, cond);

    match_n("RPAREN");

    match_n("LBRACE");
    ASTNode_n* body = statement_list_n();
    addChild_n(node, body);
    match_n("RBRACE");

    return node;
}


// factor ::= INTEGER | IDENT | ( expression )
ASTNode_n* factor_n() {
    if (strcmp(lookahead->tokenType, "INTEGER") == 0) {
        ASTNode_n* node = createASTNode_n("INTEGER",
                                          lookahead->lexeme,
                                          lookahead->line,
                                          lookahead->column);
        match_n("INTEGER");
        return node;
    }




    if (strcmp(lookahead->tokenType, "IDENTIFIER") == 0) {
        ASTNode_n* node = createASTNode_n("IDENTIFIER",
                                          lookahead->lexeme,
                                          lookahead->line,
                                          lookahead->column);
        match_n("IDENTIFIER");
        return node;
    }

    if (strcmp(lookahead->tokenType, "LPAREN") == 0) {
        match_n("LPAREN");
        ASTNode_n* node = expression_n();
        match_n("RPAREN");
        return node;
    }

    syntax_error_n("factor");
    return NULL;
}




// ---------------- 3AC Generation ----------------
int labelCount = 0;

char* newLabel() {
    static char buffer[20];
    sprintf(buffer, "L%d", labelCount++);
    return strdup(buffer);
}

void addQuad(char *op, char *arg1, char *arg2, char *result) {
    quadCount++;
    quadArray = (Quadruple **) realloc(quadArray, quadCount * sizeof(Quadruple *));
    Quadruple *q = malloc(sizeof(Quadruple));
    q->op = op ? strdup(op) : NULL;
    q->arg1 = arg1 ? strdup(arg1) : NULL;
    q->arg2 = arg2 ? strdup(arg2) : NULL;
    q->result = result ? strdup(result) : NULL;
    quadArray[quadCount - 1] = q;
}

char* generate3AC(ASTNode_n* node, char* currentScope) {
    if (!node) return NULL;

    /* ---------- Leaf nodes ---------- */
    if (strcmp(node->kind, "INTEGER") == 0 ||
        strcmp(node->kind, "IDENTIFIER") == 0) {
        return strdup(node->value);
    }

    /* ---------- Arithmetic ---------- */
    if (!strcmp(node->kind, "PLUS") ||
        !strcmp(node->kind, "MINUS") ||
        !strcmp(node->kind, "TIMES") ||
        !strcmp(node->kind, "DIVIDE")) {

        char* left = generate3AC(node->children[0], currentScope);
        char* right = generate3AC(node->children[1], currentScope);
        char* temp = newTemp();

        // Normal 3AC
        printf("%s = %s %s %s\n", temp, left, node->value, right);

        // Quadruple
        addQuad(node->value, left, right, temp);

        free(left);
        free(right);
        return temp;
    }

    /* ---------- Assignment ---------- */
    if (!strcmp(node->kind, "assignment")) {
        char* rhs = generate3AC(node->children[0], currentScope);

        // Normal 3AC
        printf("%s = %s\n", node->value, rhs);

        // Quadruple
        addQuad("=", rhs, NULL, node->value);

        free(rhs);
        return NULL;
    }

    /* ---------- IF / IF-ELSE ---------- */
    if (!strcmp(node->kind, "if_statement")) {
        char* Ltrue = newLabel();
        char* Lfalse = newLabel();
        char* Lend = newLabel();

        ASTNode_n* cond = node->children[0];
        char* left = generate3AC(cond->children[0], currentScope);
        char* right = generate3AC(cond->children[1], currentScope);

        // Normal 3AC
        printf("if %s %s %s goto %s\n", left, cond->value, right, Ltrue);
        printf("goto %s\n", Lfalse);

        // Quad
        addQuad(cond->value, left, right, Ltrue);
        addQuad("goto", NULL, NULL, Lfalse);

        // Ltrue:
        printf("%s:\n", Ltrue);
        addQuad("label", NULL, NULL, Ltrue);
        generate3AC(node->children[1], currentScope);
        printf("goto %s\n", Lend);
        addQuad("goto", NULL, NULL, Lend);

        // Lfalse:
        printf("%s:\n", Lfalse);
        addQuad("label", NULL, NULL, Lfalse);
        if (node->childCount == 3) {
            generate3AC(node->children[2], currentScope);
        }

        // Lend:
        printf("%s:\n", Lend);
        addQuad("label", NULL, NULL, Lend);

        free(left);
        free(right);
        return NULL;
    }

    /* ---------- WHILE LOOP ---------- */
    if (!strcmp(node->kind, "while_statement")) {
        char* Lstart = newLabel();
        char* Lbody = newLabel();
        char* Lend = newLabel();

        printf("%s:\n", Lstart);
        addQuad("label", NULL, NULL, Lstart);

        ASTNode_n* cond = node->children[0];
        char* left = generate3AC(cond->children[0], currentScope);
        char* right = generate3AC(cond->children[1], currentScope);

        printf("if %s %s %s goto %s\n", left, cond->value, right, Lbody);
        printf("goto %s\n", Lend);
        addQuad(cond->value, left, right, Lbody);
        addQuad("goto", NULL, NULL, Lend);

        printf("%s:\n", Lbody);
        addQuad("label", NULL, NULL, Lbody);
        generate3AC(node->children[1], currentScope);

        printf("goto %s\n", Lstart);
        addQuad("goto", NULL, NULL, Lstart);

        printf("%s:\n", Lend);
        addQuad("label", NULL, NULL, Lend);

        free(left);
        free(right);
        return NULL;
    }

    /* ---------- Statement list ---------- */
    for (int i = 0; i < node->childCount; i++) {
        generate3AC(node->children[i], currentScope);
    }

    return NULL;
}




void printQuadruples() {
    printf("\n--- Quadruples ---\n");
    printf("%-5s %-10s %-10s %-10s\n", "Op", "Arg1", "Arg2", "Result");
    for (int i = 0; i < quadCount; i++) {
        Quadruple *q = quadArray[i];
        printf("%-5s %-10s %-10s %-10s\n",
               q->op ? q->op : "",
               q->arg1 ? q->arg1 : "",
               q->arg2 ? q->arg2 : "",
               q->result ? q->result : "");
    }
}

void freeQuadruples() {
    for (int i = 0; i < quadCount; i++) {
        Quadruple *q = quadArray[i];
        if (q->op) free(q->op);
        if (q->arg1) free(q->arg1);
        if (q->arg2) free(q->arg2);
        if (q->result) free(q->result);
        free(q);
    }
    free(quadArray);
}

//symbol table
// Traverse AST and add declared variables to symbol table
void traverseAST(ASTNode_n* node, char* currentScope) {
    if (!node) return;

    if (strcmp(node->kind, "var_decl") == 0) {
        addSymbol(node->value, "integer", currentScope);
    }

    for (int i = 0; i < node->childCount; i++)
        traverseAST(node->children[i], currentScope);
}


// ---------------- Main ----------------
int main() {
    // Open derivation file
    derivation = fopen("derivation.txt", "w");
    if (!derivation) {
        fprintf(stderr, "Failed to open derivation.txt\n");
        return 1;
    }

    // Initialize symbol table
    symbolTable = createSymbolTable();

    // Start lexical analysis
    nextToken_n();

    // Parse the program and generate AST
    ASTNode_n* root = prog_n();

    // Check for extra tokens
    if (lookahead != NULL) {
        fprintf(stderr, "Syntax error: extra tokens at end starting at '%s' (line %d, col %d)\n",
                lookahead->lexeme, lookahead->line, lookahead->column);
        fclose(derivation);
        freeAST_n(root);
        freeSymbolTable();
        return 1;
    }



    // Print AST
    printf("\n--- AST ---\n");
    printAST_n(root, 0);

     // Generate symbol table (traverse AST first)
    traverseAST(root, "main");



    // Generate 3AC
    printf("\n--- 3AC ---\n");
    generate3AC(root, "main");
    // Print quadruples
    printQuadruples();

    calculateOffsets();

    printf("\n--- Updated Symbol table ---\n");
    // Print symbol table

    printSymbolTable();


    generateASM(root);

    // Cleanup
    fclose(derivation);
    freeAST_n(root);
    freeQuadruples();
    freeSymbolTable();

    return 0;
}
