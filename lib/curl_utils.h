/***
 * Monitoring Plugin - curl_utils.h
 **
 *
 * Copyright (C) 2012-2014 Marius Rieder <marius.rieder@durchmesser.ch>
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

#ifndef _CURL_UTILS_H_
#define _CURL_UTILS_H_

#include "config.h"
#include <curl/curl.h>

/* The global mysql vars. */
/** Holds the username. */
extern char *mp_curl_user;
/** Holds the password. */
extern char *mp_curl_pass;
/** Holds the subpath string. */
extern char *mp_curl_subpath;
/** Holds the SSL flag. */
extern int mp_curl_ssl;
/** Holds the curl insecure flag. */
extern int mp_curl_insecure;

#define MP_LONGOPT_CURL_SUBPATH        MP_LONGOPT_PRIV0
#define MP_LONGOPT_CURL_SSL            MP_LONGOPT_PRIV1


/** Curl specific short option string. */
#define CURL_OPTSTR "u:p:"
/** Curl specific longopt struct. */
#define CURL_LONGOPTS {"user", required_argument, NULL, (int)'u'}, \
                       {"password", required_argument, NULL, (int)'p'}, \
                       {"subpath", required_argument, NULL, MP_LONGOPT_PRIV0}, \
                       {"ssl", no_argument, (int *)&mp_curl_ssl, 1}, \
                       {"https", no_argument, (int *)&mp_curl_ssl, 1}, \
                       {"insecure", no_argument, (int *)&mp_curl_insecure, 1}


/** Data struct. */
struct mp_curl_data {
   char *data;          /**< Actual data. */
   size_t start;        /**< Read/write offset. */
   size_t size;         /**< Size of data. */
};

/** HTTP header struct */
struct mp_curl_header {
   char *key;           /**< Header name. */
   char **value;        /**< Header value(s). */
};

/**
 * Init libcurl
 * \return Return a pointer to the CURL env.
 */
CURL *mp_curl_init(void);

/**
 * Build a URL considering the curl_utils options.
 * \para[in] scheme URL scheme as string.
 * \para[in] hostname URL hostname.
 * \para[in] port URL port.
 * \para[in] path URL path.
 * \return Return a URL as NULL-terminated string.
 */
char *mp_curl_url(const char *scheme, const char *hostname, int port,
       const char *path);

/**
 * Perform curl request.
 * \para[in] curl Perform setup curl request.
 */
long mp_curl_perform(CURL *curl);

/**
 * libCurl receive blackhole data callback.
 * \para[in] content Receive data buffer.
 * \para[in] size Data unit size.
 * \para[in] nmemb Number of data units ready.
 * \para[in|out] userdata Callback user data pointer.
 * \return Return nmemb times size.
 */
size_t mp_curl_recv_blackhole(void *contents, size_t size,
        size_t nmemb, void *userdata);

/**
 * libCurl receive body data callback.
 * \para[in] content Receive data buffer.
 * \para[in] size Data unit size.
 * \para[in] nmemb Number of data units ready.
 * \para[in|out] userdata Callback user data pointer.
 * \return Return number of bytes consumed.
 */
size_t mp_curl_recv_data(void *contents, size_t size, size_t nmemb, void *userdata);

/**
 * libCurl receive header data callback.
 * \para[in] content Receive data buffer.
 * \para[in] size Data unit size.
 * \para[in] nmemb Number of data units ready.
 * \para[in|out] userdata Callback user data pointer.
 * \return Return number of bytes consumed.
 */
size_t mp_curl_recv_header(void *contents, size_t size, size_t nmemb, void *userdata);

/**
 * libCurl send body data callback.
 * \para[in] content Send data buffer.
 * \para[in] size Data unit size.
 * \para[in] nmemb Number of data units available.
 * \para[in|out] userdata Callback user data pointer.
 * \return Return number of bytes providen.
 */
size_t mp_curl_send_data(void *ptr, size_t size, size_t nmemb, void *userdata);

/**
 * Handle libcurl related command line options.
 * \param[in] c Command line option to handle.
 */
void getopt_curl(int c);

/**
 * Print the help for the subpath command line options.
 */
void print_help_curl_subpath(void);

/**
 * Print the help for the HTTP Basic Auth related command line options.
 */
void print_help_curl_basic_auth(void);

/**
 * Print the help for the HTTPS related command line options.
 */
void print_help_curl_https(void);

/**
 * Print the libcurl revision.
 */
void print_revision_curl(void);

#endif /* _CURL_UTILS_H_ */

/* vim: set ts=4 sw=4 et syn=c : */
