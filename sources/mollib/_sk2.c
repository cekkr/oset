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

int fputPageInfo(struct sk2info *info)
{

	fwrite(&info->pageinfo, SIZEOF_PageInfoFormat, 1, info->fp);
	
	return(SIZEOF_PageInfoFormat);
}

int fputPicture(struct sk2info *info)
{
	int beginpos, endpos, i;

	beginpos = ftell(info->fp);

	fputByte(info->fp, 0);  // Version = 0
	fputInteger(info->fp, 300);  // DPI = 300

	fputInteger(info->fp, info->Nstyle);  // StylesCount
	fputInteger(info->fp, (Integer)info->obj_list->Nobj);  // ObjCount

	for(i = 0; i < info->Nstyle; ++i)   // StylesList
		fputStyle(info->fp, &info->styles[i]);

	for(i = 0; i < info->obj_list->Nobj; ++i)   // ObjectsList
		fputObject(info->fp, &info->obj_list->objs[i]);


	endpos = ftell(info->fp);
	
	return(endpos-beginpos);
}


int fputPageDescription(struct sk2info *info)
{
	int beginpos, endpos, sizepos, size;

	beginpos = ftell(info->fp);

	fputByte(info->fp, 0);  // Version = 0
	fputBoolean(info->fp, FALSE);  // PageNameflag = 0

	sizepos = ftell(info->fp);
	fputLongInt(info->fp, 0);
	size = fputPageInfo(info);   // PageInfo
	fseekputLongInt(info->fp, size, sizepos); // PageInfoSize

	sizepos = ftell(info->fp);
	fputLongInt(info->fp, 0);
	size = fputPicture(info);   // Picture
	fseekputLongInt(info->fp, size, sizepos); // PictureSize

	endpos = ftell(info->fp);
	
	return(endpos-beginpos);
}

int fputDocumentDescription(struct sk2info *info)
{
	int beginpos, endpos, sizepos, size;

	beginpos = ftell(info->fp);

	
	fputByte(info->fp, 0);  // Version = 0
	fputInteger(info->fp, 1);  // PageCount = 1
	sizepos = ftell(info->fp);
	fputLongInt(info->fp, 0);
	size = fputPageDescription(info);
	fseekputLongInt(info->fp, size, sizepos);


	endpos = ftell(info->fp);
	
	return(endpos-beginpos);
}


int fputEnvironment(struct sk2info *info)
{
	int beginpos, endpos;
	EnvironmentDescriptionFormat env =
		{0,			// Active page
		200,		// Zoom = 100%. Yes, 100%
		{3, 3}};	// ViewOrigin

	beginpos = ftell(info->fp);

	fputLongInt(info->fp, SIZEOF_EnvironmentDescriptionFormat);
	fwrite(&env, SIZEOF_EnvironmentDescriptionFormat, 1, info->fp);

	endpos = ftell(info->fp);
	return(endpos-beginpos);
}

int fputDocument(struct sk2info *info)
{
	LongInt sizepos, size;

	fputPChar(info->fp, "(C) ACD $$");
	fputPChar(info->fp, ".RPT.( V 2.0 )");
	sizepos = ftell(info->fp);
	fputLongInt(info->fp, 0);
	size = fputDocumentDescription(info);
	fseekputLongInt(info->fp, size, sizepos);
	fputEnvironment(info);
	fclose(info->fp);
	
	return(1);
}

void setpageinfo_sk2(struct sk2info *info,
			int pagesize_x, int pagesize_y,
			int printablepagesize_x, int printablepagesize_y,
			double prnpages_x, double prnpages_y,
			int margin_left, int margin_top, int margin_right, int margin_bottom,
			int orientation)
{
	PageInfoFormat p = 
		{{-1, -1},											// reserved
		{(Integer)pagesize_x, (Integer)pagesize_y},							
		{(Integer)printablepagesize_x, (Integer)printablepagesize_y},			
		{(Single)prnpages_x, (Single)prnpages_y},							
		{(Integer)margin_left, (Integer)margin_top, (Integer)margin_right, (Integer)margin_bottom},	
		 (Integer)orientation};
	
	info->pageinfo = p;
}

struct sk2info *create_sk2(char *fname)
{
	struct sk2info *info;

	PageInfoFormat defpinfo =
		{{-1, -1},				// reserved
		{3301, 2551},			// PageSize (letter)
		{3301, 2551},			// PrintablePageSize
		{1.0, 1.0},				// PrnPages
		{150, 150, 150, 150},	// half-inch Margins
		LANDSCAPE};				// Orientation

	info = calloc(1, sizeof(struct sk2info));
	assert(info);

	info->filename = strdup(fname);

	info->defatomstyle = -1;
	info->defbondstyle = -1;
	info->defpenstyle = -1;
	info->defarrowstyle = -1;

	addstyle(info, CreateDefIntersectionStyle(), INTERSECTIONSTYLE);

	info->obj_list = CreateObjList();
	info->pageinfo = defpinfo;

	return(info);
}



int write_sk2(struct sk2info *info)
{
	int i;

	if((info->fp = fopen(info->filename, "wb")) == 0) {
		return(0);
	} else {
		fputDocument(info);
		fclose(info->fp);
		//destroy styles;
		for(i = 0; i < info->Nstyle; ++i)
			DestroyStyle(&info->styles[i]);
		free(info->styles);

		//destroy objects;
		for(i = 0; i < info->obj_list->Nobj; ++i)
			DestroyObject(&info->obj_list->objs[i]);
		
		free(info->obj_list->objs);
		free(info->obj_list);
		free(info->filename);
		free(info);

	}
	return(1);
}

