/*
 * dfa.h - DFA definitions for the compiler project
 *
 * I implemented two DFAs here:
 *   1. One for identifiers: letter (letter | digit)*
 *   2. One for integers: digit+
 *
 * The idea is that a DFA has states, an alphabet, a start state,
 * accepting states, and transitions. I'm simulating them character
 * by character in C.
 */

#ifndef DFA_H
#define DFA_H

#include <stdbool.h>

/* ---- Identifier DFA states ---- */
typedef enum {
    ID_START = 0,   /* haven't seen anything yet */
    ID_READ  = 1,   /* reading a valid identifier */
    ID_DEAD  = 2    /* invalid, will never accept */
} IdState;

/* ---- Integer DFA states ---- */
typedef enum {
    NUM_START = 0,
    NUM_READ  = 1,
    NUM_DEAD  = 2
} NumState;

/* ---- Character class (what kind of char is this?) ---- */
typedef enum {
    C_LETTER,
    C_DIGIT,
    C_OTHER
} CharClass;

/* return character class for a given char */
CharClass dfa_class(char c);

/* ---- Identifier DFA ---- */
IdState  id_start(void);
bool     id_accepting(IdState s);
IdState  id_transition(IdState s, CharClass c);
bool     id_accepts(const char *str);

/* ---- Integer DFA ---- */
NumState num_start(void);
bool     num_accepting(NumState s);
NumState num_transition(NumState s, CharClass c);
bool     num_accepts(const char *str);

/* ---- Print DFA info for the report ---- */
void id_print_info(void);
void num_print_info(void);

#endif
