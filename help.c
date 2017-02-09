/* help.c
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

#include "help.h"
#include "curutil.h"
#include <ncurses.h>
#include <stdlib.h>

#define HELP_LINES 25
#define HELP_COLS  80

#define HELP_CONTENTS \
 "Omnitty allows you to ssh into several machines at once. The list on the\n" \
 "shows the machines you are logged into. Every key you type will be sent\n" \
 "to the currently selected machine, except the following {control keys}:\n" \
 "\n" \
 "{F1}: brings up a menu of extended options (including {quit})\n" \
 "{F2} and {F3}: move between machines on the list. The terminal window\n" \
 "      on the right shows the display of the currently selected machine.\n" \
 "{F4}: tag current machine. This will put a mark on the machine's name.\n" \
 "      The mark by itself does nothing, but some features operate on\n" \
 "      'all tagged machines', so this is a way to indicate which machines\n" \
 "      they are to operate on. The {F1} menu has additional tag commands.\n" \
 "{F5}: allows you to add a machine to the list. You can either supply the\n" \
 "      name of the machine you want to add, or type {@file} where {file}\n" \
 "      is the name of a file that contains machine names one per line,\n" \
 "      in which case all machines in that file will be added.\n" \
 "{F6}: delete current machine. If you want to delete multiple machines,\n" \
 "      you can also tag them and choose {delete all tagged machines} from\n" \
 "      the {F1} menu.\n" \
 "{F7}: toggles between {singlecast} and {multicast} mode. In singlecast\n" \
 "      mode, the keys you type will be directed to the currently selected\n" \
 "      machine, regardless of tags. When {multicast} is selected, the\n" \
 "      keys will be sent to {all tagged machines}, allowing you to operate\n"\
 "      on several machines at once.\n"

void help_show() {
   int termwidth, termheight;
   const char *p; int i; int ch;
   WINDOW *w;

   getmaxyx(stdscr, termheight, termwidth);
   w = newwin(HELP_LINES, HELP_COLS, (termheight - HELP_LINES)/2,
                                     (termwidth  - HELP_COLS )/2);
   werase(w);
   curutil_attrset(w, 0xF4);
   curutil_window_fill(w, ' ');
   
   wborder(w, 0, 0, 0, 0, 0, 0, 0, 0);
   wmove(w, 0, 3); waddstr(w, "[ Help ]");
   
   wmove(w, i = 1, 1); 
   p = HELP_CONTENTS;
   while (*p) {
      if      (*p == '{')  curutil_attrset(w, 0xE4);
      else if (*p == '}')  curutil_attrset(w, 0xF4);
      else if (*p == '\n') wmove(w, ++i, 1);
      else                 waddch(w, *p);
      p++;
   }
   
   wrefresh(w);
   while (0 > (ch = getch())) ;
   delwin(w);
}

