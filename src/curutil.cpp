/* curutil.c
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

#include "curutil.h"
#include <ncurses.h>

void curutil_attrset(WINDOW *win, unsigned char attr) {
   int fg = (attr & 0x70) >> 4;
   int bg = attr & 0x07;
   int cp = bg * 8 + 7 - fg;

   if (cp) wattrset(win, COLOR_PAIR(cp));
   else    wattrset(win, A_NORMAL);
   
   if (attr & 0x80) wattron(win, A_BOLD);
   if (attr & 0x08) wattron(win, A_BLINK);
}

void curutil_colorpair_init() {
   int f, b, cp;
   for (f = 0; f < 8; f++) for (b = 0; b < 8; b++) {
      cp = b*8+7-f;
      if (cp) init_pair(cp, f, b);
   }
}

void curutil_window_fill(WINDOW *win, int ch) {
   int h, w;
   int r, c;

   getmaxyx(win, h, w);
   wmove(win, 0, 0);
   for (r = 0; r < h; r++) for (c = 0; c < w; c++)
      waddch(win, ch);

   wmove(win, 0, 0);
}

