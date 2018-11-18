/* molaux.c
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

#include "molshow.h"
#include <math.h>
#include "resource.h"



int round(double x)
{
	if(fmod(x, 1.0) >= 0.5)
		return((int)x + 1);
	else if(fmod(x, 1.0) <= -0.5)
		return((int)x - 1);
	else
		return((int)x);
}

int i_square(int x)
{
	return(x*x);
}


void line(HDC hdc, int x1, int y1, int x2, int y2)
{
	MoveToEx(hdc, x1, y1, NULL);
	LineTo(hdc, x2, y2);
}


int setmenuitems(HMENU hmenu, int flag, int op, int item, ...)
{
	va_list marker;
	MENUITEMINFO menuiteminfo; 

	memset(&menuiteminfo, 0, sizeof(menuiteminfo));
	menuiteminfo.cbSize = sizeof(menuiteminfo);
	menuiteminfo.fMask = MIIM_STATE;

	for(va_start(marker, item); item != 0; item = va_arg(marker, int))
	{
		if(GetMenuItemInfo(hmenu, item, FALSE, &menuiteminfo) == 0)
			return(1);
		switch(op)
		{
			case OP_SET:
				menuiteminfo.fState |= flag;
				break;

			case OP_REMOVE:
				menuiteminfo.fState &= ~flag;
				break;

			case OP_TOGGLE:
				menuiteminfo.fState ^= flag;
				break;

			default:
				return(2);
		}
		if(SetMenuItemInfo(hmenu, item, FALSE, &menuiteminfo) == 0)
			return(3);
	}
	va_end(marker);
	
	return(0);
}


