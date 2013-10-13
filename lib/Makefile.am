## Process this file with automake to produce Makefile.in

noinst_LIBRARIES  = libmonitoringplug.a \
					libmonitoringplugnotify.a \
					libmonitoringplugtemplate.a

libmonitoringplug_a_SOURCES = mp_common.c mp_common.h \
                              mp_utils.c mp_utils.h \
                              mp_args.c mp_args.h \
							  mp_getopt.c mp_getopt.h \
                              mp_check.c mp_check.h \
                              mp_perfdata.c mp_perfdata.h \
                              mp_eopt.c mp_eopt.h \
                              mp_net.c mp_net.h \
							  mp_subprocess.c mp_subprocess.h
if HAVE_TERMIOS
libmonitoringplug_a_SOURCES += mp_serial.c mp_serial.h
endif

libmonitoringplugnotify_a_SOURCES = mp_notify.c mp_notify.h

AM_YFLAGS = -d
noinst_LIBRARIES  += libmonitoringplugtemplate.a
libmonitoringplugtemplate_a_SOURCES = mp_template_yacc.y \
									  mp_template_lex.l \
									  mp_template.c \
									  mp_template.h

if HAVE_CURL
noinst_LIBRARIES += libcurlutils.a

libcurlutils_a_SOURCES = curl_utils.c curl_utils.h
libcurlutils_a_CPPFLAGS = $(LIBCURL_CPPFLAGS)
endif

if OS_LINUX
noinst_LIBRARIES += libdhcputils.a

libdhcputils_a_SOURCES = dhcp_utils.c dhcp_utils.h \
						 mp_net.c mp_net.h
endif

if HAVE_EXPAT
noinst_LIBRARIES += libexpatutils.a librhcsutils.a

libexpatutils_a_SOURCES = expat_utils.c expat_utils.h
libexpatutils_a_CPPFLAGS = $(EXPAT_CFLAGS)
librhcsutils_a_SOURCES = rhcs_utils.c rhcs_utils.h
librhcsutils_a_CPPFLAGS = $(EXPAT_CFLAGS)
endif

if HAVE_FCGI
noinst_LIBRARIES += libfcgiutils.a

libfcgiutils_a_SOURCES = fcgi_utils.c fcgi_utils.h
libfcgiutils_a_CPPFLAGS = $(FCGI_LIBS)
endif

if HAVE_LDNS
noinst_LIBRARIES += libldnsutils.a

libldnsutils_a_SOURCES = ldns_utils.c ldns_utils.h
libldnsutils_a_CPPFLAGS = $(LDNS_CFLAGS)
endif

if HAVE_LIBPQ
noinst_LIBRARIES += libpgsqlutils.a

libpgsqlutils_a_SOURCES = pgsql_utils.c pgsql_utils.h
libpgsqlutils_a_CPPFLAGS = $(PQINCPATH) $(LIBPQ_LIBS)
endif

if HAVE_LIBVIRT
noinst_LIBRARIES += libvirtutils.a

libvirtutils_a_SOURCES = virt_utils.c virt_utils.h
libvirtutils_a_CPPFLAGS = $(LIBVIRT_CFLAGS)
endif

if HAVE_MYSQL
noinst_LIBRARIES += libmysqlutils.a

libmysqlutils_a_SOURCES = mysql_utils.c mysql_utils.h
libmysqlutils_a_CPPFLAGS = $(MYSQL_CFLAGS)
endif

if HAVE_NET_SNMP
noinst_LIBRARIES += libsnmputils.a

libsnmputils_a_SOURCES = snmp_utils.c snmp_utils.h
libsnmputils_a_CPPFLAGS = $(NETSNMP_CFLAGS)
endif

if HAVE_XMLRPC
noinst_LIBRARIES += libxmlrpcutils.a

libxmlrpcutils_a_SOURCES = xmlrpc_utils.c xmlrpc_utils.h
libxmlrpcutils_a_CPPFLAGS = $(XMLRPC_CFLAGS)
endif

if HAVE_RPC
noinst_LIBRARIES += librpcutils.a

librpcutils_a_SOURCES = rpc_utils.c rpc_utils.h
endif

if HAVE_IPMI
noinst_LIBRARIES += libipmiutils.a

libipmiutils_a_SOURCES = ipmi_utils.c ipmi_utils.h
libipmiutils_a_CPPFLAGS = $(IPMI_CFLAGS)
endif

noinst_LIBRARIES += libsmsutils.a

libsmsutils_a_SOURCES = sms_utils.c sms_utils.h

## vim: set ts=4 sw=4 syn=automake :
