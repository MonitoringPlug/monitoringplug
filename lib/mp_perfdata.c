/**
 * Monitoring Plugin - mp_perfdata.c
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

#include "mp_perfdata.h"
#include "mp_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

unsigned int mp_showperfdata = 0;
char *mp_perfdata=NULL;

void perfdata_int(const char *label, int value, const char *unit,
                  int warn, int crit, int min, int max) {
   char *tmp;

   if (!mp_showperfdata)
       return;

   tmp=mp_malloc(64);
   sprintf(tmp,"'%s'=%d%s;%d;%d;%d;%d", label, value, unit, warn, crit, min, max);

   if (mp_perfdata != NULL) {
      mp_perfdata = mp_realloc(mp_perfdata, strlen(mp_perfdata) + strlen(tmp) + 2);
      strncat(mp_perfdata, " ", 1);
   } else {
      mp_perfdata = mp_malloc(strlen(tmp) + 1);
      mp_perfdata[0] = '\0';
   }
   strncat(mp_perfdata, tmp, strlen(tmp));

   free(tmp);
}

void mp_perfdata_int(const char *label, long int value, const char *unit,
      thresholds *threshold) {
   if (threshold)
      mp_perfdata_int3(label, value, unit,
	    (threshold->warning != NULL), (long int)threshold->warning->end,
	    (threshold->critical != NULL), (long int)threshold->critical->end,
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
	    (threshold->warning != NULL), (long int)threshold->warning->end,
	    (threshold->critical != NULL), (long int)threshold->critical->end,
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
   char *end;

   if (!mp_showperfdata)
      return;

   perfString = mp_malloc(128);

   if (strpbrk (label, "'= ")) {
      sprintf(perfString, "'%s'=%ld%s;", label, value, unit);
   } else {
      sprintf(perfString, "%s=%ld%s;", label, value, unit);
   }

   if (have_warn)
      sprintf(perfString, "%s%ld;", perfString, warn);
   else
      strcat(perfString, ";");

   if (have_crit)
      sprintf(perfString, "%s%ld;", perfString, crit);
   else
      strcat(perfString, ";");

   if(have_min)
      sprintf(perfString, "%s%ld;", perfString, min);
   else
      strcat(perfString, ";");

   if(have_max)
      sprintf(perfString, "%s%ld;", perfString, max);

   for( end = perfString + strlen(perfString) - 1; *(end-1) == ';'; end--) {
      *end = '\0';
   }
   mp_strcat_space(&mp_perfdata, perfString);
   free(perfString);
}

void perfdata_float(const char *label, float value, const char *unit,
                    float warn, float crit, float  min, float max) {
   char *tmp;

   if (!mp_showperfdata)
       return;

   tmp=mp_malloc(128);
   sprintf(tmp,"'%s'=%0.4f%s;%0.2f;%0.2f;%0.2f;%0.2f", label, value, unit, warn, crit, min, max);

   if (mp_perfdata != NULL) {
      mp_perfdata = mp_realloc(mp_perfdata, strlen(mp_perfdata) + strlen(tmp) + 2);
      strncat(mp_perfdata, " ", 1);
      strncat(mp_perfdata, tmp, strlen(tmp));
   } else {
      mp_perfdata = strdup(tmp);
   }

   free(tmp);
}

void mp_perfdata_float(const char *label, float value, const char *unit,
      thresholds *threshold) {
   if (threshold)
      mp_perfdata_float3(label, value, unit,
            (threshold->warning != NULL), (float)threshold->warning->end,
            (threshold->critical != NULL), (float)threshold->critical->end,
            0, 0, 0, 0);
   else
      mp_perfdata_float3(label, value, unit,
            0, 0, 0, 0,
            0, 0, 0, 0);
}

void mp_perfdata_float2(const char *label, float value, const char *unit,
      thresholds *threshold, int have_min, float min,
      int have_max, float max) {
   if (threshold)
      mp_perfdata_float3(label, value, unit,
            (threshold->warning != NULL), (float)threshold->warning->end,
            (threshold->critical != NULL), (float)threshold->critical->end,
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
   char *end;

   if (!mp_showperfdata)
      return;

   perfString = mp_malloc(128);

   if (strpbrk (label, "'= ")) {
      sprintf(perfString, "'%s'=%f%s;", label, value, unit);
   } else {
      sprintf(perfString, "%s=%f%s;", label, value, unit);
   }

   if (have_warn)
      sprintf(perfString, "%s%f;", perfString, warn);
   else
      strcat(perfString, ";");

   if (have_crit)
      sprintf(perfString, "%s%f;", perfString, crit);
   else
      strcat(perfString, ";");

   if(have_min)
      sprintf(perfString, "%s%f;", perfString, min);
   else
      strcat(perfString, ";");

   if(have_max)
      sprintf(perfString, "%s%f;", perfString, max);

   for( end = perfString + strlen(perfString) - 1; *(end-1) == ';'; end--) {
      *end = '\0';
   }

   mp_strcat_space(&mp_perfdata, perfString);
   free(perfString);
}
