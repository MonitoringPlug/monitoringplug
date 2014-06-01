## Process this file with automake to produce Makefile.in

docdir = @docdir@

doc_DATA = README-EOPT

if !DISABLE_MAN
man_MANS = monitoringplug-eopt.5 \
		   monitoringplug-range.5 \
		   monitoringplug-template.5

EXTRA_DIST = $(doc_DATA) $(man_MANS)

# base
man_MANS += check_file.1 check_memcached.1 check_nrped.1
if OS_LINUX
man_MANS += check_bonding.1 check_dhcp.1 check_mem.1 check_multipath.1 \
			check_sockets.1
endif
if HAVE_TERMIOS
man_MANS += check_gsm_signal.1
endif
EXTRA_DIST += check_file.1 check_memcached.1 check_nrped.1 check_bonding.1 \
			  check_dhcp.1 check_mem.1 check_multipath.1 check_redis.1 \
			  check_sockets.1 check_gsm_signal.1

# cups
if HAVE_CUPS
man_MANS += check_cups_jobs.1
endif #HAVE_CUPS
EXTRA_DIST += check_cups_jobs.1

# curl
if HAVE_CURL
man_MANS += check_apache_status.1 check_aspsms_credits.1 check_webdav.1
if HAVE_JSON
man_MANS += check_buildbot_slave.1 check_rabbitmq.1
endif #HAVE_JSON
if HAVE_CURL_TFTP
man_MANS += check_tftp.1
endif #HAVE_CURL_TFTP
endif #HAVE_CURL
EXTRA_DIST += check_apache_status.1 check_aspsms_credits.1 check_webdav.1 \
			  check_buildbot_slave.1 check_rabbitmq.1 check_tftp.1

#dns
if HAVE_LDNS
man_MANS += check_dns_authoritative.1 check_dns_sync.1 \
			check_dnssec_expiration.1 check_dnssec_trace.1 \
			check_dnssec_trust_anchor.1
endif #HAVE_LDNS
EXTRA_DIST += check_dns_authoritative.1 check_dns_sync.1 \
			  check_dnssec_expiration.1 check_dnssec_trace.1 \
			  check_dnssec_trust_anchor.1

#fcgi
if HAVE_FCGI
man_MANS += check_fcgi_ping.1
if HAVE_JSON
man_MANS += check_fcgi_php.1
endif #HAVE_JSON
endif #HAVE_FCGI
EXTRA_DIST += check_fcgi_ping.1 check_fcgi_php.1

#gnutls
if HAVE_GNUTLS
man_MANS += check_ssl_cert.1
endif #HAVE_GNUTLS
EXTRA_DIST += check_ssl_cert.1

#ipmi
if HAVE_IPMI
man_MANS += check_ipmi_fan.1 check_ipmi_mem.1 check_ipmi_psu.1 check_ipmi_sensor.1
endif #HAVE_IPMI
EXTRA_DIST += check_ipmi_fan.1 check_ipmi_mem.1 check_ipmi_psu.1 check_ipmi_sensor.1

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

#oping
if HAVE_OPING
man_MANS += check_oping.1
endif #HAVE_OPING
EXTRA_DIST += check_oping.1

#postgresql
if HAVE_LIBPQ
man_MANS += check_pgsql.1 check_pgsql_slave.1
endif #HAVE_LIBPQ
EXTRA_DIST +=  check_pgsql.1 check_pgsql_slave.1

#redis
if HAVE_REDIS
man_MANS += check_redis.1 check_redis_slave.1
endif #HAVE_REDIS
EXTRA_DIST += check_redis.1 check_redis_slave.1

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
man_MANS += check_akcp.1 check_arc_raid.1 check_apc_pdu.1 check_interface.1 \
			check_keepalived_vrrp.1 check_qnap_disks.1 check_qnap_vols.1 check_snmp_ups.1
endif
EXTRA_DIST += check_akcp.1 check_arc_raid.1 check_apc_pdu.1 check_interface.1 \
			  check_keepalived_vrrp.1 check_qnap_disks.1 check_qnap_vols.1 check_snmp_ups.1

#varnish
if HAVE_VARNISH
man_MANS += check_varnish.1 check_varnish_backends.1
endif
EXTRA_DIST += check_varnish.1 check_varnish_backends.1

#xmprpc
if HAVE_XMLRPC
man_MANS += check_rhn_entitlements.1 check_koji_hub.1 check_koji_builder.1
endif
EXTRA_DIST += check_rhn_entitlements.1 check_koji_hub.1 check_koji_builder.1

#notify
man_MANS += notify_mail.1 notify_stdout.1
if HAVE_TERMIOS
man_MANS += notify_sms.1
endif
if HAVE_CURL
man_MANS += notify_aspsms.1
endif
EXTRA_DIST += notify_aspsms.1 notify_mail.1 notify_sms.1 notify_stdout.1
endif # !DISABLE_MAN

DISTCLEANFILES = $(man_MANS)

DB2MAN = --xinclude -nonet http://docbook.sourceforge.net/release/xsl/current/manpages/docbook.xsl

.xml.1:
if HAVE_XSLTPROC
	$(XSLTPROC) $(DB2MAN) $<
else
	@echo "Error: XSLTPROC not found! Can not build manpages"
	@echo "       Either try --without-man or the release tarball."
	@exit 1
endif
.xml.5:
if HAVE_XSLTPROC
	$(XSLTPROC) $(DB2MAN) $<
else
	@echo "Error: XSLTPROC not found! Can not build manpages"
	@echo "       Either try --without-man or the release tarball."
	@exit 1
endif

## vim: set ts=4 sw=4 syn=automake :
