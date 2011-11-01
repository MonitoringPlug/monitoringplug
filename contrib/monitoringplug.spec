%if 0%{?rhel}
%if 0%{?rhel} >= 6
%define gnutls 1
%else
%define gnutls 0
%endif
%else
%define gnutls 1
%endif

Name:           monitoringplug
Version:        0.4
Release:        1%{?dist}
Summary:        Collection of monitoring plugins for Nagios and similar monitoring systems.

Group:          Applications/System
License:        GPL
URL:            http://svn.durchmesser.ch/trac/monitoringplug
Source0:        %{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  ldns-devel
BuildRequires:  libselinux-devel
%if 0%{?rhel} <= 5
BuildRequires:	curl-devel
%else
BuildRequires:	libcurl-devel
BuildRequires:  cups-devel
%endif
BuildRequires:	xmlrpc-c-devel
BuildRequires:	expat-devel
BuildRequires:	net-snmp-devel
%if %{gnutls}
BuildRequires:  gnutls-devel
%endif
BuildRequires:	selinux-policy

%package base
Summary:        Collection of basic monitoring plugins for Nagios and similar monitoring systems.
Group:          Applications/System
Requires:	monitoringplug

%if 0%{?rhel} >= 6
%package cups
Summary:        Collection of cups monitoring plugins for Nagios and similar monitoring systems.
Group:          Applications/System
Requires:	cups
%endif

%package curl
Summary:        Collection of curl-based monitoring plugins for Nagios and similar monitoring systems.
Group:          Applications/System
%if 0%{?rhel} <= 5
Requires:       curl
%else
Requires:       libcurl
%endif
Requires:       monitoringplug

%package dns
Summary:        Collection of dns monitoring plugins for Nagios and similar monitoring systems.
Group:          Applications/System
Requires:	ldns
Requires:       monitoringplug

%if %{gnutls}
%package gnutls
Summary:        Collection of dns monitoring plugins for Nagios and similar monitoring systems.
Group:          Applications/System
Requires:	gnutls
Requires:       monitoringplug
%endif

%package rhcs
Summary:        Collection of RedHat Cluster Suitmonitoring plugins for Nagios and similar monitoring systems.
Group:          Applications/System
Requires:	net-snmp-libs
Requires:	expat
Requires:       monitoringplug

%package rpc
Summary:        Collection of SUN RPC plugins for Nagios and similar monitoring systems.
Group:          Applications/System
Requires:       monitoringplug

%package selinux
Summary:        Collection of selinux monitoring plugins for Nagios and similar monitoring systems.
Group:          Applications/System
Requires:	libselinux
Requires:       monitoringplug

%package snmp
Summary:        Collection of snmp-based monitoring plugins for Nagios and similar monitoring systems.
Group:          Applications/System
Requires:	net-snmp-libs
Requires:       monitoringplug

%package xmlrpc
Summary:        Collection of xmlrpc monitoring plugins for Nagios and similar monitoring systems.
Group:          Applications/System
Requires:       xmlrpc-c-client
Requires:       monitoringplug

%description
Collection of monitoring plugins for Nagios and similar monitoring systems.

%description base
Collection of monitoring plugins for Nagios and similar monitoring systems.
This package contains the base and dummy plugins which don't need any
additional libraries.

%if 0%{?rhel} >= 6
%description cups
Collection of monitoring plugins for Nagios and similar monitoring systems.
This package contains the cups based plugins.
%endif

%description curl
Collection of monitoring plugins for Nagios and similar monitoring systems.
This package contains the curl based plugins.

%description dns
Collection of monitoring plugins for Nagios and similar monitoring systems.
This package contains the dns plugins which use the ldns library.

%if 0%{?rhel} >= 6
%description gnutls
Collection of monitoring plugins for Nagios and similar monitoring systems.
This package contains the dns plugins which use the gnutls library.
%endif

%description rhcs
Collection of monitoring plugins for Nagios and similar monitoring systems.
This package contains the RedHat Cluster Suite plugins using snmp and expat.

%description rpc
Collection of monitoring plugins for Nagios and similar monitoring systems.
This package contains the SUN RPC plugins.

%description selinux
Collection of monitoring plugins for Nagios and similar monitoring systems.
This package contains the selinux plugins.

%description snmp
Collection of monitoring plugins for Nagios and similar monitoring systems.
This package contains the snmp based plugins.

%description xmlrpc
Collection of monitoring plugins for Nagios and similar monitoring systems.
This package contains the xmlrpc plugins.

%prep
%setup -q

%build
%if 0%{?rhel} <= 5
%configure SELINUX_CFLAGS=" " SELINUX_LIBS=-lselinux
%endif
%if 0%{?rhel} >= 6
%configure
%endif
make %{?_smp_mflags}

cd policy
make -f /usr/share/selinux/devel/Makefile
bzip2 %{name}.pp
cd ..

%check
make %{?_smp_mflags} check

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT
%{__install} -d %{buildroot}%{_datadir}/selinux/packages/
%{__install} -m 0644 policy/%{name}.pp.bz2 %{buildroot}%{_datadir}/selinux/packages/

%clean
rm -rf $RPM_BUILD_ROOT

%post
if [ "$1" -eq "1" ]; then
    /usr/sbin/semodule -i %{_datadir}/selinux/packages/%{name}.pp.bz2
    /sbin/restorecon -F -R -v %{_libdir}/nagios/plugins/
fi
if [ "$1" -eq "2" ]; then
    /usr/sbin/semodule -u %{_datadir}/selinux/packages/%{name}.pp.bz2
fi

%postun
if [ "$1" -eq "0" ]; then
    /usr/sbin/semodule -r %{name}
fi


%files
%defattr(-,root,root,-)
%doc %{_defaultdocdir}/%{name}
%{_datadir}/selinux/packages/

%files base
%defattr(-,root,root,-)
%{_libdir}/nagios/plugins/check_file
%{_libdir}/nagios/plugins/check_bonding
%{_libdir}/nagios/plugins/check_sockets
%{_libdir}/nagios/plugins/check_dummy
%{_libdir}/nagios/plugins/check_timeout

%if 0%{?rhel} >= 6
%files cups
%defattr(-,root,root,-)
%{_libdir}/nagios/plugins/check_cups_*
%endif

%files curl
%defattr(-,root,root,-)
%{_libdir}/nagios/plugins/check_aspsms_credits
%{_libdir}/nagios/plugins/check_tftp

%files dns
%defattr(-,root,root,-)
%{_libdir}/nagios/plugins/check_dns_*
%{_libdir}/nagios/plugins/check_dnssec_*

%if %{gnutls}
%files gnutls
%defattr(-,root,root,-)
%{_libdir}/nagios/plugins/check_ssl_cert
%endif

%files rhcs
%defattr(-,root,root,-)
%attr(4755, root, root) %{_libdir}/nagios/plugins/check_clustat
%{_libdir}/nagios/plugins/check_rhcsnmp

%files rpc
%defattr(-,root,root,-)
%{_libdir}/nagios/plugins/check_nfs
%{_libdir}/nagios/plugins/check_rpc_ping

%files selinux
%defattr(-,root,root,-)
%{_libdir}/nagios/plugins/check_enforce
%{_libdir}/nagios/plugins/check_sebool

%files snmp
%defattr(-,root,root,-)
%{_libdir}/nagios/plugins/check_arc_raid
%{_libdir}/nagios/plugins/check_apc_pdu
%{_libdir}/nagios/plugins/check_qnap_disks
%{_libdir}/nagios/plugins/check_qnap_vols

%files xmlrpc
%defattr(-,root,root,-)
%{_libdir}/nagios/plugins/check_rhn_entitlements
%{_libdir}/nagios/plugins/check_koji_builder
%{_libdir}/nagios/plugins/check_koji_hub

%changelog
* Mon Oct 31 2011 Marius Rieder <marius.rieder@durchmesser.ch> - 0.4-1
- version bump

* Mon Apr 25 2011 Marius Rieder <marius.rieder@durchmesser.ch> - 0.3-1
- version bump

* Sun Mar 20 2011 Marius Rieder <marius.rieder@durchmesser.ch> - 0.2-1
- version bump

* Sat Feb 26 2011 Marius Rieder <marius.rieder@durchmesser.ch> - 0.1-1
- First release
