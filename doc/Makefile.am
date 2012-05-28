## Process this file with automake to produce Makefile.in

docdir = @docdir@

doc_DATA = README-EOPT

man_MANS = monitoringplug-eopt.5 monitoringplug-range.5

# base
man_MANS += check_file.1 check_nrped.1
if OS_LINUX
man_MANS += check_bonding.1 check_dhcp.1 check_mem.1 check_multipath.1 check_sockets.1
endif #OS_LINUX

# cups
if HAVE_CUPS
man_MANS += check_cups_jobs.1
endif #HAVE_CUPS

# curl
if HAVE_CURL
man_MANS += check_apache_status.1 check_aspsms_credits.1 check_webdav.1
if HAVE_JSON
man_MANS += check_buildbot_slave.1
endif #HAVE_JSON
endif #HAVE_CURL
if HAVE_CURL_TFTP
man_MANS += check_tftp.1
endif #HAVE_CURL_TFTP

EXTRA_DIST = $(doc_DATA)

if HAVE_XSLTPROC

EXTRA_DIST += $(man_MANS)

CLEANFILES = $(man_MANS)

DB2MAN = --xinclude -nonet http://docbook.sourceforge.net/release/xsl/current/manpages/docbook.xsl

.xml.1:
	$(XSLTPROC) $(DB2MAN) $<
.xml.5:
	$(XSLTPROC) $(DB2MAN) $<

endif

BUILT_EXTRA_DIST = $(man_MANS)

## vim: set ts=4 sw=4 syn=automake :
