/* _sk2.h
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

/*	This file contains functions for the creation of ACD/ChemSketch documents (*.sk2)
	More information at http://www.acdlabs.com    */

#ifndef sk2_h
#define sk2_h

#include <assert.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/*************************************************************************
			Fundamental data types used by .sk2 files
**************************************************************************/

typedef unsigned char Byte;
typedef short Integer;
typedef unsigned short Word;
typedef int LongInt;
typedef float Single;
typedef char ShortInt;
typedef unsigned char Char;
typedef char Boolean;
#ifndef BOOL
	#define BOOL int
#endif
#ifndef TRUE
	#define TRUE 1
#endif
#ifndef FALSE
	#define FALSE 0
#endif

//******************* Structs

typedef struct {
	Byte Red; 
	Byte Green; 
	Byte Blue; 
	Byte Reserved;	// should be zero
} TColorRef;

typedef struct {
	Integer X;
	Integer Y;
} TPoint;

typedef struct {
	Integer left;
	Integer top;
	Integer right;
	Integer bottom;
} TRect;

typedef struct {
	Single X;
	Single Y;
} TRPoint;

typedef struct {
	Single left;
	Single top;
	Single right;
	Single bottom;
} TRRect;


typedef struct {
	Word length;
	char content[0];
} PChar;

typedef struct {
	Byte length;
	char content[0];
} String;


/*************************************************************************
							Document structures
**************************************************************************/

#define SIZEOF_EnvironmentDescriptionFormat 8
typedef struct {
	Integer ActivePage;
	Integer Zoom;
	TPoint ViewOrigin;
} EnvironmentDescriptionFormat;

#define SIZEOF_PageInfoFormat 30
typedef struct {
	Integer Reserved[2];
	TPoint PageSize;
	TPoint PrnPageSize;
	TRPoint PrnPages;
	TRect Border;
	Integer Orientation;	// PORTRAIT or LANDSCAPE
} PageInfoFormat;

#define PORTRAIT	1
#define LANDSCAPE	2


/*************************************************************************
								Styles
**************************************************************************/

#define DEFAULT_STYLE -1   // use it in calls to createatom, createbond, etc. to use a default style
struct style {
	Byte type;
	void *content;
};

// Style types. This list is incomplete but it includes only types that are currently implemented
#define PENSTYLE	1
#define FONTSTYLE	3
#define ARROWSTYLE	4
#define ATOMSTYLE	129
#define BONDSTYLE	130
#define INTERSECTIONSTYLE 131

#define SIZEOF_PenStyleFormat 8
typedef struct {
	Byte Version;		// Should be zero
	Integer PenWidth;	// Width in logical units
	Byte PenStyle;		// One of the PS_ styles below
	TColorRef PenColor;
} PenStyleFormat;

// Pen styles
#define PENSTYLE_NULL			0
#define PENSTYLE_SOLID			1
#define PENSTYLE_DASH			2
#define PENSTYLE_DOT			3
#define PENSTYLE_DASHDOT		4
#define PENSTYLE_DASHDOTDOT		5


typedef struct {
	Byte Version;		// Should be zero
	Integer Size;		// Font size in half-points
	Word FontStyle;		// A combination of the FS_ styles below
	TColorRef Color;
	char *Name;			// Font name (For example, "Arial");
	Integer DPI;
} FontStyleFormat;

// Font Styles
#define FS_NORMAL		0x0000
#define FS_BOLD			0x0001
#define FS_ITALIC		0x0004
#define FS_UNDERLINE	0x0010
#define FS_STRIKEOUT	0x0040
#define FS_SUPERSCRIPT	0x0100
#define FS_SUBSCRIPT	0x0200

typedef struct {
	Byte Version;		// should be zero
	Integer DPI;
	Byte Heads[4];		// arrowhead styles for each of the heads of two arrow lines
	Integer Double;		// Distance in points between two lines of a two-line
						// arrow. If Double = 0, the arrow consists of one line.
	TPoint Size;		// Arrow size in points (1 point = 1/72 in)
} ArrowStyleFormat;

#define AS_NULL		0x0
#define AS_UP		0x1
#define AS_UPDOWN	0x2
#define AS_DOWN		0x3
#define AS_DOUBLE	0x4
#define AS_FILLED	0x10


typedef struct {
	Byte Version;				// Should be zero
	Byte NewProperties;			// Should be zero
	struct style BaseFont;
	struct style HydrogenFont;
	struct style IndexFont;
	struct style ValenceFont;
	struct style ChargeFont;
	struct style IsotopeFont;
	struct style NumberingFont;
	Integer IndexOffset;		// Vertical, in logical units
	Integer ValenceOffset;		// Vertical, in logical units
	Integer IsotopeOffset;		// Vertical, in logical units
	Integer ChargeOffset;		// Vertical, in logical units
	TPoint NumberingOffset;		// Horizontal, vertical, in logical units
	Boolean ValenceAfterHydrogen;
	Boolean ChargeAfterHydrogen;
	Boolean ShowCarbons;
	Boolean ShowTerminalCarbons;
	Boolean ShowHydrogens;
	Boolean ShowValence;
	Boolean ShowZeroCharge;
	Boolean ShowIsotope;
	Boolean ShowNumbering;
	Boolean CrossOutInvalidAtom;
} AtomStyleFormat;


typedef struct {
	Byte Version;			// Should be zero
	Byte NewProperties;		// Should be zero
	struct style SinglePen;
	struct style DoublePen;
	struct style TriplePen;
	struct style UpStereoPen;
	struct style DownStereoPen;
	struct style CoordinatingPen;
	struct style UndefinedPen;
	Integer DoubleBetween;		// In logical units
	Integer DoubleShift;		// In logical units
	Integer TripleBetween;		// In logical units
	Integer TripleShift;		// In logical units
	Integer UpStereoWidth;		// In logical units
	Integer DownStereoWidth;	// In logical units
	Integer DownStereoStep;		// In logical units
	TPoint CoordHeadSize;		// Length, width; in logical units
	Integer UndefinedWidth;		// In logical units
	Integer UndefinedStep;		// In logical units
} BondStyleFormat;

#define SIZEOF_IntersectionStyleFormat 5
typedef struct {
	Byte Version;				// Should be zero
	Byte NewProperties;			// Should be zero
	Boolean IntersectionsEnable;
	Integer WhiteSpace;
} IntersectionStyleFormat;


/*************************************************************************
								Objects
**************************************************************************/


typedef struct object {
	Byte type;
	void *content;
} Object;

typedef struct object_list {
	int Nobj;
	struct object *objs;	
} ObjectList;

// Object types
#define LINE		1
#define POLYLINE	2
#define GROUP		32
#define MOLECULE	253

typedef struct {
	Byte NodeType;
	TRPoint Pos;
	// PrevV, NextV
} GeomNodeFormat;

typedef struct {
	Integer NodesCount;		// the number of nodes
	Boolean IsShapeClosed;	// determines whether the shape is closed, i.e.
							// whether Node[0] and Node[N-1] are connected.
	GeomNodeFormat **Nodes;
} GeomShapeFormat;


typedef struct {
	Byte Version;				// should be zero for now
	Integer PenStyleIndex;		// index of pen style in the list of graphical styles of the picture
	Integer BrushStyleIndex;	// should be -1
	Integer ArrowStyleIndex;	// index of arrow style in the list of graphical styles of the picture
	GeomShapeFormat *GeomShape;
} LineFormat;

typedef struct {
	Byte Version;			// Should be zero for now
	ObjectList *ObjectsList;
} GroupFormat;

typedef struct {
	Single AutoScale;		// default = -1
	Byte NewProperties;		// should be zero for now
	Integer Style;			// index in StylesList of Picture
	Integer ElementNum;		// atomic number - 1 (e.g. hydrogen = 0)
	ShortInt Valence;		// >= 0
	ShortInt Charge;
	Integer Isotope;
	char *NumStr;
	Single X;				// in logical units
	Single Y;				// in logical units
	Single Z;				// in logical units
	Byte HydrogenPosition;  // see HPOS_ constants below
	Byte PersonalShowState;	// see PSHOWSTATES_ below
	Byte FixStates;			// a combination of FIXSTATES_ below
	Byte ShowStates;		// a combination of SHOWSTATES_ below
	Boolean SubStructureExist;	// should be false for now
	Word AtomLabel;				// should be zero for now
} AtomDataFormat;

#define HPOS_AUTO	0
#define HPOS_RIGHT	1
#define HPOS_LEFT	2
#define HPOS_BOTTOM	3
#define HPOS_TOP	4

#define PSHOWSTATE_SHOW 0
#define PSHOWSTATE_HIDE 1
#define PSHOWSTATE_UNDEFINED 2

#define FIXSTATES_VALENCEFIXED 1
#define FIXSTATES_CHARGEFIXED 2
#define FIXSTATES_CHARGEFIXEDSOFT 4
// Note: 2 and 4 cannot be used simultaneously

#define SHOWSTATES_SHOW_HYDROGENS 1
#define SHOWSTATES_SHOW_VALENCE 2
#define SHOWSTATES_SHOW_ZERO_CHARGE 4
#define SHOWSTATES_SHOW_ISOTOPE 8
#define SHOWSTATES_SHOW_NUMBERING 16
#define SHOWSTATES_CROSSOUT_INVALID_ATOM 32

#define SIZEOF_BondDataFormat 24
typedef struct {
	Single AutoScale;		// default = -1
	Byte NewProperties;		// should be zero for now
	Integer Style;			// index in StylesList of Picture
	Integer IndexOfFirstAtom;
	Integer IndexOfSecondAtom;
	Byte BondType;			// see BONDTYPE_ below
	TPoint Reserved[2];		// should be zeroes
	Byte StereoView;		// 0 or 1 ?
	Byte SymmetryOfDoubleBond;  // 0 = auto
	Byte Reservedb;			// should be zero
	Byte RingState;			
} BondDataFormat;

#define BONDTYPE_SINGLE 1
#define BONDTYPE_DOUBLE 2
#define BONDTYPE_TRIPLE 3
//... there are more bond types but we haven't implemented them yet


typedef struct {
	Byte Version;	   				// Should be zero
	Byte NewProperties;				// Should be zero
	Integer AtomsCount;
	BondDataFormat **Bonds;
	Integer BondsCount;
	AtomDataFormat **Atoms;
	Integer NumberofRadicalAtom;	// For independent structures should be < 0
	Integer AromaRingsCount;		// For the moment should be zero
//	AromaRingDataFormat **AromaRings;
} MoleculeFormat;



//**************  Main document information
struct sk2info {
	FILE *fp;
	Integer Nstyle;
	struct style *styles;
	ObjectList *obj_list;
	PageInfoFormat pageinfo;
	char *filename;
	int defatomstyle;
	int defbondstyle;
	int defpenstyle;
	int defarrowstyle;
};


/*************************************************************************
								Functions
**************************************************************************/


// _sk2.c functions
struct sk2info *create_sk2(char *fname);
int write_sk2(struct sk2info *info);
void setpageinfo_sk2(struct sk2info *info,
			int pagesize_x, int pagesize_y,
			int printablepagesize_x, int printablepagesize_y,
			double prnpages_x, double prnpages_y,
			int margin_left, int margin_top, int margin_right, int margin_bottom,
			int orientation);

// _sk2style.c functions
AtomStyleFormat *CreateDefAtomStyle();
BondStyleFormat *CreateDefBondStyle();
IntersectionStyleFormat *CreateDefIntersectionStyle();
ArrowStyleFormat *CreateDefArrowStyle();
PenStyleFormat *CreateDefPenStyle();
int addstyle(struct sk2info *info, void *style, int type);
int fputStyle(FILE *fp, struct style *style);
ArrowStyleFormat *CreateArrowStyle(int h1, int h2, int h3, int h4, int Double, int x, int y);
void *DestroyStyle(struct style *style);

// _sk2obj.c
int fputObject(FILE *fp, struct object *obj);
int addobj_sk2(struct object_list *list, void *obj, int type);
MoleculeFormat *CreateMolecule();
AtomDataFormat *CreateAtom(struct sk2info *info, Integer ElementNum, Single X, Single Y, Single Z, ShortInt Charge, Integer Style);
BondDataFormat *CreateBond(struct sk2info *info, Integer a1, Integer a2, Byte Type, Integer Style);
int AddAtom(MoleculeFormat *mol, AtomDataFormat *atom);
int AddBond(MoleculeFormat *mol, BondDataFormat *bond);
void SetMolRect(MoleculeFormat *mol, TRRect r, Single margin, BOOL change_aspect);
LineFormat *CreateLine(struct sk2info *info, int Pen, int Arrow);
LineFormat *CreateLineEx(struct sk2info *info, int Pen, int Arrow, int Npoint, double x, ...);
int AddNode(GeomShapeFormat *g, GeomNodeFormat *node);
GeomNodeFormat *CreateNode(Single X, Single Y);
ObjectList *CreateObjList();
GroupFormat *CreateGroup();
void *DestroyObject(struct object *obj);

// _sk2aux.c functions
int fputPChar(FILE *fp, char *s);
int fputString(FILE *fp, char *s);
int fputLongInt(FILE *fp, LongInt i);
int fputInteger(FILE *fp, Integer i);
int fputWord(FILE *fp, Word i);
int fputByte(FILE *fp, Byte i);
int fputShortInt(FILE *fp, ShortInt i);
int fputBoolean(FILE *fp, Boolean b);
int fputSingle(FILE *fp, Single f);
int fputTColorRef(FILE *fp, TColorRef color);
int fputTPoint(FILE *fp, TPoint point);
int fputTRPoint(FILE *fp, TRPoint point);
int fseekputLongInt(FILE *fp, LongInt i, int pos);
int fseekputInteger(FILE *fp, Integer i, int pos);
TColorRef int2color(int color);

#endif


