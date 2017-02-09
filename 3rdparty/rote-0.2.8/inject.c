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
#include "inject_csi.h"
#include <string.h>

static void cursor_line_down(RoteTerm *rt) {
   int i;
   rt->crow++;
   rt->curpos_dirty = true;
   if (rt->crow <= rt->pd->scrollbottom) return;

   /* must scroll the scrolling region up by 1 line, and put cursor on 
    * last line of it */
   rt->crow = rt->pd->scrollbottom;
   
   for (i = rt->pd->scrolltop; i < rt->pd->scrollbottom; i++) {
      rt->line_dirty[i] = true;
      memcpy(rt->cells[i], rt->cells[i+1], sizeof(RoteCell) * rt->cols);
   }
      
   rt->line_dirty[rt->pd->scrollbottom] = true;

   /* clear last row of the scrolling region */
   for (i = 0; i < rt->cols; i++) {
      rt->cells[rt->pd->scrollbottom][i].ch = 0x20;
      rt->cells[rt->pd->scrollbottom][i].attr = 0x70;
   }

}

static void cursor_line_up(RoteTerm *rt) {
   int i;
   rt->crow--;
   rt->curpos_dirty = true;
   if (rt->crow >= rt->pd->scrolltop) return;

   /* must scroll the scrolling region up by 1 line, and put cursor on 
    * first line of it */
   rt->crow = rt->pd->scrolltop;
   
   for (i = rt->pd->scrollbottom; i > rt->pd->scrolltop; i--) {
      rt->line_dirty[i] = true;
      memcpy(rt->cells[i], rt->cells[i-1], sizeof(RoteCell) * rt->cols);
   }
      
   rt->line_dirty[rt->pd->scrolltop] = true;

   /* clear first row of the scrolling region */
   for (i = 0; i < rt->cols; i++) {
      rt->cells[rt->pd->scrolltop][i].ch = 0x20;
      rt->cells[rt->pd->scrolltop][i].attr = 0x70;
   }

}

static inline void put_normal_char(RoteTerm *rt, char c) {
   if (rt->ccol >= rt->cols) {
      rt->ccol = 0;
      cursor_line_down(rt);
   }

   rt->cells[rt->crow][rt->ccol].ch = c;
   rt->cells[rt->crow][rt->ccol].attr = rt->curattr;
   rt->ccol++;

   rt->line_dirty[rt->crow] = true;
   rt->curpos_dirty = true;
}

static inline void put_graphmode_char(RoteTerm *rt, char c) {
   char nc;
   /* do some very pitiful translation to regular ascii chars */
   switch (c) {
      case 'j': case 'k': case 'l': case 'm': case 'n': case 't': 
                                    case 'u': case 'v': case 'w':
         nc = '+'; break;
      case 'x':
         nc = '|'; break;
      default:
         nc = '%';
   }

   put_normal_char(rt, nc);
}

static inline void new_escape_sequence(RoteTerm *rt) {
   rt->pd->escaped = true;
   rt->pd->esbuf_len = 0;
   rt->pd->esbuf[0] = '\0';
}

static inline void cancel_escape_sequence(RoteTerm *rt) {
   rt->pd->escaped = false;
   rt->pd->esbuf_len = 0;
   rt->pd->esbuf[0] = '\0';
}

static void handle_control_char(RoteTerm *rt, char c) {
   switch (c) {
      case '\r': rt->ccol = 0; break; /* carriage return */
      case '\n':  /* line feed */
         rt->ccol = 0; cursor_line_down(rt);
         rt->curpos_dirty = true;
         break;
      case '\b': /* backspace */
         if (rt->ccol > 0) rt->ccol--;
         rt->curpos_dirty = true;
         break;
      case '\t': /* tab */
         while (rt->ccol % 8) put_normal_char(rt, ' ');
         break;
      case '\x1B': /* begin escape sequence (aborting previous one if any) */
         new_escape_sequence(rt);
         break;
      case '\x0E': /* enter graphical character mode */
         rt->pd->graphmode = true;
         break;
      case '\x0F': /* exit graphical character mode */
         rt->pd->graphmode = false;
         break;
      case '\x9B': /* CSI character. Equivalent to ESC [ */
         new_escape_sequence(rt);
         rt->pd->esbuf[rt->pd->esbuf_len++] = '[';
         break;
      case '\x18': case '\x1A': /* these interrupt escape sequences */
         cancel_escape_sequence(rt);
         break;
      case '\a': /* bell */
         /* do nothing for now... maybe a visual bell would be nice? */
         break;
      #ifdef DEBUG
      default:
         fprintf(stderr, "Unrecognized control char: %d (^%c)\n", c, c + '@');
         break;
      #endif
   }
}

static inline bool is_valid_csi_ender(char c) {
   return (c >= 'a' && c <= 'z') ||
          (c >= 'A' && c <= 'Z') ||
          c == '@' || c == '`';
}

static void try_interpret_escape_seq(RoteTerm *rt) {
   char firstchar = rt->pd->esbuf[0];
   char lastchar  = rt->pd->esbuf[rt->pd->esbuf_len-1];

   if (!firstchar) return;  /* too early to do anything */

   if (rt->pd->handler) {
      /* call custom handler */
      #ifdef DEBUG
      fprintf(stderr, "Calling custom handler for ES <%s>.\n", rt->pd->esbuf);
      #endif

      int answer = (*(rt->pd->handler))(rt, rt->pd->esbuf);
      if (answer == ROTE_HANDLERESULT_OK) {
         /* successfully handled */
         #ifdef DEBUG
         fprintf(stderr, "Handler returned OK. Done with escape sequence.\n");
         #endif

         cancel_escape_sequence(rt);
         return;
      }
      else if (answer == ROTE_HANDLERESULT_NOTYET) {
         /* handler might handle it when more characters are appended to 
          * it. So for now we don't interpret it */
         #ifdef DEBUG
         fprintf(stderr, "Handler returned NOTYET. Waiting for more chars.\n");
         #endif

         return;
      }
   
      /* If we got here then answer == ROTE_HANDLERESULT_NOWAY */
      /* handler said it can't handle that escape sequence,
       * but we can still try handling it ourselves, so 
       * we proceed normally. */
      #ifdef DEBUG
      fprintf(stderr, "Handler returned NOWAY. Trying our handlers.\n");
      #endif
   }

   /* interpret ESC-M as reverse line-feed */
   if (firstchar == 'M') {
      cursor_line_up(rt);
      cancel_escape_sequence(rt);
      return;
   }

   if (firstchar != '[' && firstchar != ']') {
      /* unrecognized escape sequence. Let's forget about it. */
      #ifdef DEBUG
      fprintf(stderr, "Unrecognized ES: <%s>\n", rt->pd->esbuf);
      #endif

      cancel_escape_sequence(rt);
      return;
   }

   if (firstchar == '[' && is_valid_csi_ender(lastchar)) {
      /* we have a csi escape sequence: interpret it */
      rote_es_interpret_csi(rt);
      cancel_escape_sequence(rt);
   }
   else if (firstchar == ']' && lastchar == '\a') {
      /* we have an xterm escape sequence: interpret it */

      /* rote_es_interpret_xterm_es(rt);     -- TODO!*/
      #ifdef DEBUG
      fprintf(stderr, "Ignored XTerm ES.\n");
      #endif
      cancel_escape_sequence(rt);
   }

   /* if the escape sequence took up all available space and could
    * not yet be parsed, abort it */
   if (rt->pd->esbuf_len + 1 >= ESEQ_BUF_SIZE) cancel_escape_sequence(rt);
}
   
void rote_vt_inject(RoteTerm *rt, const char *data, int len) {
   int i;
   for (i = 0; i < len; i++, data++) {
      if (*data == 0) continue;  /* completely ignore NUL */
      if (*data >= 1 && *data <= 31) {
         handle_control_char(rt, *data);
         continue;
      }

      if (rt->pd->escaped && rt->pd->esbuf_len < ESEQ_BUF_SIZE) {
         /* append character to ongoing escape sequence */
         rt->pd->esbuf[rt->pd->esbuf_len] = *data;
         rt->pd->esbuf[++rt->pd->esbuf_len] = 0;

         try_interpret_escape_seq(rt);
      }
      else if (rt->pd->graphmode)
         put_graphmode_char(rt, *data);
      else
         put_normal_char(rt, *data);
   }
}

