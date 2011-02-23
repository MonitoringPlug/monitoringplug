## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)

noinst_LIBRARIES  = libmonitoringplug.a

if HAVE_EXPAT
noinst_LIBRARIES += librhcsutils.a

librhcsutils_a_SOURCES = rhcs_utils.c rhcs_utils.c
librhcsutils_a_CPPFLAGS = $(AM_CPPFLAGS) $(EXPAT_CFLAGS)
endif

if HAVE_LDNS	
noinst_LIBRARIES += libldnsutils.a

libldnsutils_a_SOURCES = ldns_utils.c ldns_utils.c
libldnsutils_a_CPPFLAGS = $(AM_CPPFLAGS) $(LDNS_CFLAGS)
endif

if HAVE_NET_SNMP	
noinst_LIBRARIES += libsnmputils.a

libsnmputils_a_SOURCES = snmp_utils.c snmp_utils.c
libsnmputils_a_CPPFLAGS = $(AM_CPPFLAGS) $(NETSNMP_CFLAGS)
endif

if HAVE_XMLRPC	
noinst_LIBRARIES += libxmlrpcutils.a

libxmlrpcutils_a_SOURCES = xmlrpc_utils.c xmlrpc_utils.c
libxmlrpcutils_a_CPPFLAGS = $(AM_CPPFLAGS) $(XMLRPC_CFLAGS)
endif

libmonitoringplug_a_SOURCES = mp_common.c mp_common.h \
							  mp_utils.c mp_utils.h \
							  mp_args.c mp_args.h \
							  mp_check.c mp_check.h \
							  mp_popen.c mp_popen.h \
							  mp_eopt.c mp_eopt.h
							  

