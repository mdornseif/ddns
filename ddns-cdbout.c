void snap_dump(char *filename, stralloc *sa)
{
  dAVLCursor c;
  dAVLNode *node;
  char strip[IP6_FMT];
  char strnum[FMT_ULONG];
  int fd;

  fd = open_trunc("filename");  
  if(fd == -1)
    strerr_warn1(ARGV0 "warning: unable to open for tcp.tmp for writing", &strerr_sys);
  
  buffer_init(&wb, write, fd, wbspace, sizeof wbspace);

  node = dAVLFirst(&c, t);
  while(node)
    {
      buffer_put(&wb, strnum, fmt_ulong(strnum, node->key));
      buffer_puts(&wb, ",");
      buffer_put(&wb, strip, ip4_fmt(strip, node->ip4));
      buffer_puts(&wb, ",");
      buffer_put(&wb, strip, ip6_fmt(strip, node->ip6));
      buffer_puts(&wb, ",LOC\n");
      
      node = dAVLNext(&c);
    }
 
  buffer_flush(&wb);
  close(fd);
}


void dodump()
{
  stralloc dummy = {0};

  buffer_putsflush(buffer_2, ARGV0 "dumping\n");

  write_fifodir("snapdir", &dummy, snap_dump);

  buffer_putsflush(buffer_2, ARGV0 "dumping ready\n");
}


void dump_db_to_cdb()
{
  char *z;

  z = "0";
  *z =+ flagchanged;
  buffer_puts(buffer_2, z);
  buffer_puts(buffer_2, " ");
  
  z = "0";
  *z =+ flagchildrunning;
  buffer_puts(buffer_2, z);
  buffer_putsflush(buffer_2, " checking if a dump is needed\n");

  if(flagsighup)
    {
      flagsighup = 0;
      buffer_putsflush(buffer_2, ARGV0 "SIGHUP recived, dumping withouth further asking\n");
    }
  if(flagchanged && !flagchildrunning)
    {
      flagchanged = 0;
      flagchildrunning++;

      buffer_putsflush(buffer_2, ARGV0 "yep, forking\n");

      /* fork of a child to do this */
      switch(fork()) 
	{
	case 0:
	  /* this is the child */
	  /* XXX close fifos? */
	  sig_ignore(sig_alarm);
	  sig_ignore(sig_hangup);
	  buffer_putsflush(buffer_2, ARGV0 "child started\n");
	  dodump();
	  buffer_putsflush(buffer_2, ARGV0 "child exiting\n");
	  _exit(0);
	case -1:
	  strerr_warn2(ARGV0, "unable to fork: ", &strerr_sys);
	  break;
	}
     
      /* this is the parent */
      flagdumpasap = 0;
      buffer_putsflush(buffer_2, ARGV0 "parent\n");
    } 
}
