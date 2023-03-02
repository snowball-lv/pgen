
CFLAGS = -I ../inc -O2 ../src/pgen.c

all: exp

exp: exp.c
	gcc $(CFLAGS) $< -o $@
	./exp
	dot states.dot -T png -o states.png

clean:
	rm -f exp states.dot states.png