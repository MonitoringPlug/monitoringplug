## Process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = lib base cups curl dns dummy fcgi gnutls ipmi ldap libvirt mysql \
		  oping pgsql redis rhcs rpc selinux smb snmp xmlrpc \
		  contrib doc policy tests notify

EXTRA_DIST = $(top_builddir)/debian

MP_DIST_CLEANFILES =
MP_DIST_CLEANDIRS =

## dist
tarballs: dist-gzip
	md5sum $(PACKAGE)-$(VERSION).tar.gz > $(PACKAGE)-$(VERSION).tar.gz.md5
	sha1sum $(PACKAGE)-$(VERSION).tar.gz > $(PACKAGE)-$(VERSION).tar.gz.sha1

## dist-rpm
if HAVE_RPMBUILD
MP_DIST_CLEANDIRS += package build

dist-rpm: rpm

srpm: dist-gzip
	$(RPMBUILD) --define "_sourcedir $(abs_top_srcdir)" \
		--define "_specdir $(abs_top_srcdir)/contrib" \
		--define "_builddir $(abs_top_srcdir)/build" \
		--define "_srcrpmdir $(abs_top_srcdir)/package" \
		--define "_rpmdir $(abs_top_srcdir)/package" \
		--nodeps -bs contrib/monitoringplug.spec
rpm: dist-gzip
	$(RPMBUILD) --define "_sourcedir $(abs_top_srcdir)" \
		--define "_specdir $(abs_top_srcdir)/contrib" \
		--define "_builddir $(abs_top_srcdir)/build" \
	   	--define "_srcrpmdir $(abs_top_srcdir)/package" \
		--define "_rpmdir $(abs_top_srcdir)/package" \
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
	dh_clean
	rm -rf $(distdir)
endif

clean-local: clean-mp-dist
clean-mp-dist:
	dh_clean ||:
	-test -z "$(MP_DIST_CLEANFILES)" || rm -f $(MP_DIST_CLEANFILES)
	-test -z "$(MP_DIST_CLEANDIRS)" || rm -rf $(MP_DIST_CLEANDIRS)
## vim: set ts=4 sw=4 syn=automake :
