
%term NUM
%term PLUS "+"
%term MINUS "-"
%term STAR "*"
%term SLASH "/"
%term LPAREN "("
%term RPAREN ")"

%union {
    int i;
}

%start S

S -> E          { printf("result is %i\n", V(0).i); };

E -> E "+" T    { R.i = V(0).i + V(2).i; }
   | E "-" T    { R.i = V(0).i - V(2).i; }
   | T          { R = V(0); };

T -> T "*" F    { R.i = V(0).i * V(2).i; }
   | T "/" F    { R.i = V(0).i / V(2).i; }
   | F          { R = V(0); };

F -> NUM        { R.i = atoi(TXT(0)); };
F -> "(" E ")"  { R = V(1); };

%code 

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
    case '(': p->src++; return (Tok){LPAREN, start, 1};
    case ')': p->src++; return (Tok){RPAREN, start, 1};
    }
    if (isdigit(*start)) goto num;
    printf("*** unexpected char %c\n", *start);
    exit(1);
num:
    while (isdigit(*p->src))
        p->src++;
    return (Tok){NUM, start, p->src - start};
}

static char *SRC = "1 + (2 + 3) * 4";

int main(int argc, char **argv) {
    Parser p = {0};
    p.src = SRC;
    parse(&p);
    return 0;
}
