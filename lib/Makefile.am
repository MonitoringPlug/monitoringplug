## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)

noinst_LIBRARIES  = libmonitoringplug.a
noinst_HEADERS = mp_longopt.h

if HAVE_EXPAT
noinst_LIBRARIES += librhcsutils.a

librhcsutils_a_SOURCES = rhcs_utils.c rhcs_utils.h
librhcsutils_a_CPPFLAGS = $(AM_CPPFLAGS) $(EXPAT_CFLAGS)
endif

if HAVE_LDNS	
noinst_LIBRARIES += libldnsutils.a

libldnsutils_a_SOURCES = ldns_utils.c ldns_utils.h
libldnsutils_a_CPPFLAGS = $(AM_CPPFLAGS) $(LDNS_CFLAGS)
endif

if HAVE_LIBVIRT
noinst_LIBRARIES += libvirtutils.a

libvirtutils_a_SOURCES = virt_utils.c virt_utils.h
libvirtutils_a_CPPFLAGS = $(AM_CPPFLAGS) $(LIBVIRT_CFLAGS)
endif

if HAVE_NET_SNMP	
noinst_LIBRARIES += libsnmputils.a

libsnmputils_a_SOURCES = snmp_utils.c snmp_utils.h
libsnmputils_a_CPPFLAGS = $(AM_CPPFLAGS) $(NETSNMP_CFLAGS)
endif

if HAVE_XMLRPC	
noinst_LIBRARIES += libxmlrpcutils.a

libxmlrpcutils_a_SOURCES = xmlrpc_utils.c xmlrpc_utils.h
libxmlrpcutils_a_CPPFLAGS = $(AM_CPPFLAGS) $(XMLRPC_CFLAGS)
endif

noinst_LIBRARIES += librpcutils.a

librpcutils_a_SOURCES = rpc_utils.c rpc_utils.h
librpcutils_a_CPPFLAGS = $(AM_CPPFLAGS)

libmonitoringplug_a_SOURCES = mp_common.c mp_common.h \
							  mp_utils.c mp_utils.h \
							  mp_args.c mp_args.h \
							  mp_check.c mp_check.h \
							  mp_popen.c mp_popen.h \
							  mp_eopt.c mp_eopt.h \
							  mp_net.c mp_net.h
							  
							  

