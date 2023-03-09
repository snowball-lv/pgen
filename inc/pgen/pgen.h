#pragma once

typedef struct {
    enum {
        _S_NONE,
        S_TERM,
        S_NON_TERM,
    } type;
    char *name;
    char *alias;
} Sym;

typedef struct {
    int lhs;
    int *rhs;
    int nrhs;
} Rule;

typedef struct {
    int rule;
    int dot;
    int lookahead;
} Item;

typedef struct {
    Item *items;
    int nitems;
} State;

typedef struct {
    enum {
        _AT_NONE,
        AT_SHIFT,
        AT_GOTO,
        AT_REDUCE,
        AT_ACCEPT,
    } type;
    int num;
} Action;

typedef struct {
    int from, to;
    int sym;  
} Edge;

enum {
    G_LR0,
    G_SLR,
    G_LR1,
    G_LALR1,
};

typedef struct {
    int type;
    Sym **syms;
    int nsyms;
    Rule **rules;
    int nrules;
    int start;
    int startrule;
    char *nullable;
    char **first;
    char **follow;
    State **states;
    int nstates;
    Action *table;
    Edge *edges;
    int nedges;
} Grammar;

#define S_EOI 1

Grammar *newgrammar();
void freegrammar(Grammar *g);
int addsym(Grammar *g, Sym *sym);
Sym *newsym(int type, char *name);
Rule *newrule(int lhs, int *rhs);
int addrule(Grammar *g, Rule *r);
void setalias(Grammar *g, int sym, char *alias);

void bake(Grammar *g);

void dumpsyms(Grammar *g);
void dumprules(Grammar *g);
void dumpnullable(Grammar *g);
void dumpfirst(Grammar *g);
void dumpfollow(Grammar *g);
void dumpstates(Grammar *g);

void dumpall(Grammar *g);

void dotdumpstates(Grammar *g, const char *path);
void dotdumptable(Grammar *g, const char *path);

void genc(Grammar *g, char *usrcode, FILE *fp);
