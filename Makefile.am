## Process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = lib base cups curl dns dummy gnutls libvirt mysql rhcs rpc selinux \
	  smb snmp xmlrpc \
	  contrib doc policy tests

## dist-deb
AX_DIST_DEB_CLEANFILES = $(top_builddir)/$(PACKAGE)*_$(VERSION)_*.deb \
                         $(top_builddir)/$(PACKAGE)_$(VERSION)_*.* \
                         $(top_builddir)/$(PACKAGE)_$(VERSION).tar.gz \
                         $(top_builddir)/$(PACKAGE)_$(VERSION).dsc
EXTRA_DIST = $(top_builddir)/debian/control \
    $(top_builddir)/debian/compat $(top_builddir)/debian/changelog \
    $(top_builddir)/debian/rules $(top_builddir)/debian/README \
    $(top_builddir)/debian/copyright $(top_builddir)/debian/$(PACKAGE)*.install

dist-deb: deb

deb: $(PACKAGE)_$(VERSION)_*.deb

$(PACKAGE)_$(VERSION)_*.deb: dist-gzip
	$(AMTAR) -zxf $(distdir).tar.gz
	cd $(distdir); debuild -us -uc -i -I
	rm -rf $(distdir)

clean-local: clean-ax-dist-deb
clean-ax-dist-deb:
	-test -z "$(AX_DIST_DEB_CLEANFILES)" || rm -f $(AX_DIST_DEB_CLEANFILES)

dist-hook: dist-hook-ax-dist-deb
dist-hook-ax-dist-deb: $(AX_DIST_DEB_EXTRA_DIST)
	-test -z "$(AX_DIST_DEB_EXTRA_DIST)" || cp $(AX_DIST_DEB_EXTRA_DIST) $(distdir)

## vim: set ts=4 sw=4 syn=automake :
