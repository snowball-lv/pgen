A library for generating and inspecting `LR` parsing tables.  
Supports `LR(0)`, `SLR`, `LR(1)` or `LALR(1)`, whichever you prefer.

## Usage

Use the `CLI` or include [inc/pgen/pgen.h](inc/pgen/pgen.h) and [src/pgen.c](src/pgen.c) into your project.  
Do check out [this example](./tests/exp.c) of how to define a simple grammar in `C` and dump all generated tables and sets.

## CLI

The command line expects you to provide a file defining your grammar. The synax isn't [yacc](https://en.wikipedia.org/wiki/Yacc) compatible, but should be intuitive, sorry!

[Here's](./tests/exp.g) an example of a valid grammar file:
```
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
```
```
Usage: pgen [flags] file.g
    -h          print this message and exit
    -t type     type of parsing table to generate [lr0, slr, lr1, lalr1], default: lalr1
    -s          print symbols
    -r          print rules
    -n          print nullable set
    -fr         print first sets
    -fl         print follow sets
    -ds file    print state transitions to file in DOT
    -dt file    print parsing table to file in DOT
```

## DOT

For better readability, state transitions can be dumped in [DOT](https://en.wikipedia.org/wiki/DOT_(graph_description_language)).  
In `grammar.g`:
```
%term NUM
S -> exp;
exp -> exp "+" NUM;
exp -> NUM;
exp -> "(" exp ")";
```
```
pgen -t slr -ds states.dot -dt table.dot grammar.g
```
```
dot states.dot -T png -o states.png
dot table.dot -T png -o table.png
```
`SLR` states  
![](states.png)

`SLR` parsing table  
![](table.png)
