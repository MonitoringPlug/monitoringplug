/***
 * monitoringplug - check_libvirt_domain.c
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

const char *progname  = "check_libvirt_domain";
const char *progvers  = "0.1";
const char *progcopy  = "2011";
const char *progauth = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "--domain DOMAIN [--connect URI]";

/* MP Includes */
#include "mp_common.h"
#include "virt_utils.h"
/* Default Includes */
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef HAVE_EXPAT
#include <expat.h>
#endif

/* Global Vars */
char *domainname;

#ifdef HAVE_EXPAT
int isDevices = 0;
int isInterface = 0;
char **interface;
int interfaces = 0;

void interface_startElement(void *userData, const char *name, const char **atts);
void interface_stopElement(void *userData, const char *name);

void interface_startElement(void *userData, const char *name, const char **atts) {
    const char **k, **v;
    if (!isDevices) {
        if (strcmp(name, "devices") == 0) {
            isDevices = 1;
        } else {
            return;
        }
    }
    if (!isInterface) {
        if (strcmp(name, "interface") == 0) {
            isInterface = 1;
        } else {
            return;
        }
    }
    if (strcmp(name, "target") == 0) {
        for(k = atts, v = atts+1; *k != NULL; k+=2, v+=2) {
            if (strcmp(*k, "dev") == 0)
                mp_array_push(&interface, strdup(*v), &interfaces);
        }
    }
}

void interface_stopElement(void *userData, const char *name) {
    if (isDevices) {
        if (isInterface) {
            if (strcmp(name, "interface") == 0) {
                isInterface = 0;
            }
        } else if (strcmp(name, "devices") == 0) {
            isDevices = 0;
        }
    }
}
#endif

int main (int argc, char **argv) {
    /* Local Vars */
    virConnectPtr   conn;
    virDomainPtr    dom = NULL;
    virDomainInfo   info;
    virDomainInterfaceStatsStruct intStats;
    int             ret;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap faild!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments faild!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    // PLUGIN CODE
    conn = virt_connect();

    dom = virDomainLookupByName(conn, domainname);
    if (dom == NULL) {
        if (mp_verbose > 0) {
            virt_showError(conn);
        }
        virConnectClose(conn);
        critical("Domain '%s' not found.");
    }

    /* Get the information */
    ret = virDomainGetInfo(dom, &info);
    if (ret < 0) {
        virDomainFree(dom);
        virConnectClose(conn);
        critical("Failed to get information for Domain %s", domainname);
    }

#ifdef HAVE_EXPAT
    XML_Parser  parser;
    char        *buf;
    int         i;

    parser = XML_ParserCreate(NULL);
    XML_SetElementHandler(parser, interface_startElement,
            interface_stopElement);

    buf = virDomainGetXMLDesc(dom, 0);

    if (!XML_Parse(parser, buf, strlen(buf), 1)) {
        unknown("%s at line %d\n",
                XML_ErrorString(XML_GetErrorCode(parser)),
                (int) XML_GetCurrentLineNumber(parser));
    }
    XML_ParserFree(parser);
    free(buf);

    buf = mp_malloc(128);

    for (i=0; i < interfaces; i++) {

        ret = virDomainInterfaceStats(dom, interface[i], &intStats, sizeof(intStats));
        if (ret == 0) {
            mp_snprintf(buf, 127, "%s_rx", interface[i]);
            perfdata_int(buf, (int)intStats.rx_bytes/1024, "kB", 0,0,0,0);
            mp_snprintf(buf, 127, "%s_tx", interface[i]);
            perfdata_int(buf, (int)intStats.tx_bytes/1024, "kB", 0,0,0,0);
        }
    }

    free(buf);
#endif

    virDomainFree(dom);
    virConnectClose(conn);

    perfdata_int("memory", info.memory, "kB", 0,0,0, info.maxMem);
    perfdata_float("cpuTime", (float)info.cpuTime/1000000000, "s", 0, 0, 0, 0);

    /* Output and return */
    ok(domainname);
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
            MP_LONGOPTS_DEFAULT,
            VIRT_LONGOPTS,
            {"domain", required_argument, NULL, (int)'D'},
            MP_LONGOPTS_TIMEOUT,
            MP_LONGOPTS_END
    };

    if (argc < 3) {
       print_help();
       exit(STATE_OK);
    }


    while (1) {
        c = getopt_long (argc, argv, MP_OPTSTR_DEFAULT"D:t:"VIRT_OPTSTR, longopts, &option);

        if (c == -1 || c == EOF)
            break;

        getopt_virt(c);

        switch (c) {
            /* Default opts */
            MP_GETOPTS_DEFAULT
            /* Domain opt */
            case 'D':
                domainname = optarg;
            /* Timeout opt */
            case 't':
                getopt_timeout(optarg);
                break;
        }
    }

    /* Check requirements */
    if(!domainname)
        usage("Domain is mandatory");

    return(OK);
}

void print_help (void) {
    print_revision();
    print_revision_virt();
    print_copyright();

    printf("\n");

    printf("This plugin check the function of libvirtd.");

    printf("\n\n");

    print_usage();
    print_help_default();
    print_help_virt();
}

/* vim: set ts=4 sw=4 et syn=c : */
