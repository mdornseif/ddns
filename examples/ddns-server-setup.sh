#! /bin/sh

# $Id: ddns-server-setup.sh,v 1.1 2000/10/06 22:03:15 drt Exp $
#
# This script will do all necessary setup to run a ddns server.

IP=127.0.0.5
# ip port to listen for ddns requests
DDNSDPORT=2353
DDNSDIP=$IP
# ip to answer dns requests on port 53 udp
FILEDNSIP=$IP
# ip to firewall dynamically
FWIP=127.0.0.2

# where to install
DESTDIR=/var/service
# which zone to serve
ZONE=d.example.com

# account for reading and writing
RWACCOUNT=ddnsrw
# account for reading only
ROACCOUNT=ddnsro
# $RWACCOUNT and $ROACCOUNT should the only be members of the same group

# ddns logging account
# !!! This should NOT be a member of $RWACCOUNT and $ROACCOUNT's group !!!
LOGACCOUNT=ddnsl

# something like this might create the needed users and group:
#
# $ groupadd ddns
# $ useradd ddnsrw -r -g ddns -d /var/service -s /bin/nologin
# $ useradd ddnsro -r -g ddns -d /var/service -s /bin/nologin

# create the firewall rules
# create chain ddns
ipchains -N ddns
# redirect all traffic for $FWIP to chain ddns
ipcahins -A input -s 0.0.0.0/0 -d $FWIP -j ddns
# traffic which wasn't handled inside chain ddns will be blocked
ipchains -A input -s 0.0.0.0/0 -d $FWIP -j REJECT

ddnsd-conf $RWACCOUNT $LOGACCOUNT $DESTDIR/ddnsd $DDNSDIP $DDNSDPORT $ZONE
filedns-conf $ROACCOUNT $LOGACCOUNT $DESTDIR/filedns $DESTDIR/ddnsd/root/dot $FILEDNSIP
ddns-cleand-conf $RWACCOUNT $LOGACCOUNT $DESTDIR/ddns-cleand $DESTDIR/ddnsd/root
ddns-ipfwo-conf $ROACCOUNT $LOGACCOUNT $DESTDIR/ddns-ipfwo $DESTDIR/ddnsd/root

# now just 
# $ cd $DESTDIR/ddnsd/root
# $ vi data
# $ make

# $ ln -s $DESTDIR/ddnsd /service
# $ ln -s $DESTDIR/filedns /service
# $ ln -s $DESTDIR/ddns-cleand /service
# $ ln -s $DESTDIR/ddnsd-aclwriter /service
