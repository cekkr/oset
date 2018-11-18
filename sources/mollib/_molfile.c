/* _molfile.c
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


struct molfileparser {
	char *buff;
	char *pos;
	int line;
	int offset;
	BOOL eof;
	BOOL eol;
	int error;
};

#define NOERROR 0
#define UNEXPECTED_EOF 1
#define UNEXPECTED_EOL 2
#define EXPECTED_SYMBOL 3
#define EXPECTED_INTEGER 4
#define EXPECTED_DOUBLE 5
#define MAXERR 5


int simb_ord( const void *arg1, const void *arg2 );

int readstr(struct molfileparser *info, char *ret, int width, BOOL force)
{
	int i;

	if(info->error)
		return(info->error);
	

	for(i = 0; (i < width) && (!info->eof) && (!info->eol); ++i) {
		ret[i] = *info->pos;
		++info->pos;
		++info->offset;
		switch(*info->pos){
			case 26: info->eof = TRUE;
				break;
			case 10:
			case 13: info->eol = TRUE;
		}
	}
	ret[i] = 0;

	if(force && (i < width))
		if(info->eof)
			info->error = UNEXPECTED_EOF;
		else if(info->eol)
			info->error = UNEXPECTED_EOL;

	return(info->error);
}


int readint(struct molfileparser *info, int *ret, int width, int lo, int hi, BOOL force)
{
	int i;
	char s[5], *aux;
	int sign = 1, val = 0;

	hi == lo;
		
	if(readstr(info, s, width, force))
		return(info->error);
	
	for(i = width-1; (s[i] == 32) && i; --i)//strip spaces
		s[i] = 0;
	for(aux = s; *aux == 32; ++aux) 
		;

	if(strlen(aux) == 0){
		if(force) 
			info->error = EXPECTED_INTEGER;
		else {
			*ret = 0;
			return(info->error);
		}
	}

	if((aux[0] == '-') || (aux[0] == '+')) {		//read sign
		if(aux[0] == '-')
			sign = -1;
		++aux;
		if(strlen(aux) == 0)
			info->error = EXPECTED_INTEGER;
	}

	while((*aux != 0) && !info->error) {	  	//read digits
		if(isdigit(*aux))
			val = val*10 + *aux - '0';
		else 
			info->error = EXPECTED_INTEGER;
		++aux;
	}
	val *= sign;
	if(!info->error)
		*ret = val;
 
	return(info->error);
}

int nextline(struct molfileparser *info, BOOL force)
{
	if(info->error)
		return(info->error);
	
	while(!info->eof && !info->eol) {
		//skip non-eol chars
		switch(*info->pos){
			case 26: info->eof = TRUE;
				break;
			case 10:
			case 13: info->eol = TRUE;
				break;
			default:
				++info->pos;
				++info->offset;
		}
	}
	
	if(info->eof && force)
		info->error = UNEXPECTED_EOF;
	else if(info->eol) { //skip eol
		if(((*info->pos == 13) && (*(info->pos+1) == 10)) || ((*info->pos == 10) && (*(info->pos+1) == 13)))
			info->pos += 2;
		else
			++info->pos;
		info->eol = FALSE;
		++info->line;
		info->offset = 1;
		if(*info->pos == 26)
			info->eof = TRUE;
	}

	return(info->error);
}

int readdouble(struct molfileparser *info, double *ret, BOOL force)
{
	char s[15], *aux;
	double val;

	if(readstr(info, s, 10, force))
		return(info->error);

	val = strtod(s, &aux);
	if(*aux == 0)
		*ret = val;
	else if(aux == s)
		info->error = EXPECTED_DOUBLE;
	else {
		*ret = val;
	 	while(*aux) {
			if(*aux != 32)
				info->error = EXPECTED_DOUBLE;
		 	++aux;
		}
	}
	return(info->error);


}

int nextchar(struct molfileparser *info, BOOL force)
{
	if(info->error)
		return(info->error);
	
	if(force)
		if(info->eof)
			info->error = UNEXPECTED_EOF;
		else if(info->eol)
			info->error = UNEXPECTED_EOL;

	if(!info->eof && !info->eol) {
		//skip char
		++info->pos;
		++info->offset;
		switch(*info->pos){
			case 26: info->eof = TRUE;
				break;
			case 10:
			case 13: info->eol = TRUE;
		}
	}

	return(info->error);
}

int readZ(struct molfileparser *info, int *ret, BOOL force)
{
	int i, Z;
	char s[5], *aux;
	extern char *symbol[];
	
	if(readstr(info, s, 3, force))
		return(info->error);

	for(i = 2; (s[i] == 32) && i; --i)//strip spaces
		s[i] = 0;
	for(aux = s; *aux == 32; ++aux) 
		;

	if((strlen(aux) == 0) && !force) {
		*ret = 6;
		return(info->error);
	}

	for (Z=1; (Z<=83) && stricmp(aux, symbol[Z]); Z++)
				;
	if (Z > 83)
		info->error = EXPECTED_SYMBOL;
	else
		*ret = Z;

	return(info->error);
}




struct mol *readmolfilebuff(char *buff)
{
	struct mol *mol;
	struct molfileparser *info;
	int i, Natom, Nbond;
	BOOL xq = FALSE; //extended charge

	static char *errmsg[MAXERR] = {"Unexpected EOF", "Unexpected EOL", 
		"Expected symbol", "Expected integer", "Expected double" };
	

	info = malloc(sizeof(struct molfileparser)); 
	assert(info);
	info->line = 1;
	info->offset = 1;
	info->buff = info->pos = buff;
	info->eof = FALSE;
	info->error = FALSE;
	info->eol = FALSE;
	mol = new_mol();
	
	nextline(info, TRUE);	   // ignore header block
	nextline(info, TRUE);
	nextline(info, TRUE);

	readint(info, &Natom, 3, 0, 255, TRUE);	// read counts line
	readint(info, &Nbond, 3, 0, 255, TRUE);
	nextline(info, TRUE);
	if(info->error != 0) goto err;

	// read atom block	
	for(i = 0; i < Natom; ++i){
		int t, q, Z, dummy;
		double x, y, z;

		readdouble(info, &x, TRUE);
		readdouble(info, &y, TRUE);
		readdouble(info, &z, FALSE);
		nextchar(info, FALSE);
		readZ(info, &Z, FALSE);
		readint(info, &dummy/*NULL*/, 3, 0, 0, FALSE);  // ignore isotope?
		readint(info, &t, 3, 0, 0, FALSE); // read old-style charge
		if((t <= 7) && (t > 0))  // convert old sytle charge
			q = 4 - t;
		else
			q = 0;
		nextline(info, TRUE);
		if(info->error) goto err;
		new_atom(mol, Z, q, x, y, z);
	}

	// read bond block
	for(i = 0; i < Nbond; ++i){
		int a1, a2, j, o;

		readint(info, &a1, 3, 1, Natom, TRUE);
		readint(info, &a2, 3, 1, Natom, TRUE);
		readint(info, &o, 3, 0, 0, FALSE);   // read bond order
		if((o < 1) || (o > 3))      // treat special bond orders like single bonds
			o = 1;					// NOTE: this is wrong for aromatics.
		nextline(info, TRUE);
		if(info->error) goto err;
		for(j = 0; j < o; ++j)
			new_bond(mol, a1-1, a2-1);
	}

	// read properties block;
	while((!info->eof) && (!info->error)){
		char s[10];

		readstr(info, s, 6, FALSE);
		if(stricmp(s, "M  CHG") == 0){ //read charge line
			int nchg;

			if(!xq){
				xq = TRUE;
				for(i = 0; i < Natom; ++i)
					setatomZ(mol, i, 0);
			}
			readint(info, &nchg, 3, 1, 8, TRUE);
			if(info->error) goto err;
			for(i = 0; i < nchg; ++i){
				int q, na;

				nextchar(info, TRUE);
				readint(info, &na, 3, 1, Natom, TRUE);
				nextchar(info, TRUE);
				readint(info, &q, 3, -15, 15, TRUE);
				if(info->error) goto err;
				if(setatomZ(mol, na-1, q) != 0) {
					goto err;
				}
			}
		} else if(stricmp(s, "M  END") == 0)
			info->eof = TRUE;

		nextline(info, FALSE);
	}


err:
	if (info->error) {
		--info->error;
		seterr("Error: %s, line %i:%i\n", info->error <= MAXERR ? errmsg[info->error] : "?", info->line, info->offset);
		mol = destroy_mol(mol);
	}	
	free(info);
	return(mol);
}

#define MOLBUFFINC 1024
#define MAXMOLFILESIZE 65535

struct mol *readmolfile(char *filename)
{
	FILE *data;
	char *buff = NULL;
	int pos = 0;
	struct mol *mol = NULL;
		
	if((data = fopen(filename,"r")) == NULL) {
		seterr("Error reading file (probably it does not exist)");
	} else {
		while((!feof(data)) && (!ferror(data)) && (pos < MAXMOLFILESIZE)){
			buff = realloc(buff, pos + MOLBUFFINC);
			pos += fread(buff+pos, 1, MOLBUFFINC, data);
		}
		buff[pos] = 26;		/* eof */
		if(ferror(data))
			seterr("Error reading file");
		else if (pos >= MAXMOLFILESIZE)
			seterr("File too long");
		else // everything OK: read buffer
			mol = readmolfilebuff(buff);
		
		fclose(data);
		free(buff);
	}
	return(mol);
}


struct struct_log *writemolfilebuf(struct mol *mol)
{
	int i, ncharge;
	extern char *symbol[];
	struct atom *atom;
	struct bond *bond;
	char s[100], s2[100];
	struct struct_log *buf;

	buf = init_log(1024);
	

	logadd(buf, "\n\n\n"); //header
	sprintf(s, "%3i%3i%3i%3i%3i%3i%3i%3i%3i%3i%3i%6s\n", mol->Natom, mol->Nbond,
			 0, 0, 0, 0, 0, 0, 0, 0, 999, "V2000");   // counts line
	logadd(buf, s);

	for(i = 0; i < mol->Natom; ++i) { //atom block
		atom = &mol->atoms[i];
		sprintf(s, "%10.4f%10.4f%10.4f %-3s%2i%3i%3i%3i%3i%3i%3i%3i%3i%3i%3i%3i\n",
				atom->x, atom->y, atom->z, symbol[atom->Z], 
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		logadd(buf, s);
	}

	for(i = 0; i < mol->Nbond; ++i) { //bond block
		bond = &mol->bonds[i];
		sprintf(s, "%3i%3i%3i%3i%3i%3i%3i\n", bond->a1+1, bond->a2+1, bond->order,
				0, 0, 0, 0);
		logadd(buf, s);
	}
	
	s2[0] = 0;
	ncharge = 0;
	for(i = 0; i < mol->Natom; ++i){ //properties block (charge)
		atom = &mol->atoms[i];
		if(atom->charge != 0){
			sprintf(s, " %3i %3i", i+1, atom->charge);
			strcat(s2, s);
			++ncharge;
			if(ncharge == 8) {
				sprintf(s, "M  CHG%3i%s\n", 8, s2);
				logadd(buf, s);
				ncharge = 0;
				s2[0] = 0;
			}
		}
	}
	if(ncharge > 0){
		sprintf(s, "M  CHG%3i%s\n", ncharge, s2);
		logadd(buf, s);
	}
	

	logadd(buf, "M  END\n");  //end

	return(buf);
	
}


int writemolfile(char *filename, struct mol *mol)
{
	FILE *data;
	struct struct_log *buf;

	if((data = fopen(filename,"w")) == NULL) {
		seterr("Error opening file for writing");
		return(-1);
	} else {
		buf = writemolfilebuf(mol);
		fwrite(buf->buffer, buf->length, 1, data);
		fclose(data);
		destroy_log(buf);
		return(0);
	}
}




