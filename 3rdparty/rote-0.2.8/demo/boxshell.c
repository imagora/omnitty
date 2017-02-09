/* Just a simple example program that creates a terminal in a frame
 * and lets the user interact with it.
 *
 * To compile:
 *    gcc -o boxshell boxshell.c $(rote-config --cflags --libs)
 */

#include <ncurses.h>
#include <stdio.h>
#include <rote/rote.h>
#include <signal.h>

static unsigned char getout = 0;
void sigchld(int signo) { getout = 1; }
int my_custom_handler(RoteTerm *rt, const char *es);
   
int screen_w, screen_h;
WINDOW *term_win;

int main() {
   RoteTerm *rt;
   int i, j, ch;

   signal(SIGCHLD, sigchld);

   initscr();
   noecho();
   start_color();
   raw();
   nodelay(stdscr, TRUE);       /* prevents getch() from blocking; rather
                                 * it will return ERR when there is no
                                 * keypress available */

   keypad(stdscr, TRUE);        /* necessary to use rote_vt_keypress */
   getmaxyx(stdscr, screen_h, screen_w);

   /* initialize the color pairs the way rote_vt_draw expects it. You might
    * initialize them differently, but in that case you would need
    * to supply a custom conversion function for rote_vt_draw to
    * call when setting attributes. The idea of this "default" mapping
    * is to map (fg,bg) to the color pair bg * 8 + 7 - fg. This way,
    * the pair (white,black) ends up mapped to 0, which means that
    * it does not need a color pair (since it is the default). Since
    * there are only 63 available color pairs (and 64 possible fg/bg
    * combinations), we really have to save 1 pair by assigning no pair
    * to the combination white/black. */
   for (i = 0; i < 8; i++) for (j = 0; j < 8; j++)
      if (i != 7 || j != 0)
         init_pair(j*8+7-i, i, j);

   /* paint the screen blue */
   attrset(COLOR_PAIR(32));
   for (i = 0; i < screen_h; i++) for (j = 0; j < screen_w; j++) addch(' ');
   refresh();

   /* create a window with a frame */
   term_win = newwin(22,72,1,4);
   wattrset(term_win, COLOR_PAIR(7*8+7-0)); /* black over white */
   wborder(term_win, 0, 0, 0, 0, 0, 0, 0, 0);
   mvwprintw(term_win, 0, 27, " Term In a Box ");
   wrefresh(term_win);

   /* create the terminal and have it run bash */
   rt = rote_vt_create(20, 70);
   rote_vt_forkpty(rt, "/bin/bash --login");

   /* add a sample custom escape sequence handler... say we want to handle
    * the sequence, say, \e{N}, which will change the application's background
    * color to N (where N is a number between 0 and 7). */
   rote_vt_install_handler(rt, my_custom_handler);

   /* keep reading keypresses from the user and passing them to the terminal;
    * also, redraw the terminal to the window at each iteration */
   ch = '\0';
   while (!getout) {
      rote_vt_draw(rt, term_win, 1, 1, NULL);
      wrefresh(term_win);
      
      ch = getch();
      if (ch != ERR) 
         rote_vt_keypress(rt, ch); /* pass the keypress for handling */
   }

   endwin();
   return 0;
}


int my_custom_handler(RoteTerm *rt, const char *es) {
   int color;
   int i, j;

   /* if the escape sequence does not begin with '{', we give up */
   if (*es != '{') return ROTE_HANDLERESULT_NOWAY;

   /* ok, now we know it begins with '{'. Now, if it does not end with '}',
    * it is not yet complete */
   if (es[strlen(es)-1] != '}') return ROTE_HANDLERESULT_NOTYET;

   /* ok, the sequence is complete */
   color = atoi(es + 1);
   if (color < 0 || color > 7) return false; /* don't recognize it */

   /* paint the background with that color */
   attrset(COLOR_PAIR(color * 8));
   move(0, 0);
   for (i = 0; i < screen_h; i++) for (j = 0; j < screen_w; j++) addch(' ');
   touchwin(stdscr);
   refresh();

   /* touch term_win to force it to do a full redraw next time */
   touchwin(term_win);

   /* and redraw the terminal window */
   wrefresh(term_win);
   
   /* escape sequence was handled ok */
   return ROTE_HANDLERESULT_OK;
}

