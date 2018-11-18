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



int fputPChar(FILE *fp, char *s)
{
	Word length;
	if(s)
		length = strlen(s);
	else
		length = 0;

	fwrite(&length, sizeof(Word), 1, fp);

	if(s)
		fwrite(s, length, 1, fp);

	return(length+sizeof(Word));
}


int fputString(FILE *fp, char *s)
{
	Byte length = strlen(s);

	fwrite(&length, sizeof(Byte), 1, fp);
	fwrite(s, length, 1, fp);

	return(length+sizeof(Byte));
}



int fputLongInt(FILE *fp, LongInt i)
{
	fwrite(&i, sizeof(LongInt), 1, fp);
	return(sizeof(LongInt));
}

int fputInteger(FILE *fp, Integer i)
{
	fwrite(&i, sizeof(Integer), 1, fp);
	return(sizeof(Integer));
}

int fputWord(FILE *fp, Word i)
{
	fwrite(&i, sizeof(Word), 1, fp);
	return(sizeof(Word));
}

int fputTColorRef(FILE *fp, TColorRef color)
{
	fwrite(&color, sizeof(TColorRef), 1, fp);
	return(sizeof(TColorRef));
}

int fputTPoint(FILE *fp, TPoint point)
{
	fwrite(&point, sizeof(TPoint), 1, fp);
	return(sizeof(TPoint));
}

int fputTRPoint(FILE *fp, TRPoint point)
{
	fwrite(&point, sizeof(TRPoint), 1, fp);
	return(sizeof(TRPoint));
}

int fputByte(FILE *fp, Byte i)
{
	fwrite(&i, sizeof(Byte), 1, fp);
	return(sizeof(Byte));
}

int fputShortInt(FILE *fp, ShortInt i)
{
	fwrite(&i, sizeof(ShortInt), 1, fp);
	return(sizeof(ShortInt));
}

int fputSingle(FILE *fp, Single f)
{
	fwrite(&f, sizeof(Single), 1, fp);
	return(sizeof(Single));
}

int fputBoolean(FILE *fp, Boolean b)
{
	fwrite(&b, sizeof(Boolean), 1, fp);
	return(sizeof(Boolean));
}

int fseekputLongInt(FILE *fp, LongInt i, int pos)
{
	int oldpos;

	oldpos = ftell(fp);
	fseek(fp, pos, SEEK_SET);
	fwrite(&i, sizeof(LongInt), 1, fp);
	fseek(fp, oldpos, SEEK_SET);
	return(sizeof(LongInt));
}

int fseekputInteger(FILE *fp, Integer i, int pos)
{
	int oldpos;

	oldpos = ftell(fp);
	fseek(fp, pos, SEEK_SET);
	fwrite(&i, sizeof(Integer), 1, fp);
	fseek(fp, oldpos, SEEK_SET);
	return(sizeof(Integer));
}




TColorRef int2color(int color)
{
	TColorRef c;

	c.Red = (color & 0x00FF0000) >> 16;
	c.Green = (color & 0x0000FF00) >> 8;
	c.Blue = (color & 0x000000FF);
	c.Reserved = 0;

	return(c);
}

