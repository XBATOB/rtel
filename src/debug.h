/*!
 * \file debug.h
 * \brief Краткое описание
 *
 * Полное описание
 *
 * $$Id: debug.h 1349 2008-06-16 11:08:57Z xbatob $$
 *
 * \author Serguei B. Khvatov <xbatob@techno.spb.ru>, (C) 2008
 */
#ifndef _RTEL_DEBUG_H_
#define _RTEL_DEBUG_H_

#ifndef RTEL_DEBUG

#define rtel_debug(fmt, ...)

#else

#include <stdio.h>
#define rtel_debug(fmt, ...) fprintf(stderr, "\r%s: " fmt "\r\n", \
		__PRETTY_FUNCTION__, ##__VA_ARGS__)

#endif

#endif
