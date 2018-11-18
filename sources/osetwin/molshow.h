/* molshow.h
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

#include "windows.h"
#include "../mollib/mollib.h"

#define	MAXX			640								
#define MAXY			480
#define MOLDISPMAXX		1024
#define MOLDISPMAXY		768
#define SCALE			50.0
#define DEFBONDLENGTH	40
#define ATOMSIZE		10
#define SELECTRADIUS	6
#define BONDSPACE		6
#define WINDOWNAME		"OSETWIN"
#define OP_SET			1
#define OP_REMOVE		2
#define OP_TOGGLE		3
#define N_TOOLS	4		 

#define MEM_SETMOL			WM_USER+1
#define	MEM_GETMOL			WM_USER+2
#define MEM_COPY			WM_USER+3
#define	MEM_CUT				WM_USER+4
#define MEM_PASTE			WM_USER+5
#define	MEM_GETMOLCOUNT		WM_USER+6
#define MEM_CANPASTE		WM_USER+7
#define	MEM_CANCOPY			WM_USER+8
#define MEM_CANUNDO			WM_USER+9
#define	MEM_GETMODIFY		WM_USER+10
#define MEM_UNDO			WM_USER+11
#define MEM_REDO			WM_USER+12
#define MEM_RESETMOL		WM_USER+13
#define MEM_TOGGLENUMBERS	WM_USER+14
#define MEM_GETSELECTEDMOL	WM_USER+15
/* If there is no selection but there is only one molecule, it returns it.
 * if the selection is not exactly one whole molecule, it returns an empty mol */
#define MEM_DELETE_SELECTION WM_USER+16


#define MAM_SETROOT			WM_USER + 100
#define MAM_UP				WM_USER + 101
#define MAM_SET_INFO_WND	WM_USER + 102
#define MAM_SET_TGT_WND		WM_USER + 103



#define MDM_SETLIST			WM_USER+1
#define MDM_NEXT			WM_USER+2
#define MDM_PREV			WM_USER+3
#define MDM_REFRESHTEXT		WM_USER+4
#define MDM_GETRXNINFO		WM_USER+5
#define MDM_SETDISPWND		WM_USER+6

#define MN_SIZENOTIFY		1
#define MN_SETSTATUSTEXT	2

#define LOSHORT(l)   ((SHORT) (l)) 
#define HISHORT(l)   ((SHORT) (((INT) (l) >> 16) & 0xFFFF)) 

struct tool
{
	HCURSOR cursor;
	BOOL select_atoms, select_bonds, select_mol;
	char *helpstr;
};
 
struct molht {
	int mol;
	int bond;
	int atom;
};

struct selection {
	int mol;
	int *atoms;
	int *bonds;
};

struct moldisplaydata
{
	POINT *points;
	RECT molrect;
};

struct tempbondinfo
{
	int startmol, endmol;
	int startatom, endatom;
	POINT startpos, endpos;
	BOOL draw;
};

struct size_notification
{
	NMHDR nmhdr;
	int x, y;
};

struct string_notification
{
	NMHDR nmhdr;
	char *s;
};


struct moleditwininfo{
	struct mol_list *mol_list;
	struct moldisplaydata *mddata;
	struct undo_node *undo;
	struct undo_node *redo;
	int scale;
	int tool;
	struct selection *selection;
	int Nselect;
	struct molht ht;
	struct tempbondinfo tbi;
	BOOL paint_numbers;
	BOOL modified;
	BOOL lbuttondown;
	BOOL firstmove;
	int lastx, lasty;
	struct tool *tools;
};


enum tools {T_SELECT, T_ERASE, T_BOND, T_HETEROATOM};


// molaux.c     very basic functions
void line(HDC hdc, int x1, int y1, int x2, int y2);
int i_square(int x);
int round(double x);
int setmenuitems(HMENU hmenu, int flag, int op, int item, ...);


// molgraph.c
void calc_moldisplaydata(struct mol *mol, struct moldisplaydata *data, int xpos, int ypos, int scale);
int paintmol(HDC hdc, struct mol *mol, struct moldisplaydata *data, BOOL paint_numbers, BOOL skeleton);
void init_fonts(void);
void InvalidateAtom(HWND hwnd, struct moldisplaydata *data, int atom);
void InvalidateBond(HWND hwnd, struct mol *mol, struct moldisplaydata *data, int bond);
void deletemoldisplaydata(struct mol_list *list, struct moldisplaydata **data, int n_mol);
void deleteatomdisplaydata(struct mol *mol, struct moldisplaydata *data, int n_atom);
void addatomdisplaydata(struct mol *mol, struct moldisplaydata *data, int x, int y);
void delete_lone_atoms(HWND hwnd, struct mol *mol, struct moldisplaydata *data);
void InvalidateTempBond(HWND hwnd, struct tempbondinfo *tbi, struct moldisplaydata *data);
struct molht mollisthittest(struct mol_list *mol_list, struct moldisplaydata *data, int x, int y);
void highlight_ht(HDC hdc, struct mol_list *mol_list, struct moldisplaydata *mddata, struct molht *ht, struct tool tool);
void highlight_selection(HDC hdc, struct moldisplaydata *mddata, struct selection *selection, int Nselect);
void draw_bond(HDC hdc, int x1, int y1, int x2, int y2, int order);
void adjustmolrect(struct mol *mol, struct moldisplaydata *data);
void movemol(struct mol *mol, struct moldisplaydata *data, int dx, int dy);
void InvalidateMol(HWND hwnd, struct moldisplaydata *data);
struct moldisplaydata *mddatadup(struct moldisplaydata *data, struct mol_list *list);
struct moldisplaydata *destroy_mddata(struct moldisplaydata *data, struct mol_list *list);



//moldlgs.c
OPENFILENAME * init_openfiledlg(HWND hwnd, char *title, char *filter, int nfilter, char *dir, char *defext);
BOOL CALLBACK DialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

// moledit.c
LRESULT CALLBACK MolEditWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

// moldisp.c
LRESULT CALLBACK MolDispWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);


// clpbrd.c
void InitClipboard();
void CopyMolToClipboard(HWND hwnd, struct mol *mol);
struct mol *GetMolFromClipboard(HWND hwnd);
