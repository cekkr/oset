/* _sym.c
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

#include "mollib.h"

int getvariable(struct struct_info *info, char *name)
{
	int i;
	int ret = -1;

	for (i = 0; (i < info->numvariables) && (ret == -1); ++i)
		if (!stricmp(info->variables[i].name, name))
			ret = i;  

	return(ret);
}


int getvariabletype(struct struct_info *info, char *name)
{
	int i;
	int ret = -1;

	for (i = 0; (i < info->numvariables) && (ret == -1); ++i)
		if (!stricmp(info->variables[i].name, name))
			ret = info->variables[i].type;

	return(ret);
}


void *getvariabledata(struct struct_info *info, char *name)
{
	int i;
	void *ret = NULL;

	for (i = 0; (i < info->numvariables) && (ret == NULL); ++i)
		if (!stricmp(info->variables[i].name, name))
			ret = &info->variables[i].data;

	return(ret);
}


int	createvariable(struct struct_info *info, char *name, int type)
{
	int var;

	if ((var = getvariable(info, name)) != -1) {
		if ((type != V_ANY) && (type != info->variables[var].type))
			err_parse(info, "%s type redefinition", name);
	} else {
		struct variable v;

		var = info->numvariables;

		memset(&v, 0, sizeof(struct variable));
		v.type = type;
		v.name = strdup(name);
		assert(v.name);

		info->variables = realloc(info->variables, sizeof(struct variable) * (info->numvariables+1));
		assert(info->variables);
		info->variables[info->numvariables] = v;
		++info->numvariables;
	}

	return(var + TOK_VARIABLE);
}


int createstring(struct struct_info *info, char *text)
{

	info->strings = realloc(info->strings, sizeof(char *) * (info->numstrings+1));
	assert(info->strings);
	info->strings[info->numstrings] = strdup(text);
	++info->numstrings;

	return(TOK_ECHOSTRING + info->numstrings-1);	
}

