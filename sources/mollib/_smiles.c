/* _smiles.c
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
#include <limits.h>
#include <ctype.h>

struct mol2smilesinfo {
	char *smiles;
	int buffsize;
	int length;
	BOOL *usedbonds;
	BOOL *usedatoms;
	struct mol *mol;
};

#define BUFFINC 32
extern char *symbol[];

void dfs(struct mol2smilesinfo *info, struct bond *bond, int natom);
void cleanstring(struct mol2smilesinfo *info);
void appendstr(struct mol2smilesinfo *info, char *s);
void insertstr(struct mol2smilesinfo *info, int index, char *s);
int freebonds(struct mol2smilesinfo *info, int natom); 
struct bond *nextfreebond(struct mol2smilesinfo *info, int natom);
void closering(struct mol2smilesinfo *info, struct bond *bond);
BOOL isavailable(struct mol2smilesinfo *info, int i, int pos1, int pos2);



/* converts a molecule to a SMILES string. Returns the newly created string.
 * It does not take charge into account.  */
char *mol2smiles(struct mol *mol)
{
	struct mol2smilesinfo *info;
	char *smiles;
        int i = 0;

	info = malloc(sizeof(struct mol2smilesinfo));
	assert(info);
	
	info->usedbonds = calloc(mol->Nbond, sizeof(BOOL));
	assert(info->usedbonds);
	info->usedatoms = calloc(mol->Natom, sizeof(int));
	assert(info->usedatoms);
	info->smiles = calloc(1, BUFFINC);
	assert(info->smiles);
	info->buffsize = BUFFINC;
	info->mol = mol;
	info->length = 0;

	if(mol->Natom > 0) {
		while(mol->atoms[i].canon_num != 1) // Find fist atom according to canonical numbering
        		++i;

		dfs(info, NULL, i);
		cleanstring(info);
	}

	smiles = info->smiles;
	free(info->usedatoms);
	free(info->usedbonds);
	free(info);
	return(smiles);
}


/* This is the procedure that actually does the conversion. It performs
 * a recursive depth-first search of the molecular graph, closing rings
 * when it finds a previously visited atom.
 * 'natom' is the number of the next atom to explore, 'bond' is the 
 * the "from" bond. 'bond' may be NULL when dealing with the first atom. */
void dfs(struct mol2smilesinfo *info, struct bond *bond, int natom)
{
	int natom2;
	struct bond *bond2;
	char s[6];

	info->usedatoms[natom] = TRUE;
	if(bond && !info->mol->atoms[natom].aromatic)
		switch(bond->order) {
			case 2:	appendstr(info, "="); break;
			case 3: appendstr(info, "#"); break;
		}
	sprintf(s, "%s{%i}", symbol[info->mol->atoms[natom].Z], natom);
	if(info->mol->atoms[natom].aromatic)
		s[0] = (char)tolower(s[0]);
	appendstr(info, s);
	
	if(!freebonds(info, natom))
		return;
	
	while(freebonds(info, natom)) {
		bond2 = nextfreebond(info, natom);
		natom2 = (bond2->a1 == natom) ? bond2->a2 : bond2->a1;

		if(info->usedatoms[natom2]) { // close ring
			closering(info, bond2);
		} else {
			if(freebonds(info, natom) > 0) { // bond to unused atom
				appendstr(info, "(");
				dfs(info, bond2, natom2);
				appendstr(info, ")");
			} else 
				dfs(info, bond2, natom2);
		}
	}
	return;
}

/* after dfs is done, this function is used to remove auxiliary atom numbers
 * from the string, as well as unnecessary parentheses. */
void cleanstring(struct mol2smilesinfo *info)
{
	char *pos = info->smiles;
	char *pos2, *p;
	int depth;

	while((pos = strchr(pos, '{')) != NULL) { //remove {%i}'s
		pos2 = strchr(pos, '}');
		memmove(pos, pos2+1, strlen(pos2));
	}

	pos = info->smiles;
	while((pos = strchr(pos+1, ')')) != NULL) { //remove unnecessary parentheses
		if((pos[1] == 0) || (pos[1] == ')')){
			memmove(pos, pos+1, strlen(pos));
			depth = 1; //remove left parenthesis
			for(p = pos-1; depth > 0; --p) {
				if(*p == '(')
					--depth;
				else if (*p == ')')
					++depth;
			}
			memmove(p+1, p+2, strlen(p+1));
			pos -= 2;
		}
	}
	
	info->buffsize = strlen(info->smiles)+1;
	info->smiles = realloc(info->smiles, info->buffsize);
}

/* appends string s to the main SMILES string, resizing the buffer if necessary */
void appendstr(struct mol2smilesinfo *info, char *s)
{
	while((info->length + (int)strlen(s)) >= info->buffsize) {
		info->buffsize += BUFFINC;
		info->smiles = realloc(info->smiles, info->buffsize);
		assert(info->smiles);
	}
	strcat(info->smiles, s);
	info->length += strlen(s);
}

/* inserts string 's' to the main SMILES string at offest 'index',
 * resizing the buffer if necessary */
void insertstr(struct mol2smilesinfo *info, int index, char *s)
{
	char *pos = info->smiles + index;

	while((info->length + (int)strlen(s)) >= info->buffsize) {
		info->buffsize += BUFFINC;
		info->smiles = realloc(info->smiles, info->buffsize);
		assert(info->smiles);
		pos = info->smiles + index;
	}
	memmove(pos + strlen(s), pos, strlen(pos)+1);
	memmove(pos, s, strlen(s));
	info->length += strlen(s);


}

/* returns the number of "free" (yet unvisited) bonds at atom 'natom' */
int freebonds(struct mol2smilesinfo *info, int natom) 
{
	int i, f = 0;
	struct atom *atom = &info->mol->atoms[natom];

	for(i = 0; i < atom->Nbond; ++i){
		if(!info->usedbonds[atom->bonds[i]])
			++f;
	}
	return(f);
}

/* returns the next free bond, and marks it as used in info->usedbonds */
struct bond *nextfreebond(struct mol2smilesinfo *info, int natom) 
{
	int i, b = -1;
	struct atom *atom = &info->mol->atoms[natom];
        int index = INT_MAX;

	for(i = 0; i < atom->Nbond; ++i){
		if((!info->usedbonds[atom->bonds[i]]) &&
               	   (info->mol->atoms[atom->neighbors[i]].canon_num < index)) {
                   index = info->mol->atoms[atom->neighbors[i]].canon_num;
                   b = atom->bonds[i];
		}
	}
        if(b < 0)
		return(NULL);
        else {
        	info->usedbonds[b] = TRUE;
        	return(&info->mol->bonds[b]);
        }

}

/* closes the ring. That is, inserts the corresponding markers in the
 * SMILES string 'bond' is the bond that closes the ring */
void closering(struct mol2smilesinfo *info, struct bond *bond)
{
	char s[10], s2[5], bonds[3];
	int pos1, pos2, temp;
	int i;

	sprintf(s, "{%i}", bond->a1);  	//find a1
	pos1 = strstr(info->smiles, s) - info->smiles;

	sprintf(s, "{%i}", bond->a2);	//find a2
	pos2 = strstr(info->smiles, s) - info->smiles;

	if(pos1 > pos2) {
		temp = pos1;
		pos1 = pos2;
		pos2 = temp;
	}
	//find available index
	i = 1;
	while(!isavailable(info, i, pos1, pos2))
		++i;

	bonds[0] = 0;
	if(!info->mol->atoms[bond->a1].aromatic)
		switch(bond->order){
			case 2:
				strcpy(bonds, "=");
				break;

			case 3:
				strcpy(bonds, "#");
				break;

			default:
				bonds[0] = 0;
				break;

		}

	//insert indices
	if(i < 10)
		sprintf(s2, "%s%i", bonds, i);
	else
		sprintf(s2, "%s%%%i", bonds, i);
	
	insertstr(info, pos2, s2); 
	insertstr(info, pos1, s2); 

}

/* Used by closering() to determine if a given index 'i' is available for
 * use to close a given ring (the indexes cannot be nested)
 * pos1 and pos2 are the sting offsets of the two atoms involved.
 * pos1 MUST BE SMALLER THAN pos2  */
BOOL isavailable(struct mol2smilesinfo *info, int i, int pos1, int pos2)
{
	char *p, *p1 = info->smiles + pos1, *p2 = info->smiles + pos2;
	BOOL ret = TRUE;
	char s[5];
	int l;
	
	if(i < 10) 
		sprintf(s, "%i", i);
	else
		sprintf(s, "%%%i", i); //two-digit numbers are preceded by %
	l = strlen(s);

	for(p = info->smiles; p < p1; ++p) { //check if "closed" at p1
		if(*p == '{')
			p = strchr(p, '}');
		else {
			if(strncmp(p, s, l) == 0)
				ret = !ret;
			if(*p == '%')
				p +=2;
		}
	}
	if(ret) { //if closed at p1, make sure it is not used before p2
		for( ; (p < p2) && ret; ++p) {
			if(*p == '{')
				p = strchr(p, '}');
			else {
				if(strncmp(p, s, l) == 0)
					ret = !ret;
				if(*p == '%')
					p +=2;
			}
		}
	}

	return(ret);
}


/****************************************************************************
 ***************************** SMILES PARSER ********************************
 ****************************************************************************/


#define MAX_RINGS				100
#define NO_ERROR				0
#define EXPECTED_SYMBOL			1
#define EXPECTED_NUMBER			2
#define INCONSISTENT_BOND_ORDER	3
#define INITIAL_BOND			4
#define MAXERR					4

struct openring {
	int atom;
	int order;
};

struct smiles_parser  {
	struct openring openrings[MAX_RINGS];
	char *pos;
	char *smiles;
	struct mol *mol;
	int error;
};



int read_bond(struct smiles_parser *info)
{
	int order = 0;

	if(*info->pos) {
		switch(*info->pos) {
			case '-':  // single bond
				order = 1;
				++info->pos;
				break;

			case '=':  // double bond
				order = 2;
				++info->pos;
				break;

			case '#':  // triple bond
				order = 3;
				++info->pos;
				break;
		}
	}
	return(order);
}

int read_symbol(struct smiles_parser *info)
{
	char s[4] = "\0\0\0\0";
	int Z = 0;

	if(info->pos[0]) {
		if(isupper(info->pos[0])) {
			s[0] = *info->pos;
			++info->pos;
			if(islower(*info->pos)) {
				s[1] = *info->pos;
				++info->pos;
			}
			Z = atomnum(s);
			if(Z < 0) {
				s[1] = 0;
				--info->pos;
				Z = atomnum(s);
			}
		} else  {// aromatic atom 
			s[0] = (char)toupper(info->pos[0]);
			++info->pos;
			Z = atomnum(s);
			if(Z > 0)
				Z += 1000;
		}
	}
	if(Z < 0) {
		info->error = EXPECTED_SYMBOL;
		info->pos -= strlen(s);
	}
	return(Z);
}

int read_number(struct smiles_parser *info)
{
	int n = 0;  // numbers are optional. zero is the default because it is not allowed to be a ring closing digit

	if(isdigit(*info->pos)){
		n = *info->pos - '0';
		++info->pos;
	} else if(*info->pos == '%') { // two-digit number
		++info->pos;
		if(isdigit(info->pos[0]) && isdigit(info->pos[1])) {
			n = 10 * (info->pos[0] - '0') + info->pos[1] - '0';
			info->pos += 2;
		} else {
			info->error = EXPECTED_NUMBER;
			++info->pos;
		}
	}
	return(n);
}

void read_numbers(struct smiles_parser *info, int n_atom)
{
	int n, order;

	do {
		order = read_bond(info);
		n = read_number(info);
		if(n > 0) {
			if(info->openrings[n].atom == -1) {  // "open ring"
				info->openrings[n].atom = n_atom;
				info->openrings[n].order = order;
			} else {						// close ring
				if((info->openrings[n].order && order) && (info->openrings[n].order != order)) {
					info->error = INCONSISTENT_BOND_ORDER;
					info->pos -= 2 + (n >= 10 ? 2 : 0);
					return;
				} else
					new_bond_ex(info->mol, n_atom, info->openrings[n].atom, 
						order ? order : (info->openrings[n].order ? info->openrings[n].order : 1) );
				
				info->openrings[n].atom = info->openrings[n].order = -1;
			}
		}
	} while (n > 0);

	if(order > 0)
		--info->pos;
}


int read_atom(struct smiles_parser *info, int prev)
{
	int bond, Z, n_atom;

	bond = read_bond(info);
	if(!bond)
		bond = 1;
	Z = read_symbol(info);
	if(!info->error) {
		n_atom = new_atom(info->mol, Z % 1000, 0, 0.0, 0.0, 0.0);
		if(Z / 1000)
			info->mol->atoms[n_atom].aromatic = TRUE;
		if(prev >= 0) {
			new_bond_ex(info->mol, prev, n_atom, bond);
		} else if(bond > 1) {
			info->error = INITIAL_BOND;
			info->pos -= 1 + strlen(symbol[Z % 1000]);
			return(n_atom);
		}
		read_numbers(info, n_atom);
	}

	return(n_atom);
}


void read_branch(struct smiles_parser *info, int prev)
{
	while(!info->error) {
		switch(*info->pos) {
			case '(':
				++info->pos;
				read_branch(info, prev);
				break;

			case ')':
				++info->pos;
				//fall through...
			case 0:
				return;
				break;

			default:
				prev = read_atom(info, prev);
				break;
		}
	}
}




struct mol *smiles2mol(char *smiles)
{
	struct smiles_parser *info;
	int first_atom = -1;
	struct mol *mol;
	static char *errmsg[MAXERR] = {"Expected symbol", "Expected integer",
		"Inconsistent bond order on ring closure", "Bond from nowhere" };
	
	info = malloc(sizeof(struct smiles_parser));
	assert(info);
	info->pos = info->smiles = smiles;
	info->error = 0;
	memset(info->openrings, -1, MAX_RINGS * sizeof(struct openring));
	mol = info->mol = new_mol();

	/* the parsing is done by the next two lines */
	first_atom = read_atom(info, -1);
	read_branch(info, first_atom);

	if(info->error) {
		char offstr[1000];
		int i;

		mol = destroy_mol(mol);
		for(i = 0; i < info->pos - info->smiles; ++i)
			offstr[i] = ' ';
		offstr[i] = '^';
		offstr[i+1] = 0;
		seterr("Error: %s\n%s\n%s\n", info->error <= MAXERR ? errmsg[info->error-1] : "?", info->smiles, offstr);
		destroy_mol(mol);
	} else {
		find_rings(mol);
		if(aromatize(mol) <= 0) {
			mol = destroy_mol(mol);
			seterr("Error: inconsistent aromaticity\n%s\n", info->smiles);
		}
	}

	free(info);
	return(mol);
}

char *smilescanon(char *smiles)
{
        struct mol *mol;
        char *ret = NULL;

        mol = smiles2mol(smiles);
        if(mol) {
                canonicalize(mol);
                ret = mol2smiles(mol);
                destroy_mol(mol);
        }

        return(ret);
}



