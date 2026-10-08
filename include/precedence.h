/*
 * precedence.h - Precedence relations and table
 *
 * The precedence table is built from the grammar's productions
 * using FIRSTVT and LASTVT.
 *
 * For each adjacent pair (X, Y) in a production:
 *   X=terminal, Y=terminal  =>  X = Y
 *   X=terminal, Y=non-term   =>  X < y  for all y in FIRSTVT(Y)
 *   X=non-term,  Y=terminal  =>  x > Y  for all x in LASTVT(X)
 *   X=non-term,  Y=non-term  =>  x < y  for all x in LASTVT(X), y in FIRSTVT(Y)
 *
 * Plus for the augmented start:
 *   $ < a  for all a in FIRSTVT(E)
 *   a > $  for all a in LASTVT(E)
 */

#ifndef PREC_H
#define PREC_H

#include "grammar.h"
#include "firstvt_lastvt.h"

typedef enum { NONE = 0, LT = 1, EQ = 2, GT = 3 } Rel;

const char *rel_name(Rel r);

typedef struct {
    Rel    **tbl;       /* [t_cnt][t_cnt] */
    int      t_cnt;
    char    *names[20];  /* terminal name pointers */
    bool     conflict;
    int      n_conflict;
} PrecTable;

void  prec_build(const Grammar *g, const FLVResult *fl, PrecTable *pt);
void  prec_free(PrecTable *pt);
void  prec_print(const PrecTable *pt);
void  prec_print_rel(const PrecTable *pt, const char *a, const char *b);

#endif
