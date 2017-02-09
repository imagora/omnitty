/* minibuf.h
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

#ifndef omnitty_minibuf_h
#define omnitty_minibuf_h

#include <stdbool.h>
#include <ncurses.h>

/* caution: the following function uses what is already in the buffer
 * as the default content of the field, so it absolutely must not
 * contain trash (and must be 0-terminated). 
 *
 * If you want to start with an empty buffer, do something like *buf = 0
 * before calling.
 *
 * Returns whether used confirmed the input, that is whether the
 * user ended the input with ENTER. If the user exitted with ESC
 * any other 'cancel-key', the return value will be false.
 */
bool minibuf_prompt(WINDOW *w, const char *prompt, unsigned char attr, 
                               char *buf, int len);

/* displays a message in the window, waits for the
 * user to acknowledge it and returns */
void minibuf_msg(WINDOW *w, const char *msg, unsigned char attr);

/* displays a message in the minibuffer and returns immediately.
 * Remember to call this function with an empty (or NULL) message
 * to erase it. */
void minibuf_put(WINDOW *w, const char *msg, unsigned char attr);

#endif

