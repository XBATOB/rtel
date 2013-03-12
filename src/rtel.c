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
* $Id: rtel.c 2445 2011-01-11 15:04:16Z xbatob $
* $Log$
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "rtel.h"
#include "node.h"
#include "gettext.h"
#include "debug.h"
#include "rts.h"

#include <sys/select.h>
#include <errno.h>
#include <error.h>
#include <stdlib.h>
#include <stdio.h>

#define HOST_QUEUE      500
#define DEVICE_QUEUE    500
#define HOST_AVAIL      10
#define DEVICE_AVAIL    10

#ifndef PACKAGE
# define PACKAGE "rtel"
#endif

#ifdef WITH_LOGGING
char *logname;
#define	LOGNAME (logname ?: PACKAGE ".log")
FILE *logfile;
#endif

enum {
    NODE_HOST=0, NODE_DEVICE,
#ifdef		WITH_FILTER
    NODE_FILTER,
#endif
    NODE_CNT
};
struct node nodes[NODE_CNT];

#ifdef		HAVE_LIBREADLINE
#include <readline/readline.h>
#include <readline/history.h>
#else	//	HAVE_LIBREADLINE
static char *readline(const char *prompt) {
    if (prompt && prompt) {
        fputs (prompt, stdout);
        fflush (stdout);
    }
    char *rp;
    int rc = getline (&rp, 0, stdin);
    if (rc < 0) return 0;
    char *nl = strchr (rp, '\n');
    if (nl) *nl = 0;
    return rp;
}
#endif	//	HAVE_LIBREADLINE

unsigned char escape = '~';
#define EOL "\r\n"

void rt_help (void) {
    fprintf (stderr, _("Enter %c. to exit%s"
                       "Enter %c%c to put %c%s"),
             escape, EOL, escape, escape, escape, EOL);
#ifdef	WITH_LOGGING
    fprintf (stderr, _("Enter %cl to toggle logging%s"), escape, EOL);
#endif
#ifdef	WITH_FILTER
    fprintf (stderr, _("Enter %c[<|>] to exec filter%s"), escape, EOL);
#endif
    fflush (stderr);
}

#ifdef WITH_LOGGING
static inline void toggle_logging(void) {
    if (logfile) {
        fclose (logfile);
        logfile = NULL;
        fprintf (stderr, _("logging is stopped\r\n"));
    } else {

        ttyReset (0, &oldopts);
        fprintf (stderr, _("Enter logging file name [%s]\n"), LOGNAME);
        {
            char *fn = readline (_("File:"));
            if (fn) {
                if (*fn) {
                    if (logname) free (logname);
                    logname = fn;
                } else free (fn);
            }
        }
        ttySetRaw (0, 0);

        logfile = fopen (LOGNAME, "a");
        if (logfile) {
            fprintf (stderr, _("started logging to %s\r\n"), LOGNAME);
        } else {
            error (0, errno, _("can not start logging to %s"), LOGNAME);
        }
    }
}

static inline void stop_logging(void) {
    if (logfile) {
        fclose (logfile);
        logfile = NULL;
    }
}
#endif

static inline void set_normal_dests() {
    nodes[NODE_HOST].dest = nodes + NODE_DEVICE;
    nodes[NODE_DEVICE].dest = nodes + NODE_HOST;
#ifdef WITH_FILTER
    nodes[NODE_FILTER].dest = 0;
#endif
}

#ifdef WITH_FILTER
#include <sys/wait.h>

pid_t filter_pid;

int filter_eof(int rc, void *data, struct node *nodep) {
    rtel_debug("closing filter files");
    if (rc < 0) return rc;
    close (nodes[NODE_FILTER].ifd);
    nodes[NODE_FILTER].ifd = -1;
    close (nodes[NODE_FILTER].ofd);
    nodes[NODE_FILTER].ofd = -1;
    set_normal_dests();
    return 1;
}

static inline void exec_filter (unsigned char mode) {
    if (filter_pid > 0) {
        filter_eof(0, 0, 0);
        return;
    }
    ttyReset (0, &oldopts);
    char *pp = readline(_("Filter:"));
    if (pp) {
        if (*pp) {
            int pi[2];	// input from program
            int po[2];	// output to program
            pipe(pi);
            pipe(po);
            if (!(filter_pid = fork())) {
                /* son - pipe roles swaped! */
                dup2 (po[0], 0);
                close (po[1]);
                close (po[0]);
                dup2 (pi[1], 1);
                close (pi[0]);
                close (pi[1]);
                close (nodes[NODE_DEVICE].ifd);
                signal (SIGPIPE, SIG_DFL);
                execl ("/bin/sh", "/bin/sh", "-c", pp, NULL);
                _exit (127);
            }

            nodes[NODE_FILTER].ifd = pi[0];
            close (pi[1]);
            nodes[NODE_FILTER].ofd = po[1];
            close (po[0]);

            switch (mode) {
            case '>':
                nodes[NODE_FILTER].dest = nodes + NODE_DEVICE;
                nodes[NODE_HOST].dest = nodes + NODE_FILTER;
                break;
            case '<':
                nodes[NODE_FILTER].dest = nodes + NODE_HOST;
                nodes[NODE_DEVICE].dest = nodes + NODE_FILTER;
                break;
            case '|':
                nodes[NODE_FILTER].dest = nodes + NODE_DEVICE;
                nodes[NODE_DEVICE].dest = nodes + NODE_FILTER;
                nodes[NODE_HOST].dest = 0;
                break;
            }
        }
        free (pp);
    }
    ttySetRaw (0, 0);
}

#endif

struct host_filter_data {
    enum {
        HF_NORM = 0, HF_CTRL, HF_ESC
    } state;
    unsigned char last;
};

static int host_filter (int chr, void *data, struct node *nodep) {
    struct host_filter_data *status = data;
    //chr &= 0xff;
    switch (status->state) {
    case HF_NORM:
        if (chr == escape) {
            status->state = HF_CTRL;
            chr |= NODE_FILTER_IGNORE;
        }
        break;
    case HF_CTRL:
        status->state = HF_NORM;
        if (chr == escape) {
            break;
        }
        switch (chr) {
        case '.':
            chr |= NODE_FILTER_EOF;
            break;
        case '?':
        case 'h':
            rt_help();
            break;
#ifdef WITH_LOGGING
        case 'l':
            toggle_logging();
            break;
#endif
#ifdef WITH_FILTER
        case '<':
        case '>':
        case '|':
            exec_filter(chr);
            break;
#endif
        default:
            status->last = chr;
            chr = escape;
            status->state = HF_ESC;
            return escape | NODE_FILTER_AGAIN;
        }
        chr |= NODE_FILTER_IGNORE;
        break;
    case HF_ESC:
        chr = status->last;
        status->state = HF_NORM;
        break;
    }
    return chr;

}
#ifdef WITH_LOGGING
static int dev_filter (int chr, void *data, struct node *nodep) {
    if (logfile) putc (chr, logfile);
    return chr;
}
#endif // WITH_LOGGING

static inline int rtel_iteration () {
    int max_fd = -1;
    fd_set fd_rd, fd_wr;
    struct node *ndp;

    FD_ZERO(&fd_rd);
    FD_ZERO(&fd_wr);

    for (ndp = nodes + NODE_CNT; --ndp >= nodes; ) {
        if (ndp->ifd >= 0 && node_room (ndp) > 0) {
            FD_SET (ndp->ifd, &fd_rd);
            if (max_fd < ndp->ifd) max_fd = ndp->ifd;
        }
        if (ndp->ofd >= 0 && node_qlen (ndp) > 0) {
            FD_SET (ndp->ofd, &fd_wr);
            if (max_fd < ndp->ofd) max_fd = ndp->ofd;
        }
    }
    int r;
#ifdef		WITH_FILTER
    if (filter_pid > 0)	{
        int s;
        r = waitpid(filter_pid, &s, WNOHANG);
        if (r == filter_pid) {
            rtel_debug ("wait(%d); status=%x", filter_pid, s);
            filter_pid = 0;
        }
    }
#endif
    r = select (max_fd + 1, &fd_rd, &fd_wr, 0, 0);
    if (r < 0) {
        error (0, errno, "select()");
        return r;
    }

    for (ndp = nodes + NODE_CNT; --ndp >= nodes; ) {
        if (ndp->ifd >= 0 && FD_ISSET (ndp->ifd, &fd_rd)) {
            r = node_read_callback(ndp);
            if (r <= 0) break;
        }
        if (ndp->ofd >= 0 && FD_ISSET (ndp->ofd, &fd_wr)) {
            r = node_write_callback(ndp);
            if (r <= 0) break;
        }
    }

    return r;
}

static int rts_ctrl (int fd, int pre, struct node *nodep) {
    if (pre) {
        set_rts_tx(fd);
    } else {
        set_rts_rx(fd);
    }
}

void rtel_loop (int dev_fd) {
    //memset (nodes, 0, sizeof (nodes);

    //nodes[NODE_HOST].ifd = 0;
    nodes[NODE_HOST].ofd = 1;
    nodes[NODE_HOST].filter = host_filter;
    struct host_filter_data host_filter_data = {
        0
    };
    nodes[NODE_HOST].filter_data = &host_filter_data;

    nodes[NODE_DEVICE].ifd = nodes[NODE_DEVICE].ofd = dev_fd;
#ifdef WITH_LOGGING
    nodes[NODE_DEVICE].filter = dev_filter;
#endif
    nodes[NODE_DEVICE].write_hook = rts_ctrl;
    init_rts(dev_fd);
#ifdef WITH_FILTER
    nodes[NODE_FILTER].ifd = nodes[NODE_FILTER].ofd = -1;
    nodes[NODE_FILTER].eof_cb = filter_eof;
    signal (SIGPIPE, SIG_IGN);
#endif
    set_normal_dests();

    for (;;) {
        int r = rtel_iteration();
        if (r <= 0) break;
    }
}

