
%term NUM
%term ID
%term PLUS "+"
%term MINUS "-"

%start S

S -> exp;

exp -> exp "+" NUM
     | exp PLUS ID
     | exp "-" NUM
     | exp MINUS ID;

exp -> NUM | ID;

%code 

#include <stdio.h>

int main(int argc, char **argv) {
    printf("Hello, World!\n");
    return 0;
}
