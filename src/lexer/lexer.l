%{
#include "token.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define YY_DECL Token* yylex(void)

int line_num = 1;
int col_num = 1;
int token_start_col;

Token* create_token(const char* tokenType, const char* yytext, int length) {
    token_start_col = col_num;
    col_num += length;

    Token* t = malloc(sizeof(Token));
    t->tokenType = strdup(tokenType);
    t->lexeme = strdup(yytext);
    t->line = line_num;
    t->column = token_start_col;
    return t;
}

void update_position(const char* text, int length) {
    for (int i = 0; i < length; i++) {
        if (text[i] == '\n') {
            line_num++;
            col_num = 1;
        } else {
            col_num++;
        }
    }
}
%}

%option noyywrap
%option yylineno

letter     [a-zA-Z]
digit      [0-9]
nonzero    [1-9]
alphanum   ({letter}|{digit}|_)
id         {letter}({alphanum})*

integerNum 0|{nonzero}{digit}*
fraction   \.0|\.{digit}*{nonzero}
exp        [eE][+-]?{integerNum}
floatNum   {integerNum}{fraction}({exp})?

%%

{floatNum}      { return create_token("FLOAT", yytext, yyleng); }
{integerNum}    { return create_token("INTEGER", yytext, yyleng); }

"("             { return create_token("LPAREN", yytext, yyleng); }
")"             { return create_token("RPAREN", yytext, yyleng); }
";"             { return create_token("SEMICOLON", yytext, yyleng); }
","             { return create_token("COMMA", yytext, yyleng); }
"{"             { return create_token("LBRACE", yytext, yyleng); }
"}"             { return create_token("RBRACE", yytext, yyleng); }
"["             { return create_token("LBRACKET", yytext, yyleng); }
"]"             { return create_token("RBRACKET", yytext, yyleng); }
"."             { return create_token("DOT", yytext, yyleng); }
"=>"            { return create_token("ARROW", yytext, yyleng); }
":="            { return create_token("ASSIGN", yytext, yyleng); }
":"             { return create_token("COLON", yytext, yyleng); }

"+"             { return create_token("PLUS", yytext, yyleng); }
"-"             { return create_token("MINUS", yytext, yyleng); }
"*"             { return create_token("TIMES", yytext, yyleng); }
"/"             { return create_token("DIVIDE", yytext, yyleng); }

"=="            { return create_token("EQ", yytext, yyleng); }
"<>"            { return create_token("NEQ", yytext, yyleng); }
"<="            { return create_token("LE", yytext, yyleng); }
">="            { return create_token("GE", yytext, yyleng); }
"<"             { return create_token("LT", yytext, yyleng); }
">"             { return create_token("GT", yytext, yyleng); }

"or"            { return create_token("OR", yytext, yyleng); }
"and"           { return create_token("AND", yytext, yyleng); }
"not"           { return create_token("NOT", yytext, yyleng); }

"else"          { return create_token("ELSE", yytext, yyleng); }
"float"         { return create_token("FLOAT_TYPE", yytext, yyleng); }
"func"          { return create_token("FUNC", yytext, yyleng); }
"if"            { return create_token("IF", yytext, yyleng); }
"implement"     { return create_token("IMPLEMENT", yytext, yyleng); }
"class"         { return create_token("CLASS", yytext, yyleng); }
"attribute"     { return create_token("ATTRIBUTE", yytext, yyleng); }
"isa"           { return create_token("ISA", yytext, yyleng); }
"private"       { return create_token("PRIVATE", yytext, yyleng); }
"public"        { return create_token("PUBLIC", yytext, yyleng); }
"read"          { return create_token("READ", yytext, yyleng); }
"return"        { return create_token("RETURN", yytext, yyleng); }
"then"          { return create_token("THEN", yytext, yyleng); }
"local"         { return create_token("LOCAL", yytext, yyleng); }
"void"          { return create_token("VOID", yytext, yyleng); }
"while"         { return create_token("WHILE", yytext, yyleng); }
"write"         { return create_token("WRITE", yytext, yyleng); }
"integer"       { return create_token("INTEGER_TYPE", yytext, yyleng); }
"self"          { return create_token("SELF", yytext, yyleng); }
"constructor"   { return create_token("CONSTRUCTOR", yytext, yyleng); }

{id}            { return create_token("VARIABLE", yytext, yyleng); }

[ \t]+          { col_num += yyleng; }  /* skip whitespace */
\n              { line_num++; col_num = 1; }

\/\/[^\n]*      { /* comment ignored */ }
\/\*([^*]|\*+[^*/])*\*+\/ {update_position(yytext, yyleng);}

.               { return create_token("ILLEGAL", yytext, yyleng); }

%%

