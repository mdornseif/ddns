# $Id: Makefile,v 1.21 2000/07/29 21:42:56 drt Exp $
#  --drt@ailis.de

DOWNLOADER = "wget"

CFLAGS=-g -Wall -Idnscache -Ilibtai
LDFLAGS=-g

defaut: client

all: client daemon

daemon: libs ddnsd ddnsd-data ddnsd-conf ddns-cleand ddns-cleand-conf filedns filedns-conf ddnsd-aclwriter ddnsd-aclwriter-conf

client: libs ddns-clientd

ddns-clientd: ddns.h ddns-clientd.o ddnsc.o ddns_pack.o dnscache.a libtai.a \
drtlib.a djblib.a dnscache/ndelay_off.o
	gcc -o $@ ddns-clientd.o ddnsc.o ddns_pack.o dnscache.a libtai.a drtlib.a djblib.a dnscache/ndelay_off.o

ddnsd: ddns.h ddnsd.o ddns_pack.o libtai.a djblib.a drtlib.a dnscache.a
	gcc -o $@ ddnsd.o ddns_pack.o libtai.a djblib.a drtlib.a dnscache.a

ddnsd-conf: ddnsd-conf.o
	$(CC) $(CFLAGS) -o ddnsd-conf ddnsd-conf.o dnscache.a

ddnsd-data: ddnsd-data.o drtlib.a djblib.a dnscache.a libtai.a
	gcc -o $@ ddnsd-data.o drtlib.a djblib.a dnscache.a libtai.a

ddns-cleand: ddns.h ddns-cleand.o libtai.a djblib.a drtlib.a dnscache.a 
	gcc -o $@ ddns-cleand.o  libtai.a djblib.a drtlib.a dnscache.a 

ddns-cleand-conf: ddns-cleand-conf.o
	$(CC) $(CFLAGS) -o ddns-cleand-conf ddns-cleand-conf.o dnscache.a

filedns: ddns.h filedns.o ddns_parseline.o server.o response.o qlog.o dd.o \
drtlib.a dnscache.a libtai.a djblib.a 
	gcc -o $@ filedns.o ddns_parseline.o server.o response.o qlog.o dd.o \
        drtlib.a dnscache.a libtai.a djblib.a 

filedns-conf: filedns-conf.o  dnscache.a
	$(CC) $(CFLAGS) -o filedns-conf filedns-conf.o dnscache.a

ddnsd-aclwriter: ddns.h ddnsd-aclwriter.o dAVLTree.o ddns_parseline.o drtlib.a libtai.a djblib.a drtlib.a dnscache.a
	gcc -o $@ ddnsd-aclwriter.o dAVLTree.o ddns_parseline.o drtlib.a libtai.a djblib.a dnscache.a

ddnsd-aclwriter-conf: ddnsd-aclwriter-conf.o
	$(CC) $(CFLAGS) -o ddnsd-aclwriter-conf ddnsd-aclwriter-conf.o dnscache.a

sig_block.o: sig_block.c hassgprm.h

sig_catch.o: sig_catch.c hassgact.h

fifo.o: fifo.c hasmkffo.h fifo.h
	$(CC) $(CFLAGS) -c -o fifo.o fifo.c

hassgprm.h: dnscache.a
	make trysgprm
	./dnscache/choose cl trysgprm hassgprm.h1 hassgprm.h2 > hassgprm.h

hassgact.h: dnscache.a
	make trysgact
	./dnscache/choose cl trysgact hassgact.h1 hassgact.h2 > hassgact.h

trysgprm: trysgprm.o

trysgact: trysgact.o

hasmkffo.h: dnscache.a
	make trymkffo
	./dnscache/choose cl trymkffo hasmkffo.h1 hasmkffo.h2 > hasmkffo.h

trymkffo: trymkffo.o

# this is broken
.8.@html@:
	groff -Thtml $< $@ 

libs: dnscache.a libtai.a drtlib.a djblib.a

dnscache.a:
	if [ ! -d dnscache ]; then \
		$(DOWNLOADER) http://cr.yp.to/dnscache/dnscache-1.00.tar.gz; \
		$(DOWNLOADER) http://www.fefe.de/dns/dnscache-1.00-ipv6.diff5; \
		tar xzvf dnscache-1.00.tar.gz; rm dnscache-1.00.tar.gz; \
		mv dnscache-1.00 dnscache; \
		cd dnscache; patch < ../dnscache-1.00-ipv6.diff5 ; cd ..; \
		rm dnscache-1.00-ipv6.diff5; \
        fi;	
	cd dnscache; \
	make; \
	grep -l ^main *.c | perl -npe 's/^(.*).c/\1.o/;' | xargs rm -fv; \
	ar cr ../dnscache.a *.o;

libtai.a:
	if [ ! -d libtai ]; then \
		$(DOWNLOADER) http://cr.yp.to/libtai/libtai-0.60.tar.gz; \
		tar xzvf libtai-0.60.tar.gz; rm libtai-0.60.tar.gz; \
		mv libtai-0.60 libtai;\
	fi	
	cd libtai; \
	make; \
	grep -l ^main *.c | perl -npe 's/^(.*).c/\1.o/;' | xargs rm -fv; \
	ar cr ../libtai.a *.o; 	

djblib.a: buffer_0.o fd.h fd_copy.o fd_move.o fmt_xint.o fmt_xlong.o \
now.o open_excl.o scan_xlong.o sig.o sig_alarm.o sig_block.o sig_catch.o \
sig_int.o sig_term.o socket_delay.o socket_local.o timeoutconn.o fifo.o coe.o \
open_write.o 
	ar cr djblib.a buffer_0.o fd_copy.o fd_move.o fmt_xint.o \
	fmt_xlong.o now.o open_excl.o scan_xlong.o sig_alarm.o sig_block.o \
	sig_catch.o timeoutconn.o sig.o sig_int.o sig_term.o socket_delay.o \
	socket_local.o fifo.o coe.o open_write.o

drtlib.a: iso2txt.o loc.o mt19937.o pad.o rijndael.o txtparse.o droprootordie.o fieldsep.o stralloc_cleanlineend.o traversedirhier.o
	ar cr drtlib.a iso2txt.o loc.o mt19937.o pad.o rijndael.o txtparse.o droprootordie.o fieldsep.o stralloc_cleanlineend.o traversedirhier.o

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
	install -Ds ddnsd-aclwriter /usr/local/bin/ddnsd-aclwriter
	install -cD ddnsd-aclwriter.8 /usr/local/man/man8/ddnsd-aclwriter.8
	install -Ds ddnsd-aclwriter-conf /usr/local/bin/ddnsd-aclwriter-conf
	install -cD ddnsd-aclwriter-conf.8 /usr/local/man/man8/ddnsd-aclwriter-conf.8

clean:
	rm -f *.o ddnsd dnsd-conf ddnsd-data ddnsd-data-conf filedns filedns-conf ddns-cleand ddns-cleand-conf ddns-clientd ddnsd-aclwriter \
hassgprm.h hassgact.h hasmkffo.h *.a

distclean: clean
	rm -f *~ *.cdb core
	rm -Rf dnscache libtai
	rm -f *.html 
