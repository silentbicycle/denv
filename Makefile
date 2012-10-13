all: denv test

denv: denv.c
	${CC} -Wall -pedantic -o denv denv.c

test: denv
	./test_denv

clean:
	rm -f denv *.o *.core
