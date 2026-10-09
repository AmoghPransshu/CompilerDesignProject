/*
 * precedence.c - Operator precedence table construction
 *
 * This builds the precedence table from the grammar and
 * the FIRSTVT / LASTVT sets.
 *
 * For each adjacent pair (X, Y) in a production:
 *   both terminals  =>  X = Y
 *   X=term, Y=non   =>  X < y  for all y in FIRSTVT(Y)
 *   X=non,  Y=term  =>  x > Y  for all x in LASTVT(X)
 *   both non        =>  x < y  for all x in LASTVT(X), y in FIRSTVT(Y)
 *
 * Plus augmented start E' -> E $:
 *   $ < a  for all a in FIRSTVT(E)
 *   a > $  for all a in LASTVT(E)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "precedence.h"
#include "grammar.h"
#include "firstvt_lastvt.h"

const char *rel_name(Rel r)
{
    switch (r) {
        case LT: return "<";
        case EQ: return "=";
        case GT: return ">";
        default: return " ";
    }
}

/* find terminal position in grammar */
static int term_pos(const Grammar *g, const char *name)
{
    for (int i = 0; i < g->t_cnt; i++)
        if (strcmp(g->ts[i].name, name) == 0) return i;
    return -1;
}

/* find terminal position in table */
static int tbl_pos(const PrecTable *pt, const char *name)
{
    for (int i = 0; i < pt->t_cnt; i++)
        if (strcmp(pt->names[i], name) == 0) return i;
    return -1;
}

/* set a relation, detect conflicts */
static void set_rel(PrecTable *pt, int r, int c, Rel rel)
{
    if (pt->tbl[r][c] == NONE) {
        pt->tbl[r][c] = rel;
    } else if (pt->tbl[r][c] != rel) {
        pt->conflict   = true;
        pt->n_conflict++;
    }
}

/* build the precedence table */
void prec_build(const Grammar *g, const FLVResult *fl, PrecTable *pt)
{
    memset(pt, 0, sizeof(*pt));
    pt->t_cnt = g->t_cnt;

    /* allocate table */
    pt->tbl = malloc(pt->t_cnt * sizeof(Rel *));
    assert(pt->tbl != NULL);
    for (int i = 0; i < pt->t_cnt; i++) {
        pt->tbl[i] = calloc(pt->t_cnt, sizeof(Rel));
        assert(pt->tbl[i] != NULL);
    }

    /* copy terminal name pointers */
    for (int i = 0; i < pt->t_cnt; i++) {
        pt->names[i] = g->ts[i].name;
    }

    printf("\n  --- Constructing Precedence Relations ---\n");

    /* process each production's adjacent pairs */
    for (int p = 0; p < g->prod_cnt; p++) {
        const Prod *prod = &g->prods[p];

        printf("\n    Production %d: %s -> ", p + 1, prod->lhs.name);
        for (int j = 0; j < prod->rhs_len; j++) {
            printf("%s", prod->rhs[j].name);
            if (j < prod->rhs_len - 1) printf(" ");
        }
        printf("\n");

        for (int i = 0; i < prod->rhs_len - 1; i++) {
            const Symbol *X = &prod->rhs[i];
            const Symbol *Y = &prod->rhs[i + 1];

            printf("      Pair (%s, %s): ", X->name, Y->name);

            if (X->type == T && Y->type == T) {
                printf("term-term => =\n");
                int xi = term_pos(g, X->name);
                int yi = term_pos(g, Y->name);
                set_rel(pt, xi, yi, EQ);

            } else if (X->type == T && Y->type == NT) {
                printf("term-nonterm => < FIRSTVT(%s)\n", Y->name);
                int xi = term_pos(g, X->name);
                /* find Y's non-terminal index */
                int y_nt = -1;
                for (int k = 0; k < g->nt_cnt; k++) {
                    if (strcmp(g->nts[k].name, Y->name) == 0) { y_nt = k; break; }
                }
                for (int t = 0; t < g->t_cnt; t++) {
                    if (TS_HAS(fl->firstvt[y_nt], t)) {
                        int ti = term_pos(g, g->ts[t].name);
                        set_rel(pt, xi, ti, LT);
                    }
                }

            } else if (X->type == NT && Y->type == T) {
                printf("nonterm-term => LASTVT(%s) > \n", X->name);
                int yi = term_pos(g, Y->name);
                int x_nt = -1;
                for (int k = 0; k < g->nt_cnt; k++) {
                    if (strcmp(g->nts[k].name, X->name) == 0) { x_nt = k; break; }
                }
                for (int t = 0; t < g->t_cnt; t++) {
                    if (TS_HAS(fl->lastvt[x_nt], t)) {
                        int ti = term_pos(g, g->ts[t].name);
                        set_rel(pt, ti, yi, GT);
                    }
                }

            } else {
                /* both non-terminals */
                printf("non-non => LASTVT(%s) < FIRSTVT(%s)\n", X->name, Y->name);
                int x_nt = -1, y_nt = -1;
                for (int k = 0; k < g->nt_cnt; k++) {
                    if (strcmp(g->nts[k].name, X->name) == 0) x_nt = k;
                    if (strcmp(g->nts[k].name, Y->name) == 0) y_nt = k;
                }
                for (int xt = 0; xt < g->t_cnt; xt++) {
                    if (TS_HAS(fl->lastvt[x_nt], xt)) {
                        for (int yt = 0; yt < g->t_cnt; yt++) {
                            if (TS_HAS(fl->firstvt[y_nt], yt)) {
                                int xi = term_pos(g, g->ts[xt].name);
                                int yi = term_pos(g, g->ts[yt].name);
                                set_rel(pt, xi, yi, LT);
                            }
                        }
                    }
                }
            }
        }
    }

    /* augmented grammar: E' -> E $ */
    printf("\n    Augmented: E' -> E $\n");
    int e_nt = 0;  /* E is always index 0 in our grammar */
    int dp = term_pos(g, "$");

    printf("      $ < FIRSTVT(E): ");
    for (int t = 0; t < g->t_cnt; t++) {
        if (TS_HAS(fl->firstvt[e_nt], t)) {
            int ti = term_pos(g, g->ts[t].name);
            set_rel(pt, dp, ti, LT);
            printf("%s ", g->ts[t].name);
        }
    }
    printf("\n");

    printf("      LASTVT(E) > $: ");
    for (int t = 0; t < g->t_cnt; t++) {
        if (TS_HAS(fl->lastvt[e_nt], t)) {
            int ti = term_pos(g, g->ts[t].name);
            set_rel(pt, ti, dp, GT);
            printf("%s ", g->ts[t].name);
        }
    }
    printf("\n");
}

void prec_free(PrecTable *pt)
{
    if (pt->tbl) {
        for (int i = 0; i < pt->t_cnt; i++) free(pt->tbl[i]);
        free(pt->tbl);
        pt->tbl = NULL;
    }
}

void prec_print(const PrecTable *pt)
{
    int n = pt->t_cnt;

    printf("\n");
    printf("  ┌───────────────────────────────────────────────────────────────┐\n");
    printf("  │           OPERATOR PRECEDENCE TABLE                          │\n");
    printf("  ├──────────┬────────┬───────┬───────┬───────┬───────┬──────────┤\n");

    printf("  │    R\\C   │");
    for (int j = 0; j < n; j++) printf("  %4s  │", pt->names[j]);
    printf("\n");

    printf("  ├──────────┼────────┼───────┼───────┼───────┼───────┼──────────┤\n");

    for (int i = 0; i < n; i++) {
        printf("  │ %-8s │", pt->names[i]);
        for (int j = 0; j < n; j++) {
            printf("   %s    │", rel_name(pt->tbl[i][j]));
        }
        printf("\n");
    }

    printf("  └──────────┴────────┴───────┴───────┴───────┴───────┴──────────┘\n");

    if (pt->conflict) {
        printf("\n  *** %d CONFLICT(S) DETECTED ***\n", pt->n_conflict);
    } else {
        printf("\n  No conflicts. Grammar is a valid operator precedence grammar.\n");
    }

    printf("\n  Legend:\n");
    printf("    <  : yields precedence (shift)\n");
    printf("    =  : equal precedence\n");
    printf("    >  : takes precedence (reduce)\n\n");
}

void prec_print_rel(const PrecTable *pt, const char *a, const char *b)
{
    int ai_idx = tbl_pos(pt, a);
    int bi_idx = tbl_pos(pt, b);

    if (ai_idx < 0 || bi_idx < 0) {
        printf("  Terminal not found.\n");
        return;
    }

    Rel r = pt->tbl[ai_idx][bi_idx];
    printf("  %s %s %s: %s\n",
           a, rel_name(r), b,
           r == LT ? "SHIFT" : r == EQ ? "EQUAL" : r == GT ? "REDUCE" : "NONE");
}
