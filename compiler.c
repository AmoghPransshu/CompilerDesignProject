typedef int bool;
#define TRUE 1
#define FALSE 0

typedef enum {
    ERR_NONE = 0,
    ERR_INVALID_CHAR,
    ERR_UNCLOSED_PAREN,
    ERR_EXTRA_CLOSE_PAREN,
    ERR_EMPTY_EXPR,
    ERR_TWO_OPERATORS,
    ERR_START_OPERATOR,
    ERR_END_OPERATOR
} ErrorCode;

typedef enum {
    TOK_ID,
    TOK_PLUS,
    TOK_MINUS,
    TOK_MULT,
    TOK_DIV,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_END,
    TOK_INVALID,
    TOK_EMPTY
} TokenType;

typedef struct {
    TokenType type;
    char value[32];
} Token;

typedef enum {
    REL_SHIFT,
    REL_REDUCE,
    REL_EQUAL,
    REL_ACCEPT
} Relation;

typedef struct {
    TokenType type;
    char value[32];
} StackEntry;

typedef enum {
    RESULT_ACCEPT,
    RESULT_REJECT,
    RESULT_ERROR
} ParseResult;

typedef struct {
    char result[32];
    char arg1[32];
    char arg2[32];
    char op;
    int is_temp;
} TACInstruction;

#define MAX_INPUT   512
#define MAX_TOKENS  256
#define MAX_STACK   512
#define MAX_TAC     256

#define SYS_READ   0
#define SYS_WRITE  1
#define SYS_EXIT   60

#define STDIN  0
#define STDOUT 1

static long my_syscall(long n, long a1, long a2, long a3) {
    long ret;
    __asm__ volatile("syscall"
        : "=a"(ret)
        : "a"(n), "D"(a1), "S"(a2), "d"(a3)
        : "rcx", "r11", "memory");
    return ret;
}

static void write_char(char c) {
    char b[1];
    b[0] = c;
    my_syscall(SYS_WRITE, STDOUT, (long)b, 1);
}

static int string_len(const char *s) {
    int n = 0;
    while (s[n] != '\0') n++;
    return n;
}

static void write_str(const char *s) {
    my_syscall(SYS_WRITE, STDOUT, (long)s, string_len(s));
}

static void write_newline(void) {
    write_char('\n');
}

static void write_uint(unsigned int n) {
    char buf[32];
    int len = 0;
    if (n == 0) { write_char('0'); return; }
    while (n > 0) {
        buf[len++] = '0' + (n % 10);
        n /= 10;
    }
    {
        int i;
        for (i = len - 1; i >= 0; i--) write_char(buf[i]);
    }
}

static void write_int(int n) {
    if (n < 0) { write_char('-'); n = -n; }
    write_uint((unsigned int)n);
}

static int read_line(char *buf, int max_len) {
    int i = 0;
    char c;
    int ret;
    int first = 1;

    while (i < max_len - 1) {
        ret = (int)my_syscall(SYS_READ, STDIN, (long)&c, 1);
        if (ret <= 0) { if (first) return -1; break; }
        first = 0;
        if (c == '\n') break;
        buf[i++] = c;
    }
    buf[i] = '\0';
    return i;
}

static int is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static int is_space(char c) {
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}

static int is_digit(char c) {
    return (c >= '0' && c <= '9');
}

static int str_cmp(const char *a, const char *b) {
    int i = 0;
    while (a[i] != '\0' && b[i] != '\0' && a[i] == b[i]) i++;
    return (int)((unsigned char)a[i]) - (int)((unsigned char)b[i]);
}

static void str_cpy(char *dest, const char *src) {
    int i = 0;
    while ((dest[i] = src[i]) != '\0') i++;
}

static void str_ncpy(char *dest, const char *src, int n) {
    int i = 0;
    while (i < n && src[i] != '\0') { dest[i] = src[i]; i++; }
    dest[i] = '\0';
}

static void show_error(ErrorCode err) {
    if (err == ERR_INVALID_CHAR)
        write_str("Error: Invalid character in expression\n");
    else if (err == ERR_UNCLOSED_PAREN)
        write_str("Error: Unclosed parenthesis\n");
    else if (err == ERR_EXTRA_CLOSE_PAREN)
        write_str("Error: Extra closing parenthesis\n");
    else if (err == ERR_EMPTY_EXPR)
        write_str("Error: Empty expression\n");
    else if (err == ERR_TWO_OPERATORS)
        write_str("Error: Two operators cannot appear consecutively\n");
    else if (err == ERR_START_OPERATOR)
        write_str("Error: Expression cannot start with an operator\n");
    else if (err == ERR_END_OPERATOR)
        write_str("Error: Expression cannot end with an operator\n");
    else
        write_str("Error: Unknown error\n");
}

static int is_op(TokenType t) {
    return (t == TOK_PLUS || t == TOK_MINUS || t == TOK_MULT || t == TOK_DIV);
}

static int tokenize(const char *input, Token tokens[], int max_tokens) {
    int count = 0;
    int i = 0;

    while (input[i] != '\0' && is_space(input[i])) i++;

    if (input[i] == '\0') {
        tokens[0].type = TOK_EMPTY;
        tokens[0].value[0] = '\0';
        return 1;
    }

    while (input[i] != '\0' && count < max_tokens - 1) {
        char c = input[i];

        if (is_alpha(c)) {
            tokens[count].type = TOK_ID;
            tokens[count].value[0] = c;
            tokens[count].value[1] = '\0';
            count++; i++;
        } else if (c == '+') {
            tokens[count].type = TOK_PLUS;
            tokens[count].value[0] = '+'; tokens[count].value[1] = '\0';
            count++; i++;
        } else if (c == '-') {
            tokens[count].type = TOK_MINUS;
            tokens[count].value[0] = '-'; tokens[count].value[1] = '\0';
            count++; i++;
        } else if (c == '*') {
            tokens[count].type = TOK_MULT;
            tokens[count].value[0] = '*'; tokens[count].value[1] = '\0';
            count++; i++;
        } else if (c == '/') {
            tokens[count].type = TOK_DIV;
            tokens[count].value[0] = '/'; tokens[count].value[1] = '\0';
            count++; i++;
        } else if (c == '(') {
            tokens[count].type = TOK_LPAREN;
            tokens[count].value[0] = '('; tokens[count].value[1] = '\0';
            count++; i++;
        } else if (c == ')') {
            tokens[count].type = TOK_RPAREN;
            tokens[count].value[0] = ')'; tokens[count].value[1] = '\0';
            count++; i++;
        } else if (is_space(c)) {
            i++;
        } else {
            tokens[count].type = TOK_INVALID;
            tokens[count].value[0] = c; tokens[count].value[1] = '\0';
            count++; i++;
            break;
        }
    }

    if (count > 0 && tokens[count - 1].type != TOK_END) {
        tokens[count].type = TOK_END;
        tokens[count].value[0] = '$'; tokens[count].value[1] = '\0';
        count++;
    }

    return count;
}

static void show_token(const Token *t) {
    const char *names[] = {
        "ID", "PLUS", "MINUS", "MULT", "DIV",
        "LPAREN", "RPAREN", "END", "INVALID", "EMPTY"
    };
    write_str("[");
    write_str(names[t->type]);
    write_str(": '");
    write_str(t->value);
    write_str("']");
}

static ErrorCode validate(const Token tokens[], int token_count) {
    int paren_depth = 0;
    int i;

    if (token_count == 1 && tokens[0].type == TOK_EMPTY)
        return ERR_EMPTY_EXPR;

    for (i = 0; i < token_count; i++) {
        TokenType t = tokens[i].type;

        if (t == TOK_END) continue;

        if (t == TOK_INVALID)
            return ERR_INVALID_CHAR;

        if (t == TOK_LPAREN) {
            paren_depth++;
        } else if (t == TOK_RPAREN) {
            paren_depth--;
            if (paren_depth < 0) return ERR_EXTRA_CLOSE_PAREN;
        }

        if (is_op(t)) {
            if (i == 0) return ERR_START_OPERATOR;
            if (i > 0 && is_op(tokens[i - 1].type)) return ERR_TWO_OPERATORS;
            {
                int is_last = 1;
                int j;
                for (j = i + 1; j < token_count; j++) {
                    if (tokens[j].type != TOK_END) { is_last = 0; break; }
                }
                if (is_last) return ERR_END_OPERATOR;
            }
        }
    }

    if (paren_depth > 0) return ERR_UNCLOSED_PAREN;

    return ERR_NONE;
}

static int token_idx(TokenType t) {
    switch (t) {
        case TOK_ID:     return 0;
        case TOK_PLUS:   return 1;
        case TOK_MINUS:  return 2;
        case TOK_MULT:   return 3;
        case TOK_DIV:    return 4;
        case TOK_LPAREN: return 5;
        case TOK_RPAREN: return 6;
        case TOK_END:    return 7;
        default:         return 0;
    }
}

static const unsigned char prec_table[8][8] = {
    {  1,   1,   1,   1,   1,   1,   2,   2 },
    {  1,   2,   2,   2,   2,   1,   2,   2 },
    {  1,   2,   2,   2,   2,   1,   2,   2 },
    {  1,   1,   1,   2,   2,   1,   2,   2 },
    {  1,   1,   1,   2,   2,   1,   2,   2 },
    {  1,   1,   1,   1,   1,   1,   3,   1 },
    {  2,   2,   2,   2,   2,   2,   2,   2 },
    {  1,   1,   1,   1,   1,   1,   2,   4 }
};

static Relation lookup(TokenType stack_top, TokenType next_input) {
    unsigned char val = prec_table[token_idx(stack_top)][token_idx(next_input)];
    if (val == 1) return REL_SHIFT;
    if (val == 2) return REL_REDUCE;
    if (val == 3) return REL_EQUAL;
    return REL_ACCEPT;
}

static void show_table(void) {
    const char *names[] = { "id", "+", "-", "*", "/", "(", ")", "$" };
    const char *rels[]  = { "S", "R", "E", "A" };
    int i, j;

    write_str("\n      ");
    for (j = 0; j < 8; j++) {
        write_str("  "); write_str(names[j]); write_str("  ");
    }
    write_newline();

    for (i = 0; i < 8; i++) {
        write_str("  "); write_str(names[i]); write_str("  ");
        for (j = 0; j < 8; j++) {
            write_str("  "); write_str(rels[prec_table[i][j] - 1]); write_str("  ");
        }
        write_newline();
    }
    write_newline();
}

static TACInstruction g_tac[MAX_TAC];
static int g_tac_count = 0;
static int temp_counter = 0;

static const char *next_temp(void) {
    static char buf[16];
    int n = temp_counter + 1;
    temp_counter = n;
    buf[0] = 't';
    if (n < 10) { buf[1] = '0' + n; buf[2] = '\0'; }
    else { buf[1] = '0' + (n / 10); buf[2] = '0' + (n % 10); buf[3] = '\0'; }
    return buf;
}

static int op_precedence(char op) {
    if (op == '*' || op == '/') return 2;
    if (op == '+' || op == '-') return 1;
    return 0;
}

static void emit(const char *left, const char *right, char op) {
    if (g_tac_count >= MAX_TAC) return;
    {
        const char *tmp = next_temp();
        int k;
        for (k = 0; k < 32 && tmp[k] != '\0'; k++)
            g_tac[g_tac_count].result[k] = tmp[k];
        g_tac[g_tac_count].result[k] = '\0';

        k = 0;
        while (left[k] != '\0' && k < 31) {
            g_tac[g_tac_count].arg1[k] = left[k]; k++;
        }
        g_tac[g_tac_count].arg1[k] = '\0';

        k = 0;
        while (right[k] != '\0' && k < 31) {
            g_tac[g_tac_count].arg2[k] = right[k]; k++;
        }
        g_tac[g_tac_count].arg2[k] = '\0';

        g_tac[g_tac_count].op = op;
        g_tac[g_tac_count].is_temp = 1;
        g_tac_count++;
    }
}

static void resolve_e(char *out) {
    int i;
    for (i = g_tac_count - 1; i >= 0; i--) {
        if (g_tac[i].result[0] == 't' && g_tac[i].is_temp) {
            int k;
            for (k = 0; g_tac[i].result[k] != '\0' && k < 31; k++)
                out[k] = g_tac[i].result[k];
            out[k] = '\0';
            return;
        }
    }
    out[0] = 'E';
    out[1] = '\0';
}

static int build_tac(const Token tokens[], int token_count, TACInstruction tac[]) {
    static char op_stack[MAX_STACK];
    static int op_top = -1;
    static char val_stack[MAX_STACK][32];
    static int val_top = -1;
    int i;

    op_top = -1;
    val_top = -1;
    g_tac_count = 0;
    temp_counter = 0;

    for (i = 0; i < token_count; i++) {
        TokenType t = tokens[i].type;

        if (t == TOK_ID) {
            val_top++;
            {
                int k = 0;
                while (tokens[i].value[k] != '\0' && k < 31) {
                    val_stack[val_top][k] = tokens[i].value[k]; k++;
                }
                val_stack[val_top][k] = '\0';
            }
        } else if (t == TOK_LPAREN) {
            op_top++;
            op_stack[op_top] = '(';
        } else if (t == TOK_RPAREN) {
            while (op_top >= 0 && op_stack[op_top] != '(') {
                if (val_top < 1) break;
                {
                    char b[32], a[32];
                    int k;
                    for (k = 0; k < 32; k++) b[k] = val_stack[val_top][k];
                    val_top--;
                    for (k = 0; k < 32; k++) a[k] = val_stack[val_top][k];
                    val_top--;
                    emit(a, b, op_stack[op_top]);
                    op_top--;

                    {
                        int n = temp_counter;
                        char tmp[16];
                        tmp[0] = 't';
                        if (n < 10) { tmp[1] = '0' + n; tmp[2] = '\0'; }
                        else { tmp[1] = '0' + (n/10); tmp[2] = '0' + (n%10); tmp[3] = '\0'; }
                        val_top++;
                        for (k = 0; tmp[k] != '\0' && k < 31; k++)
                            val_stack[val_top][k] = tmp[k];
                        val_stack[val_top][k] = '\0';
                    }
                }
            }
            if (op_top >= 0 && op_stack[op_top] == '(') op_top--;
        } else if (is_op(t)) {
            char op = tokens[i].value[0];
            while (op_top >= 0 && op_stack[op_top] != '(' &&
                   op_precedence(op_stack[op_top]) >= op_precedence(op)) {
                if (val_top < 1) break;
                {
                    char b[32], a[32];
                    int k;
                    for (k = 0; k < 32; k++) b[k] = val_stack[val_top][k];
                    val_top--;
                    for (k = 0; k < 32; k++) a[k] = val_stack[val_top][k];
                    val_top--;
                    emit(a, b, op_stack[op_top]);
                    op_top--;

                    {
                        int n = temp_counter;
                        char tmp[16];
                        tmp[0] = 't';
                        if (n < 10) { tmp[1] = '0' + n; tmp[2] = '\0'; }
                        else { tmp[1] = '0' + (n/10); tmp[2] = '0' + (n%10); tmp[3] = '\0'; }
                        val_top++;
                        for (k = 0; tmp[k] != '\0' && k < 31; k++)
                            val_stack[val_top][k] = tmp[k];
                        val_stack[val_top][k] = '\0';
                    }
                }
            }
            op_top++;
            op_stack[op_top] = op;
        }
    }

    while (op_top >= 0 && op_stack[op_top] != '(') {
        if (val_top < 1) break;
        {
            char b[32], a[32];
            int k;
            for (k = 0; k < 32; k++) b[k] = val_stack[val_top][k];
            val_top--;
            for (k = 0; k < 32; k++) a[k] = val_stack[val_top][k];
            val_top--;
            emit(a, b, op_stack[op_top]);
            op_top--;

            {
                int n = temp_counter;
                char tmp[16];
                tmp[0] = 't';
                if (n < 10) { tmp[1] = '0' + n; tmp[2] = '\0'; }
                else { tmp[1] = '0' + (n/10); tmp[2] = '0' + (n%10); tmp[3] = '\0'; }
                val_top++;
                for (k = 0; tmp[k] != '\0' && k < 31; k++)
                    val_stack[val_top][k] = tmp[k];
                val_stack[val_top][k] = '\0';
            }
        }
    }

    for (i = 0; i < g_tac_count && i < MAX_TAC; i++) {
        int r;
        tac[i].op = g_tac[i].op;
        tac[i].is_temp = g_tac[i].is_temp;
        for (r = 0; g_tac[i].result[r] != '\0' && r < 31; r++)
            tac[i].result[r] = g_tac[i].result[r];
        tac[i].result[r] = '\0';
        for (r = 0; g_tac[i].arg1[r] != '\0' && r < 31; r++)
            tac[i].arg1[r] = g_tac[i].arg1[r];
        tac[i].arg1[r] = '\0';
        for (r = 0; g_tac[i].arg2[r] != '\0' && r < 31; r++)
            tac[i].arg2[r] = g_tac[i].arg2[r];
        tac[i].arg2[r] = '\0';
    }

    return g_tac_count;
}

static void show_tac(const TACInstruction tac[], int count) {
    int i;
    write_str("\nThree Address Code:\n");
    for (i = 0; i < count; i++) {
        write_str("  ");
        write_str(tac[i].result);
        write_str(" = ");
        write_str(tac[i].arg1);
        write_char(' ');
        write_char(tac[i].op);
        write_char(' ');
        write_str(tac[i].arg2);
        write_newline();
    }
}

static StackEntry pstack[MAX_STACK];
static int pstack_top = -1;

static void push_stack(TokenType t, const char *val) {
    if (pstack_top >= MAX_STACK - 1) return;
    pstack_top++;
    {
        int k = 0;
        while (val[k] != '\0' && k < 31) {
            pstack[pstack_top].value[k] = val[k]; k++;
        }
        pstack[pstack_top].value[k] = '\0';
    }
    pstack[pstack_top].type = t;
}

static StackEntry pop_stack(void) {
    StackEntry entry;
    entry.type = TOK_END;
    entry.value[0] = '\0';
    if (pstack_top >= 0) {
        entry = pstack[pstack_top];
        pstack_top--;
    }
    return entry;
}

static TokenType top_terminal(void) {
    int i;
    for (i = pstack_top; i >= 0; i--) {
        if (pstack[i].type == TOK_ID && str_cmp(pstack[i].value, "E") == 0)
            continue;
        return pstack[i].type;
    }
    return TOK_END;
}

static int reduce(void) {
    if (pstack_top >= 2 &&
        pstack[pstack_top].type == TOK_ID &&
        is_op(pstack[pstack_top - 1].type) &&
        pstack[pstack_top - 2].type == TOK_ID) {
        StackEntry right = pop_stack();
        StackEntry op    = pop_stack();
        StackEntry left  = pop_stack();
        emit(left.value, right.value, op.value[0]);
        push_stack(TOK_ID, "E");
        return 3;
    }

    if (pstack_top >= 2 &&
        pstack[pstack_top].type == TOK_ID &&
        is_op(pstack[pstack_top - 1].type) &&
        pstack[pstack_top - 2].type == TOK_ID &&
        str_cmp(pstack[pstack_top - 2].value, "E") == 0) {
        StackEntry right = pop_stack();
        StackEntry op    = pop_stack();
        char left_name[32];
        resolve_e(left_name);
        emit(left_name, right.value, op.value[0]);
        return 2;
    }

    if (pstack_top >= 0 &&
        pstack[pstack_top].type == TOK_ID &&
        str_cmp(pstack[pstack_top].value, "E") != 0) {
        pstack[pstack_top].value[0] = 'E';
        pstack[pstack_top].value[1] = '\0';
        return 1;
    }

    return 0;
}

static void handle_close_paren(void) {
    while (TRUE) {
        TokenType tt = top_terminal();
        if (tt == TOK_LPAREN) break;
        if (tt == TOK_END) break;
        if (reduce() == 0) break;
    }
    {
        int i;
        for (i = pstack_top; i >= 0; i--) {
            if (pstack[i].type == TOK_LPAREN) {
                int j;
                for (j = i; j < pstack_top; j++)
                    pstack[j] = pstack[j + 1];
                pstack_top--;
                break;
            }
        }
    }
}

static void show_stack(void) {
    int i;
    write_str("Stack: [");
    for (i = 0; i <= pstack_top; i++) {
        write_str(pstack[i].value);
        if (i < pstack_top) write_str(" ");
    }
    write_str("]");
}

static void show_input(const Token tokens[], int ip, int tc) {
    int i;
    write_str(" Input: [");
    for (i = ip; i < tc; i++) {
        write_str(tokens[i].value);
        if (i < tc - 1) write_str(" ");
    }
    write_str("]");
}

static ParseResult parse(const Token tokens[], int token_count) {
    int ip = 0;
    int iterations = 0;

    pstack_top = -1;
    push_stack(TOK_END, "$");

    while (TRUE) {
        if (++iterations > 5000) return RESULT_REJECT;
        if (ip >= token_count) return RESULT_REJECT;

        TokenType stack_t = top_terminal();
        TokenType input_t = tokens[ip].type;

        show_stack();
        show_input(tokens, ip, token_count);
        write_str(" -> ");

        if (stack_t == TOK_END && input_t == TOK_END) {
            write_str("ACCEPT\n");
            return RESULT_ACCEPT;
        }

        if (input_t == TOK_RPAREN) {
            write_str("REDUCE_Paren ");
            handle_close_paren();
            write_newline();
            ip++;
            continue;
        }

        Relation rel = lookup(stack_t, input_t);

        if (rel == REL_SHIFT) {
            write_str("SHIFT\n");
            push_stack(input_t, tokens[ip].value);
            ip++;
        } else if (rel == REL_REDUCE) {
            write_str("REDUCE\n");
            if (reduce() == 0) {
                write_str("REJECT (stuck)\n");
                return RESULT_REJECT;
            }
        } else if (rel == REL_EQUAL) {
            write_str("EQUAL\n");
            push_stack(input_t, tokens[ip].value);
            ip++;
        } else {
            write_str("REJECT\n");
            return RESULT_REJECT;
        }
    }
}

static int matches(const char *s, const char *cmd) {
    return str_cmp(s, cmd) == 0;
}

int main(void) {
    char input[MAX_INPUT];
    Token tokens[MAX_TOKENS];
    TACInstruction tac[MAX_TAC];

    write_str("========================================\n");
    write_str("  Operator Precedence Expression Compiler\n");
    write_str("  Type 'table' to see precedence table\n");
    write_str("  Type 'exit' or 'quit' to quit\n");
    write_str("========================================\n\n");

    while (TRUE) {
        write_str("> ");
        {
            int len = read_line(input, MAX_INPUT);
            if (len < 0) break;
            if (len == 0) continue;

            if (matches(input, "exit") || matches(input, "quit")) break;

            if (matches(input, "table")) {
                show_table();
                continue;
            }

            write_str("\nInput: ");
            write_str(input);
            write_newline();

            int tc = tokenize(input, tokens, MAX_TOKENS);
            write_str("\nTokens:\n");
            {
                int ti;
                for (ti = 0; ti < tc; ti++) {
                    show_token(&tokens[ti]);
                    write_newline();
                }
            }

            ErrorCode err = validate(tokens, tc);
            if (err != ERR_NONE) {
                show_error(err);
                write_newline();
                continue;
            }
            write_str("\nValidation: PASSED\n");

            write_str("\nParsing (Shift-Reduce):\n");
            ParseResult presult = parse(tokens, tc);
            if (presult != RESULT_ACCEPT) {
                write_str("\nParse Result: REJECTED\n");
                continue;
            }
            write_str("\nParse Result: ACCEPTED\n");

            int tac_count = build_tac(tokens, tc, tac);
            show_tac(tac, tac_count);
            write_newline();
        }
    }

    write_str("Goodbye!\n");
    return 0;
}
