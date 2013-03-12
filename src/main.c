/*
*  C Implementation: main
*
* Description:
*
*
* Author: Serguei B. Khvatov <xbatob@techno.spb.ru>, (C) 2005
*
* Copyright: See COPYING file that comes with this distribution
*
* $Id: main.c 2212 2010-06-24 13:18:03Z xbatob $
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "tty.h"
#include "rts.h"
#include "rtel.h"
#include "gettext.h"

#ifndef LOCALEDIR
#define LOCALEDIR NULL
#endif

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <error.h>
#include <errno.h>

#ifndef PACKAGE_STRING
#define PACKAGE_STRING "lighthweight serial communicator 1.5"
#define PACKAGE_BUGREPORT "Sergey Khvatov <xbatob@techno.spb.ru>"
#endif


TtyState oldopts;

struct rts rts;
static const char *dev;
static int devd;
static speed_t speed = B0;
static tcflag_t par_clr = CRTSCTS, par_set = 0;

#include <argp.h>

const char *argp_program_version = PACKAGE_STRING " ("
#ifdef  HAVE_LIBREADLINE
    "READLINE,"
#endif
#ifdef  WITH_FILTER
    "FILTER,"
#endif
#ifdef  WITH_LOGGING
    "LOGGING,"
#endif
    ")\n$Id: main.c 2212 2010-06-24 13:18:03Z xbatob $";
const char *argp_program_bug_address = PACKAGE_BUGREPORT;

static int init_hook(struct termios *state) {
    state->c_cflag &= ~par_clr;
    state->c_cflag |= par_set;
    return 0;
}

static error_t apf (int key, char *arg, struct argp_state *state) {
    switch (key) {
    case 'd':
    case ARGP_KEY_ARG:
        dev = arg;
        break;
    case 's':
        speed = ttyParseSpeed(strtoul (arg, 0, 10));
        if (speed == B0)
            argp_error (state, _("bad speed value: %s"), arg);
        break;
    case 'o': {
        static const char *pars[] = {
            "odd", "even", "none", "cs8", "cs7",
            0
        };
#define STTY_PARS "odd,even,none,cs8,cs7"
        char *val;
        while (*arg) {
            int i = getsubopt (&arg, (char *const *)pars, &val);
            if (i < 0)
                argp_error (state, _("bad tty option: %s"), val);
            switch (i) {
            case 0:
                par_set |= PARENB | PARODD;
                break;
            case 1:
                par_set = (par_set & ~PARODD) | PARENB;
                par_clr |= PARODD;
                break;
            case 2:
                par_set &= ~(PARENB | PARODD);
                par_clr |= PARENB | PARODD;
                break;
            case 3:
                par_clr |= CSIZE;
                par_set = (par_set & ~CSIZE) | CS8;
                break;
            case 4:
                par_clr |= CSIZE;
                par_set = (par_set & ~CSIZE) | CS7;
                break;
            }
        }
        break;
    }
    case 'r':
        if (!parse_rts (arg))
            argp_error (state, _("bad RTS control value: %s"), arg);
        break;
    case 'e':
        escape = *arg;
        break;
    case ARGP_KEY_END:
        if (!dev)
            argp_error (state, _("device is not set"));
        devd = ttyInit (dev, speed, init_hook);
        if (devd < 0)
            argp_failure (state, 1, 0, _("can not init tty"));
        break;
    default:
        return  ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static char *aph (int key, const char *text, void *input) {
    switch (key) {
    case 'e': {
        char *b;
        asprintf(&b, text, escape);
        return b;
    }
    case 'o': {
        char *b;
        asprintf(&b, text, STTY_PARS);
        return b;
    }
    }
    return (char *)text;
}

static const struct argp_option apo[] = {
    {
        "device", 'd', "FILE", 0,
        N_("Device name")
    }, {
        "speed", 's', "SPEED", 0,
        N_("Set device speed")
    }, {
        "option", 'o', "LIST", 0,
        N_("Coma-separated list of tty options to set (%s)")
    }, {
        "rts", 'r', "1|0|t|r", 0,
        N_("Set rts line to given state"
        " (1 or 0 allways, 1 on transmit, 1 on receive)")
    }, {"escape",  'e', "CHAR", 0,
        N_("Escape character [%c]")
    }, {
        0
    }
};

static const struct argp ap = {
    apo, apf, N_("[device]"),
    N_("lighthweight serial communicator"),
    0, aph
};


int main(int argc, char *argv[]) {
#if			ENABLE_NLS
    setlocale (LC_ALL, "");
    bindtextdomain (PACKAGE, LOCALEDIR);
    textdomain (PACKAGE);
#endif	//	ENABLE_NLS

    argp_parse (&ap, argc, argv, 0, 0, 0);
    rt_help();
    if (!ttySetRaw (0, &oldopts))
        return EXIT_FAILURE;
    rtel_loop (devd);
    ttyReset (0, &oldopts);
    close (devd);
    error (0, 0, _("\nGood Bye!"));
    return EXIT_SUCCESS;
}
