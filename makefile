CC = gcc
LEX = flex
YACC = bison

CFLAGS = -Wall

all: calc

calc: calc-fun.o calc.tab.o lex.yy.o
	$(CC) $(CFLAGS) -o calc calc-fun.o calc.tab.o lex.yy.o -lm

calc-fun.o: calc-fun.c calc.tab.h
	$(CC) $(CFLAGS) -c calc-fun.c

calc.tab.o: calc.tab.c
	$(CC) $(CFLAGS) -c calc.tab.c

lex.yy.o: lex.yy.c
	$(CC) $(CFLAGS) -c lex.yy.c

calc.tab.c calc.tab.h: calc.y
	$(YACC) -d calc.y

lex.yy.c: Lex.l
	$(LEX) Lex.l

teste: calc
	./calc < teste.txt

clean:
	rm -f calc lex.yy.c *.o calc.tab.*
