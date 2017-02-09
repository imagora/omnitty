/* machine.h
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

#ifndef omnitty_machine_h
#define omnitty_machine_h

#include <rote/rote.h>
#define TAGSTACK_SIZE 8

/* This structure represents each machine the program interacts with */
typedef struct Machine_ {
   char *name;  /* name of the machine */
   bool tag;    /* whether the machine is 'tagged' */

   bool alive;  /* initially set to true; set to false when 
                 * main program notifies that a certain pid has died and
                 * it matches this machine's ssh pid */

   RoteTerm *vt; /* the machine's virtual terminal (ROTE library) */
   pid_t pid;    /* pid of ssh process running in terminal */

   /* the following stack is used for storing the 'tagged' state for
    * later retrieval */
   bool tagstack[TAGSTACK_SIZE];
   int tagstack_count;
} Machine;

/* Creates a new machine with the given name and virtual terminal dimensions.
 * Returns a pointer to the newly created machine. The machine must be
 * destroyed with machine_destroy after use. This function starts
 * a child ssh process to connect to the machine and puts its PID
 * in the pid field of the returned machine structure. The ssh runs
 * in the RoteTerm virtual terminal, whose address is also returned in
 * the structure. */
Machine *machine_new(const char *name, int vtrows, int vtcols);

/* Destroyes a machine previously created my machine_new. */
void machine_destroy(Machine*);

/* Save/restore machine's 'tagged' state.  */
void machine_tag_push(Machine*);
void machine_tag_pop(Machine*);

/* Rename a machine */
void machine_rename(Machine*, char*);

#endif

