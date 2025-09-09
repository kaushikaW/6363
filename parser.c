#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"

extern Token *yylex();

Token *lookahead; // current token
FILE *derivation; // output file for derivation

// Forward declarations
void prog();

void classOrImplOrFuncList();

void classOrImplOrFunc();

void classDecl();

void implDef();

void InheritanceOpt();

void MemberList();

void visibility();

void memberDecl();

void attributeDecl();

void funcDecl();

void funcHead();

void fParams();

void fParamsTailList();

void fParamsTail();

void returnType();

void arraySizeList();

void varDecl();

void type();

void funcDef();

void funcBody();

void varDeclOrStmtList();

void varDeclOrStmt();

void localVarDecl();

void statement();

void assignStat();

void variable();

void idOrSelf();

void indiceList();

void indice();

void expr();

void arithExpr();

void arithExpr_();

void term();

void factor();

void addOp();

void idOrSelfTail();

void functionCall();

void aParams();

void idNestTail();

void assignOp();

void term_();

void relOp();

void multOp();

void FuncDefList();

void aParamsTailList();

void statementTail();

void statementList();

void statBlock();

void relExpr();

void idOrSelfStatement();

void arraySize();

void idOrSelfTailWithAssignOrCall();


static void nextToken() {
    if (lookahead) {
        free(lookahead->tokenType);
        free(lookahead->lexeme);
        free(lookahead);
    }

    lookahead = yylex();
    if (lookahead) {
        printf("TOKEN: %-15s Lexeme: %-10s\n", lookahead->tokenType, lookahead->lexeme);
    }
}


static void syntax_error(const char *expected) {
    if (lookahead) {
        fprintf(stderr, "Syntax error: expected %s but found %s (lexeme '%s') at line %d, col %d\n",
                expected, lookahead->tokenType, lookahead->lexeme, lookahead->line, lookahead->column);
    } else {
        fprintf(stderr, "Syntax error: expected %s but found EOF\n", expected);
    }
    exit(1);
}

static void match(const char *expectedType) {
    if (lookahead && strcmp(lookahead->tokenType, expectedType) == 0) {
        nextToken();
    } else {
        syntax_error(expectedType);
    }
}

// Grammar rules

void prog() {
    fprintf(derivation, "prog -> classOrImplOrFuncList\n");
    classOrImplOrFuncList();
}

void classOrImplOrFuncList() {
    if (lookahead &&
        (strcmp(lookahead->tokenType, "CLASS") == 0 ||
         strcmp(lookahead->tokenType, "IMPLEMENT") == 0 ||
         strcmp(lookahead->tokenType, "FUNC") == 0)) {
        fprintf(derivation, "classOrImplOrFuncList -> classOrImplOrFunc classOrImplOrFuncList\n");
        classOrImplOrFunc();
        classOrImplOrFuncList();
    } else {
        fprintf(derivation, "classOrImplOrFuncList -> ε\n");
    }
}


void classOrImplOrFunc() {
    if (lookahead == NULL) {
        syntax_error("classOrImplOrFunc (classDecl, implDef, or funcDef)");
        return;
    }

    if (strcmp(lookahead->tokenType, "CLASS") == 0) {
        fprintf(derivation, "classOrImplOrFunc -> classDecl\n");
        classDecl();
    } else if (strcmp(lookahead->tokenType, "IMPLEMENT") == 0) {
        fprintf(derivation, "classOrImplOrFunc -> implDef\n");
        implDef();
    } else if (strcmp(lookahead->tokenType, "FUNC") == 0 ||
               strcmp(lookahead->tokenType, "CONSTRUCTOR") == 0) {
        fprintf(derivation, "classOrImplOrFunc -> funcDef\n");
        funcDef();
    } else {
        syntax_error("classOrImplOrFunc (expected class, implement, or function)");
    }
}


void classDecl() {
    fprintf(derivation, "classDecl -> 'class' 'id' InheritanceOpt '{' MemberList '}' ';'\n");
    match("CLASS");
    match("VARIABLE");
    InheritanceOpt();
    match("LBRACE");
    MemberList();
    match("RBRACE");
    match("SEMICOLON");
}

void InheritanceOpt() {
    fprintf(derivation, "InheritanceOpt -> ε\n");
}

// arraySize → '[' 'intLit' ']' | '[' ']'
void arraySize() {
    if (!lookahead) return;

    if (strcmp(lookahead->tokenType, "LBRACKET") == 0) {
        match("LBRACKET");

        if (strcmp(lookahead->tokenType, "INTEGER") == 0) {
            fprintf(derivation, "arraySize -> '[' intLit ']'\n");
            match("INTEGER");
            match("RBRACKET");
        } else {
            fprintf(derivation, "arraySize -> '[' ']'\n");
            match("RBRACKET");
        }

    } else {
        fprintf(stderr, "Syntax error in arraySize: expected '[' but found %s\n", lookahead->tokenType);
        exit(1);
    }
}



void MemberList() {
    if (lookahead &&
        (strcmp(lookahead->tokenType, "PUBLIC") == 0 ||
         strcmp(lookahead->tokenType, "PRIVATE") == 0)) {
        fprintf(derivation, "MemberList -> visibility memberDecl MemberList\n");

        visibility();
        memberDecl();
        MemberList();
    } else {
        fprintf(derivation, "MemberList -> ε\n");
    }
}

void visibility() {
    if (lookahead && strcmp(lookahead->tokenType, "PUBLIC") == 0) {
        match("PUBLIC");
        fprintf(derivation, "visibility -> public\n");
    } else if (lookahead && strcmp(lookahead->tokenType, "PRIVATE") == 0) {
        match("PRIVATE");
        fprintf(derivation, "visibility -> private\n");
    } else {
        syntax_error("visibility ('public' or 'private')");
    }
}

// implDef -> 'implement' 'id' '{' FuncDefList '}'
void implDef() {
    fprintf(derivation, "implDef -> 'implement' 'id' '{' FuncDefList '}'\n");
    match("IMPLEMENT");
    match("VARIABLE"); // the class name
    match("LBRACE");
    FuncDefList(); // <-- call FuncDefList, not just funcDef
    match("RBRACE");
}

//FuncDefList         → funcDef FuncDefList | ε

void FuncDefList() {
    if (lookahead &&
        (strcmp(lookahead->tokenType, "FUNC") == 0 ||
         strcmp(lookahead->tokenType, "CONSTRUCTOR") == 0)) {
        fprintf(derivation, "FuncDefList -> funcDef FuncDefList\n");
        funcDef();
        FuncDefList();
    } else {
        fprintf(derivation, "FuncDefList -> ε\n");
    }
}

void memberDecl() {
    if (lookahead &&
        (strcmp(lookahead->tokenType, "FUNC") == 0 ||
         strcmp(lookahead->tokenType, "CONSTRUCTOR") == 0)) {
        fprintf(derivation, "memberDecl -> funcDecl\n");
        funcDecl();
    } else if (lookahead && strcmp(lookahead->tokenType, "ATTRIBUTE") == 0) {
        fprintf(derivation, "memberDecl -> attributeDecl\n");
        attributeDecl();
    } else {
        syntax_error("memberDecl (funcDecl or attributeDecl)");
    }
}

void funcDecl() {
    fprintf(derivation, "funcDecl -> funcHead ;\n");
    funcHead();
    match("SEMICOLON");
}

/* attributeDecl → 'attribute' varDecl */
void attributeDecl() {
    fprintf(derivation, "attributeDecl -> 'attribute' varDecl\n");
    match("ATTRIBUTE");
    varDecl();
}

/* varDecl → 'id' ':' type arraySizeList ';' */
void varDecl() {
    fprintf(derivation, "varDecl -> 'id' ':' type arraySizeList ';'\n");

    match("VARIABLE");
    match("COLON");
    type();
    arraySizeList();
    match("SEMICOLON");
}

/* funcDef → funcHead funcBody */
void funcDef() {
    fprintf(derivation, "funcDef -> funcHead funcBody\n");
    funcHead();
    funcBody();
}

/* funcBody -> '{' varDeclOrStmtList '}' */
void funcBody() {
    fprintf(derivation, "funcBody -> '{' varDeclOrStmtList '}'\n");
    match("LBRACE");
    varDeclOrStmtList();
    match("RBRACE");
}

/* varDeclOrStmtList → varDeclOrStmt varDeclOrStmtList  | ε */
void varDeclOrStmtList() {
    if (lookahead &&
        (strcmp(lookahead->tokenType, "LOCAL") == 0 ||
         strcmp(lookahead->tokenType, "VARIABLE") == 0 ||
         strcmp(lookahead->tokenType, "IF") == 0 ||
         strcmp(lookahead->tokenType, "WHILE") == 0 ||
         strcmp(lookahead->tokenType, "READ") == 0 ||
         strcmp(lookahead->tokenType, "WRITE") == 0 ||
         strcmp(lookahead->tokenType, "RETURN") == 0 ||
         strcmp(lookahead->tokenType, "FUNC") == 0 || // function call start
         strcmp(lookahead->tokenType, "CONSTRUCTOR") == 0 ||
         strcmp(lookahead->tokenType, "SELF") == 0
        )
    ) {
        fprintf(derivation, "varDeclOrStmtList -> varDeclOrStmt varDeclOrStmtList\n");
        varDeclOrStmt();
        varDeclOrStmtList();
    } else {
        fprintf(derivation, "varDeclOrStmtList -> ε\n"); // empty body
    }
}

/* varDeclOrStmt → localVarDecl | statement */
void varDeclOrStmt() {
    if (lookahead && strcmp(lookahead->tokenType, "LOCAL") == 0) {
        fprintf(derivation, "varDeclOrStmt → localVarDecl\n");
        localVarDecl();
    } else {
        fprintf(derivation, "varDeclOrStmt → statement\n");
        statement(); // TODO
    }
}

/* localVarDecl → 'local' varDecl */
void localVarDecl() {
    fprintf(derivation, "localVarDecl -> 'local' varDecl\n");
    match("LOCAL");
    varDecl();
}

// arraySizeList → arraySize arraySizeList | ε
void arraySizeList() {
    if (!lookahead) return;

    if (strcmp(lookahead->tokenType, "LBRACKET") == 0) {
        fprintf(derivation, "arraySizeList -> arraySize arraySizeList\n");
        arraySize();
        arraySizeList();
    } else {
        fprintf(derivation, "arraySizeList -> ε\n");
    }
}

void type() {
    if (lookahead) {
        if (strcmp(lookahead->tokenType, "INTEGER_TYPE") == 0) {
            fprintf(derivation, "type -> integer\n");
            match("INTEGER_TYPE");
        } else if (strcmp(lookahead->tokenType, "FLOAT_TYPE") == 0) {
            fprintf(derivation, "type -> float\n");
            match("FLOAT_TYPE");
        } else if (strcmp(lookahead->tokenType, "VARIABLE") == 0) {
            fprintf(derivation, "type -> id\n");
            match("VARIABLE"); // user-defined type name
        } else {
            syntax_error("type (integer, float, or id)");
        }
    } else {
        syntax_error("type (integer, float, or id)");
    }
}

/* funcHead → 'func' 'id' '(' fParams ')' => returnType | 'constructor' '(' fParams ') */
void funcHead() {
    if (lookahead && strcmp(lookahead->tokenType, "FUNC") == 0) {
        fprintf(derivation, "funcHead -> 'func' 'id' '(' fParams ')' '=>' returnType\n");

        match("FUNC");
        match("VARIABLE");
        match("LPAREN");
        fParams();
        match("RPAREN");
        match("ARROW");
        returnType();
    } else if (lookahead && strcmp(lookahead->tokenType, "CONSTRUCTOR") == 0) {
        fprintf(derivation, "funcHead -> 'constructor' '(' fParams ')'\n");

        match("CONSTRUCTOR");
        match("LPAREN");
        fParams();
        match("RPAREN");
    } else {
        syntax_error("funcHead (func|constructor)");
    }
}


/* fParams → 'id' ':' type arraySizeList fParamsTailList | ε */
void fParams() {
    if (lookahead && strcmp(lookahead->tokenType, "VARIABLE") == 0) {
        fprintf(derivation, "fParams -> 'id' ':' type arraySizeList fParamsTailList\n");
        match("VARIABLE");
        match("COLON");
        type();
        arraySizeList(); // TODO
        fParamsTailList();
    } else {
        fprintf(derivation, "fParams -> ε\n");
    }
}

/* fParamsTailList → fParamsTail fParamsTailList | ε */
void fParamsTailList() {
    if (lookahead && strcmp(lookahead->tokenType, "COMMA") == 0) {
        fprintf(derivation, "fParamsTailList -> fParamsTail fParamsTailList\n");
        fParamsTail();
        fParamsTailList();
    } else {
        fprintf(derivation, "fParamsTailList -> ε\n");
    }
}

/* fParamsTail → ',' 'id' ':' type arraySizeList */
void fParamsTail() {
    fprintf(derivation, "fParamsTail -> ',' 'id' ':' type arraySizeList\n");
    match("COMMA");
    match("VARIABLE");
    match("COLON");
    type();
    arraySizeList(); // TODO
}

void returnType() {
    if (lookahead &&
        (strcmp(lookahead->tokenType, "INTEGER_TYPE") == 0 ||
         strcmp(lookahead->tokenType, "FLOAT_TYPE") == 0 ||
         strcmp(lookahead->tokenType, "VARIABLE") == 0)) {
        fprintf(derivation, "returnType -> type\n");
        match(lookahead->tokenType);
    } else if (lookahead && strcmp(lookahead->tokenType, "VOID") == 0) {
        fprintf(derivation, "returnType -> void\n");
        match("VOID");
    } else {
        syntax_error("returnType (type|void)");
    }
}

/* statement           → 'if' '(' relExpr ')' 'then' statBlock 'else' statBlock ';'
                    | 'while' '(' relExpr ')' statBlock ';'
                    | 'read' '(' variable ')' ';'
                    | 'write' '(' expr ')' ';'
                    | 'return' '(' expr ')' ';'
                    | idOrSelf idOrSelfTail statementTail
*/


void statement() {
    if (!lookahead) return;

    if (strcmp(lookahead->tokenType, "READ") == 0) {
        fprintf(derivation, "statement -> 'read' '(' variable ')' ';'\n");
        match("READ");
        match("LPAREN");
        variable();
        match("RPAREN");
        match("SEMICOLON");
    } else if (strcmp(lookahead->tokenType, "WRITE") == 0) {
        fprintf(derivation, "statement -> 'write' '(' expr ')' ';'\n");
        match("WRITE");
        match("LPAREN");
        expr();
        match("RPAREN");
        match("SEMICOLON");
    } else if (strcmp(lookahead->tokenType, "RETURN") == 0) {
        fprintf(derivation, "statement -> 'return' '(' expr ')' ';'\n");
        match("RETURN");
        match("LPAREN");
        expr();
        match("RPAREN");
        match("SEMICOLON");
    } else if (strcmp(lookahead->tokenType, "IF") == 0) {
        fprintf(derivation, "statement -> 'if' '(' relExpr ')' 'then' statBlock 'else' statBlock ';'\n");
        match("IF");
        match("LPAREN");
        relExpr();
        match("RPAREN");
        match("THEN");
        statBlock(); // statBlock
        match("ELSE");
        statBlock(); // statBlock
        match("SEMICOLON");
    } else if (strcmp(lookahead->tokenType, "WHILE") == 0) {
        fprintf(derivation, "statement -> 'while' '(' relExpr ')' statBlock ';'\n");
        match("WHILE");
        match("LPAREN");
        relExpr();
        match("RPAREN");
        statBlock();
        match("SEMICOLON");
    } else if (strcmp(lookahead->tokenType, "VARIABLE") == 0 || strcmp(lookahead->tokenType, "SELF") == 0) {
        // Start of assignStat or functionCall
        fprintf(derivation, "statement -> idOrSelfStatement ;\n");
        idOrSelfStatement();
        match("SEMICOLON");
    } else {
        fprintf(stderr, "Syntax error in statement: unexpected token %s\n", lookahead->tokenType);
        exit(1);
    }
}

//idOrSelfStatement   → idOrSelf idOrSelfTailWithAssignOrCall
void idOrSelfStatement() {
    fprintf(derivation, "idOrSelfStatement   → idOrSelf idOrSelfTailWithAssignOrCall\n");
    idOrSelf();
    idOrSelfTailWithAssignOrCall();
}

/*
idOrSelfTailWithAssignOrCall → assignOp expr
                             | '(' aParams ')' idNestTail
                             | indiceList idNestTail
 */

void idOrSelfTailWithAssignOrCall() {
    if (strcmp(lookahead->tokenType, "ASSIGN") == 0) {
        // :=
        fprintf(derivation, "idOrSelfTailWithAssignOrCall -> assignOp expr\n");
        assignOp();
        expr();
    } else if (strcmp(lookahead->tokenType, "LPAREN") == 0) {
        // function call
        fprintf(derivation, "idOrSelfTailWithAssignOrCall -> '(' aParams ')' idNestTail\n");
        match("LPAREN");
        aParams();
        match("RPAREN");
        idNestTail();
    } else if (strcmp(lookahead->tokenType, "LBRACK") == 0 ||
               strcmp(lookahead->tokenType, "DOT") == 0
    ) {
        // array index
        fprintf(derivation, "idOrSelfTailWithAssignOrCall -> indiceList idNestTail\n");
        indiceList();
        idNestTail();
    } else {
        fprintf(stderr, "Syntax error in idOrSelfTailWithAssignOrCall: unexpected token %s\n", lookahead->tokenType);
        exit(1);
    }
}


// statementTail → '(' aParams ')' idNestTail ';' | assignOp expr ';'
void statementTail() {
}

// assignStat → variable assignOp expr
void assignStat() {
    fprintf(derivation, "assignStat -> variable assignOp expr\n");
    variable();
    assignOp();
    expr();
}

// variable -> idOrSelf idOrSelfTail
void variable() {
    fprintf(derivation, "variable -> idOrSelf idOrSelfTail\n");
    idOrSelf();
    idOrSelfTail();
}

// idOrSelf → 'id' | 'self'
void idOrSelf() {
    if (lookahead && strcmp(lookahead->tokenType, "VARIABLE") == 0) {
        fprintf(derivation, "idOrSelf -> id\n");
        match("VARIABLE");
    } else if (lookahead && strcmp(lookahead->tokenType, "SELF") == 0) {
        fprintf(derivation, "idOrSelf -> self\n");
        match("SELF");
    } else {
        syntax_error("idOrSelf ('id' or 'self')");
    }
}

// indiceList → indice indiceList | ε
void indiceList() {
    if (lookahead && strcmp(lookahead->tokenType, "LBRACKET") == 0) {
        fprintf(derivation, "indiceList -> indice indiceList\n");
        indice();
        indiceList();
    } else {
        fprintf(derivation, "indiceList -> ε\n");
    }
}

// indice → '[' arithExpr ']'
void indice() {
    fprintf(derivation, "indice -> '[' arithExpr ']'\n");
    match("LBRACKET");
    arithExpr(); // TODO: implement arithmetic expression parsing
    match("RBRACKET");
}


// expr → arithExpr | relExpr
void expr() {
    fprintf(derivation, "expr -> arithExpr\n");
    arithExpr();

    // After arithExpr, check if it extends into a relExpr
    if (lookahead &&
        (strcmp(lookahead->tokenType, "EQ") == 0 ||
         strcmp(lookahead->tokenType, "NEQ") == 0 ||
         strcmp(lookahead->tokenType, "LT") == 0 ||
         strcmp(lookahead->tokenType, "LEQ") == 0 ||
         strcmp(lookahead->tokenType, "GT") == 0 ||
         strcmp(lookahead->tokenType, "GEQ") == 0)) {
        fprintf(derivation, "expr -> relExpr\n");
        relOp();
        arithExpr();
    }
}

// relExpr → arithExpr relOp arithExpr
void relExpr() {
    fprintf(derivation, "relExpr -> arithExpr relOp arithExpr\n");
    arithExpr();
    relOp();
    arithExpr();
}

// arithExpr → term arithExpr_
void arithExpr() {
    fprintf(derivation, "arithExpr -> term arithExpr_\n");
    term();
    arithExpr_();
}

void arithExpr_() {
    if (lookahead &&
        (strcmp(lookahead->tokenType, "PLUS") == 0 ||
         strcmp(lookahead->tokenType, "MINUS") == 0 ||
         strcmp(lookahead->tokenType, "OR") == 0)) {
        fprintf(derivation, "arithExpr_ -> addOp term arithExpr_\n");
        addOp();
        term();
        arithExpr_();
    } else {
        fprintf(derivation, "arithExpr_ -> ε\n"); // no further addition/subtraction
    }
}

void addOp() {
    if (lookahead && strcmp(lookahead->tokenType, "PLUS") == 0) {
        fprintf(derivation, "addOp -> '+'\n");
        match("PLUS");
    } else if (lookahead && strcmp(lookahead->tokenType, "MINUS") == 0) {
        fprintf(derivation, "addOp -> '-'\n");
        match("MINUS");
    } else if (lookahead && strcmp(lookahead->tokenType, "OR") == 0) {
        fprintf(derivation, "addOp -> 'or'\n");
        match("OR");
    } else {
        syntax_error("addOp ('+', '-', or 'or')");
    }
}

void term() {
    fprintf(derivation, "term -> factor term_\n");
    factor();
    term_();
}

// factor -> idOrSelf idOrSelfTail | 'intLit' | 'floatLit' | '(' arithExpr ')' | 'not' factor | sign factor
void factor() {
    if (lookahead && (strcmp(lookahead->tokenType, "VARIABLE") == 0 || strcmp(lookahead->tokenType, "SELF") == 0)) {
        fprintf(derivation, "factor -> idOrSelf idOrSelfTail\n");
        idOrSelf();
        idOrSelfTail();
    } else if (lookahead && strcmp(lookahead->tokenType, "INTEGER") == 0) {
        fprintf(derivation, "factor -> 'intLit'\n");
        match("INTEGER");
    } else if (lookahead && strcmp(lookahead->tokenType, "FLOAT") == 0) {
        fprintf(derivation, "factor -> 'floatLit'\n");
        match("FLOAT");
    } else if (lookahead && strcmp(lookahead->tokenType, "LPAREN") == 0) {
        fprintf(derivation, "factor -> '(' arithExpr ')'\n");
        match("LPAREN");
        arithExpr();
        match("RPAREN");
    } else if (lookahead && strcmp(lookahead->tokenType, "NOT") == 0) {
        fprintf(derivation, "factor -> 'not' factor\n");
        match("NOT");
        factor();
    } else if (lookahead && (strcmp(lookahead->tokenType, "PLUS") == 0 || strcmp(lookahead->tokenType, "MINUS") == 0)) {
        fprintf(derivation, "factor -> sign factor\n");
        match(lookahead->tokenType);
        factor();
    } else {
        syntax_error("factor (idOrSelf, intLit, floatLit, or '(' arithExpr ')')");
    }
}

// idOrSelfTail -> '(' aParams ')' idNestTail | indiceList idNestTail | ε
void idOrSelfTail() {
    if (lookahead && strcmp(lookahead->tokenType, "LPAREN") == 0) {
        fprintf(derivation, "idOrSelfTail -> '(' aParams ')' idNestTail\n");
        match("LPAREN");
        aParams(); // TODO: implement aParams parsing
        match("RPAREN");
        idNestTail();
    } else if (lookahead && strcmp(lookahead->tokenType, "LBRACKET") == 0 ||
               lookahead && strcmp(lookahead->tokenType, "DOT") == 0
    ) {
        fprintf(derivation, "idOrSelfTail -> indiceList idNestTail\n");
        indiceList();
        idNestTail();
    } else {
        fprintf(derivation, "idOrSelfTail -> ε\n");
    }
}

// idNestTail -> '.' idOrSelf idOrSelfTail | ε
void idNestTail() {
    if (lookahead && strcmp(lookahead->tokenType, "DOT") == 0) {
        fprintf(derivation, "idNestTail -> '.' idOrSelf idOrSelfTail\n");
        match("DOT");
        idOrSelf();
        idOrSelfTail();
    } else {
        fprintf(derivation, "idNestTail -> ε\n");
    }
}

// functionCall -> idOrSelf '(' aParams ')' idNestTail
void functionCall() {
    fprintf(derivation, "functionCall -> idOrSelf '(' aParams ')' idNestTail\n");
    idOrSelf();
    match("LPAREN");
    aParams(); // TODO: implement
    match("RPAREN");
    idNestTail();
}

void aParams() {
    if (lookahead &&
        (strcmp(lookahead->tokenType, "VARIABLE") == 0 ||
         strcmp(lookahead->tokenType, "SELF") == 0 ||
         strcmp(lookahead->tokenType, "INTEGER") == 0 ||
         strcmp(lookahead->tokenType, "FLOAT") == 0 ||
         strcmp(lookahead->tokenType, "LPAREN") == 0 ||
         strcmp(lookahead->tokenType, "PLUS") == 0 ||
         strcmp(lookahead->tokenType, "MINUS") == 0 ||
         strcmp(lookahead->tokenType, "NOT") == 0)) {
        fprintf(derivation, "aParams -> expr aParamsTailList\n");
        expr();
        aParamsTailList();
    } else {
        fprintf(derivation, "aParams -> ε\n");
    }
}

void aParamsTailList() {
    if (lookahead && strcmp(lookahead->tokenType, "COMMA") == 0) {
        fprintf(derivation, "aParamsTailList -> ',' expr aParamsTailList\n");
        match("COMMA");
        expr();
        aParamsTailList();
    } else {
        fprintf(derivation, "aParamsTailList -> ε\n");
    }
}

void assignOp() {
    fprintf(derivation, "assignOp -> := \n");
    match("ASSIGN");
}

// term_ → multOp factor term_ | ε
void term_() {
    if (lookahead &&
        (strcmp(lookahead->tokenType, "TIMES") == 0 ||
         strcmp(lookahead->tokenType, "DIVIDE") == 0 ||
         strcmp(lookahead->tokenType, "AND") == 0)) {
        fprintf(derivation, "term_ -> multOp factor term_\n");
        multOp();
        factor();
        term_();
    } else {
        fprintf(derivation, "term_ -> ε\n"); // no further multiplication/division/and
    }
}


/* relOp → '==' | '<>' | '<' | '>' | '<=' | '>=' */
void relOp() {
    fprintf(derivation, "relOp -> ");

    if (lookahead == NULL) {
        syntax_error("relOp (unexpected end of input)");
        return;
    }

    if (strcmp(lookahead->tokenType, "EQ") == 0) {
        // '=='
        fprintf(derivation, "==\n");
        match("EQ");
    } else if (strcmp(lookahead->tokenType, "NEQ") == 0) {
        // '<>'
        fprintf(derivation, "<>\n");
        match("NEQ");
    } else if (strcmp(lookahead->tokenType, "LT") == 0) {
        // '<'
        fprintf(derivation, "<\n");
        match("LT");
    } else if (strcmp(lookahead->tokenType, "GT") == 0) {
        // '>'
        fprintf(derivation, ">\n");
        match("GT");
    } else if (strcmp(lookahead->tokenType, "LE") == 0) {
        // '<='
        fprintf(derivation, "<=\n");
        match("LE");
    } else if (strcmp(lookahead->tokenType, "GE") == 0) {
        // '>='
        fprintf(derivation, ">=\n");
        match("GE");
    } else {
        syntax_error("relOp (expected relational operator)");
    }
}


/* multOp → '*' | '/' | 'and' */
void multOp() {
    fprintf(derivation, "multOp -> ");

    if (lookahead == NULL) {
        syntax_error("multOp (unexpected end of input)");
        return;
    }

    if (strcmp(lookahead->tokenType, "TIMES") == 0) {
        // '*'
        fprintf(derivation, "*\n");
        match("TIMES");
    } else if (strcmp(lookahead->tokenType, "DIVIDE") == 0) {
        // '/'
        fprintf(derivation, "/\n");
        match("DIVIDE");
    } else if (strcmp(lookahead->tokenType, "AND") == 0) {
        // 'and'
        fprintf(derivation, "and\n");
        match("AND");
    } else {
        syntax_error("multOp (expected *, /, or 'and')");
    }
}

/* statementList -> statement statementList | ε */
void statementList() {
    if (lookahead &&
        (strcmp(lookahead->tokenType, "IF") == 0 ||
         strcmp(lookahead->tokenType, "WHILE") == 0 ||
         strcmp(lookahead->tokenType, "READ") == 0 ||
         strcmp(lookahead->tokenType, "WRITE") == 0 ||
         strcmp(lookahead->tokenType, "RETURN") == 0 ||
         strcmp(lookahead->tokenType, "VARIABLE") == 0 ||
         strcmp(lookahead->tokenType, "SELF") == 0)) {
        fprintf(derivation, "statementList -> statement statementList\n");
        statement();
        statementList();
    } else {
        fprintf(derivation, "statementList -> ε\n");
    }
}

void statBlock() {
    if (lookahead && strcmp(lookahead->tokenType, "LBRACE") == 0) {
        /* block form: { statementList } */
        fprintf(derivation, "statBlock -> '{' statementList '}'\n");
        match("LBRACE");
        statementList();
        match("RBRACE");
    } else if (lookahead &&
               (strcmp(lookahead->tokenType, "IF") == 0 ||
                strcmp(lookahead->tokenType, "WHILE") == 0 ||
                strcmp(lookahead->tokenType, "READ") == 0 ||
                strcmp(lookahead->tokenType, "WRITE") == 0 ||
                strcmp(lookahead->tokenType, "RETURN") == 0 ||
                strcmp(lookahead->tokenType, "VARIABLE") == 0 ||
                strcmp(lookahead->tokenType, "SELF") == 0)) {
        fprintf(derivation, "statBlock -> statement\n");
        statement();
    } else {
        fprintf(derivation, "statBlock -> ε\n");
    }
}


// Main driver
int main() {
    derivation = fopen("derivation.txt", "w");
    if (!derivation) {
        perror("derivation.txt");
        return 1;
    }

    nextToken(); // prime the first token
    prog();

    if (lookahead != NULL) {
        fprintf(stderr, "Syntax error: extra tokens at end starting at '%s' (line %d, col %d)\n",
                lookahead->lexeme, lookahead->line, lookahead->column);
        fclose(derivation);
        return 1;
    }

    printf("Parsing successful! Derivation written to derivation.txt\n");
    fclose(derivation);
    return 0;
}
