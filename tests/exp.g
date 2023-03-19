

# single-line comments

# if start sym not specified - lhs of the first rule is used
%start S

# declare all your terminals
# undeclared symbols are assumed to be non-terminals
%term NUM
%term PLUS

# terminals can have string aliases (you still have to tokenize them yourself)
%term MINUS "-"
%term LPAREN "("
%term RPAREN ")"

S -> exp;

exp -> exp PLUS NUM
     | exp "-" NUM
     | NUM;

exp -> "(" exp ")";
