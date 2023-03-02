A library for generating and inspecting `LR` parsing tables.  
Supports `LR(0)`, `SLR` or `LR(1)`, whichever you prefer.

## Usage

There's no `CLI`, sorry!  
Include [inc/pgen/pgen.h](inc/pgen/pgen.h) and [src/pgen.c](src/pgen.c) into your project.  
Do check out [this example](./tests/exp.c) of how to define a simple grammar and dump all generated tables and sets.

## DOT

For better readability, state transitions can be dumped in [DOT](https://en.wikipedia.org/wiki/DOT_(graph_description_language)).

```
S -> exp
exp -> exp + NUM
exp -> NUM
exp -> ( exp )
```

```bash
dot states.dot -T png -o states.png
```

![](states.png)