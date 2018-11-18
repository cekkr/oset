/* _funcgrp.c
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


BOOL	debugfuncgrp = FALSE;
void printfuncgrp(struct funcgrpnode *f, int level);

struct funcgrpnode *parsefuncgrp(char *s);
struct ringgrp *parseringgrp(char *s);
struct multiringgrp *parsemultiringgrp(struct struct_info *info);


extern int primes[];


void readfuncgrp(struct struct_info *info)
{
	struct variable v;

	memset(&v, 0, sizeof(struct variable));
	match(info, TOK_ID);
	v.type = V_FUNCGRP;
	v.name = strdup(info->token.valor);
	assert(v.name);
	scanner(info);
	matchandscan(info, TOK_IGUAL);
	match(info, TOK_STRING);
	v.data.fg.funcgrpnode = parsefuncgrp(info->token.valor);
	if (debugfuncgrp) 
		printf("%s\n", info->token.valor);

	scanner(info);
	matchandscan(info, TOK_COMA);
	match(info,TOK_NUMERO);
	v.data.fg.complex = *((int *)info->token.valor);
	scanner(info);

	if (debugfuncgrp)  {
		printfuncgrp(v.data.fg.funcgrpnode,0);
	}

	info->variables = realloc(info->variables, sizeof(struct variable) * (info->numvariables+1));
	assert(info->variables);
	info->variables[info->numvariables] = v;
	++info->numvariables;
}


void readringgrp(struct struct_info *info)
{
	struct variable v;

	memset(&v, 0, sizeof(struct variable));
	match(info, TOK_ID);
	v.name = strdup(info->token.valor);
	assert(v.name);
	scanner(info);
	matchandscan(info, TOK_IGUAL);

	matchlist(info, "Expected ringpattern", TOK_STRING, TOK_PARENABRE, 0);

	if (info->token.tipo == TOK_STRING) {
		/* ringgrp */
		v.type = V_RINGGRP;
		v.data.rg.ringgrp = parseringgrp(info->token.valor);

		scanner(info);

		matchandscan(info, TOK_COMA);
		match(info,TOK_NUMERO);
		v.data.rg.complex = *((int *)info->token.valor);
		scanner(info);

		info->variables = realloc(info->variables, sizeof(struct variable) * (info->numvariables+1));
		assert(info->variables);
		info->variables[info->numvariables] = v;
		++info->numvariables;
	} else if (info->token.tipo == TOK_PARENABRE) {
		/* multiringgrp */
		v.type = V_MULTIRINGGRP;
		v.data.mrg.multiringgrp = parsemultiringgrp(info);
		v.data.mrg.multiringgrp->ringtype = info->numvariables;

		matchandscan(info, TOK_COMA);
		match(info,TOK_NUMERO);
		v.data.rg.complex = *((int *)info->token.valor);
		scanner(info);

		info->variables = realloc(info->variables, sizeof(struct variable) * (info->numvariables+1));
		assert(info->variables);
		info->variables[info->numvariables] = v;
		++info->numvariables;
	}
}



char *nextspace(char *aux)
{
	int par = 0;

	for (; *aux; ++aux) {
		switch(*aux) {
			case '(':
				++par;
				break;
			case ')':
				--par;
				break;
			case ' ':
				if (par == 0)
					return(aux);
				break;
		}
	}

	return(NULL);
}



struct funcgrpnode *parsefuncgrp(char *s)
{
	struct funcgrpnode *node;
	char *ch;
	int state = 0;
	int ofs = 0;
	int child = 0;
	int aliph = 0;
	int arom = 0;

	ch = s;

	node = malloc(sizeof(struct funcgrpnode));
	memset(node, 0, sizeof(struct funcgrpnode));
	node->order = 1;
	node->min = node->max = 1;
	node->structural = TRUE;

	while (state != -1) {
		aliph = 0;
		switch(state) {
			case 0:		/* inicio, espera =%[ CH, etc*/
				switch(*ch) {
					case '~':
						node->order = -1;
						state = 1;
						break;
					case '=':
						node->order = 2;
						state = 1;
						break;
					case '#':
						node->order = 3;
						state = 1;
						break;
					case '[':
						node->structural = FALSE;
						state = 2;
						break;
					case ' ':
						break;
					default:
						--ch;
						state = 2;
						break;
				}
				break;

			case 1:		/* se espera [ CH, etc */
				switch (*ch) {
					case '[':
						node->structural = FALSE;
						state = 2;
						break;
					default:
						--ch;
						state = 2;
						break;
				}
				break;

			case 2:		/* se espera atomo */
				switch (*ch) {
					case ']':
						assert(!node->structural);
						state = 3;
						break;
					case '(':
						state = 4;
						break;
					case ' ':
					case 0:
						state = -1;
						break;

					case '/':
						aliph = 1;
						++ch;

						// fall-through;
					default:
						if (ofs < sizeof(node->atoms)-1) {
							int arom = 0;
							node->atoms[ofs].Z = aromatomnum(&ch, &arom);
							--ch;

							assert(node->atoms[ofs].Z != -1);
							if (*(ch+1) == '+') {
								++ch;
								node->atoms[ofs].charge = 1;
							}
							if (*(ch+1) == '-') {
								++ch;
								node->atoms[ofs].charge = -1;
							}

							if ((!arom && !aliph) || (arom && aliph))
								node->atoms[ofs].aromaticity = FGATOM_DONTCARE;
							else if (arom)
								node->atoms[ofs].aromaticity = FGATOM_AROMATIC;
							else
								node->atoms[ofs].aromaticity = FGATOM_ALIPHATIC;

							++ofs;
						}
						break;
				}
				break;

			case 3:	/* se cerro corchete, espera potencia */
				switch (*ch) {
					case '^':
						state = 5;
						break;
					default:
						state = -1;
						break;
				}
				break;

			case 4:	{/* se abrio parentesis */
					char *s = strdup(ch);
					char *aux, *aux2;
					int par = 0;

					for (aux = s; *aux && (par >= 0); ++aux) {
						if (*aux == '(')
							++par;
						if (*aux == ')') 
							--par;
						if (par < 0)
							break;
					}
					assert(*aux);
					*aux = 0;
					ch += (aux+1 - s);

					aux = s;
					if ((aux2 = nextspace(aux)) != NULL)
						*aux2 = 0;
					while (aux && (child < MAXNEIGH)) {
						node->children[child++] = parsefuncgrp(aux);

						if (aux2) { 
							for (aux = aux2+1; *aux == ' '; ++aux)
								;
							if ((aux2 = nextspace(aux)) != NULL)
								*aux2 = 0;
						} else
							aux = NULL;
					}
					free(s);
					state = -1;
				}
				break;

			case 5:		/* espera potencia +*/
				switch (*ch) {
					case '{':
						state = 6;
						break;
					default:
						node->min = node->max = atoi(ch);
						state = -1;
						break;
				}
				break;

			case 6:		/* encontro llave, espera min-max -*/
				node->min = atoi(ch);
				++ch;
				assert(*ch == '-');
				++ch;
				node->max = atoi(ch);
				++ch;
				assert(*ch == '}');
				state = -1;
				break;
				
		} 
		++ch;
	}

/*

	[=,%]\[Nombre\]^{2-3}
	[=,%]Nombre[()]


*/


	return(node);
}


extern char *symbol[];
static char *printatomlist(struct funcgrpnode *f, char *buff, int size)
{
	int i;
	memset(buff, 0, size);

	for (i = 0; i < sizeof(f->atoms)/sizeof(f->atoms[0]); ++i) {
		if (f->atoms[i].Z > 0)
			strcat(buff, symbol[f->atoms[i].Z]);
	}

	return(buff);
}

void printfuncgrp(struct funcgrpnode *f, int level)
{
	int i;
	char buff[40];

	printf("%*s(%i)%-20s %c %i-%i\n", level+1, "", f->order, printatomlist(f, buff, sizeof(buff)), f->structural ? 'G' : ' ', f->min, f->max);
	for (i = 0; i < MAXNEIGH; ++i) 
		if (f->children[i])
			printfuncgrp(f->children[i], level+1);
}



struct ringgrp *parseringgrp(char *s)
{
	int state = 0;
	char sym[2];
	struct ringgrp *ringgrp = calloc(1, sizeof(struct ringgrp));	
	assert(ringgrp);

	memset(sym, 0, sizeof(sym));
	ringgrp->hash = 1;

	while (*s) {
		switch (state) {
			case 0:
				if (islower(*s)) {
					assert(ringgrp->aromatic || (ringgrp->size == 0));
					ringgrp->aromatic = TRUE;
				}
				sym[0] = toupper(*s);
				ringgrp->atoms[ringgrp->size].Z = atomnum(sym);
				ringgrp->atoms[ringgrp->size].order = 1;

				assert(ringgrp->atoms[ringgrp->size].Z != -1);
				ringgrp->hash *= primes[ringgrp->atoms[ringgrp->size].Z];
				++s;
				state = 1;
				break;
			case 1:
				switch(*s) {
					case '=':
						ringgrp->atoms[ringgrp->size].order = 2;
						break;
					case '#':
						ringgrp->atoms[ringgrp->size].order = 3;
						break;
					default:
						--s;
						break;
				}
				state = 0;
				++ringgrp->size;
				assert(ringgrp->size < MAXRINGGRPSIZE);
				++s;
				break;
		}
	}

	if (state == 1)
		++ringgrp->size;

	assert(ringgrp->size < MAXRINGGRPSIZE);

	return(ringgrp);
}


/**************************************************************/




void ringbond(struct struct_info *info, struct multiringgrp *mrg, int ringnum)
{
	int i,j;
	char bondid;

	match(info, TOK_ID);
	if ((strlen(info->token.valor) != 1) || (!isalpha(((char *)info->token.valor)[0])))
		err_parse(info, "Expected ringid (A..Z)");

	bondid = toupper(((char *)info->token.valor)[0]);

	for (i = 0; i < mrg->nummultibonds; ++i)
		if (bondid == mrg->ringfussions[i].name)
			break;
	if (i == mrg->nummultibonds) {
		mrg->ringfussions = realloc(mrg->ringfussions, sizeof(struct mrbond) * (mrg->nummultibonds+1));
		assert(mrg->ringfussions);
		memset(&mrg->ringfussions[mrg->nummultibonds], -1, sizeof(struct mrbond));
		++mrg->nummultibonds;

		mrg->ringfussions[i].name = bondid;
	}

	for (j = 0; j < MAXRINGMULTIBOND; ++j)
		if (mrg->ringfussions[i].rbond[j].numring == -1)
			break;

	if (j == MAXRINGMULTIBOND)
		err_parse(info, "Exceded MAXRINGMULTIBOND");

	mrg->ringfussions[i].rbond[j].numring = ringnum;

	scanner(info);
	matchandscan(info, TOK_IGUAL);
	matchandscan(info, TOK_OPENBRACK);
	match(info, TOK_NUMERO);
	mrg->ringfussions[i].rbond[j].a1 = *(int *)info->token.valor - 1;
	scanner(info);
	matchandscan(info, TOK_COMA);
	match(info, TOK_NUMERO);
	mrg->ringfussions[i].rbond[j].a2 = *(int *)info->token.valor - 1;
	scanner(info);
	matchandscan(info, TOK_CLOSEBRACK);
}	


void ringbondlist(struct struct_info *info, struct multiringgrp *mrg, int ringnum)
{
	while (info->token.tipo == TOK_ID)
		ringbond(info, mrg, ringnum);
}


void ring(struct struct_info *info, struct multiringgrp *mrg)
{
	int var;
	int ringnum;

	matchandscan(info, TOK_PARENABRE);
	match(info, TOK_ID);

	var = getvariable(info, info->token.valor);
	if ((var == -1) || (info->variables[var].type != V_RINGGRP))
		err_parse(info, "Expected RINGGRP");

	ringnum = mrg->numrings;
	mrg->ringtypes = realloc(mrg->ringtypes, sizeof(int) * (mrg->numrings + 1));
	assert(mrg->ringtypes);
	++mrg->numrings;

	mrg->ringtypes[ringnum] = var;
	
	scanner(info);
	matchandscan(info, TOK_COMA);

	ringbondlist(info, mrg, ringnum);

	matchandscan(info, TOK_PARENCIERRA);
}


struct multiringgrp *parsemultiringgrp(struct struct_info *info)
{
	struct multiringgrp *mrg = calloc(1, sizeof(struct multiringgrp));	
	assert(mrg);

	while (info->token.tipo == TOK_PARENABRE)
		ring(info, mrg);

	if(debugfuncgrp){
		int i,j;
		printf("Multiringgrp = ");
		for (i = 0; i < mrg->numrings; ++i)
			printf("[%i,%s]", mrg->ringtypes[i], info->variables[mrg->ringtypes[i]].name);
		for (i = 0; i < mrg->nummultibonds; ++i) {
			printf(" %c:", mrg->ringfussions[i].name);
			for (j = 0; j < MAXRINGMULTIBOND; ++j)
				if (mrg->ringfussions[i].rbond[j].numring != -1)
					printf("[%i:%i,%i]", mrg->ringfussions[i].rbond[j].numring, mrg->ringfussions[i].rbond[j].a1, mrg->ringfussions[i].rbond[j].a2);
		}
		printf("\n");
	}

	return(mrg);
}
