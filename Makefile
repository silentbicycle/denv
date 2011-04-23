denv: denv.c
	${CC} -Wall -pedantic -o denv denv.c

test: denv
	./denv d foo
