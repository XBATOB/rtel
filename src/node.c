/*!
 * \file node.c
 * \brief Краткое описание
 *
 * Полное описание
 *
 * $$Id: node.c 1349 2008-06-16 11:08:57Z xbatob $$
 *
 * \author Serguei B. Khvatov <xbatob@techno.spb.ru>, (C) 2008
 */

#include "node.h"
#include "debug.h"

#include <sys/uio.h>
#include <unistd.h>

int node_read_callback(struct node *nodep) {
    int bs = node_room(nodep);
    char buff[bs];
    int rc = read (nodep->ifd, buff, bs);
    if (rc <= 0) {
        rtel_debug("read(%d)=%d, %m", nodep->ifd, rc);
Ex:
        if (nodep->eof_cb)
            rc = (*(nodep->eof_cb))(rc, nodep->eof_data, nodep);
        return rc;
    }
    unsigned char *p;
    int i;
    for (i = rc, p = (unsigned char *)buff; i-- > 0; ) {
        int c = *p++;
        do {
            if (nodep->filter)
                c = (*(nodep->filter))(c, nodep->filter_data, nodep);
            if (c & NODE_FILTER_EOF) {
                rc = 0;
                goto Ex;
            }
            if (!(c & NODE_FILTER_IGNORE) && nodep->dest)
                node_putc(c, nodep->dest);
        } while (c & NODE_FILTER_AGAIN);
    }
    return (rc);
}

int node_write_callback(struct node *nodep) {
    struct iovec iov[2];
    int ioc;
    iov[0].iov_base = nodep->oque.buff + nodep->oque.ri;
    if (nodep->oque.wi <= nodep->oque.ri) {
        iov[0].iov_len = IOQUEUE_SIZE - nodep->oque.ri;
        iov[1].iov_base = nodep->oque.buff;
        iov[1].iov_len = nodep->oque.wi;
        ioc = 2;
    } else {
        iov[0].iov_len = nodep->oque.wi - nodep->oque.ri;
        ioc = 1;
    }

    if (nodep->write_hook) (*(nodep->write_hook))(nodep->ofd, 1, nodep);
    ioc = writev (nodep->ofd, iov, ioc);
    if (ioc <= 0)
        return ioc;
    if ((nodep->oque.len -= ioc) == 0) {
        if (nodep->write_hook) (*(nodep->write_hook))(nodep->ofd, 0, nodep);
        nodep->oque.wi = nodep->oque.ri = 0;
    } else {
        nodep->oque.ri += ioc;
        if (nodep->oque.ri > IOQUEUE_SIZE)
            nodep->oque.ri -= IOQUEUE_SIZE;
    }
    return ioc;
}
