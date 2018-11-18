/* _path.c
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

#include "../mollib/mollib.h"

/*
struct analysisnode
{
	struct mol_metalist *mml;    // Current mol metalist
	struct analysisnode *target; // "parent"
	struct analysisnode ***precs; // precursors
	struct mol_list *orphans;
	int depth;
	struct targetkey {
		int rxn;
		int prec;
	} targetkey;
	void *user;
}; */


// Fills orphans automatically...
struct analysisnode *new_analysisnode(struct analysisnode *target, int rxn, int prec, struct mol_metalist *mml)
{
	struct analysisnode *node;
	int i;

	node = calloc(1, sizeof(struct analysisnode));
	assert(node);
	node->mml = mml;
	node->target = target;
	node->targetkey.rxn = rxn;
	node->targetkey.prec = prec;
	node->user = NULL;

	node->precs = calloc(mml->Nlist, sizeof(struct analysisnode **));
	assert(node->precs);
/*	for(i = 0; i < mml->Nlist; ++i){
		node->precs[i] = calloc(mml->lists[i]->Nmol, sizeof(struct analysisnode *));
		assert(node->precs[i]);
	}*/

	if(target) {
		node->depth = target->depth + 1;
		node->orphans = mol_listcpy(target->orphans);
		for(i = 0; i < target->mml->lists[rxn]->Nmol; ++i) {
			if(i != prec)
				append_mol(node->orphans, target->mml->lists[rxn]->mols[i]);
		}
		if(target->precs[rxn] == NULL) {
			target->precs[rxn] = calloc(target->mml->lists[rxn]->Nmol, sizeof(struct analysisnode *));
			assert(target->precs[rxn]);
		}
		target->precs[rxn][prec] = node;
	} else {  // target == NULL means this is the root (ultimate target)
		node->depth = 0;
		node->orphans = new_mol_list(NULL);
	}
	return(node);
}




