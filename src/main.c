#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <pgen/pgen.h>

enum {
    T_NONE,
    T_EOF,
    T_SYM,
    T_ARROW,
    T_STR,
    T_BAR,
    T_SEMI,
    T_START,
    T_TERM,
    T_CODE,
};

typedef struct {
    int type;
    char *start;
    int len;
    int sym;
} Tok;

typedef struct {
    char *src;
    Tok prev;
    Tok cur;
    Grammar *g;
    char **strs;
    int nstrs;
    Tok *syms;
    int nsyms;
    char *code;
} Parser;

static char *readfile(char *path) {
    FILE *fp = fopen(path, "r");
    if (!fp) {
        printf("*** couldn't open file %s\n", path);
        exit(1);
    }
    fseek(fp, 0, SEEK_END);
    int sz = ftell(fp);
    rewind(fp);
    char *buf = malloc(sz + 1);
    int n = fread(buf, sz, 1, fp);
    fclose(fp);
    if (n) return buf;
    free(buf);
    return 0;
}

static Tok nexttok(Parser *p) {
    while (isspace(*p->src))
        p->src++;
    char *start = p->src;
    switch (*start) {
    case 0: return (Tok){T_EOF, start, 0};
    case '|': p->src++; return (Tok){T_BAR, start, 1};
    case ';': p->src++; return (Tok){T_SEMI, start, 1};
    case '"': p->src++; goto str;
    case '%': p->src++; goto sym;
    case '#':
        while (*p->src && *p->src != '\n')
            p->src++;
        return nexttok(p);
    }
    if (start[0] == '-' && start[1] == '>') {
        p->src += 2;
        return (Tok){T_ARROW, start, 2};
    }
    else if (isalpha(*start)) goto sym;
    printf("*** unexpected character %c\n", *start);
    exit(1);
sym:
    while (isalnum(*p->src))
        p->src++;
    return (Tok){T_SYM, start, p->src - start};
str:
    while (*p->src) {
        p->src++;
        if (p->src[-1] == '"')
            return (Tok){T_STR, start, p->src - start};
    }
    printf("*** unterminated string\n");
    exit(1);
}

static void advance(Parser *p) {
    p->prev = p->cur;
    p->cur = nexttok(p);
    if (p->cur.type == T_SYM && p->cur.start[0] == '%') {
        if (strncmp(p->cur.start, "%start", p->cur.len) == 0)
            p->cur.type = T_START;
        else if (strncmp(p->cur.start, "%term", p->cur.len) == 0)
            p->cur.type = T_TERM;
        else if (strncmp(p->cur.start, "%code", p->cur.len) == 0)
            p->cur.type = T_CODE;
    }
}

static int match(Parser *p, int type) {
    if (p->cur.type != type) return 0;
    advance(p);
    return 1;
}

static int peek(Parser *p) {
    return p->cur.type;
}

static void expect(Parser *p, int type) {
    if (match(p, type)) return;
    printf("*** unexpected token %.*s\n", p->cur.len, p->cur.start);
    exit(1);
}

static char *getstr(Parser *p, Tok t) {
    for (int i = 0; i < p->nstrs; i++) {
        char *str = p->strs[i];
        if (strlen(str) != t.len) continue;
        if (strncmp(str, t.start, t.len) == 0) return str;
    }
    char *cpy = malloc(t.len + 1);
    memcpy(cpy, t.start, t.len);
    cpy[t.len] = 0;
    p->nstrs++;
    p->strs = realloc(p->strs, p->nstrs * sizeof(char *));
    p->strs[p->nstrs - 1] = cpy;
    return cpy;
}

static int getsym(Parser *p, Tok t) {
    for (int i = 0; i < p->nsyms; i++) {
        Tok b = p->syms[i];
        if (t.len != b.len) continue;
        if (strncmp(t.start, b.start, t.len) == 0)
            return p->syms[i].sym;
    }
    return 0;
}

static void addtoksym(Parser *p, Tok t) {
    p->nsyms++;
    p->syms = realloc(p->syms, p->nsyms * sizeof(Tok));
    p->syms[p->nsyms - 1] = t;
}

static int getnonterm(Parser *p, Tok t) {
    int sym = getsym(p, t);
    if (sym) {
        if (p->g->syms[sym]->type == S_NON_TERM) return sym;
        printf("*** %.*s alread defined as a terminal\n", t.len, t.start);
        exit(1);
    }
    t.sym = addsym(p->g, newsym(S_NON_TERM, getstr(p, t)));
    addtoksym(p, t);
    return t.sym;
}

static int getterm(Parser *p, Tok t) {
    int sym = getsym(p, t);
    if (sym) {
        if (p->g->syms[sym]->type == S_TERM) return sym;
        printf("*** %.*s alread defined as a non-terminal\n", t.len, t.start);
        exit(1);
    }
    t.sym = addsym(p->g, newsym(S_TERM, getstr(p, t)));
    addtoksym(p, t);
    return t.sym;
}

static void parserule(Parser *p) {
    expect(p, T_SYM);
    Tok lhs = p->prev;
    int lhssym = getnonterm(p, lhs);
    if (!p->g->start)
        p->g->start = lhssym;
    expect(p, T_ARROW);
    while (1) {
        int rhs[64];
        int nrhs = 0;
        while (!match(p, T_BAR) && !match(p, T_SEMI)) {
            if (match(p, T_SYM)) {
                int sym = getsym(p, p->prev);
                if (!sym) sym = getnonterm(p, p->prev);
                rhs[nrhs] = sym;
                nrhs++;
            }
            else if (match(p, T_STR)) {
                int sym = getterm(p, p->prev);
                rhs[nrhs] = sym;
                nrhs++;
            }
            else {
                printf("*** unexpected token %.*s\n", p->cur.len, p->cur.start);
                exit(1);
            }
        }
        rhs[nrhs] = 0;
        addrule(p->g, newrule(lhssym, rhs));
        if (p->prev.type == T_SEMI) break;
    }
}

static void parse(Parser *p) {
    advance(p);
    while (!match(p, T_EOF)) {
        if (match(p, T_START)) {
            expect(p, T_SYM);
            p->g->start = getnonterm(p, p->prev);
        }
        else if (match(p, T_TERM)) {
            expect(p, T_SYM);
            getterm(p, p->prev);
        }
        else if (peek(p) == T_CODE) {
            int len = strlen(p->src);
            p->code = malloc(len + 1);
            strcpy(p->code, p->src);
            return;
        }
        else {
            parserule(p);
        }
    }
}

static char *OPTS[] = {
    "-h:print this message and exit",
    "-t type:type of parsing table to generate [lr0, slr, lr1, lalr1], default: lalr1",
    "-s:print symbols",
    "-r:print rules",
    "-n:print nullable set",
    "-fr:print first sets",
    "-fl:print follow sets",
    "-ds file:print state transitions to file in DOT",
    "-dt file:print parsing table to file in DOT",
    "-g file:generate a parser in C and write to file",
    0,
};

static void help() {
    printf("Usage: pgen [flags] file.g\n");
    for (char **opt = OPTS; *opt; opt++) {
        char *sep = strchr(*opt, ':');
        printf("%4s%-12.*s", "", (int)(sep - *opt), *opt);
        printf("%s\n", sep + 1);
    }
}

int main(int argc, char **argv) {
    char *path = 0;
    int psyms = 0;
    int prules = 0;
    int pnul = 0;
    int pfirst = 0;
    int pfollow = 0;
    char *dotstates = 0;
    char *dottable = 0;
    char *cparser = 0;
    int type = G_LALR1;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0) {
            help();
            return 0;
        }
        else if (strcmp(argv[i], "-s") == 0) psyms = 1;
        else if (strcmp(argv[i], "-r") == 0) prules = 1;
        else if (strcmp(argv[i], "-n") == 0) pnul = 1;
        else if (strcmp(argv[i], "-fr") == 0) pfirst = 1;
        else if (strcmp(argv[i], "-fl") == 0) pfollow = 1;
        else if (strcmp(argv[i], "-ds") == 0) dotstates = argv[++i];
        else if (strcmp(argv[i], "-dt") == 0) dottable = argv[++i];
        else if (strcmp(argv[i], "-t") == 0) {
            i++;
            if (strcmp(argv[i], "lr0") == 0) type = G_LR0;
            else if (strcmp(argv[i], "slr") == 0) type = G_SLR;
            else if (strcmp(argv[i], "lr1") == 0) type = G_LR1;
            else if (strcmp(argv[i], "lalr1") == 0) type = G_LALR1;
        }
        else if (strcmp(argv[i], "-g") == 0) cparser = argv[++i];
        else if (!path) path = argv[i];
        else {
            printf("*** stray argument %s\n", argv[i]);
            help();
            exit(1);
        }
    }
    if (!path) {
        printf("*** grammar file not specified\n");
        help();
        exit(1);
    }
    char *src = readfile(path);
    Parser p = {0};
    p.g = newgrammar();
    p.src = src;
    parse(&p);
    p.g->type = type;
    bake(p.g);
    if (psyms) dumpsyms(p.g);
    if (prules) dumprules(p.g);
    if (pnul) dumpnullable(p.g);
    if (pfirst) dumpfirst(p.g);
    if (pfollow) dumpfollow(p.g);
    if (dotstates) dotdumpstates(p.g, dotstates);
    if (dottable) dotdumptable(p.g, dottable);
    if (cparser) {
        FILE *fp = fopen(cparser, "w");
        if (fp) {
            genc(p.g, p.code, fp);
            fclose(fp);
        }
    }
    freegrammar(p.g);
    free(src);
    if (p.strs) free(p.strs);
    if (p.syms) free(p.syms);
    if (p.code) free(p.code);
    return 0;
}
