/* molgraph.c
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


LOGFONT lognumfont, logsymbolfont;
HFONT	numfont, symbolfont;

//it is recommended to normalize the molecule first.
void calc_moldisplaydata(struct mol *mol, struct moldisplaydata *data, int xpos, int ypos, int scale)
{
	int i;
	struct atom *atom;

	data->molrect.left = data->molrect.right  = xpos;
	data->molrect.top  = data->molrect.bottom = ypos;

	if(mol->Natom > 0) {

		data->points = malloc(mol->Natom * sizeof(POINT));
		assert(data->points);

		for(i = 0; i < mol->Natom; ++i){
			atom = &mol->atoms[i];
			data->points[i].x = xpos + round(atom->x * scale);
			data->points[i].y = ypos + round(atom->y * -scale);
			if(data->points[i].x > data->molrect.right)
				data->molrect.right = data->points[i].x;
			if(data->points[i].y > data->molrect.bottom)
				data->molrect.bottom = data->points[i].y;
		}
	}
	else {
		data->points = malloc(sizeof(POINT));
		assert(data->points);
	}

}


void adjustmolrect(struct mol *mol, struct moldisplaydata *data)
{
	RECT *r = &data->molrect;
	int i;

	r->left = r->right  = data->points[0].x;
	r->top  = r->bottom = data->points[0].y;

	for(i = 1; i < mol->Natom; ++i) {
		if(data->points[i].x < r->left)
			r->left = data->points[i].x;
		else if(data->points[i].x > r->right)
			r->right = data->points[i].x;
		
		if(data->points[i].y < r->top)
			r->top = data->points[i].y;
		else if(data->points[i].y > r->bottom)
			r->bottom = data->points[i].y;
	}

}

void deletemoldisplaydata(struct mol_list *list, struct moldisplaydata **data, int n_mol)
{
	free((*data)[n_mol].points);
	if(n_mol < list->Nmol)
		(*data)[n_mol]= (*data)[list->Nmol];
	*data = realloc(*data, list->Nmol * sizeof(struct moldisplaydata));
	assert(*data || (list->Nmol == 0));
}

void deleteatomdisplaydata(struct mol *mol, struct moldisplaydata *data, int n_atom)
{
	if(n_atom < mol->Natom)	
		data->points[n_atom] = data->points[mol->Natom];
	data->points = realloc(data->points, mol->Natom*sizeof(POINT));
	assert((data->points) || (!mol->Natom));

}

void addatomdisplaydata(struct mol *mol, struct moldisplaydata *data, int x, int y)
{
	data->points = realloc(data->points, mol->Natom*sizeof(POINT));
	assert(data->points);
	data->points[mol->Natom - 1].x = x;
	data->points[mol->Natom - 1].y = y;
}

struct moldisplaydata *mddatadup(struct moldisplaydata *data, struct mol_list *list)
{
	struct moldisplaydata *newdata;
	int i;

	newdata = malloc(list->Nmol * sizeof(struct moldisplaydata));
	assert(newdata);
	memcpy(newdata, data, list->Nmol * sizeof(struct moldisplaydata));
	for(i = 0; i < list->Nmol; ++i) {
		newdata[i].points = malloc(list->mols[i]->Natom * sizeof(POINT));
		assert(newdata[i].points);
		memcpy(newdata[i].points, data[i].points, list->mols[i]->Natom * sizeof(POINT));
	}
	return(newdata);
}

struct moldisplaydata *destroy_mddata(struct moldisplaydata *data, struct mol_list *list)
{
	int i;

	for(i = 0; i < list->Nmol; ++i)
		free(data[i].points);
	free(data);

	return(NULL);
}



int _near_atom(struct mol *mol, struct moldisplaydata *data, int x, int y)
{
	int i;
	
		for(i=0;
			(i < mol->Natom) && ((abs(x-data->points[i].x) > SELECTRADIUS) || (abs(y-data->points[i].y) > SELECTRADIUS));
			++i );

	if(i<mol->Natom)
		return(i);
	else
		return(-1);
}

int _near_bond(struct mol *mol, struct moldisplaydata *data, int x, int y)
{
	POINT b, r; //b = bond vector from a1 to a2; r = vector from a1 to (x, y)
	int i,a1,a2, b2,d2; //b2 = sq(|b|); d2 = sq(|r.b|)
	
	for(i=0; (i < mol->Nbond); ++i)
	{
		a1 = mol->bonds[i].a1;
		a2 = mol->bonds[i].a2;
		b.x = data->points[a2].x - data->points[a1].x;
		b.y = data->points[a2].y - data->points[a1].y;
		r.x = x - data->points[a1].x;
		r.y = y - data->points[a1].y;
		b2 = b.x*b.x + b.y*b.y;
		d2 = r.x*b.x + r.y*b.y;
		if((d2 < b2) && (d2 > 0) && (abs(r.x*b.y - r.y*b.x) < round(SELECTRADIUS*sqrt(b2))))
			break;
	}
	
	if(i < mol->Nbond)
		return(i);
	else
		return(-1);
}

BOOL _nearmol(struct mol *mol, struct moldisplaydata *data, int x, int y)
{
	return((x >= data->molrect.left) && (x <= data->molrect.right)
		&& (y >= data->molrect.top) && (y <= data->molrect.bottom));
}


struct molht mollisthittest(struct mol_list *mol_list, struct moldisplaydata *data, int x, int y)
{
	struct molht ht;
	int i;
	
	ht.atom = ht.mol = ht.bond = -1;
	
	for(i = 0; i < mol_list->Nmol; ++i){
		if((ht.atom = _near_atom(mol_list->mols[i], &data[i], x, y)) >= 0){
			ht.mol = i;
			break;
		}
	}
	
	for(i = 0; i < mol_list->Nmol; ++i){
		if((ht.bond = _near_bond(mol_list->mols[i], &data[i], x, y)) >= 0){
			if(ht.mol < 0)
				ht.mol = i;
			else if(ht.mol != i)
				ht.bond = -1;
			break;
		}
	}

	if(ht.mol < 0){
		for(i = 0; i < mol_list->Nmol; ++i){
			if(_nearmol(mol_list->mols[i], &data[i], x, y)) {
				if(ht.mol < 0)
					ht.mol = i;
//				else if (((data[i].molrect.right - data[i].molrect.left) < 
//					(data[ht.mol].molrect.right - data[ht.mol].molrect.left))
//					&&	((data[i].molrect.bottom - data[i].molrect.top) < 
//					(data[ht.mol].molrect.bottom - data[ht.mol].molrect.top)))
//						ht.mol = i;
			}
		}
	}
  
	return(ht);
}




void InvalidateAtom(HWND hwnd, struct moldisplaydata *data, int atom)
{
	RECT r;

	r.top = data->points[atom].y  - ATOMSIZE;
	r.bottom = r.top + 2*ATOMSIZE;
	r.left = data->points[atom].x  - ATOMSIZE;
	r.right = r.left + 2*ATOMSIZE;
	InvalidateRect(hwnd, &r, TRUE);
}

void InvalidateBond(HWND hwnd, struct mol *mol, struct moldisplaydata *data, int bond)
{
	RECT r;
	int t;

	r.top = data->points[mol->bonds[bond].a1].y;
	r.bottom = data->points[mol->bonds[bond].a2].y;
	r.left = data->points[mol->bonds[bond].a1].x;
	r.right = data->points[mol->bonds[bond].a2].x;
	if(r.top > r.bottom)
	{
		t = r.top;
		r.top = r.bottom;
		r.bottom = t;
	}
	if(r.left > r.right)
	{
		t = r.left;
		r.left = r.right;
		r.right = t;
	}	
	r.top -= BONDSPACE+1;
	r.bottom += BONDSPACE+1;
	r.left -= BONDSPACE+1;
	r.right += BONDSPACE+1;
		
	InvalidateRect(hwnd, &r, TRUE);
}

void InvalidateTempBond(HWND hwnd, struct tempbondinfo *tbi, struct moldisplaydata *data)
{
	RECT r;
	int t;

	r.top = tbi->startpos.y;
	r.bottom = tbi->endpos.y;
	r.left = tbi->startpos.x;
	r.right = tbi->endpos.x;

	if(r.top > r.bottom)
	{
		t = r.top;
		r.top = r.bottom;
		r.bottom = t;
	}
	if(r.left > r.right)
	{
		t = r.left;
		r.left = r.right;
		r.right = t;
	}	
	--r.top;
	++r.bottom;
	--r.left;
	++r.right;
		
	InvalidateRect(hwnd, &r, TRUE);
}

void InvalidateMol(HWND hwnd, struct moldisplaydata *data)
{
	RECT r;

	r = data->molrect;
	r.top -= 20;
	r.bottom += 20;
	r.left -= 20;
	r.right += 20;
	InvalidateRect(hwnd, &r, TRUE);

}



void init_fonts(void)
{
	memset(&lognumfont, 0, sizeof(lognumfont));
	lognumfont.lfHeight = 12;
	lognumfont.lfCharSet = ANSI_CHARSET;
	strcpy(lognumfont.lfFaceName,"arial");
	numfont = CreateFontIndirect(&lognumfont);

	memset(&logsymbolfont, 0, sizeof(logsymbolfont));
	logsymbolfont.lfHeight = 20;
	logsymbolfont.lfCharSet = ANSI_CHARSET;
	logsymbolfont.lfWeight = FW_BOLD;
	strcpy(logsymbolfont.lfFaceName,"arial");
	symbolfont = CreateFontIndirect(&logsymbolfont);

}





void draw_bond(HDC hdc, int x1, int y1, int x2, int y2, int order)
{
	double length, deltax, deltay;
	int dx, dy;

	if(order > 1)
	{
		deltay = y2 - y1;
		deltax = x2 - x1;
		length = sqrt(deltax*deltax + deltay*deltay);
		dx = (int)(deltay * BONDSPACE / length );
		dy = (int)(deltax * BONDSPACE / length );
	}
	if(order != 2)
	{
		line(hdc, x1, y1, x2, y2);
		if(order == 3)
		{
			line(hdc, x1 + dx, y1 - dy, x2 + dx, y2 - dy);
			line(hdc, x1 - dx, y1 + dy, x2 - dx, y2 + dy);
		}
	}
	else     ///type = 2
	{
		line(hdc, x1 + dx/2, y1 - dy/2, x2 + dx/2, y2 - dy/2);
		line(hdc, x1 - dx/2, y1 + dy/2, x2 - dx/2, y2 + dy/2);
	}
}


void highlight_bond(HDC hdc, int x1, int y1, int x2, int y2)
{
	double length, deltax, deltay;
	int dx, dy;

	deltay = y2 - y1;
	deltax = x2 - x1;
	length = sqrt(deltax*deltax + deltay*deltay);
	dx = (int)(deltay * BONDSPACE / length );
	dy = (int)(deltax * BONDSPACE / length );

	line(hdc, x1 + dx, y1 - dy, x2 + dx, y2 - dy);
	LineTo(hdc, x2 - dx, y2 + dy);
	LineTo(hdc, x1 - dx, y1 + dy);
	LineTo(hdc, x1 + dx, y1 - dy);
}

void highlight_atom(HDC hdc, struct mol *mol, struct moldisplaydata *data, int atom)
{
	int x, y;
	x = data->points[atom].x;
	y = data->points[atom].y;

	Ellipse(hdc, x-SELECTRADIUS, y-SELECTRADIUS, x+SELECTRADIUS, y+SELECTRADIUS);
}

void highlight_mol(HDC hdc, struct moldisplaydata *data)
{
	Rectangle(hdc, data->molrect.left - 10, data->molrect.top - 10,
		 data->molrect.right + 10, data->molrect.bottom + 10); 
}


int paintmol(HDC hdc, struct mol *mol, struct moldisplaydata *data, BOOL paint_numbers, BOOL skeleton)
{
	int i, x, y;
	struct bond *bond;
	struct atom *atom;
	extern char *symbol[];
	HPEN oldpen;
	HFONT oldfont;
	char s[10];

	for(i=0; i<mol->Nbond; ++i) {
		bond = &mol->bonds[i];
		draw_bond(hdc, data->points[bond->a1].x, data->points[bond->a1].y,
				  data->points[bond->a2].x, data->points[bond->a2].y,
				  skeleton ? 1 : bond->order);
	}

	if(!skeleton)
		for(i=0; i< mol->Natom; ++i)
		{
			atom = &mol->atoms[i];

			x = data->points[i].x;
			y = data->points[i].y;

			if(atom->Z != C)  // writes symbol
			{
				oldpen = SelectObject(hdc, GetStockObject(NULL_PEN));
				oldfont = SelectObject(hdc, symbolfont); 
									
				Ellipse(hdc, x-ATOMSIZE, y-ATOMSIZE, x+ATOMSIZE, y+ATOMSIZE);

				SelectObject(hdc, oldpen);
				SetTextAlign(hdc, TA_CENTER);

				ExtTextOut(hdc, x, y-ATOMSIZE, 0, NULL,
						 symbol[atom->Z], strlen(symbol[atom->Z]), NULL);

				SelectObject(hdc, oldfont);

			} 
			if (paint_numbers)				 /////writes atom number
			{
				oldfont = SelectObject(hdc, numfont); 
				wsprintf(s, "%i", i+1);
				ExtTextOut(hdc, x + 7, y + 7, 0, NULL, s, strlen(s), NULL);
				SelectObject(hdc, oldfont);
			}
		}

	return(0);
}

void highlight_ht(HDC hdc, struct mol_list *mol_list, struct moldisplaydata *mddata, struct molht *ht, struct tool tool)
{
	HPEN oldpen;
	HBRUSH oldbrush;

	oldpen = SelectObject(hdc, CreatePen(PS_SOLID, 0, RGB(0, 0, 255)));
	oldbrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));

	if(ht->mol >= 0) {
		struct moldisplaydata *data = &mddata[ht->mol];
		if(ht->atom >= 0){
			highlight_atom(hdc, mol_list->mols[ht->mol], data, ht->atom);
		} else if(ht->bond >= 0){
			struct bond *bond = &mol_list->mols[ht->mol]->bonds[ht->bond];
			highlight_bond(hdc, data->points[bond->a1].x, data->points[bond->a1].y,
					  data->points[bond->a2].x, data->points[bond->a2].y );
		} else if((ht->mol >= 0) && tool.select_mol) {
			highlight_mol(hdc, data);
		}

	}
	DeleteObject(SelectObject(hdc, oldpen));
	SelectObject(hdc, oldbrush);

}


void highlight_selection(HDC hdc, struct moldisplaydata *mddata, struct selection *selection, int Nselect)
{
	HPEN oldpen;
	HBRUSH oldbrush;
	int i;

	oldpen = SelectObject(hdc, CreatePen(PS_SOLID, 0, RGB(255, 0, 0)));
	oldbrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));

	for(i = 0; i < Nselect; ++i){
		struct moldisplaydata *data = &mddata[selection[i].mol];
		highlight_mol(hdc, data);
	}

	DeleteObject(SelectObject(hdc, oldpen));
	SelectObject(hdc, oldbrush);

}


	
void delete_lone_atoms(HWND hwnd, struct mol *mol, struct moldisplaydata *data)
{
	int i;
	
	for(i=0; i < mol->Natom; ++i)
		while((i < mol->Natom) && (mol->atoms[i].Nbond == 0))
		{
			InvalidateAtom(hwnd, data, i);
			deleteatom(mol, i);
			deleteatomdisplaydata(mol, data, i);

		}
}

void movemol(struct mol *mol, struct moldisplaydata *data, int dx, int dy)
{
	int i;

	data->molrect.left += dx;
	data->molrect.right += dx;
	data->molrect.top += dy;
	data->molrect.bottom += dy;

	for(i = 0; i < mol->Natom; ++i) {
		data->points[i].x += dx;
		data->points[i].y += dy;
	}

}
