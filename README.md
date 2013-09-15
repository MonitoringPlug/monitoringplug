MonitoringPlug
==============

My collection of monitoring plugins for Nagios and similar monitoring systems. 

Base
----

Basic plugins which don't use any library.

*  check_bonding -- Check bonding status.
*  check_dhcp -- Check DHCP server for functionality.
*  check_file -- Check the property of a file.
*  check_gsm_signal -- Check the signal quality of a GSM modem.
*  check_mem -- Check memory usage.
*  check_memcached -- Check memcached status.
*  check_multipath -- Check multipath for failed paths.
*  check_nrped -- Check if run inside of nrpe.
*  check_sockets -- Check socket count (Linux only). 

CUPS
----

Use [CUPS](http://www.cups.org)) to query CUPS server.

*  check_cups_jobs -- Check CUPS job count and age. 

CURL
----

Use the [libcurl](http://curl.haxx.se/libcurl/) to test network servers.

*  check_apache_status -- Check apache mod_status output.
*  check_aspsms_credits -- Check available ​ASPSMS credits.
*  check_buildbot_slave -- Check BuildBot slave state by json-api.
*  check_tftp -- Check if a file can be downloaded from tftp.
*  check_webdav -- Check a WebDAV share. 

DNS
---

Use the [ldns](http://www.nlnetlabs.nl/projects/ldns/) library to query dns servers.

*  check_dns_authoritative -- Check a Authoritative DNS server.
*  check_dns_sync -- Check if a DNS zone is in sync.
*  check_dnssec_expiration -- Check if a DNS zone signatur is not expired.
*  check_dnssec_trace -- Check if a DNS zone signatur is traceable from a trust anchor.
*  check_dnssec_trust_anchor -- Check if the trust anchors in named.conf are valid.

FastCGI
-------

Use the FastCGI library to test fcgi Daemons.

*  check_fcgi_ping -- Check a FastCGI daemon.
*  check_fcgi_phpfpm -- Check a PHP-FPM Pool.

GNUTLS
------

Use the [GnuTLS](http://www.gnutls.org) library to check SSL/TLS related things.

*  check_ssl_cert -- Check expiration and trust of a SSL certificate. 

IPMI
----

Use the [OpenIPMI](http://openipmi.sourceforge.net/) library to check BMC Sensors.

*  check_ipmi_fan -- Check the give or all FANs by IPMI.
*  check_ipmi_mem -- Check Memory status by IPMI.
*  check_ipmi_psu -- Check one or all PSU by IPMI sensor.
*  check_ipmi_sensor -- Check the give or all IPMI Sensors. 

LibVirt
-------

Use the [LibVirt](http://libvirt.org/) library to check VirtualMachines.

*  check_libvirtd -- Check the libvirtd itself.
*  check_libvirt_domain -- Check if a given domain is running. 

MySQL
-----

Use libmysqlclient library to check a MySQL Server.

*  check_mysql -- Check MySQL connectivity and status.
*  check_mysql_rows -- Check mysql table row count. 

PostgreSQL
----------

Use the [libpq](http://www.postgresql.org/) library to check PostgreSQL.

*  check_pgsql -- Check PostgreSQL connectivity.

RHCS
----

Plugins to check state of a RedHat Cluster Suite.

*  check_clustat -- Parse the clustat output. (Requires expat.)
*  check_rhcsnmp -- Check the state of a RedHat Cluster Suite by snmp. 

(SUN)RPC
--------

Plugins to check SUNRPC service state.

*  check_nfs -- Check if the Host is exporting at least one or the named path.
*  check_rpc_ping -- Check if named RPC program is responding. 

SELinux
-------

Check SELinux factors.

*  check_enforce -- Check SELinux state and policy.
*  check_sebool -- Check SELinux boolean state. 

SMB
---

Check SMB/CIFS.

*  check_smb_share -- Check SMB/CIFS share connection. 

SNMP
----

Use the ​net-snmp library to query SNMP agents.

*  check_akcp -- Check the state of a AKCP environment sensor.
*  check_apc_pdu -- Check the psu and outlet status of a APC PDU.
*  check_arc_raid -- Check the raid status of a Areca RAID.
*  check_interface -- Check Interface state with SNMP IF-MIB.
*  check_qnap_disks -- Check the dist status of a QNap.
*  check_qnap_vols -- Check the volume status of a QNap.
*  check_snmp_ups -- Check status of a UPS conforming to RFC 1628 by SNMP.

Varnish
-------

Use the [libvarnish](http://www.varnish-cache.org/) to check Varnish.

*  check_varnish -- Check a Varnish cache server.

XMLRPC
------

Use the [xmlrpc-c](http://xmlrpc-c.sourceforge.net/) library to query XMLRPC server.

*  check_koji_builder -- This plugin check a Koji-Builder.
*  check_koji_hub -- This plugin check a Koji-Hub.
*  check_rhn_entitlements -- Check available entitlement on a RedHat Satelite or RHN. 

Dummy
-----

*  check_dummy -- Don't check anything, return arguments.
*  check_timeout -- Don't check anything, cause a timeout. 

Notify
------

Some notifications plugins for Nagios.

*  notify_aspsms -- Send a notification by SMS with a ASPSMS account.
*  notify_mail -- Send a notification by mail.
*  notify_sms -- Send a notification by SMS with a Modem.
*  notify_stdout -- Print a notification to stdout for debuging. 

Enjoy!
  Marius
