/*
LICENSE INFORMATION:
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License (LGPL) as published by the Free Software Foundation.

Please refer to the COPYING file for more information.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

Copyright (c) 2004 Bruno T. C. de Oliveira
*/


#ifndef btco_ROTE_vtstate_h
#define btco_ROTE_vtstate_h

#define ESEQ_BUF_SIZE 128  /* size of escape sequence buffer */
#define MAX_CUSTOM_ES_HANDLERS 32

/* Terminal private data */
struct RoteTermPrivate_ {
   bool escaped;              /* whether we are currently reading an
                               * escape sequence */

   bool graphmode;            /* whether terminal is in graphical 
                               * character mode or not */

   int scrolltop, scrollbottom;  /* current scrolling region of terminal */
   int saved_x, saved_y;         /* saved cursor position */

   char esbuf[ESEQ_BUF_SIZE]; /* 0-terminated string. Does NOT include
                               * the initial escape (\x1B) character. */
   int esbuf_len;             /* length of buffer. The following property
                               * is always kept: esbuf[esbuf_len] == '\0' */

   int pty;                   /* file descriptor for the pty attached to
                               * this terminal. -1 if none. */

   /* custom escape sequence handler */
   rote_es_handler_t handler;
};

#endif

