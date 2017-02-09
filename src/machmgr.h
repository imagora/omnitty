/* machmgr.h
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

#ifndef omnitty_machmgr_h
#define omnitty_machmgr_h

#include <ncurses.h>
#include <sys/types.h>

/* Initialize the machine manager. The given window will be used as the
 * "list window", that is, the window that will display the list of the
 * available machines. The vtrows,vtcols arguments specify the
 * size of the virtual terminal of the machines. It needs to know this
 * because it will handle machine creation when you call machmgr_add()
 * for example, and it needs to pass those dimensions to the machine-creating
 * functions (e.g. machine_new) */
void machmgr_init(WINDOW *w, int vtrows, int vtcols);

/* Draws the machine list onto the list window */
void machmgr_draw_list();

/* Draws the summary area in the passed window. The "summary" consists of
 * a few characters for each machine, and those characters give the user an 
 * idea of what is going on in that machine's terminal. Those characters
 * will be the characters that are "near" the cursor. This is difficult
 * to define, so see the implementation :-) 
 *
 * The supplied window should be on either side of the list window, and
 * vertically aligned with it (i.e. its top should match the list window's top,
 * same for bottom), because this function will draw the summaries that 
 * correspond to each machine in the list window in the corresponding line
 * of the supplied window.
 *
 *  list win     summary window (the one you supply to this function)
 * +----------+--------------------------+
 * | mach 1   | summary for machine 1    |
 * | mach 2   | summary for machine 2    |
 * | mach 3   | summary for machine 3    |
 * | ...      | ...                      |
 * +----------+--------------------------+
 */
void machmgr_draw_summary(WINDOW *w);

/* Draws the virtual terminal for the currently selected machine in the
 * given window. Assumes the dimensions of the given window match the
 * vtrows,vtcols arguments passed to machmgr_init. */
void machmgr_draw_vt(WINDOW *w);

/* Adds a new machine to the machine manager given its name. Takes care
 * of creating the machine and adding it to the list. */
void machmgr_add(const char *);

/* Deletes currently selected machine. */
void machmgr_delete_current();

/* Updates the virtual terminals of all machines. This function should
 * be called regularly. */
void machmgr_update();

/* Toggles multicast mode. When multicast mode is on, machmgr_forward_keypress
 * will send the keypress to all tagged machines; when multicast mode is
 * off, machmgr_forward_keypress will send keypresses only to the currently
 * selected machine. 
 *
 * Multicast mode is initially off.
 */
void machmgr_toggle_multicast();

/* Returns whether multicast mode is on. */
bool machmgr_is_multicast();

/* Toggles the 'tagged' state of the currently selected machine */
void machmgr_toggle_tag_current();

/* Deletes all the machines! */
void machmgr_delete_all();

/* Deletes all dead machines, that is, all machines whose 'alive' flag
 * is false. A machine's 'alive' flag is dropped when you call
 * machmgr_handle_death(pid) with a pid matching that machine's child
 * ssh process. Ordinarily the main program should monitor SIGCHLD signals
 * and call machmgr_handle_death appropriately when it detects the death
 * of a child process. */
void machmgr_delete_dead();

/* Deletes all tagged machines */
void machmgr_delete_tagged();

/* Tags all machines. If ignore_dead, does not tag dead machines (i.e. machines
 * whose alive flag is down). */
void machmgr_tag_all(bool ignore_dead);

/* Resets the 'tagged' attribute of all machines */
void machmgr_untag_all();

/* Moves the selection to the previous machine; that is, makes the previous 
 * machine in the list the selected machine. */
void machmgr_prev_machine();

/* Moves to the next machine, that is, makes the next machine in the list
 * the selected machine. */
void machmgr_next_machine();

/* Forwards the given keypress to the appropriate machines. If multicast
 * mode is on, the keypress will be forwarded to all tagged machines;
 * otherwise, it will be directed only to the currently selected machine. */
void machmgr_forward_keypress(int k);

/* Handles the death of PID p. This will check if that PID matches the PID
 * of the child ssh process of any of the machines registered in the manager.
 * If so, it will mark that machine as dead. */
void machmgr_handle_death(pid_t p);

/* Rename the currently selected machine */
void machmgr_rename(char *newname);

#endif

