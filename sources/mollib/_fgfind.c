/* _fgfind.c
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

BOOL debuggrep = FALSE;
BOOL debugpaths = FALSE;

#define	NEIGHEMPTY	-1
#define NEIGHHIDRO	-2

/*
void generaformula(struct struct_info *info, struct mol *mol, int start);
BOOL grep(char *string, char *patt, BOOL start);
BOOL buscagrupo(struct mol *mol, char *formula, char *atomos, char *nombre, char *search, int id);
*/
int findgrp(struct mol *mol, int atom, struct funcgrpnode *gpofunc, int *atoms);
BOOL alreadyfound(struct mol *mol, int id, int numatoms, int *atoms);

void busca_funcionales(struct struct_info *info, struct mol *mol)
{
	int i,j,k;
	int numatoms;
	int atoms[50];
	extern char *symbol[];
	extern BOOL debug_mol;

	assert(mol);

	destroy_fg(mol);
	for (i = 0; i < mol->Natom; ++i) {
		for (j = 0; j < info->numvariables; ++j)
			if (info->variables[j].type == V_FUNCGRP) {
				memset(atoms, NEIGHEMPTY, sizeof(atoms));

				if ((numatoms = findgrp(mol, i, info->variables[j].data.fg.funcgrpnode,atoms)) > 0) {
					mol->gpofunc = realloc(mol->gpofunc, sizeof(struct gpofunc) * (mol->Nfunc+1));
					assert(mol->gpofunc);
					mol->gpofunc[mol->Nfunc].tipo = j;
					mol->gpofunc[mol->Nfunc].atomos = malloc(sizeof(int) * (numatoms+1));
					assert(mol->gpofunc[mol->Nfunc].atomos);
					memset(mol->gpofunc[mol->Nfunc].atomos,0,sizeof(int) * (numatoms+1));

					for (k = 0; k < numatoms; ++k) 
						mol->gpofunc[mol->Nfunc].atomos[numatoms-1-k] = atoms[k];
					mol->gpofunc[mol->Nfunc].atomos[k] = NEIGHEMPTY;
					mol->gpofunc[mol->Nfunc].repeat = alreadyfound(mol, j, numatoms, atoms);
					++mol->Nfunc;
				}
			}
	}

	/***/

// This block was used when testing fgfind and findpaths
	if (debugpaths) {
		char *log;
		struct paths *paths;
		int i,j,k,l;

		log = list_fgroups(info, mol);
		printf("%s", log);

		printf("\nPATHS\n");

		for (i = 0; i < mol->Nfunc; ++i)
			for (j = 0; j < mol->Nfunc; ++j)
				if (i != j) {
					printf("\nEntre %i y %i: \n", i,j);
					paths = findpaths(mol, mol->gpofunc[i].atomos[0], mol->gpofunc[j].atomos[0]);

					for (k = 0; k < paths->numpaths; ++k) {
						printf("> ");
						for (l = 0; l < paths->paths[k].len; ++l)
							printf("%i ", paths->paths[k].atoms[l] + 1);

						printf("\n");
					}

					freepaths(paths);
				}
	}

}


BOOL match_atom(struct atom *atom, struct funcgrpnodeatom *fgatom)
{
	BOOL ret = FALSE;

	if (fgatom->Z == atom->Z) {
		switch (fgatom->aromaticity) {
			case FGATOM_DONTCARE:
				ret = TRUE;
				break;
			case FGATOM_AROMATIC:
				ret = atom->aromatic;
				break;
			case FGATOM_ALIPHATIC:
				ret = !atom->aromatic;
				break;
		}

		ret &= (atom->charge == fgatom->charge);
	}

	return(ret);
}


BOOL match_bond(struct mol *mol, int lastatom, int nextatomidx, struct funcgrpnode *gpofunc)
{
	int i;
	struct atom *atom;

	atom = &mol->atoms[lastatom];

	if (nextatomidx < atom->Nbond) {
		/* es un vecino real */
		if ((gpofunc->order != -1) && (mol->bonds[atom->bonds[nextatomidx]].order != gpofunc->order))
			return(FALSE);

		for (i = 0;  i < sizeof(gpofunc->atoms)/sizeof(gpofunc->atoms[0]); ++i) {
			if (gpofunc->atoms[i].Z == 0)
				return(FALSE);

			if (match_atom(&mol->atoms[atom->neighbors[nextatomidx]], &gpofunc->atoms[i]))
				break;
		}

		if (i == sizeof(gpofunc->atoms)/sizeof(gpofunc->atoms[0]))
			return(FALSE);
	} else {
		/* es un vecino implicito = hidrogeno */
		for (i = 0;  i < sizeof(gpofunc->atoms)/sizeof(gpofunc->atoms[0]); ++i) {
			if (gpofunc->atoms[i].Z == 0)
				return(FALSE);

			if (gpofunc->atoms[i].Z == 1)
					break;
		}

		if (i == sizeof(gpofunc->atoms)/sizeof(gpofunc->atoms[0]))
			return(FALSE);
	}
	

	return(TRUE);
}



BOOL match_neighbors(struct mol *mol, int atom, struct funcgrpnode *gpofunc, int *lastneigh, int *atoms, int lastatom)
{
	int i,j;
	int neigh[MAXNEIGH];
	int localatoms[50];

	memset(localatoms, NEIGHEMPTY, sizeof(localatoms));

	/* checa si ya no hay condiciones -> return TRUE */
	for (i = 0; i < MAXNEIGH; ++i)
		if (gpofunc->children[i])
			break;
	if (i == MAXNEIGH)
		return(TRUE);

	if (!lastneigh) {
		/* create temporal neighbor list */
		memset(neigh, NEIGHEMPTY, sizeof(neigh));
		for (i = 0; i < min(MAXNEIGH, mol->atoms[atom].Nbond); ++i)
			if (mol->atoms[atom].neighbors[i] != lastatom)
				neigh[i] = mol->atoms[atom].neighbors[i];
		for (j = 0; (j < mol->atoms[atom].nH) && (i+j < MAXNEIGH); ++j)
			neigh[i+j] = NEIGHHIDRO;
	} else 
		memmove(neigh, lastneigh, sizeof(neigh));

	/* match */
	for (i = 0; (i < MAXNEIGH); ++i) 
		if (gpofunc->children[i])
			for (j = 0; j < MAXNEIGH; ++j)
				if (neigh[j] != NEIGHEMPTY)
					if (match_bond(mol, atom, j, gpofunc->children[i]) && mol->Nbond && (match_neighbors(mol, mol->atoms[atom].neighbors[j], gpofunc->children[i], NULL, atoms, atom))) {
						int tempneig;
						struct funcgrpnode *tempcond = NULL;
						BOOL ret = -1;
						int k;

						tempneig = neigh[j];
						neigh[j] = NEIGHEMPTY;

						for (k = 0; k < sizeof(neigh)/sizeof(neigh[0]); ++k)
							if (neigh[k] != NEIGHEMPTY)
								break;

						--gpofunc->children[i]->max;
						--gpofunc->children[i]->min;
						if (gpofunc->children[i]->max == 0) {
							tempcond = gpofunc->children[i];
							gpofunc->children[i] = NULL;
						}

						if (k == sizeof(neigh)/sizeof(neigh[0])) {
							/* se acabaron los vecinos! */
							for (k = 0; k < MAXNEIGH; ++k)
								if ((gpofunc->children[k]) && (gpofunc->children[k]->min > 0)) {
									/* no se cumplio una condicion -> FALSE */
									ret = FALSE;
									break;
								}

							/* se cumplieron todas las condiciones */
							if (ret == -1)
								ret = TRUE;
						} else {
							ret = match_neighbors(mol, atom, gpofunc, neigh, localatoms, atom);
						}

						if (gpofunc->children[i] == NULL) 
							gpofunc->children[i] = tempcond;
						++gpofunc->children[i]->max;
						++gpofunc->children[i]->min;
						neigh[j] = tempneig;

						if (ret) {
							if (gpofunc->children[i]->structural) {
								int l;

								for (k = 0; localatoms[k] != NEIGHEMPTY; ++k)
									;
								for (l = 0; atoms[l] != NEIGHEMPTY; ++l, ++k)
									localatoms[k] = atoms[l];
								localatoms[k++] = tempneig;

								memmove(atoms,localatoms,sizeof(localatoms));
							}
							return(TRUE);
						}
					}

	return(FALSE);
}


int findgrp(struct mol *mol, int atom, struct funcgrpnode *gpofunc, int *atoms)
{
	if (match_atom(&mol->atoms[atom], &gpofunc->atoms[0])) {
		/* match root */
		if (match_neighbors(mol, atom, gpofunc, NULL, atoms, -1)) {
			int i;
			/* Functional group match!   */
//			printf("Match!!!!!\n");
			for (i = 0; atoms[i] != -1; ++i)
				;
			atoms[i] = atom; 

			return(i+1);
		}
	}

	return(0);
}























#ifdef COMM

void dfs(struct struct_info *info, struct mol *mol, int pos, char *result)
{
	int i;
	int used = 0;
	struct bond *bond;

	result[strlen(result)] = (char)(pos+1);

	for (i = 0; i < mol->atoms[pos].Nbond; ++i) {
		bond = &mol->bonds[mol->atoms[pos].bonds[i]];

		/* checa que el atomo que esta del otro lado del enlace no forme ciclo */
		if (!strchr(result, (pos == bond->a1) ? bond->a2+1 : bond->a1+1)) {
			/* si no hay ciclo, lo recorremos */
			used = 1;

			dfs(info, mol, (pos == bond->a1) ? bond->a2 : bond->a1, result);
		}
	}

	if (!used) {
		/* llego a un punto terminal */
		char *res;
		char formula[200];
		char atomos[200];
		int ofs = 0;
		int j;
		extern char *symbol[];
		extern BOOL debug_mol;

		memset(formula, 0, sizeof(formula));
		memset(atomos, -1, sizeof(atomos));

		j = 0;
		for (res = result; *res; ++res) {
			assert(ofs < sizeof(formula));

			atomos[j++] = (char)(*res - 1);
			ofs += sprintf(formula + ofs, "%s(", symbol[mol->atoms[*res - 1].Z]);
			for (i = 0; i < mol->atoms[*res - 1].nH; ++i)
				ofs += sprintf(formula + ofs, "H");
			for (i = 0; i < mol->atoms[*res - 1].Nbond; ++i) {
				bond = &mol->bonds[mol->atoms[*res - 1].bonds[i]];

				//if (!strchr(result, (*res - 1 == bond->a1) ? bond->a2+1 : bond->a1+1)) 
				ofs += sprintf(formula + ofs, "%s",symbol[mol->atoms[(*res - 1 == bond->a1) ? bond->a2 : bond->a1].Z]);
			}
			ofs += sprintf(formula + ofs, ") ");
		}
#ifdef _DEBUG
		if (debug_mol)
			printf("%s",formula);
#endif

		for (i = 0; i < info->numvariables; ++i)
			if (info->variables[i].order == V_FUNCGRP)
				buscagrupo(mol, formula, atomos, info->variables[i].name, info->variables[i].data.fg.funcgrp, i);

#ifdef _DEBUG
		if (debug_mol)
			printf("\n");
#endif
	}

	result[strlen(result)-1] = 0;
}



BOOL buscagrupo(struct mol *mol, char *formula, char *atomos, char *nombre, char *search, int id)
{
	int found;
	int numatoms, i, j;
	char *c;
	BOOL yaexiste;
	extern BOOL debug_mol;

	grep(formula,search,TRUE);
	while ((found = grep(formula, search,FALSE)) != FALSE) {
		for (numatoms = 0, c = search; *c; ++c)
			if (*c == '(')
				++numatoms;

#ifdef _DEBUG
		if (debug_mol) {
			printf("= %s %i (", nombre, found);

			for (i = found-1; i < found-1+numatoms; ++i) 
				printf("%i ", atomos[i]);
			printf(")");
		}
#endif


		/* checks if the fg has already been found */
		for (i = 0, yaexiste = FALSE; (i < mol->Nfunc) && !yaexiste; ++i) {
			if (mol->gpofunc[i].tipo == id) {
				for (j = 0; j < numatoms; ++j)
					if (atomos[found-1+j] != mol->gpofunc[i].atomos[j])
						break;

				if (j == numatoms)
					yaexiste = TRUE;
			}
		}

		if (!yaexiste) {
			mol->gpofunc = realloc(mol->gpofunc, sizeof(struct gpofunc) * (mol->Nfunc+1));
			assert(mol->gpofunc);
			mol->gpofunc[mol->Nfunc].tipo = id;
			mol->gpofunc[mol->Nfunc].atomos = malloc(sizeof(int) * (numatoms+1));
			assert(mol->gpofunc[mol->Nfunc].atomos);
			memset(mol->gpofunc[mol->Nfunc].atomos,0,sizeof(int) * (numatoms+1));

			for (i = 0; i < numatoms; ++i) 
				mol->gpofunc[mol->Nfunc].atomos[i] = atomos[i+found-1];
			mol->gpofunc[mol->Nfunc].atomos[i] = -1;
			++mol->Nfunc;
		}
	}

	return(found);
}


int grepatoms(char *s)
{
	int atoms = 0;
	char *aux;

	//"C(O[CH]^(2-3)) O(CO) O(OH)"

	for (aux = s; aux && *aux; aux = strchr(aux, ' '), ++atoms) {
		while (*aux == ' ')
			++aux;
		if (!*aux)
			break;
	}

	return(atoms);
}

char *grepnext(char *s)
{
	s = strchr(s, ' ');
	while (s && (*s == ' '))
		++s;
	return(s);
}

BOOL grepsymmatch(char *s1, char *s2)
{
	while (s1 && s2 && (*s1 == *s2) && (*s1 != '(')) {
		++s1; ++s2;
	}

	return(s1 && s2 && (*s1 == *s2));
}

char *grepfind(char *string, char *srch)
{
	char *ret = NULL;

	while (!ret && *srch) {
		ret = strchr(string, *srch);
		++srch;
	}

	return(ret);
}


BOOL grep(char *mol, char *patt, BOOL start)
{
	int pattc;
	int molmax, pattmax;
	char *pattaux;
	char *molaux2;
	char molvecino[30], pattvecino[30], *vecino, *aux;
	char var[30];
	BOOL found = FALSE;

	static int molc;
	static char *molaux;

	if (start) {
		molc = 0; molaux = mol;

		if (debuggrep)
			printf("\nGrep \"%s\", molmax=%i, pattmax=%i", patt, molmax,pattmax);

		return(TRUE);
	}

	molmax = grepatoms(mol);
	pattmax = grepatoms(patt);

	for (; (molc < molmax) && !found; ++molc, molaux = grepnext(molaux)) {
		for (pattc = 0, pattaux = patt, molaux2 = molaux; pattc < pattmax; ++pattc, pattaux = grepnext(pattaux), molaux2 = grepnext(molaux2)) {
			if (grepsymmatch(molaux2, pattaux)) {
				strncpy(molvecino, strchr(molaux2, '(')+1, sizeof(molvecino));
				if ((aux = strchr(molvecino, ')')) != NULL)
					*aux = 0;
				strncpy(pattvecino, strchr(pattaux, '(')+1, sizeof(pattvecino));
				if ((aux = strchr(pattvecino, ')')) != NULL)
					*aux = 0;

				for (vecino = pattvecino; *vecino; ++vecino) {
					if (*vecino == '[') {
						/* variable */
						++vecino;
						strncpy(var, vecino, sizeof(var));
						if ((aux = strchr(var, ']')) != NULL)
							*aux = 0;
						vecino = strchr(vecino, ']');
						assert(vecino);
						++vecino;

						if (*vecino == '*') {
							BOOL match = TRUE;
							while (match)
								if ((aux = grepfind(molvecino, var)) != NULL)
									*aux = ' ';
								else
									match = FALSE;
						} else if (*vecino == '^') {
							int matchmin, matchmax;
							int i;

							++vecino;
							assert(isdigit(*vecino) || (*vecino == '{'));

							if (isdigit(*vecino)) {
								matchmin = matchmax = atoi(vecino);
								while (isdigit(*vecino))
									++vecino;
								--vecino;
							} else if (*vecino == '{') {
								++vecino;
								matchmin = atoi(vecino);
								vecino = strchr(vecino, '-');
								assert(vecino);
								++vecino;
								matchmax = atoi(vecino);
								vecino = strchr(vecino, '}');
								assert(vecino);
							}

							for (i = 0; i < matchmin; ++i) {
								if ((aux = grepfind(molvecino, var)) != NULL)
									*aux = ' ';
								else
									break;
							}

							if (i != matchmin)
								break;

							for (i = matchmin; i < matchmax; ++i) {
								if ((aux = grepfind(molvecino, var)) != NULL)
									*aux = ' ';
								else
									break;
							}
						
						} else {
							--vecino; 

							/* match one */
							if ((aux = grepfind(molvecino, var)) != NULL)
								*aux = ' ';
							else
								break;
						}
					} else {
						/* fijo! */
						if ((aux = strchr(molvecino, *vecino)) != NULL)
							*aux = ' ';
						else
							break;
					}
				}

				if (*vecino)
					break;

				for (aux = molvecino; *aux; ++aux)
					if (*aux != ' ')
						break;
				if (*aux)
					break;
			} else
				break;
		}
		if (pattc == pattmax)
			found = TRUE;
	}


	return(found ? molc : FALSE);



/*
	ALGORITMO BRUTE FORCE
	mol es la molecula real
	grep es el pattern que buscamos

	for (molc = 0; molc < atomos(mol); ++molc) {
		for (grepc = 0; grepc < atomos(grep); ++grepc) {
			if (sym(mol[molc+grepc]) == sym(grep[grepc])) {
				temp = vecinos(mol[molc+grepc]);
				while (aux = tok(grep[grepc])) {
					if (aux es fijo) {
						if res=strchr(temp, aux)
							*res = ' '
						else
							break;
					} else (aux es variable) {
						if (tipo(aux) == '*') {
							match = true;
							while (match)
								if res=find(temp, aux))
									*res = ' ';
								else
									match = false;
						} else if (tipo(aux) == '^') {
							for (i = 0; i < minpot; ++i)
								if res=find(temp,aux)
									*res = ' ';
								else
									break;
							for (i = minpot; i < maxpot; ++i)
								if res =find(temp,aux)
									*res = ' ';
						} else {
							if res=find(temp,aux)
								*res = ' ';
							else
								break;
						}
					}
				}

				if (temp == '        ')
					FOUND
				else
					NOTFOUND;
			} else
				break;
		}
	}

*/






/*
	for (i = 0; (i < lens) && !found; ++i) {
		j = 0;
		k = i;

		if (string[i] == '(')
			++atom_ofs;

		while (patt[j]) {
			if (patt[j] == '[') {
				++j;
				strncpy(lista, &patt[j], sizeof(lista));
				if ((aux = strchr(lista, ']')) != NULL)
					*aux = 0;

				while (strchr(lista, string[k]))
					++k;

				j += strlen(lista)+1;
			} else if (string[k] == patt[j]) {
				++k; ++j;
			} else
				break;
		}

		if (!patt[j]) 
			found = TRUE;
	}	

	return(found ? atom_ofs + 1 : FALSE);
*/

	return(FALSE);
}

void generaformula(struct struct_info *info, struct mol *mol, int start)
{
	char result[300];

	memset(result, 0, sizeof(result));
	dfs(info, mol, start, result);
}


#endif

char *list_fgroups(struct struct_info *info, struct mol *mol)
{
	struct struct_log *buf;
	int i,j;
	char s[100], s2[20], *p;

	buf = init_log(128);

	for(i = 0; i < mol->Nfunc; ++i) {
		if (!mol->gpofunc[i].repeat) {
			sprintf(s, "%s (", info->variables[mol->gpofunc[i].tipo].name);
			for(j = 0; mol->gpofunc[i].atomos[j] != -1; ++j){
				sprintf(s2, "%i ", mol->gpofunc[i].atomos[j]+1);
				strcat(s, s2);
			}
			s[strlen(s)-1] = 0;
			strcat(s, ")\n");
			//_strlwr(s);
			p = s;
			while(*p){
				*p = (char)tolower(*p);
				++p;
			}
			logadd(buf, "%s", s);
		}
	}

	return(log2str(buf));
}





BOOL alreadyfound(struct mol *mol, int id, int numatoms, int *atoms)
{
	BOOL yaexiste;
	int i,j,k;
	int match;

	/* checks if the fg has already been found */
	for (i = 0, yaexiste = FALSE; (i < mol->Nfunc) && !yaexiste; ++i) {
		if (mol->gpofunc[i].tipo == id) {
			match = 0;
			for (j = 0; j < numatoms; ++j)
				for (k = j; k < numatoms; ++k)
					if (atoms[j] == mol->gpofunc[i].atomos[k])
						++match;

			if (match == numatoms)
				yaexiste = TRUE;
		}
	}

	return(yaexiste);
}
