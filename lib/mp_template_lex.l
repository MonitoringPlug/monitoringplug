/***
 * Monitoring Plugin - mp_template_lex.l
 **
 *
 * Copyright (C) 2012 Marius Rieder <marius.rieder@durchmesser.ch>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * $Id: $
 */
%{
#include <stdlib.h>
#include "mp_template.h"
#include "mp_template_yacc.h"
%}

%option nounput
%option noinput

%x TT
%x COMMENT

TT_START    \[%
TT_END      "%]"

IDENT       [a-zA-Z_][a-zA-Z0-9_]*

%%

{TT_START}#         { BEGIN(COMMENT); }
\n{TT_START}-       { BEGIN(TT); ++yylineno; }
{TT_START}-?        { BEGIN(TT); }

[^\n\[]+            { yylval.sval = yytext; return TEXT; }
\n                  { ++yylineno; return EOL; }

.                   { yylval.sval = yytext; return TEXT; }

<TT>-{TT_END}\n     { BEGIN(INITIAL); ++yylineno; }
<TT>-?{TT_END}        { BEGIN(INITIAL); }

<TT>[ \t]+          { }

<TT>"GET"           { return GET; }
<TT>"SET"           { return SET; }
<TT>"IF"            { return IF; }
<TT>"UNLESS"        { return UNLESS; }
<TT>"ELSE"          { return ELSE; }
<TT>"SWITCH"        { return SWITCH; }
<TT>"CASE"          { return CASE; }
<TT>"END"           { return END; }
<TT>"=="            { return OP_EQ; }
<TT>">="            { return OP_GE; }
<TT>"<="            { return OP_LE; }

<TT>\"[^\"]*\"      { yylval.sval = yytext+1; yytext[strlen(yytext)-1] = '\0'; return STRING; }
<TT>\'[^\']*\'      { yylval.sval = yytext+1; yytext[strlen(yytext)-1] = '\0'; return STRING; }
<TT>#.*             { /* Eat comment */ }
<TT>[0-9]+          { yylval.ival = (int)strtol(yytext, NULL, 10); return INT; }
<TT>[0-9]+\.[0-9]+  { yylval.fval = strtof(yytext, NULL); return FLOAT; }
<TT>{IDENT}         { yylval.sval = yytext; return LABEL; }
<TT>.               { return yytext[0]; }
<TT>\n              { ++yylineno; }

<COMMENT>-{TT_END}\n    { BEGIN(INITIAL); ++yylineno; }
<COMMENT>-?{TT_END}     { BEGIN(INITIAL); }
<COMMENT>[^\n\%\-]+       { }
<COMMENT>\n             { ++yylineno; }
<COMMENT>.              { }

%%

int yywrap(void) {
    return 1;
}

void mp_template_parse_file(FILE *in) {
    yyin = in;

    do {
        yyparse();
    } while (!feof(yyin));
    yylex_destroy();
}

void mp_template_parse_string(const char *in) {
    yy_scan_string(in);
    yyparse();
    yylex_destroy();
}

/* vim: set ts=4 sw=4 et syn=lex : */
