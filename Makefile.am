## Process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = lib base cups curl dns dummy gnutls libvirt mysql rhcs rpc selinux \
	  smb snmp xmlrpc \
	  contrib doc policy tests

dist-deb:
	debuild -us -uc
## vim: set ts=4 sw=4 syn=automake :
