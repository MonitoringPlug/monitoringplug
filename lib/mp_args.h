/**
 * Monitoring Plugin - mp_args.h
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

#ifndef _MP_ARGS_H_
#define _MP_ARGS_H_

/**
 * prototype for argument parsing function. Needs to be implemented by checks.
 * \param]in] argc number of arguments
 * \param[in] atgv list of argumen strings 
 */
int process_arguments(int argc, char **argv);

/**
 * Defining a range with start and end
 */
typedef struct range_struct {
    double  start;              /** start of range.     */
    int     start_infinity;     /** start in -infinity  */
    double  end;                /** end of range        */
    int     end_infinity;       /** end in infinity     */
    int     alert_on;           /* invert range         */
} range;

/**
 * Bundle two ranges to a trashold
 */
typedef struct thresholds_struct {
    range   *warning;           /** warning range       */
    range   *critical;          /** critical range      */
} thresholds;

/**
 * helper enum for range
 */
enum {
    OUTSIDE = 0,                /**  Outside - 0        */
    INSIDE = 1                  /**  Inside  - 1        */
};

/**
 * helper enum for multiplier
 */
enum {
    NOEXT = -1,                 /** No Extension        */
    BISI = 0,                   /** Binary/SI Prefix    */
    TIME = 1,                   /** Times               */
};

/**
 *  helper functions
 */
int setWarn(thresholds *my_threshold, char *str, int multiplier);
int setCrit(thresholds *my_threshold, char *str, int multiplier);
int setWarnTime(thresholds *my_threshold, char *str);
int setCritTime(thresholds *my_threshold, char *str);
int parse_range_string(range *range, char *str, int multiplier);

/**
 * k  => 1000                       K  => 1024
 * m  => 1000000                    M  => 1048578
 * g  => 1000000000                 G  => 1073741824
 * t  => 1000000000000              T  => 1099511627776
 * p  => 1000000000000000           P  => 1125899906842624
 * e  => 1000000000000000000        E  => 1152921504606846976
 */
double parse_multiplier_string(char *str);

/**
 * s,sec => 1
 * m,min => 60
 * h,hr  => 3600
 * d,day => 86400
 * w,week => 604800
 */
double parse_time_multiplier_string(char *str);


int check_range(double value, range *my_range);
int get_status(double value, thresholds *my_thresholds);
void print_thresholds(const char *threshold_name, thresholds *my_threshold);


/** getopt option for help */
#define MP_ARGS_HELP	{"help", no_argument, NULL, (int)'h'}
/** getopt option for version */
#define MP_ARGS_VERS    {"version", no_argument, NULL, (int)'V'}
/** getopt option for verbose */
#define MP_ARGS_VERB    {"verbose", no_argument, NULL, (int)'v'}
/** getopt option for timeout */
#define MP_ARGS_TIMEOUT {"timeout", required_argument, NULL, (int)'t'}
/** getopt option for hostname */
#define MP_ARGS_HOST    {"hostname", required_argument, NULL, (int)'H'}
/** getopt option for warning */
#define MP_ARGS_WARN    {"warning", required_argument, NULL, (int)'w'}
/** getopt option for critical */
#define MP_ARGS_CRIT    {"critical", required_argument, NULL, (int)'c'}

/** getopt option end */
#define MP_ARGS_END     {0, 0, NULL, 0}

/** getopt cases for help, version and verbose */
#define MP_ARGS_CASE_DEF case 'h': \
            print_help(); \
            exit(0); \
         case 'V': \
            print_revision(); \
            exit (0); \
         case 'v': \
            mp_verbose++; \
	    break;
/** getopt case for timeout */
#define MP_ARGS_CASE_TIMEOUT case 't': \
            mp_timeout = atoi (optarg); \
            break;
/** getopt case for hostname */
#define  MP_ARGS_CASE_HOST case 'H': \
	    hostname = optarg; \
	    break;
/** getopt case for warning */
#define  MP_ARGS_CASE_WARN case 'w': \
            warn = optarg; \
            break; \
/** getopt case for critical */
#define  MP_ARGS_CASE_CRIT case 'w': \
            crit = optarg; \
            break; \

/** argument helps for help, version and verbose */
#define MP_ARGS_HELP_DEF "\n\
Options:\n\
 -h, --help\n\
      Print detailed help screen.\n\
 -V, --version\n\
      Print version information.\n\
 -v, --verbose\n\
      Show details for command-line debugging.\n"
/** argument help for timeout */
#define MP_ARGS_HELP_TIMEOUT "\
 -t, --timeout=INTEGER\n\
      Seconds before  check timesout.\n"
/** argument help for hostname */
#define MP_ARGS_HELP_HOST "\
 -H, --hostname=ADDRESS\n\
      Host name or IP Address.\n"
/** argument help for warning */
#define MP_ARGS_HELP_WARN_TIME(DEFAULT) "\
 -w, --warning=time[d|h|m|s]\n\
      Return warning if elapsed time exceeds value. Default to "DEFAULT"\n"
/** argument help for critical */
#define MP_ARGS_HELP_CRIT_TIME(DEFAULT) "\
 -c, --critical=time[d|h|m|s]\n\
      Return critical if elapsed time exceeds value. Default to "DEFAULT"\n"

#endif /* _MP_ARGS_H_ */
