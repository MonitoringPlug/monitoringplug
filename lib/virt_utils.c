/***
 * Monitoring Plugin - virt_utils.c
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

#include "mp_common.h"
#include "virt_utils.h"

#include <libvirt/libvirt.h>
#include <libvirt/virterror.h>

#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

char *mp_virt_username = NULL;
char *mp_virt_password = NULL;
char *mp_virt_uri = "qemu:///system";

/* The list of credential types supported by our auth callback */
static int mp_virt_credTypes[] = {
    VIR_CRED_AUTHNAME,
    VIR_CRED_PASSPHRASE
};

/* The auth struct that will be passed to virConnectOpenAuth */
static virConnectAuth mp_virt_auth = {
    mp_virt_credTypes,
    sizeof(mp_virt_credTypes) / sizeof(int),
    virt_authCallback,
    NULL, // cbdata will be initialized in main
};

virConnectPtr  virt_connect() {
    virConnectPtr conn;
    char *uri;

    conn = virConnectOpenAuth(mp_virt_uri, &mp_virt_auth, VIR_CONNECT_RO);

    if (NULL == conn) {
        if (mp_verbose > 0) {
            virt_showError(conn);
        }
        critical("No connection to hypervisor '%s'.", mp_virt_uri);
    }

    uri = virConnectGetURI(conn);
    if (uri == NULL) {
        if (mp_verbose > 0) {
            virt_showError(conn);
        }
        critical("Failed to get URI for hypervisor connection.");
    }
    free(uri);

    if (mp_verbose > 0) {
        printf("Connected to hypervisor at \"%s\"\n", uri);
    }

   return conn;
}

void virt_showError(virConnectPtr conn) {
    int ret;
    virErrorPtr err;

    err = mp_malloc(sizeof(*err));

    ret = virConnCopyLastError(conn, err);

    switch (ret) {
        case 0:
            printf("No error found\n");
            break;
        case -1:
            printf("Parameter error when attempting to get last error\n");
            break;
        default:
            printf("libvirt reported: \"%s\"\n", err->message);
            break;
    } // switch (ret)
    virResetError(err);
    free(err);
}

int virt_authCallback(virConnectCredentialPtr cred, unsigned int ncred, void *cbdata) {
    int i;

    for (i = 0; i < ncred ; ++i) {
        switch (cred[i].type) {
            case VIR_CRED_AUTHNAME:
                cred[i].result = strdup(mp_virt_username);
                if (cred[i].result == NULL)
                    return -1;
                cred[i].resultlen = strlen(cred[i].result);
                break;
            case VIR_CRED_PASSPHRASE:
                cred[i].result = strdup(mp_virt_password);
                if (cred[i].result == NULL)
                    return -1;
                cred[i].resultlen = strlen(cred[i].result);
                break;
            default:
                return -1;
        }
    }

    return 0;
}

void getopt_virt(int c) {
    switch ( c ) {
        case 'C':
            mp_virt_uri = optarg;
            break;
        case 'u':
            mp_virt_username = optarg;
            break;
        case 'p':
            mp_virt_password = optarg;
            break;
    }
}

void print_help_virt(void) {
    printf("\
 -C, --connect=LIBVIRTD-URL\n\
      Connect to the specified URI.\n\
 -u, --username=USERNAME\n\
      Username to connect as.\n\
 -p, --password=PASSWORD\n\
      Password used for authentication.\n");
}

void print_revision_virt(void) {
   unsigned long libVer;
   unsigned int major, minor, release;

   virGetVersion(&libVer, NULL, NULL);

   major = (unsigned int)libVer / 1000000;
   libVer = libVer % 1000000;
   minor = (unsigned int)libVer / 1000;
   release = libVer %  1000;

   printf(" libvirt v%d.%d.%d\n", major, minor, release);
}

/* vim: set ts=4 sw=4 et : */
