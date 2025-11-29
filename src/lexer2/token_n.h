#ifndef TOKEN_N_H
#define TOKEN_N_H

typedef struct Token_n {
    char* tokenType;
    char* lexeme;
    int line;
    int column;
} Token_n;

Token_n* create_token_n(const char* tokenType, const char* yytext, int length);

#endif
