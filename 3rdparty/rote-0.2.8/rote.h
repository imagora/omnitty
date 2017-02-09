/* ROTE - Our Own Terminal Emulation library 
 * Copyright (c) 2004 Bruno T. C. de Oliveira
 * All rights reserved
 *
 * 2004-08-25
 */

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


#ifndef btco_ROTE_rote_h
#define btco_ROTE_rote_h

#include <ncurses.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

/* Color codes: 0 = black, 1 = red, 2 = green, 3 = yellow, 4 = blue,
 *              5 = magenta, 6 = cyan, 7 = white. 
 *
 * An 'attribute' as used in this library means an 8-bit value that conveys
 * a foreground color code, a background color code, and the bold
 * and blink bits. Each cell in the virtual terminal screen is associated
 * with an attribute that specifies its appearance. The bits of an attribute,
 * from most significant to least significant, are 
 *
 *  bit:      7 6 5 4 3 2 1 0
 *  content:  S F F F H B B B
 *            | `-,-' | `-,-'
 *            |   |   |   |
 *            |   |   |   `----- 3-bit background color code (0 - 7)
 *            |   |   `--------- blink bit (if on, text is blinking)
 *            |   `------------- 3-bit foreground color code (0 - 7)
 *            `----------------- bold bit
 *
 * It is however recommended that you use the provided macros rather
 * than dealing with this format directly.
 *
 * Sometimes we will call the 'SFFF' nibble above the 'extended
 * foreground color code', and the 'HBBB' nibble the 'extended background
 * color code'. So the extended color codes are just the regular
 * color codes except that they have an additional bit (the most significant
 * bit) indicating bold/blink.
 */

/* retrieve attribute fields */
#define ROTE_ATTR_BG(attr)              ((attr) & 0x07)
#define ROTE_ATTR_FG(attr)              (((attr) & 0x70) >> 4)

/* retrieve 'extended' color codes (see above for info) */
#define ROTE_ATTR_XBG(attr)             ((attr) & 0x0F)
#define ROTE_ATTR_XFG(attr)             (((attr) & 0xF0) >> 4)

/* set attribute fields. This requires attr to be an lvalue, and it will
 * be evaluated more than once. Use with care. */
#define ROTE_ATTR_MOD_BG(attr, newbg)    attr &= 0xF8, attr |= (newbg)
#define ROTE_ATTR_MOD_FG(attr, newfg)    attr &= 0x8F, attr |= ((newfg) << 4)
#define ROTE_ATTR_MOD_XBG(attr, newxbg)  attr &= 0xF0, attr |= (newxbg)
#define ROTE_ATTR_MOD_XFG(attr, newxfg)  attr &= 0x0F, attr |= ((newxfg) << 4)
#define ROTE_ATTR_MOD_BOLD(attr, boldbit) \
                               attr &= 0x7F, attr |= (boldbit)?0x80:0x00
#define ROTE_ATTR_MOD_BLINK(attr, blinkbit) \
                               attr &= 0xF7, attr |= (blinkbit)?0x08:0x00

/* these return non-zero for 'yes', zero for 'no'. Don't rely on them being 
 * any more specific than that (e.g. being exactly 1 for 'yes' or whatever). */
#define ROTE_ATTR_BOLD(attr)            ((attr) & 0x80)
#define ROTE_ATTR_BLINK(attr)           ((attr) & 0x08)

/* Represents each of the text cells in the terminal screen */
typedef struct RoteCell_ {
   unsigned char ch;    /* >= 32, that is, control characters are not
                         * allowed to be on the virtual screen */

   unsigned char attr;  /* a color attribute, as described previously */
} RoteCell;

/* Declaration of opaque rote_Term_Private structure */
typedef struct RoteTermPrivate_ RoteTermPrivate;

/* Represents a virtual terminal. You may directly access the fields
 * of this structure, but please pay close attention to the fields
 * marked read-only or with special usage notes. */
typedef struct RoteTerm_ {
   int rows, cols;              /* terminal dimensions, READ-ONLY. You
                                 * can't resize the terminal by changing
                                 * this (a segfault is about all you will 
                                 * accomplish). */

   RoteCell **cells;            /* matrix of cells. This
                                 * matrix is indexed as cell[row][column]
                                 * where 0 <= row < rows and
                                 *       0 <= col < cols
                                 *
                                 * You may freely modify the contents of
                                 * the cells.
                                 */

   int crow, ccol;              /* cursor coordinates. READ-ONLY. */

   unsigned char curattr;       /* current attribute, that is the attribute
                                 * that will be used for newly inserted
                                 * characters */

   pid_t childpid;              /* pid of the child process running in the
                                 * terminal; 0 for none. This is READ-ONLY. */

   RoteTermPrivate *pd;         /* private state data */

   /* --- dirtiness flags: the following flags will be raised when the
    * corresponding items are modified. They can only be unset by YOU
    * (when, for example, you redraw the term or something) --- */
   bool curpos_dirty;           /* whether cursor location has changed */
   bool *line_dirty;            /* whether each row is dirty  */
   /* --- end dirtiness flags */
} RoteTerm;

/* Creates a new virtual terminal with the given dimensions. You
 * must destroy it with rote_vt_destroy after you are done with it.
 * The terminal will be initially blank and the cursor will
 * be at the top-left corner. 
 *
 * Returns NULL on error.
 */
RoteTerm *rote_vt_create(int rows, int cols);

/* Destroys a virtual terminal previously created with
 * rote_vt_create. If rt == NULL, does nothing. */
void rote_vt_destroy(RoteTerm *rt);

/* Starts a forked process in the terminal. The <command> parameter
 * is a shell command to execute (it will be interpreted by '/bin/sh -c') 
 * Returns the pid of the forked process. 
 *
 * Some useful reminders: If you want to be notified when child processes exit,
 * you should handle the SIGCHLD signal.  If, on the other hand, you want to
 * ignore exitting child processes, you should set the SIGCHLD handler to
 * SIG_IGN to prevent child processes from hanging around the system as 'zombie
 * processes'. 
 *
 * Continuing to write to a RoteTerm whose child process has died does not
 * accomplish a lot, but is not an error and should not cause your program
 * to crash or block indefinitely or anything of that sort :-)
 * If, however, you want to be tidy and inform the RoteTerm that its
 * child has died, call rote_vt_forsake_child when appropriate.
 *
 * If there is an error, returns -1. Notice that passing an invalid
 * command will not cause an error at this level: the shell will try
 * to execute the command and will exit with status 127. You can catch
 * that by installing a SIGCHLD handler if you want.
 */
pid_t rote_vt_forkpty(RoteTerm *rt, const char *command);

/* Disconnects the RoteTerm from its forked child process. This function
 * should be called when the child process dies or something of the sort.
 * It is not strictly necessary to call this function, but it is
 * certainly tidy. */
void rote_vt_forsake_child(RoteTerm *rt);

/* Does some data plumbing, that is, sees if the sub process has
 * something to write to the terminal, and if so, write it. If you
 * called rote_vt_fork to start a forked process, you must call
 * this function regularly to update the terminal. 
 *
 * This function will not block, that is, if there is no data to be
 * read from the child process it will return immediately. */
void rote_vt_update(RoteTerm *rt);

/* Puts data into the terminal: if there is a forked process running,
 * the data will be sent to it. If there is no forked process,
 * the data will simply be injected into the terminal (as in
 * rote_vt_inject) */
void rote_vt_write(RoteTerm *rt, const char *data, int length);

/* Inject data into the terminal. <data> needs NOT be 0-terminated:
 * its length is solely determined by the <length> parameter. Please
 * notice that this writes directly to the terminal, that is,
 * this function does NOT send the data to the forked process
 * running in the terminal (if any). For that, you might want
 * to use rote_vt_write.
 */
void rote_vt_inject(RoteTerm *rt, const char *data, int length);

/* Paints the virtual terminal screen on the given window, putting
 * the top-left corner at the given position. The cur_set_attr
 * function must set the curses attributes given a Rote attribute
 * byte. It should, for example, do wattrset(win, COLOR_PAIR(n)) where
 * n is the colorpair appropriate for the attribute and such.
 *
 * If you pass NULL for cur_set_attr, the default implementation will
 * set the color pair given by (bg * 8 + 7 - fg), which seems to be
 * a common mapping, and the bold and blink attributes will be mapped 
 * to A_BOLD and A_BLINK.
 *
 * At the end of the function, the cursor will be left where the virtual 
 * cursor of the terminal is supposed to be.
 *
 * This function does not call wrefresh(win); you have to do that yourself.
 * This function automatically calls rote_vt_update prior to drawing
 * so that the drawn contents are accurate.
 */
void rote_vt_draw(RoteTerm *rt, WINDOW *win, int startrow, int startcol,
                  void (*cur_set_attr)(WINDOW *win, unsigned char attr));

/* Indicates to the terminal that the given key has been pressed.
 * This will cause the terminal to rote_vt_write() the appropriate
 * escape sequence for that key (that is, the escape sequence
 * that the linux text-mode console would produce for it). The argument,
 * keycode, must be a CURSES EXTENDED KEYCODE, the ones you get
 * when you use keypad(somewin, TRUE) (see man page). */
void rote_vt_keypress(RoteTerm *rt, int keycode);

/* Takes a snapshot of the current contents of the terminal and
 * saves them to a dynamically allocated buffer. Returns a pointer
 * to the newly created buffer, which you can pass to
 * rote_vt_restore_snapshot. Caller is responsible for free()'ing when
 * the snapshot is no longer needed. */
void *rote_vt_take_snapshot(RoteTerm *rt);

/* Restores a snapshot previously taken with rote_vt_take_snapshot.
 * This function does NOT free() the passed buffer */
void rote_vt_restore_snapshot(RoteTerm *rt, void *snapbuf);

/* Returns the pseudo tty descriptor associated with the given terminal.
 * Please don't do weird things with it (like close it for instance),
 * or things will break 
 * 
 * This function returns -1 if the given terminal does not yet have
 * an associated pty. A pty is only associated to a terminal when
 * needed, e.g. on a call to rote_vt_forkpty. */
int rote_vt_get_pty_fd(RoteTerm *rt);

/* Declaration of custom escape sequence callback type. See the
 * rote_vt_add_es_handler function for more info */
typedef int (*rote_es_handler_t)(RoteTerm *rt, const char *es);

/* Installs a custom escape sequence handler for the given RoteTerm.
 * The handler will be called by the library every time it tries to
 * recognize an escape sequence; depending on the return value of the
 * handler, it will proceed in a different manner. See the description
 * of the possible return values (ROTE_HANDLERESULT_* constants) below
 * for more info.
 *
 * This handler will be called EACH TIME THE ESCAPE SEQUENCE BUFFER
 * RECEIVES A CHARACTER. Therefore, it must execute speedily in order
 * not to create too heavy a performance penalty. In particular, the
 * writer of the handler should take care to quickly test for invalid
 * or incomplete escape sequences before trying to do more elaborate
 * parsing.
 *
 * The handler will NOT be called with an empty escape sequence (i.e.
 * one in which only the initial ESC was received).
 *
 * The custom handler receives the terminal it pertains to and the
 * escape sequence as a string (without the initial escape character).
 *
 * The handler may of course modify the terminal as it sees fit, taking 
 * care not to corrupt it of course (in particular, it should appropriately 
 * raise the line_dirty[] and curpos_dirty flags to indicate what it has 
 * changed).
 */
void rote_vt_install_handler(RoteTerm *rt, rote_es_handler_t handler);
                            
/* Possible return values for the custom handler function and their
 * meanings: */
#define ROTE_HANDLERESULT_OK 0      /* means escape sequence was handled */

#define ROTE_HANDLERESULT_NOTYET 1  /* means the escape sequence was not
                                     * recognized yet, but there is hope that
                                     * it still will once more characters
                                     * arrive (i.e. it is not yet complete).
                                     *
                                     * The library will thus continue collecting
                                     * characters and calling the handler as
                                     * each character arrives until
                                     * either OK or NOWAY is returned.
                                     */

#define ROTE_HANDLERESULT_NOWAY 2   /* means the escape sequence was not
                                     * recognized, and there is no chance
                                     * that it will even if more characters
                                     * are added to it. */

#endif

