/* clipbrd.c
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
#include <commctrl.h>


static UINT ctFormat;

void InitClipboard()
{
	ctFormat = RegisterClipboardFormat("MDLCT");
}



void CopyMolToClipboard(HWND hwnd, struct mol *mol)
{
	HGLOBAL hbuf;
	struct struct_log *buf;
	char *newbuf, *p1, *p2;


	if(OpenClipboard(hwnd)){
		EmptyClipboard();
		buf = writemolfilebuf(mol);
		hbuf = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE | GMEM_ZEROINIT,
			 buf->length+2);
		newbuf = GlobalLock(hbuf);
		memmove(newbuf+1, buf->buffer, buf->length+1);
		p1 = newbuf;
		p2 = strchr(p1+1, '\n');
		while(p2 != NULL){
			*p1 = p2-p1-1;
			p1 = p2;
			p2 = strchr(p1+1, '\n');
		}
		*p1 = strlen(p1+1);
		destroy_log(buf);
		GlobalUnlock(hbuf);
		SetClipboardData(ctFormat, hbuf);
		CloseClipboard();
	}

}

struct mol *readmolfilebuff(char *buff);

struct mol *GetMolFromClipboard(HWND hwnd)
{
	HGLOBAL hbuf;
	unsigned char *buf, *clipbrd, *p, *p2;
	BOOL b; int size;
	struct mol *mol = NULL;

	if(b= IsClipboardFormatAvailable(ctFormat)){
		if(OpenClipboard(hwnd)){
			hbuf = GetClipboardData(ctFormat);
			size = GlobalSize(hbuf); 
			clipbrd = GlobalLock(hbuf);
			buf = malloc(size+1);
			memcpy(buf, clipbrd, size);
			buf[size] = 26;
			GlobalUnlock(hbuf);
			CloseClipboard();
			
			p = buf;
			while(p < buf+size) {
				p2 = p + *p + 1;
				*p = 10;
				p = p2;
			}
			mol = readmolfilebuff(buf+1);
			free(buf);
		}
	}
	return(mol);
}

