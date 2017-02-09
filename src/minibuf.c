/* minibuf.c
 *
 * Omnitty SSH Multiplexer
 * Copyright (c) 2004 Bruno Takahashi C. de Oliveira
 * All rights reserved.
 *
 * LICENSE INFORMATION:
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 * Copyright (c) 2002 Bruno T. C. de Oliveira
 */

#include "minibuf.h"
#include "curutil.h"
#include <string.h>

bool minibuf_prompt(WINDOW *w, const char *prompt, unsigned char attr, 
                               char *buf, int len) {
   int pos = strlen(buf);
   int ch;
   int decision = 0;

   while (!decision) {
      werase(w);
      wmove(w, 0, 0);
      curutil_attrset(w, attr);
      waddstr(w, prompt);
      curutil_attrset(w, 0x70);
      waddstr(w, buf);
      wrefresh(w);
      
      ch = getch();
      if (ch < 0) continue;
      
      switch (ch) {
         case 127: case KEY_BACKSPACE: case '\b': 
            if (pos) pos--; break; /* bs */
         case ('U'-'A'+1):
            pos = 0; break;        /* ^U */
         
         case '\r': case '\n':
            decision = 1;   /* Enter */
            break;

         case ('C'-'A'+1): case ('G'-'A'+1): case 0x1B:
            decision = -1;  /* cancel */
            break;
      }

      if (ch >= 32 && ch != 127 && ch < 256 && pos + 1 < len)
         /* regular char, and we have room: put it in the buffer */
         buf[pos++] = ch;

      buf[pos] = 0;
   }
   
   werase(w);
   wrefresh(w);
   return decision == 1;
}

void minibuf_msg(WINDOW *w, const char *msg, unsigned char attr) {
   minibuf_put(w, msg, attr);
   wrefresh(w);
   while (ERR == getch()) ;
   wclear(w);
}

void minibuf_put(WINDOW *w, const char *msg, unsigned char attr) {
   werase(w);
   wmove(w, 0, 0);

   if (msg) {
      curutil_attrset(w, attr);
      waddstr(w, msg);
   }

   wrefresh(w);
}


