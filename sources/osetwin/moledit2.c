/* moledit2.c
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

void startnewbond(struct tempbondinfo *tbi, struct moldisplaydata *data, struct molht *ht, int x, int y);
void endnewbond(struct tempbondinfo *tbi, struct mol_list **list, struct moldisplaydata **mddata, struct molht *ht, int scale);
void tempbond(struct tempbondinfo *tbi, struct moldisplaydata *data, struct molht *ht, int x, int y, BOOL anydir);
struct tool *init_tools(void);



#define MAX_UNDO 10

struct undo_node
{
	struct mol_list *mol_list;
	struct moldisplaydata *mddata;
	struct undo_node *next;
};

void ClearSelection(struct moleditwininfo *info)
{
	int i;

	for(i = 0; i < info->Nselect; ++i) {
		free(info->selection[i].atoms);
		free(info->selection[i].bonds);
	}
	free(info->selection);
	info->selection = NULL;
	info->Nselect = 0;
}

void ClearHitTest(struct molht *ht)
{
	ht->mol = ht->atom = ht->bond = -1;
}

void StoreUndoInfo(struct moleditwininfo *info)
{
	struct undo_node *newnode, *curr;

	int i;
	
	newnode = malloc(sizeof(struct undo_node));
	assert(newnode);


	newnode->mol_list = mol_listdup(info->mol_list);
	newnode->mddata = mddatadup(info->mddata, info->mol_list);
	newnode->next = info->undo;
	info->undo = newnode;

	for(i = 1, curr = info->undo; (i < MAX_UNDO) && (curr != NULL); ++i, curr = curr->next) 
		;
	if(curr && curr->next){
		destroy_mddata(curr->next->mddata, curr->next->mol_list);
		destroy_mol_list(curr->next->mol_list);
		free(curr->next);
		curr->next = NULL;
	}

	if(info->redo) {  // destroy redo info
		struct undo_node *next;
		for(curr = info->redo; curr != NULL; curr = next) {
			next = curr->next;
			destroy_mddata(curr->mddata, curr->mol_list);
			destroy_mol_list(curr->mol_list);
			free(curr);
		}
		info->redo = NULL;
	}
}

void Undo(struct moleditwininfo *info)
{
	struct undo_node temp;

	if(info->undo){
		// here it stores the redo info...
		temp = *(info->undo);
		info->undo->mol_list = info->mol_list;
		info->undo->mddata = info->mddata;
		info->undo->next = info->redo;
		info->redo = info->undo;

		info->mol_list = temp.mol_list;
		info->mddata = temp.mddata;
		info->undo = temp.next;
		ClearSelection(info);
	}
}

void Redo(struct moleditwininfo *info)
{
	struct undo_node temp;

	if(info->redo){
		//store the undo info...
		temp = *(info->redo);
		info->redo->mol_list = info->mol_list;
		info->redo->mddata = info->mddata;
		info->redo->next = info->undo;
		info->undo = info->redo;

		info->mol_list = temp.mol_list;
		info->mddata = temp.mddata;
		info->redo = temp.next;
		ClearSelection(info);
	}
}


LRESULT CALLBACK MolEditWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
		
	struct moleditwininfo *info;
	struct mol_list *mol_list;

	info = (struct moleditwininfo*) GetWindowLong(hwnd, 0);
	if(info) mol_list = info->mol_list;

	switch (message) {
		case WM_CREATE: 


			info = calloc(1, sizeof(struct moleditwininfo));
			assert(info);
			info->mol_list = new_mol_list(NULL);
			info->tools = init_tools();

			info->paint_numbers = TRUE;
			info->ht.mol = info->ht.bond = info->ht.atom = -1;
			info->scale = DEFBONDLENGTH;
			SetWindowLong(hwnd, 0, (long) info);
			//SetFocus(hwnd);
			break;

		case WM_PAINT:
			if (GetUpdateRect(hwnd,NULL,FALSE)) {
				PAINTSTRUCT ps;
				HDC hdc;
				int i;

				hdc = BeginPaint(hwnd, &ps);
				for(i = 0; i < mol_list->Nmol; ++i){
					paintmol(hdc, mol_list->mols[i], &info->mddata[i],
					 info->paint_numbers, info->lbuttondown && 
							(info->tool == T_SELECT) && (info->ht.mol == i));
				}
				
				highlight_ht(hdc, mol_list, info->mddata, &info->ht, info->tools[info->tool]);
				highlight_selection(hdc, info->mddata, info->selection, info->Nselect);
				if(info->tbi.draw)
					draw_bond(hdc, info->tbi.startpos.x, info->tbi.startpos.y,
						info->tbi.endpos.x, info->tbi.endpos.y, 1);

				EndPaint(hwnd, &ps);
			}
			break;


		case MEM_SETMOL:{
			struct mol *mol = (struct mol*)lParam;
			int i;

			normalize_mol(mol, 1.0);
			separate_mols(mol, mol_list);
			info->mddata = malloc(mol_list->Nmol * sizeof(struct moldisplaydata));
			assert(info->mddata);
			for(i = 0; i < mol_list->Nmol; ++i){
				calc_moldisplaydata(mol_list->mols[i], &info->mddata[i], 50, 50, info->scale);
				adjustmolrect(mol_list->mols[i], &info->mddata[i]);
				molorigin(mol_list->mols[i]);
			}
			InvalidateRect(hwnd, NULL, TRUE);
			info->modified = FALSE;
			}
			break;

		case MEM_GETMOL:
			if((lParam < mol_list->Nmol) && (lParam >= 0))
				return((long)moldup(mol_list->mols[lParam]));
			else{
				struct mol *mol = new_mol();
				return((long)mol);
			}
			break;

		case MEM_GETSELECTEDMOL:{
			struct mol *ret = NULL;
			if(mol_list->Nmol == 1)
				ret = moldup(mol_list->mols[0]);
			else if(info->Nselect == 1) {
				if((info->selection[0].atoms == NULL) && (info->selection[0].bonds == NULL))
					ret = moldup(mol_list->mols[info->selection[0].mol]);
			}
			if(ret == NULL)
				ret = new_mol();	
			return((long)ret);
			}break;
			
		case MEM_RESETMOL:{
			int i;
			for(i=0; i < mol_list->Nmol; ++i)
				free(info->mddata[i].points);
			free(info->mddata);
			info->mddata = NULL;
			reset_mol_list(mol_list);
			ClearSelection(info);
			ClearHitTest(&info->ht);
			InvalidateRect(hwnd, NULL, TRUE);
			}
			break;

		case MEM_COPY:
		case MEM_CUT:
			if((info->Nselect == 1) && (info->selection[0].atoms == NULL) && (info->selection[0].bonds == NULL))
				CopyMolToClipboard(hwnd, mol_list->mols[info->selection[0].mol]);
			if(message == MEM_COPY)
				break;
			// else fall through to "cut"

		case MEM_DELETE_SELECTION:
			if((info->Nselect == 1) && (info->selection[0].atoms == NULL) && (info->selection[0].bonds == NULL)) {
				StoreUndoInfo(info);
				delete_mol(mol_list, info->selection[0].mol);
				deletemoldisplaydata(mol_list, &info->mddata, info->selection[0].mol);
				InvalidateRect(hwnd, NULL, TRUE);
				ClearSelection(info);
				ClearHitTest(&info->ht);
			}
			break;

		case MEM_PASTE:{
			struct mol *mol = GetMolFromClipboard(hwnd);
			if(mol){
				StoreUndoInfo(info);
				append_mol(mol_list, mol);
				normalize_mol(mol, 1.0);
				molorigin(mol);
				info->mddata = realloc(info->mddata, mol_list->Nmol * sizeof(struct moldisplaydata));
				assert(info->mddata);
				calc_moldisplaydata(mol, &info->mddata[mol_list->Nmol-1], 50, 50, info->scale);
				adjustmolrect(mol, &info->mddata[mol_list->Nmol-1]);
				InvalidateRect(hwnd, NULL, TRUE);
			}

			}break;

		
		case MEM_GETMOLCOUNT://count
			return(mol_list->Nmol);
			break;

		case MEM_UNDO:
			Undo(info);
			InvalidateRect(hwnd, NULL, TRUE);
			break;

		case MEM_REDO:
			Redo(info);
			InvalidateRect(hwnd, NULL, TRUE);
			break;

		case MEM_CANPASTE:
		case MEM_CANCOPY:
		case MEM_CANUNDO:
		case MEM_GETMODIFY:
			break;	

		case MEM_TOGGLENUMBERS:
			info->paint_numbers = !info->paint_numbers; 
			InvalidateRect(hwnd, NULL, TRUE);
			break;

		case WM_KEYDOWN:
			switch(wParam){
				case VK_BACK:
				case VK_DELETE:
					SendMessage(hwnd, MEM_DELETE_SELECTION, 0, 0);
					break;

				default:
					return (DefWindowProc(hwnd, message, wParam, lParam));
			}
			break;

		case WM_LBUTTONDOWN:{
			struct mol *mol;
			struct moldisplaydata *mddata;

			info->lbuttondown = TRUE;
			if(info->ht.mol >= 0) {
				mol = mol_list->mols[info->ht.mol];
				mddata = &info->mddata[info->ht.mol];
			}

			switch(info->tool)
			{	
				case T_ERASE:
					if((info->ht.atom >=0) || (info->ht.bond >= 0)) {
						StoreUndoInfo(info);
						if(info->ht.atom >= 0) {
							deleteatom(mol, info->ht.atom);
							deleteatomdisplaydata(mol, mddata, info->ht.atom);
							info->ht.atom = -1;
							delete_lone_atoms(hwnd, mol, mddata);
						} else {
							breakbond(mol, info->ht.bond);
							info->ht.bond = -1;
							delete_lone_atoms(hwnd, mol, mddata);
						}
						
						// here it should separate the molecules,
						// check if a molecule was killed, etc.
						if(mol->Natom == 0) { //kill the molecule
							delete_mol(mol_list, info->ht.mol);
							deletemoldisplaydata(mol_list, &info->mddata, info->ht.mol);
							ClearHitTest(&info->ht);
						} else if(separate_mols(mol, NULL) > 1)	{
							struct mol_list *newlist = new_mol_list(NULL);
							int i, n;
							n = separate_mols(mol, newlist);
							info->mddata = mddata = realloc(info->mddata, (mol_list->Nmol+n) * sizeof(struct moldisplaydata));
							assert(info->mddata);
							for(i = 0; i < newlist->Nmol; ++i){
								struct mol *newmol = newlist->mols[i];
								append_mol(mol_list, newmol);
								//normalize_mol(mol, 1.0);
								calc_moldisplaydata(newmol, &mddata[mol_list->Nmol-1], mddata[info->ht.mol].molrect.left,
									 mddata[info->ht.mol].molrect.top, info->scale);
								molorigin(newmol);
								adjustmolrect(newmol, &mddata[mol_list->Nmol-1]);
							}
							disband_mol_list(newlist);
							delete_mol(mol_list, info->ht.mol);
							deletemoldisplaydata(mol_list, &info->mddata, info->ht.mol);
						}
					InvalidateRect(hwnd, NULL, TRUE);
					}
					break;

				case T_BOND:
					StoreUndoInfo(info);
					if(info->ht.bond >= 0) {
						InvalidateBond(hwnd, mol, mddata, info->ht.bond);
						rotate_bondorder(mol, info->ht.bond);
						info->lbuttondown = FALSE;
					}
					else {
						SetCapture(hwnd);
						startnewbond(&info->tbi, info->mddata, &info->ht, LOSHORT(lParam), HISHORT(lParam));
					}

					break;

				case T_HETEROATOM:
					if(info->ht.atom >= 0) {
						struct atom *atom;

						StoreUndoInfo(info);
						atom = &mol->atoms[info->ht.atom];
						setatomZ(mol, info->ht.atom, ((atom->Z - 6) + 1) % 3 + 6);
						InvalidateAtom(hwnd, mddata, info->ht.atom);
					}

					break;

				case T_SELECT:			
					if(info->ht.mol >= 0) {
						SetCapture(hwnd);
						info->firstmove = TRUE;
						info->lbuttondown = TRUE;
						info->lastx = LOSHORT(lParam);
						info->lasty = HISHORT(lParam);
					}
					break;
			}
			}
			break;

		case WM_MOUSEMOVE:
			SetCursor(info->tools[info->tool].cursor);
			if(info->lbuttondown && (info->tool == T_SELECT)) {
				if (info->ht.mol >= 0) { //move the selected molecule
					struct mol *mol = mol_list->mols[info->ht.mol];
					struct moldisplaydata *data = &(info->mddata[info->ht.mol]);
					if(info->firstmove) {
						StoreUndoInfo(info);
						info->firstmove = FALSE;
					}
					InvalidateMol(hwnd, data);
					movemol(mol, data, LOSHORT(lParam) - info->lastx, HISHORT(lParam) - info->lasty);
					info->lastx = LOSHORT(lParam);
					info->lasty = HISHORT(lParam);
					InvalidateMol(hwnd, data);
				}//else it _should_ draw a lasso or something
			}
			else
			{
				struct molht htest, *ht = &info->ht;
				struct moldisplaydata *data = info->mddata;

				htest = mollisthittest(info->mol_list, info->mddata, LOSHORT(lParam), HISHORT(lParam));
				
				if((info->tools[info->tool].select_atoms) && (ht->atom != htest.atom)) {
					if(ht->atom >= 0)
						InvalidateAtom(hwnd, &data[ht->mol], ht->atom);
					if(htest.atom >= 0) {
						if(ht->bond >= 0) {
							InvalidateBond(hwnd, mol_list->mols[ht->mol], &data[ht->mol], ht->bond);
							ht->bond = -1;
						}
						InvalidateAtom(hwnd, &data[htest.mol], htest.atom);
					}
										
					ht->atom = htest.atom;
				} 
				if((ht->atom < 0) && (info->tools[info->tool].select_bonds)
					 && (htest.bond != ht->bond) ) //looks for near bond if no near atom
				{
					if(ht->bond >= 0)
						InvalidateBond(hwnd, mol_list->mols[ht->mol], &data[ht->mol], ht->bond);
					if(htest.bond >= 0)
						InvalidateBond(hwnd, mol_list->mols[htest.mol], &data[htest.mol], htest.bond);
										
					ht->bond = htest.bond;
				}

				if(info->tools[info->tool].select_mol)
					if(htest.mol != ht->mol) {
					if(ht->mol >= 0)
						InvalidateMol(hwnd, &data[ht->mol]);
					if(htest.mol >= 0)
						InvalidateMol(hwnd, &data[htest.mol]);
				}
				ht->mol = htest.mol;
				

				if(info->lbuttondown && (info->tool == T_BOND))
				{
					InvalidateTempBond(hwnd, &info->tbi, info->mddata);
					tempbond(&info->tbi, data, ht, LOSHORT(lParam), HISHORT(lParam), (wParam & MK_CONTROL));
					InvalidateTempBond(hwnd, &info->tbi, info->mddata);
				}
			}
			break;

		case WM_LBUTTONUP:
			if(info->lbuttondown) {
				ReleaseCapture();
				info->lbuttondown = FALSE;
				InvalidateRect(hwnd, NULL, TRUE);
				if(info->tool == T_BOND)
					endnewbond(&info->tbi, &(info->mol_list), &(info->mddata), &info->ht, info->scale);
				else if(info->tool == T_SELECT){
					if(info->ht.mol >= 0) { //select molecule
						info->Nselect = 1;
						info->selection = calloc(1, sizeof(struct selection));
						info->selection[0].mol = info->ht.mol;
					} else  //unselect molecule
						ClearSelection(info);
				}
			}
			break;
		

		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case ID_BUTTONERASE:{
					struct string_notification sn;

					info->tool = T_ERASE;
					sn.nmhdr.hwndFrom = hwnd;
					sn.nmhdr.idFrom =	GetWindowLong(hwnd, GWL_ID); 
					sn.nmhdr.code = MN_SETSTATUSTEXT;
					sn.s = info->tools[info->tool].helpstr;
					SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (long)&sn);
					ClearSelection(info);
					InvalidateRect(hwnd, NULL, TRUE);
					}
					break;

				case ID_BUTTONSELECT:{
					struct string_notification sn;

					info->tool = T_SELECT;
					sn.nmhdr.hwndFrom = hwnd;
					sn.nmhdr.idFrom =	GetWindowLong(hwnd, GWL_ID); 
					sn.nmhdr.code = MN_SETSTATUSTEXT;
					sn.s = info->tools[info->tool].helpstr;
					SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (long)&sn);
					}
					break;

				case ID_BUTTONHETEROATOM:{
					struct string_notification sn;

					info->tool = T_HETEROATOM;
					sn.nmhdr.hwndFrom = hwnd;
					sn.nmhdr.idFrom =	GetWindowLong(hwnd, GWL_ID); 
					sn.nmhdr.code = MN_SETSTATUSTEXT;
					sn.s = info->tools[info->tool].helpstr;
					SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (long)&sn);
					ClearSelection(info);
					InvalidateRect(hwnd, NULL, TRUE);
					}
					break;

				case ID_BUTTONBOND:{
					struct string_notification sn;

					info->tool = T_BOND;
					sn.nmhdr.hwndFrom = hwnd;
					sn.nmhdr.idFrom =	GetWindowLong(hwnd, GWL_ID); 
					sn.nmhdr.code = MN_SETSTATUSTEXT;
					sn.s = info->tools[info->tool].helpstr;
					SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (long)&sn);
					ClearSelection(info);
					InvalidateRect(hwnd, NULL, TRUE);
					}
					break;


			}
			break;


		default:                  /* Passes it on if unproccessed    */
			return (DefWindowProc(hwnd, message, wParam, lParam));
	}

	return (0);
}

		
void startnewbond(struct tempbondinfo *tbi, struct moldisplaydata *data, struct molht *ht, int x, int y)
{

	tbi->draw = FALSE;
	
	tbi->endatom = -1;
	tbi->endmol  = -1;
	if((tbi->startatom = ht->atom) >= 0)
	{
		tbi->startmol = ht->mol;
		tbi->startpos.x = data[ht->mol].points[ht->atom].x;
		tbi->startpos.y = data[ht->mol].points[ht->atom].y;
		tbi->endpos.x = tbi->startpos.x;
		tbi->endpos.y =	tbi->startpos.y;
	}
	else
	{
		tbi->startmol = -1;
		tbi->startpos.x = x;
		tbi->startpos.y = y;
		tbi->endpos.x = tbi->startpos.x;
		tbi->endpos.y =	tbi->startpos.y;
	}
}

void endnewbond(struct tempbondinfo *tbi, struct mol_list **list, struct moldisplaydata **mddata, struct molht *ht, int scale)
{
	double x, y;
	struct mol *mol;
	struct moldisplaydata *data = *mddata;
	struct mol_list *mol_list = *list;
	int sprout = 0;

	tbi->draw = FALSE;
	
	if((tbi->startmol < 0) && (tbi->endmol < 0)){ //newmol
		mol = calloc(1, sizeof(struct mol));
		assert(mol);
		mol_list->mols = realloc(mol_list->mols, (mol_list->Nmol + 1) * sizeof(struct mol *));
		assert(mol_list->mols);
		mol_list->mols[mol_list->Nmol] = mol;
		tbi->startmol = tbi->endmol = mol_list->Nmol;
		data = realloc(data, (mol_list->Nmol + 1) * sizeof(struct moldisplaydata));
		assert(data);
		calc_moldisplaydata(mol, &data[mol_list->Nmol], 0, 0, scale);
		++mol_list->Nmol;
	}
	else if((tbi->startmol >= 0) && (tbi->endmol >= 0) && (tbi->startmol != tbi->endmol)){
		//combine molecules
		//MessageBox(NULL, "comb", "", MB_OK);
		//return;
		mol = combine_mols(mol_list->mols[tbi->startmol], mol_list->mols[tbi->endmol],
			(data[tbi->endmol].molrect.left - data[tbi->startmol].molrect.left) / (double)scale,
			(-data[tbi->endmol].molrect.top + data[tbi->startmol].molrect.top) / (double)scale);
		free(data[tbi->startmol].points);
		calc_moldisplaydata(mol, &data[tbi->startmol], data[tbi->startmol].molrect.left, data[tbi->startmol].molrect.top, scale);
		tbi->endatom += mol_list->mols[tbi->startmol]->Natom;
		destroy_mol(mol_list->mols[tbi->startmol]);
		destroy_mol(mol_list->mols[tbi->endmol]);
		mol_list->mols[tbi->startmol] = mol;
		--mol_list->Nmol;
		if(tbi->endmol < mol_list->Nmol){
			mol_list->mols[tbi->endmol] = mol_list->mols[mol_list->Nmol];
			mol_list->mols = realloc(mol_list->mols, mol_list->Nmol * sizeof(struct mol*));
			free(data[tbi->endmol].points);
			data[tbi->endmol] = data[mol_list->Nmol];
			data = realloc(data, mol_list->Nmol * sizeof(struct moldisplaydata));
			if(tbi->startmol == mol_list->Nmol)
				tbi->startmol = tbi->endmol;
			else
				tbi->endmol = tbi->startmol;
		} else {
			mol_list->mols = realloc(mol_list->mols, mol_list->Nmol * sizeof(struct mol*));
			free(data[tbi->endmol].points);
			data = realloc(data, mol_list->Nmol * sizeof(struct moldisplaydata));
			tbi->endmol = tbi->startmol;
		}

	}
	else { //bond involves only one molecule
		if(tbi->startmol < 0)
			tbi->startmol = tbi->endmol;
		else if(tbi->endmol < 0)
			tbi->endmol = tbi->startmol;
		mol = mol_list->mols[tbi->startmol];
	}


	if(tbi->startatom < 0)
	{
		x =	(double)(tbi->startpos.x - data[tbi->startmol].molrect.left)/ (double)scale;
		y = -(double)(tbi->startpos.y - data[tbi->startmol].molrect.top)/ (double)scale;
		tbi->startatom = new_atom(mol, C, 0, x, y, 0.0);
		addatomdisplaydata(mol, &data[tbi->startmol], tbi->startpos.x, tbi->startpos.y);
	}

	if(tbi->endatom < 0)
	{
		if((tbi->endpos.x == tbi->startpos.x) && (tbi->endpos.y == tbi->startpos.y)) { // sprout
			sprout = sprout_atom(mol, tbi->startatom, C);
			tbi->endpos.x = (int)(mol->atoms[sprout].x * scale + data[tbi->startmol].molrect.left);
			tbi->endpos.y = (int)(-mol->atoms[sprout].y * scale + data[tbi->startmol].molrect.top);
		} else {
			x =	(double)(tbi->endpos.x - data[tbi->startmol].molrect.left)/ (double)scale;
			y = -(double)(tbi->endpos.y - data[tbi->startmol].molrect.top)/ (double)scale;
			tbi->endatom = new_atom(mol, C, 0, x, y, 0.0);
		}
		addatomdisplaydata(mol, &data[tbi->startmol], tbi->endpos.x, tbi->endpos.y);
	}

	if(!sprout)
		new_bond(mol, tbi->startatom, tbi->endatom);
	adjustmolrect(mol, &data[tbi->startmol]);
	molorigin(mol);
	*mddata = data;
	*list = mol_list;
	ht->atom = ht->mol = ht->bond = -1;
}


/*
	int startatom, endatom, startmol, endmol;
	POINT startpos, endpos;
	BOOL anydir, draw;
  */
void tempbond(struct tempbondinfo *tbi, struct moldisplaydata *data, struct molht *ht, int x, int y, BOOL anydir)
{
	double th;
	
	if(ht->atom >=0)
	{
		if((ht->atom == tbi->startatom) && (ht->mol == tbi->startmol)) //sprout
		{
			tbi->draw = FALSE;
			tbi->endpos.x = tbi->startpos.x;
			tbi->endpos.y =	tbi->startpos.y;
		}
		else    // bond to a pre-existing atom
		{
			tbi->endatom = ht->atom;
			tbi->endmol  = ht->mol;
			tbi->endpos.x = data[ht->mol].points[ht->atom].x;
			tbi->endpos.y = data[ht->mol].points[ht->atom].y;
			tbi->draw = TRUE;
		}
	}
	else  //bond to a new atom
	{
		tbi->endatom = tbi->endmol = -1;
		tbi->draw = TRUE;

		if(anydir)
		{
			tbi->endpos.x = x;
			tbi->endpos.y = y;
		}
		else
		{
			th = (round(atan2(y - tbi->startpos.y, x - tbi->startpos.x) * 12.0/3.141593)) * 3.141593/12.0;
			tbi->endpos.x = tbi->startpos.x + (int)(DEFBONDLENGTH * cos(th));
			tbi->endpos.y = tbi->startpos.y + (int)(DEFBONDLENGTH * sin(th));
		}

	}


}


struct tool *init_tools(void)
{
	struct tool *tools;
	extern HINSTANCE hInst;
	
	tools = calloc(N_TOOLS, sizeof(struct tool));
	assert(tools);
					
	tools[T_SELECT].cursor = LoadCursor(NULL, IDC_ARROW);
	tools[T_SELECT].select_atoms = FALSE;
	tools[T_SELECT].select_bonds = FALSE;
	tools[T_SELECT].select_mol = TRUE;
	tools[T_SELECT].helpstr = "Selector tool";

	tools[T_ERASE].cursor = LoadCursor(hInst, "ERASERCURSOR");
	tools[T_ERASE].select_atoms = TRUE;
	tools[T_ERASE].select_bonds = TRUE;
	tools[T_ERASE].select_mol = FALSE;
	tools[T_ERASE].helpstr = "Click to erase atom or bond";

	tools[T_BOND].cursor = LoadCursor(NULL, IDC_CROSS);
	tools[T_BOND].select_atoms = TRUE;
	tools[T_BOND].select_bonds = TRUE;
	tools[T_BOND].select_mol = FALSE;
	tools[T_BOND].helpstr = "Click and drag to draw bond";

	tools[T_HETEROATOM].cursor = LoadCursor(NULL, IDC_CROSS);
	tools[T_HETEROATOM].select_atoms = TRUE;
	tools[T_HETEROATOM].select_bonds = FALSE;
	tools[T_HETEROATOM].select_mol = FALSE;
	tools[T_HETEROATOM].helpstr = "Click to insert heteroatom";

	return(tools);


}


