/***
 * Monitoring Plugin - mp_perfdata.c
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id$
 */

#include "mp_perfdata.h"
#include "mp_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#define T_WARN (threshold->warning ? (threshold->warning->end ? threshold->warning->end : threshold->warning->start) : 0)
#define T_CRIT (threshold->critical ? (threshold->critical->end ? threshold->critical->end : threshold->critical->start) : 0)

unsigned int mp_showperfdata = 0;
char *mp_perfdata=NULL;

void mp_perfdata_int(const char *label, long int value, const char *unit,
      thresholds *threshold) {
   if (threshold)
      mp_perfdata_int3(label, value, unit,
	    (threshold->warning != NULL), (long int)T_WARN,
	    (threshold->critical != NULL), (long int)T_CRIT,
	    0, 0, 0, 0);
   else
      mp_perfdata_int3(label, value, unit,
	    0, 0, 0, 0,
	    0, 0, 0, 0);
}

void mp_perfdata_int2(const char *label, long int value, const char *unit,
      thresholds *threshold, int have_min, long int min,
      int have_max, long int max) {
   if (threshold)
      mp_perfdata_int3(label, value, unit,
	    (threshold->warning != NULL), (long int)T_WARN,
	    (threshold->critical != NULL), (long int)T_CRIT,
	    have_min, min, have_max, max);
   else
      mp_perfdata_int3(label, value, unit,
	    0, 0, 0, 0,
	    have_min, min, have_max, max);
}

void mp_perfdata_int3(const char *label, long int value, const char *unit,
      int have_warn, long int warn, int have_crit, long int crit,
      int have_min, long int min, int have_max, long int max) {
   char *perfString;
   char *valString;
   char *end;

   if (!mp_showperfdata)
      return;

   perfString = mp_malloc(128);
   valString = mp_malloc(16);

   if (strpbrk (label, "'= ")) {
      snprintf(perfString, 64, "'%s'=%ld%s;", label, value, unit);
   } else {
      snprintf(perfString, 64, "%s=%ld%s;", label, value, unit);
   }

   if (have_warn) {
      snprintf(valString, 16, "%ld;", warn);
      strcat(perfString,valString);
   } else {
      strcat(perfString, ";");
   }

   if (have_crit) {
      snprintf(valString, 16, "%ld;", crit);
      strcat(perfString,valString);
   } else {
      strcat(perfString, ";");
   }

   if(have_min) {
      snprintf(valString, 16, "%ld;", min);
      strcat(perfString,valString);
   } else {
      strcat(perfString, ";");
   }

   if(have_max) {
      snprintf(valString, 16, "%ld;", max);
      strcat(perfString,valString);
   }

   for( end = perfString + strlen(perfString) - 1; *(end-1) == ';'; end--) {
      *end = '\0';
   }
   mp_strcat_space(&mp_perfdata, perfString);
   free(perfString);
}

void mp_perfdata_float(const char *label, float value, const char *unit,
      thresholds *threshold) {
   if (threshold) {
      mp_perfdata_float3(label, value, unit,
            (threshold->warning != NULL), (float)T_WARN,
            (threshold->critical != NULL), (float)T_CRIT,
            0, 0, 0, 0);
   } else {
      mp_perfdata_float3(label, value, unit,
            0, 0, 0, 0,
            0, 0, 0, 0);
   }
}

void mp_perfdata_float2(const char *label, float value, const char *unit,
      thresholds *threshold, int have_min, float min,
      int have_max, float max) {
   if (threshold)
      mp_perfdata_float3(label, value, unit,
            (threshold->warning != NULL), (float)T_WARN,
            (threshold->critical != NULL), (float)T_CRIT,
            have_min, min, have_max, max);
   else
      mp_perfdata_float3(label, value, unit,
            0, 0, 0, 0,
            have_min, min, have_max, max);
}

void mp_perfdata_float3(const char *label, float value, const char *unit,
      int have_warn, float warn, int have_crit, float crit,
      int have_min, float min, int have_max, float max) {
   char *perfString;
   char *valString;
   char *end;

   if (!mp_showperfdata)
      return;

   perfString = mp_malloc(128);
   valString = mp_malloc(16);

   if (strpbrk (label, "'= ")) {
      snprintf(perfString, 64, "'%s'=%f%s;", label, value, unit);
   } else {
      snprintf(perfString, 64, "%s=%f%s;", label, value, unit);
   }

   if (have_warn) {
      snprintf(valString, 16, "%f;", warn);
      strcat(perfString, valString);
   } else {
      strcat(perfString, ";");
   }

   if (have_crit) {
      snprintf(valString, 16, "%f;", crit);
      strcat(perfString, valString);
   } else {
      strcat(perfString, ";");
   }

   if(have_min) {
      snprintf(valString, 16, "%f;", min);
      strcat(perfString, valString);
   } else {
      strcat(perfString, ";");
   }

   if(have_max) {
      snprintf(valString, 16, "%f;", max);
      strcat(perfString, valString);
   }

   for( end = perfString + strlen(perfString) - 1; *(end-1) == ';'; end--) {
      *end = '\0';
   }

   mp_strcat_space(&mp_perfdata, perfString);
   free(perfString);
   free(valString);
}

/* vim: set ts=4 sw=4 et syn=c : */
