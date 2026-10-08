/*
 * lexer.h - Lexer interface
 *
 * The lexer takes a string and breaks it into tokens.
 * It uses the DFA module to recognise identifiers and numbers.
 */

#ifndef LEXER_H
#define LEXER_H

/* token types */
typedef enum {
    TK_ID = 0,
    TK_NUM,
    TK_PLUS,
    TK_MINUS,
    TK_MUL,
    TK_DIV,
    TK_LPAREN,
    TK_RPAREN,
    TK_INVALID,
    TK_END
} TokenType;

#define MAX_LEXEME 128

typedef struct {
    TokenType type;
    char      lexeme[MAX_LEXEME];
    int       pos;       /* position in the original string */
} Token;

/* initialise lexer with a string */
void lex_init(const char *src);

/* get next token */
Token lex_next(void);

/* print a token nicely */
void lex_print(Token t);

/* tokenize a whole expression and print the result */
void lex_tokenize(const char *expr);

#endif
