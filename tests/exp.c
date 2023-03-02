#include <stdio.h>
#include <pgen/pgen.h>

struct {
    int S;
    int exp;
    int NUM;
    int PLUS;
    int L_PAREN;
    int R_PAREN;
} S;

// S -> exp
// exp -> exp + NUM
// exp -> NUM
// exp -> ( exp )

int main(int argc, char **argv) {
    Grammar *g = newgrammar();

    S.NUM = addsym(g, newsym(S_TERM, "NUM"));
    S.PLUS = addsym(g, newsym(S_TERM, "+"));
    S.L_PAREN = addsym(g, newsym(S_TERM, "("));
    S.R_PAREN = addsym(g, newsym(S_TERM, ")"));

    S.S = addsym(g, newsym(S_NON_TERM, "S"));
    S.exp = addsym(g, newsym(S_NON_TERM, "exp"));

    addrule(g, newrule(S.S, (int[]){S.exp, 0}));
    addrule(g, newrule(S.exp, (int[]){S.exp, S.PLUS, S.NUM, 0}));
    addrule(g, newrule(S.exp, (int[]){S.NUM, 0}));
    addrule(g, newrule(S.exp, (int[]){S.L_PAREN, S.exp, S.R_PAREN, 0}));

    g->type = G_SLR;
    g->start = S.S;

    bake(g);
    dumpall(g);
    dotdumpstates(g, "states.dot");

    freegrammar(g);
    return 0;
}
