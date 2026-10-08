/*
 * grammar.c - Grammar implementation
 *
 * Stores the expression grammar as data structures.
 *
 * Grammar:
 *   E -> E + T | E - T | T
 *   T -> T * F | T / F | F
 *   F -> ( E ) | id | num
 *
 * This is an operator precedence grammar because:
 *  - No production has two adjacent non-terminals on RHS
 *  - No epsilon productions
 *  - Left recursion is okay for operator precedence parsing
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "grammar.h"

/* helper to add a production */
static void add_prod(Grammar *g, int idx, const char *lhs,
                     const char *rhs[], int rhs_len)
{
    Prod *p = &g->prods[idx];
    p->lhs.type = NT;
    strncpy(p->lhs.name, lhs, MAX_NAME);
    p->lhs.idx = -1;  /* will be set properly later */

    p->rhs_len = rhs_len;
    for (int i = 0; i < rhs_len; i++) {
        p->rhs[i].type = NT;  /* temporary, fixed below */
        strncpy(p->rhs[i].name, rhs[i], MAX_NAME);
        p->rhs[i].idx = -1;
    }
}

void gram_init(Grammar *g)
{
    memset(g, 0, sizeof(*g));

    /* non-terminals */
    g->nt_cnt = 3;
    strcpy(g->nts[0].name, "E"); g->nts[0].type = NT; g->nts[0].idx = 0;
    strcpy(g->nts[1].name, "T"); g->nts[1].type = NT; g->nts[1].idx = 1;
    strcpy(g->nts[2].name, "F"); g->nts[2].type = NT; g->nts[2].idx = 2;

    /* terminals */
    g->t_cnt = 9;
    strcpy(g->ts[0].name, "id");  g->ts[0].type = T; g->ts[0].idx = 3;
    strcpy(g->ts[1].name, "num"); g->ts[1].type = T; g->ts[1].idx = 4;
    strcpy(g->ts[2].name, "+");   g->ts[2].type = T; g->ts[2].idx = 5;
    strcpy(g->ts[3].name, "-");   g->ts[3].type = T; g->ts[3].idx = 6;
    strcpy(g->ts[4].name, "*");   g->ts[4].type = T; g->ts[4].idx = 7;
    strcpy(g->ts[5].name, "/");   g->ts[5].type = T; g->ts[5].idx = 8;
    strcpy(g->ts[6].name, "(");   g->ts[6].type = T; g->ts[6].idx = 9;
    strcpy(g->ts[7].name, ")");   g->ts[7].type = T; g->ts[7].idx = 10;
    strcpy(g->ts[8].name, "$");   g->ts[8].type = T; g->ts[8].idx = 11;

    /* productions - I defined them based on the standard expression grammar */
    g->prod_cnt = 9;

    /* E -> E + T */
    { const char *r[] = {"E", "+", "T"}; add_prod(g, 0, "E", r, 3); }
    /* E -> E - T */
    { const char *r[] = {"E", "-", "T"}; add_prod(g, 1, "E", r, 3); }
    /* E -> T */
    { const char *r[] = {"T"}; add_prod(g, 2, "E", r, 1); }
    /* T -> T * F */
    { const char *r[] = {"T", "*", "F"}; add_prod(g, 3, "T", r, 3); }
    /* T -> T / F */
    { const char *r[] = {"T", "/", "F"}; add_prod(g, 4, "T", r, 3); }
    /* T -> F */
    { const char *r[] = {"F"}; add_prod(g, 5, "T", r, 1); }
    /* F -> ( E ) */
    { const char *r[] = {"(", "E", ")"}; add_prod(g, 6, "F", r, 3); }
    /* F -> id */
    { const char *r[] = {"id"}; add_prod(g, 7, "F", r, 1); }
    /* F -> num */
    { const char *r[] = {"num"}; add_prod(g, 8, "F", r, 1); }

    /* fix up symbol types and indices for RHS symbols */
    for (int i = 0; i < g->prod_cnt; i++) {
        for (int j = 0; j < g->prods[i].rhs_len; j++) {
            const char *nm = g->prods[i].rhs[j].name;
            int found = 0;
            for (int k = 0; k < g->nt_cnt; k++) {
                if (strcmp(g->nts[k].name, nm) == 0) {
                    g->prods[i].rhs[j].type = NT;
                    g->prods[i].rhs[j].idx = k;
                    found = 1;
                    break;
                }
            }
            if (!found) {
                for (int k = 0; k < g->t_cnt; k++) {
                    if (strcmp(g->ts[k].name, nm) == 0) {
                        g->prods[i].rhs[j].type = T;
                        g->prods[i].rhs[j].idx = g->nt_cnt + k;
                        break;
                    }
                }
            }
        }
    }

    /* start symbol */
    g->start.type = NT;
    g->start.idx = 0;
    strcpy(g->start.name, "E");
}

const Symbol *gram_find(const Grammar *g, const char *name)
{
    for (int i = 0; i < g->nt_cnt; i++)
        if (strcmp(g->nts[i].name, name) == 0) return &g->nts[i];
    for (int i = 0; i < g->t_cnt; i++)
        if (strcmp(g->ts[i].name, name) == 0) return &g->ts[i];
    return NULL;
}

void gram_print_nts(const Grammar *g)
{
    printf("  Non-terminals: ");
    for (int i = 0; i < g->nt_cnt; i++) {
        printf("%s", g->nts[i].name);
        if (i < g->nt_cnt - 1) printf(", ");
    }
    printf("\n");
}

void gram_print_ts(const Grammar *g)
{
    printf("  Terminals: ");
    for (int i = 0; i < g->t_cnt; i++) {
        printf("%s", g->ts[i].name);
        if (i < g->t_cnt - 1) printf(", ");
    }
    printf("\n");
}

void gram_print(const Grammar *g)
{
    printf("\n  Grammar: G = (V, T, P, E)\n");
    printf("  where\n");
    printf("    V = {E, T, F}  (non-terminals)\n");
    gram_print_nts(g);
    printf("    T = {id, num, +, -, *, /, (, ), $}\n");
    gram_print_ts(g);
    printf("    E = E  (start symbol)\n\n");

    printf("  Productions:\n");
    for (int i = 0; i < g->prod_cnt; i++) {
        printf("    (%d)  %s -> ", i + 1, g->prods[i].lhs.name);
        for (int j = 0; j < g->prods[i].rhs_len; j++) {
            printf("%s", g->prods[i].rhs[j].name);
            if (j < g->prods[i].rhs_len - 1) printf(" ");
        }
        printf("\n");
    }

    printf("\n  This is an operator precedence grammar because:\n");
    printf("  - No production has two adjacent non-terminals on the RHS\n");
    printf("  - No epsilon (empty) productions\n");
    printf("  - Left recursion is fine for operator precedence parsing\n\n");
}
