assemblerProg: assembler.o
	gcc -g -Wall -ansi -pedantic assembler.o -o assemblerProg -lm
assembler.o: assembler.c
	gcc -c -Wall -ansi -pedantic assembler.c -o assembler.o -lm
