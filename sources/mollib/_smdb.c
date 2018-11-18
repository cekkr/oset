/* _smdb.c
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
#include <ctype.h>

void readsmiledb(struct struct_info *info)
{
	char smile[100], descr[100];
	char *aux;

	match(info, TOK_STRING);
	strncpy(smile, info->token.valor, sizeof(smile)-1);
	smile[sizeof(smile)-1] = 0;
	if ((aux = smilescanon(smile)) != NULL) {
		strncpy(smile, aux, sizeof(smile)-1);
		free(aux);
	}

	scanner(info);
	matchandscan(info, TOK_IGUAL);
	match(info, TOK_STRING);
	strncpy(descr, info->token.valor, sizeof(descr)-1);
	descr[sizeof(descr)-1] = 0;
	scanner(info);

	info->smiledb = realloc(info->smiledb, sizeof(char *) * (info->numsmiledb + 1));
	assert(info->smiledb);
	info->smiledb[info->numsmiledb] = malloc(strlen(smile)+strlen(descr)+2);
	assert(info->smiledb[info->numsmiledb]);
	sprintf(info->smiledb[info->numsmiledb], "%s%c%s", smile, 0, descr);

	++info->numsmiledb;
}



int compsmiledb(const void *a1, const void *a2)
{
	char **e1 = (char **)a1;
	char **e2 = (char **)a2;

	return(strcmp(*e1, *e2));
}



void sortsmiledb(struct struct_info *info)
{
	qsort(info->smiledb, info->numsmiledb, sizeof(char *), compsmiledb);
}

char *findsmiledb(struct struct_info *info, char *smile)
{
	char **aux = bsearch(&smile, info->smiledb, info->numsmiledb, sizeof(char *), compsmiledb);
	char *aux2;

	if (aux) {
		aux2 = *aux;
		aux2 += strlen(aux2)+1;
		return(aux2);
	} else
		return(NULL);
}
