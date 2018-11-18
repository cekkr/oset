/* _canon.c
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

/* This canonicalization algorithm is based on:
 * Weininger, et. al., J. Chem. Inf. Comp. Sci. 29[2], 97-101 (1989)
 */

#include "mollib.h"

int primes[MAXATOM];

struct struct_rank {
	int nclass, oldclass;
	int key;
};

int atom_invariants(struct mol *mol, int n_atom);
static int compare(const void *elem1, const void *elem2);
int rank_classes(struct mol *mol, struct struct_rank **rank);
int _canon(struct mol *mol, struct struct_rank *v, struct struct_rank **rank, int nclasses);


void init_primes()
{
	int i, n;
	for(i=0, n=2; i < MAXATOM; ++n){
		int j;
		for(j = 0; j < i; ++j)
			if((n % primes[j]) == 0) 
				break;
		if(j == i) {
			primes[i] = n;
			++i;
		}
	}
}



struct mol *canonicalize(struct mol *mol)
{
	struct struct_rank *v, **rank;  //"atomic vector", rank vector
	int i, nclasses, oldnclasses = 0;

	v = calloc(mol->Natom, sizeof(struct struct_rank));
	assert(v);
	rank = calloc(mol->Natom, sizeof(struct struct_rank *));
	assert(rank);

	for(i = 0; i < mol->Natom; ++i) {  // set initial invariants
		v[i].key = atom_invariants(mol, i);
		rank[i] = &v[i];
	}	

	nclasses = rank_classes(mol, rank);
    nclasses = _canon(mol, v, rank, nclasses);

    for(i = 0; i < mol->Natom; ++i) {
          mol->atoms[i].nclass = v[i].nclass;
          /* printf("%4i%4i%8i\n", i+1, v[i].nclass, v[i].key); */
    }

    while(nclasses < mol->Natom) {
    	int tie = -1;
    	for(i = 0; i < mol->Natom; ++i) {   // set nclass to twice the original nclass
        	if((tie < 0) && (rank[i]->nclass == rank[i+1]->nclass)){
             	tie = i;
                rank[i]->nclass = rank[i]->oldclass = rank[i]->nclass * 2 - 1 ; //except for the first tie
            } else
	         	 rank[i]->oldclass = rank[i]->nclass *= 2;
        }
		nclasses = _canon(mol, v, rank, nclasses);
 /*		for(i = 0; i < mol->Natom; ++i)
    		printf("%4i%4i%8i\n", i+1, v[i].nclass, v[i].key); */
    }
//        printf("\n");
		for(i = 0; i < mol->Natom; ++i){
/*   		printf("%4i%4i%8i\n", i+1, v[i].nclass, v[i].key);  */
			mol->atoms[i].canon_num = v[i].nclass;
        }



	free(v);
	free(rank);
	return(mol);
}


int atom_invariants(struct mol *mol, int n_atom)
{
	struct atom *atom = &mol->atoms[n_atom];
	int ret, i;

	ret = atom->Nbond * 10000000; //10^7
	for(i = 0; i < atom->Nbond; ++i)
		ret += mol->bonds[atom->bonds[i]].order * 100000; // 10^5
	ret += atom->Z * 1000;
	if(atom->charge > 0)
		ret += 100;
	else if(atom->charge < 0)
		ret += 200;
	ret += 10 * abs(atom->charge) + atom->nH;

	return(ret);
}


int _canon(struct mol *mol, struct struct_rank *v, struct struct_rank **rank, int nclasses)
{
        int i, j, oldnclasses = 0;


	while((nclasses > oldnclasses) && (nclasses < mol->Natom)){
		oldnclasses = nclasses;

		for(i = 0; i < mol->Natom; ++i){  //set key to product of neighbors' primes
			struct atom *atom = &mol->atoms[i];
			for(j = 0, v[i].key = 1; j < atom->Nbond; ++j)
				v[i].key *= primes[v[atom->neighbors[j]].nclass-1];
		}

		nclasses = rank_classes(mol, rank);
	}
        return(nclasses);
}


int rank_classes(struct mol *mol, struct struct_rank **rank)
{
	int i, nclasses = 0;

	if(mol->Natom > 0) {
		qsort(rank, mol->Natom, sizeof(struct struct_rank *), compare);
		rank[0]->nclass = nclasses = 1;
		for (i = 1; i < mol->Natom; ++i){
			if(compare(&rank[i], &rank[i-1]) != 0)
				++nclasses;
			rank[i]->nclass = nclasses;
		}
		for (i = 0; i < mol->Natom; ++i)
			rank[i]->oldclass = rank[i]->nclass;

	}
	return(nclasses);
}



static int compare(const void *elem1, const void *elem2)
{
	struct struct_rank *e1 = *((struct struct_rank **)elem1);
	struct struct_rank *e2 = *((struct struct_rank **)elem2);
	
    if(e1->oldclass == e2->oldclass) {
		if(e1->key < e2->key)
			return(-1);
		else if(e1->key > e2->key)
			return(1);
		else
			return(0);
    } else
		return((e1->oldclass < e2->oldclass)? -1 : 1);
}


char *list_eq_classes(struct mol *mol)
{
	struct struct_log *log = init_log(64);
	int i, j;
	BOOL *used, classes = FALSE;
	char s[1000], s2[10], *ret;
	
	used = calloc(mol->Natom + 1, sizeof(int));
	assert(used);

	for(i = 0; i < mol->Natom; ++i){
		int curr_class = mol->atoms[i].nclass;
		BOOL logclass = FALSE;
		if(!used[curr_class]) {
			used[curr_class] = TRUE;
			sprintf(s, "%i", i + 1);
			for(j = i+1; j < mol->Natom; ++j){
				if(mol->atoms[j].nclass == curr_class) {
					sprintf(s2, " = %i", j + 1);
					strcat(s, s2);
					logclass = TRUE;
					classes = TRUE;
				}
			}
			strcat(s, "\n");
			if(logclass)
				logadd(log, s);
		}
	}

	if(classes)
		ret = strdup(log->buffer);
	else
		ret = strdup("All atoms are non-equivalent");

	destroy_log(log);
	free(used);

	return(ret);
}
