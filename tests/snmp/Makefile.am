## Process this file with automake to produce Makefile.in

EXTRA_DIST = check_akcp.t \
			 check_apc_pdu.t \
			 check_arc_raid.t \
			 check_keepalived_vrrp.t \
			 check_qnap_disks.t \
			 check_qnap_vols.t

if HAVE_NET_SNMP

BASE=$abs_top_srcdir
TESTS = $(EXTRA_DIST)

endif

## vim: set ts=4 sw=4 syn=automake :
