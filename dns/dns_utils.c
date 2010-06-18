/**
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

ldns_resolver* createResolver(const char *dns_server){
    ldns_resolver   *res = NULL;
    ldns_status     status;
    ldns_rdf        *serv_rdf;
    
    if (dns_server) {
        // Use the given DNS server
        res = ldns_resolver_new();
        if (!res)
            return NULL;
        
        /* add the nameserver */
        serv_rdf = getaddr(NULL, dns_server);
        
        if (!serv_rdf) {
            ldns_resolver_free(res);
            ldns_rdf_deep_free(serv_rdf);
            return NULL;
        } else {
            if (ldns_resolver_push_nameserver(res, serv_rdf) != LDNS_STATUS_OK) {
                ldns_resolver_free(res);
                ldns_rdf_deep_free(serv_rdf);
                return NULL;
            } else {
                ldns_rdf_deep_free(serv_rdf);
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

ldns_rdf* getaddr(ldns_resolver *res, const char *hostname) {
    ldns_rdf        *rdf;
    ldns_resolver   *r;
    ldns_rr_list    *rrl;
    ldns_status     status;
    ldns_pkt        *pkt;
   
   
    /* Check if hostname is a ip */
    rdf = ldns_rdf_new_frm_str(LDNS_RDF_TYPE_A, hostname);
#ifdef USE_IPV6
    if (!rdf) {
        rdf = ldns_rdf_new_frm_str(LDNS_RDF_TYPE_AAAA, hostname);
    }
#endif
    if (rdf)
        return rdf;
   
    rdf = ldns_rdf_new_frm_str(LDNS_RDF_TYPE_DNAME, hostname);
    if (!rdf)
        return NULL;
    
    r = res;
    if (res == NULL) {
        /* prepare a new resolver, using /etc/resolv.conf as a guide  */
        status = ldns_resolver_new_frm_file(&r, NULL);
        if (status != LDNS_STATUS_OK) {
            return NULL;
        }
    }

#ifdef USE_IPV6
    pkt = ldns_resolver_query(r, rdf, LDNS_RR_TYPE_AAAA, LDNS_RR_CLASS_IN, LDNS_RD);
    
    if (pkt) {
        rrl = ldns_pkt_rr_list_by_name_and_type(pkt, rdf,
            LDNS_RR_TYPE_AAAA, LDNS_SECTION_ANSWER);
            
        if (ldns_rr_list_rr_count(rrl) > 0)
            return ldns_rr_rdf(ldns_rr_list_rr(rrl,0),0);
    }
#endif

    pkt = ldns_resolver_query(r, rdf, LDNS_RR_TYPE_A, LDNS_RR_CLASS_IN, LDNS_RD);
    
    if (pkt) {
        rrl = ldns_pkt_rr_list_by_name_and_type(pkt, rdf,
            LDNS_RR_TYPE_A, LDNS_SECTION_ANSWER);
            
        if (ldns_rr_list_rr_count(rrl) > 0)
            return ldns_rr_rdf(ldns_rr_list_rr(rrl,0),0);
    }
    
    return NULL;
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


/* EOF */