## Process this file with automake to produce Makefile.in

docdir = @docdir@

doc_DATA = README-EOPT

man_MANS = monitoringplug-eopt.5 monitoringplug-range.5

EXTRA_DIST = $(doc_DATA) $(man_MANS)

# base
man_MANS += check_file.1 check_nrped.1
if OS_LINUX
man_MANS += check_bonding.1 check_dhcp.1 check_mem.1 check_multipath.1 \
			check_sockets.1
endif
EXTRA_DIST += check_file.1 check_nrped.1 check_bonding.1 check_dhcp.1 \
			  check_mem.1 check_multipath.1 check_sockets.1

# cups
if HAVE_CUPS
man_MANS += check_cups_jobs.1
endif #HAVE_CUPS
EXTRA_DIST += check_cups_jobs.1

# curl
if HAVE_CURL
man_MANS += check_apache_status.1 check_aspsms_credits.1 check_webdav.1
if HAVE_JSON
man_MANS += check_buildbot_slave.1
endif #HAVE_JSON
if HAVE_CURL_TFTP
man_MANS += check_tftp.1
endif #HAVE_CURL_TFTP
endif #HAVE_CURL
EXTRA_DIST += check_apache_status.1 check_aspsms_credits.1 check_webdav.1 \
			  check_buildbot_slave.1 check_tftp.1

#dns
if HAVE_LDNS
man_MANS += check_dns_sync.1 check_dnssec_expiration.1 check_dnssec_trace.1 \
			check_dnssec_trust_anchor.1
endif #HAVE_LDNS
EXTRA_DIST += check_dns_sync.1 check_dnssec_expiration.1 check_dnssec_trace.1 \
			  check_dnssec_trust_anchor.1

#gnutls
if HAVE_GNUTLS
man_MANS += check_ssl_cert.1
endif #HAVE_GNUTLS
EXTRA_DIST += check_ssl_cert.1

#libvirt
if HAVE_LIBVIRT
man_MANS += check_libvirtd.1 check_libvirt_domain.1
endif #HAVE_LIBVIRT
EXTRA_DIST += check_libvirtd.1 check_libvirt_domain.1

#mysql
if HAVE_MYSQL
man_MANS += check_mysql.1 check_mysql_rows.1
endif #HAVE_MYSQL
EXTRA_DIST += check_mysql.1 check_mysql_rows.1

#rhcs
if HAVE_EXPAT
man_MANS += check_clustat.1
if HAVE_NET_SNMP
man_MANS += check_rhcsnmp.1
endif #HAVE_NET_SNMP
endif #HAVE_EXPAT
EXTRA_DIST += check_clustat.1 check_rhcsnmp.1

#rpc
if HAVE_RPC
man_MANS += check_nfs.1 check_rpc_ping.1
endif #HAVE_RPC
EXTRA_DIST += check_nfs.1 check_rpc_ping.1

#selinux
if HAVE_SELINUX
man_MANS += check_enforce.1 check_sebool.1
endif #HAVE_SELINUX
EXTRA_DIST += check_enforce.1 check_sebool.1

#smb
if HAVE_SMBCLIENT
man_MANS += check_smb_share.1
endif
EXTRA_DIST += check_smb_share.1

#snmp
if HAVE_NET_SNMP
man_MANS += check_akcp.1 check_arc_raid.1 check_apc_pdu.1 check_interface.1 check_qnap_disks.1 check_qnap_vols.1
endif
EXTRA_DIST += check_akcp.1 check_arc_raid.1 check_apc_pdu.1 check_interface.1 check_qnap_disks.1 check_qnap_vols.1

#xmprpc
if HAVE_XMLRPC
man_MANS += check_rhn_entitlements.1 check_koji_hub.1 check_koji_builder.1
endif
EXTRA_DIST += check_rhn_entitlements.1 check_koji_hub.1 check_koji_builder.1

if HAVE_XSLTPROC

CLEANFILES = $(man_MANS)

DB2MAN = --xinclude -nonet http://docbook.sourceforge.net/release/xsl/current/manpages/docbook.xsl

.xml.1:
	$(XSLTPROC) $(DB2MAN) $<
.xml.5:
	$(XSLTPROC) $(DB2MAN) $<

.xml.html:
	$(XSLTPROC) $(DB2HTML) $< > $@
endif

## vim: set ts=4 sw=4 syn=automake :
