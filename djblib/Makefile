all: byte.a tai.a djblib.a

clean:
	rm -f `cat TARGETS`

djblib.a: alloc.o alloc_re.o buffer.o buffer_0.o buffer_1.o buffer_2.o \
buffer_copy.o buffer_get.o buffer_put.o byte_chr.o byte_copy.o \
byte_cr.o byte_diff.o byte_dup.o byte_zero.o case_diffb.o case_diffs.o \
case_diffs.o case_lowerb.o case_lowers.o case_starts.o env.o error.o \
error_str.o fd_copy.o fd_move.o fifo.o fmt_str.o fmt_strn.o fmt_uint.o \
fmt_uint0.o fmt_ulong.o fmt_xlong.o getln.o getln2.o iopause.o \
ip4_fmt.o ip4_scan.o ip6_fmt.o ip6_scan.o ndelay_off.o ndelay_on.o \
open_append.o open_read.o open_trunc.o open_write.o openreadclose.o \
pathexec_env.o pathexec_run.o readclose.o scan_0x.o scan_8long.o \
scan_long.o scan_sign.o scan_ulong.o seek_set.o sig.o sig_block.o \
sig_catch.o sig_pause.o socket_accept.o socket_accept6.o socket_bind.o \
socket_bind6.o socket_broadcast.o socket_conn.o socket_conn6.o \
socket_delay.o socket_listen.o socket_local.o socket_local6.o \
socket_mcjoin4.o socket_mcjoin6.o socket_mcleave4.o socket_mcleave6.o \
socket_mcloop4.o socket_mcloop6.o socket_mcttl4.o socket_mcttl6.o \
socket_opts.o socket_recv.o socket_recv6.o socket_remote.o \
socket_remote6.o socket_send.o socket_send6.o socket_tcp.o \
socket_tcp6.o socket_udp.o socket_udp6.o str_chr.o str_cpy.o \
str_diff.o str_diffn.o str_len.o str_rchr.o str_start.o stralloc_cat.o \
stralloc_catb.o stralloc_cats.o stralloc_copy.o stralloc_eady.o \
stralloc_free.o stralloc_num.o stralloc_opyb.o stralloc_opys.o \
stralloc_pend.o strerr_die.o strerr_sys.o tai_add.o tai_now.o \
tai_pack.o tai_sub.o tai_uint.o tai_unpack.o taia_add.o taia_approx.o \
taia_frac.o taia_less.o taia_now.o taia_pack.o taia_sub.o taia_tai.o \
taia_uint.o timeoutaccept.o timeoutaccept6.o timeoutconn.o \
timeoutconn6.o timeoutread.o timeoutwrite.o uint16_pack.o \
uint16_unpack.o uint32_pack.o uint32_unpack.o wait_nohang.o wait_pid.o \
dns_dfd.o dns_domain.o dns_ip.o dns_ipq.o \
dns_name.o dns_nd.o dns_packet.o dns_random.o dns_rcip.o dns_rcrw.o \
dns_resolve.o dns_sortip.o dns_transmit.o dns_txt.o dns_ip6.o \
dns_sortip6.o dns_nd6.o dns_ipq6.o prot.o taia_unpack.o \
cdb.o cdb_hash.o cdb_make.o now.o datetime.o coe.o
	./makelib djblib.a alloc.o alloc_re.o buffer.o buffer_0.o buffer_1.o buffer_2.o \
	buffer_copy.o buffer_get.o buffer_put.o byte_chr.o byte_copy.o \
	byte_cr.o byte_diff.o byte_dup.o byte_zero.o case_diffb.o case_diffs.o \
	case_diffs.o case_lowerb.o case_lowers.o case_starts.o env.o error.o \
	error_str.o fd_copy.o fd_move.o fifo.o fmt_str.o fmt_strn.o fmt_uint.o \
	fmt_uint0.o fmt_ulong.o fmt_xlong.o getln.o getln2.o iopause.o \
	ip4_fmt.o ip4_scan.o ip6_fmt.o ip6_scan.o ndelay_off.o ndelay_on.o \
	open_append.o open_read.o open_trunc.o open_write.o openreadclose.o \
	pathexec_env.o pathexec_run.o readclose.o scan_0x.o scan_8long.o \
	scan_long.o scan_sign.o scan_ulong.o seek_set.o sig.o sig_block.o \
	sig_catch.o sig_pause.o socket_accept.o socket_accept6.o socket_bind.o \
	socket_bind6.o socket_broadcast.o socket_conn.o socket_conn6.o \
	socket_delay.o socket_listen.o socket_local.o socket_local6.o \
	socket_mcjoin4.o socket_mcjoin6.o socket_mcleave4.o socket_mcleave6.o \
	socket_mcloop4.o socket_mcloop6.o socket_mcttl4.o socket_mcttl6.o \
	socket_opts.o socket_recv.o socket_recv6.o socket_remote.o \
	socket_remote6.o socket_send.o socket_send6.o socket_tcp.o \
	socket_tcp6.o socket_udp.o socket_udp6.o str_chr.o str_cpy.o \
	str_diff.o str_diffn.o str_len.o str_rchr.o str_start.o stralloc_cat.o \
	stralloc_catb.o stralloc_cats.o stralloc_copy.o stralloc_eady.o \
	stralloc_free.o stralloc_num.o stralloc_opyb.o stralloc_opys.o \
	stralloc_pend.o strerr_die.o strerr_sys.o tai_add.o tai_now.o \
	tai_pack.o tai_sub.o tai_uint.o tai_unpack.o taia_add.o taia_approx.o \
	taia_frac.o taia_less.o taia_now.o taia_pack.o taia_sub.o taia_tai.o \
	taia_uint.o timeoutaccept.o timeoutaccept6.o timeoutconn.o \
	timeoutconn6.o timeoutread.o timeoutwrite.o uint16_pack.o \
	uint16_unpack.o uint32_pack.o uint32_unpack.o wait_nohang.o wait_pid.o \
	dns_dfd.o dns_domain.o dns_ip.o dns_ipq.o \
	dns_name.o dns_nd.o dns_packet.o dns_random.o dns_rcip.o dns_rcrw.o \
	dns_resolve.o dns_sortip.o dns_transmit.o dns_txt.o dns_ip6.o \
	dns_sortip6.o dns_nd6.o dns_ipq6.o prot.o taia_unpack.o \
    cdb.o cdb_hash.o cdb_make.o now.o datetime.o coe.o


prot.o: \
compile prot.c hasshsgr.h prot.h
	./compile prot.c

hasshsgr.h: \
choose compile load tryshsgr.c hasshsgr.h1 hasshsgr.h2 chkshsgr \
warn-shsgr
	./chkshsgr || ( cat warn-shsgr; exit 1 )
	./choose clr tryshsgr hasshsgr.h1 hasshsgr.h2 > hasshsgr.hhasshsgr.h: \

chkshsgr: \
load chkshsgr.o
	./load chkshsgr 

chkshsgr.o: \
compile chkshsgr.c exit.h
	./compile chkshsgr.c

taia_unpack.o: \
compile taia_unpack.c taia.h tai.h uint64.h
	./compile taia_unpack.c

cdb.o: \
compile cdb.c readwrite.h error.h seek.h byte.h cdb.h uint32.h
	./compile cdb.c

cdb_hash.o: \
compile cdb_hash.c cdb.h uint32.h
	./compile cdb_hash.c

cdb_make.o: \
compile cdb_make.c readwrite.h seek.h error.h alloc.h cdb.h uint32.h \
cdb_make.h buffer.h uint32.h
	./compile cdb_make.c

makelib: \
warn-auto.sh systype
	( cat warn-auto.sh; \
	echo 'main="$$1"; shift'; \
	echo 'rm -f "$$main"'; \
	echo 'ar cr "$$main" $${1+"$$@"}'; \
	case "`cat systype`" in \
	sunos-5.*) ;; \
	unix_sv*) ;; \
	irix64-*) ;; \
	irix-*) ;; \
	dgux-*) ;; \
	hp-ux-*) ;; \
	sco*) ;; \
	*) echo 'ranlib "$$main"' ;; \
	esac \
	) > makelib
	chmod 755 makelib

systype: \
find-systype.sh conf-cc conf-ld trycpp.c x86cpuid.c
	( cat warn-auto.sh; \
	echo CC=\'`head -1 conf-cc`\'; \
	echo LD=\'`head -1 conf-ld`\'; \
	cat find-systype.sh; \
	) | sh > systype

compile: \
warn-auto.sh conf-cc
	( cat warn-auto.sh; \
	echo exec "`head -1 conf-cc`" '-c $${1+"$$@"}' \
	) > compile
	chmod 755 compile

iopause.h: \
choose compile trypoll.c iopause.h1 iopause.h2
	./choose clr trypoll iopause.h1 iopause.h2 > iopause.h

alloc.a: \
makelib alloc.o alloc_re.o getln.o getln2.o stralloc_cat.o \
stralloc_catb.o stralloc_cats.o stralloc_copy.o stralloc_eady.o \
stralloc_num.o stralloc_opyb.o stralloc_opys.o stralloc_pend.o \
stralloc_free.o byte_dup.o
	./makelib alloc.a alloc.o alloc_re.o getln.o getln2.o \
	stralloc_cat.o stralloc_catb.o stralloc_cats.o \
	stralloc_copy.o stralloc_eady.o stralloc_num.o \
	stralloc_opyb.o stralloc_opys.o stralloc_pend.o \
	stralloc_free.o byte_dup.o 

alloc.o: \
compile alloc.c alloc.h error.h
	./compile alloc.c

alloc_re.o: \
compile alloc_re.c alloc.h byte.h
	./compile alloc_re.c

getln.o: \
compile getln.c byte.h getln.h buffer.h stralloc.h gen_alloc.h
	./compile getln.c

getln2.o: \
compile getln2.c byte.h getln.h buffer.h stralloc.h gen_alloc.h
	./compile getln2.c

stralloc_cat.o: \
compile stralloc_cat.c byte.h stralloc.h gen_alloc.h
	./compile stralloc_cat.c

stralloc_catb.o: \
compile stralloc_catb.c stralloc.h gen_alloc.h byte.h
	./compile stralloc_catb.c

stralloc_cats.o: \
compile stralloc_cats.c byte.h str.h stralloc.h gen_alloc.h
	./compile stralloc_cats.c

stralloc_copy.o: \
compile stralloc_copy.c byte.h stralloc.h gen_alloc.h
	./compile stralloc_copy.c

stralloc_eady.o: \
compile stralloc_eady.c alloc.h stralloc.h gen_alloc.h \
gen_allocdefs.h
	./compile stralloc_eady.c

stralloc_num.o: \
compile stralloc_num.c stralloc.h gen_alloc.h
	./compile stralloc_num.c

stralloc_opyb.o: \
compile stralloc_opyb.c stralloc.h gen_alloc.h byte.h
	./compile stralloc_opyb.c

stralloc_opys.o: \
compile stralloc_opys.c byte.h str.h stralloc.h gen_alloc.h
	./compile stralloc_opys.c

stralloc_pend.o: \
compile stralloc_pend.c alloc.h stralloc.h gen_alloc.h \
gen_allocdefs.h
	./compile stralloc_pend.c

stralloc_free.o: \
compile stralloc_free.c alloc.h stralloc.h gen_alloc.h \
gen_allocdefs.h
	./compile stralloc_free.c

byte_dup.o: compile byte_dup.c byte.h
	./compile byte_dup.c

buffer.a: \
makelib buffer.o buffer_0.o buffer_1.o buffer_2.o buffer_copy.o \
buffer_get.o buffer_put.o strerr_die.o strerr_sys.o
	./makelib buffer.a buffer.o buffer_0.o buffer_1.o buffer_2.o \
	buffer_copy.o buffer_get.o buffer_put.o strerr_die.o \
	strerr_sys.o

buffer.o: \
compile buffer.c buffer.h
	./compile buffer.c

buffer_0.o: \
compile buffer_0.c readwrite.h buffer.h
	./compile buffer_0.c

buffer_1.o: \
compile buffer_1.c readwrite.h buffer.h
	./compile buffer_1.c

buffer_2.o: \
compile buffer_2.c readwrite.h buffer.h
	./compile buffer_2.c

buffer_copy.o: \
compile buffer_copy.c buffer.h
	./compile buffer_copy.c

buffer_get.o: \
compile buffer_get.c buffer.h byte.h error.h
	./compile buffer_get.c

buffer_put.o: \
compile buffer_put.c buffer.h str.h byte.h error.h
	./compile buffer_put.c

strerr_die.o: \
compile strerr_die.c buffer.h exit.h strerr.h
	./compile strerr_die.c

strerr_sys.o: \
compile strerr_sys.c error.h strerr.h
	./compile strerr_sys.c

byte.a: \
makelib byte_chr.o byte_copy.o byte_cr.o byte_diff.o byte_zero.o \
case_diffb.o case_diffs.o case_lowerb.o case_lowers.o case_starts.o \
case_diffs.o fmt_ulong.o ip4_fmt.o ip4_scan.o scan_ulong.o \
str_chr.o str_diff.o str_len.o str_rchr.o str_start.o uint16_pack.o \
uint16_unpack.o uint32_pack.o uint32_unpack.o ip6_fmt.o ip6_scan.o \
scan_0x.o fmt_xlong.o str_cpy.o str_diffn.o scan_8long.o fmt_str.o \
fmt_uint.o fmt_uint0.o scan_sign.o scan_long.o fmt_strn.o
	./makelib byte.a byte_chr.o byte_copy.o byte_cr.o \
	byte_diff.o byte_zero.o case_diffb.o case_diffs.o \
	case_lowerb.o case_lowers.o case_starts.o case_diffs.o \
	fmt_ulong.o ip4_fmt.o ip4_scan.o scan_ulong.o str_chr.o str_diff.o \
	str_len.o str_rchr.o str_start.o uint16_pack.o uint16_unpack.o \
	uint32_pack.o uint32_unpack.o ip6_fmt.o ip6_scan.o scan_0x.o \
	fmt_xlong.o str_cpy.o str_diffn.o scan_8long.o fmt_str.o \
	fmt_uint.o fmt_uint0.o scan_sign.o scan_long.o fmt_strn.o

byte_chr.o: \
compile byte_chr.c byte.h
	./compile byte_chr.c

byte_rchr.o: \
compile byte_rchr.c byte.h
	./compile byte_rchr.c

byte_copy.o: \
compile byte_copy.c byte.h
	./compile byte_copy.c

byte_cr.o: \
compile byte_cr.c byte.h
	./compile byte_cr.c

byte_diff.o: \
compile byte_diff.c byte.h
	./compile byte_diff.c

byte_zero.o: \
compile byte_zero.c byte.h
	./compile byte_zero.c

case_starts.o: \
compile case_starts.c case.h
	./compile case_starts.c

case_diffb.o: \
compile case_diffb.c case.h
	./compile case_diffb.c

case_diffs.o: \
compile case_diffs.c case.h
	./compile case_diffs.c

case_lowerb.o: \
compile case_lowerb.c case.h
	./compile case_lowerb.c

case_lowers.o: \
compile case_lowers.c case.h
	./compile case_lowers.c

fmt_ulong.o: \
compile fmt_ulong.c fmt.h
	./compile fmt_ulong.c

fmt_xlong.o: \
compile fmt_xlong.c scan.h
	./compile fmt_xlong.c

fmt_str.o: \
compile fmt_str.c scan.h
	./compile fmt_str.c

fmt_strn.o: \
compile fmt_strn.c scan.h
	./compile fmt_strn.c

fmt_uint.o: \
compile fmt_uint.c scan.h
	./compile fmt_uint.c

fmt_uint0.o: \
compile fmt_uint0.c scan.h
	./compile fmt_uint0.c

ip4_fmt.o: \
compile ip4_fmt.c fmt.h ip4.h
	./compile ip4_fmt.c

ip6_fmt.o: \
compile ip6_fmt.c fmt.h ip6.h ip4.h
	./compile ip6_fmt.c

ip4_scan.o: \
compile ip4_scan.c scan.h ip4.h
	./compile ip4_scan.c

ip6_scan.o: \
compile ip6_scan.c scan.h ip6.h
	./compile ip6_scan.c

scan_ulong.o: \
compile scan_ulong.c scan.h
	./compile scan_ulong.c

scan_long.o: \
compile scan_long.c scan.h
	./compile scan_long.c

scan_8long.o: \
compile scan_8long.c scan.h
	./compile scan_8long.c

scan_0x.o: \
compile scan_0x.c scan.h
	./compile scan_0x.c

scan_sign.o: \
compile scan_sign.c scan.h
	./compile scan_sign.c

str_chr.o: \
compile str_chr.c str.h
	./compile str_chr.c

str_cpy.o: \
compile str_cpy.c str.h
	./compile str_cpy.c

str_diff.o: \
compile str_diff.c str.h
	./compile str_diff.c

str_diffn.o: \
compile str_diffn.c str.h
	./compile str_diffn.c

str_len.o: \
compile str_len.c str.h
	./compile str_len.c

str_rchr.o: \
compile str_rchr.c str.h
	./compile str_rchr.c

str_start.o: \
compile str_start.c str.h
	./compile str_start.c

uint16_pack.o: \
compile uint16_pack.c uint16.h
	./compile uint16_pack.c

uint16_unpack.o: \
compile uint16_unpack.c uint16.h
	./compile uint16_unpack.c

uint32_pack.o: \
compile uint32_pack.c uint32.h
	./compile uint32_pack.c

uint32_unpack.o: \
compile uint32_unpack.c uint32.h
	./compile uint32_unpack.c

uint32.h: \
tryulong32.c compile load uint32.h1 uint32.h2
	( ( ./compile tryulong32.c && ./load tryulong32 && \
	./tryulong32 ) >/dev/null 2>&1 \
	&& cat uint32.h2 || cat uint32.h1 ) > uint32.h
	rm -f tryulong32.o tryulong32

uint64.h: \
choose compile load tryulong64.c uint64.h1 uint64.h2
	./choose clr tryulong64 uint64.h1 uint64.h2 > uint64.h

sig.o: \
compile sig.c sig.h
	./compile sig.c

sig_block.o: \
compile sig_block.c sig.h hassgprm.h
	./compile sig_block.c

sig_catch.o: \
compile sig_catch.c sig.h hassgact.h
	./compile sig_catch.c

sig_pause.o: \
compile sig_pause.c sig.h hassgprm.h
	./compile sig_pause.c

sig.a: \
makelib sig.o sig_block.o sig_catch.o sig_pause.o
	./makelib sig.a sig.o sig_block.o sig_catch.o sig_pause.o

hassgact.h: \
choose compile load trysgact.c hassgact.h1 hassgact.h2
	./choose cl trysgact hassgact.h1 hassgact.h2 > hassgact.h

hassgprm.h: \
choose compile load trysgprm.c hassgprm.h1 hassgprm.h2
	./choose cl trysgprm hassgprm.h1 hassgprm.h2 > hassgprm.h

tai.a: \
makelib tai_add.o tai_now.o tai_pack.o tai_sub.o tai_uint.o \
tai_unpack.o taia_add.o taia_approx.o taia_frac.o taia_less.o \
taia_now.o taia_pack.o taia_sub.o taia_tai.o taia_uint.o
	./makelib tai.a tai_add.o tai_now.o tai_pack.o \
	tai_sub.o tai_uint.o tai_unpack.o taia_add.o taia_approx.o \
	taia_frac.o taia_less.o taia_now.o taia_pack.o taia_sub.o \
	taia_tai.o taia_uint.o

tai_add.o: \
compile tai_add.c tai.h uint64.h
	./compile tai_add.c

tai_now.o: \
compile tai_now.c tai.h uint64.h
	./compile tai_now.c

tai_pack.o: \
compile tai_pack.c tai.h uint64.h
	./compile tai_pack.c

tai_sub.o: \
compile tai_sub.c tai.h uint64.h
	./compile tai_sub.c

tai_uint.o: \
compile tai_uint.c tai.h uint64.h
	./compile tai_uint.c

tai_unpack.o: \
compile tai_unpack.c tai.h uint64.h
	./compile tai_unpack.c

taia_add.o: \
compile taia_add.c taia.h tai.h uint64.h
	./compile taia_add.c

taia_approx.o: \
compile taia_approx.c taia.h tai.h uint64.h
	./compile taia_approx.c

taia_frac.o: \
compile taia_frac.c taia.h tai.h uint64.h
	./compile taia_frac.c

taia_less.o: \
compile taia_less.c taia.h tai.h uint64.h
	./compile taia_less.c

taia_now.o: \
compile taia_now.c taia.h tai.h uint64.h
	./compile taia_now.c

taia_pack.o: \
compile taia_pack.c taia.h tai.h uint64.h
	./compile taia_pack.c

taia_sub.o: \
compile taia_sub.c taia.h tai.h uint64.h
	./compile taia_sub.c

taia_tai.o: \
compile taia_tai.c taia.h tai.h uint64.h
	./compile taia_tai.c

taia_uint.o: \
compile taia_uint.c taia.h tai.h uint64.h
	./compile taia_uint.c
timeoutconn.o: \
compile timeoutconn.c ndelay.h socket.h uint16.h iopause.h taia.h \
tai.h uint64.h error.h timeoutconn.h uint16.h
	./compile timeoutconn.c

timeoutconn6.o: \
compile timeoutconn6.c ndelay.h socket.h uint16.h iopause.h taia.h \
tai.h uint64.h error.h timeoutconn.h uint16.h
	./compile timeoutconn6.c

timeoutwrite.o: \
compile timeoutwrite.c ndelay.h socket.h uint16.h iopause.h taia.h \
tai.h uint64.h error.h timeoutwrite.h uint16.h readwrite.h
	./compile timeoutwrite.c

timeoutread.o: \
compile timeoutread.c ndelay.h socket.h uint16.h iopause.h taia.h \
tai.h uint64.h error.h timeoutread.h uint16.h readwrite.h
	./compile timeoutread.c

timeoutaccept.o: \
compile timeoutaccept.c ndelay.h socket.h uint16.h iopause.h taia.h \
tai.h uint64.h error.h timeoutaccept.h uint16.h
	./compile timeoutaccept.c

timeoutaccept6.o: \
compile timeoutaccept6.c ndelay.h socket.h uint16.h iopause.h taia.h \
tai.h uint64.h error.h timeoutaccept.h uint16.h
	./compile timeoutaccept6.c

timeout.a: \
makelib timeoutconn.o timeoutconn6.o timeoutread.o timeoutwrite.o \
timeoutaccept.o timeoutaccept6.o
	./makelib timeout.a timeoutconn.o timeoutconn6.o timeoutwrite.o \
timeoutread.o timeoutaccept.o timeoutaccept6.o
error.o: \
compile error.c error.h
	./compile error.c

error_str.o: \
compile error_str.c error.h
	./compile error_str.c

error_temp.o: \
compile error_temp.c error.h
	./compile error_temp.c

ndelay_off.o: \
compile ndelay_off.c ndelay.h
	./compile ndelay_off.c

ndelay_on.o: \
compile ndelay_on.c ndelay.h
	./compile ndelay_on.c

open_read.o: \
compile open_read.c open.h
	./compile open_read.c

open_trunc.o: \
compile open_trunc.c open.h
	./compile open_trunc.c

open_append.o: \
compile open_append.c open.h
	./compile open_append.c

open_write.o: \
compile open_write.c open.h
	./compile open_write.c

openreadclose.o: \
compile openreadclose.c error.h open.h readclose.h stralloc.h \
gen_alloc.h openreadclose.h stralloc.h
	./compile openreadclose.c

readclose.o: \
compile readclose.c readwrite.h error.h readclose.h stralloc.h \
gen_alloc.h
	./compile readclose.c

seek_set.o: \
compile seek_set.c seek.h
	./compile seek_set.c

socket_accept.o: \
compile socket_accept.c byte.h socket.h uint16.h
	./compile socket_accept.c

socket_bind.o: \
compile socket_bind.c byte.h socket.h uint16.h
	./compile socket_bind.c

socket_conn.o: \
compile socket_conn.c readwrite.h byte.h socket.h uint16.h
	./compile socket_conn.c

socket_listen.o: \
compile socket_listen.c socket.h uint16.h
	./compile socket_listen.c

socket_recv.o: \
compile socket_recv.c byte.h socket.h uint16.h
	./compile socket_recv.c

socket_send.o: \
compile socket_send.c byte.h socket.h uint16.h
	./compile socket_send.c

socket_tcp.o: \
compile socket_tcp.c ndelay.h socket.h uint16.h
	./compile socket_tcp.c

socket_tcp6.o: \
compile socket_tcp6.c ndelay.h socket.h uint16.h haveip6.h error.h
	./compile socket_tcp6.c

socket_udp.o: \
compile socket_udp.c ndelay.h socket.h uint16.h
	./compile socket_udp.c

socket_udp6.o: \
compile socket_udp6.c ndelay.h socket.h uint16.h haveip6.h error.h
	./compile socket_udp6.c

socket_delay.o: \
compile socket_delay.c socket.h uint16.h
	./compile socket_delay.c

socket_local.o: \
compile socket_local.c byte.h socket.h uint16.h
	./compile socket_local.c

socket_remote.o: \
compile socket_remote.c byte.h socket.h uint16.h
	./compile socket_remote.c

socket_opts.o: \
compile socket_opts.c socket.h uint16.h
	./compile socket_opts.c

socket_conn6.o: \
compile socket_conn6.c byte.h socket.h uint16.h ip6.h haveip6.h error.h
	./compile socket_conn6.c

socket_bind6.o: \
compile socket_bind6.c byte.h socket.h uint16.h ip6.h haveip6.h error.h
	./compile socket_bind6.c

socket_accept6.o: \
compile socket_accept6.c byte.h socket.h uint16.h ip6.h haveip6.h error.h
	./compile socket_accept6.c

socket_recv6.o: \
compile socket_recv6.c byte.h socket.h uint16.h ip6.h haveip6.h error.h
	./compile socket_recv6.c

socket_send6.o: \
compile socket_send6.c byte.h socket.h uint16.h ip6.h haveip6.h error.h
	./compile socket_send6.c

socket_local6.o: \
compile socket_local6.c byte.h socket.h uint16.h ip6.h haveip6.h error.h
	./compile socket_local6.c

socket_remote6.o: \
compile socket_remote6.c byte.h socket.h uint16.h ip6.h haveip6.h error.h
	./compile socket_remote6.c

fd_copy.o: \
compile fd_copy.c fd.h
	./compile fd_copy.c

fd_move.o: \
compile fd_move.c fd.h
	./compile fd_move.c

pathexec_env.o: \
compile pathexec_env.c stralloc.h gen_alloc.h alloc.h str.h byte.h \
env.h pathexec.h
	./compile pathexec_env.c

pathexec_run.o: \
compile pathexec_run.c error.h stralloc.h gen_alloc.h str.h env.h \
pathexec.h
	./compile pathexec_run.c

env.o: \
compile env.c str.h env.h
	./compile env.c

haveip6.h: \
tryip6.c choose compile haveip6.h1 haveip6.h2
	./choose c tryip6 haveip6.h1 haveip6.h2 > haveip6.h

select.h: \
choose compile trysysel.c select.h1 select.h2
	./choose c trysysel select.h1 select.h2 > select.h

iopause.o: \
compile iopause.c taia.h tai.h uint64.h select.h iopause.h taia.h
	./compile iopause.c

coe.o: \
compile coe.h
	./compile coe.c

direntry.h: \
choose compile trydrent.c direntry.h1 direntry.h2
	./choose c trydrent direntry.h1 direntry.h2 > direntry.h

fifo.o: \
compile fifo.c hasmkffo.h fifo.h
	./compile fifo.c

hasmkffo.h: \
trymkffo.c compile load
	( ( ./compile trymkffo.c && ./load trymkffo ) >/dev/null \
	2>&1 \
	&& echo \#define HASMKFIFO 1 || exit 0 ) > hasmkffo.h
	rm -f trymkffo.o trymkffo

wait_nohang.o: \
compile wait_nohang.c haswaitp.h
	./compile wait_nohang.c

wait_pid.o: \
compile wait_pid.c error.h haswaitp.h
	./compile wait_pid.c

haswaitp.h: \
trywaitp.c compile load
	( ( ./compile trywaitp.c && ./load trywaitp ) >/dev/null \
	2>&1 \
	&& echo \#define HASWAITPID 1 || exit 0 ) > haswaitp.h
	rm -f trywaitp.o trywaitp

socket_broadcast.o: \
compile socket_broadcast.c socket.h uint16.h
	./compile socket_broadcast.c

socket_mcloop4.o: \
compile socket_mcloop4.c socket.h uint16.h
	./compile socket_mcloop4.c

socket_mcloop6.o: \
compile socket_mcloop6.c socket.h uint16.h haveip6.h
	./compile socket_mcloop6.c

socket_mcjoin4.o: \
compile socket_mcjoin4.c socket.h uint16.h byte.h
	./compile socket_mcjoin4.c

socket_mcjoin6.o: \
compile socket_mcjoin6.c socket.h uint16.h byte.h haveip6.h ip6.h error.h
	./compile socket_mcjoin6.c

socket_mcleave4.o: \
compile socket_mcleave4.c socket.h uint16.h byte.h
	./compile socket_mcleave4.c

socket_mcleave6.o: \
compile socket_mcleave6.c socket.h uint16.h byte.h haveip6.h error.h
	./compile socket_mcleave6.c

socket_mcttl4.o: \
compile socket_mcttl4.c socket.h uint16.h
	./compile socket_mcttl4.c

socket_mcttl6.o: \
compile socket_mcttl6.c socket.h uint16.h byte.h haveip6.h ip6.h
	./compile socket_mcttl6.c

unix.a: \
makelib error.o error_str.o error_temp.o ndelay_off.o ndelay_on.o \
open_read.o open_trunc.o openreadclose.o readclose.o seek_set.o \
socket_accept.o socket_bind.o socket_conn.o socket_listen.o socket_recv.o \
socket_send.o socket_tcp.o socket_udp.o socket_delay.o socket_local.o \
socket_opts.o socket_remote.o socket_conn6.o socket_bind6.o \
socket_accept6.o socket_recv6.o socket_send6.o socket_local6.o \
socket_remote6.o socket_tcp6.o socket_udp6.o fd_move.o fd_copy.o \
pathexec_env.o pathexec_run.o env.o iopause.o fifo.o wait_nohang.o \
wait_pid.o socket_broadcast.o socket_mcloop4.o socket_mcloop6.o \
socket_mcjoin4.o socket_mcjoin6.o socket_mcttl4.o socket_mcttl6.o \
socket_mcleave4.o socket_mcleave6.o open_append.o open_write.o
	./makelib unix.a error.o error_str.o ndelay_off.o \
	ndelay_on.o open_read.o open_trunc.o openreadclose.o \
	readclose.o seek_set.o socket_accept.o socket_bind.o \
	socket_conn.o socket_listen.o socket_recv.o socket_send.o \
	socket_tcp.o socket_udp.o socket_delay.o socket_local.o \
	socket_opts.o socket_remote.o socket_conn6.o socket_bind6.o \
	socket_accept6.o socket_recv6.o socket_send6.o \
	socket_local6.o socket_remote6.o socket_tcp6.o socket_udp6.o \
	fd_move.o fd_copy.o pathexec_run.o pathexec_env.o env.o \
	iopause.o fifo.o wait_nohang.o wait_pid.o socket_broadcast.o \
	socket_mcloop4.o socket_mcloop6.o socket_mcjoin4.o \
	socket_mcjoin6.o socket_mcttl4.o socket_mcttl6.o \
	socket_mcleave4.o socket_mcleave6.o open_append.o open_write.o

dns.a: \
makelib dns_dfd.o dns_domain.o dns_dtda.o dns_ip.o dns_ipq.o \
dns_name.o dns_nd.o dns_packet.o dns_random.o dns_rcip.o dns_rcrw.o \
dns_resolve.o dns_sortip.o dns_transmit.o dns_txt.o dns_ip6.o \
dns_sortip6.o dns_nd6.o dns_ipq6.o
	./makelib dns.a dns_dfd.o dns_domain.o dns_dtda.o dns_ip.o \
	dns_ipq.o dns_name.o dns_nd.o dns_packet.o dns_random.o \
	dns_rcip.o dns_rcrw.o dns_resolve.o dns_sortip.o \
	dns_transmit.o dns_txt.o dns_ip6.o dns_sortip6.o dns_nd6.o \
	dns_ipq6.o

dns_dfd.o: \
compile dns_dfd.c error.h alloc.h byte.h dns.h stralloc.h gen_alloc.h \
iopause.h taia.h tai.h uint64.h taia.h
	./compile dns_dfd.c

dns_domain.o: \
compile dns_domain.c error.h alloc.h case.h byte.h dns.h stralloc.h \
gen_alloc.h iopause.h taia.h tai.h uint64.h taia.h
	./compile dns_domain.c

dns_ip.o: \
compile dns_ip.c stralloc.h gen_alloc.h uint16.h byte.h dns.h \
stralloc.h iopause.h taia.h tai.h uint64.h taia.h
	./compile dns_ip.c

dns_ip6.o: \
compile dns_ip6.c stralloc.h gen_alloc.h uint16.h byte.h dns.h \
stralloc.h iopause.h taia.h tai.h uint64.h taia.h ip6.h ip4.h alloc.h
	./compile dns_ip6.c

dns_ipq.o: \
compile dns_ipq.c stralloc.h gen_alloc.h case.h byte.h str.h dns.h \
stralloc.h iopause.h taia.h tai.h uint64.h taia.h
	./compile dns_ipq.c

dns_ipq6.o: \
compile dns_ipq6.c stralloc.h gen_alloc.h case.h byte.h str.h dns.h \
stralloc.h iopause.h taia.h tai.h uint64.h taia.h ip6.h alloc.h
	./compile dns_ipq6.c

dns_name.o: \
compile dns_name.c stralloc.h gen_alloc.h uint16.h byte.h dns.h \
stralloc.h iopause.h taia.h tai.h uint64.h taia.h ip6.h
	./compile dns_name.c

dns_nd.o: \
compile dns_nd.c byte.h fmt.h dns.h stralloc.h gen_alloc.h iopause.h \
taia.h tai.h uint64.h taia.h
	./compile dns_nd.c

dns_nd6.o: \
compile dns_nd6.c byte.h fmt.h dns.h stralloc.h gen_alloc.h iopause.h \
taia.h tai.h uint64.h taia.h
	./compile dns_nd6.c

dns_packet.o: \
compile dns_packet.c error.h dns.h stralloc.h gen_alloc.h iopause.h \
taia.h tai.h uint64.h taia.h
	./compile dns_packet.c

dns_random.o: \
compile dns_random.c dns.h stralloc.h gen_alloc.h iopause.h taia.h \
tai.h uint64.h taia.h taia.h uint32.h
	./compile dns_random.c

dns_rcip.o: \
compile dns_rcip.c taia.h tai.h uint64.h openreadclose.h stralloc.h \
gen_alloc.h byte.h ip4.h env.h dns.h stralloc.h iopause.h taia.h \
taia.h
	./compile dns_rcip.c

dns_rcrw.o: \
compile dns_rcrw.c taia.h tai.h uint64.h env.h byte.h str.h \
openreadclose.h stralloc.h gen_alloc.h dns.h stralloc.h iopause.h \
taia.h taia.h
	./compile dns_rcrw.c

dns_resolve.o: \
compile dns_resolve.c iopause.h taia.h tai.h uint64.h taia.h byte.h \
dns.h stralloc.h gen_alloc.h iopause.h taia.h
	./compile dns_resolve.c

dns_sortip.o: \
compile dns_sortip.c byte.h dns.h stralloc.h gen_alloc.h iopause.h \
taia.h tai.h uint64.h taia.h
	./compile dns_sortip.c

dns_sortip6.o: \
compile dns_sortip6.c byte.h dns.h stralloc.h gen_alloc.h iopause.h \
taia.h tai.h uint64.h taia.h
	./compile dns_sortip6.c

dns_transmit.o: \
compile dns_transmit.c socket.h uint16.h alloc.h error.h byte.h \
readwrite.h uint16.h dns.h stralloc.h gen_alloc.h iopause.h taia.h \
tai.h uint64.h taia.h
	./compile dns_transmit.c

dns_txt.o: \
compile dns_txt.c stralloc.h gen_alloc.h uint16.h byte.h dns.h \
stralloc.h iopause.h taia.h tai.h uint64.h taia.h
	./compile dns_txt.c

