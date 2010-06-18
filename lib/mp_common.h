/**
 * Monitoring Plugin - mp_common.h
 **
 *
 * Copyright (C) 2010 Marius Rieder <marius.rieder@durchmesser.ch>
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#ifndef _MP_COMMON_H_
#define _MP_COMMON_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mp_args.h"
#include "mp_check.h"

extern const char *progname;
extern const char *progusage;
extern const char *progvers;
extern const char *progauth;
extern const char *progcopy;

extern unsigned int mp_timeout;
extern unsigned int mp_verbose;


/**
 * Default return values for functions
 */
enum {
   OK = 0,              /**  0 - Work as expected */
   ERROR = -1,          /** -1 - A Error occured */
   FALSE = 0,           /**  0 - False */
   TRUE = 1             /**  1 - True */
};

/**
 * Nagios Plugin return value enum.
 * Provide nice names for the nagios plugin return codes.
 */
enum {
   STATE_OK,            /** 0 - OK/UP */
   STATE_WARNING,       /** 1 - Warning */
   STATE_CRITICAL,      /** 2 - Critical/Down */
   STATE_UNKNOWN,       /** 3 - Unknown */
   STATE_DEPENDENT      /** 4 - Dependent */
};

/**
 * prints to the stdout and exit with STATE_OK.
 */
void ok(const char *fmt, ...) __attribute__((__noreturn__));

/**
 * prints to the stdout and exit with STATE_WARNING.
 */
void warning(const char *fmt, ...) __attribute__((__noreturn__));

/**
 * prints to the stdout and exit with STATE_CRITICAL.
 */
void critical(const char *fmt, ...) __attribute__((__noreturn__));

/**
 * prints to the stdout and exit with STATE_UNKNOWN.
 */
void unknown(const char *fmt, ...) __attribute__((__noreturn__));

/**
 * prints to the stdout and exit with STATE_UNKNOWN.
 */
void usage(const char *fmt, ...) __attribute__((__noreturn__));

void print_help (void);
void print_usage (void);
void print_revision (void);
void print_copyright (void);

/* mp timeout functions */
void timeout_alarm_handler (int);

#endif /* _MP_COMMON_H_ */
