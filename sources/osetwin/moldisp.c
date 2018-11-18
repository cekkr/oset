/* moldisp.c
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

struct moldispwininfo
{
	struct mol_metalist *list;
	int currlist;
	int scale;
	struct moldisplaydata *mddata; 
	struct reaction *reaction;
};

struct moldisplaydata * init_mddatalist(struct mol_list *list, int scale);

LRESULT CALLBACK MolDispWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
		
	static BOOL lbuttondown;
	static int lastx, lasty;
	struct moldispwininfo *info;

	info = (struct moldispwininfo*) GetWindowLong(hwnd, 0);

	switch (message) {
		case WM_CREATE: 
			lbuttondown = FALSE;
			info = calloc(1, sizeof(struct moldispwininfo));
			info->reaction = ((CREATESTRUCT*)lParam)->lpCreateParams;
			info->scale = 35;
			SetWindowLong(hwnd, 0, (long) info);
			break;

		case WM_PAINT:
			if (GetUpdateRect(hwnd,NULL,FALSE)) {
				PAINTSTRUCT ps;
				HDC hdc;
				struct mol_list *mol_list;
				int i;

				hdc = BeginPaint(hwnd, &ps);
				if(info->list){
					mol_list = info->list->lists[info->currlist];
					for(i = 0; i < mol_list->Nmol; ++i){
						paintmol(hdc, mol_list->mols[i], &info->mddata[i], FALSE, FALSE);
					}
				}


				EndPaint(hwnd, &ps);
			}
			break;

		case MDM_SETLIST:
			info->list = (struct mol_metalist*)lParam;
			if(info->list->Nlist == 0)
				info->list = NULL;
			info->currlist = 0;
			if(info->mddata){
				free(info->mddata);
				info->mddata = NULL;
			}
			if(info->list)
				info->mddata = init_mddatalist(info->list->lists[info->currlist], info->scale);
			InvalidateRect(hwnd, NULL, TRUE);
			break;

		case MDM_NEXT:
			if(info->list){
				info->currlist = (info->currlist + 1) % info->list->Nlist;
				free(info->mddata);
				info->mddata = init_mddatalist(info->list->lists[info->currlist], info->scale);
			}
			break;

		case MDM_PREV:
			if(info->list){
				info->currlist = (info->currlist + info->list->Nlist -1) % info->list->Nlist;
				free(info->mddata);
				info->mddata = init_mddatalist(info->list->lists[info->currlist], info->scale);
			}
			break;

		case MDM_GETRXNINFO:
			if(info->list){
				char *s;
				struct rxn_info *rxn_info;

				rxn_info = info->list->lists[info->currlist]->rxn_info;
				s = malloc(200);
				sprintf(s, "%i of %i\r\n%s\r\nRating = %i\r\nSimplification = %i\r\nEvaluation = %i",
				info->currlist+1, info->list->Nlist,
				info->reaction[rxn_info->rxn].name,
				rxn_info->rating,
				rxn_info->simplification,
				rxn_info->eval);
				
				return((long)s);
			}
			else{
				char *s = malloc(1);
				s[0] = 0;
				return((long)s);
			}
			break;

		case WM_LBUTTONDOWN:
			SetFocus(hwnd);
			break;

/*		case WM_MOUSEMOVE:
			if(lbuttondown) 
			{
//				info->mddata->pos.x += LOSHORT(lParam) - lastx;
//				info->mddata->pos.y += HISHORT(lParam) - lasty;
				lastx = LOSHORT(lParam);
				lasty = HISHORT(lParam);
				InvalidateRect(hwnd, NULL, TRUE);
			}
			break;

		case WM_LBUTTONUP:
			if(lbuttondown)
			{
				ReleaseCapture();
				lbuttondown = FALSE;
				InvalidateRect(hwnd, NULL, TRUE);
			}
			break;
		
		case WM_RBUTTONDOWN:
			break;

*/
		default:                  /* Passes it on if unproccessed    */
			return (DefWindowProc(hwnd, message, wParam, lParam));
	}

	return (0);
}

struct moldisplaydata * init_mddatalist(struct mol_list *list, int scale)
{
	int i, x = 10;
	struct moldisplaydata *mddata;

	mddata = calloc(list->Nmol, sizeof(struct moldisplaydata));
	assert(mddata);

	for(i = 0; i < list->Nmol; ++i){
		molorigin(list->mols[i]);
		calc_moldisplaydata(list->mols[i], &mddata[i], x, 10, scale);
		x += mddata[i].molrect.right - mddata[i].molrect.left + 30;
	}

	return(mddata);
}



LRESULT CALLBACK MolRxnInfoWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	extern WNDPROC defeditproc;
	int ht;
	static BOOL sizing;
	HWND dispwnd;
	 
	dispwnd = (HWND) GetWindowLong(hwnd, GWL_USERDATA);
	
	switch(message){
		case MDM_SETDISPWND:
			SetWindowLong(hwnd, GWL_USERDATA, lParam);
			break;

		case WM_NCHITTEST:
			ht = DefWindowProc(hwnd, message, wParam, lParam);
			if((ht == HTRIGHT) || (ht == HTTOP) || (ht == HTBOTTOM) || 
				(ht == HTTOPRIGHT) || (ht == HTBOTTOMRIGHT))
				return(HTNOWHERE);
			else if ((ht == HTTOPLEFT) || (ht == HTBOTTOMLEFT))
				return(HTLEFT);
			else
				return(ht);
			break;
	
		case WM_SIZE:
			if(sizing){
				struct size_notification sn;
				RECT r;

				GetWindowRect(hwnd, &r);
				sizing = FALSE;

				sn.nmhdr.hwndFrom = hwnd;
				sn.nmhdr.idFrom =	GetWindowLong(hwnd, GWL_ID); 
				sn.nmhdr.code = MN_SIZENOTIFY;
				sn.x = r.right-r.left;
				sn.y = r.bottom-r.top;

				SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (long)&sn);
			}
			break;
		
		case WM_SIZING:
			sizing = TRUE;
			break;
		
		case MDM_REFRESHTEXT:{
			char *s;

			s = (char*)SendMessage(dispwnd, MDM_GETRXNINFO, 0, 0);
			SetWindowText(hwnd, s);
			free(s);
			return(0);
		}
		
	}


	return(CallWindowProc(defeditproc, hwnd, message, wParam, lParam));

}
 
LRESULT CALLBACK MolTreeWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	extern WNDPROC deftreeproc;
	static BOOL sizing;
	int ht;
	HWND dispwnd;
	 
	dispwnd = (HWND) GetWindowLong(hwnd, GWL_USERDATA);
	
	switch(message){
		case MDM_SETDISPWND:
			SetWindowLong(hwnd, GWL_USERDATA, lParam);
			break;

		case WM_SIZE:
			if(sizing){
				struct size_notification sn;
				sizing = FALSE;

				sn.nmhdr.hwndFrom = hwnd;
				sn.nmhdr.idFrom =	GetWindowLong(hwnd, GWL_ID); 
				sn.nmhdr.code = MN_SIZENOTIFY;
				sn.x = LOWORD(lParam);
				sn.y = HIWORD(lParam);

				SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (long)&sn);
			}
			break;
		
		case WM_SIZING:
			sizing = TRUE;
			break;
		
		case WM_NCHITTEST:
			ht = DefWindowProc(hwnd, message, wParam, lParam);
			if (ht == HTBOTTOMLEFT || ht == HTTOPLEFT)
				return(HTLEFT);
			else
				return(ht);
			break;
	}


	return(CallWindowProc(deftreeproc, hwnd, message, wParam, lParam));

}
 

void init_tree(HWND tree)
{
	TV_INSERTSTRUCT is;
	int i,j,k;
	char s[20];
	HTREEITEM hitem;

	is.hInsertAfter = TVI_LAST;
	is.item.mask = TVIF_TEXT;
	is.item.pszText = s;

	for(i=1; i <= 6; ++i){
		is.hParent = TVI_ROOT;
		sprintf(is.item.pszText, "item %i", i);
		hitem = TreeView_InsertItem(tree, &is);		
		for(j=1; j <= 3; ++j){
			is.hParent = hitem;
			sprintf(is.item.pszText, "item %i%i", i, j);
			is.hParent = TreeView_InsertItem(tree, &is);		
			for(k=1; k <= 2; ++k){
				sprintf(is.item.pszText, "item %i%i%i", i, j, k);
				TreeView_InsertItem(tree, &is);		
			}
		}
	}
	

}