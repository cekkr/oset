/* _moltopo.c
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
#include <math.h>


int _destroy_atom(struct atom *atom);
int _destroy_gpofunc(struct gpofunc *gpofunc);
void _changebondinatom(struct mol * mol, int na, int nb_old, int nb_new);
void _removebondfromatom(struct mol *mol, int na, int nb);
void _addbondtoatom(struct mol *mol, int na, int nb);
double bondlength(struct mol *mol, int n_bond);


/*	marks all of the atoms in a connected graph (usually called "molecule")
 *	with number, starting at atom n_atom. The marks are stored in *used, 
 *	which must be an array of at least Natom integers. Does not alter *mol.
 *	This function is used by separate_mols. */	
void _markatoms(struct mol *mol, int *used, int n_atom, int number)
{
	struct atom *atom = &mol->atoms[n_atom];
	int i;

	used[n_atom] = number;
	
	for(i = 0; i < atom->Nbond; ++i){
		if(used[atom->neighbors[i]] == 0)
			_markatoms(mol, used, atom->neighbors[i], number);
	}
		
}

/*	returns the number of discrete fragments in *mol.
 *	if *mol_list is NULL, the function only counts the fragments
 *	if not NULL, the separated fragments are stored in *mol_list
 *	NOTE: if *mol_list contains molecules, they are destroyed and replaced!
 *	*mol is not affected. */
int separate_mols(struct mol *mol, struct mol_list *mol_list)
{
	int n = 0, i, j, k;
	int *used, *atomcorrlist, *bondcorrlist; 
	struct mol *newmol; 	
	struct atom *atomsrc, *atomdest;
	struct bond *bondsrc, *bonddest;

	used = calloc(mol->Natom, sizeof(int));
	assert(used);

	for(i = 0; i < mol->Natom; ++i)
		if(used[i] == 0)
			_markatoms(mol, used, i, ++n);

	if(mol_list) { // separate fragments only if a list is given
		if(mol_list->Nmol)
			reset_mol_list(mol_list);

		mol_list->Nmol = n;
		mol_list->mols = malloc(n * sizeof(struct mol *));
		assert(mol_list->mols);
		atomcorrlist = calloc(mol->Natom, sizeof(int));
		assert(atomcorrlist);
		bondcorrlist = calloc(mol->Nbond, sizeof(int));
		assert(bondcorrlist);
		for(i = 0; i < n; ++i) { //i = fragment number
			newmol = mol_list->mols[i] = calloc(1, sizeof(struct mol));
			assert(newmol);
			newmol->atoms = calloc(mol->Natom, sizeof(struct atom));
			assert(newmol->atoms);
			newmol->bonds = malloc(mol->Nbond * sizeof(struct bond));
			assert(newmol->bonds);

			// copy atoms
			for(j = 0; j < mol->Natom; ++j) {
				if(used[j] == i+1){
					atomcorrlist[j] = newmol->Natom;
					atomsrc = &mol->atoms[j];
					atomdest = &newmol->atoms[newmol->Natom];
					memmove(atomdest, atomsrc, sizeof(struct atom));
					atomdest->bonds = malloc(atomsrc->Nbond * sizeof(int));
					assert(atomdest->bonds);
					atomdest->neighbors = malloc(atomsrc->Nbond * sizeof(int));
					assert(atomdest->neighbors);
					++newmol->Natom;
				}
			}

			// copy bonds
			for(j = 0; j < mol->Nbond; ++j) {
				bondsrc = &mol->bonds[j];
				if(used[bondsrc->a1] == i+1) {
					bondcorrlist[j] = newmol->Nbond;
					bonddest = &newmol->bonds[newmol->Nbond];
					bonddest->a1 = atomcorrlist[bondsrc->a1];
					bonddest->a2 = atomcorrlist[bondsrc->a2];
					bonddest->order = bondsrc->order;
					++newmol->Nbond;
				}
			}

			// build each atom's bond and neighbor list
			for(j = 0; j < mol->Natom; ++j) {
				if(used[j] == i+1){
					atomsrc = &mol->atoms[j];
					atomdest = &newmol->atoms[atomcorrlist[j]];
					for(k = 0; k < atomsrc->Nbond; ++k) {
						atomdest->bonds[k] = bondcorrlist[atomsrc->bonds[k]];
						atomdest->neighbors[k] = atomcorrlist[atomsrc->neighbors[k]];
					}
				}
			}
			// resize atom and bond lists to the actual size for the fragments
			newmol->atoms = realloc(newmol->atoms, newmol->Natom * sizeof(struct atom));
			assert(newmol->atoms);
			newmol->bonds = realloc(newmol->bonds, newmol->Nbond * sizeof(struct bond));
			assert((newmol->bonds)||(newmol->Nbond == 0));
		}
		free(bondcorrlist);
		free(atomcorrlist);
	}


	free(used);
	return(n);
}
	
/*	Combines two molecules, returning a new one. mol1 and mol2 are not affected
 *	dx and dy are added to the coordinates of mol2. This can be used to avoid
 *	overlaps.  This function is the inverse operation of separate_mols()*/
struct mol *combine_mols(struct mol *mol1, struct mol *mol2, double dx, double dy)
{
	int i, j;
	struct mol *newmol = NULL; 	
	struct atom *atomsrc, *atomdest;


	if(mol1 && mol2) {
		newmol = moldup(mol1);

		newmol->Natom += mol2->Natom;
		newmol->atoms = realloc(newmol->atoms, newmol->Natom * sizeof(struct atom));
		assert(newmol->atoms);
		newmol->Nbond += mol2->Nbond;
		newmol->bonds = realloc(newmol->bonds, newmol->Nbond * sizeof(struct bond));
		assert(newmol->bonds);

		for(i = 0; i < mol2->Natom; ++i){ //copy atoms from mol2 to newmol
			atomsrc = &mol2->atoms[i];
			atomdest = &newmol->atoms[mol1->Natom + i];

			memmove(atomdest, atomsrc, sizeof(struct atom));
			atomdest->x += dx;
			atomdest->y += dy;

			atomdest->bonds = malloc(atomsrc->Nbond * sizeof(int));
			assert(atomdest->bonds);
			atomdest->neighbors = malloc(atomsrc->Nbond * sizeof(int));
			assert(atomdest->neighbors);

			for(j = 0; j < atomdest->Nbond; ++j) { //build neighbor & bond list
				atomdest->bonds[j] = atomsrc->bonds[j] + mol1->Nbond;
				atomdest->neighbors[j] = atomsrc->neighbors[j] + mol1->Natom;
			}
		}

		for(i = 0; i < mol2->Nbond; ++i){
			newmol->bonds[i+mol1->Nbond].a1 = mol2->bonds[i].a1 + mol1->Natom;
			newmol->bonds[i+mol1->Nbond].a2 = mol2->bonds[i].a2 + mol1->Natom;
			newmol->bonds[i+mol1->Nbond].order = mol2->bonds[i].order;
		}

	}
	
	return(newmol);
}			
	


/*	removes a bond from the bond list and the neighbor list of an atom.
 *	It also takes care of the hydrogen count.
 *	na is the atom's number, nb the bond's number.
 *	this function is called by breakbond()  */
void _removebondfromatom(struct mol *mol, int na, int nb)
{
	int i;
	struct atom *atom = &mol->atoms[na];
	struct bond *bond = &mol->bonds[nb];

	for(i=0; (i < atom->Nbond) && (atom->bonds[i] != nb); ++i);
	if(atom->bonds[i] == nb)
	{
		--(atom->Nbond);
		atom->nH += bond->order;
		if(i < atom->Nbond)
		{
			atom->bonds[i] = atom->bonds[atom->Nbond];
			atom->neighbors[i] = atom->neighbors[atom->Nbond];
		}
		atom->bonds = realloc(atom->bonds, atom->Nbond*sizeof(int));
		assert((atom->bonds) || (atom->Nbond == 0));
		atom->neighbors = realloc(atom->neighbors, atom->Nbond*sizeof(int));
		assert((atom->neighbors) || (atom->Nbond == 0));
	}
	else
		assert(0);
}

/* adds a bond to the bond list and the neighbor list of an atom.
 * It also takes care of the hydrogen count.
 * na is the atom's number, nb the bond's number.
 * this function is called by newbond()
 * Notice that, like new_bond(), it only works with single bonds   */
void _addbondtoatom(struct mol *mol, int na, int nb)
{
	struct atom *atom = &mol->atoms[na];

	atom->bonds = realloc(atom->bonds, (atom->Nbond + 1)*sizeof(int));
	assert(atom->bonds);
	atom->bonds[atom->Nbond] = nb;

	atom->neighbors = realloc(atom->neighbors, (atom->Nbond + 1)*sizeof(int));
	assert(atom->neighbors);
	atom->neighbors[atom->Nbond] = mol->bonds[nb].a1 == na ? mol->bonds[nb].a2 : mol->bonds[nb].a1;
	
	++atom->Nbond;
//	--atom->nH;
}


/* finds a bond in the bond list of an atom and replaces it with another
 * this is used when moving or swapping bonds in the molecule's bond list
 * na is the atom's number, nb_old the bond's number, and nb_new the new bond's number
 */
void _changebondinatom(struct mol *mol, int na, int nb_old, int nb_new)
{
	int i;
	struct atom *atom = &mol->atoms[na];

	for(i=0; (i < atom->Nbond) && (atom->bonds[i] != nb_old); ++i);
	if(atom->bonds[i] == nb_old)
	{
		atom->bonds[i] = nb_new;
		atom->neighbors[i] = mol->bonds[nb_new].a1 == na ? mol->bonds[nb_new].a2 : mol->bonds[nb_new].a1;
	}
	else
		assert(0);

}

/* breaks and deletes a bond regardless of its type, syncronizing the
 * hydrogen count and bond and neighbor lists of the atoms involved
 * nb is the bond's number */
void breakbond(struct mol *mol, int nb)
{
	struct bond *bond;

	bond = &mol->bonds[nb];

	_removebondfromatom(mol, bond->a1, nb);
	_removebondfromatom(mol, bond->a2, nb);

	--(mol->Nbond);
	if(nb < mol->Nbond) // "compress" bond list moving the last bond to the empty spot
	{
		bond = &(mol->bonds[mol->Nbond]);
		mol->bonds[nb] = *bond;

		_changebondinatom(mol, bond->a1, mol->Nbond, nb);
		_changebondinatom(mol, bond->a2, mol->Nbond, nb);

	}

	mol->bonds = realloc(mol->bonds, mol->Nbond*sizeof(struct bond));
	assert((mol->bonds) || (mol->Nbond == 0));
}

/* Deletes an atom and all of its bonds. This does NOT delete
 * "lone atoms" automatically                */
void deleteatom(struct mol *mol, int n_atom) 
{
	int i, j;
	struct atom *atom;
	struct bond *bond;


	atom = &(mol->atoms[n_atom]);

	while(atom->Nbond)
		breakbond(mol, atom->bonds[0]);

	--mol->Natom;
	_destroy_atom(&mol->atoms[n_atom]);

	if(n_atom < mol->Natom) 
	{				   // compress atom list moving the last atom to the empty spot
		atom = &(mol->atoms[mol->Natom]);
		mol->atoms[n_atom] = *atom;
		for(i=0; i < atom->Nbond; ++i)
		{
			bond = &(mol->bonds[atom->bonds[i]]);
			if(bond->a1 == mol->Natom)  // change bonds to the new atom's number
				bond->a1 = n_atom;
			else 
				bond->a2 = n_atom;
		}

		for(i = 0; i < mol->Natom; ++i){
			atom = &(mol->atoms[i]);
			for(j = 0; j < atom->Nbond; ++j) {
				if(atom->neighbors[j] == mol->Natom) //change neighbor lists to the new atom's number
					atom->neighbors[j] = n_atom;
			} 
		}
	}
	mol->atoms = realloc(mol->atoms, mol->Natom*sizeof(struct atom));
	assert((mol->atoms) || (mol->Natom == 0));
		
}

/*	Deletes all the atoms marked as TRUE in the array *used.
	*used must be an array of Natom integers                   */
void deleteatoms(struct mol *mol, int *used) 
{
	int i, j, n_atom;
	struct atom *atom;
	struct bond *bond;


	for (n_atom = 0; n_atom < mol->Natom; ++n_atom) {
		if (used[n_atom]) {
			atom = &(mol->atoms[n_atom]);

			while(atom->Nbond)
				breakbond(mol, atom->bonds[0]);

			--mol->Natom;
			_destroy_atom(&mol->atoms[n_atom]);

			if(n_atom < mol->Natom) 
			{				   // compress atom list moving the last atom to the empty spot
				atom = &(mol->atoms[mol->Natom]);
				mol->atoms[n_atom] = *atom;
				used[n_atom] = used[mol->Natom];
				for(i=0; i < atom->Nbond; ++i)
				{
					bond = &(mol->bonds[atom->bonds[i]]);
					if(bond->a1 == mol->Natom)  // change bonds to the new atom's number
						bond->a1 = n_atom;
					else 
						bond->a2 = n_atom;
				}

				for(i = 0; i < mol->Natom; ++i){
					atom = &(mol->atoms[i]);
					for(j = 0; j < atom->Nbond; ++j) {
						if(atom->neighbors[j] == mol->Natom) //change neighbor lists to the new atom's number
							atom->neighbors[j] = n_atom;
					} 
				}
			}
			mol->atoms = realloc(mol->atoms, mol->Natom*sizeof(struct atom));
			assert((mol->atoms) || (mol->Natom == 0));

			--n_atom;
		}
	}		
}


int can_addbond(struct mol *mol, int na)
{
	BOOL ret = FALSE;

	if(mol->atoms[na].nH > 0)
		return(TRUE);
	else switch(mol->atoms[na].Z) {
		case P:
		case N:
			ret = (mol->atoms[na].valence < 5);
			break;

		case S:
			ret = (mol->atoms[na].valence < 6);
			break;

		case B:
			ret = (mol->atoms[na].valence < 4);
			break;
	}
	return(ret);
}


void calc_charge(struct mol *mol, int na)
{
	switch(mol->atoms[na].Z) {
		case P:
		case N:
			if(mol->atoms[na].valence == 4)
				mol->atoms[na].charge = +1;
			else if (mol->atoms[na].valence == 5)
				mol->atoms[na].charge = 0;
			break;

		case B:
			if(mol->atoms[na].valence < 4)
				mol->atoms[na].charge = -1;
			break;
	}

}


/* increases the bond order of a bond, taking hydrogens into account
 * if there are no hydrogens on both atoms the order cannot be increased,
 * so inc_bondorder() returns nonzero */
int inc_bondorder(struct mol *mol, int n_bond)
{
	struct bond *bond;

	bond = &mol->bonds[n_bond];

//	if((bond->order < 3) && (mol->atoms[bond->a1].nH > 0) && (mol->atoms[bond->a2].nH > 0))
	if((bond->order < 3) && can_addbond(mol, bond->a1) && can_addbond(mol, bond->a2))
	{
		++bond->order;
		if(mol->atoms[bond->a1].nH > 0)
			--mol->atoms[bond->a1].nH;
		else {
			++mol->atoms[bond->a1].valence;
		}
		calc_charge(mol, bond->a1);
		if(mol->atoms[bond->a2].nH > 0)
			--mol->atoms[bond->a2].nH;
		else {
			++mol->atoms[bond->a2].valence;
		}
		calc_charge(mol, bond->a2);
		
		return(0);
	}
	else
		return(-1);

}

/* decreases the order of a bond by one. If the initial order is one, this is
 * equivalent to breakbond() */
void dec_bondorder(struct mol *mol, int n_bond)
{
	struct bond *bond;

	bond = &mol->bonds[n_bond];
	if(mol->bonds[n_bond].order == 1)
		breakbond(mol, n_bond);
	else
	{
		--bond->order;
		++mol->atoms[bond->a1].nH;
		++mol->atoms[bond->a2].nH;
	}
}

/* 	increases the bond order by one, but if it is not possible it sets it at one */
void rotate_bondorder(struct mol *mol, int n_bond)
{

	if(inc_bondorder(mol, n_bond) != 0)
		while(mol->bonds[n_bond].order > 1)
			dec_bondorder(mol, n_bond);

}



int set_bondorder(struct mol *mol, int n_bond, int order)
{
	struct bond *bond = &mol->bonds[n_bond];
	int err = 0;

	if(order > bond->order) {
		while(order > bond->order)
			if(inc_bondorder(mol, n_bond)) {
				err = -1;
				break;
			}
	} else {
		while((order < bond->order) && (bond->order > 0))
			dec_bondorder(mol, n_bond);
		if(bond->order == 0)
			err = -1;
	}

	return(err);
}




/* creates a new atom with the specified parameters
 * returns the number of the new atom */
int new_atom(struct mol *mol, int Z, int charge, double x, double y, double z)
{
	struct atom *atom;

	mol->atoms = realloc(mol->atoms, (mol->Natom + 1)*sizeof(struct atom));
	assert(mol->atoms);
	atom = &mol->atoms[mol->Natom];
	atom->x = x;
	atom->y = y;
	atom->z = z;
	atom->Z = Z;
	atom->charge = charge;
	atom->Nbond = 0;
	atom->nH = valence(Z);
	atom->bonds = NULL;
	atom->neighbors = NULL;
	atom->aromatic = FALSE;

	if(group(Z)>14) 
		atom->nH += charge;
	else if(valence(Z))
		atom->nH -= charge;

	atom->valence = atom->nH;

	++mol->Natom;

	return(mol->Natom - 1);
}

/*	returns the number of the bond joining atoms a1 and a2. If they are not
 *	bonded, returns -1*/
int are_bonded(struct mol *mol, int a1, int a2)
{
	struct atom *atom1, *atom2;
	int i, j;

	atom1 = &mol->atoms[a1];
	atom2 = &mol->atoms[a2];

	for(i=0; i < atom1->Nbond; ++i)
		for(j=0; j < atom2->Nbond; ++j)
			if(atom1->bonds[i] == atom2->bonds[j])
				return(atom1->bonds[i]);
	
	return(-1);
}


/* Returns the order of the bond between two given atoms. If the atoms are not
 * bonded, returns zero. */
int bond_order(struct mol *mol, int a1, int a2)
{
        int bond;
        int order = 0;

        if((bond = are_bonded(mol, a1, a2)) >= 0) {
                order = mol->bonds[bond].order;
        }
        return(order);
}

/*	creates a bond betwen a1 and a2. If they are already bonded, it does the
 *	same as inc_bondorder(). Returns the bond's number if successful, or
 *	-1 otherwise */
int new_bond(struct mol *mol, int a1, int a2)
{
	int n;
	struct bond *bond;

	if((n = are_bonded(mol, a1, a2)) >= 0)
	{
		if(inc_bondorder(mol, n) == 0)
			return(n);
		else
			return(-1);
	}
	else
	{
		mol->bonds = realloc(mol->bonds, (mol->Nbond+1)*sizeof(struct bond));
		assert(mol->bonds);
		bond = &mol->bonds[mol->Nbond];
		bond->a1 = a1;
		bond->a2 = a2;
		bond->order = 0;
		_addbondtoatom(mol, a1, mol->Nbond);
		_addbondtoatom(mol, a2, mol->Nbond);
		++mol->Nbond;
		inc_bondorder(mol, mol->Nbond-1);
		
		return(mol->Nbond -1);
	}

}


int new_bond_ex(struct mol *mol, int a1, int a2, int order)
{
	int n, err = 0, i;

	if(are_bonded(mol, a1, a2) < 0)
	{
		n = new_bond(mol, a1, a2);
		for(i = 1; i < order; ++i)
			if((err = inc_bondorder(mol, n)) != 0)
				break;
		if(err)
			return(-1);
		else
			return(n);
	} else
		return(-1);
}



/*	Changes the element of an atom, taking into account the hydrogen count
 *	returns zero if successfull or nonzero otherwise  */
int setatomZ(struct mol *mol, int n_atom, int Z)
{
	struct atom *atom = &mol->atoms[n_atom];
	int i;
		
	if(valence(Z) >= atom->Nbond){

		atom->Z = Z;
		for(i = 0, atom->nH = valence(Z); i< atom->Nbond; ++i)
			atom->nH -= mol->bonds[atom->bonds[i]].order;

		if(group(Z)>14) 
			atom->nH += atom->charge;
		else if(valence(Z))
			atom->nH -= atom->charge;
		
		return(0);
	}
	else {
		return(-1);
	}

}



void calc_formula(struct mol *mol)
{
	int i;

	memset(mol->formula, 0, sizeof(mol->formula));
	for(i = 0; i < mol->Natom; ++i) {
		mol->formula[1] = (unsigned char)(mol->formula[1] + mol->atoms[i].nH);
		mol->formula[1] %= 100;

		switch(mol->atoms[i].Z) {
			case C:
				++mol->formula[0];
				mol->formula[0] %= 100;
				break;

			case O:
				++mol->formula[2];
				mol->formula[2] %= 100;
				break;

			case N:
				++mol->formula[3];
				mol->formula[2] %= 100;
				break;
		}
	}

}







