#!/usr/bin/perl
#
# $Id: settestdata.pl,v 1.1 2000/04/26 00:38:21 drt Exp $
#
# uses ddndc to set 10000 ip adresses on ddnsd
#              -- drt@ailis.de
#
# $Log: settestdata.pl,v $
# Revision 1.1  2000/04/26 00:38:21  drt
# initial revision
#

$host = "rc23.cx";
$numuids = 10000;

 $| = 1;

for( $i = 0 ; $i < $numuids ; $i++)
{
	print "$i:\t ";
	system("tcpclient $host 12345 ./ddnsc $i 127.0.0.1 s");
}
