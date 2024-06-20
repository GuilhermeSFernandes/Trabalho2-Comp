bison-calc: Lex.l calc.y calc.h
		bison -d calc.y
		flex -o calc.lex.c Lex.l
		gcc -o $@ calc.tab.c calc.lex.c calc-fun.c -lm
		@echo Parser da Calculadora com Cmds, funcoes, ... estah pronto!