/*
*  C Implementation: tty
*
* Description:
*
*
* Author: Serguei B. Khvatov <xbatob@techno.spb.ru>, (C) 2005-2007 Techno Group, Inc
*
* Copyright: See COPYING file that comes with this distribution
*
* $Id: tty.h 1750 2009-02-18 11:37:16Z xbatob $
* $Log$
*/

#ifndef tty__h
#define tty__h

#include <termios.h>
#include <unistd.h>

typedef struct termios TtyState;

typedef int (*ttyinit_hook_t)(struct termios *state);

extern speed_t ttyParseSpeed (int speed);
extern int ttySetRaw (int fd, TtyState *saveopts);
#define ttyReset(fd,savedopts) tcsetattr(fd, TCSAFLUSH, savedopts)
extern int ttyInit (const char *dev, speed_t speed,
                        ttyinit_hook_t hook);
extern int ttySetRTS (int fd, int st);
#endif /* tty__h */
