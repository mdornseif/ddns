# $Id: Makefile,v 1.14 2000/07/13 18:20:47 drt Exp $

DOWNLOADER = "wget"

CFLAGS=-g -Wall -Idnscache -Ilibtai

defaut: client

all: client daemon

daemon: libs ddnsd ddnsd-data ddns-cleand filedns

client: libs ddns-clientd

ddnsd: ddnsd.o fmt_xint.o fmt_xlong.o open_excl.o now.o rijndael.o \
iso2txt.o mt19937.o dnscache.a libtai.a 
	gcc -o $@ $^

ddns-cleand: ddns-cleand.o scan_xlong.o \
sig_alarm.o sig_block.o sig_catch.o now.o dnscache.a libtai.a 
	gcc -o $@ ddns-cleand.o scan_xlong.o sig_alarm.o sig_block.o sig_catch.o now.o dnscache.a libtai.a 

ddns-clientd: ddns-clientd.o ddnsc.o loc.o fmt_xint.o fmt_xlong.o rijndael.o mt19937.o pad.o txtparse.o dnscache.a libtai.a sig_int.o \
sig.o sig_catch.o sig_block.o sig_term.o sig_alarm.o fd_move.o fd_copy.o timeoutconn.o dnscache/ndelay_off.o
	gcc -o $@ $^

ddnsd-data: ddnsd-data.o buffer_0.o rijndael.o pad.o txtparse.o dnscache.a libtai.a
	gcc -o $@ $^

filedns: filedns.o server.o txtparse.o dnscache.a libtai.a
	gcc -o $@ $^

sig_block.o: sig_block.c hassgprm.h

sig_catch.o: sig_catch.c hassgact.h

hassgprm.h: dnscache.a
	make trysgprm
	./dnscache/choose cl trysgprm hassgprm.h1 hassgprm.h2 > hassgprm.h

hassgact.h: dnscache.a
	make trysgact
	./dnscache/choose cl trysgact hassgact.h1 hassgact.h2 > hassgact.h

trysgprm: trysgprm.o

trysgact: trysgact.o

libs: dnscache.a libtai.a

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

clean:
	rm -f *.o ddnsd ddnsd-data ddns-clientd filedns hassgprm.h hassgact.h *.a

distclean: clean
	rm -f *~ *.cdb core
	rm -Rf dnscache libtai 
