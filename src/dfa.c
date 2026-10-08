/*
 * dfa.c - DFA implementation
 *
 * This file has the actual DFA logic. I have two DFAs:
 * one for identifiers and one for numbers.
 *
 * Each DFA works by taking one character at a time and
 * moving between states. If we end in an accepting state,
 * the string is valid.
 */

#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include "dfa.h"

/* ---- helper: figure out what class a character belongs to ---- */
CharClass dfa_class(char c)
{
    if (isalpha((unsigned char)c)) return C_LETTER;
    if (isdigit((unsigned char)c)) return C_DIGIT;
    return C_OTHER;
}

/* ================================================================
 *  IDENTIFIER DFA
 *
 *  Accepts: letter (letter | digit)*
 *
 *  States:
 *    0 = START  (waiting for first letter)
 *    1 = READ   (reading identifier, this is accepting)
 *    2 = DEAD   (something went wrong, never accept from here)
 *
 *  Transition logic:
 *    From START (0): only letters go to READ (1), everything else -> DEAD
 *    From READ  (1): letters and digits stay in READ, other -> DEAD
 *    From DEAD  (2): everything stays in DEAD
 * ================================================================ */

IdState id_start(void)
{
    return ID_START;
}

bool id_accepting(IdState s)
{
    return s == ID_READ;
}

IdState id_transition(IdState s, CharClass c)
{
    if (s == ID_START) {
        return (c == C_LETTER) ? ID_READ : ID_DEAD;
    }
    if (s == ID_READ) {
        if (c == C_LETTER || c == C_DIGIT) return ID_READ;
        return ID_DEAD;
    }
    return ID_DEAD;
}

bool id_accepts(const char *str)
{
    IdState s = id_start();
    for (int i = 0; str[i] != '\0'; i++) {
        s = id_transition(s, dfa_class(str[i]));
        if (s == ID_DEAD) return false;
    }
    return id_accepting(s);
}

/* ================================================================
 *  INTEGER DFA
 *
 *  Accepts: digit+
 *
 *  States:
 *    0 = START
 *    1 = READ  (accepting)
 *    2 = DEAD
 *
 *  Transition logic:
 *    From START (0): only digits go to READ
 *    From READ  (1): only digits stay in READ
 *    From DEAD  (2): everything stays dead
 * ================================================================ */

NumState num_start(void)
{
    return NUM_START;
}

bool num_accepting(NumState s)
{
    return s == NUM_READ;
}

NumState num_transition(NumState s, CharClass c)
{
    if (s == NUM_START) {
        return (c == C_DIGIT) ? NUM_READ : NUM_DEAD;
    }
    if (s == NUM_READ) {
        return (c == C_DIGIT) ? NUM_READ : NUM_DEAD;
    }
    return NUM_DEAD;
}

bool num_accepts(const char *str)
{
    NumState s = num_start();
    for (int i = 0; str[i] != '\0'; i++) {
        s = num_transition(s, dfa_class(str[i]));
        if (s == NUM_DEAD) return false;
    }
    return num_accepting(s);
}

/* ---- print DFA diagrams and transition tables ---- */

void id_print_info(void)
{
    printf("\n  --- Identifier DFA ---\n");
    printf("  Accepts: letter (letter | digit)*\n\n");
    printf("  States:\n");
    printf("    0 = START   (waiting for first letter)\n");
    printf("    1 = READ    (reading identifier) [ACCEPTING]\n");
    printf("    2 = DEAD    (invalid string)\n\n");
    printf("  Transition Table:\n");
    printf("  State  |  letter  |  digit  |  other\n");
    printf("  -------|----------|---------|--------\n");
    printf("    0    |     1    |    2    |    2\n");
    printf("    1    |     1    |    1    |    2\n");
    printf("    2    |     2    |    2    |    2\n\n");
}

void num_print_info(void)
{
    printf("\n  --- Integer DFA ---\n");
    printf("  Accepts: digit+\n\n");
    printf("  States:\n");
    printf("    0 = START   (waiting for first digit)\n");
    printf("    1 = READ    (reading integer) [ACCEPTING]\n");
    printf("    2 = DEAD    (invalid string)\n\n");
    printf("  Transition Table:\n");
    printf("  State  |  letter  |  digit  |  other\n");
    printf("  -------|----------|---------|--------\n");
    printf("    0    |     2    |    1    |    2\n");
    printf("    1    |     2    |    1    |    2\n");
    printf("    2    |     2    |    2    |    2\n\n");
}
