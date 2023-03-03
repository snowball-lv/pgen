
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

clean:
	rm -f exp lalr non-lalr *.dot *.png