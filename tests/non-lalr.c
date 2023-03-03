#include <stdio.h>
#include <pgen/pgen.h>

// S -> a E c
// S -> a F d
// S -> b F c
// S -> b E d
// E -> e
// F -> e

struct {
    int S;
    int E;
    int F;
    int a;
    int b;
    int c;
    int d;
    int e;
} S;

int main(int argc, char **argv) {
    Grammar *g = newgrammar();
    
    S.a = addsym(g, newsym(S_TERM, "a"));
    S.b = addsym(g, newsym(S_TERM, "b"));
    S.c = addsym(g, newsym(S_TERM, "c"));
    S.d = addsym(g, newsym(S_TERM, "d"));
    S.e = addsym(g, newsym(S_TERM, "e"));

    S.S = addsym(g, newsym(S_NON_TERM, "S"));
    S.E = addsym(g, newsym(S_NON_TERM, "E"));
    S.F = addsym(g, newsym(S_NON_TERM, "F"));

    addrule(g, newrule(S.S, (int[]){S.a, S.E, S.c, 0}));
    addrule(g, newrule(S.S, (int[]){S.a, S.F, S.d, 0}));
    addrule(g, newrule(S.S, (int[]){S.b, S.F, S.c, 0}));
    addrule(g, newrule(S.S, (int[]){S.b, S.E, S.d, 0}));
    addrule(g, newrule(S.E, (int[]){S.e, 0}));
    addrule(g, newrule(S.F, (int[]){S.e, 0}));

    // g->type = G_LR1;
    g->type = G_LALR1;
    g->start = S.S;

    bake(g);

    // dumpall(g);
    dotdumpstates(g, "states.dot");
    dotdumptable(g, "table.dot");

    freegrammar(g);
    return 0;
}
