/* _sk2.c 
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

#include "sk2.h"



int fputStyle(FILE *fp, struct style *style)
{
	int sizepos, beginpos, endpos, size;

	fputByte(fp, style->type);  
	fputByte(fp, 0);			
	sizepos = ftell(fp);
	fputLongInt(fp, 0);
	beginpos = ftell(fp);

	switch(style->type){
		case ATOMSTYLE: {
			AtomStyleFormat *atom = (AtomStyleFormat *)style->content;
			fputByte(fp, atom->Version);
			fputByte(fp, atom->NewProperties);

			fputStyle(fp, &atom->BaseFont);
			fputStyle(fp, &atom->HydrogenFont);
			fputStyle(fp, &atom->IndexFont);
			fputStyle(fp, &atom->ValenceFont);
			fputStyle(fp, &atom->ChargeFont);
			fputStyle(fp, &atom->IsotopeFont);
			fputStyle(fp, &atom->NumberingFont);

			fputInteger(fp, atom->IndexOffset);
			fputInteger(fp, atom->ValenceOffset);
			fputInteger(fp, atom->IsotopeOffset);
			fputInteger(fp, atom->ChargeOffset);
			fputTPoint(fp, atom->NumberingOffset);
			fputBoolean(fp, atom->ValenceAfterHydrogen);
			fputBoolean(fp, atom->ChargeAfterHydrogen);
			fputBoolean(fp, atom->ShowCarbons);
			fputBoolean(fp, atom->ShowTerminalCarbons);
			fputBoolean(fp, atom->ShowHydrogens);
			fputBoolean(fp, atom->ShowValence);
			fputBoolean(fp, atom->ShowZeroCharge);
			fputBoolean(fp, atom->ShowIsotope);
			fputBoolean(fp, atom->ShowNumbering);
			fputBoolean(fp, atom->CrossOutInvalidAtom);


			}
			break;
		
		case BONDSTYLE: {
			BondStyleFormat *bond = (BondStyleFormat *)style->content;

			fputByte(fp, bond->Version);
			fputByte(fp, bond->NewProperties);

			fputStyle(fp, &bond->SinglePen);
			fputStyle(fp, &bond->DoublePen);
			fputStyle(fp, &bond->TriplePen);
			fputStyle(fp, &bond->UpStereoPen);
			fputStyle(fp, &bond->DownStereoPen);
			fputStyle(fp, &bond->CoordinatingPen);
			fputStyle(fp, &bond->UndefinedPen);

			fputInteger(fp, bond->DoubleBetween);
			fputInteger(fp, bond->DoubleShift);
			fputInteger(fp, bond->TripleBetween);
			fputInteger(fp, bond->TripleShift);
			fputInteger(fp, bond->UpStereoWidth);
			fputInteger(fp, bond->DownStereoWidth);
			fputInteger(fp, bond->DownStereoStep);
			fputTPoint(fp, bond->CoordHeadSize);
			fputInteger(fp, bond->UndefinedWidth);
			fputInteger(fp, bond->UndefinedStep);
			
			}
			break;
		
		case FONTSTYLE: {
			FontStyleFormat *font = (FontStyleFormat *)style->content;

			fputByte(fp, font->Version);
			fputInteger(fp, font->Size);
			fputWord(fp, font->FontStyle);
			fputTColorRef(fp, font->Color);
			fputString(fp, font->Name);
			fputInteger(fp, font->DPI);
			}
			break;

		case PENSTYLE:{
			PenStyleFormat *pen = (PenStyleFormat *)style->content;

			fputByte(fp, pen->Version);
			fputInteger(fp, pen->PenWidth);
			fputByte(fp, pen->PenStyle);
			fputTColorRef(fp, pen->PenColor);
			}
			break;

		case INTERSECTIONSTYLE:{
			IntersectionStyleFormat *i = (IntersectionStyleFormat *)style->content;

			fputByte(fp, i->Version);
			fputByte(fp, i->NewProperties);
			fputBoolean(fp, i->IntersectionsEnable);
			fputInteger(fp, i->WhiteSpace);
			}
			break;

		case ARROWSTYLE: {
			ArrowStyleFormat *a = (ArrowStyleFormat *)style->content;
			int i;

			fputByte(fp, a->Version);
			fputInteger(fp, a->DPI);
			for(i = 0; i < 4; ++i)
				fputByte(fp, a->Heads[i]);
			fputInteger(fp, a->Double);
			fputTPoint(fp, a->Size);
		}break;
		
	}
	endpos = ftell(fp);
	size = endpos-beginpos;
	fseekputLongInt(fp, size, sizepos);

	return(size);

}


int addstyle(struct sk2info *info, void *style, int type)
{
	info->styles = realloc(info->styles, sizeof(struct style) * (info->Nstyle +1));
	assert(info->styles);
	info->styles[info->Nstyle].content = style;
	info->styles[info->Nstyle].type = type;
	++info->Nstyle;
	return(info->Nstyle - 1);
}

void *DestroyStyle(struct style *style)
{
	switch(style->type){
		case ATOMSTYLE: {
			AtomStyleFormat *atom = (AtomStyleFormat *)style->content;
			
			DestroyStyle(&atom->BaseFont);
			DestroyStyle(&atom->HydrogenFont);
			DestroyStyle(&atom->IndexFont);
			DestroyStyle(&atom->ValenceFont);
			DestroyStyle(&atom->ChargeFont);
			DestroyStyle(&atom->IsotopeFont);
			DestroyStyle(&atom->NumberingFont);
			}
			break;
		
		case BONDSTYLE: {
			BondStyleFormat *bond = (BondStyleFormat *)style->content;
			
			DestroyStyle(&bond->SinglePen);
			DestroyStyle(&bond->DoublePen);
			DestroyStyle(&bond->TriplePen);
			DestroyStyle(&bond->UpStereoPen);
			DestroyStyle(&bond->DownStereoPen);
			DestroyStyle(&bond->CoordinatingPen);
			DestroyStyle(&bond->UndefinedPen);
			free(bond);
			}
			break;
		
		case FONTSTYLE: {
			FontStyleFormat *font = (FontStyleFormat *)style->content;
			free(font->Name);
			free(font);
			}
			break;

		case PENSTYLE:
		case INTERSECTIONSTYLE:
		case ARROWSTYLE:
			free(style->content);
		break;
		
	}

	return(NULL);
}


FontStyleFormat *CreateFont(int size, int style, int color, char *name)
{
	FontStyleFormat *font;

	font = malloc(sizeof(FontStyleFormat));
	assert(font);

	font->Version = 0;
	font->Size = size;
	font->FontStyle = style;
	font->Color = int2color(color);
	font->Name = strdup(name);
	font->DPI = 300;
	return(font);
}


PenStyleFormat *CreatePen(int width, int style, int color)
{
	PenStyleFormat *pen;

	pen = malloc(sizeof(PenStyleFormat));
	assert(pen);

	pen->Version = 0;
	pen->PenWidth = width;
	pen->PenStyle = style;
	pen->PenColor = int2color(color);
	return(pen);
}

PenStyleFormat *CreateDefPenStyle()
{
	return(CreatePen(3, PENSTYLE_SOLID, 0));
}


AtomStyleFormat *CreateDefAtomStyle()
{
	AtomStyleFormat *style;

	style = calloc(1, sizeof(AtomStyleFormat));
	assert(style);

	style->Version = 0;
	style->NewProperties = 0;
	style->BaseFont.type = FONTSTYLE;
	style->BaseFont.content = CreateFont(20, FS_NORMAL, 0, "Arial");
	style->HydrogenFont.type = FONTSTYLE;
	style->HydrogenFont.content = CreateFont(20, FS_NORMAL, 0, "Arial");
	style->IndexFont.type = FONTSTYLE;
	style->IndexFont.content = CreateFont(16, FS_NORMAL, 0, "Arial");
	style->ValenceFont.type = FONTSTYLE;
	style->ValenceFont.content = CreateFont(16, FS_NORMAL, 0, "Arial");
	style->ChargeFont.type = FONTSTYLE;
	style->ChargeFont.content = CreateFont(16, FS_NORMAL, 0, "Arial");
	style->IsotopeFont.type = FONTSTYLE;
	style->IsotopeFont.content = CreateFont(16, FS_NORMAL, 0, "Arial");
	style->NumberingFont.type = FONTSTYLE;
	style->NumberingFont.content = CreateFont(16, FS_NORMAL, 0, "Arial");
	style->IndexOffset = 10;
	style->ValenceOffset = -30;
	style->IsotopeOffset = -30;
	style->ChargeOffset = -30;
	style->NumberingOffset.X = 0;
	style->NumberingOffset.Y = 30;
	style->ValenceAfterHydrogen = TRUE;
	style->ChargeAfterHydrogen = TRUE;
	style->ShowCarbons = FALSE;
	style->ShowTerminalCarbons = FALSE;
	style->ShowHydrogens = TRUE;
	style->ShowValence = FALSE;
	style->ShowZeroCharge = FALSE;
	style->ShowIsotope = FALSE;
	style->ShowNumbering = FALSE;
	style->CrossOutInvalidAtom = FALSE;

	return(style);
}



BondStyleFormat *CreateDefBondStyle()
{
	BondStyleFormat *style;

	style = calloc(1, sizeof(BondStyleFormat));
	assert(style);

	style->Version = 0;
	style->NewProperties = 0;
	style->SinglePen.type = PENSTYLE;
	style->SinglePen.content = CreatePen(3, PENSTYLE_SOLID, 0);
	style->DoublePen.type = PENSTYLE;
	style->DoublePen.content = CreatePen(3, PENSTYLE_SOLID, 0);
	style->TriplePen.type = PENSTYLE;
	style->TriplePen.content = CreatePen(3, PENSTYLE_SOLID, 0);
	style->UpStereoPen.type = PENSTYLE;
	style->UpStereoPen.content = CreatePen(3, PENSTYLE_SOLID, 0);
	style->DownStereoPen.type = PENSTYLE;
	style->DownStereoPen.content = CreatePen(3, PENSTYLE_SOLID, 0);
	style->CoordinatingPen.type = PENSTYLE;
	style->CoordinatingPen.content = CreatePen(3, PENSTYLE_SOLID, 0);
	style->UndefinedPen.type = PENSTYLE;
	style->UndefinedPen.content = CreatePen(3, PENSTYLE_SOLID, 0);
	style->DoubleBetween = 16;
	style->DoubleShift = 20;
	style->TripleBetween = 16;
	style->TripleShift = 20;
	style->UpStereoWidth = 24;
	style->DownStereoWidth = 24;
	style->DownStereoStep = 7;
	style->CoordHeadSize.X = 16;
	style->CoordHeadSize.Y = 6;
	style->UndefinedWidth = 24;
	style->UndefinedStep = 12;
	
	return(style);
}

IntersectionStyleFormat *CreateDefIntersectionStyle()
{
	IntersectionStyleFormat *i;

	i = calloc(1, sizeof(IntersectionStyleFormat));
	assert(i);
	return(i);	
}


ArrowStyleFormat *CreateDefArrowStyle()
{
	ArrowStyleFormat *a;

	a = malloc(sizeof(ArrowStyleFormat));
	assert(a);

	a->Version = 0;
	a->DPI = 300;
	a->Heads[0] = AS_NULL;
	a->Heads[1] = AS_NULL;
	a->Heads[2] = AS_NULL;
	a->Heads[3] = AS_NULL;
	a->Double = 0;
	a->Size.X = 0;
	a->Size.Y = 0;

	return(a);
}

ArrowStyleFormat *CreateArrowStyle(int h0, int h1, int h2, int h3, int Double, int x, int y)
{
	ArrowStyleFormat *a;

	a = malloc(sizeof(ArrowStyleFormat));
	assert(a);

	a->Version = 0;
	a->DPI = 300;
	a->Heads[0] = h0;
	a->Heads[1] = h1;
	a->Heads[2] = h2;
	a->Heads[3] = h3;
	a->Double = Double;
	a->Size.X = x;
	a->Size.Y = y;

	return(a);
}
