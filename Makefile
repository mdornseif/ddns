CFLAGS=-g

defaut: ddnsd ddnsc ddnsd-data

ddnsd: ddnsd.o lib/tools.a djblib/djblib.a
	gcc -o $@ $^

ddnsc: ddnsc.o lib/tools.a djblib/djblib.a
	gcc -o $@ $^

ddnsd-data: ddnsd-data.o lib/tools.a djblib/djblib.a
	gcc -o $@ $^

djblib/djblib.a: 
	cd djblib
	make
	cd ..

lib/tools.a:
	cd lib
	make
	cd ..

clean:
	rm -f *.o *.a ddnsd ddnsd-data ddnsc

distclean: clean
	rm -f *~ *.cdb
	(cd djblib && make distclean)
	(cd lib && make distclean)
