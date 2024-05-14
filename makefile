.PHONY: clear build docs run

express: express.c
	gcc -lpthread $< -o $@

build: express

run: express
	./express

docs: Doxyfile
	doxygen

clear:
	${RM} express
	${RM} -r html latex
