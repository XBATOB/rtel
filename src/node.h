/*
*  C Implementation: node
*
* Description:
*
*
* Author: Serguei B. Khvatov <xbatob@techno.spb.ru>, (C) 2008
*
* Copyright: See COPYING file that comes with this distribution
*
* $Id: node.h 1345 2008-06-16 09:27:12Z xbatob $
* $Log$
*/

#ifndef node__h
#define node__h

#define IOQUEUE_SIZE 1000

struct ioqueue {
    int  wi, ri;
    int  len;
    char buff[IOQUEUE_SIZE];
};

static inline int ioq_room (struct ioqueue *ioqp) {
    return IOQUEUE_SIZE - ioqp->len;
}

static inline int ioq_putc (int c, struct ioqueue *ioqp) {
    if (ioq_room(ioqp) <= 0) return -1;
    ioqp->buff[ioqp->wi++] = c;
    if (ioqp->wi >= IOQUEUE_SIZE) ioqp->wi = 0;
    ++ioqp->len;
    return c;
}

struct node;

typedef int (*node_filter_t) (int chr, void *data, struct node *nodep);
#define	NODE_FILTER_EOF		0x100
#define	NODE_FILTER_IGNORE	0x200
#define	NODE_FILTER_AGAIN	0x400

typedef int (*node_eof_t) (int rc, void *data, struct node *nodep);
typedef int (*node_whook_t) (int fd, int pre, struct node *nodep);

/*! \brief Узел: устройство, терминал или программа-фильтр
 */
struct node {
    /*! дескриптор для чтения */
    int ifd;
    /*! дескриптор для записи. Может совтадать с ifd */
    int ofd;
    /*! приёмник для принятых данных.
     *  Может быть NULL.
     *  В этом случае считанные данные теряются (после фильтра) */
    struct node *dest;
    /*! фильтр для считываемых данных. Может быть NULL */
    node_filter_t filter;
    /*! параметр, передаваемый фильтру */
    void *filter_data;
    /*! обработчик EOF или ошибки */
    node_eof_t eof_cb;
    /*! параметр, передаваемый обработчику */
    void *eof_data;
    /*! очередь вывода */
    struct ioqueue oque;
    node_whook_t write_hook;
};

static inline int node_room(struct node *nodep) {
    return nodep->dest ? ioq_room(&nodep->dest->oque) : IOQUEUE_SIZE;
}
static inline int node_qlen(struct node *nodep) {
    return nodep->oque.len;
}

static inline int node_putc (int c, struct node *nodep) {
    return ioq_putc(c, &nodep->oque);
}

extern int node_read_callback(struct node *nodep);
extern int node_write_callback(struct node *nodep);

#endif /* node__h */
