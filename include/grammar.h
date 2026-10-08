/*
 * grammar.h - Grammar definitions
 *
 * The grammar for my expression language:
 *   E -> E + T | E - T | T
 *   T -> T * F | T / F | F
 *   F -> ( E ) | id | num
 *
 * I store productions as actual structs so the code can work with them,
 * not just print them as text.
 */

#ifndef GRAMMAR_H
#define GRAMMAR_H

#include <stdbool.h>

#define MAX_NT   10
#define MAX_T    20
#define MAX_PROD 30
#define MAX_RHS  5
#define MAX_NAME 32

typedef enum { NT = 0, T = 1 } SymType;

typedef struct {
    SymType type;
    char    name[MAX_NAME];
    int     idx;   /* 0..nt_count-1 for non-terminals, nt_count.. for terminals */
} Symbol;

typedef struct {
    Symbol  lhs;
    Symbol  rhs[MAX_RHS];
    int     rhs_len;
} Prod;

typedef struct {
    Symbol  nts[MAX_NT];
    int     nt_cnt;

    Symbol  ts[MAX_T];
    int     t_cnt;

    Prod    prods[MAX_PROD];
    int     prod_cnt;

    Symbol  start;  /* start symbol */
} Grammar;

/* initialise the grammar with standard productions */
void gram_init(Grammar *g);

/* print the grammar */
void gram_print(const Grammar *g);

/* find a symbol by name */
const Symbol *gram_find(const Grammar *g, const char *name);

/* print non-terminals */
void gram_print_nts(const Grammar *g);

/* print terminals */
void gram_print_ts(const Grammar *g);

#endif
