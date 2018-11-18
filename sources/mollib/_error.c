/* _error.c
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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
#include "caoscomp.h"

extern jmp_buf mark;

void err_msg(char *s, ...)
{	
	char buff[300];
	char *args = ((char *)(&s) + sizeof(char *));

	vsprintf(buff, s, args);
	printf(buff);
	exit(1);
}



void err_scan(struct struct_info *info, char *s, ...)
{
	char s2[200];
	char buff[300];
	char *args = ((char *)(&s) + sizeof(char *));

	sprintf(s2, "Error scanner, file %s line %i: %s\n", info->fname, info->line+1, s);

	vsprintf(buff, s2, args);
	printf(buff);
	longjmp(mark, -1);
	exit(1);
}


void err_parse(struct struct_info *info, char *s, ...)
{
	char s2[200];
	char buff[300];
	char *args = ((char *)(&s) + sizeof(char *));

	sprintf(s2, "Error parser, file %s line %i: %s\n", info->fname, info->line+1, s);

	vsprintf(buff, s2, args);
	printf(buff);
	longjmp(mark, -1);
	exit(1);
}


void err_exec(struct struct_info *info, char *s, ...)
{
	char s2[200];
	char buff[300];
	char *args = ((char *)(&s) + sizeof(char *));

	info;

	sprintf(s2, "Error exec: %s\n", s);

	vsprintf(buff, s2, args);
	printf(buff);
	exit(1);
}


static char errorstring[300];
static BOOL error;

void seterr(char *s, ...)
{
	char *args = ((char *)(&s) + sizeof(char *));
	vsprintf(errorstring, s, args);
	error = TRUE;
}


char *geterr(void)
{
	if (error) {
		error = FALSE;
		return(errorstring);
	} else
		return("");
}

