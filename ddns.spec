Summary: ddns dynamic dns protocol client
Name: ddns-client
Version: alpha2
Release: 1
Packager: Doobee R. Tzeck <drt@ailis.de>
Source: http://rc23.cx/ddns/ddns-%{version}.tar.gz
Copyright: is myth
Group: Applications/Internet
#BuildRoot: /tmp/ddns-%{version}-buildroot
#Requires: openssl
#BuildPreReq: perl

%description 
This package includes the clients necessary to make encrypted connections
... bla

%changelog

%prep 

%setup -n ddns

%build
make

%install
#rm -rf $RPM_BUILD_ROOT
make setup-client

%clean
#rm -rf $RPM_BUILD_ROOT

%post

%preun 

%files
%defattr(-,root,root)
%doc OVERVIEW PROTO README
%attr(0755,root,root) /usr/local/bin/ddns-clientd
%attr(0644,root,root) /usr/local/man/man8/ddns-clientd.8

