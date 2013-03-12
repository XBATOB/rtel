/*
*  C Implementation: rtel
*
* Description:
*
*
* Author: Serguei B. Khvatov <xbatob@techno.spb.ru>, (C) 2005
*
* Copyright: See COPYING file that comes with this distribution
*
* $Id: rtel.h 1345 2008-06-16 09:27:12Z xbatob $
* $Log$
*/

#ifndef rtel__h
#define rtel__h

#include "tty.h"
extern TtyState oldopts;

#ifdef WITH_LOGGING
#include <stdio.h>
extern char *logname;
extern FILE *logfile;
#endif
extern unsigned char escape;
extern void rtel_loop (int dev_fd);
extern void rt_help (void);

#endif /* rtel__h */
