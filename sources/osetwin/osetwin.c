/* molshow.c
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
#include <stdarg.h>
#include <math.h>
#include <commctrl.h>
#include "resource.h"

#define getmol()	(struct mol*)SendMessage(hactive, MEM_GETSELECTEDMOL, 0, 0)

#define	N_BUTTONBITMAPS 9

#define MODE_EDIT		0
#define MODE_ANALYSIS	1

///////////////////     Global Variables ////////////////////////
HINSTANCE hInst;
HWND	mainwnd;
char *commandline;
WNDPROC defeditproc, deftreeproc;
const char *appname = "OSETWIN";
struct cplx_params *cplx_params; 
struct struct_info *comp_info;
/////////////////////////////////////////////////////////////////


LRESULT CALLBACK MolRxnInfoWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MolTreeWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL InitApplication(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow);
HWND init_toolbar(TBBUTTON **buttons, HWND hwnd, HINSTANCE hInst);
void init_tree(HWND tree);
LRESULT CALLBACK TextWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MolAnalysisWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;    
	HACCEL haccel;

	commandline = lpCmdLine;

	if (!InitApplication(hInstance))
		return (FALSE); 

	if (!InitInstance(hInstance, nCmdShow))
		return (FALSE);

	haccel = LoadAccelerators(hInst, "MYACCELERATORS");

	
	while (GetMessage(&msg,NULL,0,0)) {
		if(!TranslateAccelerator(mainwnd, haccel, &msg))
		{
			TranslateMessage(&msg);   
			DispatchMessage(&msg);    
		}
	}
	
	return (msg.wParam);     
} 




BOOL InitApplication(HINSTANCE hInstance)
{
	WNDCLASS wc;
				  
	wc.style = 0L;           
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0; 
	wc.cbWndExtra = 0; 
	wc.hInstance = hInstance;   
	wc.hIcon = LoadIcon(hInstance, "DielsAlderIcon");
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName =  "Mymenu";
	wc.lpszClassName = "MolWClass"; 
	
	if(!RegisterClass(&wc))
		return(0);

	wc.style = 0L; 
	wc.lpfnWndProc = MolEditWndProc; 
	wc.cbWndExtra = sizeof(void *);             
	wc.hIcon = NULL;
	wc.hCursor = NULL;
	wc.lpszMenuName =  NULL;
	wc.lpszClassName = "MolEditWClass";

	if(!RegisterClass(&wc))
		return(0);

	wc.style = 0L; 
	wc.lpfnWndProc = MolAnalysisWndProc; 
	wc.cbWndExtra = sizeof(void *) * 2;
	wc.hIcon = NULL;
	wc.hCursor = NULL;
	wc.lpszMenuName =  NULL;
	wc.lpszClassName = "MolAnalysisWClass";

	if(!RegisterClass(&wc))
		return(0);

	wc.style = 0L;
	wc.lpfnWndProc = MolDispWndProc; 
	wc.cbWndExtra = 2 * sizeof(void *);             
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = "MolDispWClass";

	if(!RegisterClass(&wc))
		return(0);

	wc.style = 0L;
	wc.lpfnWndProc = TextWndProc; 
	wc.cbWndExtra = 0;
	wc.lpszClassName = "TextWindow";

	if(!RegisterClass(&wc))
		return(0);

	return(TRUE);
}



BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance;
	
	mainwnd = CreateWindow("MolWClass", WINDOWNAME, WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT, CW_USEDEFAULT, MAXX, MAXY,
			NULL, NULL, hInstance, NULL);

	if(mainwnd) {
		UpdateWindow(mainwnd);
		return (TRUE);
	} else
		return(FALSE);
}



LRESULT CALLBACK TextWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	 
	switch (message) {
		case WM_CREATE:{
			 HGLOBAL hr;
			 char *s;
			 int size, err;
			 HRSRC hrsrc;

			 hrsrc = FindResource(hInst, MAKEINTRESOURCE(IDMOL_GPLTEXT), "TEXTFILE");
			 err = GetLastError();
			 hr = LoadResource(hInst, hrsrc);
			 size = GlobalSize(hr);
			 s = LockResource(hr);
			 s[size-1] = 0;
			 
			 CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", s,
			 WS_CHILD | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_LEFT | WS_HSCROLL | WS_VISIBLE,
			 0, 0, 0, 0, hwnd, (HMENU) 1, hInst, 0);
		}break;

		case WM_SIZE:{
			RECT r;

			GetClientRect(hwnd, &r);
			MoveWindow(GetDlgItem(hwnd, 1), 0, 0, r.right, r.bottom, TRUE);
			}
			break;

		default:                  /* Passes it on if unproccessed    */
			return (DefWindowProc(hwnd, message, wParam, lParam));
	}

	return (0);

}



LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND hmoleditwnd; 
	static HWND hmoldispwnd;
	static HWND hmolanalysiswnd;
	static HWND hmolrxninfownd;
	static HWND hstatus;
	static HWND htoolbar;
	static HWND htooltips;
	static HWND hactive;
	static OPENFILENAME *lpofn;
	static TBBUTTON *buttons;
	static char windowname[100]= "";
	static int arr[2] = {0,0};
	static int mode;
	static int vert_division;
	static int horiz_division = 45;


	switch (message) {
		case WM_CREATE:{

			comp_info = init_comp_info();
			init_primes();
			cplx_params = init_cplx_params();

			InitClipboard();
			init_fonts();

			lpofn = init_openfiledlg(hwnd, "Open molecule", "Molfile (.mol)\0*.mol\0\0", 1, "C:\\IVAN\\C\\MOLSHOW\\", "mol");

			hstatus = CreateStatusWindow(WS_CHILD | WS_VISIBLE , "Ready", hwnd, 44444);

			htoolbar = init_toolbar(&buttons, hwnd, hInst);
			htooltips = (HWND)SendMessage(htoolbar, TB_GETTOOLTIPS, 0, 0);

			hactive = hmoleditwnd = CreateWindowEx(WS_EX_CLIENTEDGE, "MolEditWClass", "",
			 WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd, NULL, hInst, 0);

			hmolanalysiswnd = CreateWindowEx(WS_EX_CLIENTEDGE, "MolAnalysisWClass", "",
			 WS_CHILD | WS_HSCROLL, 0, 0, 0, 0, hwnd, NULL, hInst, 0);
  
			hmoldispwnd = CreateWindowEx(WS_EX_CLIENTEDGE, "MolDispWClass", "",
			 WS_CHILD, 0, 0, 0, 0, hwnd, NULL, hInst, comp_info->reaction);

			hmolrxninfownd = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",
			 WS_CLIPSIBLINGS | WS_THICKFRAME | WS_CHILD | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_LEFT,
			 0, 0, 0, 0, hwnd, NULL, hInst, 0);

			defeditproc = (WNDPROC) GetWindowLong(hmolrxninfownd, GWL_WNDPROC);
			SetWindowLong(hmolrxninfownd, GWL_WNDPROC, (long) MolRxnInfoWndProc);
			SendMessage(hmolrxninfownd, MDM_SETDISPWND, 0, (long) hmoldispwnd);
 
			SendMessage(hmolanalysiswnd, MAM_SET_INFO_WND, 0, (long) hmolrxninfownd);
			SendMessage(hmolanalysiswnd, MAM_SET_TGT_WND, 0, (long) hmoldispwnd);

			mode = MODE_EDIT;
			vert_division = 75;

///// command-line fileopen

			if(strlen(commandline) > 0){
				int tokend;
				struct mol *mol;

				if(strchr(commandline, ' ') != NULL)
					tokend = strchr(commandline, ' ') - commandline;
				else
					tokend = strlen(commandline);

				strncpy(lpofn->lpstrFileTitle, commandline, tokend);
				lpofn->lpstrFileTitle[tokend] = 0;

				if((mol = readmolfile(lpofn->lpstrFileTitle)) == NULL)
					MessageBox(hwnd, "File does not exist", appname, MB_OK);
				else {
					sprintf(windowname, "%s (%s)", WINDOWNAME, lpofn->lpstrFileTitle);
					SetWindowText(hwnd, windowname);
					SendMessage(hmoleditwnd, MEM_SETMOL, 0, (long)mol);
					InvalidateRect(hwnd, NULL, TRUE);
					destroy_mol(mol);
				}

			}

	
			}
			break;

		case WM_CLOSE:
			if(1 || (MessageBox(hwnd, "Are you sure you want to quit?", appname, MB_YESNO)== IDYES))
				DestroyWindow(hwnd);
			break;

		case WM_PAINT:
			if (GetUpdateRect(hwnd,NULL,FALSE)) {
				PAINTSTRUCT ps;
				HDC hdc;

				hdc = BeginPaint(hwnd, &ps);
				UpdateWindow(hmoleditwnd);
				EndPaint(hwnd, &ps);
			}
			break;


		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		case WM_KEYDOWN:
			return(DefWindowProc(hwnd, message, wParam, lParam));
			break;

		case WM_MENUSELECT:
			MenuHelp(WM_MENUSELECT, wParam, lParam, GetMenu(hwnd), hInst, hstatus, arr);
			break;

		case WM_SIZE:{
			int cx, cy;
			RECT r, r2;

			cx = GetSystemMetrics(SM_CXFRAME);
			cy = GetSystemMetrics(SM_CYFRAME);

			GetClientRect(hwnd, &r);
			SendMessage(htoolbar, TB_AUTOSIZE, 0, 0);
			GetWindowRect(htoolbar, &r2);
			r.top = r2.bottom - r2.top;

			GetWindowRect(hstatus, &r2);
			r.bottom -= r2.bottom - r2.top;
			
			MoveWindow(hstatus, 0, r.bottom, r.right, r2.bottom - r2.top, TRUE);
			
			MoveWindow(hmoleditwnd,0,r.top, r.right, r.bottom - r.top, TRUE);

			MoveWindow(hmoldispwnd,0,r.top, r.right * vert_division / 100, (r.bottom - r.top) * horiz_division / 100, TRUE);
			MoveWindow(hmolanalysiswnd,0,(r.bottom - r.top) * horiz_division / 100 + r.top, r.right * vert_division / 100, (r.bottom - r.top) * (100-horiz_division) / 100, TRUE);
			MoveWindow(hmolrxninfownd, r.right * vert_division / 100, r.top,
				 r.right - r.right * vert_division / 100 + cy, r.bottom - r.top + cx, TRUE);
			}
			break;



		case WM_NOTIFY:
			{
				NMHDR *nmhdr = (NMHDR*)lParam;

				if((nmhdr->hwndFrom == htooltips) && (nmhdr->code == TTN_NEEDTEXT))	{
					TOOLTIPTEXT *ttt = (TOOLTIPTEXT*)nmhdr;
					ttt->lpszText = (LPSTR)nmhdr->idFrom;
					ttt->hinst = hInst;
				
				}else if((nmhdr->hwndFrom == hmolrxninfownd) && (nmhdr->code == MN_SIZENOTIFY)) {
					int x;
					RECT r;
					struct size_notification *sn = (void*)nmhdr;

					x = sn->x;// - GetSystemMetrics(SM_CYFRAME);
				
					GetClientRect(hwnd, &r);
				
					vert_division = 100 - 100 * x / (r.right);
					SendMessage(hwnd, WM_SIZE, SIZE_RESTORED, r.bottom << 16 | r.right);
				} else if((nmhdr->hwndFrom == hactive) && (nmhdr->code == MN_SETSTATUSTEXT)) {
					SendMessage(hstatus, WM_SETTEXT, 0, (long)((struct string_notification*)nmhdr)->s);
				}
			}
			break;
		
		case WM_INITMENU:{
			HMENU hmenu = GetMenu(hwnd);
			if(SendMessage(hmoleditwnd, MEM_GETMOLCOUNT, 0, 0)){
				setmenuitems(hmenu, MFS_GRAYED, OP_REMOVE, IDMOL_FILE_CLOSE,
					 IDMOL_SHOWATOMTYPES, IDMOL_SHOWERRORS, IDMOL_ANALYZE,
					 IDMOL_COMPLEXITY, IDMOL_SHOWFG, IDMOL_SHOWEQ, IDMOL_SMILES,
					 IDMOL_EDIT_COPY, IDMOL_EDIT_CUT, IDMOL_SHOWRINGS, 0);
			}
			else {
				setmenuitems(hmenu, MFS_GRAYED, OP_SET, IDMOL_FILE_CLOSE, 
					IDMOL_SHOWATOMTYPES, IDMOL_SHOWERRORS, IDMOL_ANALYZE, 
					IDMOL_COMPLEXITY, IDMOL_SHOWFG, IDMOL_SHOWEQ, IDMOL_SMILES,
					IDMOL_EDIT_COPY, IDMOL_EDIT_CUT, IDMOL_SHOWRINGS, 0);
			}
			if(mode == MODE_EDIT)
				setmenuitems(hmenu, MFS_GRAYED, OP_SET, IDMOL_EDITMODE, 0);
			else
				setmenuitems(hmenu, MFS_GRAYED, OP_REMOVE, IDMOL_EDITMODE, 0);
			
			}break;

		case WM_SETFOCUS:
			if(mode == MODE_EDIT)
				SetFocus(hactive);
			break;

		case WM_COMMAND:
			switch(LOWORD(wParam)) {

				case ID_BUTTONOPEN:
				case IDMOL_FILE_OPEN:
					lpofn->lpstrTitle = "Open molecule";
					if(GetOpenFileName(lpofn)) {
						struct mol *mol;

						if((mol = readmolfile(lpofn->lpstrFile)) == NULL)
							MessageBox(hwnd, "Error reading molfile", appname, MB_OK);
						else {
							sprintf(windowname, "%s (%s)", WINDOWNAME, lpofn->lpstrFileTitle);
							SetWindowText(hwnd, windowname);
							InvalidateRect(hwnd, NULL, TRUE);
							SendMessage(hmoleditwnd, MEM_SETMOL, 0, (long)mol);
							destroy_mol(mol);
						}
					}
					break;

				case ID_BUTTONSAVE:
				case IDMOL_FILE_SAVE:
					if(lpofn->lpstrFileTitle[0]){
						struct mol *mol = (struct mol *)SendMessage(hmoleditwnd, MEM_GETMOL, 0, 0);
						writemolfile(lpofn->lpstrFile, mol);
						destroy_mol(mol);
						break;
					}

				case IDMOL_FILE_SAVEAS:
					lpofn->lpstrTitle = "Save as";
					if(GetSaveFileName(lpofn)) {
						struct mol *mol;

						sprintf(windowname, "%s (%s)", WINDOWNAME, lpofn->lpstrFileTitle);
						SetWindowText(hwnd, windowname);
						mol = (struct mol *)SendMessage(hmoleditwnd, MEM_GETMOL, 0, 0);
						writemolfile(lpofn->lpstrFile, mol);
						destroy_mol(mol);
					}
					break;
					
				case IDMOL_FILE_CLOSE:
						SendMessage(hmoleditwnd, MEM_RESETMOL, 0, 0);
						SetWindowText(hwnd, WINDOWNAME);
					break;
					
				case IDMOL_FILE_EXIT:
					SendMessage(hwnd, WM_CLOSE, 0, 0);
					break;
				
				case IDMOL_EDITMODE:
					mode = MODE_EDIT;
					hactive = hmoleditwnd;
					SetFocus(hactive);
					ShowWindow(hmoleditwnd, SW_SHOWNORMAL);
					ShowWindow(hmoldispwnd, SW_HIDE);
					ShowWindow(hmolanalysiswnd, SW_HIDE);
					ShowWindow(hmolrxninfownd, SW_HIDE);
					SendMessage(htoolbar, TB_HIDEBUTTON, ID_BUTTONANALYZEMODE, FALSE);
					SendMessage(htoolbar, TB_HIDEBUTTON, ID_BUTTONANALYZETOOL, TRUE);
					SendMessage(htoolbar, TB_HIDEBUTTON, ID_BUTTONHETEROATOM, FALSE);
					SendMessage(htoolbar, TB_HIDEBUTTON, ID_BUTTONBOND, FALSE);
					SendMessage(htoolbar, TB_HIDEBUTTON, ID_BUTTONERASE, FALSE);
					SendMessage(htoolbar, TB_HIDEBUTTON, ID_BUTTONCUT, FALSE);
					SendMessage(htoolbar, TB_HIDEBUTTON, ID_BUTTONPASTE, FALSE);
					SendMessage(htoolbar, TB_HIDEBUTTON, IDMOL_ABOUT, FALSE);
					SendMessage(htoolbar, TB_HIDEBUTTON, ID_BUTTONUP, TRUE);
					break;

				case IDMOL_EDIT_COPY:
				case ID_BUTTONCOPY:	
					SendMessage(hactive, MEM_COPY, 0, 0);
					break;

				case IDMOL_EDIT_PASTE:
				case ID_BUTTONPASTE:
					SendMessage(hactive, MEM_PASTE, 0, 0);
					break;

				case IDMOL_EDIT_CUT:
				case ID_BUTTONCUT:
					SendMessage(hactive, MEM_CUT, 0, 0);
					break;
					
				case IDMOL_EDIT_DELETE:
					SendMessage(hactive, MEM_DELETE_SELECTION, 0, 0);
					break;
					
				case IDMOL_EDIT_UNDO:
				case ID_BUTTONUNDO:
					SendMessage(hactive, MEM_UNDO, 0, 0);
					break;

				case IDMOL_EDIT_REDO:
				case ID_BUTTONREDO:
					SendMessage(hactive, MEM_REDO, 0, 0);
					break;

				case IDMOL_TOGGLENUMBERS:
					if(SendMessage(htoolbar, TB_ISBUTTONCHECKED, ID_BUTTONNUM, 0))
						SendMessage(htoolbar, TB_CHECKBUTTON, ID_BUTTONNUM, FALSE);
					else
						SendMessage(htoolbar, TB_CHECKBUTTON, ID_BUTTONNUM, TRUE);

					// fall through...
				case ID_BUTTONNUM:
					setmenuitems(GetMenu(hwnd), MFS_CHECKED, OP_TOGGLE, IDMOL_TOGGLENUMBERS, 0);
					SendMessage(hactive, MEM_TOGGLENUMBERS, 0, 0);
					break;

				case IDMOL_SHOWERRORS:{
					struct chem_err *err = _init_err();
					struct mol *mol = getmol();
					
					classify_atoms(mol, err);
					if(err->num_err == 0)
						strcpy(err->error_log, "No errors.");
					MessageBox(hwnd, err->error_log, "Error log", MB_OK);
					destroy_mol(mol);
					_destroy_err(err);
					}break;

				case IDMOL_SHOWATOMTYPES:{
					char *atom_types;
					struct mol *mol = getmol();

					find_rings(mol);
					classify_atoms(mol, NULL);
					atom_types = list_atomtypes(mol);
					MessageBox(hwnd, atom_types, "Atom types", MB_OK); 
					free(atom_types);
					destroy_mol(mol);
					}break;

				case IDMOL_SHOWRINGS:{
					char *s;
					struct mol *mol = getmol();
					
					find_rings(mol);
					s = list_rings(comp_info, mol);
					MessageBox(hwnd, s, appname, MB_OK); 
					free(s);
					destroy_mol(mol);
					}break;

				case IDMOL_SHOWFG:{
					struct chem_err *err = _init_err();
					struct mol *mol = getmol();

					classify_atoms(mol, err);
					if(err->num_err > 0)
						MessageBox(hwnd, "Molecule has errors", appname, MB_OK);
					else {
						char *buf;

						busca_funcionales(comp_info, mol);
						buf = list_fgroups(comp_info, mol);
						MessageBox(hwnd, buf, "Functional Groups", MB_OK); 
						free(buf);
					}
					_destroy_err(err);
					}break;

				case IDMOL_SHOWEQ: {
					struct mol *mol = getmol();
					char *s;

					canonicalize(mol);
					s = list_eq_classes(mol);
					MessageBox(hwnd, s, "Equivalence classes", MB_OK); 
					free(s);
					destroy_mol(mol);
					}break;

				case IDMOL_COMPLEXITY:{
					struct chem_err *err = _init_err();
					struct mol *mol = getmol();
					
					classify_atoms(mol, err);
					if(err->num_err > 0)
						MessageBox(hwnd, "Molecule has errors", appname, MB_OK);
					else {
						int complexity;
						char s[30];

						busca_funcionales(comp_info, mol);
						complexity = mol_complexity(mol, comp_info);
						sprintf(s, "Complexity = %i", complexity);
						MessageBox(hwnd, s, "Complexity", MB_OK);
					}
					_destroy_err(err);
					destroy_mol(mol);
					}break;

				case IDMOL_SMILES: {
					struct mol *mol = getmol();
					char *s;

					canonicalize(mol);
					s = mol2smiles(mol);
					MessageBox(hwnd, s, "SMILES", MB_OK); 
					free(s);
					destroy_mol(mol);
					}break;


				case ID_BUTTONANALYZEMODE:
				case IDMOL_ANALYZE:
					SendMessage(hmolanalysiswnd, MAM_SETROOT, 0, (long)getmol());
					ShowWindow(hmoleditwnd, SW_HIDE);
					ShowWindow(hmoldispwnd, SW_SHOWNORMAL);
					ShowWindow(hmolanalysiswnd, SW_SHOWNORMAL);
					ShowWindow(hmolrxninfownd, SW_SHOWNORMAL);
					mode = MODE_ANALYSIS;
					SendMessage(htoolbar, TB_HIDEBUTTON, ID_BUTTONANALYZEMODE, TRUE);
					SendMessage(htoolbar, TB_HIDEBUTTON, ID_BUTTONANALYZETOOL, FALSE);
					SendMessage(htoolbar, TB_HIDEBUTTON, ID_BUTTONHETEROATOM, TRUE);
					SendMessage(htoolbar, TB_HIDEBUTTON, ID_BUTTONBOND, TRUE);
					SendMessage(htoolbar, TB_HIDEBUTTON, ID_BUTTONERASE, TRUE);
					SendMessage(htoolbar, TB_HIDEBUTTON, ID_BUTTONCUT, TRUE);
					SendMessage(htoolbar, TB_HIDEBUTTON, ID_BUTTONPASTE, TRUE);
					SendMessage(htoolbar, TB_HIDEBUTTON, IDMOL_ABOUT, TRUE);
					SendMessage(htoolbar, TB_HIDEBUTTON, ID_BUTTONUP, FALSE);
					hactive = hmolanalysiswnd;
					SetFocus(hactive);
					break;

				case IDMOL_ABOUT:
					DialogBox(hInst, "Mydialog", hwnd, DialogProc);
					break;

				case IDMOL_LICENSE:
					CreateWindow("TextWindow", "GNU General Public License", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
						CW_USEDEFAULT, CW_USEDEFAULT, MAXX, MAXY,
						hwnd, NULL, hInst, NULL);
					break;

				case IDMOL_HOMEPAGE:
					WinExec("start http://litio.pquim.unam.mx/caos/index.html", SW_HIDE);
					break;
				
				case ID_BUTTONERASE:
				case ID_BUTTONSELECT:
				case ID_BUTTONHETEROATOM:
				case ID_BUTTONBOND:
				case ID_BUTTONANALYZETOOL:
					SendMessage(hactive, message, wParam, lParam);
					break;
	
				case ID_BUTTONUP:
					SendMessage(hactive, MAM_UP, 0, 0);
					break;

				case ID_BUTTONNEXT:
					SendMessage(hmoldispwnd, MDM_NEXT, 0, 0);
					SendMessage(hmolrxninfownd, MDM_REFRESHTEXT, 0, 0);
					InvalidateRect(hmoldispwnd, NULL, TRUE);
					break;

				case ID_BUTTONPREV:
					SendMessage(hmoldispwnd, MDM_PREV, 0, 0);
					SendMessage(hmolrxninfownd, MDM_REFRESHTEXT, 0, 0);
					InvalidateRect(hmoldispwnd, NULL, TRUE);
					break;

	
	
			}
			break;
		default:                  /* Passes it on if unproccessed    */
			return (DefWindowProc(hwnd, message, wParam, lParam));
	}

	return (0);
}


HWND init_toolbar(TBBUTTON **buttons, HWND hwnd, HINSTANCE hInst )
{
	HWND htoolbar;
	TBADDBITMAP tbab;
	int i = -1, firstbitmap, secondbitmap;

	*buttons = calloc(40, sizeof(TBBUTTON));
	assert(*buttons);

	htoolbar = CreateWindow( TOOLBARCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | 
					WS_CLIPSIBLINGS | CCS_TOP | TBSTYLE_TOOLTIPS | TBSTYLE_WRAPABLE,
					0, 0, 0, 0, hwnd, (HMENU)ID_MYTOOLBAR, hInst, 0);

	SendMessage(htoolbar, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);

	tbab.hInst = HINST_COMMCTRL;
	tbab.nID = IDB_STD_SMALL_COLOR;
	
	assert((firstbitmap = SendMessage(htoolbar, TB_ADDBITMAP, 0, (long)&tbab)) != -1);

	tbab.hInst = hInst;
	tbab.nID = ID_MYTOOLBAR;

	assert((firstbitmap = SendMessage(htoolbar, TB_ADDBITMAP, N_BUTTONBITMAPS, (long)&tbab)) != -1);

	tbab.hInst = HINST_COMMCTRL;
	tbab.nID = IDB_VIEW_SMALL_COLOR;

	assert((secondbitmap = SendMessage(htoolbar, TB_ADDBITMAP, 0, (long)&tbab)) != -1);

	(*buttons)[++i].iBitmap = STD_FILEOPEN;
	(*buttons)[i].idCommand = ID_BUTTONOPEN;;
	(*buttons)[i].fsState = TBSTATE_ENABLED;
	(*buttons)[i].fsStyle = TBSTYLE_BUTTON;

	(*buttons)[++i].iBitmap = STD_FILESAVE;
	(*buttons)[i].idCommand = ID_BUTTONSAVE;
	(*buttons)[i].fsState = TBSTATE_ENABLED;
	(*buttons)[i].fsStyle = TBSTYLE_BUTTON;

	(*buttons)[++i].fsStyle = TBSTYLE_SEP;

	(*buttons)[++i].iBitmap = STD_CUT;
	(*buttons)[i].idCommand = ID_BUTTONCUT;
	(*buttons)[i].fsState = TBSTATE_ENABLED;
	(*buttons)[i].fsStyle = TBSTYLE_BUTTON;

	(*buttons)[++i].iBitmap = STD_COPY;
	(*buttons)[i].idCommand = ID_BUTTONCOPY;
	(*buttons)[i].fsState = TBSTATE_ENABLED;
	(*buttons)[i].fsStyle = TBSTYLE_BUTTON;

	(*buttons)[++i].iBitmap = STD_PASTE;
	(*buttons)[i].idCommand = ID_BUTTONPASTE;
	(*buttons)[i].fsState = TBSTATE_ENABLED;
	(*buttons)[i].fsStyle = TBSTYLE_BUTTON;

	(*buttons)[++i].fsStyle = TBSTYLE_SEP;

	(*buttons)[++i].iBitmap = STD_UNDO;
	(*buttons)[i].idCommand = ID_BUTTONUNDO;
	(*buttons)[i].fsState = TBSTATE_ENABLED;
	(*buttons)[i].fsStyle = TBSTYLE_BUTTON;
	
	(*buttons)[++i].iBitmap = STD_REDOW;
	(*buttons)[i].idCommand = ID_BUTTONREDO;
	(*buttons)[i].fsState = TBSTATE_ENABLED;
	(*buttons)[i].fsStyle = TBSTYLE_BUTTON;

	(*buttons)[++i].fsStyle = TBSTYLE_SEP;

	(*buttons)[++i].iBitmap = 0 + firstbitmap;
	(*buttons)[i].idCommand = ID_BUTTONSELECT;
	(*buttons)[i].fsState = TBSTATE_ENABLED | TBSTATE_CHECKED;
	(*buttons)[i].fsStyle = TBSTYLE_CHECKGROUP;

	(*buttons)[++i].iBitmap = 1 + firstbitmap;
	(*buttons)[i].idCommand = ID_BUTTONERASE;
	(*buttons)[i].fsState = TBSTATE_ENABLED;
	(*buttons)[i].fsStyle = TBSTYLE_CHECKGROUP;

	(*buttons)[++i].iBitmap = 2 + firstbitmap;
	(*buttons)[i].idCommand = ID_BUTTONBOND;
	(*buttons)[i].fsState = TBSTATE_ENABLED;
	(*buttons)[i].fsStyle = TBSTYLE_CHECKGROUP;

	(*buttons)[++i].iBitmap = 3 + firstbitmap;
	(*buttons)[i].idCommand = ID_BUTTONHETEROATOM;
	(*buttons)[i].fsState = TBSTATE_ENABLED;
	(*buttons)[i].fsStyle = TBSTYLE_CHECKGROUP;

	(*buttons)[++i].iBitmap = 8 + firstbitmap;
	(*buttons)[i].idCommand = ID_BUTTONANALYZETOOL;
	(*buttons)[i].fsState = TBSTATE_ENABLED | TBSTATE_HIDDEN;
	(*buttons)[i].fsStyle = TBSTYLE_CHECKGROUP;

	(*buttons)[++i].iBitmap = 4 + firstbitmap;
//	(*buttons)[++i].iBitmap = secondbitmap + VIEW_PARENTFOLDER;
	(*buttons)[i].idCommand = IDMOL_ABOUT;
	(*buttons)[i].fsState = TBSTATE_ENABLED;
	(*buttons)[i].fsStyle = TBSTYLE_CHECKGROUP;

	(*buttons)[++i].fsStyle = TBSTYLE_SEP;

	(*buttons)[++i].iBitmap = 5 + firstbitmap;
	(*buttons)[i].idCommand = ID_BUTTONNUM;
	(*buttons)[i].fsState = TBSTATE_ENABLED | TBSTATE_CHECKED;
	(*buttons)[i].fsStyle = TBSTYLE_CHECK ;

	(*buttons)[++i].fsStyle = TBSTYLE_SEP;
	
/*	(*buttons)[++i].iBitmap = 7 + firstbitmap;
	(*buttons)[i].idCommand = ID_BUTTONPREV;
	(*buttons)[i].fsState = TBSTATE_ENABLED | TBSTATE_HIDDEN;
	(*buttons)[i].fsStyle = TBSTYLE_BUTTON;

	(*buttons)[++i].iBitmap = 6 + firstbitmap;
	(*buttons)[i].idCommand = ID_BUTTONNEXT;
	(*buttons)[i].fsState = TBSTATE_ENABLED | TBSTATE_HIDDEN;
	(*buttons)[i].fsStyle = TBSTYLE_BUTTON;

	(*buttons)[++i].fsStyle = TBSTYLE_SEP;
*/	
	(*buttons)[++i].iBitmap = 8 + firstbitmap;
	(*buttons)[i].idCommand = ID_BUTTONANALYZEMODE;
	(*buttons)[i].fsState = TBSTATE_ENABLED;
	(*buttons)[i].fsStyle = TBSTYLE_BUTTON;

	(*buttons)[++i].iBitmap = secondbitmap + VIEW_PARENTFOLDER;
	(*buttons)[i].idCommand = ID_BUTTONUP;
	(*buttons)[i].fsState = TBSTATE_ENABLED | TBSTATE_HIDDEN;
	(*buttons)[i].fsStyle = TBSTYLE_BUTTON;

	*buttons = realloc(*buttons, (i+1)*sizeof(TBBUTTON));
	assert(buttons);

	SendMessage(htoolbar, TB_ADDBUTTONS, i+1, (long)*buttons);
//	htoolbar = CreateToolbarEx(hwnd, WS_CHILD | WS_VISIBLE | TBSTYLE_TOOLTIPS, ID_MYTOOLBAR, N_BUTTONBITMAPS, hInst, ID_MYTOOLBAR,
//					 *buttons, N_BUTTONS, 16, 16, 16, 16, sizeof(TBBUTTON));
	


	return(htoolbar);	
}




