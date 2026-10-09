/*
 * firstvt_lastvt.c - FIRSTVT and LASTVT computation
 *
 * I implemented this using the iterative fixed-point method.
 * The key thing is to first add direct terminals (from productions
 * that start/end with a terminal), then iterate to propagate
 * through non-terminals.
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "firstvt_lastvt.h"
#include "grammar.h"

/* helper: find terminal index by name */
static int t_idx(const Grammar *g, const char *name)
{
    for (int i = 0; i < g->t_cnt; i++)
        if (strcmp(g->ts[i].name, name) == 0) return i;
    return -1;
}

/* helper: find non-terminal index by name */
static int nt_idx(const Grammar *g, const char *name)
{
    for (int i = 0; i < g->nt_cnt; i++)
        if (strcmp(g->nts[i].name, name) == 0) return i;
    return -1;
}

/* ================================================================
 *  FIRSTVT computation
 *
 *  Phase 1: for A -> a..., add a to FIRSTVT(A) directly
 *            for A -> B..., skip for now
 *
 *  Phase 2: loop - for A -> B..., add FIRSTVT(B) to FIRSTVT(A)
 *           until no changes
 * ================================================================ */

static void calc_firstvt(const Grammar *g, TSet fv[])
{
    for (int i = 0; i < g->nt_cnt; i++) fv[i] = TS_EMPTY;

    /* Phase 1: direct terminals only */
    for (int p = 0; p < g->prod_cnt; p++) {
        if (g->prods[p].rhs_len == 0) continue;
        const Symbol *first = &g->prods[p].rhs[0];
        if (first->type == T) {
            int lhs = nt_idx(g, g->prods[p].lhs.name);
            int ti = t_idx(g, first->name);
            assert(ti >= 0);
            TS_ADD(fv[lhs], ti);
        }
    }

    /* Phase 2: propagate through non-terminals */
    bool changed = true;
    int iter = 0;
    while (changed) {
        changed = false;
        iter++;

        for (int p = 0; p < g->prod_cnt; p++) {
            if (g->prods[p].rhs_len == 0) continue;
            const Symbol *first = &g->prods[p].rhs[0];
            if (first->type == NT) {
                int lhs = nt_idx(g, g->prods[p].lhs.name);
                int b   = nt_idx(g, first->name);
                TSet old = fv[lhs];
                fv[lhs] = TS_UNION(fv[lhs], fv[b]);
                if (fv[lhs] != old) changed = true;
            }
        }
    }

    printf("    [FIRSTVT converged after %d iterations]\n", iter);
}

/* ================================================================
 *  LASTVT computation - same approach as FIRSTVT but from the end
 * ================================================================ */

static void calc_lastvt(const Grammar *g, TSet lv[])
{
    for (int i = 0; i < g->nt_cnt; i++) lv[i] = TS_EMPTY;

    /* Phase 1: direct terminals only */
    for (int p = 0; p < g->prod_cnt; p++) {
        if (g->prods[p].rhs_len == 0) continue;
        const Symbol *last = &g->prods[p].rhs[g->prods[p].rhs_len - 1];
        if (last->type == T) {
            int lhs = nt_idx(g, g->prods[p].lhs.name);
            int ti = t_idx(g, last->name);
            assert(ti >= 0);
            TS_ADD(lv[lhs], ti);
        }
    }

    /* Phase 2: propagate */
    bool changed = true;
    int iter = 0;
    while (changed) {
        changed = false;
        iter++;

        for (int p = 0; p < g->prod_cnt; p++) {
            if (g->prods[p].rhs_len == 0) continue;
            const Symbol *last = &g->prods[p].rhs[g->prods[p].rhs_len - 1];
            if (last->type == NT) {
                int lhs = nt_idx(g, g->prods[p].lhs.name);
                int b   = nt_idx(g, last->name);
                TSet old = lv[lhs];
                lv[lhs] = TS_UNION(lv[lhs], lv[b]);
                if (lv[lhs] != old) changed = true;
            }
        }
    }

    printf("    [LASTVT converged after %d iterations]\n", iter);
}

void flv_compute(const Grammar *g, FLVResult *r)
{
    memset(r, 0, sizeof(*r));
    r->nt_cnt = g->nt_cnt;

    printf("\n  --- Computing FIRSTVT ---\n");
    calc_firstvt(g, r->firstvt);

    printf("\n  --- Computing LASTVT ---\n");
    calc_lastvt(g, r->lastvt);
}

/* ---- printing ---- */

void flv_print_firstvt(const Grammar *g, const FLVResult *r)
{
    printf("\n  FIRSTVT:\n");
    printf("  --------\n");
    for (int i = 0; i < r->nt_cnt; i++) {
        printf("    FIRSTVT(%s) = { ", g->nts[i].name);
        bool first = true;
        for (int t = 0; t < g->t_cnt; t++) {
            if (TS_HAS(r->firstvt[i], t)) {
                if (!first) printf(", ");
                printf("%s", g->ts[t].name);
                first = false;
            }
        }
        printf(" }\n");
    }
    printf("\n");
}

void flv_print_lastvt(const Grammar *g, const FLVResult *r)
{
    printf("\n  LASTVT:\n");
    printf("  -------\n");
    for (int i = 0; i < r->nt_cnt; i++) {
        printf("    LASTVT(%s) = { ", g->nts[i].name);
        bool first = true;
        for (int t = 0; t < g->t_cnt; t++) {
            if (TS_HAS(r->lastvt[i], t)) {
                if (!first) printf(", ");
                printf("%s", g->ts[t].name);
                first = false;
            }
        }
        printf(" }\n");
    }
    printf("\n");
}

void flv_print(const Grammar *g, const FLVResult *r)
{
    flv_print_firstvt(g, r);
    flv_print_lastvt(g, r);
}
