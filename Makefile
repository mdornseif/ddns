DOWNLOADER = "wget"

CFLAGS=-g -Wall -Idnscache -Ilibtai

defaut: client daemon

daemon: libs ddnsd ddnsd-data ddns-cleand filedns

client: libs ddnsc

ddnsd: ddnsd.o fmt_xint.o fmt_xlong.o open_excl.o now.o rijndael.o mt19937.o dnscache.a libtai.a 
	gcc -o $@ $^

ddns-cleand: ddns-cleand.o scan_xlong.o \
sig_alarm.o sig_block.o sig_catch.o now.o dnscache.a libtai.a 
	gcc -o $@ ddns-cleand.o scan_xlong.o sig_alarm.o sig_block.o sig_catch.o now.o dnscache.a libtai.a 

ddnsc: ddnsc.o fmt_xint.o fmt_xlong.o rijndael.o mt19937.o pad.o txtparse.o dnscache.a libtai.a 
	gcc -o $@ $^

ddnsd-data: ddnsd-data.o buffer_0.o rijndael.o pad.o txtparse.o dnscache.a libtai.a
	gcc -o $@ $^

filedns: filedns.o server.o dnscache.a libtai.a
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
		tar xzvf dnscache-1.00.tar.gz; rm dnscache-1.00.tar.gz; \
		mv dnscache-1.00 dnscache; \
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
	rm -f *.o ddnsd ddnsd-data ddnsc filedns hassgprm.h hassgact.h

distclean: clean
	rm -f *~ *.cdb core
	rm -Rf dnscache libtai
