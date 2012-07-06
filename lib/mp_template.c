#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "mp_template.h"

const int memblock = 64;

char *out;
int outLen;
int outPos;
int conditionalDeep = 0;
int conditionalHide = 0;

void mp_template_if(int expr) {

   ++conditionalDeep;

   if (conditionalHide && conditionalDeep >= conditionalHide)
         return;

   if (expr == 0)
      conditionalHide = conditionalDeep;
}

void mp_template_else(void) {
   if (conditionalHide && conditionalDeep != conditionalHide)
      return;

   if (conditionalHide)
      conditionalHide = 0;
   else
      conditionalHide = conditionalDeep;
}

void mp_template_endif(void) {

   --conditionalDeep;

   if (conditionalHide && conditionalDeep >= conditionalHide)
            return;

   conditionalHide = 0;
}

void mp_template_append(const char *s) {
   int len;

   if (conditionalHide && conditionalDeep >= conditionalHide)
      return;
   if (!s)
      return;

   len = strlen(s);

   // Resize out buffer
   if (outLen < (outPos + len + 1)) {
      while (outLen < (outPos + len + 1))
     outLen += memblock;
      outLen += memblock;
      out = realloc(out, outLen);
   }

   strncpy(out+outPos, s, len+1);
   outPos += len;
}

char *mp_template(FILE *template) {
   yyin = template;

   // Init out buffer
   out = malloc(memblock);
   memset(out, 0, memblock);
   outLen = memblock;
   outPos = 0;

   do {
      yyparse();
   } while (!feof(yyin));
   yylex_destroy();

   return out;
}

char *mp_template_str(const char *in) {
   // Init out buffer
   out = malloc(memblock);
   memset(out, 0, memblock);
   outLen = memblock;
   outPos = 0;

   yy_scan_string (in);
   yyparse();
   /* to avoid leakage */
   yylex_destroy();

   return out;
}

void yyerror(char *s) {
   extern char *yytext;
            printf("Error: %s at symbol '%s' on line %d\n", s, yytext, yylineno);
	                exit(1);
}

