# $Id: Makefile,v 1.20 2000/07/19 22:13:28 drt Exp $
#  --drt@ailis.de

DOWNLOADER = "wget"

CFLAGS=-g -Wall -Idnscache -Ilibtai

defaut: client

all: client daemon

daemon: libs ddnsd ddnsd-data ddns-cleand filedns

client: libs ddns-clientd

ddnsd: ddns.h ddnsd.o libtai.a djblib.a drtlib.a dnscache.a
	gcc -o $@ ddnsd.o libtai.a djblib.a drtlib.a dnscache.a

ddns-cleand: ddns.h ddns-cleand.o libtai.a djblib.a drtlib.a dnscache.a 
	gcc -o $@ ddns-cleand.o libtai.a djblib.a drtlib.a dnscache.a 

ddns-clientd: ddns.h ddns-clientd.o ddnsc.o dnscache.a libtai.a \
drtlib.a djblib.a dnscache/ndelay_off.o
	gcc -o $@ ddns-clientd.o ddnsc.o dnscache.a libtai.a drtlib.a \
	djblib.a dnscache/ndelay_off.o

ddnsd-data: ddnsd-data.o buffer_0.o rijndael.o pad.o txtparse.o dnscache.a libtai.a
	gcc -o $@ ddnsd-data.o buffer_0.o rijndael.o pad.o txtparse.o dnscache.a libtai.a

filedns: ddns.h filedns.o server.o txtparse.o response.o qlog.o dd.o \
dnscache.a libtai.a djblib.a drtlib.a  
	gcc -o $@ filedns.o server.o txtparse.o response.o qlog.o dd.o \
        dnscache.a libtai.a djblib.a drtlib.a

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
sig_int.o sig_term.o socket_delay.o socket_local.o timeoutconn.o
	ar cr djblib.a buffer_0.o fd_copy.o fd_move.o fmt_xint.o \
	fmt_xlong.o now.o open_excl.o scan_xlong.o sig_alarm.o sig_block.o \
	sig_catch.o timeoutconn.o sig.o sig_int.o sig_term.o socket_delay.o \
	socket_local.o

drtlib.a: iso2txt.o loc.o mt19937.o pad.o rijndael.o txtparse.o droprootordie.o
	ar cr drtlib.a iso2txt.o loc.o mt19937.o pad.o rijndael.o txtparse.o droprootordie.o

setup-client: ddns-cleand
	install -Ds ddns-clientd /usr/local/bin/ddns-clientd
	install -D ddns-clientd.8 /usr/local/man/man8/ddns-clientd.8

setup-server:
	install -Ds ddnsd /usr/local/bin/ddnsd
	install -D ddnsd.8 /usr/local/man/man8/ddnsd.8
	install -Ds ddnsd-data /usr/local/bin/ddnsd-data
	install -D ddnsd-data.8 /usr/local/man/man8/ddnsd-data.8
	install -Ds filedns /usr/local/bin/filedns
	install -D filedns.8 /usr/local/man/man8/filedns.8
	install -Ds ddnsd-cleand /usr/local/bin/ddnsd-cleand
	install -D ddnsd-cleand.8 /usr/local/man/man8/ddns-cleand.8

clean:
	rm -f *.o ddnsd ddnsd-data ddns-clientd filedns hassgprm.h hassgact.h *.a

distclean: clean
	rm -f *~ *.cdb core
	rm -Rf dnscache libtai 
