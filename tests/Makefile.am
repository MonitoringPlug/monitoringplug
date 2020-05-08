## Process this file with automake to produce Makefile.in

LDADD = $(CHECK_LIBS) ../lib/libmonitoringplug.a
AM_CFLAGS = $(CHECK_CFLAGS) -Dabs_srcdir="\"$(abs_srcdir)\""

if HAVE_CHECK

check_PROGRAMS = check_monitoringplug \
		check_args \
		check_check \
		check_sms \
		check_subprocess \
		check_template

check_monitoringplug_SOURCES = main.c main.h \
    check_common.c \
    check_eopt.c \
    check_utils.c \
	check_perfdata.c

check_sms_LDADD = ../lib/libsmsutils.a $(LDADD)

check_template_LDADD = ../lib/libmonitoringplugtemplate.a $(LDADD)

if HAVE_EXPAT
check_PROGRAMS += check_rhcs

check_rhcs_LDADD = ../lib/librhcsutils.a $(EXPAT_LIBS) $(LDADD)
check_rhcs_CFLAGS = $(AM_CFLAGS) $(EXPAT_CFLAGS)
endif

#if HAVE_NET_SNMP
#check_PROGRAMS += check_snmp
#
#check_snmp_LDADD = ../lib/libsnmputils.a $(NETSNMP_LIBS) $(LDADD)
#check_snmp_CFLAGS = $(NETSNMP_CFLAGS)
#endif

endif

SHARNESSSCRIPTS = base/check_bonding.t \
				   base/check_file.t \
				   base/check_memcached.t \
				   curl/check_rabbitmq.t \
				   curl/check_buildbot_slave.t \
				   redis/check_redis.t \
				   redis/check_redis_slave.t \
				   snmp/check_akcp.t \
				   snmp/check_apc_pdu.t \
				   snmp/check_arc_raid.t \
				   snmp/check_keepalived_vrrp.t \
				   snmp/check_qnap_disks.t \
				   snmp/check_qnap_vols.t

TESTS = $(check_PROGRAMS)
#		$(SHARNESSSCRIPTS)

EXTRA_DIST = mpcheck.h eopt.ini eopt_long.ini testdata sharness.sh $(SHARNESSSCRIPTS)
## vim: set ts=4 sw=4 syn=automake :
