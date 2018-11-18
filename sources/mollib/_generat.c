/* _generat.c
 *
 * Copyright (C) 2000 Ivan Tubert and Eduardo Tubert
 * 
 * Contact: tubert@eros.pquim.unam.mx
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * All we ask is that proper credit is given for our work, which includes
 * - but is not limited to - adding the above copyright notice to the beginning
 * of your source code files, and to any copyright notice that you may distribute
 * with programs based on this work.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA. 
 * 
 */

#include "mollib.h"
#include <ctype.h>
#include <limits.h>


#ifdef _DEBUG
BOOL debuggen = FALSE;
#endif 


#define	REACTION_BLOCK 1024



void	generate_code(struct struct_info *info, int code)
{
	struct reaction *reaction = &info->reaction[info->numreact];
	
	if (reaction->mechsize == reaction->mechalloc) {
		reaction->mechanism = realloc(reaction->mechanism, reaction->mechalloc + REACTION_BLOCK);
		assert(reaction->mechanism);
		reaction->mechalloc += REACTION_BLOCK;
	}

	if (debuggen) {
		static int last = -1;

		if (last != TOK_NUMERO) {
			printf("GEN: %i %s\n", code, tokenstring(info, code));
			last = code;
		} else {
			printf("GEN: %i\n", code);
			last = -1;
		}
		getchar();
	}

	reaction->mechanism[reaction->mechsize++] = code;
}



