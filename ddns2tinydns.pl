#!/usr/bin/perl
#
# $Id: ddns2tinydns.pl,v 1.2 2000/07/14 05:54:07 drt Exp $
#
# converts the data created by ddnsd to a tinydns-data compatible file
#              -- drt@ailis.de
#
# $Log: ddns2tinydns.pl,v $
# Revision 1.2  2000/07/14 05:54:07  drt
# *** empty log message ***
#
# Revision 1.1  2000/04/26 00:36:40  drt
# initial revision
#

$root = "root/dot";

# Here comes our authority data and so on
print <<EOD;
# autogenerated
.dyn.rc23.cx:62.159.58.131:gate-walledcity-rc23-cx
.rc23.cx:62.159.58.131:gate-walledcity-rc23-cx
=gate.walledcity.rc23.cx:62.159.58.131

# dynamic data follows
EOD

sub dodir 
  {
    chdir @_[0];
    opendir DIR, ".";
    
    while($_ = readdir(DIR))
      {
	if(!/^\./)
	  { 
	   $fqdn = $_ . "." . $_[1];
	    if(-d $_)
	      {
		dodir($_, $_ . "." . $_[1]);
	      }
	    else
	      {
		open FILE, $_ or die "can't open $_";
		$f = $_;
		while(<FILE>) 
		  {
		    if(s/^=(.*)$/\1/)
		      {
			@l = split(/:/);
			print "=" . $f . "." . "$_[1]:@l[0]\n";
		      }
		  }
		$fqdn = '';
		close FILE;
	      }
	  }
      }
    closedir DIR;
    chdir "..";
  }

&dodir($root, "");
