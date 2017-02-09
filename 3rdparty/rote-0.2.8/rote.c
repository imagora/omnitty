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


#include "rote.h"
#include "roteprivate.h"
#include <stdlib.h>
#include <pty.h>
#include <stdio.h>
#include <string.h>

#define ROTE_VT_UPDATE_ITERATIONS 5

RoteTerm *rote_vt_create(int rows, int cols) {
   RoteTerm *rt;
   int i, j;

   if (rows <= 0 || cols <= 0) return NULL;

   if (! (rt = (RoteTerm*) malloc(sizeof(RoteTerm))) ) return NULL;
   memset(rt, 0, sizeof(RoteTerm));

   /* record dimensions */
   rt->rows = rows;
   rt->cols = cols;

   /* create the cell matrix */
   rt->cells = (RoteCell**) malloc(sizeof(RoteCell*) * rt->rows);
   for (i = 0; i < rt->rows; i++) {
      /* create row */
      rt->cells[i] = (RoteCell*) malloc(sizeof(RoteCell) * rt->cols);

      /* fill row with spaces */
      for (j = 0; j < rt->cols; j++) {
         rt->cells[i][j].ch = 0x20;    /* a space */
         rt->cells[i][j].attr = 0x70;  /* white text, black background */
      }
   }
   
   /* allocate dirtiness array */
   rt->line_dirty = (bool*) malloc(sizeof(bool) * rt->rows);

   /* initialization of other public fields */
   rt->crow = rt->ccol = 0;
   rt->curattr = 0x70;  /* white text over black background */

   /* allocate private data */
   rt->pd = (RoteTermPrivate*) malloc(sizeof(RoteTermPrivate));
   memset(rt->pd, 0, sizeof(RoteTermPrivate));

   rt->pd->pty = -1;  /* no pty for now */

   /* initial scrolling area is the whole window */
   rt->pd->scrolltop = 0;
   rt->pd->scrollbottom = rt->rows - 1;

   #ifdef DEBUG
   fprintf(stderr, "Created a %d x %d terminal.\n", rt->rows, rt->cols);
   #endif
   
   return rt;
}

void rote_vt_destroy(RoteTerm *rt) {
   int i;
   if (!rt) return;

   free(rt->pd);
   free(rt->line_dirty);
   for (i = 0; i < rt->rows; i++) free(rt->cells[i]);
   free(rt->cells);
   free(rt);
}

static void default_cur_set_attr(WINDOW *win, unsigned char attr) {
   int cp = ROTE_ATTR_BG(attr) * 8 + 7 - ROTE_ATTR_FG(attr);
   if (!cp) wattrset(win, A_NORMAL);
   else     wattrset(win, COLOR_PAIR(cp));

   if (ROTE_ATTR_BOLD(attr))     wattron(win, A_BOLD);
   if (ROTE_ATTR_BLINK(attr))    wattron(win, A_BLINK);
}

static inline unsigned char ensure_printable(unsigned char ch) 
                                        { return ch >= 32 ? ch : 32; }

void rote_vt_draw(RoteTerm *rt, WINDOW *win, int srow, int scol, 
                                void (*cur_set_attr)(WINDOW*,unsigned char)) {

   int i, j;
   rote_vt_update(rt);
   
   if (!cur_set_attr) cur_set_attr = default_cur_set_attr;
   for (i = 0; i < rt->rows; i++) {
      wmove(win, srow + i, scol);
      for (j = 0; j < rt->cols; j++) {
         (*cur_set_attr)(win, rt->cells[i][j].attr);
         waddch(win, ensure_printable(rt->cells[i][j].ch));
      }
   }

   wmove(win, srow + rt->crow, scol + rt->ccol);
}

pid_t rote_vt_forkpty(RoteTerm *rt, const char *command) {
   struct winsize ws;
   pid_t childpid;
   
   ws.ws_row = rt->rows;
   ws.ws_col = rt->cols;
   ws.ws_xpixel = ws.ws_ypixel = 0;

   childpid = forkpty(&rt->pd->pty, NULL, NULL, &ws);
   if (childpid < 0) return -1;

   if (childpid == 0) {
      /* we are the child, running under the slave side of the pty. */

      /* Cajole application into using linux-console-compatible escape
       * sequences (which is what we are prepared to interpret) */
      setenv("TERM", "linux", 1);

      /* Now we will exec /bin/sh -c command. */
      execl("/bin/sh", "/bin/sh", "-c", command, NULL);

      fprintf(stderr, "\nexecl() failed.\nCommand: '%s'\n", command);
      exit(127);  /* error exec'ing */
   }

   /* if we got here we are the parent process */
   rt->childpid = childpid;
   return childpid;
}

void rote_vt_forsake_child(RoteTerm *rt) {
   if (rt->pd->pty >= 0) close(rt->pd->pty);
   rt->pd->pty = -1;
   rt->childpid = 0;
}

void rote_vt_update(RoteTerm *rt) {
   fd_set ifs;
   struct timeval tvzero;
   char buf[512];
   int bytesread;
   int n = ROTE_VT_UPDATE_ITERATIONS;
   if (rt->pd->pty < 0) return;  /* nothing to pump */

   while (n--) { /* iterate at most ROVE_VT_UPDATE_ITERATIONS times.
                  * As Phil Endecott pointed out, if we don't restrict this,
                  * a program that floods the terminal with output
                  * could cause this loop to iterate forever, never
                  * being able to catch up. So we'll rely on the client
                  * calling rote_vt_update often, as the documentation
                  * recommends :-) */

      /* check if pty has something to say */
      FD_ZERO(&ifs); FD_SET(rt->pd->pty, &ifs);
      tvzero.tv_sec = 0; tvzero.tv_usec = 0;
   
      if (select(rt->pd->pty + 1, &ifs, NULL, NULL, &tvzero) <= 0)
         return; /* nothing to read, or select() failed */

      /* read what we can. This is guaranteed not to block, since
       * select() told us there was something to read. */
      bytesread = read(rt->pd->pty, buf, 512);
      if (bytesread <= 0) return;   

      /* inject the data into the terminal */
      rote_vt_inject(rt, buf, bytesread);
   }
}

void rote_vt_write(RoteTerm *rt, const char *data, int len) {
   if (rt->pd->pty < 0) {
      /* no pty, so just inject the data plain and simple */
      rote_vt_inject(rt, data, len);
      return;
   }

   /* write data to pty. Keep calling write() until we have written
    * everything. */
   while (len > 0) {
      int byteswritten = write(rt->pd->pty, data, len);
      if (byteswritten < 0) {
         /* very ugly way to inform the error. Improvements welcome! */
         static char errormsg[] = "\n(ROTE: pty write() error)\n";
         rote_vt_inject(rt, errormsg, strlen(errormsg));
         return;
      }

      data += byteswritten;
      len  -= byteswritten;
   }
}

void rote_vt_install_handler(RoteTerm *rt, rote_es_handler_t handler) {
   rt->pd->handler = handler;
}

void *rote_vt_take_snapshot(RoteTerm *rt) {
   int i;
   int bytes_per_row = sizeof(RoteCell) * rt->cols;
   void *buf = malloc(bytes_per_row * rt->rows);
   void *ptr = buf;

   for (i = 0; i < rt->rows; i++, ptr += bytes_per_row)
      memcpy(ptr, rt->cells[i], bytes_per_row);

   return buf;
}

void rote_vt_restore_snapshot(RoteTerm *rt, void *snapbuf) {
   int i;
   int bytes_per_row = sizeof(RoteCell) * rt->cols;

   for (i = 0; i < rt->rows; i++, snapbuf += bytes_per_row) {
      rt->line_dirty[i] = true;
      memcpy(rt->cells[i], snapbuf, bytes_per_row);
   }
}

int rote_vt_get_pty_fd(RoteTerm *rt) {
   return rt->pd->pty;
}

