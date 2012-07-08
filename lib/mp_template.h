/***
 * Monitoring Plugin - mp_template.h
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
 * $Id$
 */

#ifndef _MP_TEMPLATE_H_
#define _MP_TEMPLATE_H_

#include <stdio.h>

/** conditional linked list struct */
struct mp_template_conditional_list {
    /** Value type */
    int type;
    /** */
    int deep;
    /** Value */
    union {
        char    *sval;
        int     ival;
        float   fval;
    } value;
    /** Upper conditional */
    struct mp_template_conditional_list *upper;
};

enum {
    COND_STRING,
    COND_INT,
    COND_FLOAT,
};

extern int yylineno;            /**< yylineno from mp_template_lex.c */
extern char *yytext;            /**< yytext from mp_template_lex.c */
extern int yylex();             /**< yylex from mp_template_lex.c */
extern int yyparse();           /**< yyparse from mp_template_yacc.c */
/** mp_template_parse_file from mp_template_lex.c */
extern void mp_template_parse_file(FILE *in);
/** mp_template_parse_string from mp_template_lex.c */
extern void mp_template_parse_string(const char *in);

/**
 * Read template from file and return a string.
 * \para[in] template FILE ponter of the template.
 * \return Return the executed template.
 */
char *mp_template(FILE *template);

/**
 * Read template from sting and return a string.
 * \para[in] in Template string to parse.
 * \return Return the executed template.
 */
char *mp_template_str(const char *in);

/**
 * Append to output string if enabled.
 * \para[in] s String to append.
 */
void mp_template_append(const char *s);

/**
 * IF handling function.
 * \para expr Solved expression
 */
void mp_template_if(int expr);

/**
 * ELSE handling function.
 */
void mp_template_else(void);

/**
 * SWITCH with integer handling function.
 * \para[in] i Reference value
 */
void mp_template_switch_int(int i);

/**
 * CASE with integer handling function.
 * \para[in] i Check value
 */
void mp_template_case_int(int i);

/**
 * END handling function.
 */
void mp_template_end();

/**
 * URLEncode a string
 * Replace all non alphanumeric characters exept -_.~ to %XX representation.
 * \para[in] in String to URL Encode
 * \return Return a URL encoded copy of the string or in if no urlencoding is
 * needed.
 */
char *mp_template_urlencode(const char *in);

/**
 * Output parser error messages
 * \para str Error message.
 */
#define yyerror mp_template_error
void mp_template_error(char *s);

#endif /* _MP_TEMPLATE_H_ */

/* vim: set ts=4 sw=4  et syn=c : */
