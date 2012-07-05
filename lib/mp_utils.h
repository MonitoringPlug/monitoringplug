/***
 * Monitoring Plugin - mp_utils.h
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

#ifndef MP_UTILS_H_
#define MP_UTILS_H_

#include <stdlib.h>
#include <sys/time.h>

/**
 * Call sprintf, call critical if faild.
 */
int mp_sprintf(char *s, const char *format, ...);

/**
 * Call snprintf, call critical if faild.
 */
int mp_snprintf(char *s, size_t n, const char *format, ...);

/**
 * Call asprintf, call critical if faild.
 */
int mp_asprintf(char **retp, const char *format, ...);

/**
 * Call malloc, call critical if faild.
 */
void *mp_malloc(size_t size);

/**
 * Call calloc, call critical if faild.
 */
void *mp_calloc(size_t nmemb, size_t size);

/**
 * Call realloc, call critical if faild.
 */
void *mp_realloc(void *ptr, size_t size);

/**
 * concat strings.
 */
void mp_strcat(char **target, char *source);

/**
 * concat strings with a separating space.
 */
void mp_strcat_space(char **target, char *source);

/**
 * concat strings with a separating comma.
 */
void mp_strcat_comma(char **target, char *source);

/**
 * Compare two strings.
 */
int mp_strcmp(const char *s1, const char *s2);

/**
 * push a string or some comma separated strings to a array.
 * \para[in|out] array Pointer to the array to push to.
 * \para[in] obj Object to push to the array.
 * \para[in\out] num Number of array entries.
 */
void mp_array_push(char ***array, char *obj, int *num);

/**
 * Free a array and its strings.
 * \para[in] array Pointer to the array to free.
 * \para[in] num Number of array entries.
 */
void mp_array_free(char ***array, int *num);

/**
 * Return time in secound since tv.
 */
double mp_time_delta(struct timeval tv);

/**
 * Return a human readable size.
 */
char *mp_human_size(float size);

#endif /* MP_UTILS_H_ */

/* vim: set ts=4 sw=4 et syn=c : */
