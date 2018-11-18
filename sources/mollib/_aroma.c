/* _aroma.c
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



/*  These algorithms are loosely based on the ideas given in:

Weininger; SMILES, a Chemical Language and Information System. 
1. Introduction to Methodology and Encoding Rules
J. Chem. Inf. Comput. Sci. 28, 31-36 (1988)
*/


/* gives the number of "aromatizable" pi electrons for a given atom in a molecule */
int n_pi(struct mol *mol, int atom_index)
{
	int i, n = -777;
	struct atom *atom = &mol->atoms[atom_index];
	int orderf[4];  // order frequency
	
	memset(orderf, 0, sizeof(orderf));
	for(i = 0; i < atom->Nbond; ++i)
		++orderf[mol->bonds[atom->bonds[i]].order];

	if(!(orderf[3]) && !(orderf[2] > 1) && !(atom->Nbond > 3)) {  // disqualifying conditions
		if(orderf[2] == 1)
			n = 1;		  // benzene, pyridine...
		else {
			switch(atom->Z) {
				case N:
					if (atom->charge > 0)
						break;   // for example, protonated pyrrole
					// else fall through...

				case O:
				case S:
					n = 2;  // pyrrole, furane, thiophene...
					break;

				case C:
					if(atom->charge == +1)
						n = 0;         // tropylium
					else if(atom->charge == -1)
						n = 2;         // cyclopentadienyl
					break;
			}
		}
	}

	return(n);
}

BOOL is_aromatic(struct mol *mol, int ring_index)
{
	int i, size, pi = 0, n;
	struct ringpath *ring = &mol->rings[ring_index];
	BOOL aromatic = FALSE;

	switch(size = (ring->len - 1)) {
		case 6:
			for(i = 0; i < size; ++i) {
				if(n_pi(mol, ring->path[i]) == 1)
					++pi;
				else
					break;
			}
			if(pi == 6)
				aromatic = TRUE;
			break;

		case 5:
			for(i = 0; i < size; ++i) {
				n = n_pi(mol, ring->path[i]);
				if(n >= 1)
					pi += n;
				else {
					pi = 0;
					break;
				}
			}
			if(pi == 6)
				aromatic = TRUE;
			break;

		case 7:
			for(i = 0; i < size; ++i) {
				if(n_pi(mol, ring->path[i]) <= 1)
					++pi;
				else
					break;
			}
			if(pi == 6)
				aromatic = TRUE;
			break;


	}

	return(aromatic);
}




void detect_aromaticity(struct mol *mol)
{
	int i, j;

	for(i = 0; i < mol->Natom; ++i)
		mol->atoms[i].aromatic = FALSE;

	for(i = 0; i < mol->Nring; ++i) {
		struct ringpath *r = &mol->rings[i];
		if(is_aromatic(mol, i)) {
			r->type |= RING_AROMATIC;
			for(j = 0; j < r->len - 1; ++j)
				mol->atoms[r->path[j]].aromatic = TRUE;
		}
	}
}


#define AROMATIZE_OK	1
#define AROMATIZE_RETRY	0
#define AROMATIZE_ERR	-1
#define MAXAROMSIZE		8
#define MAXTRIES		100000

struct aromatize_info {
	struct mol *mol;
	int *used;
	int tries;
};

int next_arom_ring(struct mol *mol, int curr)
{
	int j;

	for(++curr; curr < mol->Nring; ++curr) {
		if(mol->rings[curr].len > MAXAROMSIZE)
			continue;
		for(j = 0; j < mol->rings[curr].len - 1 ; ++j) {
			if(!mol->atoms[mol->rings[curr].path[j]].aromatic)
				break;
		}
		if(j >= mol->rings[curr].len - 1)
			break;
	}
	if(curr >= mol->Nring)
		curr = -1;

	return(curr);
}

int next_dbond(struct aromatize_info *info, int ring_index, int startnode)
{
	int i, j, bond, res;
	BOOL ok;
	struct ringpath *ring;

	
	if(ring_index < 0) {  // no more rings: terminal checks
		// check if all aromatic rings are ok and mark used atoms...
		++info->tries;
		if(info->tries > MAXTRIES)
			return(AROMATIZE_ERR);
		memset(info->used, 0, info->mol->Natom * sizeof(int));
		for(i = -1; (i = next_arom_ring(info->mol, i)) >= 0; ) {
			if(!is_aromatic(info->mol, i))
				return(AROMATIZE_RETRY);
			else {
				ring = &info->mol->rings[i];
				for(j = 0; j < ring->len - 1; ++j)
					info->used[ring->path[j]] = TRUE;
			}
		}

		//check if all aromatic atoms have been used...
		for(i = 0; i < info->mol->Natom; ++i)
			if(info->mol->atoms[i].aromatic && !info->used[i])
				return(AROMATIZE_ERR);
		return(AROMATIZE_OK);
	} else {
		ring = &info->mol->rings[ring_index];
		

		for(i = startnode; i < ring->len - 1; ++i) {
			bond = are_bonded(info->mol, ring->path[i], ring->path[i+1]);
			
			for(j = 0, ok = TRUE; j < info->mol->atoms[ring->path[i]].Nbond; ++j)
				if(info->mol->bonds[info->mol->atoms[ring->path[i]].bonds[j]].order > 1) {
					ok = FALSE;
					break;
				}
			
			if(ok) {
				if(inc_bondorder(info->mol, bond) == 0) {
					res = next_dbond(info, ring_index, i + 2);  // recurse
					if(res != AROMATIZE_RETRY)
						return(res);
					dec_bondorder(info->mol, bond);
				}

			}
		}
		res = next_dbond(info, next_arom_ring(info->mol, ring_index), 0);  // recurse
		return(res);
	}
}

int aromatize(struct mol *mol)
{
	struct aromatize_info info;
	int result = 0;

	if(mol) {
		info.mol = mol;
		info.used = calloc(mol->Natom, sizeof(int));
		assert(info.used);
		info.tries = 0;
		
		result = next_dbond(&info, next_arom_ring(mol, -1), 0);

		free(info.used);

	}
	return(result);
}








