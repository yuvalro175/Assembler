.DEFAULT_GOAL := all

assembler: main.o preAss.o firstPass.o secondPass.o
	gcc main.o preAss.o firstPass.o secondPass.o -Wall -ansi -pedantic -o assembler -lm

main.o: main.c HEDER.h
	gcc main.c -Wall -ansi -pedantic -c

preAss.o: preAss.c HEDER.h
	gcc preAss.c -Wall -ansi -pedantic -c

firstPass.o: firstPass.c HEDER.h
	gcc firstPass.c -Wall -ansi -pedantic -c

secondPass.o: secondPass.c HEDER.h
	gcc secondPass.c -Wall -ansi -pedantic -c

clean:
	rm -f *.o *.am *.ob *.ent *.ext *.afp *.asp

.PHONY: all clean
all: assembler

