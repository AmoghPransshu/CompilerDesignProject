/*
 * main.c - Phase 1 Operator Precedence Compiler Demonstration
 *
 * Menu-driven program that demonstrates:
 *   1. Lexical rules
 *   2. DFA construction
 *   3. Tokenization
 *   4. Context-free grammar
 *   5. FIRSTVT computation
 *   6. LASTVT computation
 *   7. Precedence relations
 *   8. Precedence table
 */

#include <stdio.h>
#include <string.h>
#include "dfa.h"
#include "lexer.h"
#include "grammar.h"
#include "firstvt_lastvt.h"
#include "precedence.h"

static void demo_lexical_rules(void);
static void demo_id_dfa(void);
static void demo_num_dfa(void);
static void demo_tokenize(void);
static void demo_grammar(void);
static void demo_firstvt(void);
static void demo_lastvt(void);
static void demo_prec_rel(void);
static void demo_prec_table(void);
static void demo_complete(void);

static Grammar g;
static FLVResult fl;

int main(void)
{
    gram_init(&g);

    int choice;

    printf("\n");
    printf("============================================\n");
    printf("  OPERATOR PRECEDENCE COMPILER - PHASE 1\n");
    printf("============================================\n");

    do {
        printf("\n");
        printf("  1. Display Lexical Rules\n");
        printf("  2. Test Identifier DFA\n");
        printf("  3. Test Number DFA\n");
        printf("  4. Tokenize Expression\n");
        printf("  5. Display Grammar\n");
        printf("  6. Calculate FIRSTVT\n");
        printf("  7. Calculate LASTVT\n");
        printf("  8. Generate Precedence Relations\n");
        printf("  9. Display Precedence Table\n");
        printf(" 10. Run Complete Phase 1 Demo\n");
        printf("  0. Exit\n");
        printf("\nChoice: ");
        fflush(stdout);

        if (scanf("%d", &choice) != 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
            choice = -1;
        }
        getchar(); /* consume newline */

        printf("\n");

        switch (choice) {
            case 1:  demo_lexical_rules(); break;
            case 2:  demo_id_dfa();        break;
            case 3:  demo_num_dfa();       break;
            case 4:  demo_tokenize();      break;
            case 5:  demo_grammar();       break;
            case 6:  demo_firstvt();       break;
            case 7:  demo_lastvt();        break;
            case 8:  demo_prec_rel();      break;
            case 9:  demo_prec_table();    break;
            case 10: demo_complete();      break;
            case 0:  printf("Bye!\n\n");  break;
            default: printf("Invalid choice.\n"); break;
        }
    } while (choice != 0);

    return 0;
}

/* ---- option 1 ---- */
static void demo_lexical_rules(void)
{
    printf("LEXICAL RULES\n");
    printf("-------------\n\n");
    printf("IDENTIFIER: letter (letter | digit)*\n");
    printf("  valid:   a, abc, x1, variable123\n");
    printf("  invalid: 123abc, 1x\n\n");
    printf("NUMBER: digit+\n");
    printf("  valid:   0, 25, 1000\n");
    printf("  invalid: 12a, abc\n\n");
    printf("OPERATORS: +  -  *  /\n");
    printf("PARENTHESES: (  )\n");
    printf("WHITESPACE: ignored\n\n");
    printf("INVALID: 123abc, a**b, +a, a+\n");
}

/* ---- option 2 ---- */
static void demo_id_dfa(void)
{
    printf("IDENTIFIER DFA\n");
    printf("--------------\n");
    id_print_info();
}

/* ---- option 3 ---- */
static void demo_num_dfa(void)
{
    printf("INTEGER DFA\n");
    printf("-----------\n");
    num_print_info();
}

/* ---- option 4 ---- */
static void demo_tokenize(void)
{
    char expr[256];
    printf("Enter expression: ");
    fflush(stdout);

    if (fgets(expr, sizeof(expr), stdin) == NULL) {
        printf("(no input)\n");
        return;
    }

    size_t len = strlen(expr);
    if (len > 0 && expr[len - 1] == '\n') expr[len - 1] = '\0';

    lex_tokenize(expr);
}

/* ---- option 5 ---- */
static void demo_grammar(void)
{
    printf("CONTEXT-FREE GRAMMAR\n");
    printf("--------------------\n");
    gram_print(&g);
}

/* ---- option 6 ---- */
static void demo_firstvt(void)
{
    printf("FIRSTVT COMPUTATION\n");
    printf("-------------------\n");
    flv_compute(&g, &fl);
    flv_print_firstvt(&g, &fl);
}

/* ---- option 7 ---- */
static void demo_lastvt(void)
{
    printf("LASTVT COMPUTATION\n");
    printf("------------------\n");
    /* compute if not already done */
    if (fl.nt_cnt == 0) flv_compute(&g, &fl);
    flv_print_lastvt(&g, &fl);
}

/* ---- option 8 ---- */
static void demo_prec_rel(void)
{
    printf("PRECEDENCE RELATIONS\n");
    printf("--------------------\n");

    if (fl.nt_cnt == 0) flv_compute(&g, &fl);

    PrecTable pt;
    prec_build(&g, &fl, &pt);

    printf("\nKey relations:\n");
    prec_print_rel(&pt, "+", "*");
    prec_print_rel(&pt, "*", "+");
    prec_print_rel(&pt, "+", "-");
    prec_print_rel(&pt, "(", ")");

    prec_free(&pt);
}

/* ---- option 9 ---- */
static void demo_prec_table(void)
{
    printf("PRECEDENCE TABLE\n");
    printf("----------------\n");

    if (fl.nt_cnt == 0) flv_compute(&g, &fl);

    PrecTable pt;
    prec_build(&g, &fl, &pt);
    prec_print(&pt);
    prec_free(&pt);
}

/* ---- option 10: complete demo with a + b * c ---- */
static void demo_complete(void)
{
    printf("============================================\n");
    printf("  COMPLETE PHASE 1 DEMO: a + b * c\n");
    printf("============================================\n");

    printf("\n[1] Input Expression\n");
    printf("    a + b * c\n");

    printf("\n[2] Tokenization\n");
    lex_tokenize("a + b * c");

    printf("[3] Grammar\n");
    gram_print(&g);

    printf("[4] FIRSTVT\n");
    flv_compute(&g, &fl);
    flv_print_firstvt(&g, &fl);

    printf("[5] LASTVT\n");
    flv_print_lastvt(&g, &fl);

    printf("[6] Precedence Relations\n");
    PrecTable pt;
    prec_build(&g, &fl, &pt);

    prec_print_rel(&pt, "+", "*");
    prec_print_rel(&pt, "*", "+");
    prec_print_rel(&pt, "+", "-");
    prec_print_rel(&pt, "(", ")");

    printf("\n[7] Precedence Table\n");
    prec_print(&pt);
    prec_free(&pt);

    printf("============================================\n");
    printf("  Phase 1 complete.\n");
    printf("  Parser and TAC will use this table (Phase 2).\n");
    printf("============================================\n");
}
