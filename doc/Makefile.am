## Process this file with automake to produce Makefile.in

docdir = @docdir@

doc_DATA = README-EOPT

man_MANS = check_file.1 check_nrped.1 monitoringplug-eopt.5
if OS_LINUX
man_MANS += check_bonding.1 check_dhcp.1 check_mem.1 check_multipath.1 check_sockets.1
endif
if HAVE_CUPS
man_MANS += check_cups_jobs.1
endif

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
