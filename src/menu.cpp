/* menu.c
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

#include "menu.h"
#include "curutil.h"
#include "machmgr.h"
#include <ncurses.h>
#include <stdlib.h>
#include "help.h"

#define MENU_LINES 12
#define MENU_COLS  38

#define MENU_CONTENTS \
                    "{[h]} online help\n" \
		    "{[r]} rename machine\n" \
                    "{[t]} tag all machines (live only)\n" \
                    "{[T]} tag all machines (live & dead)\n" \
                    "{[u]} untag all machines\n" \
                    "{[z]} delete dead machines\n" \
                    "{[d]} delete all TAGGED machines\n" \
                    "{[X]} delete all machines\n" \
                    "{[q]} quit application\n"

#include "minibuf.h"

static WINDOW *minibuf = NULL;

void menu_init(WINDOW *minibuf_win) {
   minibuf = minibuf_win;
}

void menu_show() {
   int termwidth, termheight;
   char buf[16];
   const char *p; int i; int ch;
   WINDOW *w;

   getmaxyx(stdscr, termheight, termwidth);
   w = newwin(MENU_LINES, MENU_COLS, (termheight - MENU_LINES)/2,
                                     (termwidth  - MENU_COLS )/2);
   werase(w);
   curutil_attrset(w, 0xF4);
   curutil_window_fill(w, ' ');
   
   wborder(w, 0, 0, 0, 0, 0, 0, 0, 0);
   wmove(w, 0, 3); waddstr(w, "[ Menu ]");
   
   wmove(w, i = 1, 1); 
   p = MENU_CONTENTS;
   while (*p) {
      if      (*p == '{')  curutil_attrset(w, 0xE4);
      else if (*p == '}')  curutil_attrset(w, 0xF4);
      else if (*p == '\n') wmove(w, ++i, 1);
      else                 waddch(w, *p);
      p++;
   }
   
   wmove(w, ++i, 1);
   curutil_attrset(w, 0xE4);
   waddstr(w, "Select an option: ");
   wrefresh(w);

   while (0 > (ch = getch())) ;

   switch (ch) {
      case 'h': help_show(); break;
      case 'r': 
		*buf = 0;
		minibuf_prompt(minibuf,
			"Enter new machine name: ", 0x90, buf, 15);
		machmgr_rename(buf);
		break;
      case 'q': *buf = 0;
                if (minibuf_prompt(minibuf, 
                      "Really quit application [y/n]?", 0x90, buf, 2)
                       && (*buf == 'y' || *buf == 'Y')) {
                   endwin();
                   exit(0);
                }

                break;
      case 't': machmgr_tag_all(true); break;
      case 'T': machmgr_tag_all(false); break;
      case 'u': machmgr_untag_all(); break;
      case 'z': machmgr_delete_dead(); break;
      case 'd': *buf = 0;
                if (minibuf_prompt(minibuf, 
                      "Really delete all TAGGED machines [y/n]?", 0x90, buf, 2)
                       && (*buf == 'y' || *buf == 'Y')) machmgr_delete_tagged();
                break;
      case 'X': *buf = 0;
                if (minibuf_prompt(minibuf, 
                      "Really delete ALL machines [y/n]?", 0x90, buf, 2)
                       && (*buf == 'y' || *buf == 'Y')) machmgr_delete_all();
                break;
   }
   
   delwin(w);
}

