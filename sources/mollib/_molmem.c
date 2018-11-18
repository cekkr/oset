/* _molmem.c
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

int _destroy_atom(struct atom *atom);
int _destroy_gpofunc(struct gpofunc *gpofunc);


/*	frees the internal structures of an atom. Does not free the atom itself */
int _destroy_atom(struct atom *atom)
{
	free(atom->bonds);
	free(atom->neighbors);
	return(0);
}

/*	frees the internal structures of one functional group instance
 *	in a molecule*/
int _destroy_gpofunc(struct gpofunc *gpofunc)
{
	free(gpofunc->atomos);
	return(0);
}

/*	destroys the ring list in a molecule */
void destroy_rings(struct mol *mol)
{
	int i;

	for(i = 0; i < mol->Nring; ++i) 
		free_ringpath(&mol->rings[i]);

	free(mol->rings);
	mol->rings = NULL;
	mol->Nring =0;
}

/*	frees the functional group list in a molecule */
void destroy_fg(struct mol *mol)
{
	int i;
	
	for(i=0; i< mol->Nfunc; ++i)
		_destroy_gpofunc(&mol->gpofunc[i]);
	free(mol->gpofunc);
	mol->gpofunc = NULL;
	mol->Nfunc = 0;

}

/* frees a molecule and all its substructures returns NULL for convenience*/
struct mol *destroy_mol(struct mol *mol)
{
	int i;

	if(mol) {
		for(i=0; i< mol->Nfunc; ++i)
			_destroy_gpofunc(&mol->gpofunc[i]);
		for(i=0; i < mol->Natom; ++i)
			_destroy_atom(&mol->atoms[i]);
		free(mol->bonds);
		free(mol->atoms);
		free(mol->gpofunc);
		destroy_rings(mol);

		if (mol->smiles)
			free(mol->smiles);
		free(mol);
	}
	return(NULL);
}

/*	destroys all internal structures of a molecule and sets all values to zero,
 *	without freeing the molecule */
void reset_mol(struct mol *mol)
{
	int i;

	if(mol) {
		for(i=0; i< mol->Nfunc; ++i)
			_destroy_gpofunc(&mol->gpofunc[i]);
		for(i=0; i < mol->Natom; ++i)
			_destroy_atom(&mol->atoms[i]);
		if(mol->bonds)
			free(mol->bonds);
		if(mol->atoms)
			free(mol->atoms);
		if(mol->gpofunc)
			free(mol->gpofunc);
		destroy_rings(mol);
		if (mol->smiles)
			free(mol->smiles);
		memset(mol, 0, sizeof(struct mol));
	}

}

struct mol *new_mol()
{
	struct mol *mol;
	mol = calloc(1, sizeof(struct mol));
	assert(mol);
	return(mol);
}

struct mol_list *new_mol_list(struct mol *mol)
{
	struct mol_list *list;

	list = calloc(1, sizeof(struct mol_list));
	assert(list);
	if(mol)
		append_mol(list, mol);
	return(list);
}

struct mol_metalist *new_mol_metalist(struct mol_list *list)
{
	struct mol_metalist *metalist;

	metalist = calloc(1, sizeof(struct mol_metalist));
	assert(metalist);
	if(list)
		append_mol_list(metalist, list);
	return(metalist);


}


/*	destroys a molecule list and all of the listed molecules */
struct mol_list *destroy_mol_list(struct mol_list *list)
{
	int i;

	if(list) {
		for(i = 0; i < list->Nmol; ++i) 
			destroy_mol(list->mols[i]);
		free(list->rxn_info);
		free(list);
	}
	return(NULL);
}

/*	destroys all of the molecules and reaction information in a mol_list
 *	but does not free the list itself. Sets all internal values to zero*/	
void reset_mol_list(struct mol_list *list)
{
	int i;

	if(list) {
		for(i = 0; i < list->Nmol; ++i) 
			destroy_mol(list->mols[i]);
		free(list->rxn_info);
		memset(list, 0, sizeof(struct mol_list));
	}
}

/* destroys a mol_list, but it does not destroy the molecules in the list.
 * Use it only if you already have copies of every molecule elsewhere! */
struct mol_list *disband_mol_list(struct mol_list *list)
{
	if(list) {
		free(list->rxn_info);
                free(list->mols);
		free(list);
	}
	return(NULL);
}

/* destroys a mol_metalist, but it does not destroy the molecules in the list.
 * Use it only if you already have copies of every mol_list elsewhere! */
struct mol_list *disband_mol_metalist(struct mol_metalist *mml)
{
        if(mml) {
                free(mml->lists);
                free(mml);
	}
	return(NULL);
}



/* Deletes a molecule from a mol_list. The molecule is destroyed */
void delete_mol(struct mol_list *list, int n_mol)
{
	if(list->mols[n_mol]) {
		destroy_mol(list->mols[n_mol]);
		--list->Nmol;
		if(n_mol < list->Nmol)  //compress list
			list->mols[n_mol] = list->mols[list->Nmol];
		list->mols = realloc(list->mols, sizeof(struct mol*) * list->Nmol);
		assert(list->mols || (list->Nmol == 0));
	}
		
}

/* appends mol at the end of list. Returns list */
struct mol_list *append_mol(struct mol_list *list, struct mol *mol)
{
	++list->Nmol;
	list->mols = realloc(list->mols, list->Nmol * sizeof(struct mol*));
	assert(list->mols);
	list->mols[list->Nmol-1] = mol;
	return(list);
}

struct mol_metalist *append_mol_list(struct mol_metalist *mml, struct mol_list *list)
{
	++mml->Nlist;
	mml->lists = realloc(mml->lists, mml->Nlist * sizeof(struct mol_list*));
	assert(mml->lists);
	mml->lists[mml->Nlist-1] = list;
	return(mml);
}



/* appends list2 after list1. List 2 is not affected. Returns list1 */
struct mol_list *mol_list_cat(struct mol_list *list1, struct mol_list *list2)
{
	int i;

	for(i = 0; i < list2->Nmol; ++i) {
		append_mol(list1, list2->mols[i]);
	}

	return(list1);
}

struct mol_metalist *mol_metalist_cat(struct mol_metalist *ml1, struct mol_metalist *ml2)
{
	int i;

	for(i = 0; i < ml2->Nlist; ++i) {
                append_mol_list(ml1, ml2->lists[i]);
	}

	return(ml1);
}


/* destroys list n_list in mml */
struct mol_metalist *delete_mol_list(struct mol_metalist *mml, int n_list)
{

	if(mml->lists[n_list]) {
		destroy_mol_list(mml->lists[n_list]);
		--mml->Nlist;
/*		if(n_list < mml->Nlist)  //compress metalist
			mml->lists[n_list] = mml->lists[mml->Nlist];
*/		memmove(&mml->lists[n_list], &mml->lists[n_list+1], (mml->Nlist- n_list) *sizeof(struct mol_list*));
		mml->lists = realloc(mml->lists, sizeof(struct mol_list*) * mml->Nlist);
		assert(mml->lists || (mml->Nlist == 0));
	}
	return(mml);
}


/*	destroys a molecule metalist and all of the listed molecules */
struct mol_metalist *destroy_mol_metalist(struct mol_metalist *metalist)
{
	int i;

	if(metalist) {
		for(i = 0; i < metalist->Nlist; ++i) 
			destroy_mol_list(metalist->lists[i]);
		free(metalist);
	}
	return(NULL);
}



/*	Returns a copy of a molecule */
struct mol *moldup(struct mol *mol)
{
	struct mol *temp;
	int i, j;
	struct atom *atomsrc, *atomdest;
	struct gpofunc *funcsrc, *funcdest;

	temp = malloc(sizeof(struct mol));
	assert(temp);
	memmove(temp, mol, sizeof(struct mol));

	temp->atoms = malloc(mol->Natom * sizeof(struct atom));
	assert(temp->atoms);
	temp->bonds = malloc(mol->Nbond * sizeof(struct bond));
	assert(temp->bonds);
	temp->gpofunc = malloc(mol->Nfunc * sizeof(struct gpofunc));
	assert(temp->gpofunc);
	temp->rings = malloc(mol->Nring * sizeof(struct ringpath));
	assert(temp->rings);
	if(mol->smiles)
		temp->smiles = strdup(mol->smiles);

	for(i = 0; i < mol->Natom; ++i){
		atomsrc = &mol->atoms[i];
		atomdest = &temp->atoms[i];
		memmove(atomdest, atomsrc, sizeof(struct atom));
		atomdest->bonds = malloc(atomsrc->Nbond * sizeof(int));
		assert(atomdest->bonds);
		memmove(atomdest->bonds, atomsrc->bonds, atomsrc->Nbond * sizeof(int));
		atomdest->neighbors = malloc(atomsrc->Nbond * sizeof(int));
		assert(atomdest->neighbors);
		memmove(atomdest->neighbors, atomsrc->neighbors, atomsrc->Nbond * sizeof(int));

	}

	memmove(temp->bonds, mol->bonds, sizeof(struct bond) * mol->Nbond);

	for(i = 0; i < mol->Nfunc; ++i){
		funcsrc = &mol->gpofunc[i];
		funcdest = &temp->gpofunc[i];
		memmove(funcdest, funcsrc, sizeof(struct gpofunc));
		
		for(j = 0; funcsrc->atomos[j] != -1; ++j)
			;
		funcdest->atomos = malloc((j+1)*sizeof(int));
		assert(funcdest->atomos);
		memmove(funcdest->atomos, funcsrc->atomos, (j+1)* sizeof(int));
	}

	for (i = 0; i < mol->Nring; ++i) 
		dup_ringpath(mol, &temp->rings[i], &mol->rings[i]);
	
	return(temp);
}

/* returns a "clone" of a mol_list, copying all of the molecules.
 * NOTE: Does not copy rxn info */
struct mol_list *mol_listdup(struct mol_list *list)
{
	struct mol_list *newlist = new_mol_list(NULL);
	int i;

	for(i = 0; i < list->Nmol; ++i)
		append_mol(newlist, moldup(list->mols[i]));

	return(newlist);
}


/* Returns a copy of a mol_list. Does not copy the molecules, only the
 * references */
struct mol_list *mol_listcpy(struct mol_list *list)
{
	struct mol_list *newlist = new_mol_list(NULL);
	int i;

	for(i = 0; i < list->Nmol; ++i)
		append_mol(newlist, list->mols[i]);

	return(newlist);
}
