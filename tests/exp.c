#include <stdio.h>
#include <pgen/pgen.h>

// S -> exp
// exp -> exp + NUM
// exp -> NUM
// exp -> ( exp )

struct {
    int S;
    int exp;
    int NUM;
    int PLUS;
    int L_PAREN;
    int R_PAREN;
} S;

int main(int argc, char **argv) {
    Grammar *g = newgrammar();

    S.NUM = addsym(g, newsym(S_TERM, "NUM"));
    S.PLUS = addsym(g, newsym(S_TERM, "+"));
    S.L_PAREN = addsym(g, newsym(S_TERM, "("));
    S.R_PAREN = addsym(g, newsym(S_TERM, ")"));

    S.S = addsym(g, newsym(S_NON_TERM, "S"));
    S.exp = addsym(g, newsym(S_NON_TERM, "exp"));

    // right-hand sides must be termianted by a 0
    // 0 is the id of a reserved/invalid symbol
    // for ε-productions your right-hand sides would just contain a 0
    
    // S -> exp
    addrule(g, newrule(S.S, (int[]){S.exp, 0}));

    // exp -> exp + NUM
    addrule(g, newrule(S.exp, (int[]){S.exp, S.PLUS, S.NUM, 0}));

    // exp -> NUM
    addrule(g, newrule(S.exp, (int[]){S.NUM, 0}));

    // exp -> ( exp )
    addrule(g, newrule(S.exp, (int[]){S.L_PAREN, S.exp, S.R_PAREN, 0}));

    // select type of table you'd like to generate
    g->type = G_SLR;

    // specify your start symbol
    g->start = S.S;

    // generate parsing table and everything else along the way
    bake(g);

    dumpall(g);
    dotdumpstates(g, "states.dot");
    dotdumptable(g, "table.dot");

    freegrammar(g);
    return 0;
}
