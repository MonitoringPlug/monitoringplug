
#ifndef _MP_TEMPLATE_H_
#define _MP_TEMPLATE_H_

#include <stdio.h>

extern int yylex();
extern FILE *yyin;
extern int yylineno; // defined and maintained in lex.c

void yyerror(char *);
void mp_template_append(const char *s);
void mp_template_if(int expr);
void mp_template_else(void);
void mp_template_endif(void);
extern char *mp_template(FILE *template);
extern char *mp_template_str(const char *in);
#endif // _MP_TEMPLATE_H_
