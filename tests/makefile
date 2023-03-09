
CFLAGS = -I ../inc -O2 ../src/pgen.c

all: exp

.PHONY: exp
exp: exp.c
	gcc $(CFLAGS) $< -o $@
	./exp
	dot states.dot -T png -o states.png
	dot table.dot -T png -o table.png

.PHONY: lalr
lalr: lalr.c
	gcc $(CFLAGS) $< -o $@
	./lalr
	dot states.dot -T png -o states.png
	dot table.dot -T png -o table.png

.PHONY: non-lalr
non-lalr: non-lalr.c
	gcc $(CFLAGS) $< -o $@
	./non-lalr
	dot states.dot -T png -o states.png
	dot table.dot -T png -o table.png

.PHONY: expg
expg:
	./../bin/pgen -ds states.dot -dt table.dot -s -r -n -fr -fl exp.g
	dot states.dot -T png -o states.png
	dot table.dot -T png -o table.png

.PHONY: expp
expp:
	./../bin/pgen -ds states.dot -dt table.dot -t slr -g expp.c -s -r expp.g
	dot states.dot -T png -o states.png
	dot table.dot -T png -o table.png
	gcc -O2 expp.c -o expp
	./expp

clean:
	rm -f exp lalr non-lalr *.dot *.png