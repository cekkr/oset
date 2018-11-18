/* _log.c
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



void logadd(struct struct_log *log, char *s, ...)
{
	int l;
	char *args = ((char *)(&s) + sizeof(char *));
	char buff[20000];

	if(log && s) {
		vsprintf(buff, s, args);
		l = strlen(buff);
		while(log->length+l >= log->bufsize) {
			log->bufsize += log->bufinc;
			log->buffer = realloc(log->buffer, log->bufsize);
			assert(log->buffer);
		}

		strcat(log->buffer, buff);
		log->length += l;
		++(log->nlogs);
	}
}




struct struct_log * init_log(int bufinc)
{
	struct struct_log *log;

	log = malloc(sizeof(struct struct_log));
	assert(log);
	log->buffer = malloc(bufinc);
	assert(log->buffer);
	log->buffer[0] = 0;
	log->length = 0;
	log->nlogs = 0;
	log->bufsize = bufinc;
	log->bufinc = bufinc;
	
	return(log);
}

struct struct_log * destroy_log(struct struct_log *log)
{
	if(log){
		free(log->buffer);
		free(log);
	}
	return(NULL);
}


/* Destroys the log structure and returns the string buffer    */
char *log2str(struct struct_log *log)
{
	char *s;
	if(log) {
		s = realloc(log->buffer, log->length + 1);
		assert(s);
		free(log);
		return(s);
	} else
		return(NULL);
}

