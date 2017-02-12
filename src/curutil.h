#pragma once


#include <ncurses.h>

/* This function initializes the curses color pairs (through init_pair())
 * according to the standard used by the ROTE library: if the foreground
 * color is f (0-7) and the background color is b (0-7), then the
 * corresponding color pair number is 8 * b + (7 - f). There is no
 * pair for foreground 7 with background 0 (since that is the default
 * color and doesn't need a pair) */
void curutil_colorpair_init();


/* Sets the curses attributes of the supplied window to the attribute
 * byte passed as parameter. The meaning of this byte-packed attribute
 * is defined in the ROTE library (see rote.h) */
void curutil_attrset(WINDOW *w, unsigned char attr);


/* Returns the size of the passed window in *width and *height. */
void curutil_window_size(WINDOW *w, int *width, int *height);


/* Fills the given window with the given character. */
void curutil_window_fill(WINDOW *w, int ch);

