## Process this file with automake to produce Makefile.in

docdir = @docdir@

doc_DATA = README-EOPT

man_MANS = monitoringplug-eopt.5

EXTRA_DIST = $(doc_DATA)

if HAVE_XSLTPROC

EXTRA_DIST += $(man_MANS)

CLEANFILES = $(man_MANS)

DB2MAN = -nonet http://docbook.sourceforge.net/release/xsl/current/manpages/docbook.xsl

%.5 : %.xml
	$(XSLTPROC) $(DB2MAN) $<
%.8 : %.xml
	$(XSLTPROC) $(DB2MAN) $<

endif

BUILT_EXTRA_DIST = $(man_MANS)

## vim: set ts=4 sw=4 syn=automake :
