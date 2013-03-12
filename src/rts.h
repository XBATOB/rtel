/*
*  C Implementation: rts
*
* Description:
*
*
* Author: Serguei B. Khvatov <xbatob@techno.spb.ru>, (C) 2007
*
* Copyright: See COPYING file that comes with this distribution
*
* $Id: rts.h 1259 2008-04-28 08:28:07Z xbatob $
* $Log$
*/

#ifndef rts__h
#define rts__h

#include "tty.h"

extern struct rts {
        unsigned char	txstate;
        unsigned char	init;
        unsigned char	ctrl;
    }
rts;

static inline void init_rts(int fd) {
    if (rts.init)
        ttySetRTS (fd, !rts.txstate);
}

static inline void set_rts_tx (int fd) {
    if (rts.ctrl)
        ttySetRTS (fd, rts.txstate);
}

static inline void set_rts_rx (int fd) {
    if (!rts.ctrl)
        return;
    tcdrain(fd);
    ttySetRTS (fd, !rts.txstate);
}

static inline int parse_rts (const char *o) {
    if (!o)
        return 0;
    switch (*o) {
    case '0':
        rts.txstate = 0;
        rts.init = 1;
        rts.ctrl = 0;
        break;
    case '1':
        rts.txstate = 1;
        rts.init = 1;
        rts.ctrl = 0;
        break;
    case 'r':
        rts.txstate = 0;
        rts.init = 1;
        rts.ctrl = 1;
        break;
    case 't':
        rts.txstate = 1;
        rts.init = 1;
        rts.ctrl = 1;
        break;
    default:
        return 0;
    }
    return 1;
}


#endif /* rts__h */
