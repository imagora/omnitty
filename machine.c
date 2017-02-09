/* machine.c
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

#include "machine.h"
#include <string.h>
#include <stdlib.h>

#define CMD_FORMAT "/usr/bin/ssh %s"

Machine *machine_new(const char *name, int vtrows, int vtcols) {
   static char cmd[128];
   Machine *m = malloc(sizeof(Machine));
   memset(m, 0, sizeof(Machine));
   
   m->alive = true;
   m->name = strdup(name);
   m->vt = rote_vt_create(vtrows, vtcols);

   /* build the command line and fork an ssh to the given machine */
   if (120 < snprintf(cmd, 120, CMD_FORMAT, m->name)) abort();
   m->pid = rote_vt_forkpty(m->vt, cmd);

   return m;
}

void machine_destroy(Machine *m) {
   if (!m) return;
   free(m->name);
   rote_vt_destroy(m->vt);
   free(m);
}

void machine_rename(Machine *m, char *newname) {
 
  if (!m) return;
  free(m->name);
  m->name = strdup(newname);
}

void machine_tag_push(Machine *m) {
   if (m->tagstack_count >= TAGSTACK_SIZE) return;
   m->tagstack[m->tagstack_count++] = m->tag;
}

void machine_tag_pop(Machine *m) {
   if (!m->tagstack_count) return;
   m->tag = m->tagstack[--m->tagstack_count];
}

