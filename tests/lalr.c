#include <stdio.h>
#include <pgen/pgen.h>

// S -> V = E
// S -> E
// E -> V
// V -> x
// V -> * E

struct {
    int S;
    int V;
    int E;
    int x;
    int eq;
    int star;
} S;

int main(int argc, char **argv) {
    Grammar *g = newgrammar();

    S.x = addsym(g, newsym(S_TERM, "x"));
    S.star = addsym(g, newsym(S_TERM, "*"));
    S.eq = addsym(g, newsym(S_TERM, "="));

    S.S = addsym(g, newsym(S_NON_TERM, "S"));
    S.E = addsym(g, newsym(S_NON_TERM, "E"));
    S.V = addsym(g, newsym(S_NON_TERM, "V"));
    
    addrule(g, newrule(S.S, (int[]){S.V, S.eq, S.E, 0}));
    addrule(g, newrule(S.S, (int[]){S.E, 0}));
    addrule(g, newrule(S.E, (int[]){S.V, 0}));
    addrule(g, newrule(S.V, (int[]){S.x, 0}));
    addrule(g, newrule(S.V, (int[]){S.star, S.E, 0}));

    // g->type = G_LR1;
    g->type = G_LALR1;
    g->start = S.S;

    bake(g);

    dumpall(g);
    dotdumpstates(g, "states.dot");
    dotdumptable(g, "table.dot");

    freegrammar(g);
    return 0;
}
