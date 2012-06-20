## Process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = lib base cups curl dns dummy gnutls libvirt mysql rhcs rpc selinux \
	  smb snmp xmlrpc \
	  contrib doc policy tests

EXTRA_DIST = $(top_builddir)/debian 

MP_DIST_CLEANFILES =
MP_DIST_CLEANDIRS =

## dist-rpm
if HAVE_RPMBUILD
MP_DIST_CLEANFILES += $(top_builddir)/$(PACKAGE)-$(VERSION)-*.rpm
MP_DIST_CLEANDIRS += x86_64 i386 build

dist-rpm: rpm

srpm: dist-gzip
	$(RPMBUILD) --define "_sourcedir $(abs_top_srcdir)" \
		--define "_specdir $(abs_top_srcdir)/contrib" \
		--define "_builddir $(abs_top_srcdir)/build" \
		--define "_srcrpmdir $(abs_top_srcdir)" \
		--define "_rpmdir $(abs_top_srcdir)" \
		--nodeps -bs contrib/monitoringplug.spec
rpm: dist-gzip
	$(RPMBUILD) --define "_sourcedir $(abs_top_srcdir)" \
		--define "_specdir $(abs_top_srcdir)/contrib" \
		--define "_builddir $(abs_top_srcdir)/build" \
	   	--define "_srcrpmdir $(abs_top_srcdir)" \
		--define "_rpmdir $(abs_top_srcdir)" \
		--nodeps -ba contrib/monitoringplug.spec

endif
## dist-deb
if HAVE_DEBUILD
MP_DIST_CLEANFILES += $(top_builddir)/$(PACKAGE)*_$(VERSION)_*.deb \
                      $(top_builddir)/$(PACKAGE)_$(VERSION)_*.* \
                      $(top_builddir)/$(PACKAGE)_$(VERSION).tar.gz \
                      $(top_builddir)/$(PACKAGE)_$(VERSION).dsc

dist-deb: deb

deb: $(PACKAGE)_$(VERSION)_*.deb

$(PACKAGE)_$(VERSION)_*.deb: dist-gzip
	$(AMTAR) -zxf $(distdir).tar.gz
	cd $(distdir); debuild -us -uc -i -I
	rm -rf $(distdir)
endif

clean-local: clean-mp-dist
clean-mp-dist:
	-test -z "$(MP_DIST_CLEANFILES)" || rm -f $(MP_DIST_CLEANFILES)
	-test -z "$(MP_DIST_CLEANDIRS)" || rm -rf $(MP_DIST_CLEANDIRS)
## vim: set ts=4 sw=4 syn=automake :
