

# single-line comments

# if start sym not specified - lhs of the first rule is used
%start S

# declare all your terminals that aren't quoted string literals
# undeclared symbols are assumed to be non-terminals
%term NUM

S -> exp;

exp -> exp "+" NUM
     | exp "-" NUM
     | NUM;

exp -> "(" exp ")";
