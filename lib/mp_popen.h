/***
 * Monitoring Plugin - mp_popen.h
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

#ifndef MP_POPEN_H_
#define MP_POPEN_H_

#include <stdio.h>

/**
 * MP wrapper arount popen
 * \param[in] command Command to run
 * \return return the filehandle.
 */
FILE *mp_popen(char *command[]);

/**
 * MP wrapper arount pclose
 * \param[in] file popen filehandle to close
 * \return return returncode of called command.
 */
int mp_pclose(FILE * file);

#endif /* MP_POPEN_H_ */

/* vim: set ts=4 sw=4 et syn=c : */
