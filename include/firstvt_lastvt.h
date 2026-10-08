/*
 * firstvt_lastvt.h - FIRSTVT and LASTVT computation
 *
 * FIRSTVT(A) = all terminals that can appear first in a string
 *              derived from non-terminal A.
 *
 * LASTVT(A)  = all terminals that can appear last in a string
 *              derived from non-terminal A.
 *
 * Algorithm:
 *   FIRSTVT:
 *     For each production A -> aX (a is terminal): a goes into FIRSTVT(A)
 *     Repeat until nothing changes:
 *       For each production A -> BX (B is non-terminal):
 *         copy FIRSTVT(B) into FIRSTVT(A)
 *
 *   LASTVT:
 *     For each production A -> Xa (a is terminal): a goes into LASTVT(A)
 *     Repeat until nothing changes:
 *       For each production A -> XB (B is non-terminal):
 *         copy LASTVT(B) into LASTVT(A)
 */

#ifndef FLV_H
#define FLV_H

#include "grammar.h"

/* bitmask for terminal sets (up to 32 terminals) */
typedef unsigned int TSet;

#define TS_EMPTY  0u
#define TS_ADD(s, i)  ((s) |= (1u << (i)))
#define TS_HAS(s, i)  (((s) & (1u << (i))) != 0u)
#define TS_UNION(a, b) ((a) | (b))

/* result of FIRSTVT / LASTVT computation */
typedef struct {
    TSet firstvt[MAX_NT];
    TSet lastvt[MAX_NT];
    int  nt_cnt;
} FLVResult;

/* compute FIRSTVT and LASTVT */
void flv_compute(const Grammar *g, FLVResult *r);

/* print results */
void flv_print_firstvt(const Grammar *g, const FLVResult *r);
void flv_print_lastvt(const Grammar *g, const FLVResult *r);
void flv_print(const Grammar *g, const FLVResult *r);

#endif
