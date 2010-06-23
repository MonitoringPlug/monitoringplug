/***
 * Monitoring Plugin - dns_utils.h
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

#include "mp_common.h"
#include "dns_utils.h"

#include <stdio.h>

ldns_resolver* createResolver(const char *dnsserver) {
    ldns_resolver   *res = NULL;
    ldns_status     status;
    ldns_rdf        *ns_rdf;

    if (dnsserver) {
        // Use the given DNS server
        res = ldns_resolver_new();
        if (!res)
            return NULL;
        
        // Create rdf from dnsserver
        ns_rdf = ldns_rdf_new_frm_str(LDNS_RDF_TYPE_A, dnsserver);
#ifdef USE_IPV6
        if (!ns_rdf)
            ns_rdf = ldns_rdf_new_frm_str(LDNS_RDF_TYPE_AAAA, dnsserver);
#endif
        if (!ns_rdf) {
            ldns_resolver_deep_free(res);
            return NULL;
        } else {
            status = ldns_resolver_push_nameserver(res, ns_rdf);
            ldns_rdf_deep_free(ns_rdf);
            if (status != LDNS_STATUS_OK) {
                ldns_resolver_free(res);
                return NULL;
            }
        }
    } else {
        // Use a DNS server from resolv.conf
        status = ldns_resolver_new_frm_file(&res, NULL);
        if (status != LDNS_STATUS_OK) {
            ldns_resolver_free(res);
            return NULL;
        }
    }

    return res;
}

void resolverEnableDnssec(ldns_resolver* res) {
    ldns_resolver_set_dnssec(res,1);
    ldns_resolver_set_dnssec_cd(res, 1);
}

ldns_rr_list* getaddr_rdf(ldns_resolver *res, ldns_rdf *hostrdf) {
    ldns_rdf        *rdf;
    ldns_resolver   *r = NULL;
    ldns_rr_list    *rrl;
    ldns_rr_list    *ret = NULL;
    ldns_pkt        *pkt;
    ldns_status     status;

    if (ldns_rdf_get_type(hostrdf) == LDNS_RDF_TYPE_A
#ifdef USE_IPV6
        || ldns_rdf_get_type(hostrdf) == LDNS_RDF_TYPE_AAAA
#endif
       ) {
        rdf = ldns_rdf_address_reverse(hostrdf);

        r = res;
        if (res == NULL) {
            status = ldns_resolver_new_frm_file(&r, NULL);
            if (status != LDNS_STATUS_OK)
                return NULL;
        }

        // Fetch PTR
        pkt = ldns_resolver_query(r, rdf, LDNS_RR_TYPE_PTR, LDNS_RR_CLASS_IN,
                                          LDNS_RD);

        if (pkt == NULL || ldns_pkt_get_rcode(pkt) != LDNS_RCODE_NOERROR)
            return NULL;

        rrl = ldns_pkt_rr_list_by_name_and_type(pkt, rdf, LDNS_RR_TYPE_PTR,
                                                          LDNS_SECTION_ANSWER);

        if (ldns_rr_list_rr_count(rrl) != 1)
            return NULL;

        ldns_rdf_deep_free(rdf);
        rdf = ldns_rdf_clone(ldns_rr_rdf(ldns_rr_list_rr(rrl,0),0));
        ldns_pkt_free(pkt);
        ldns_rr_list_deep_free(rrl);
    } else if (ldns_rdf_get_type(hostrdf) == LDNS_RDF_TYPE_DNAME) {
        rdf = hostrdf;
    } else {
        return NULL;
    }

    if (r == NULL) {
        r = res;
        if (res == NULL) {
            status = ldns_resolver_new_frm_file(&r, NULL);
            if (status != LDNS_STATUS_OK)
                return NULL;
        }
    }

#ifdef USE_IPV6
    if (ldns_rdf_get_type(hostrdf) != LDNS_RDF_TYPE_A) {
        // Fetch AAAA
        pkt = ldns_resolver_query(r, rdf, LDNS_RR_TYPE_AAAA, LDNS_RR_CLASS_IN,
                                          LDNS_RD);

        if (pkt != NULL && ldns_pkt_get_rcode(pkt) == LDNS_RCODE_NOERROR) {
            rrl = ldns_pkt_rr_list_by_name_and_type(pkt, rdf, LDNS_RR_TYPE_AAAA,
                                                    LDNS_SECTION_ANSWER);
            ldns_pkt_free(pkt);

            if (ldns_rr_list_rr_count(rrl) > 0) {
                ret = rrl;
                rrl = NULL;
            } else {
                ldns_rr_list_free(rrl);
            }
        }
    }

    if (ldns_rdf_get_type(hostrdf) != LDNS_RDF_TYPE_AAAA) {
#else
    if (1) {
#endif /* USE_IPV6 */

        // Fetch AA
        pkt = ldns_resolver_query(r, rdf, LDNS_RR_TYPE_A, LDNS_RR_CLASS_IN,
                                          LDNS_RD);

        if (pkt != NULL && ldns_pkt_get_rcode(pkt) == LDNS_RCODE_NOERROR) {

            rrl = ldns_pkt_rr_list_by_name_and_type(pkt, rdf, LDNS_RR_TYPE_A,
                                                    LDNS_SECTION_ANSWER);

            ldns_pkt_free(pkt);

            if (ldns_rr_list_rr_count(rrl) > 0) {
                if (ret == NULL) {
                    ret = rrl;
                } else {
                    ldns_rr_list_cat(ret, rrl);
                    ldns_rr_list_free(rrl);
                }
            } else {
                ldns_rr_list_free(rrl);
            }
        } // if (pkt != NULL && ldns_pkt_ge...
    }

    if (res == NULL)
        ldns_resolver_deep_free(r);

    return ret;
}

ldns_rr_list* getaddr(ldns_resolver *res, const char *hostname) {

    ldns_rdf    *rdf;

    /* Check if hostname is a ip */
    rdf = ldns_rdf_new_frm_str(LDNS_RDF_TYPE_A, hostname);
#ifdef USE_IPV6
    if (!rdf) {
        rdf = ldns_rdf_new_frm_str(LDNS_RDF_TYPE_AAAA, hostname);
    }
#endif
    if (!rdf) {
        rdf = ldns_rdf_new_frm_str(LDNS_RDF_TYPE_DNAME, hostname);
    }
    if (!rdf)
        return NULL;

    return getaddr_rdf(res, rdf);
}

ldns_rr_list* loadKeyfile(const char *filename) {

    int     col = 0;
    int     line = 0;
    FILE    *key_file;
    char    c;
    char    linebuffer[LDNS_MAX_PACKETLEN];

    ldns_status     status;
    ldns_rr         *rr;
    ldns_rr_list    *trusted_keys;

    // Try open trusted key file
    key_file = fopen(filename, "r");
    if (!key_file) {
        if (mp_verbose >= 1)
            fprintf(stderr,"Error opening trusted-key file %s: %s\n", filename,
                                                           strerror(errno));
        return NULL;
    }

    // Create empty list
    trusted_keys = ldns_rr_list_new();

    // Read File
    do {
        c = getc(key_file);
        if (c == '\n' || c == EOF) {
            linebuffer[col] = '\0';
            line++;
            if (linebuffer[0] == ';' || col == 0) {
                col = 0;
                continue;
            }
            col = 0;

            status = ldns_rr_new_frm_str(&rr, linebuffer, 0, NULL, NULL);
            if (status != LDNS_STATUS_OK) {
                if (mp_verbose >= 1)
                    fprintf(stderr, "Error parsing RR in %s:%d: %s\n",
                        filename, line, ldns_get_errorstr_by_id(status));
                if (mp_verbose >= 2)
                    fprintf(stderr, "%s\n", linebuffer);
                ldns_rr_free(rr);
            } else if (ldns_rr_get_type(rr) == LDNS_RR_TYPE_DNSKEY ||
                       ldns_rr_get_type(rr) == LDNS_RR_TYPE_DS) {
                ldns_rr_list_push_rr(trusted_keys, rr);
            } else {
                ldns_rr_free(rr);
			}
        } else {
            linebuffer[col++] = c;
        }

    } while (c != EOF);

    fclose(key_file);

    return trusted_keys;
}

ldns_rr_list* loadAnchorfile(const char *filename) {

    int     col = 0;
    int     line = 0;
    int     grouped = 0;
    int     tk_section = 0;
    FILE    *key_file;
    char    c;
    char    linebuffer[LDNS_MAX_PACKETLEN];

    ldns_rdf        *rd;
    ldns_rr         *rr;
    ldns_rr_list    *trusted_keys;

    // Try open trusted key file
    key_file = fopen(filename, "r");
    if (!key_file) {
        if (mp_verbose >= 1)
            fprintf(stderr,"Error opening trusted-key file %s: %s\n", filename,
                                                           strerror(errno));
        return NULL;
    }

    // Create empty list
    trusted_keys = ldns_rr_list_new();

    // Read File
    do {
        c = getc(key_file);
        if ((c == '\n' && grouped == 0) || c == EOF) {
            linebuffer[col] = '\0';
            line++;

            if(strstr(linebuffer, "trusted-keys")) {
                col = 0;
                tk_section = 1;
                continue;
            }

            if (linebuffer[0] == ';' || col == 0 || tk_section == 0) {
                col = 0;
                continue;
            }
            col = 0;

            rr = ldns_rr_new();
            ldns_rr_set_class(rr, LDNS_RR_CLASS_IN);
            ldns_rr_set_type(rr, LDNS_RR_TYPE_DNSKEY);
            ldns_rr_set_ttl(rr, 3600);

            char *cur = linebuffer;
            char *t = strtok(cur, " ");
            cur += strlen(t)+1;


            ldns_str2rdf_dname(&rd, t);
            ldns_rr_set_owner(rr, rd);

            t = strtok(cur, " ");
            cur += strlen(t)+1;

            ldns_str2rdf_int16(&rd, t);
            ldns_rr_push_rdf(rr, rd);

            t = strtok(cur, " ");
            cur += strlen(t)+1;

            ldns_str2rdf_int8(&rd, t);
            ldns_rr_push_rdf(rr, rd);

            t = strtok(cur, " ");
            cur += strlen(t)+1;

            ldns_str2rdf_alg(&rd, t);
            ldns_rr_push_rdf(rr, rd);

            t = strtok(cur, " ");

            if (t[strlen(t)-1] == ';')
                t[strlen(t)-1] = '\0';

            ldns_str2rdf_b64(&rd, t);
            ldns_rr_push_rdf(rr, rd);

            ldns_rr_list_push_rr(trusted_keys,rr);

        } else {
            if (c == '}') {
                tk_section = 0;
            } else if (c == '"') {
                grouped = (grouped+1)%2;
            } else {
                linebuffer[col++] = c;
            }
        }

    } while (c != EOF);

    fclose(key_file);

    return trusted_keys;
}


/* vim: set ts=4 sw=4 et : */
