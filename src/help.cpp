#include <ncurses.h>
#include <stdlib.h>
#include "curutil.h"
#include "help.h"


#define HELP_LINES 25
#define HELP_COLS  80
#define HELP_CONTENTS \
"Omnitty allows you to ssh into several machines at once. The list on the\n" \
"shows the machines you are logged into. Every key you type will be sent\n" \
"to the currently selected machine, except the following {control keys}:\n" \
"\n" \
"{F1}: brings up a menu of extended options (including {quit})\n" \
"{F2} and {F3}: move between machines on the list. The terminal window\n" \
"      on the right shows the display of the currently selected machine.\n" \
"{F4}: tag current machine. This will put a mark on the machine's name.\n" \
"      The mark by itself does nothing, but some features operate on\n" \
"      'all tagged machines', so this is a way to indicate which machines\n" \
"      they are to operate on. The {F1} menu has additional tag commands.\n" \
"{F5}: allows you to add a machine to the list. You can either supply the\n" \
"      name of the machine you want to add, or type {@file} where {file}\n" \
"      is the name of a file that contains machine names one per line,\n" \
"      in which case all machines in that file will be added.\n" \
"{F6}: delete current machine. If you want to delete multiple machines,\n" \
"      you can also tag them and choose {delete all tagged machines} from\n" \
"      the {F1} menu.\n" \
"{F7}: toggles between {singlecast} and {multicast} mode. In singlecast\n" \
"      mode, the keys you type will be directed to the currently selected\n" \
"      machine, regardless of tags. When {multicast} is selected, the\n" \
"      keys will be sent to {all tagged machines}, allowing you to operate\n"\
"      on several machines at once.\n"


void help_show() {
    int termwidth, termheight;
    const char *p; int i; int ch;
    WINDOW *w;
    
    getmaxyx(stdscr, termheight, termwidth);
    w = newwin(HELP_LINES, HELP_COLS, (termheight - HELP_LINES)/2,
               (termwidth  - HELP_COLS )/2);
    werase(w);
    curutil_attrset(w, 0xF4);
    curutil_window_fill(w, ' ');
    
    wborder(w, 0, 0, 0, 0, 0, 0, 0, 0);
    wmove(w, 0, 3); waddstr(w, "[ Help ]");
    
    wmove(w, i = 1, 1);
    p = HELP_CONTENTS;
    while (*p) {
        if      (*p == '{')  curutil_attrset(w, 0xE4);
        else if (*p == '}')  curutil_attrset(w, 0xF4);
        else if (*p == '\n') wmove(w, ++i, 1);
        else                 waddch(w, *p);
        p++;
    }
    
    wrefresh(w);
    while (0 > (ch = getch())) ;
    delwin(w);
}

