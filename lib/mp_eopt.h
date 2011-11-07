/***
 * Monitoring Plugin - mp_eopt.h
 **
 *
 * Copyright (C) 2011 Marius Rieder <marius.rieder@durchmesser.ch>
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
 * $Id$
 */

#ifndef MP_EOPT_H_
#define MP_EOPT_H_

char **mp_eopt(int *argc, char **orig_argv, char *optarg);

/**
 * Print the help for the eopt option.
 */
void print_help_eopt(void);

/** longopts option for timeout */
#define MP_LONGOPTS_EOPT {"eopt", optional_argument, NULL, (int)MP_LONGOPT_EOPT}

#endif /* MP_EOPT_H_ */
