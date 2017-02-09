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


#include "inject_csi.h"
#include "roteprivate.h"
#include <stdlib.h>
#include <string.h>

#define MAX_CSI_ES_PARAMS 32
   
static inline void clamp_cursor_to_bounds(RoteTerm *rt) {
   if (rt->crow < 0) rt->curpos_dirty = true, rt->crow = 0;
   if (rt->ccol < 0) rt->curpos_dirty = true, rt->ccol = 0;

   if (rt->crow >= rt->rows) 
      rt->curpos_dirty = true, rt->crow = rt->rows - 1;

   if (rt->ccol >= rt->cols)
      rt->curpos_dirty = true, rt->ccol = rt->cols - 1;
}

/* interprets a 'set attribute' (SGR) CSI escape sequence */
static void interpret_csi_SGR(RoteTerm *rt, int param[], int pcount) {
   int i;

   if (pcount == 0) {
      /* special case: reset attributes */
      rt->curattr = 0x70;
      return;
   }

   for (i = 0; i < pcount; i++) {

// From http://vt100.net/docs/vt510-rm/SGR table 5-16
// 0 	All attributes off
// 1 	Bold
// 4 	Underline
// 5 	Blinking
// 7 	Negative image
// 8 	Invisible image
// 10 	The ASCII character set is the current 7-bit
//	display character set (default) - SCO Console only.
// 11 	Map Hex 00-7F of the PC character set codes
//	to the current 7-bit display character set
//	- SCO Console only.
// 12 	Map Hex 80-FF of the current character set to
//	the current 7-bit display character set - SCO
//	Console only.
// 22 	Bold off
// 24 	Underline off
// 25 	Blinking off
// 27 	Negative image off
// 28 	Invisible image off

      if (param[i] == 0) rt->curattr = 0x70;
      else if (param[i] == 1 || param[i] == 2 || param[i] == 4)  /* set bold */
         ROTE_ATTR_MOD_BOLD(rt->curattr,1);
      else if (param[i] == 5)  /* set blink */
         ROTE_ATTR_MOD_BLINK(rt->curattr,1);
      else if (param[i] == 7 || param[i] == 27) { /* reverse video */
	int fg = ROTE_ATTR_FG(rt->curattr);
	int bg = ROTE_ATTR_BG(rt->curattr);
	ROTE_ATTR_MOD_FG(rt->curattr, bg);
	ROTE_ATTR_MOD_BG(rt->curattr, fg);
      }
      else if (param[i] == 8) rt->curattr = 0x0;    /* invisible */
      else if (param[i] == 22 || param[i] == 24) /* bold off */
        ROTE_ATTR_MOD_BOLD(rt->curattr,0);
      else if (param[i] == 25) /* blink off */
        ROTE_ATTR_MOD_BLINK(rt->curattr,0);
      else if (param[i] == 28) /* invisible off */
        rt->curattr = 0x70;
      else if (param[i] >= 30 && param[i] <= 37)    /* set fg */
         ROTE_ATTR_MOD_FG(rt->curattr, param[i] - 30);
      else if (param[i] >= 40 && param[i] <= 47)    /* set bg */
         ROTE_ATTR_MOD_BG(rt->curattr, param[i] - 40);
      else if (param[i] == 39)  /* reset foreground to default */
         ROTE_ATTR_MOD_FG(rt->curattr, 7);
      else if (param[i] == 49)  /* reset background to default */
         ROTE_ATTR_MOD_BG(rt->curattr, 0);
   }
}

/* interprets an 'erase display' (ED) escape sequence */
static void interpret_csi_ED(RoteTerm *rt, int param[], int pcount) {
   int r, c;
   int start_row, start_col, end_row, end_col;

   /* decide range */
   if (pcount && param[0] == 2) 
                      start_row = 0, start_col = 0, end_row = rt->rows - 1,
                      end_col = rt->cols - 1;

   else if (pcount && param[0] == 1)
                      start_row = 0, start_col = 0, end_row = rt->crow,
                      end_col = rt->ccol;

   else start_row = rt->crow, start_col = rt->ccol,
        end_row = rt->rows - 1, end_col = rt->cols - 1;

   /* clean range */
   for (r = start_row; r <= end_row; r++) {
      rt->line_dirty[r] = true;

      for (c = (r == start_row ? start_col : 0);
                             c <= (r == end_row ? end_col : rt->cols - 1);
                             c++) {
         rt->cells[r][c].ch = 0x20;
         rt->cells[r][c].attr = rt->curattr;
      }
   }
}

/* interprets a 'move cursor' (CUP) escape sequence */
static void interpret_csi_CUP(RoteTerm *rt, int param[], int pcount) {
   if (pcount == 0) {
      /* special case */
      rt->crow = rt->ccol = 0;
      return;
   }
   else if (pcount < 2) return;  /* malformed */

   rt->crow = param[0] - 1;  /* convert from 1-based to 0-based */
   rt->ccol = param[1] - 1;  /* convert from 1-based to 0-based */

   rt->curpos_dirty = true;

   clamp_cursor_to_bounds(rt);
}

/* Interpret the 'relative mode' sequences: CUU, CUD, CUF, CUB, CNL,
 * CPL, CHA, HPR, VPA, VPR, HPA */
static void interpret_csi_C(RoteTerm *rt, char verb, 
                                                int param[], int pcount) {
   int n = (pcount && param[0] > 0) ? param[0] : 1;

   switch (verb) {
      case 'A':           rt->crow -= n; break;
      case 'B': case 'e': rt->crow += n; break;
      case 'C': case 'a': rt->ccol += n; break;
      case 'D':           rt->ccol -= n; break;
      case 'E':           rt->crow += n; rt->ccol = 0; break;
      case 'F':           rt->crow -= n; rt->ccol = 0; break;
      case 'G': case '`': rt->ccol  = param[0] - 1; break;
      case 'd':           rt->crow  = param[0] - 1; break;
   }

   rt->curpos_dirty = true;
   clamp_cursor_to_bounds(rt);
}

/* Interpret the 'erase line' escape sequence */
static void interpret_csi_EL(RoteTerm *rt, int param[], int pcount) {
   int erase_start, erase_end, i;
   int cmd = pcount ? param[0] : 0;

   switch (cmd) {
      case 1:  erase_start = 0;           erase_end = rt->ccol;     break;
      case 2:  erase_start = 0;           erase_end = rt->cols - 1; break;
      default: erase_start = rt->ccol;    erase_end = rt->cols - 1; break;
   }

   for (i = erase_start; i <= erase_end; i++) {
      rt->cells[rt->crow][i].ch = 0x20; 
      rt->cells[rt->crow][i].attr = rt->curattr;
   }

   rt->line_dirty[rt->crow] = true;
}

/* Interpret the 'insert blanks' sequence (ICH) */
static void interpret_csi_ICH(RoteTerm *rt, int param[], int pcount) {
   int n = (pcount && param[0] > 0) ? param[0] : 1; 
   int i;
   for (i = rt->cols - 1; i >= rt->ccol + n; i--)
      rt->cells[rt->crow][i] = rt->cells[rt->crow][i - n];
   for (i = rt->ccol; i < rt->ccol + n; i++) {
      rt->cells[rt->crow][i].ch = 0x20;
      rt->cells[rt->crow][i].attr = rt->curattr;
   }

   rt->line_dirty[rt->crow] = true;
}

/* Interpret the 'delete chars' sequence (DCH) */
static void interpret_csi_DCH(RoteTerm *rt, int param[], int pcount) {
   int n = (pcount && param[0] > 0) ? param[0] : 1; 
   int i;
   for (i = rt->ccol; i < rt->cols; i++) {
     if (i + n < rt->cols)
         rt->cells[rt->crow][i] = rt->cells[rt->crow][i + n];
     else {
         rt->cells[rt->crow][i].ch = 0x20;
         rt->cells[rt->crow][i].attr = rt->curattr;
     }
   }

   rt->line_dirty[rt->crow] = true;
}

/* Interpret an 'insert line' sequence (IL) */
static void interpret_csi_IL(RoteTerm *rt, int param[], int pcount) {
   int n = (pcount && param[0] > 0) ? param[0] : 1;
   int i, j;

   for (i = rt->pd->scrollbottom; i >= rt->crow + n; i--) 
      memcpy(rt->cells[i], rt->cells[i - n], sizeof(RoteCell) * rt->cols);

   for (i = rt->crow; i < rt->crow + n && i <= rt->pd->scrollbottom; i++) {
      rt->line_dirty[i] = true;
      for (j = 0; j < rt->cols; j++) 
         rt->cells[i][j].ch = 0x20, rt->cells[i][j].attr = rt->curattr;
   }

}

/* Interpret a 'delete line' sequence (DL) */
static void interpret_csi_DL(RoteTerm *rt, int param[], int pcount) {
   int n = (pcount && param[0] > 0) ? param[0] : 1;
   int i, j;

   for (i = rt->crow; i <= rt->pd->scrollbottom; i++) {
      rt->line_dirty[i] = true;
      if (i + n <= rt->pd->scrollbottom)
         memcpy(rt->cells[i], rt->cells[i+n], sizeof(RoteCell) * rt->cols);
      else {
         for (j = 0; j < rt->cols; j++)
            rt->cells[i][j].ch = 0x20, rt->cells[i][j].attr = rt->curattr;
      }
   }
}

/* Interpret an 'erase characters' (ECH) sequence */
static void interpret_csi_ECH(RoteTerm *rt, int param[], int pcount) {
   int n = (pcount && param[0] > 0) ? param[0] : 1;
   int i;

   for (i = rt->ccol; i < rt->ccol + n && i < rt->cols; i++) {
      rt->cells[rt->crow][i].ch = 0x20;
      rt->cells[rt->crow][i].attr = rt->curattr;
   }

   rt->line_dirty[rt->crow] = true;
}
        
/* Interpret a 'set scrolling region' (DECSTBM) sequence */
static void interpret_csi_DECSTBM(RoteTerm *rt, int param[], int pcount) {
   int newtop, newbottom;
   
   if (!pcount) {
      newtop = 0;
      newbottom = rt->rows - 1;
   }
   else if (pcount < 2) return; /* malformed */
   else {
      newtop = param[0] - 1;
      newbottom = param[1] - 1;
   }

   /* clamp to bounds */
   if (newtop < 0) newtop = 0;
   if (newtop >= rt->rows) newtop = rt->rows - 1;
   if (newbottom < 0) newbottom = 0;
   if (newbottom >= rt->rows) newbottom = rt->rows - 1;

   /* check for range validity */
   if (newtop > newbottom) return;
   rt->pd->scrolltop = newtop;
   rt->pd->scrollbottom = newbottom;
}
         
static void interpret_csi_SAVECUR(RoteTerm *rt, int param[], int pcount) {
   rt->pd->saved_x = rt->ccol;
   rt->pd->saved_y = rt->crow;
}

static void interpret_csi_RESTORECUR(RoteTerm *rt, int param[], int pcount) {
   rt->ccol = rt->pd->saved_x;
   rt->crow = rt->pd->saved_y;
   rt->curpos_dirty = true;
}

void rote_es_interpret_csi(RoteTerm *rt) {
   static int csiparam[MAX_CSI_ES_PARAMS];
   int param_count = 0;
   const char *p = rt->pd->esbuf + 1;
   char verb = rt->pd->esbuf[rt->pd->esbuf_len - 1];

   if (!strncmp(rt->pd->esbuf, "[?", 2)) { /* private-mode CSI, ignore */
      #ifdef DEBUG
      fprintf(stderr, "Ignoring private-mode CSI: <%s>\n", rt->pd->esbuf);
      #endif
      return; 
   }

   /* parse numeric parameters */
   while ((*p >= '0' && *p <= '9') || *p == ';') {
      if (*p == ';') {
         if (param_count >= MAX_CSI_ES_PARAMS) return; /* too long! */
         csiparam[param_count++] = 0;
      }
      else {
         if (param_count == 0) csiparam[param_count++] = 0;
         csiparam[param_count - 1] *= 10;
         csiparam[param_count - 1] += *p - '0';
      }

      p++;
   }

   /* delegate handling depending on command character (verb) */
   switch (verb) {
      case 'm': /* it's a 'set attribute' sequence */
         interpret_csi_SGR(rt, csiparam, param_count); break;
      case 'J': /* it's an 'erase display' sequence */
         interpret_csi_ED(rt, csiparam, param_count); break;
      case 'H': case 'f': /* it's a 'move cursor' sequence */
         interpret_csi_CUP(rt, csiparam, param_count); break;
      case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
      case 'e': case 'a': case 'd': case '`':
         /* it is a 'relative move' */
         interpret_csi_C(rt, verb, csiparam, param_count); break;
      case 'K': /* erase line */
         interpret_csi_EL(rt, csiparam, param_count); break;
      case '@': /* insert characters */
         interpret_csi_ICH(rt, csiparam, param_count); break;
      case 'P': /* delete characters */
         interpret_csi_DCH(rt, csiparam, param_count); break;
      case 'L': /* insert lines */
         interpret_csi_IL(rt, csiparam, param_count); break;
      case 'M': /* delete lines */
         interpret_csi_DL(rt, csiparam, param_count); break;
      case 'X': /* erase chars */
         interpret_csi_ECH(rt, csiparam, param_count); break;
      case 'r': /* set scrolling region */
         interpret_csi_DECSTBM(rt, csiparam, param_count); break;
      case 's': /* save cursor location */
         interpret_csi_SAVECUR(rt, csiparam, param_count); break;
      case 'u': /* restore cursor location */
         interpret_csi_RESTORECUR(rt, csiparam, param_count); break;
      #ifdef DEBUG
      default:
         fprintf(stderr, "Unrecogized CSI: <%s>\n", rt->pd->esbuf); break;
      #endif
   }
}

