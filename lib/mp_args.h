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
 * $Id$
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
int setWarn(thresholds **threshold, const char *str, int multiplier);

/**
 * Set the critical range of the given trashold.
 * If the thresholds or the critical range of the thresholds is NULL,
 * the needed memory is allocated.
 * \param[out] threshold Trashold pointer to write in.
 * \param[in] str Range string to parse.
 * \param[in] multiplier Multiplier syntar/function to use.
 * \return \ref OK or \ref ERROR.
 */
int setCrit(thresholds **threshold, const char *str, int multiplier);

/**
 * Set the warning range of the given trashold using the time multiplier
 * function.
 * If the thresholds or the warning range of the thresholds is NULL,
 * the needed memory is allocated.
 * \param[out] threshold Trashold pointer to write in.
 * \param[in] str Range string to parse.
 * \return \ref OK or \ref ERROR.
 */
int setWarnTime(thresholds **threshold, const char *str);

/**
 * Set the critical range of the given trashold using the time multiplier
 * function.
 * If the thresholds or the critical range of the thresholds is NULL,
 * the needed memory is allocated.
 * \param[out] threshold Trashold pointer to write in.
 * \param[in] str Range string to parse.
 * \return \ref OK or \ref ERROR.
 */
int setCritTime(thresholds **threshold, const char *str);

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
int parse_range_string(range *range, const char *str, int multiplier);

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

/**
 * Prints the help message for the defaults options.
 */
inline void print_help_default(void) __attribute__((always_inline));

/**
 * Prints the help to the timeout option.
 */
inline void print_help_timeout(void) __attribute__((always_inline));

/**
 * Prints the help for the host option
 */
inline void print_help_host(void) __attribute__((always_inline));

/**
 * Prints the help for the port option.
 * \param[in] def The default string.
 */
inline void print_help_port(const char *def) __attribute__((always_inline));

/**
 * Print the help for a warning time option.
 * \param[in] def The default string.
 */
inline void print_help_warn_time(const char *def) __attribute__((always_inline));

/**
 * Print the help for a critical time option.
 * \param[in] def The default string.
 */
inline void print_help_crit_time(const char *def) __attribute__((always_inline));

/**
 * Parse the options for help, version, and verbose.
 * \param[in] c option to test
 */
inline void getopt_default(int c) __attribute__((always_inline));

/**
 * Parse the option for timeout.
 * \param[in] c option to test
 * \param[in] optarg option argument
 */
inline void getopt_timeout(int c, const char *optarg) __attribute__((always_inline));

/**
 * Parse the option for host.
 * \param[in] c option to test
 * \param[in] optarg option argument
 * \param[out] hostname hostname variable to set
 */
inline void getopt_host(int c, const char *optarg, const char **hostname) __attribute__((always_inline));

/**
 * Parse the option for host. Allow only IPs.
 * \param[in] c option to test
 * \param[in] optarg option argument
 * \param[out] hostname hostname variable to set
 */
inline void getopt_host_ip(int c, const char *optarg, const char **hostname) __attribute__((always_inline));

/**
 * Parse the option for port.
 * \param[in] c option to test
 * \param[in] optarg option argument
 * \param[out] port port variable to set
 */
inline void getopt_port(int c, const char *optarg, int *port) __attribute__((always_inline));

inline void getopt_wc(int c, const char *optarg, thresholds **threshold) __attribute__((always_inline));
inline void getopt_wc_time(int c, const char *optarg, thresholds **threshold) __attribute__((always_inline));
inline void getopt_46(int c, int *ipv4, int *ipv6) __attribute__((always_inline));

/** optstring for help, version, verbose */
#define MP_OPTSTR_DEFAULT   "hVv"
/** longopts option for help, version, verbose */
#define MP_LONGOPTS_DEFAULT {"help", no_argument, NULL, (int)'h'}, \
                            {"version", no_argument, NULL, (int)'V'}, \
                            {"verbose", no_argument, NULL, (int)'v'}

/** optstring for timeout */
#define MP_OPTSTR_TIMEOUT   "t:"
/** longopts option for timeout */
#define MP_LONGOPTS_TIMEOUT {"timeout", required_argument, NULL, (int)'t'}

/** optstring for hostname */
#define MP_OPTSTR_HOST      "H:"
/** longopts option for hostname */
#define MP_LONGOPTS_HOST    {"hostname", required_argument, NULL, (int)'H'}

/** optstring for port */
#define MP_OPTSTR_PORT      "P:"
/** longopts option for port */
#define MP_LONGOPTS_PORT    {"port", required_argument, NULL, (int)'P'}

/** optstring for ipv4 and ipv6 */
#define MP_OPTSTR_IPV       "46"
/** longopts option for ipv4 and ipv6 */
#define MP_LONGOPTS_IPV     {"ipv4", no_argument, NULL, (int)'4'}, \
                            {"ipv6", no_argument, NULL, (int)'6'}

/** optstring for warn and crit */
#define MP_OPTSTR_WC        "w:c:"
/** longopts option for warn and crit */
#define MP_LONGOPTS_WC      {"warning", required_argument, NULL, (int)'w'}, \
                            {"critical", required_argument, NULL, (int)'c'}

/** longopts final */
#define MP_LONGOPTS_END     {0, 0, NULL, 0}
      
#endif /* _MP_ARGS_H_ */
