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

#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

static const char *keytable[KEY_MAX+1];
static int initialized = 0;

static void keytable_init();

void rote_vt_keypress(RoteTerm *rt, int keycode) {
   char c = (char) keycode;

   if (!initialized) keytable_init();

   if (keycode >= 0 && keycode < KEY_MAX && keytable[keycode])
      rote_vt_write(rt, keytable[keycode], strlen(keytable[keycode]));
   else
      rote_vt_write(rt, &c, 1); /* not special, just write it */
}

static void keytable_init() {
   initialized = 1;
   memset(keytable, 0, KEY_MAX+1 * sizeof(const char*));

   keytable['\n']          = "\r";
   keytable[KEY_UP]        = "\e[A";
   keytable[KEY_DOWN]      = "\e[B";
   keytable[KEY_RIGHT]     = "\e[C";
   keytable[KEY_LEFT]      = "\e[D";
   keytable[KEY_BACKSPACE] = "\b";
   keytable[KEY_HOME]      = "\e[1~";
   keytable[KEY_IC]        = "\e[2~";
   keytable[KEY_DC]        = "\e[3~";
   keytable[KEY_END]       = "\e[4~";
   keytable[KEY_PPAGE]     = "\e[5~";
   keytable[KEY_NPAGE]     = "\e[6~";
   keytable[KEY_SUSPEND]   = "\x1A";  /* Ctrl+Z gets mapped to this */
   keytable[KEY_F(1)]      = "\e[[A";
   keytable[KEY_F(2)]      = "\e[[B";
   keytable[KEY_F(3)]      = "\e[[C";
   keytable[KEY_F(4)]      = "\e[[D";
   keytable[KEY_F(5)]      = "\e[[E";
   keytable[KEY_F(6)]      = "\e[17~";
   keytable[KEY_F(7)]      = "\e[18~";
   keytable[KEY_F(8)]      = "\e[19~";
   keytable[KEY_F(9)]      = "\e[20~";
   keytable[KEY_F(10)]     = "\e[21~";
}


