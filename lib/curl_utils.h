/***
 * Monitoring Plugin - curl_utils.h
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

#ifndef _CURL_UTILS_H_
#define _CURL_UTILS_H_

#include "config.h"
#include <curl/curl.h>

struct mp_curl_data {
   char *data;
   size_t start;
   size_t size;
};

struct mp_curl_header {
   char *key;
   char **value;
};

/**
 * Init libcurl
 */
CURL *mp_curl_init(void);

/**
 * Perform curl request
 */
long mp_curl_perform(CURL *curl);

/**
 * Blackhole data
 */
size_t mp_curl_recv_blackhole(void *contents, size_t size, size_t nmemb, void *userdata);

/**
 * Receive body data
 */
size_t mp_curl_recv_data(void *contents, size_t size, size_t nmemb, void *userdata);

/**
 * Receive header data
 */
size_t mp_curl_recv_header(void *contents, size_t size, size_t nmemb, void *userdata);

/**
 * Send body data
 */
size_t mp_curl_send_data(void *ptr, size_t size, size_t nmemb, void *userdata);

/**
 * Print the libcurl revision.
 */
void print_revision_curl(void);

#endif /* _CURL_UTILS_H_ */

/* vim: set ts=4 sw=4 et syn=c : */
