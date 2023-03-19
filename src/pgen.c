#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pgen/pgen.h>

Grammar *newgrammar() {
    Grammar *g = calloc(1, sizeof(Grammar));
    addsym(g, newsym(_S_NONE, "_zero"));
    addsym(g, newsym(S_TERM, "$"));
    return g;
}

void freegrammar(Grammar *g) {
    for (int i = 0; i < g->nsyms; i++)
        free(g->syms[i]);
    if (g->syms) free(g->syms);
    for (int i = 0; i < g->nrules; i++) {
        Rule *r = g->rules[i];
        if (r->rhs) free(r->rhs);
        free(r);
    }
    if (g->rules) free(g->rules);
    if (g->nullable) free(g->nullable);
    if (g->first) {
        for (int i = 0; i < g->nsyms; i++)
            free(g->first[i]);
        free(g->first);
    }
    if (g->follow) {
        for (int i = 0; i < g->nsyms; i++)
            free(g->follow[i]);
        free(g->follow);
    }
    if (g->states) {
        for (int i = 0; i < g->nstates; i++) {
            State *s = g->states[i];
            if (s->items) free(s->items);
            free(s);
        }
        free(g->states);
    }
    if (g->table) free(g->table);
    if (g->edges) free(g->edges);
    free(g);
}

Sym *newsym(int type, char *name) {
    Sym *sym = calloc(1, sizeof(Sym));
    sym->type = type;
    sym->name = name;
    return sym;
}

int addsym(Grammar *g, Sym *sym) {
    g->nsyms++;
    g->syms = realloc(g->syms, g->nsyms * sizeof(Sym *));
    g->syms[g->nsyms - 1] = sym;
    return g->nsyms - 1;
}

void setalias(Grammar *g, int sym, char *alias) {
    g->syms[sym]->alias = alias;
}

void setusraction(Grammar *g, int rule, char *usraction) {
    g->rules[rule]->usraction = usraction;
}

Rule *newrule(int lhs, int *rhs) {
    Rule *r = calloc(1, sizeof(Rule));
    r->lhs = lhs;
    while (rhs[r->nrhs])
        r->nrhs++;
    r->rhs = calloc(r->nrhs, sizeof(int));
    memcpy(r->rhs, rhs, r->nrhs * sizeof(int));
    return r;
}

int addrule(Grammar *g, Rule *r) {
    g->nrules++;
    g->rules = realloc(g->rules, g->nrules * sizeof(Rule *));
    g->rules[g->nrules - 1] = r;
    return g->nrules - 1;
}

void dumpsyms(Grammar *g) {
    printf("Symbols:\n");
    for (int i = 1; i < g->nsyms; i++) {
        Sym *sym = g->syms[i];
        printf("%4i: [%i] %s\n", i, sym->type, sym->name);
    }
}

static void dumprule(Grammar *g, int ri) {
    Rule *r = g->rules[ri];
    printf("%s ->", g->syms[r->lhs]->name);
    for (int k = 0; k < r->nrhs; k++)
        printf(" %s", g->syms[r->rhs[k]]->name);
    printf("\n");
}

void dumprules(Grammar *g) {
    printf("Rules:\n");
    for (int i = 0; i < g->nrules; i++) {
        printf("%4i: ", i);
        dumprule(g, i);
    }
}

void dumpnullable(Grammar *g) {
    printf("Nullable:\n");
    for (int i = 0; i < g->nsyms; i++) {
        if (!g->nullable[i]) continue;
        printf("%4i: %s\n", i, g->syms[i]->name);
    }
}

void dumpfirst(Grammar *g) {
    printf("First:\n");
    for (int i = 1; i < g->nsyms; i++) {
        printf("%4i: %-8s", i, g->syms[i]->name);
        char *first = g->first[i];
        for (int k = 1; k < g->nsyms; k++) {
            if (!first[k]) continue;
            printf(" %s", g->syms[k]->name);
        }
        printf("\n");
    }
}

void dumpfollow(Grammar *g) {
    printf("Follow:\n");
    for (int i = 1; i < g->nsyms; i++) {
        printf("%4i: %-8s", i, g->syms[i]->name);
        char *follow = g->follow[i];
        for (int k = 1; k < g->nsyms; k++) {
            if (!follow[k]) continue;
            printf(" %s", g->syms[k]->name);
        }
        printf("\n");
    }
}

static void dumpstate(Grammar *g, int si) {
    State *s = g->states[si];
    printf("State #%i:\n", si);
    for (int i = 0; i < s->nitems; i++) {
        Item item = s->items[i];
        Rule *r = g->rules[item.rule];
        printf("%4s%s ->", "", g->syms[r->lhs]->name);
        for (int k = 0; k < r->nrhs; k++) {
            if (k == item.dot) printf(" .");
            printf(" %s", g->syms[r->rhs[k]]->name);
        }
        if (r->nrhs == item.dot) printf(" .");
        if (g->type == G_LR1 || g->type == G_LALR1) {
            char *lookahead = "$";
            if (item.lookahead)
                lookahead = g->syms[item.lookahead]->name;
            printf(" [%s]", lookahead);
        }
        printf("\n");
    }
}

void dumpstates(Grammar *g) {
    printf("States:\n");
    for (int i = 0; i < g->nstates; i++)
        dumpstate(g, i);
}

void dumpall(Grammar *g) {
    printf("Start: %s\n", g->syms[g->start]->name);
    dumpsyms(g);
    dumprules(g);
    dumpnullable(g);
    dumpfirst(g);
    dumpfollow(g);
    dumpstates(g);
}

static void calcnullable(Grammar *g) {
    memset(g->nullable, 0, g->nsyms);
changed:
    for (int ri = 0; ri < g->nrules; ri++) {
        Rule *r = g->rules[ri];
        int nullable = 1;
        for (int i = 0; i < r->nrhs; i++) {
            if (g->nullable[r->rhs[i]]) continue;
            nullable = 0;
            break;
        }
        if (nullable && !g->nullable[r->lhs]) {
            g->nullable[r->lhs] = 1;
            goto changed;
        }
    }
}

static void setunion(char *dst, char *src, int n) {
    while (n--)
        dst[n] |= src[n];
}

static void calcfirst(Grammar *g) {
    for (int i = 0; i < g->nsyms; i++)
        memset(g->first[i], 0, g->nsyms);
    for (int i = 0; i < g->nsyms; i++) {
        if (g->syms[i]->type != S_TERM) continue;
        g->first[i][i] = 1;
    }
    char *tmp = calloc(g->nsyms, sizeof(char));
changed:
    for (int ri = 0; ri < g->nrules; ri++) {
        Rule *r = g->rules[ri];
        memcpy(tmp, g->first[r->lhs], g->nsyms);
        for (int *rhs = r->rhs; rhs < r->rhs + r->nrhs; rhs++) {
            setunion(g->first[r->lhs], g->first[*rhs], g->nsyms);
            if (!g->nullable[*rhs]) break;
        }
        if (memcmp(tmp, g->first[r->lhs], g->nsyms) != 0)
            goto changed;
    }
    free(tmp);
}

static void calcfollow(Grammar *g) {
    for (int i = 0; i < g->nsyms; i++)
        memset(g->follow[i], 0, g->nsyms);
    char *tmp = calloc(g->nsyms, sizeof(char));
changed:
    for (int ri = 0; ri < g->nrules; ri++) {
        Rule *r = g->rules[ri];
        for (int i = 0; i < r->nrhs; i++) {
            int a = r->rhs[i];
            memcpy(tmp, g->follow[a], g->nsyms);
            int nullable = 1;
            for (int k = i + 1; k < r->nrhs; k++) {
                int b = r->rhs[k];
                setunion(g->follow[a], g->first[b], g->nsyms);
                if (!g->nullable[b]) {
                    nullable = 0;
                    break;
                }
            }
            if (nullable)
                setunion(g->follow[a], g->follow[r->lhs], g->nsyms);
            if (memcmp(tmp, g->follow[a], g->nsyms) != 0)
                goto changed;
        }
    }
    free(tmp);
}

static State *newstate() {
    State *s = calloc(1, sizeof(State));
    return s;
}

static int itemeq(Item a, Item b) {
    return a.rule == b.rule && a.dot == b.dot && a.lookahead == b.lookahead;
}

static int hasitem(State *s, Item item) {
    for (int i = 0; i < s->nitems; i++)
        if (itemeq(s->items[i], item))
            return 1;
    return 0;
}

static int additem(State *s, Item item) {
    if (hasitem(s, item))
        return 0;
    s->nitems++;
    s->items = realloc(s->items, s->nitems * sizeof(Item));
    s->items[s->nitems - 1] = item;
    return 1;
}

static void first(char *dst, Grammar *g, int syma, int symb) {
    memset(dst, 0, g->nsyms);
    if (syma) {
        setunion(dst, g->first[syma], g->nsyms);
        if (!g->nullable[syma]) return;
    }
    if (symb) {
        setunion(dst, g->first[symb], g->nsyms);
    }
}

static void closure(State *s, Grammar *g) {
    char *tmp = calloc(1, g->nsyms);
change:
    for (int i = 0; i < s->nitems; i++) {
        Item item = s->items[i];
        Rule *r = g->rules[item.rule];
        if (item.dot >= r->nrhs) continue;
        int sym = r->rhs[item.dot];
        if (g->syms[sym]->type != S_NON_TERM) continue;
        int repeat = 0;
        for (int ri = 0; ri < g->nrules; ri++) {
            if (g->rules[ri]->lhs != sym) continue;
            if (g->type == G_LR1 || g->type == G_LALR1) {
                int nextsym = 0;
                if (item.dot + 1 < r->nrhs)
                    nextsym = r->rhs[item.dot + 1];
                first(tmp, g, nextsym, item.lookahead);
                for (int k = 1; k < g->nsyms; k++) {
                    if (!tmp[k]) continue;
                    repeat |= additem(s, (Item){ri, 0, k});
                }
            }
            else {
                repeat |= additem(s, (Item){ri, 0});
            }
        }
        if (repeat)
            goto change;
    }
    free(tmp);
}

static int addstate(Grammar *g, State *s) {
    g->nstates++;
    g->states = realloc(g->states, g->nstates * sizeof(State *));
    g->states[g->nstates - 1] = s;
    return g->nstates - 1;
}

static int edgeeq(Edge a, Edge b) {
    return a.from == b.from && a.to == b.to && a.sym == b.sym;
}

static int hasedge(Grammar *g, Edge e) {
    for (int i = 0; i < g->nedges; i++)
        if (edgeeq(g->edges[i], e))
            return 1;
    return 0;
}

static State *goto_(State *s, int sym, Grammar *g) {
    State *j = newstate();
    for (int i = 0; i < s->nitems; i++) {
        Item item = s->items[i];
        Rule *r = g->rules[item.rule];
        if (item.dot >= r->nrhs) continue;
        if (r->rhs[item.dot] != sym) continue;
        item.dot++;
        additem(j, item);
    }
    closure(j, g);
    return j;
}

static int addedge(Grammar *g, Edge e) {
    g->nedges++;
    g->edges = realloc(g->edges, g->nedges * sizeof(Edge));
    g->edges[g->nedges - 1] = e;
    return g->nedges - 1;
}

static int stateeq(State *a, State *b) {
    if (a->nitems != b->nitems) return 0;
    for (int i = 0; i < b->nitems; i++)
        if (!hasitem(a, b->items[i]))
            return 0;
    return 1;
}

static int stateeqlalr(State *a, State *b) {
    for (int i = 0; i < a->nitems; i++) {
        Item ia = a->items[i];
        for (int k = 0; k < b->nitems; k++) {
            Item ib = b->items[k];
            if (ia.rule == ib.rule && ia.dot == ib.dot)
                goto found;
        }
        return 0;
found:
        continue;
    }
    return 1;
}

static int findstate(Grammar *g, State *s) {
    for (int i = 0; i < g->nstates; i++) {
        if (g->type == G_LALR1 && stateeqlalr(g->states[i], s))
            return i;
        else if (g->type != G_LALR1 && stateeq(g->states[i], s))
            return i;
    }
    return -1;
}

static void freestate(State *s) {
    free(s->items);
    s->nitems = 0;
    s->items = 0;
}

static const char *aname(Action a) {
    switch (a.type) {
    case AT_SHIFT: return "shift";
    case AT_REDUCE: return "reduce";
    case AT_GOTO: return "goto";
    case AT_ACCEPT: return "accept";
    default: return "???";
    }
}

void dumpstate(Grammar *g, int si);
void dumprule(Grammar *g, int ri);

static void dumpaction(Grammar *g, Action a) {
    switch (a.type) {
    case AT_SHIFT:
        printf("shift to state %i\n", a.num);
        dumpstate(g, a.num);
        break;
    case AT_GOTO: printf("goto state %i\n", a.num); break;
    case AT_REDUCE:
        printf("reduce by rule ");
        dumprule(g, a.num);
        break;
    case AT_ACCEPT: printf("AT_ACCEPT\n"); break;
    default: printf("?"); break;
    }
}

static void setaction(Grammar *g, int state, int sym, Action a) {
    Action *t = &g->table[state * g->nsyms + sym];
    if (t->type == _AT_NONE) {
        *t = a;
        return;
    }
    if (t->type != a.type || t->num != a.num) {
        printf("*** %s/%s conflict in state %i\n", aname(*t), aname(a), state);
        dumpstate(g, state);
        printf("*** on symbol %s ", g->syms[sym]->name);
        dumpaction(g, *t);
        printf("*** on symbol %s ", g->syms[sym]->name);
        dumpaction(g, a);
        exit(1);
    }
}

static void calctable(Grammar *g) {
    State *s = newstate();
    additem(s, (Item){g->startrule, 0});
    closure(s, g);
    addstate(g, s);
changed:
    for (int i = 0; i < g->nstates; i++) {
        State *s = g->states[i];
        int repeat = 0;
        for (int k = 0; k < s->nitems; k++) {
            Item item = s->items[k];
            Rule *r = g->rules[item.rule];
            if (item.dot >= r->nrhs) continue;
            int sym = r->rhs[item.dot];
            if (sym == S_EOI) {
                Edge e = {i, 0, S_EOI};
                if (!hasedge(g, e)) {
                    addedge(g, e);
                    repeat = 1;
                }
                continue;
            }
            State *j = goto_(s, sym, g);
            int ji = findstate(g, j);
            if (ji != -1) {
                if (g->type == G_LALR1) {
                    for (int n = 0; n < j->nitems; n++)
                        additem(g->states[ji], j->items[n]);
                }
                freestate(j);
            }
            else {
                ji = addstate(g, j);
                repeat = 1;
            }
            Edge e = {i, ji, sym};
            if (!hasedge(g, e)) {
                addedge(g, e);
                repeat = 1;
            }
        }
        if (repeat)
            goto changed;
    }
    g->table = calloc(g->nstates, g->nsyms * sizeof(Action));
    for (Edge *e = g->edges; e < g->edges + g->nedges; e++) {
        Sym *sym = g->syms[e->sym];
        Action a = {0};
        a.num = e->to;
        if (e->sym == S_EOI) a.type = AT_ACCEPT;
        else if (sym->type == S_TERM) a.type = AT_SHIFT;
        else if (sym->type == S_NON_TERM) a.type = AT_GOTO;
        setaction(g, e->from, e->sym, a);
    }
    for (int i = 0; i < g->nstates; i++) {
        State *s = g->states[i];
        for (Item *item = s->items; item < s->items + s->nitems; item++) {
            Rule *r = g->rules[item->rule];
            if (item->dot < r->nrhs) continue;
            if (g->type == G_LR1 || g->type == G_LALR1) {
                Action a = {AT_REDUCE, item->rule};
                setaction(g, i, item->lookahead, a);
                continue;
            }
            for (int k = 1; k < g->nsyms; k++) {
                if (g->syms[k]->type != S_TERM) continue;
                if (g->type == G_SLR) {
                    if (!g->follow[r->lhs][k]) continue;
                }
                Action a = {AT_REDUCE, item->rule};
                setaction(g, i, k, a);
            }
        }
    }
}

void bake(Grammar *g) {
    int Sp = addsym(g, newsym(S_NON_TERM, "S'"));
    g->startrule = addrule(g, newrule(Sp, (int[]){g->start, S_EOI, 0}));
    g->start = Sp;
    g->nullable = calloc(g->nsyms, sizeof(char));
    calcnullable(g);
    g->first = calloc(g->nsyms, sizeof(char *));
    for (int i = 0; i < g->nsyms; i++)
        g->first[i] = calloc(g->nsyms, sizeof(char));
    calcfirst(g);
    g->follow = calloc(g->nsyms, sizeof(char *));
    for (int i = 0; i < g->nsyms; i++)
        g->follow[i] = calloc(g->nsyms, sizeof(char));
    calcfollow(g);
    calctable(g);
}

static void dotprintsafe(FILE *fp, char *str) {
    while (*str) {
        switch (*str) {
            case '{': case '}':
            case '<': case '>':
            case '|': case '"':
                fprintf(fp, "\\");
        }
        fprintf(fp, "%c", *str);
        str++;
    }
}

static void dotprintitem(FILE *fp, Item item, Grammar *g) {
    Rule *r = g->rules[item.rule];
    dotprintsafe(fp, g->syms[r->lhs]->name);
    dotprintsafe(fp, " ->");
    for (int i = 0; i < r->nrhs; i++) {
        if (item.dot == i) fprintf(fp, " .");
        fprintf(fp, " ");
        dotprintsafe(fp, g->syms[r->rhs[i]]->name);
    }
    if (item.dot >= r->nrhs) fprintf(fp, " .");
}

void dotdumpstates(Grammar *g, const char *path) {
    FILE *fp = fopen(path, "w");
    if (!fp) return;
    fprintf(fp, "digraph {\n");
    fprintf(fp, "rankdir=LR;\n");
    // fprintf(fp, "node [shape=box];\n");
    fprintf(fp, "node [shape=record];\n");
    for (int si = 0; si < g->nstates; si++) {
        State *s = g->states[si];
        fprintf(fp, "%i [", si);
        fprintf(fp, "label=\"");
        fprintf(fp, "State #%i|{{", si);
        for (Item *i = s->items; i < s->items + s->nitems; i++) {
            if (i != s->items) fprintf(fp, "|");
            dotprintitem(fp, *i, g);
            fprintf(fp, "\\l");
        }
        fprintf(fp, "}");
        if (g->type == G_LR1 || g->type == G_LALR1) {
            fprintf(fp, "|{");
            for (Item *i = s->items; i < s->items + s->nitems; i++) {
                if (i != s->items) fprintf(fp, "|");
                char *sym = "?";
                if (i->lookahead)
                    sym = g->syms[i->lookahead]->name;
                dotprintsafe(fp, sym);
            }
            fprintf(fp, "}");
        }
        fprintf(fp, "}\"]\n");
    }
    for (Edge *e = g->edges; e < g->edges + g->nedges; e++) {
        if (e->sym == S_EOI) continue;
        fprintf(fp, "%i -> %i [label=\"", e->from, e->to);
        dotprintsafe(fp, g->syms[e->sym]->name);
        fprintf(fp, "\"];\n");
    }
    fprintf(fp, "}\n");
    fclose(fp);
}

static Action getaction(Grammar *g, int state, int sym) {
    return g->table[state * g->nsyms + sym];
}

static void dotdumpsymcol(Grammar *g, FILE *fp, int sym) {
    fprintf(fp, "|{");
    dotprintsafe(fp, g->syms[sym]->name);
    for (int si = 0; si < g->nstates; si++) {
        Action a = getaction(g, si, sym);
        switch (a.type) {
        case AT_SHIFT: fprintf(fp, "|s%i", a.num); break;
        case AT_REDUCE: fprintf(fp, "|r%i", a.num); break;
        case AT_GOTO: fprintf(fp, "|g%i", a.num); break;
        case AT_ACCEPT: fprintf(fp, "|a"); break;
        default: fprintf(fp, "|"); break;
        }
    }
    fprintf(fp, "}");
}

void dotdumptable(Grammar *g, const char *path) {
    FILE *fp = fopen(path, "w");
    if (!fp) return;
    fprintf(fp, "digraph {\n");
    fprintf(fp, "node [shape=record];\n");
    fprintf(fp, "0 [");
    fprintf(fp, "label=\"");
    
    fprintf(fp, "{");
    for (int i = 0; i < g->nstates; i++) {
        fprintf(fp, "|%i", i);
    }
    fprintf(fp, "}");

    // terminals
    for (int i = 0; i < g->nsyms; i++)
        if (g->syms[i]->type == S_TERM && i != S_EOI)
            dotdumpsymcol(g, fp, i);

    // eof terminal
    dotdumpsymcol(g, fp, S_EOI);
    
    // non-terminals
    for (int i = 0; i < g->nsyms; i++)
        if (g->syms[i]->type == S_NON_TERM && i != g->start)
            dotdumpsymcol(g, fp, i);

    fprintf(fp, "\"");
    fprintf(fp, "];\n");
    fprintf(fp, "}");
    fclose(fp);
}

#define P(fmt, ...) fprintf(fp, "%*s" fmt, indent, "", ## __VA_ARGS__);

void genc(Grammar *g, char *usrcode, FILE *fp) {
    fprintf(fp, "#define T_NONE 0\n");
    fprintf(fp, "#define T_EOF 1\n");
    for (int i = 2; i < g->nsyms; i++) {
        Sym *sym = g->syms[i];
        if (sym->type != S_TERM) continue;
        fprintf(fp, "#define %s %i\n", sym->name, i);
    }
    fprintf(fp, "%s", usrcode ? usrcode : "");
    int indent = 0;
    P("static void parse(Parser *p) {\n");
    indent += 4;
        P("Stack s = {0};\n");
        P("int input;\n");
        P("push(&s, (Item){0});\n");
        P("advance(p);\n");
        P("input = p->cur.type;\n");
        fprintf(fp, "main_loop:\n");
        P("switch (top(&s).state) {\n");
        for (int i = 0; i < g->nstates; i++) {
            P("case %i: {\n", i);
            indent += 4;
            P("switch (input) {\n");
            for (int k = 0; k < g->nsyms; k++) {
                Action a = getaction(g, i, k);
                if (a.type == _AT_NONE) continue;
                P("case %i: { // %s\n", k, g->syms[k]->name);
                indent += 4;
                    switch (a.type) {
                    case AT_SHIFT:
                        P("push(&s, (Item){%i, %i, p->cur});\n", a.num, k);
                        P("advance(p);\n");
                        P("input = p->cur.type;\n");
                        P("goto main_loop;\n");
                        break;
                    case AT_REDUCE: {
                        Rule *r = g->rules[a.num];
                        P("s.nitems -= %i;\n", r->nrhs);
                        P("input = %i;\n", r->lhs);
                        if (r->usraction)
                            P("%s;", r->usraction);
                        P("goto main_loop;\n");
                        break;
                    }
                    case AT_GOTO:
                        P("push(&s, (Item){%i, %i});\n", a.num, k);
                        P("input = p->cur.type;\n");
                        P("goto main_loop;\n");
                        break;
                    case AT_ACCEPT:
                        P("goto accept;\n");
                        break;
                    default:
                        printf("*** bad action\n");
                        exit(1);
                    }
                indent -= 4;
                P("}\n");
            }
            P("default: goto error;\n");
            P("}\n");
            indent -= 4;
            P("}\n");
        }
        P("default: goto error;\n");
        P("}\n");
        
fprintf(fp, "accept:\n");
        P("printf(\"successful parse!\\n\");\n");
        P("return;\n");
        fprintf(fp, "error:\n");
        P("printf(\"*** parse error\\n\");\n");
        P("exit(1);\n");
    indent -= 4;
    P("}\n");
}
