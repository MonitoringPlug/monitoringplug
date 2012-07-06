/***
 * Monitoring Plugin - mp_template_yacc.y
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
#include <stdlib.h> // getenv
#include <string.h> // strcmp

#include "mp_utils.h"
#include "mp_template.h"
%}

%token TAG_START TAG_END

// Directives
%token GET SET

%token IF UNLESS ELSE END
%token OP_EQ OP_GE OP_LE
%token EOL

%union {
   char     *sval;
   int      ival;
   float    fval;
}

%token <sval> SPACE
%token <sval> LABEL
%token <sval> TEXT
%token <fval> FLOAT
%token <ival> INT
%token <sval> STRING

%type <ival> bexpr
%type <sval> sexpr
%type <ival> iexpr
%type <fval> fexpr

%left '<' '>' OP_GE OP_EQ OP_LE
%left '+' '-'
%left '*' '/'

%start	template

%%

template: blocks
	;

blocks: blocks block
      | block
      | /* empty */
      ;

block: TEXT		{ mp_template_append($1); }
     | EOL		{ mp_template_append("\n"); }
     | statement
     ;

statement: conditionals
         | get
         | set
         ;

conditionals: conditionals_start blocks conditionals_else blocks conditionals_end
	    | conditionals_start blocks conditionals_end
	    ;
conditionals_start: IF bexpr { mp_template_if($2); }
                  | UNLESS bexpr { mp_template_if(!$2); }
		  ;
conditionals_else: ELSE		{ mp_template_else(); }
		 ;
conditionals_end: END		{ mp_template_endif(); }
		;

get: GET sexpr              { mp_template_append($2); }
   | sexpr                  { mp_template_append($1); }

set: SET LABEL '=' sexpr     { }
   ;

bexpr: fexpr OP_EQ fexpr	{ $$ = ($1 == $3); }
     | fexpr OP_GE fexpr	{ $$ = ($1 >= $3); }
     | fexpr OP_LE fexpr	{ $$ = ($1 <= $3); }
     | fexpr '>' fexpr		{ $$ = ($1 > $3); }
     | fexpr '<' fexpr		{ $$ = ($1 < $3); }
     | '!' bexpr			{ $$ = $2 ? 0 : 1; }
     | '(' bexpr ')'		{ $$ = $2; }
     | sexpr OP_EQ sexpr	{ $$ = (strcmp($1, $3) == 0); }
     | sexpr			    { $$ = $1 ? strlen($1) > 0 : 0; }
     | iexpr                { $$ = $1 != 0; }
     | fexpr                { $$ = $1 != 0; }
     ;

iexpr: iexpr '+' iexpr  { $$ = $1 + $3; }
     | iexpr '-' iexpr  { $$ = $1 - $3; }
     | iexpr '/' iexpr  { $$ = $1 / $3; }
     | iexpr '*' iexpr  { $$ = $1 * $3; }
     | '(' iexpr ')'    { $$ = $2; }
     | INT
     ;

fexpr: fexpr '+' fexpr  { $$ = $1 + $3; }
     | fexpr '-' fexpr  { $$ = $1 - $3; }
     | fexpr '/' fexpr  { $$ = $1 / $3; }
     | fexpr '*' fexpr  { $$ = $1 * $3; }
     | '(' fexpr ')'    { $$ = $2; }
     | FLOAT            { $$ = $1; }
     | INT              { $$ = (float)$1; }
     ;

sexpr: STRING			{ }
     | LABEL			{ $$ = getenv($1); }
     | iexpr            { mp_asprintf(&$$, "%d", $1);  }
     | fexpr            { mp_asprintf(&$$, "%f", $1);  }
     ;


%%

/* vim: set ts=4 sw=4 et syn=yacc : */
