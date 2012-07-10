/***
 * Monitoring Plugin - mp_notify.h
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

#ifndef _MP_NOTIFY_H_
#define _MP_NOTIFY_H_

#include "mp_common.h"
#include "mp_template.h"

/** The global notify message variable. */
const char *mp_notify_msg;
/** The global notify file variable. */
const char *mp_notify_file;

void getopt_notify(int c);

#endif /* _MP_NOTIFY_H_ */

/* vim: set ts=4 sw=4 et syn=c : */
