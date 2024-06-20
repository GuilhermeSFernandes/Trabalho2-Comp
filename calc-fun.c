#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "calc.h"

/* funcoes em C para TS */
/* funcao hashing */
static unsigned symhash(char *sym)
{
    unsigned int hash = 0;
    unsigned c;

    while (c = *sym++)
        hash = hash * 9 ^ c;

    return hash;
}

struct symbol *lookup(char *sym)
{
    struct symbol *sp = &symtab[symhash(sym) % NHASH];
    int scount = NHASH;

    while (--scount >= 0)
    {
        if (sp->name && !strcasecmp(sp->name, sym))
            return sp;

        if (!sp->name)
        {
            /* nova entrada na TS */
            sp->name = strdup(sym);
            sp->value = 0;
            sp->func = NULL;
            sp->syms = NULL;
            return sp;
        }

        if (++sp >= symtab + NHASH)
            sp = symtab; /* tenta a prox. entrada */
    }
    yyerror("overflow na tab. simbolos\n");
    abort(); /* tabela esta cheia */
}

struct ast *newast(int nodetype, struct ast *l, struct ast *r)
{
    struct ast *a = malloc(sizeof(struct ast));

    if (!a)
    {
        yyerror("sem espaco");
        exit(0);
    }
    a->nodetype = nodetype;
    a->l = l;
    a->r = r;
    return a;
}

struct ast *newnum(double d)
{
    struct numval *a = malloc(sizeof(struct numval));

    if (!a)
    {
        yyerror("sem espaco");
        exit(0);
    }
    a->nodetype = 'K';
    a->number = d;
    return (struct ast *)a;
}

struct ast *newcmp(int cmptype, struct ast *l, struct ast *r)
{
    struct ast *a = malloc(sizeof(struct ast));
    if (!a)
    {
        yyerror("sem espaco");
        exit(0);
    }
    a->nodetype = '0' + cmptype;
    a->l = l;
    a->r = r;
    return a;
}

struct ast *newfunc(int functype, struct ast *l)
{
    struct fncall *a = malloc(sizeof(struct fncall));

    if (!a)
    {
        yyerror("sem espaco");
        exit(0);
    }
    a->nodetype = 'F';
    a->l = l;
    a->functype = functype;
    return (struct ast *)a;
}

struct ast *newcall(struct symbol *s, struct ast *l)
{
    struct ufncall *a = malloc(sizeof(struct ufncall));

    if (!a)
    {
        yyerror("sem espaco");
        exit(0);
    }
    a->nodetype = 'C';
    a->l = l;
    a->s = s;
    return (struct ast *)a;
}

struct ast *newref(struct symbol *s)
{
    struct symref *a = malloc(sizeof(struct symref));

    if (!a)
    {
        yyerror("sem espaco");
        exit(0);
    }
    a->nodetype = 'N';
    a->s = s;
    return (struct ast *)a;
}

struct ast *newasgn(struct symbol *s, struct ast *v)
{
    struct symasgn *a = malloc(sizeof(struct symasgn));

    if (!a)
    {
        yyerror("sem espaco");
        exit(0);
    }
    a->nodetype = '=';
    a->s = s;
    a->v = v;
    return (struct ast *)a;
}

struct ast *newflow(int nodetype, struct ast *cond, struct ast *tl, struct ast *el)
{
    struct flow *a = malloc(sizeof(struct flow));

    if (!a)
    {
        yyerror("sem espaco");
        exit(0);
    }
    a->nodetype = nodetype;
    a->cond = cond;
    a->tl = tl;
    a->el = el;
    return (struct ast *)a;
}

/* libera uma arvore de AST */
void treefree(struct ast *a)
{
    switch (a->nodetype)
    {
        /* duas subarvores */
        case '+':
        case '-':
        case '*':
        case '/':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case 'L':
            treefree(a->r);
            /* uma subarvore */
        case 'C':
        case 'F':
            treefree(a->l);
            /* sem subarvore */
        case 'K':
        case 'N':
            break;

        case '=':
            free(((struct symasgn *)a)->v);
            break;

        /* acima de 3 subarvores */
        case 'I':
        case 'W':
            free(((struct flow *)a)->cond);
            if (((struct flow *)a)->tl) treefree(((struct flow *)a)->tl);
            if (((struct flow *)a)->el) treefree(((struct flow *)a)->el);
            break;

        default:
            printf("erro interno: free bad node %c\n", a->nodetype);
    }

    free(a); /* sempre libera o proprio no */
}

struct symlist *newsymlist(struct symbol *sym, struct symlist *next)
{
    struct symlist *sl = malloc(sizeof(struct symlist));

    if (!sl)
    {
        yyerror("sem espaco");
        exit(0);
    }
    sl->sym = sym;
    sl->next = next;
    return sl;
}

/* libera uma lista de simbolos */
void symlistfree(struct symlist *sl)
{
    struct symlist *nsl;

    while (sl)
    {
        nsl = sl->next;
        free(sl);
        sl = nsl;
    }
}

/* etapa principal >> avaliacao de expressoes, comandos, funcoes, ... */
static double callbuiltin(struct fncall *);
static double calluser(struct ufncall *);

double eval(struct ast *a)
{
    double v;

    if (!a)
    {
        yyerror("erro interno, null eval");
        return 0.0;
    }

    switch (a->nodetype)
    {
        /* constante */
        case 'K':
            v = ((struct numval *)a)->number;
            break;

        /* referencia de nome */
        case 'N':
            v = ((struct symref *)a)->s->value;
            break;

        /* atribuicao */
        case '=':
            v = ((struct symasgn *)a)->s->value = eval(((struct symasgn *)a)->v);
            break;

        /* expressoes */
        case '+':
            v = eval(a->l) + eval(a->r);
            break;
        case '-':
            v = eval(a->l) - eval(a->r);
            break;
        case '*':
            v = eval(a->l) * eval(a->r);
            break;
        case '/':
            v = eval(a->l) / eval(a->r);
            break;

        /* comparacoes */
        case '1':
            v = (eval(a->l) > eval(a->r)) ? 1 : 0;
            break;
        case '2':
            v = (eval(a->l) < eval(a->r)) ? 1 : 0;
            break;
        case '3':
            v = (eval(a->l) != eval(a->r)) ? 1 : 0;
            break;
        case '4':
            v = (eval(a->l) == eval(a->r)) ? 1 : 0;
            break;
        case '5':
            v = (eval(a->l) >= eval(a->r)) ? 1 : 0;
            break;
        case '6':
            v = (eval(a->l) <= eval(a->r)) ? 1 : 0;
            break;

        /* controle de fluxo */
        /* gramatica permite expressoes vazias, entao devem ser verificadas */
        /* if/then/else */
        case 'I':
            if (eval(((struct flow *)a)->cond) != 0)
            {
                /* verifica condicao */
                if (((struct flow *)a)->tl)
                {
                    /* ramo verdadeiro */
                    v = eval(((struct flow *)a)->tl);
                }
                else
                    v = 0.0; /* valor default */
            }
            else
            {
                if (((struct flow *)a)->el)
                {
                    /* ramo falso */
                    v = eval(((struct flow *)a)->el);
                }
                else
                    v = 0.0; /* valor default */
            }
            break;

        /* while/do */
        case 'W':
            v = 0.0; /* valor default */

            if (((struct flow *)a)->tl)
            {
                /* testa se lista de comandos nao eh vazia */
                while (eval(((struct flow *)a)->cond) != 0)
                    /* avalia a condicao */
                    v = eval(((struct flow *)a)->tl);
                /* avalia comandos */
            }
            break; /* valor do ultimo comando eh valor do while/do */

        case 'L':
            eval(a->l);
            v = eval(a->r);
            break;

        case 'F':
            v = callbuiltin((struct fncall *)a);
            break;

        case 'C':
            v = calluser((struct ufncall *)a);
            break;

        default:
            printf("erro interno: bad node %c\n", a->nodetype);
    }
    return v;
}

static double callbuiltin(struct fncall *f)
{
    enum bifs functype = f->functype;
    double v = eval(f->l);

    switch (functype)
    {
        case B_sqrt:
            return sqrt(v);
        case B_exp:
            return exp(v);
        case B_log:
            return log(v);
        case B_print:
            printf("= %4.4g\n", v);
            return v;
        default:
            yyerror("funcao pre-definida desconhecida %d", functype);
            return 0.0;
    }
}

static double calluser(struct ufncall *f)
{
    struct symbol *fn = f->s; /* nome da funcao */
    struct symlist *sl;       /* args da funcao */
    struct ast *args = f->l;  /* lista de args passada */
    double *oldval, *newval;  /* salva valores de args */
    double v = 0.0;
    int nargs;
    int i;

    if (!fn->func)
    {
        yyerror("chamada de funcao indefinida", fn->name);
        return 0;
    }

    /* conta o numero de args */
    sl = fn->syms;
    for (nargs = 0; sl; sl = sl->next)
        nargs++;

    /* prepara valores de args */
    oldval = (double *)malloc(nargs * sizeof(double));
    newval = (double *)malloc(nargs * sizeof(double));

    if (!oldval || !newval)
    {
        yyerror("sem espaco na chamada de funcao");
        return 0.0;
    }

    /* eval args e coloca nos valores novos */
    for (i = 0; i < nargs; i++)
    {
        if (!args)
        {
            yyerror("poucos args na chamada de funcao", fn->name);
            free(oldval);
            free(newval);
            return 0.0;
        }

        if (args->nodetype == 'L')
        { /* if this is a list node */
            newval[i] = eval(args->l);
            args = args->r;
        }
        else
        { /* if it's the end of the list */
            newval[i] = eval(args);
            args = NULL;
        }
    }

    /* salva valores antigos dos args e coloca os novos */
    sl = fn->syms;
    for (i = 0; i < nargs; i++)
    {
        struct symbol *s = sl->sym;

        oldval[i] = s->value;
        s->value = newval[i];
        sl = sl->next;
    }

    free(newval);

    /* avalia a funcao */
    v = eval(fn->func);

    /* restaura valores antigos dos args */
    sl = fn->syms;
    for (i = 0; i < nargs; i++)
    {
        struct symbol *s = sl->sym;

        s->value = oldval[i];
        sl = sl->next;
    }

    free(oldval);
    return v;
}
void yyerror(char *s, ...)
{
    va_list ap;
    va_start(ap, s);

    fprintf(stderr, "%d: erro: ", yylineno);
    vfprintf(stderr, s, ap);
    fprintf(stderr, "\n");
}
int main()
{
    printf("> ");
    return yyparse();
}