/***
 * Monitoring Plugin - curl_utils.c
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

#include "mp_common.h"
#include "curl_utils.h"

#include <string.h>
#include <curl/curl.h>

char *mp_curl_user = NULL;
char *mp_curl_pass = NULL;
char *mp_curl_subpath = "";
int mp_curl_ssl = 0;
int mp_curl_insecure = 0;

CURL *mp_curl_init(void) {
    CURL        *curl;
    CURLcode    ret;
    char        *buf;

    /* Global init */
    ret = curl_global_init(CURL_GLOBAL_ALL);
    if (ret != CURLE_OK)
        critical("libcurl initialisation failed!");

    /* Handler init */
    curl = curl_easy_init();
    if (!curl)
        critical("libcurl handler initialisation failed!");

    /* UA setup */
    buf = mp_malloc(64);
    mp_snprintf(buf, 64, "monitoringplug-%s/%s", progname, progvers);
    ret = curl_easy_setopt(curl, CURLOPT_USERAGENT, buf);
    if (ret != CURLE_OK)
        critical("libcurt setting User-Agent failed");
    free(buf);

    /* Debug setup */
    if (mp_verbose > 2) {
        ret = curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        if (ret != CURLE_OK)
            critical("libcurt setting verbose failed");
    }

    /* Setup HTTP basic auth */
    if (mp_curl_user != NULL || mp_curl_pass != NULL) {
        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, (long)CURLAUTH_ANY);
        char *userpwd;
        mp_asprintf(&userpwd, "%s:%s", mp_curl_user, mp_curl_pass);
        curl_easy_setopt(curl, CURLOPT_USERPWD, userpwd);
    }

    /* Set insecure option */
    if (mp_curl_insecure == 1) {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, (long) 0);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, (long) 0);
    }

    return curl;
}

char *mp_curl_url(const char *scheme, const char *hostname, int port, const char *path) {
    char *url;

    mp_asprintf(&url, "%s%s://%s:%d%s%s", scheme, mp_curl_ssl ? "s": "",
            hostname, port, mp_curl_subpath, path);

    return url;
}

long mp_curl_perform(CURL *curl) {
    CURLcode    ret;
    long        code;

    ret = curl_easy_perform(curl);
    if(ret != CURLE_OK)
        critical(curl_easy_strerror(ret));

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);

    return code;
}

size_t mp_curl_recv_blackhole(void *contents, size_t size, size_t nmemb, void *userdata) {
    return size*nmemb;
}

size_t mp_curl_recv_data(void *contents, size_t size, size_t nmemb, void *userdata) {
    size_t data_size = size * nmemb;

    struct mp_curl_data *data = (struct mp_curl_data *)userdata;

    data->data = mp_realloc(data->data, data->size + data_size + 1);

    memcpy(&(data->data[data->size]), contents, data_size);

    data->size += data_size;
    data->data[data->size] = 0;

    return data_size;
}

size_t mp_curl_recv_header(void *contents, size_t size, size_t nmemb, void *userdata) {
    size_t data_size = size * nmemb;
    size_t key_size;
    char *value;

    struct mp_curl_header *header = (struct mp_curl_header *)userdata;

    for (; header->key; header++) {
        key_size = strlen(header->key);
        if (*(header->value) != NULL)
            continue;
        if (strncmp(header->key, contents, key_size) != 0)
            continue;
        if (((char *)contents)[key_size] != ':')
            continue;
        value = mp_malloc(data_size-key_size-2);
        memcpy(value, &(((char *)contents)[key_size+2]), data_size-key_size-2);
        value[strcspn(value, "\n\r")] = '\0';
        *(header->value) = value;
    }

    return data_size;
}

size_t mp_curl_send_data(void *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t read_size = size * nmemb;

    struct mp_curl_data *data = (struct mp_curl_data *)userdata;

    if (read_size > (data->size - data->start))
        read_size = (data->size - data->start);

    memcpy(ptr, &(data->data[data->start]), read_size);

    data->start += read_size;

    return read_size;
}

void getopt_curl(int c) {
    switch ( c ) {
        case 'u':
            mp_curl_user = optarg;
            break;
        case 'p':
            mp_curl_pass = optarg;
            break;
        case MP_LONGOPT_CURL_SUBPATH:
            mp_curl_subpath = optarg;
            break;
    }
}

void print_help_curl_subpath(void) {
    printf("     --subpath=SUBPATH\n");
    printf("      Prepand subpath to url.\n");
}

void print_help_curl_basic_auth(void) {
    printf(" -u, --user=USER\n");
    printf("      HTTP Basic Auth user.\n");
    printf(" -p, --password=PASSWORD\n");
    printf("      HTTP Basic Auth password.\n");
}

void print_help_curl_https(void) {
    printf("     --https\n");
    printf("      Use HTTPS.\n");
    printf("     --insecure\n");
    printf("      Do not validate SSL Certificates.\n");
}

void print_revision_curl(void) {
    printf(" %s\n", curl_version());
}

/* vim: set ts=4 sw=4 et syn=c : */
