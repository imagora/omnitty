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


#ifndef btco_ROTE_inject_csi_h
#define btco_ROTE_inject_csi_h

#include "rote.h"

/* Interprets a CSI escape sequence stored in rt->pd->esbuf,
 * changing rt to reflect the effect of the sequence. This function
 * will not change rt->pd->esbuf, rt->pd->escaped or other escape-sequence
 * related fields in it */
void rote_es_interpret_csi(RoteTerm *rt);

#endif

