/***
 * Monitoring Plugin - mp_eopt.h
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

#ifndef MP_EOPT_H_
#define MP_EOPT_H_

/**
 * Inject extended options from a ini file.
 * \para[in|out] argc Number of arguments in argv.
 * \para[in] orig_argv Original args array.
 * \para[in] optarg Arguments to eopt parameter.
 * \return Return the a new extendet argv array.
 */
char **mp_eopt(int *argc, char **orig_argv, char *optarg);

/**
 * Print the help for the eopt option.
 */
void print_help_eopt(void);

/** longopts option for timeout */
#define MP_LONGOPTS_EOPT {"eopt", optional_argument, NULL, (int)MP_LONGOPT_EOPT}

#endif /* MP_EOPT_H_ */

/* vim: set ts=4 sw=4 et syn=c : */
