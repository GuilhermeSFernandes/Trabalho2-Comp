# Nome do executável
EXEC = parser

# Arquivos fonte
LEXER_SRC = Lex.l
PARSER_SRC = calc.y

# Arquivos gerados
LEXER_C = lex.yy.c
PARSER_C = calc.tab.c
PARSER_HEADER = calc.tab.h

# Compilador e flags
CC = gcc
CFLAGS = -Wall -lfl

# Regras
all: $(EXEC)

$(EXEC): $(LEXER_C) $(PARSER_C)
	$(CC) -o $(EXEC) $(PARSER_C) $(LEXER_C) $(CFLAGS)

$(LEXER_C): $(LEXER_SRC) $(PARSER_HEADER)
	flex $(LEXER_SRC)

$(PARSER_C) $(PARSER_HEADER): $(PARSER_SRC)
	bison -d $(PARSER_SRC)

clean:
	rm -f $(EXEC) $(LEXER_C) $(PARSER_C) $(PARSER_HEADER)

# Regra para executar o analisador com um arquivo de teste
run: $(EXEC)
	./$(EXEC) teste1.bas

.PHONY: all clean run
