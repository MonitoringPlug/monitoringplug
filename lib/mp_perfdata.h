/***
 * Monitoring Plugin - mp_perfdata.h
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

#ifndef _MP_PERFDATA_H_
#define _MP_PERFDATA_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mp_args.h"

/** The global perfdata variable. */
extern unsigned int mp_showperfdata;

/** The global perf data string. */
extern char *mp_perfdata;

/**
 * add integer pref data for printing on exit.
 */
void perfdata_int(const char *label, int value, const char *unit,
		  int warn, int crit, int min, int max);

void mp_perfdata_int(const char *label, long int value, const char *unit,
      thresholds *threshold);
void mp_perfdata_int2(const char *label, long int value, const char *unit,
      thresholds *threshold, int have_min, long int min,
      int have_max, long int max);
void mp_perfdata_int3(const char *label, long int value, const char *unit,
      int have_warn, long int warn, int have_crit, long int crit,
      int have_min, long int min, int have_max, long int max);

/**
 * add float perf data for printing on exit.
 */
void perfdata_float(const char *label, float value, const char *unit,
		    float warn, float crit, float min, float max);

void mp_perfdata_float(const char *label, float value, const char *unit,
      thresholds *threshold);
void mp_perfdata_float2(const char *label, float value, const char *unit,
      thresholds *threshold, int have_min, float min,
      int have_max, float max);
void mp_perfdata_float3(const char *label, float value, const char *unit,
      int have_warn, float warn, int have_crit, float crit,
      int have_min, float min, int have_max, float max);

#endif /* _MP_PERFDATA_H_ */
