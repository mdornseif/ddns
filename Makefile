DOWNLOADER = "wget"

CFLAGS=-g -Idnscache -Ilibtai

defaut: client daemon

daemon: libs ddnsd ddnsd-data filedns

client: libs ddnsc

ddnsd: ddnsd.o fmt_xint.o fmt_xlong.o open_excl.o now.o rijndael.o mt19937.o dnscache.a libtai.a 
	gcc -o $@ $^

ddnsc: ddnsc.o fmt_xint.o fmt_xlong.o rijndael.o mt19937.o dnscache.a libtai.a 
	gcc -o $@ $^

ddnsd-data: ddnsd-data.o buffer_0.o rijndael.o dnscache.a libtai.a
	gcc -o $@ $^

filedns: filedns.o server.o dnscache.a libtai.a
	gcc -o $@ $^

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
	rm -f *.o *.a ddnsd ddnsd-data ddnsc filedns

distclean: clean
	rm -f *~ *.cdb
	rm -Rf dnscache libtai
