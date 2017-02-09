/* main.c
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

#include <ncurses.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

#include "curutil.h"
#include "machine.h"
#include "machmgr.h"
#include "minibuf.h"
#include "menu.h"

/* minimum terminal dimensions to run program */
#define MIN_REQUIRED_WIDTH 80
#define MIN_REQUIRED_HEIGHT 25

#define REMINDER_LINE "OmNiTTY-R v" OMNITTY_VERSION \
                      "  \007F1\007:menu  \006F2/3\007:sel  \003F4\007:tag" \
                      "  \002F5\007:add  \001F6\007:del" \
                      "  \005F7\007:mcast"

#define SPLASH_LINE_1 "OmNiTTY Reborn v" OMNITTY_VERSION
#define SPLASH_LINE_2 "Copyright (c) 2004 Bruno T. C. de Oliveira"

/* how many characters wide the list window will be, by default */
#define LISTWIN_DEFAULT_CHARS 8
#define TERMWIN_DEFAULT_CHARS 80
#define TERMWIN_MIN 80

#define RTFM "Syntax: omnitty [-W list_width] [-T term_width]\n" \
             "\n" \
             "     -W        specifies width of machine list area\n" \
             "               (default is 8 characters)\n" \
             "\n" \
             "     -T        specifies width of terminal area\n" \
             "               (default is 80 characters)\n" \
             "\n"

static WINDOW *listwin   = NULL;
static WINDOW *sumwin    = NULL;
static WINDOW *vtwin     = NULL;
static WINDOW *minibuf = NULL;
static volatile int zombie_count = 0;

void sigchld_handler(int signo) { zombie_count++; }

void curses_init() {
   int w, h, i = 0;

   initscr();
   start_color();
   noecho();
   keypad(stdscr, TRUE);
   timeout(200);
   raw();
   curutil_colorpair_init();
   clear();

   /* register some alternate escape sequences for the function keys,
    * to improve compatibility with several types of terminal emulators */
   define_key("\eOP",   KEY_F(1)); define_key("\eOQ",   KEY_F(2));
   define_key("\eOR",   KEY_F(3)); define_key("\eOS",   KEY_F(4));
   define_key("\e[11~", KEY_F(1)); define_key("\e[12~", KEY_F(2));
   define_key("\e[13~", KEY_F(3)); define_key("\e[14~", KEY_F(4));
   define_key("\e[15~", KEY_F(5)); define_key("\e[17~", KEY_F(6));
   define_key("\e[18~", KEY_F(7)); define_key("\e[19~", KEY_F(8));
   define_key("\e[20~", KEY_F(9)); define_key("\e[21~", KEY_F(10));

   getmaxyx(stdscr, h, w);
   if (h < MIN_REQUIRED_HEIGHT || w < MIN_REQUIRED_WIDTH) {
      endwin();
      fprintf(stderr, "ERROR: omnitty requires a %d x %d terminal to run.\n",
                        MIN_REQUIRED_WIDTH, MIN_REQUIRED_HEIGHT);
      exit(1);
   }
   
   wmove(stdscr, h/2, (w - strlen(SPLASH_LINE_1))/2);
   curutil_attrset(stdscr, 0x40);
   waddstr(stdscr, SPLASH_LINE_1);

   curutil_attrset(stdscr, 0x70);
   wmove(stdscr, h/2 + 1, (w - strlen(SPLASH_LINE_2))/2);
   waddstr(stdscr, SPLASH_LINE_2);

   wrefresh(stdscr);
   while (getch() < 0 && i < 10) i++;

   wclear(stdscr);
   wrefresh(stdscr);
}

/* Window layout:
 * 
 *      list    summary     terminal window
 *     window   window
 *    |-------|--------X|--------------------------------| 
 *    0       A        BC                             termcols-1
 *
 * A = list_win_chars + 2
 */
void wins_init(int *vtrows, int *vtcols, int list_win_chars, 
                                                int term_win_chars) {
   int termcols, termrows, A, B, C;
   const char *p;

   /* obtain terminal dimensions */
   getmaxyx(stdscr, termrows, termcols);

   /* the geometry is hard-coded here, but nowhere else... so I don't
    * see a lot of point using #defines or anything any more sophisticated */
   A = list_win_chars + 2;
   C = termcols - term_win_chars;
   B = C-1;
   if (B < A) B = A, C = B + 1;
   
   *vtcols = termcols - C;
   *vtrows = termrows - 3;

   /* actually create the windows */
   listwin = newwin(termrows-3, A - 0, 1, 0);
   sumwin  = (B - A >= 3) ? newwin(termrows-3, B - A, 1, A) : NULL;
   vtwin   = newwin(termrows-3, *vtcols, 1, C);
   minibuf = newwin(1, termcols, termrows-1, 0);

   /* draw the top decoration line */
   wattrset(stdscr, COLOR_PAIR(3) | A_BOLD);
   wmove(stdscr, 0, 0);
   whline(stdscr, ACS_HLINE | A_NORMAL, termcols);
   
   /* draw instruction line */
   wattrset(stdscr, COLOR_PAIR(4*8) | A_BOLD);
   wmove(stdscr, termrows-2, 0);
   whline(stdscr, ' ', termcols);
   wmove(stdscr, termrows-2, 0);
   p = REMINDER_LINE;
   while (*p) {
      if (*p >= 0 && *p <= 7)
         wattrset(stdscr, COLOR_PAIR(4*8 + 7 - *p) | A_BOLD);
      else 
         waddch(stdscr, *p);
      p++;
   }
   
   /* draw the separator at column B */
   wattrset(stdscr, COLOR_PAIR(3) | A_BOLD);
   wmove(stdscr, 0, B);
   wvline(stdscr, ACS_VLINE | A_NORMAL, termrows - 2);
   wmove(stdscr, 0, B);          waddch(stdscr, ACS_TTEE);
   wrefresh(stdscr);

   /* draw window titles */
   if (termcols > 90) { 
      wmove(stdscr, 0, 2);
      waddstr(stdscr, "[Machines]");
      wmove(stdscr, 0, B+2);
      waddstr(stdscr, "[Terminal]");
   }

   /* make the cursor position be irrelevant for all windows except
    * the terminal window */
   leaveok(listwin, TRUE);
   if (sumwin) leaveok(sumwin, TRUE);

   /* draw all windows */
   touchwin(listwin);   wclear(listwin);
   touchwin(vtwin);     wclear(vtwin);
   if (sumwin) { touchwin(sumwin); wclear(sumwin); }
   touchwin(minibuf); wclear(minibuf);
}

void update_cast_label() {
   /* draws the label that says 'single cast' or 'multicast' on minibuffer */
   int termwidth, termheight;
   const char *msg;
   getmaxyx(minibuf, termheight, termwidth);
   
   if (machmgr_is_multicast()) {
      curutil_attrset(minibuf, 0xF9); /* bright blinking white over red */
      msg = "!!! MULTICAST MODE !!!";
   }
   else {
      curutil_attrset(minibuf, 0x40);
      msg = "singlecast mode";
   }
   
   werase(minibuf);
   wmove(minibuf, 0, termwidth - strlen(msg));
   waddstr(minibuf, msg);

   leaveok(minibuf, TRUE);  /* prevent cursor movement */
   wrefresh(minibuf);
   leaveok(minibuf, FALSE);
}

void redraw(bool force_full_redraw) {
   if (force_full_redraw) {
      touchwin(stdscr);
      wrefresh(stdscr);
   }

   /* draw machine list */
   machmgr_draw_list(); 
   if (force_full_redraw) touchwin(listwin);
   wrefresh(listwin);
   
   /* draw summary window, if there is one */
   if (sumwin) {
      machmgr_draw_summary(sumwin);
      if (force_full_redraw) touchwin(sumwin);
      wrefresh(sumwin);
   }

   /* draw vt window */
   machmgr_draw_vt(vtwin); 
   if (force_full_redraw) touchwin(vtwin);
   wrefresh(vtwin);

   /* draw the 'multicast/singlecast' label */
   update_cast_label();
}

static void add_machines_from_file(const char *file) {
   static char buf[128];
   bool pipe = false;
   FILE *f;

   if (getenv("OMNITTY_AT_COMMAND")) {
      /* popen() a command */
      pipe = true;
      strcpy(buf, getenv("OMNITTY_AT_COMMAND"));
      strcat(buf, " ");
      strcat(buf, file);
      strcat(buf, " 2>/dev/null");
      f = popen(buf, "r");
   }
   else f = fopen(file, "r");
   
   if (!f) {
      minibuf_msg(minibuf, pipe ? 
         "Can't execute command specified by OMNITTY_AT_COMMAND" : 
         "Can't read that file.", 0xF1);
      return;
   }

   minibuf_put(minibuf, pipe ? "Adding machines supplied by command..." :
                               "Adding machines from file...", 0x70);

   while (1 == fscanf(f, "%s", buf)) machmgr_add(buf);

   if (pipe) {
      if (0 != pclose(f)) 
         minibuf_msg(minibuf, "Command given by OMNITTY_AT_COMMAND exited "
                              "with error.", 0xF1);
      /* at this point SIGCHLD will have caused zombie_count to be one more
       * than it should, since the child command has already been reaped
       * by pclose(). If we don't correct zombie_count, wait() will block
       * in the main loop, since it will try to reap a zombie that does not yet 
       * exist. */
      zombie_count--;
   }
   else
      fclose(f);

   minibuf_put(minibuf, NULL, 0x70);
}

static void add_machine() {
   static char buf[32];

   *buf = 0;
   if (minibuf_prompt(minibuf, "Add: ", 0xE0, buf, 32)) {
      if (*buf == '@') add_machines_from_file(buf+1);
      else machmgr_add(buf);
   }
}

static void delete_machine() {
   static char buf[2];
   *buf = 0;
   if (minibuf_prompt(minibuf, "Really delete it [y/n]?", 0x90, buf, 2)
       && (*buf == 'y' || *buf == 'Y')) machmgr_delete_current();
}

int main(int argc, char **argv) {
   int vtcols, vtrows, ch = 0;
   int list_win_chars = LISTWIN_DEFAULT_CHARS;
   int term_win_chars = TERMWIN_DEFAULT_CHARS;
   bool quit = false;
   pid_t chldpid;

   /* process command-line options */
   while ( 0 < (ch = getopt(argc, argv, "W:T:")) ) {
      switch (ch) {
         case 'W': list_win_chars = atoi(optarg); break;
	 case 'T': term_win_chars = atoi(optarg);
		   if( term_win_chars < TERMWIN_MIN ) {
		       fprintf(stderr, " terminal area too narrow: %d\n", 
                                                        term_win_chars);
		       fputs(RTFM, stderr);
		       exit(2);
		   }
		   break;
         default: fputs(RTFM, stderr); exit(2);
      }
   }
   signal(SIGCHLD, sigchld_handler);
   curses_init();
   wins_init(&vtrows, &vtcols, list_win_chars, term_win_chars);
   menu_init(minibuf);

   machmgr_init(listwin, vtrows, vtcols);

   while (!quit) {
      if (zombie_count) {
         zombie_count--;
         chldpid = wait(NULL);
         machmgr_handle_death(chldpid);
      }

      machmgr_update();
      redraw(false);

      ch = getch();
      if (ch < 0) continue;

      switch (ch) {
         case KEY_F(1): menu_show(); redraw(true); break;
         case KEY_F(2): machmgr_prev_machine(); break;
         case KEY_F(3): machmgr_next_machine(); break;
         case KEY_F(4): machmgr_toggle_tag_current(); break;
         case KEY_F(5): add_machine(); break;
         case KEY_F(6): delete_machine(); break;
         case KEY_F(7): machmgr_toggle_multicast(); break;
         default: machmgr_forward_keypress(ch); break;
      }
   }

   endwin();
   return 0;
}

