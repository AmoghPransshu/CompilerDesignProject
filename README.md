# Operator Precedence Compiler — Phase 1

## University Compiler Design Project

This is my Phase 1 submission for the Compiler Design course project. The goal here is to build the theoretical foundation for an operator precedence parser — things like lexical analysis, DFA construction, grammar definition, FIRSTVT/LASTVT computation, and the precedence table.

Phase 2 (the actual parser and TAC generation) comes later once this is approved.

## What This Phase Does

- Defines lexical rules for identifiers and integers
- Builds and simulates DFAs for pattern recognition
- Tokenizes arithmetic expressions
- Defines the context-free grammar
- Computes FIRSTVT and LASTVT sets
- Constructs the operator precedence table

## What This Phase Does NOT Do

- No shift-reduce parser (Phase 2)
- No parse tree (Phase 2)
- No Three Address Code generation (Phase 2)
- No GUI or fancy output

## How to Build

```bash
make
```

Or manually:

```bash
gcc -Wall -std=c11 -Iinclude src/dfa.c src/lexer.c src/grammar.c src/firstvt_lastvt.c src/precedence.c src/main.c -o build/opc
```

## How to Run

```bash
./build/opc
```

Then pick a menu option. Option 10 runs the complete demonstration with the expression `a + b * c`.

## Project Structure

```
operator-precedence-compiler/
├── README.md
├── Makefile
├── docs/
│   ├── lexical-rules.md
│   ├── dfa.md
│   ├── grammar.md
│   ├── firstvt-lastvt.md
│   └── precedence-table.md
├── include/
│   ├── dfa.h
│   ├── lexer.h
│   ├── grammar.h
│   ├── firstvt_lastvt.h
│   └── precedence.h
├── src/
│   ├── main.c
│   ├── dfa.c
│   ├── lexer.c
│   ├── grammar.c
│   ├── firstvt_lastvt.c
│   └── precedence.c
└── tests/
    └── test_cases.txt
```

## Example Output

```
Input expression: "a + b * c"

Token stream:
<ID, a>
<PLUS, +>
<ID, b>
<MUL, *>
<ID, c>

Grammar:
  E -> E + T | E - T | T
  T -> T * F | T / F | F
  F -> ( E ) | id | num

FIRSTVT:
  E: { id, num, ( }
  T: { id, num, ( }
  F: { id, num, ( }

LASTVT:
  E: { id, num, ) }
  T: { id, num, ) }
  F: { id, num, ) }

Precedence Table:
        id   num   +    -    *    /    (    )    $
    id       =    <    <    <    <    <    >    >
    ...
```

## Notes

- The DFA is implemented from scratch — no Flex/Lex
- The grammar is stored as actual data structures, not just printed text
- FIRSTVT/LASTVT are computed algorithmically using fixed-point iteration
- The precedence table is built from the grammar relations, not hard-coded

## Phase 2 — Future Work

The following are intentionally left for later:

1. Shift-reduce parser (will use the precedence table from Phase 1)
2. Parse tree construction
3. Three Address Code generation
4. Complete compiler pipeline
