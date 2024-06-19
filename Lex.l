%{
#include "calc.tab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
%}

ESPACO              [ \t\n]+
FOR                 "FOR"
AND                 "AND"
OR                  "OR"
IGUAL               "="
PONTO_VIRGULA       ";"
MAIS                "+"
MENOS               "-"
ASTERISCO           "*"
BARRA               "/"
ABRE_PARENTESE      "("
FECHA_PARENTESE     ")"
ABRE_CHAVE          "{"
FECHA_CHAVE         "}"
MENOR               "<"
MAIOR               ">"
IDENT               [a-zA-Z_][a-zA-Z0-9_]*
NUM                 [0-9]+

%%

{ESPACO}                        /* Espaços em branco */
{FOR}                           { return FOR; }
{AND}                           { return AND; }
{OR}                            { return OR; }
{IGUAL}                         { return '='; }
{PONTO_VIRGULA}                 { return ';'; }
{MAIS}                          { return '+'; }
{MENOS}                         { return '-'; }
{ASTERISCO}                     { return '*'; }
{BARRA}                         { return '/'; }
{ABRE_PARENTESE}                { return '('; }
{FECHA_PARENTESE}               { return ')'; }
{ABRE_CHAVE}                    { return '{'; }
{FECHA_CHAVE}                   { return '}'; }
{MENOR}                         { return '<'; }
{MAIOR}                         { return '>'; }
{IDENT} {
    yylval.str = strdup(yytext);
    if (yylval.str == NULL) {
        fprintf(stderr, "Erro: falha ao alocar memória\n");
        exit(EXIT_FAILURE);
    }
    return IDENT;
}
{NUM} {
    yylval.num = atoi(yytext);
    return NUM;
}

.                               { /* Desconhecido: ignorar */ }

%%

int yywrap() {
    return 1;
}
