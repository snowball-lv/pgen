
%term NUM
%term ID
%term PLUS

%start S

S -> exp;

exp -> exp "+" NUM
     | exp PLUS ID;

exp -> NUM | ID;

%code 

#include <stdio.h>

int main(int argc, char **argv) {
    printf("Hello, World!\n");
    return 0;
}
