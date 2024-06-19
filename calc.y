%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "calc.tab.h"

#define MAX_VARS 100

typedef struct {
    char* name;
    int value;
} Variable;

Variable symbolTable[MAX_VARS];
int varCount = 0;

extern int yylex();
extern int yyparse();
extern FILE *yyin;

void yyerror(const char *s);

int getVarIndex(const char* name) {
    for (int i = 0; i < varCount; ++i) {
        if (strcmp(symbolTable[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int getVarValue(const char* name) {
    int index = getVarIndex(name);
    if (index != -1) {
        return symbolTable[index].value;
    } else {
        fprintf(stderr, "Erro: Variável %s não declarada.\n", name);
        exit(EXIT_FAILURE);
    }
}

void setVarValue(const char* name, int value) {
    int index = getVarIndex(name);
    if (index != -1) {
        symbolTable[index].value = value;
    } else {
        symbolTable[varCount].name = strdup(name);
        symbolTable[varCount].value = value;
        varCount++;
    }
}
%}

%union {
    int num;
    char* str;
}

%token <num> NUM
%token <str> IDENT 
%token FOR AND OR

%type <num> expressao termo fator expressao_logica
%type <str> atribuicao

%%

programa: comando_list
        { printf("Programa reconhecido.\n"); }
        ;

comando_list: comando
            | comando_list comando
            ;

comando: atribuicao
       | for_comando
       ;

atribuicao: IDENT '=' expressao ';'
          { setVarValue($1, $3); printf("Atribuição: %s = %d;\n", $1, $3); }
          ;

for_comando: FOR '(' atribuicao ';' expressao_logica ';' atribuicao ')' '(' comando_list ')'
           {
               // Inicialização
               setVarValue($3, getVarValue($5));

               // Execução do loop
               while (getVarValue($3) < getVarValue($7)) {
                   // Executa os comandos dentro do bloco FOR
                   yyparse();  // Executa os comandos no corpo do FOR

                   // Incremento
                   int currentValue = getVarValue($3) + 1;
                   setVarValue($3, currentValue);
               }
               printf("Comando FOR executado.\n");
           }
           ;

expressao_logica: expressao '<' expressao
                { $$ = $1 < $3; }
                | expressao '>' expressao
                { $$ = $1 > $3; }
                | '(' expressao_logica ')'
                { $$ = $2; }
                ;

expressao: termo
         { $$ = $1; }
         | expressao '+' termo
         { $$ = $1 + $3; }
         | expressao '-' termo
         { $$ = $1 - $3; }
         ;

termo: fator
      { $$ = $1; }
      | termo '*' fator
      { $$ = $1 * $3; }
      | termo '/' fator
      { $$ = $1 / $3; }
      ;

fator: NUM
      { $$ = $1; }
      | IDENT
      { $$ = getVarValue($1); }
      | '(' expressao ')'
      { $$ = $2; }
      ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Erro: %s\n", s);
}

int main(int argc, char **argv) {
    if (argc > 1) {
        FILE *file = fopen(argv[1], "r");
        if (!file) {
            fprintf(stderr, "Não foi possível abrir %s\n", argv[1]);
            return 1;
        }
        yyin = file;
    }
    yyparse();
    for (int i = 0; i < varCount; ++i) {
        printf("%s = %d\n", symbolTable[i].name, symbolTable[i].value);
    }
    return 0;
}
