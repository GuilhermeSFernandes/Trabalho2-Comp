#ifndef CALC_H
#define CALC_H

/**
 * @file calc.h
 * @brief Definições de estruturas e protótipos de funções para o parser da calculadora avançada.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

/* Estruturas de dados */

extern int yylineno;
void yyerror(char *s, ...);

struct symbol {
    char *name;
    double value;
    struct ast *func;
    struct symlist *syms;
};

#define NHASH 9997
extern struct symbol symtab[NHASH];
struct symbol *lookup(char *sym);

struct symlist {
    struct symbol *sym;
    struct symlist *next;
};

struct symlist *newsymlist(struct symbol *sym, struct symlist *next);
void symlistfree(struct symlist *sl);

enum bifs {
    B_sqrt = 1,
    B_exp,
    B_log,
    B_print
};

struct ast {
    int nodetype;
    struct ast *l;
    struct ast *r;
};

struct fncall {
    int nodetype;
    struct ast *l;
    enum bifs functype;
};

struct ufncall {
    int nodetype;
    struct ast *l;
    struct symbol *s;
};

struct flow {
    int nodetype;
    struct ast *cond;
    struct ast *tl;
    struct ast *el;
};

struct numval {
    int nodetype;
    double number;
};

struct symref {
    int nodetype;
    struct symbol *s;
};

struct symasgn {
    int nodetype;
    struct symbol *s;
    struct ast *v;
};

/* Protótipos das funções */
struct ast *newast(int nodetype, struct ast *l, struct ast *r);
struct ast *newcmp(int cmptype, struct ast *l, struct ast *r);
struct ast *newfunc(int functype, struct ast *l);
struct ast *newcall(struct symbol *s, struct ast *l);
struct ast *newref(struct symbol *s);
struct ast *newasgn(struct symbol *s, struct ast *v);
struct ast *newnum(double d);
struct ast *newflow(int nodetype, struct ast *cond, struct ast *tl, struct ast *tr);

void dodef(struct symbol *name, struct symlist *syms, struct ast *stmts);

double eval(struct ast *a);
void treefree(struct ast *a);

#endif /* CALC_H */