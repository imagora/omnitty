#include <ncurses.h>
#include "curutil.h"


void curutil_attrset(WINDOW *win, unsigned char attr) {
    int fg = (attr & 0x70) >> 4;
    int bg = attr & 0x07;
    int cp = bg * 8 + 7 - fg;
    
    if (cp) wattrset(win, COLOR_PAIR(cp));
    else    wattrset(win, A_NORMAL);
    
    if (attr & 0x80) wattron(win, A_BOLD);
    if (attr & 0x08) wattron(win, A_BLINK);
}


void curutil_colorpair_init() {
    int f, b, cp;
    for (f = 0; f < 8; f++) for (b = 0; b < 8; b++) {
        cp = b*8+7-f;
        if (cp) init_pair(cp, f, b);
    }
}


void curutil_window_fill(WINDOW *win, int ch) {
    int h, w;
    int r, c;
    
    getmaxyx(win, h, w);
    wmove(win, 0, 0);
    for (r = 0; r < h; r++) for (c = 0; c < w; c++)
        waddch(win, ch);
    
    wmove(win, 0, 0);
}

