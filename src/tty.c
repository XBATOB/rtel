/*
*  C Implementation: tty
*
* Description:
*
*
* Author: Serguei B. Khvatov <xbatob@techno.spb.ru>, (C) 2005
*
* Copyright: See COPYING file that comes with this distribution
*
* $Id: tty.c 2069 2009-12-08 12:39:09Z xbatob $
* $Log$
*/

#include "tty.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <error.h>

speed_t ttyParseSpeed (int speed) {
    struct speed_def {
        unsigned long val;
        speed_t arg;
    };
    static const struct speed_def sds[] = {
        {
            230400, B230400
        }, {
            115200, B115200
        }, {
            57600, B57600
        }, {
            38400, B38400
        }, {
            19200, B19200
        }, {
            9600, B9600
        }, {
            4800, B4800
        }, {
            2400, B2400
        }, {
            0, B0
        },
    };
    const struct speed_def *sdp;

    for (sdp = sds; sdp->val; ++sdp) {
        if (sdp->val == speed)
            return (sdp->arg);
    }
    return B0;
}

int ttySetRaw (int fd, TtyState *saveopts) {
    struct termios opts;
    if (tcgetattr (fd, &opts)) {
        error (0, errno, "tcgetattr(%d)", fd);
        return 0;
    }
    if (saveopts)
        *saveopts = opts;
    cfmakeraw (&opts);
    if (tcsetattr(fd, TCSAFLUSH, &opts)) {
        error (0, errno, "tcsetattr(%d)", fd);
        return 0;
    }
    return 1;
}

int ttyInit (const char *dev, speed_t speed,
             ttyinit_hook_t hook) {
    int fd = open(dev, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0) {
        error (0, errno, "open(%s)", dev);
        return fd;
    }
    fcntl(fd, F_SETFL, 0);

    struct termios opts;
    if (tcgetattr (fd, &opts)) {
        error (0, errno, "tcgetattr(%s)", dev);
Fault:
        close (fd);
        return -1;
    }
    cfmakeraw (&opts);
    if (speed != B0) {
        cfsetispeed(&opts, speed);
        cfsetospeed(&opts, speed);
    }
    if (hook && (*hook)(&opts) < 0) goto Fault;

    if (tcsetattr(fd, TCSAFLUSH, &opts)) {
        error (0, errno, "tcsetattr(%s)", dev);
        goto Fault;
    }
    return fd;
}

int ttySetRTS (int fd, int st) {
    unsigned int status = 0;
    if (ioctl(fd, TIOCMGET, &status) < 0) {
        error (0, errno, "ioctl(TIOCMGET)");
        return 0;
    }
    if (st)
        status |= TIOCM_RTS;
    else
        status &= ~TIOCM_RTS;
    if (ioctl(fd, TIOCMSET, &status) < 0) {
        error (0, errno, "ioctl(TIOCMSET)");
        return 0;
    }
    return 1;
}
