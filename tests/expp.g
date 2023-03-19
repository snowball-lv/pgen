
%term NUM
%term PLUS "+"
%term MINUS "-"
%term STAR "*"
%term SLASH "/"

%start S

S -> exp { printf("result %i\n", V(0).i); };

exp -> exp "+" num { R.i = V(0).i + V(2).i; printf("%i + %i\n", V(0).i, V(2).i); }
     | exp "-" num { R.i = V(0).i - V(2).i; printf("%i - %i\n", V(0).i, V(2).i); }
     | exp "*" num { R.i = V(0).i * V(2).i; printf("%i * %i\n", V(0).i, V(2).i); }
     | exp "/" num { R.i = V(0).i / V(2).i; printf("%i / %i\n", V(0).i, V(2).i); };

exp -> num { R = V(0); };

num -> NUM { R.i = atoi(TXT(0)); };

%code 

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

typedef struct {
    int type;
    char *start;
    int len;
} Tok;

typedef struct {
    char *src;
    Tok prev;
    Tok cur;
} Parser;

static Tok nexttok(Parser *p) {
    while (isspace(*p->src))
        p->src++;
    char *start = p->src;
    switch (*start) {
    case 0: return (Tok){T_EOF};
    case '+': p->src++; return (Tok){PLUS, start, 1};
    case '-': p->src++; return (Tok){MINUS, start, 1};
    case '*': p->src++; return (Tok){STAR, start, 1};
    case '/': p->src++; return (Tok){SLASH, start, 1};
    }
    if (isdigit(*start)) goto num;
    printf("*** unexpected char %c\n", *start);
    exit(1);
num:
    while (isdigit(*p->src))
        p->src++;
    return (Tok){NUM, start, p->src - start};
}

static void advance(Parser *p) {
    p->prev = p->cur;
    p->cur = nexttok(p);
}

typedef struct {
    int i;
} Value;

typedef struct {
    int state;
    int sym;
    Tok tok;
    Value val;
} Item;

typedef struct {
    Item *items;
    int nitems;
} Stack;

static void push(Stack *s, Item item) {
    s->nitems++;
    s->items = realloc(s->items, s->nitems * sizeof(Item));
    s->items[s->nitems - 1] = item;
}

static Item top(Stack *s) {
    return s->items[s->nitems - 1];
}

static void parse(Parser *p);

static char *SRC = "1 + 2 - 3 + 4 + 5";

#define S(n) (s.items[s.nitems + (n)])
#define LEN(n) (S(n).tok.len)
#define TXT(n) (S(n).tok.start)
#define V(n) (S(n).val)

int main(int argc, char **argv) {
    printf("Hello, World!\n");
    Parser p = {0};
    p.src = SRC;
    parse(&p);
    return 0;
}

