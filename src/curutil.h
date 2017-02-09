/* curutil.h
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

#ifndef omnitty_curutil_h
#define omnitty_curutil_h

#include <ncurses.h>

/* This function initializes the curses color pairs (through init_pair()) 
 * according to the standard used by the ROTE library: if the foreground
 * color is f (0-7) and the background color is b (0-7), then the
 * corresponding color pair number is 8 * b + (7 - f). There is no
 * pair for foreground 7 with background 0 (since that is the default
 * color and doesn't need a pair) */
void curutil_colorpair_init();

/* Sets the curses attributes of the supplied window to the attribute
 * byte passed as parameter. The meaning of this byte-packed attribute
 * is defined in the ROTE library (see rote.h) */
void curutil_attrset(WINDOW *w, unsigned char attr);

/* Returns the size of the passed window in *width and *height. */
void curutil_window_size(WINDOW *w, int *width, int *height);

/* Fills the given window with the given character. */
void curutil_window_fill(WINDOW *w, int ch);

#endif


