DOWNLOADER = "wget"

CFLAGS=-g -Idnscache -Ilibtai -Idjblib  

defaut: ddnsd ddnsc ddnsd-data filedns

ddnsd: ddnsd.o tools.a dnscache.a libtai.a djblib/djblib.a
	gcc -o $@ $^

ddnsc: ddnsc.o tools.a dnscache.a libtai.a djblib/djblib.a
	gcc -o $@ $^

ddnsd-data: ddnsd-data.o tools.a dnscache.a libtai.a djblib/djblib.a
	gcc -o $@ $^

filedns: filedns.o dnscache/server.o dnscache/response.o dnscache/qlog.o dnscache/prot.o dnscache/dd.o libtai.a dnscache.a djblib/djblib.a
	gcc -o $@ $^

djblib/djblib.a: 
	cd djblib
	make
	cd ..

tools.a:
	cd lib; \
	make; \
	ar cr ../tools.a *.o; 

dnscache.a:
	if [ ! -d dnscache ]; then \
		$(DOWNLOADER) http://cr.yp.to/dnscache/dnscache-1.00.tar.gz; \
		tar xzvf dnscache-1.00.tar.gz; rm dnscache-1.00.tar.gz; \
		mv dnscache-1.00 dnscache; \
        fi;	
	cd dnscache; \
	make; \
	grep -l ^main *.c | perl -npe 's/^(.*).c/\1.o/;' | xargs rm -f; \
	ar cr ../dnscache.a *.o;

libtai.a:
	if [ ! -d libtai ]; then \
		$(DOWNLOADER) http://cr.yp.to/libtai/libtai-0.60.tar.gz; \
		tar xzvf libtai-0.60.tar.gz; rm libtai-0.60.tar.gz; \
		mv libtai-0.60 libtai;\
	fi	
	cd libtai; \
	make; \
	grep -l ^main *.c | perl -npe 's/^(.*).c/\1.o/;' | xargs rm -f; \
	ar cr ../libtai.a *.o; 	

libs:
	if [ ! -f djblib/djblib.a ]; then \
		cd djblib; \
		make; \
		cd ..; \
	fi;

clean:
	rm -f *.o *.a ddnsd ddnsd-data ddnsc filedns

realclean: clean
	cd dnscache; rm -f `cat TARGETS`
	cd libtai; rm -f `cat TARGETS`
	cd lib ; make clean

distclean: realclean
	rm -f *~ *.cdb
	cd lib ; make distclean