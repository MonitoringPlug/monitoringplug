/***
 * Monitoring Plugin - mp_template.c
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "mp_template.h"
#include "mp_common.h"
#include "mp_utils.h"

/* Global Vars */
int mp_template_output_disable = 0;
char *mp_template_output = NULL;
int mp_template_output_len = 0;
int mp_template_output_pos = 0;
struct mp_template_conditional_list *mp_template_conditionals = NULL;

const int memblock = 64;

char *mp_template(FILE *template) {
    // Init out buffer
    mp_template_output = malloc(memblock);
    memset(mp_template_output, 0, memblock);
    mp_template_output_len = memblock;
    mp_template_output_pos = 0;

    mp_template_parse_file(template);

    return mp_template_output;
}

char *mp_template_str(const char *in) {
    // Init out buffer
    mp_template_output = malloc(memblock);
    memset(mp_template_output, 0, memblock);
    mp_template_output_len = memblock;
    mp_template_output_pos = 0;

    mp_template_parse_string(in);

    return mp_template_output;
}

void mp_template_append(const char *s) {
    int len;

    if (!s || mp_template_output_disable)
        return;

    len = strlen(s);

    // Resize out buffer
    if (mp_template_output_len < (mp_template_output_pos + len + 1)) {
        while (mp_template_output_len < (mp_template_output_pos + len + 1))
            mp_template_output_len += memblock;
        mp_template_output_len += memblock;
        mp_template_output = realloc(mp_template_output, mp_template_output_len);
    }

    strncpy(mp_template_output+mp_template_output_pos, s, len+1);
    mp_template_output_pos += len;
}

void mp_template_if(int expr) {
    struct mp_template_conditional_list *cond = NULL;

    if (mp_template_output_disable) {
        mp_template_conditionals->deep += 1;
        return;
    }

    cond = mp_malloc(sizeof(struct mp_template_conditional_list));
    memset(cond, 0, sizeof(struct mp_template_conditional_list));

    cond->type = COND_INT;
    cond->deep = 0;
    cond->value.ival = expr;
    cond->upper = mp_template_conditionals;

    mp_template_conditionals = cond;

    mp_template_output_disable = expr ? 0 : 1;
}

void mp_template_else(void) {
    if (mp_template_conditionals->deep > 0)
        return;

    mp_template_output_disable = mp_template_conditionals->value.ival ? 1 : 0;
}

void mp_template_switch_int(int i) {
    struct mp_template_conditional_list *cond = NULL;

    if (mp_template_output_disable) {
        mp_template_conditionals->deep += 1;
        return;
    }

    cond = mp_malloc(sizeof(struct mp_template_conditional_list));
    memset(cond, 0, sizeof(struct mp_template_conditional_list));

    cond->type = COND_INT;
    cond->deep = 0;
    cond->value.ival = i;
    cond->upper = mp_template_conditionals;

    mp_template_conditionals = cond;

    mp_template_output_disable = 1;

}
void mp_template_case_int(int i){
    if (mp_template_conditionals->deep > 0)
        return;

    mp_template_output_disable = mp_template_conditionals->value.ival == i ? 0 : 1;
}

void mp_template_end() {
    struct mp_template_conditional_list *cond = NULL;

    if (mp_template_conditionals->deep > 0) {
        mp_template_conditionals->deep -= 1;
        return;
    }

    cond = mp_template_conditionals;
    mp_template_conditionals = cond->upper;

    free(cond);

    mp_template_output_disable = 0;
}

char *mp_template_urlencode(const char *in) {
    char *ptr;
    char *out;
    char *optr;
    size_t len = 0;

    // Check input is set.
    if (!in)
        return (char*)in;

    // Calcuclate the string size delta.
    for (ptr = (char *)in; *ptr; ptr++) {
        if (isalnum(*ptr) || *ptr == '-' || *ptr == '_' || *ptr == '.' || *ptr == '~')
            continue;
        len += 2;
    }

    // Check if there are any char to encode.
    if (len == 0)
        return (char *) in;

    len += strlen(in);

    out = mp_malloc(len+1);
    memset(out , 0, len+1);

    // Encode the string
    optr = out;
    for (ptr = (char *)in; *ptr; ptr++) {
        if (isalnum(*ptr) || *ptr == '-' || *ptr == '_' || *ptr == '.' || *ptr == '~') {
            *optr = *ptr;
            optr += 1;
            continue;
        }
        sprintf(optr, "%%%02X", (int)*ptr);
        optr += 3;
    }

    return out;
}

void mp_template_error(char *s) {
    unknown("Error: %s at symbol '%s' on line %d\n", s, yytext, yylineno);
}

/* vim: set ts=4 sw=4 et syn=c : */
