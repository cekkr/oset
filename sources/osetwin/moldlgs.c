/* moldlgs.c
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
#include "resource.h"





OPENFILENAME * init_openfiledlg(HWND hwnd, char *title, char *filter, int nfilter, char *dir, char *defext)
{
	OPENFILENAME *lpofn;
	
	lpofn = malloc(sizeof(OPENFILENAME));
	assert(lpofn);
		
	memset(lpofn, 0, sizeof(OPENFILENAME));
	lpofn->lStructSize = sizeof(OPENFILENAME);
	lpofn->hwndOwner = hwnd;
	lpofn->nMaxFile = 299;
	lpofn->lpstrTitle = title;
	lpofn->Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	lpofn->lpstrFilter = filter;
 	lpofn->nFilterIndex = nfilter;
	lpofn->lpstrInitialDir = dir;
	lpofn->lpstrFile = malloc(300);
	assert(lpofn->lpstrFile);
	lpofn->lpstrFileTitle = malloc(80);
	assert(lpofn->lpstrFileTitle);
	*lpofn->lpstrFile = 0;
	*lpofn->lpstrFileTitle = 0;
	lpofn->lpstrDefExt = defext;

	return(lpofn);
}




BOOL CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:
			SetDlgItemText(hwndDlg, IDC_DATE, __DATE__);
			return(1);
			break;

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDOK:
					EndDialog(hwndDlg, IDOK);
					break;

				case IDCANCEL:
					EndDialog(hwndDlg, IDCANCEL);
					break;
			}
			break;

	}
	return(0);		
}
