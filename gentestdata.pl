#!/usr/bin/perl
#
# $Id: gentestdata.pl,v 1.1 2000/04/26 00:38:21 drt Exp $
#
# Generates 10000 test records for ddnsd-data
#              -- drt@ailis.de
#
# $Log: gentestdata.pl,v $
# Revision 1.1  2000/04/26 00:38:21  drt
# initial revision
#

$numuids = 10000;
$prefix = "test";

for( $i = 0 ; $i < $numuids ; $i++)
{
	print "$i:$prefix-$i:geheimgeheimgehe\n";
}
