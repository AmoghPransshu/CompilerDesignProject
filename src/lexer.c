/*
 * lexer.c - Lexer implementation
 *
 * The lexer scans an expression and produces tokens.
 * For identifiers and numbers it uses the DFA module.
 * For operators and parentheses it just matches the character.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"
#include "dfa.h"

static const char *src;   /* the input string */
static int pos;            /* current position */

/* read an identifier using the DFA */
static Token read_id(void)
{
    Token t;
    int start = pos;

    while (src[pos] && (isalpha((unsigned char)src[pos]) ||
                        isdigit((unsigned char)src[pos]))) {
        pos++;
    }

    int len = pos - start;
    if (len >= MAX_LEXEME) len = MAX_LEXEME - 1;

    char buf[MAX_LEXEME];
    strncpy(buf, src + start, len);
    buf[len] = '\0';

    if (id_accepts(buf)) {
        t.type = TK_ID;
        strncpy(t.lexeme, buf, MAX_LEXEME);
    } else {
        t.type = TK_INVALID;
        t.lexeme[0] = '\0';
    }
    t.pos = start;
    return t;
}

/* read a number using the DFA */
static Token read_num(void)
{
    Token t;
    int start = pos;

    while (src[pos] && isdigit((unsigned char)src[pos])) {
        pos++;
    }

    int len = pos - start;
    if (len >= MAX_LEXEME) len = MAX_LEXEME - 1;

    char buf[MAX_LEXEME];
    strncpy(buf, src + start, len);
    buf[len] = '\0';

    if (num_accepts(buf)) {
        t.type = TK_NUM;
        strncpy(t.lexeme, buf, MAX_LEXEME);
    } else {
        t.type = TK_INVALID;
        t.lexeme[0] = '\0';
    }
    t.pos = start;
    return t;
}

/* ---- public API ---- */

void lex_init(const char *s)
{
    src = s;
    pos = 0;
}

Token lex_next(void)
{
    Token t;

    /* skip whitespace */
    while (src[pos] && isspace((unsigned char)src[pos])) pos++;

    if (src[pos] == '\0') {
        t.type = TK_END;
        t.lexeme[0] = '\0';
        t.pos = pos;
        return t;
    }

    char c = src[pos];

    if (isalpha((unsigned char)c)) return read_id();
    if (isdigit((unsigned char)c)) return read_num();

    /* single-char tokens */
    pos++;
    switch (c) {
        case '+': t.type = TK_PLUS;   break;
        case '-': t.type = TK_MINUS;  break;
        case '*': t.type = TK_MUL;    break;
        case '/': t.type = TK_DIV;    break;
        case '(': t.type = TK_LPAREN; break;
        case ')': t.type = TK_RPAREN; break;
        default:  t.type = TK_INVALID;
                   t.lexeme[0] = c;
                   t.lexeme[1] = '\0';
                   break;
    }
    t.pos = pos - 1;
    return t;
}

const char *tk_name(TokenType t)
{
    switch (t) {
        case TK_ID:      return "ID";
        case TK_NUM:     return "NUM";
        case TK_PLUS:    return "PLUS";
        case TK_MINUS:   return "MINUS";
        case TK_MUL:     return "MUL";
        case TK_DIV:     return "DIV";
        case TK_LPAREN:  return "LPAREN";
        case TK_RPAREN:  return "RPAREN";
        case TK_INVALID: return "INVALID";
        case TK_END:     return "END";
    }
    return "UNKNOWN";
}

void lex_print(Token t)
{
    if (t.type == TK_END) {
        printf("<END, $>\n");
    } else if (t.type == TK_INVALID) {
        printf("<INVALID, '%s'>\n", t.lexeme);
    } else {
        printf("<%s, %s>\n", tk_name(t.type), t.lexeme);
    }
}

void lex_tokenize(const char *expr)
{
    printf("\n  Input: \"%s\"\n\n", expr);

    lex_init(expr);

    printf("  Tokens:\n");
    Token t;
    int count = 0;
    bool error = false;

    while ((t = lex_next()).type != TK_END) {
        printf("    ");
        lex_print(t);
        count++;
        if (t.type == TK_INVALID) error = true;
    }

    printf("  Total: %d tokens\n", count);
    if (error) printf("  *** invalid token(s) found ***\n");
    printf("\n");
}
