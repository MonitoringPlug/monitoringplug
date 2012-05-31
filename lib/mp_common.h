/***
 * Monitoring Plugin - mp_common.h
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

#ifndef _MP_COMMON_H_
#define _MP_COMMON_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mp_args.h"
#include "mp_check.h"
#include "mp_perfdata.h"
#include "mp_popen.h"
#include "mp_utils.h"
#include "mp_longopt.h"

/** Pointer to the program name. Each plugin must define this. */
extern const char *progname;
/** Pointer to the program usage string. Each plugin must define this. */
extern const char *progusage;
/** Pointer to the program version string. Each plugin must define this. */
extern const char *progvers;
/** Pointer to the program author string. Each plugin must define this. */
extern const char *progauth;
/** Pointer to the program copyright year. Each plugin must define this. */
extern const char *progcopy;


/** The global timeout variable. */
extern unsigned int mp_timeout;
/** The global verbose variable. */
extern unsigned int mp_verbose;

/**
 * Default return values for functions
 */
enum {
   OK = 0,              /**<  0 - Work as expected */
   ERROR = -1,          /**< -1 - A Error occured */
};

#ifndef FALSE
#   define FALSE    (0)
#endif
#ifndef TRUE
#   define TRUE (1)
#endif


/**
 * Nagios Plugin return value enum.
 * Provide nice names for the nagios plugin return codes.
 */
enum {
   STATE_OK,            /**< 0 - OK/UP */
   STATE_WARNING,       /**< 1 - Warning */
   STATE_CRITICAL,      /**< 2 - Critical/Down */
   STATE_UNKNOWN,       /**< 3 - Unknown */
   STATE_DEPENDENT      /**< 4 - Dependent */
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

/**
  * Print the program help.
  * Needs to be implemented by checks.
  */
void print_help(void);

/**
  * Print the program usage.
  */
void print_usage(void);

/**
  * Print the program version information.
  */
void print_revision(void);


/**
  * Print the program copyright information.
  */
void print_copyright(void);

/**
 * Default timeout handler.
 * Print a message and exit with STATE_CRITICAL.
 * \param[in] signo Signal number of the trigering signal.
 */
void timeout_alarm_handler(int signo);

/**
 * Abort if none-root runs root-only plugin.
 */
void mp_noneroot_die(void);

#endif /* _MP_COMMON_H_ */
