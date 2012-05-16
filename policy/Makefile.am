## Process this file with automake to produce Makefile.in

EXTRA_DIST = monitoringplug.te monitoringplug.fc


if HAVE_SELINUX

all: monitoringplug.pp

monitoringplug.pp: monitoringplug.te monitoringplug.fc
	make -f /usr/share/selinux/devel/Makefile || :

endif## vim: set ts=4 sw=4 syn=automake :
