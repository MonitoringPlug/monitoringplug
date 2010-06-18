/***
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
 * prototype for argument parsing function.
 * Needs to be implemented by checks.
 * \param[in] argc number of arguments
 * \param[in] argv list of argumen strings 
 * \return \ref OK or \ref ERROR.
 */
int process_arguments(int argc, char **argv);

/**
 * Defining a range with start and end
 */
typedef struct range_struct {
    double  start;              /**< start of range.    */
    int     start_infinity;     /**< start in -infinity */
    double  end;                /**< end of range       */
    int     end_infinity;       /**< end in infinity    */
    int     alert_on;           /**< invert range       */
} range;

/**
 * Bundle two ranges to a trashold
 */
typedef struct thresholds_struct {
    range   *warning;           /**< warning range      */
    range   *critical;          /**< critical range     */
} thresholds;

/**
 * Helper enum for \ref range_struct.
 * Used for bether readability of \ref range_struct.alert_on
 */
enum {
    OUTSIDE = 0,                /**<  Match if outside of range */
    INSIDE = 1,                 /**<  Match if inside of range  */
};

/**
 * Helper enum for multiplier
 * Used for bether readability in \ref setWarn, \ref setCrit
 */
enum {
    NOEXT = -1,                 /**< No Extension                       */
    BISI = 0,                   /**< Binary/SI Prefix. Use
                                     \ref parse_multiplier_string       */
    TIME = 1,                   /**< Times. Use              
                                     \ref parse_time_multiplier_string  */
};

/**
 * Set the warning range of the given trashold.
 * If the thresholds or the warning range of the thresholds is NULL,
 * the needed memory is allocated.
 * \param[out] threshold Trashold pointer to write in.
 * \param[in] str Range string to parse.
 * \param[in] multiplier Multiplier syntar/function to use.
 * \return \ref OK or \ref ERROR.
 */
int setWarn(thresholds **threshold, char *str, int multiplier);

/**
 * Set the critical range of the given trashold.
 * If the thresholds or the critical range of the thresholds is NULL,
 * the needed memory is allocated.
 * \param[out] threshold Trashold pointer to write in.
 * \param[in] str Range string to parse.
 * \param[in] multiplier Multiplier syntar/function to use.
 * \return \ref OK or \ref ERROR.
 */
int setCrit(thresholds **threshold, char *str, int multiplier);

/**
 * Set the warning range of the given trashold using the time multiplier
 * function.
 * If the thresholds or the warning range of the thresholds is NULL,
 * the needed memory is allocated.
 * \param[out] threshold Trashold pointer to write in.
 * \param[in] str Range string to parse.
 * \return \ref OK or \ref ERROR.
 */
int setWarnTime(thresholds **threshold, char *str);

/**
 * Set the critical range of the given trashold using the time multiplier
 * function.
 * If the thresholds or the critical range of the thresholds is NULL,
 * the needed memory is allocated.
 * \param[out] threshold Trashold pointer to write in.
 * \param[in] str Range string to parse.
 * \return \ref OK or \ref ERROR.
 */
int setCritTime(thresholds **threshold, char *str);

/**
 * Parse a string into a range.
 *
 * Range String Syntax: [@][START:]END
 *
 * - "@" at the beginning makes the range inside out.
 * - START is 0 if not definied.
 * - START can be "~" for inf.
 * - END can be "~" or "" for inf.
 *
 * \param[out] range The range to parse into.
 * \param[in] str String to parse
 * \param[in] multiplier Multiplier syntar/function to use.
 */
int parse_range_string(range *range, char *str, int multiplier);

/**
 * Parse a string into a multipliert for SI and Binary extensions.
 * - k  => 1'000                      K  => 1'024
 * - m  => 1'000'000                  M  => 1'048'578
 * - g  => 1'000'000'000              G  => 1'073'741'824
 * - t  => 1'000'000'000'000          T  => 1'099'511'627'776
 * - p  => 1'000'000'000'000'000      P  => 1'125'899'906'842'624
 * - e  => 1'000'000'000'000'000'000  E  => 1'152'921'504'606'846'976
 * \param[in] str String to parse.
 * \return Multipliert for the string or 1.
 */
double parse_multiplier_string(char *str);

/**
 * Parse a string into a multipliert for time extensions.
 * - s,sec => 1
 * - m,min => 60
 * - h,hr  => 3600
 * - d,day => 86400
 * - w,week => 604800
 * \param[in] str String to parse.
 * \return Multipliert for the string or 1.
 */
double parse_time_multiplier_string(char *str);

/**
 * Check of a value is inside a range.
 * \param[in] value Value to check.
 * \param[in] my_range Range to check.
 * \return 1 if value is in my_range, 0 otherwise.
 */
int check_range(double value, range *my_range);

/**
 * Get the state for a value, trashold combination.
 * \param[in] value Value to check.
 * \param[in] my_thresholds Trashold to check.
 * \return \ref STATE_CRITICAL if value is in critcal range of trashold,
 * \ref STATE_WARNING if value is in warning range of trashold only and
 * \ref STATE_OK otherwise.
 */
int get_status(double value, thresholds *my_thresholds);

/**
 * Prints a human-readable version of the my_threshold to stdout for
 * debuging  purpose.
 * \param[in] threshold_name Name of the trashold.
 * \param[in] my_threshold Trashold to print.
 */
void print_thresholds(const char *threshold_name, thresholds *my_threshold);


/** getopt option for help */
#define MP_ARGS_HELP	{"help", no_argument, NULL, (int)'h'}
/** getopt option for version */
#define MP_ARGS_VERS    {"version", no_argument, NULL, (int)'V'}
/** \def getopt option for verbose */
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
#define  MP_ARGS_CASE_CRIT case 'c': \
            crit = optarg; \
            break; \
/** getopt case for warning time */
#define  MP_ARGS_CASE_WARN_TIME(TRASH) case 'w': \
            setWarnTime(&TRASH, optarg); \
            break;
/** getopt case for critical time */
#define  MP_ARGS_CASE_CRIT_TIME(TRASH) case 'c': \
            setCritTime(&TRASH, optarg); \
            break;

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
