/* menu.h
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

#ifndef omnitty_menu_h
#define omnitty_menu_h

#include <ncurses.h>

/* Initializes the menu system. minibuf is the window it should use
 * as "minibuffer", that is, the window where it will show prompts
 * and confirmation messages. */
void menu_init(WINDOW *minibuf);

/* Shows the Omnitty extended menu onscreen. After calling this function,
 * the screen will be dirty so you must touchwin() all your windows to
 * get them to redraw fully. */
void menu_show();

#endif

