# $Id: Makefile,v 1.27 2000/11/21 21:29:48 drt Exp $
#  --drt@ailis.de

VERSION=\"preAlpha4\"

CFLAGS=-g -Wall -Idjblib -DVERSION=$(VERSION)
LDFLAGS=-g

default: client

all: client daemon

daemon: ddnsd ddnsd-data ddnsd-conf \
        ddns-cleand ddns-cleand-conf \
        filedns filedns-conf \
        ddns-snapd ddns-snapd-conf \
        ddns-ipfwo ddns-ipfwo-conf \
	html

client: ddns-clientd ddns-clientd.html

html: ddnsd.html ddnsd-conf.html ddnsd-data.html \
      ddns-cleand.html ddns-cleand-conf.html \
      filedns.html filedns-conf.html \
      ddns-clientd.html ddns-file.html    

ddns-clientd: djblib.a ddns.h ddns-clientd.o ddnsc.o ddns_pack.o drtlib.a
	gcc -o $@ ddns-clientd.o ddnsc.o ddns_pack.o drtlib.a djblib.a

ddnsd: djblib.a ddns.h ddnsd.h ddnsd.o ddnsd_setentry.o ddnsd_renewentry.o \
       ddnsd_killentry.o ddns_pack.o ddnsd_net.o ddnsd_log.o drtlib.a
	$(CC) $(CFLAGS) -o $@ ddnsd.o ddnsd_setentry.o ddnsd_renewentry.o ddnsd_killentry.o \
	ddns_pack.o ddnsd_net.o ddnsd_log.o drtlib.a  djblib.a

ddnsd-conf: ddnsd-conf.o generic-conf.o auto_home.o djblib.a
	$(CC) $(CFLAGS) -o $@ ddnsd-conf.o generic-conf.o auto_home.o djblib.a

ddnsd-data: djblib.a ddnsd-data.o drtlib.a  
	$(CC) $(CFLAGS) -o $@ ddnsd-data.o drtlib.a djblib.a 

ddns-cleand: djblib.a ddns.h ddns-cleand.o djblib.a drtlib.a ddnsd_fifo.c
	$(CC) $(CFLAGS) -o $@ ddns-cleand.o drtlib.a djblib.a  

ddns-cleand-conf: djblib.a ddns-cleand-conf.o generic-conf.o auto_home.o
	$(CC) $(CFLAGS) -o $@ ddns-cleand-conf.o generic-conf.o auto_home.o djblib.a

filedns: djblib.a ddns.h filedns.o ddns_parseline.o server.o response.o qlog.o dd.o \
drtlib.a 
	$(CC) $(CFLAGS) -o $@ filedns.o ddns_parseline.o server.o response.o qlog.o dd.o \
        drtlib.a djblib.a 

filedns-conf: djblib.a  filedns-conf.o generic-conf.o auto_home.o
	$(CC) $(CFLAGS) -o $@ filedns-conf.o generic-conf.o auto_home.o djblib.a

ddns-snapd: djblib.a ddns.h ddns-snapd.o ddns_snapdump.o dAVLTree.o \
	ddns_parseline.o write_fifodir.o drtlib.a drtlib.a
	$(CC) $(CFLAGS) -o $@ ddns-snapd.o ddns_snapdump.o dAVLTree.o ddns_parseline.o write_fifodir.o drtlib.a djblib.a 

ddns-snapd-conf: djblib.a ddns-snapd-conf.o generic-conf.o auto_home.o
	$(CC) $(CFLAGS) -o $@ ddns-snapd-conf.o generic-conf.o auto_home.o djblib.a

ddns-ipfwo: djblib.a ddns-ipfwo.o ddns-ipfwo.linux.o ddns_parseline.o drtlib.a djblib.a drtlib.a 
	$(CC) $(CFLAGS) -o $@ ddns-ipfwo.o ddns-ipfwo.linux.o ddns_parseline.o drtlib.a djblib.a drtlib.a 

ddns-ipfwo-conf: djblib.a ddns-ipfwo-conf.o generic-conf.o auto_home.o
	$(CC) $(CFLAGS) -o $@ ddns-ipfwo-conf.o generic-conf.o auto_home.o djblib.a

%.html: %.8
	groff -Thtml -mandoc $< > $@ 

%.html: %.5
	groff -Thtml -mandoc $< > $@ 

libs: drtlib.a djblib.a

djblib.a: djblib/*
	cd djblib; make
	cp djblib/djblib.a .


drtlib.a: iso2txt.o loc.o loc_pack.o mt19937.o pad.o rijndael.o \
txtparse.o droprootordie.o fieldsep.o stralloc_cleanlineend.o \
traversedirhier.o stralloc_free.o write_fifodir.o droproot.o open_excl.o
	ar cr drtlib.a iso2txt.o loc.o loc_pack.o mt19937.o pad.o rijndael.o \
	txtparse.o droprootordie.o fieldsep.o stralloc_cleanlineend.o \
	traversedirhier.o stralloc_free.o write_fifodir.o droproot.o open_excl.o

setup-all: setup-client setup-server

setup-client: client
	install -Ds ddns-clientd /usr/local/bin/ddns-clientd
	install -D ddns-clientd.8 /usr/local/man/man8/ddns-clientd.8

setup-server: daemon
	install -Ds ddnsd /usr/local/bin/ddnsd
	install -cD ddnsd.8 /usr/local/man/man8/ddnsd.8
	install -Ds ddnsd-data /usr/local/bin/ddnsd-data
	install -cD ddnsd-data.8 /usr/local/man/man8/ddnsd-data.8
	install -Ds ddnsd-conf /usr/local/bin/ddnsd-conf
	install -cD ddnsd-conf.8 /usr/local/man/man8/ddnsd-conf.8
	install -Ds filedns /usr/local/bin/filedns
	install -cD filedns.8 /usr/local/man/man8/filedns.8
	install -Ds filedns-conf /usr/local/bin/filedns-conf
	install -cD filedns-conf.8 /usr/local/man/man8/filedns-conf.8
	install -Ds ddns-cleand /usr/local/bin/ddns-cleand
	install -cD ddns-cleand.8 /usr/local/man/man8/ddns-cleand.8
	install -Ds ddns-cleand-conf /usr/local/bin/ddns-cleand-conf
	install -cD ddns-cleand-conf.8 /usr/local/man/man8/ddns-cleand-conf.8
	install -Ds ddns-snapd /usr/local/bin/ddns-snapd
#	install -cD ddns-snapd.8 /usr/local/man/man8/ddnsd-snapd.8
	install -Ds ddns-snapd-conf /usr/local/bin/ddns-snapd-conf
#	install -cD ddns-snapd-conf.8 /usr/local/man/man8/ddnsd-snapd-conf.8
	install -Ds ddns-ipfwo /usr/local/bin/ddns-ipfwo
#	install -cD ddns-ipfwo.8 /usr/local/man/man8/ddnsd-ipfwo.8
	install -Ds ddns-ipfwo-conf /usr/local/bin/ddns-ipfwo-conf
#	install -cD ddns-ipfwo-conf.8 /usr/local/man/man8/ddnsd-ipfwo-conf.8

clean:
	rm -f *.o ddnsd dnsd-conf ddnsd-data ddnsd-data-conf filedns filedns-conf \
ddns-cleand ddns-cleand-conf ddns-clientd ddnsd-snapd ddns-snapd-conf ddns-ipfwo.linux \
hassgprm.h hassgact.h hasmkffo.h *.a
	cd djblib; make clean

distclean: clean
	rm -f *~ *.cdb core
	rm -f *.html 


html: ddns-cleand-conf.html ddns-cleand.html ddns-file.html ddns-ipfwo-conf.html ddns-ipfwo.linux.html ddns-pipe.html ddnsd-conf.html ddnsd-data.html ddnsd.html filedns-conf.html filedns.html
