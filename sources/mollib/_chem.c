/* _chem.c
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
													   
/* Chemical elements' symbols. This global variable is imported in several places. (but hopefully not modified)
 * Probably it would be a good idea to change the first and last "symbols" */
const char *symbol[84] = 
{"!","H","He","Li","Be","B","C","N","O","F","Ne","Na","Mg","Al","Si","P","S","Cl","Ar","K","Ca","Sc","Ti",
 "V","Cr","Mn","Fe","Co","Ni","Cu","Zn","Ga","Ge","As","Se","Br","Kr","Rb","Sr","Y","Zr","Nb","Mo","Tc",
 "Ru","Rh","Pd","Ag","Cd","In","Sn","Sb","Te","I","Xe","Cs","Ba","La","Ce","Pr","Nd","Pm","Sm","Eu","Gd",
 "Tb","Dy","Ho","Er","Tm","Yb","Lu","Hf","Ta","W","Re","Os","Ir","Pt","Au","Hg","Tl","Pb","*"};


/* Returns the valence of element with atomic number z. Only works for
 * "representative elements" (blocks s and p) */
int valence(int z)
{
	if (z==1) 
		return(1);
	if (z<=20) 
		return(4-abs((z-2)%8-4));
	if ((z<=30) || ((z>=39) && (z<=48))) 
		return(0);
	if (z<=56) 
		return(4-abs((z-4)%8-4));
	return(0);
}

/* Returns the group number (1-18) of element z */
int group(int z)
{
	if(z<=2) 
		return(2-z);
	if(z<=18) 
		return(((z-2)%8<3) ? (z-2)%8 : (z-2)%8+10);
	if(z<=56) 
		return(z%18);
	return(0);
}


/* Converts a symbol to an atomic number. Returns -1 if element not found */
int atomnum(char *sym)
{
	int i;
	for (i = 0; i < (sizeof(symbol) / sizeof(symbol[0])); ++i)
		if (!strcmp(sym, symbol[i]))
			break;

	if (i == (sizeof(symbol) / sizeof(symbol[0])))
		i = -1;

	return(i);
}

/* Converts a symbol (can be aromatic) to an atomic number. Returns -1 if element not found 
   Lowercase atoms are considered aromatic, as in SMILES */
int aromatomnum(char **sym, BOOL *arom)
{
	char s[4] = "\0\0\0\0";
	int Z = 0;
	char *ch = *sym;

	if (arom)
		*arom = FALSE;

	if(*ch) {
		if(isupper(*ch)) {
			s[0] = *ch;
			++ch;
			if(islower(*ch)) {
				s[1] = *ch;
				++ch;
			}
			Z = atomnum(s);
			if(Z < 0) {
				s[1] = 0;
				--ch;
				Z = atomnum(s);
			}
		} else  {// aromatic atom 
			s[0] = (char)toupper(*ch);
			++ch;
			Z = atomnum(s);
			if ((Z > 0) && arom)
				*arom = TRUE;
		}
	}

	*sym = ch;

	return(Z);
}

