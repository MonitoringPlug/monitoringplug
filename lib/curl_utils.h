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
 * Print the libcurl revision.
 */
void print_revision_curl(void);

#endif /* _CURL_UTILS_H_ */

/* vim: set ts=4 sw=4 et syn=c : */
