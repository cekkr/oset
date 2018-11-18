/* molan.c
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

#define MOLAN_BONDLENGTH 35

struct molanalysiswininfo
{
	struct analysisnode *node;	
	int currlist;
	HWND rxninfownd;
	HWND tgtdispwnd;
};


////////// global variables
extern struct cplx_params *cplx_params; 
extern struct struct_info *comp_info;
extern const char *appname;
////////////


void ClearSelection(struct moleditwininfo *info);
struct moldisplaydata *init_mddatalist(struct mol_list *list, int scale);

#define T_ANALYSIS	N_TOOLS


void AdjustScrollBar(HWND hwnd, struct molanalysiswininfo *info)
{
	SCROLLINFO si;

	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_ALL;
	si.nMin = 0;
	si.nMax = info->node->mml->Nlist - 1;
	si.nPage = 1;
	si.nPos = 0;
	SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);
}



LRESULT CALLBACK MolAnalysisWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	struct molanalysiswininfo *an_info;
	struct moleditwininfo *ed_info;

	ed_info = (struct moleditwininfo*) GetWindowLong(hwnd, 0);
	an_info = (struct molanalysiswininfo*) GetWindowLong(hwnd, 4);

	switch (message) {
		case WM_CREATE: 
			CallWindowProc(MolEditWndProc, hwnd, message, wParam, lParam);
			ed_info = (struct moleditwininfo*) GetWindowLong(hwnd, 0);
			an_info = calloc(1, sizeof(struct molanalysiswininfo));
			assert(an_info);
			SetWindowLong(hwnd, 4, (long) an_info);

			ed_info->scale = MOLAN_BONDLENGTH;
			ed_info->tools = realloc(ed_info->tools, sizeof(struct tool) * (N_TOOLS +1));
			ed_info->tools[T_ANALYSIS].cursor = LoadCursor((HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), "RETROCURSOR");
			ed_info->tools[T_ANALYSIS].select_atoms = FALSE;
			ed_info->tools[T_ANALYSIS].select_bonds = FALSE;
			ed_info->tools[T_ANALYSIS].select_mol = TRUE;
			ed_info->tools[T_ANALYSIS].helpstr = "Click on a molecule to analyze";
			break;


		case WM_PAINT: 
			if (GetUpdateRect(hwnd,NULL,FALSE)) {
				PAINTSTRUCT ps;
				HDC hdc;
				char *name;
				int i;
				struct mol_list *mlist;
				extern HFONT numfont;
				HFONT oldfont;

				CallWindowProc(MolEditWndProc, hwnd, message, wParam, lParam);
	
				if (an_info->node->mml->lists) {
					mlist = an_info->node->mml->lists[an_info->currlist];
				
					InvalidateRect(hwnd, NULL, FALSE);
					hdc = BeginPaint(hwnd, &ps);
		
					oldfont = SelectObject(hdc, numfont);
					for(i = 0; i < mlist->Nmol; ++i) {
						if(mlist->mols[i]->smiles)
							name = findsmiledb(comp_info, mlist->mols[i]->smiles);
						else 
							name = NULL;
						if(name) {
							struct moldisplaydata *mddata = &ed_info->mddata[i];
							ExtTextOut(hdc, mddata->molrect.left, mddata->molrect.bottom + 5, 0, NULL,
								 name, strlen(name), NULL);
						}
					}
					
					SelectObject(hdc, oldfont);
					EndPaint(hwnd, &ps);
				}
			}

			break;

		case MAM_SETROOT:  // sets the molecule to analyze
			//first it _should_ free the tree
			an_info->node = new_analysisnode(NULL, 0, 0, new_mol_metalist(new_mol_list((struct mol *)lParam)));
			
			ed_info->mol_list = new_mol_list((struct mol *)lParam);

			ed_info->mddata = init_mddatalist(ed_info->mol_list, ed_info->scale);
			an_info->currlist = 0;
			AdjustScrollBar(hwnd, an_info);
			InvalidateRect(hwnd, NULL, TRUE);
			break;

		case MAM_SET_INFO_WND:
			an_info->rxninfownd = (HWND)lParam;
			break;

		case MAM_SET_TGT_WND:
			an_info->tgtdispwnd = (HWND)lParam;
			break;

		case MEM_CUT:
		case MEM_PASTE: 
			break;		 //trap these messages


		case WM_LBUTTONDOWN:
			if(ed_info->tool == T_ANALYSIS) {
				if(ed_info->ht.mol >= 0) {
					struct chem_err *err = _init_err();
					struct mol *mol = ed_info->mol_list->mols[ed_info->ht.mol];
					struct mol_list *mol_list;

//					find_rings(mol);
//					classify_atoms(mol, err);
//					if(err->num_err > 0)
//						MessageBox(hwnd, "Molecule has errors", appname, MB_OK);
/*					else*/ {
//						busca_funcionales(comp_info, mol);
						parse_mol(comp_info, mol, err);

						an_info->node = new_analysisnode(an_info->node, an_info->currlist, ed_info->ht.mol,
							analyze_mol(comp_info, mol));
						AdjustScrollBar(hwnd, an_info);
						an_info->currlist = 0;
						if(an_info->node->mml->Nlist > 0) {
							mol_list = ed_info->mol_list = an_info->node->mml->lists[0];
							ed_info->mddata = init_mddatalist(mol_list, ed_info->scale);
							SetWindowText(an_info->rxninfownd, get_rxntext(comp_info, mol_list->rxn_info));
							SendMessage(an_info->tgtdispwnd, MDM_SETLIST, 0, (long) new_mol_metalist(new_mol_list(an_info->node->target->mml->lists[an_info->node->targetkey.rxn]->mols[an_info->node->targetkey.prec])));
						} else {
							ed_info->mol_list = new_mol_list(NULL);
						}

						InvalidateRect(hwnd, NULL, TRUE);

					}
					_destroy_err(err);
				}
			} else
				return(CallWindowProc(MolEditWndProc, hwnd, message, wParam, lParam));
			break;

		
		case WM_HSCROLL:
			switch(LOWORD(wParam)){
				case SB_LINEUP:
				case SB_PAGEUP:
					--an_info->currlist;
					break;

				case SB_LINEDOWN:
				case SB_PAGEDOWN:
					++an_info->currlist;
					break;
				
				case SB_THUMBPOSITION:
					an_info->currlist = HIWORD(wParam);
					break;
			}
			an_info->currlist = max(0, min(an_info->currlist, an_info->node->mml->Nlist - 1));
			if(an_info->currlist != GetScrollPos(hwnd, SB_HORZ)) {
				struct mol_list *mol_list;

				SetScrollPos(hwnd, SB_HORZ, an_info->currlist, TRUE);
				ClearSelection(ed_info);
				destroy_mddata(ed_info->mddata, ed_info->mol_list);
				mol_list = ed_info->mol_list = an_info->node->mml->lists[an_info->currlist];
				
				ed_info->mddata = init_mddatalist(mol_list, ed_info->scale);
				InvalidateRect(hwnd, NULL, TRUE);
				SetWindowText(an_info->rxninfownd, get_rxntext(comp_info, mol_list->rxn_info));
			}
			break;
		
			case MAM_UP:
				if(an_info->node->target) {
					an_info->currlist = an_info->node->targetkey.rxn;
					an_info->node = an_info->node->target;
					ClearSelection(ed_info);
					destroy_mddata(ed_info->mddata, ed_info->mol_list);
					ed_info->mol_list = an_info->node->mml->lists[an_info->currlist];
				
					ed_info->mddata = init_mddatalist(ed_info->mol_list, ed_info->scale);
					AdjustScrollBar(hwnd, an_info);
					InvalidateRect(hwnd, NULL, TRUE);

					if(an_info->node->target)
						SendMessage(an_info->tgtdispwnd, MDM_SETLIST, 0, (long) new_mol_metalist(new_mol_list(an_info->node->target->mml->lists[an_info->node->targetkey.rxn]->mols[an_info->node->targetkey.prec])));
					else
						SendMessage(an_info->tgtdispwnd, MDM_SETLIST, 0, (long) new_mol_metalist(NULL));
				}
				break;

		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case ID_BUTTONANALYZETOOL:{
					struct string_notification sn;

					ed_info->tool = T_ANALYSIS;
					sn.nmhdr.hwndFrom = hwnd;
					sn.nmhdr.idFrom =	GetWindowLong(hwnd, GWL_ID); 
					sn.nmhdr.code = MN_SETSTATUSTEXT;
					sn.s = ed_info->tools[ed_info->tool].helpstr;
					SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (long)&sn);
					ClearSelection(ed_info);
					InvalidateRect(hwnd, NULL, TRUE);
					}break;

			}
		
		default:
			return(CallWindowProc(MolEditWndProc, hwnd, message, wParam, lParam));

	}

	return(0);
}



 
