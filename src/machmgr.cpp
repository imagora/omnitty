/* machmgr.c
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

#include "machmgr.h"
#include "curutil.h"
#include "machine.h"
#include "minibuf.h"
#include <string.h>
#include <alloca.h>

#define MACHINE_MAX 256

/* List of machines */
static Machine *machs[MACHINE_MAX];
static int machcount = 0;
static int selmach = 0;    /* currently selected machine */
static int scrollpos = 0;  /* machine being shown at the top of the list */
static int vtrows, vtcols;
bool multicast = false;    /* whether keystrokes are sent to all tagged
                            * machines or not */
static WINDOW *listwin;

void machmgr_init(WINDOW *lw, int vtr, int vtc) {
   machcount = selmach = scrollpos = 0;
   vtrows = vtr, vtcols = vtc;
   listwin = lw;
}

static void machmgr_setselmach(int s) {
   int screenwidth, screenheight;
   getmaxyx(listwin, screenheight, screenwidth);

   selmach = s;

   /* clamp selmach to bounds */
   if (selmach >= machcount) selmach = machcount - 1;
   if (selmach < 0) selmach = 0;

   /* in particular, if machcount == 0, selmach will be 0 */

   /* correct scrolling if needed */
   if (machcount > 0) {
      if (selmach < scrollpos) scrollpos = selmach;
      if (selmach >= scrollpos + screenheight)
         scrollpos = selmach - screenheight + 1;
   }
}

static void machmgr_selmach_validate() {
   machmgr_setselmach(selmach);
}

void machmgr_draw_list() {
   unsigned char attr;
   const char *p;
   int w, h;
   int i, j;

   werase(listwin);
   getmaxyx(listwin, h, w);

   for (i = scrollpos; i < scrollpos + h && i < machcount; i++) {
      /* decide color */
      attr = machs[i]->alive ? 0x70 : 0x80;
      if (i == selmach)  attr &= 0xF0, attr |= 0x01; /* red background */
      if (machs[i]->tag) attr &= 0x0F, attr |= machs[i]->alive ? 0xA0 : 0x20;
                                                     /* green foreground */
      
      curutil_attrset(listwin, attr);
      wmove(listwin, i - scrollpos, 0);

      waddch(listwin, machs[i]->tag ? '*' : ' ');

      /* now we have to print the first w-2 characters of machs[i]->name,
       * padding with spaces at the end if necessary to complete w-2
       * characters. We say w-2 because one character of the width was
       * used up when printing '*' and another one must be left blank
       * at the end */
      p = machs[i]->name; j = w - 2;
      while (j--) {
         waddch(listwin, *p ? *p : ' ');
         if (*p) p++;
      }
   }
}

void machmgr_add(const char *machname) {
   if (!*machname) return;
   if (machcount >= MACHINE_MAX) return;
   machs[machcount++] = machine_new(machname, vtrows, vtcols);
   machmgr_selmach_validate();
}

static void machmgr_delete(int index) {
   int i;
   if (index < 0 || index >= machcount) return;

   machine_destroy(machs[index]);
   for (i = index; i < machcount - 1; i++) machs[i] = machs[i+1];
   machcount--;

   machmgr_selmach_validate();
}

void machmgr_delete_tagged() {
   int i, j;

   i = 0; /* next position in the vector we will write to */
   j = 0; /* next position in the vector we will look at in search
           * of nontagged machines */
   while (j < machcount) {
      /* advance j to the next untagged machine; finish if no such thing.
       * Delete tagged machines on the way. */
      while (j < machcount && machs[j]->tag) machine_destroy(machs[j++]);

      if (j >= machcount) break;  /* no more machines */
      machs[i++] = machs[j++];
   }
   
   machcount = i;
   machmgr_selmach_validate();
}

void machmgr_delete_current() {
   machmgr_delete(selmach);
}

static void make_vt_summary(RoteTerm *rt, char *buf, int len) {
   int r = rt->crow, c = rt->ccol;
   int i;

   buf[len-1] = 0;
   for (i = len - 2; i >= 0; i--) {
      if (r > 0) {
         buf[i] = rt->cells[r][c].ch;
         if ((buf[i] >= 0 && buf[i] < 32) || buf[i] == 127) buf[i] = 32;
      }
      else buf[i] = 32;
      
      if (--c < 0) {
         r--, c = rt->cols - 1;
         if (r > 0) while (c > 0 && rt->cells[r][c-1].ch == 32) c--;
      }
   }
}

void machmgr_draw_summary(WINDOW *w) {
   int i;
   int sumheight, sumwidth;
   char sumbuf[80];
   werase(w);
   wmove(w, 0, 0);
   getmaxyx(w, sumheight, sumwidth);
   
   for (i = scrollpos; i < scrollpos + sumheight && i < machcount; i++) {
      curutil_attrset(w, 0x80);
      wmove(w, i - scrollpos, 0);

      make_vt_summary(machs[i]->vt, sumbuf, sumwidth);
      waddstr(w, sumbuf);
   }
}

void machmgr_draw_vt(WINDOW *w) {
   werase(w);
   if (selmach >= 0 && selmach < machcount)
      rote_vt_draw(machs[selmach]->vt, w, 0, 0, NULL);
}

void machmgr_update() {
   int i;
   for (i = 0; i < machcount; i++)
      rote_vt_update(machs[i]->vt);
}

void machmgr_rename(char *newname) {
   machine_rename(machs[selmach], newname);
   machmgr_update();
}

void machmgr_prev_machine() {
   machmgr_setselmach(selmach - 1);
}

void machmgr_next_machine() {
   machmgr_setselmach(selmach + 1);
}

void machmgr_forward_keypress(int k) {
   int i;

   if (multicast) {
      for (i = 0; i < machcount; i++)
         if (machs[i]->tag) rote_vt_keypress(machs[i]->vt, k);
   }
   else if (selmach >= 0 && selmach < machcount)
      rote_vt_keypress(machs[selmach]->vt, k);
}

void machmgr_handle_death(pid_t p) {
   int i;
   for (i = 0; i < machcount; i++) {
      if (machs[i]->pid == p) {
         machs[i]->alive = false;
         rote_vt_forsake_child(machs[i]->vt);
      }
   }
}

void machmgr_toggle_multicast() {
   multicast = !multicast;
}

bool machmgr_is_multicast() {
   return multicast;
}

void machmgr_toggle_tag_current() {
   if (selmach >= 0 && selmach < machcount)
      machs[selmach]->tag = !machs[selmach]->tag;
}

void machmgr_delete_all() {
   int i;
   for (i = 0; i < machcount; i++) machine_destroy(machs[i]);
   machcount = selmach = scrollpos = 0;
}

void machmgr_tag_all(bool ignore_dead) {
   int i;
   for (i = 0; i < machcount; i++)
      machs[i]->tag = (!ignore_dead || machs[i]->alive);
}

void machmgr_untag_all() {
   int i;
   for (i = 0; i < machcount; i++) machs[i]->tag = false;
}

void machmgr_delete_dead() {
   int i;
   for (i = machcount - 1; i >= 0; i--)
      if (!machs[i]->alive) machmgr_delete(i);
}

