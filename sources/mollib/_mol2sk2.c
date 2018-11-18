/* _mol2sk2.c
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

/*	This file contains functions for the creation of ACD/ChemSketch documents (*.sk2)
	More information at http://www.acdlabs.com    */

#include "mollib.h"

/* This function puts a struct mol (the internal molecule structure used by OSET) into a
 * ChemSketch .sk2 file. Remember to create the sketch first by calling create_sk2() and to
 * close it when done by calling write_sk2(). 
 * left, top, right, and bottom define the rectangle in which the structure is put, in units
 * of 1/300 inch. The margin is the space between the structure and the rectangle.
 * Notice that the structure is NOT distorted. */
void putmol_sk2(struct sk2info *info, struct mol *mol, double left, double top, double right, double bottom, double margin)
{
	MoleculeFormat *mol2;
	int i;
	TRRect r;

//****** Create molecule object
	addobj_sk2(info->obj_list, mol2 = CreateMolecule(), MOLECULE);

	for(i = 0; i < mol->Natom; ++i){  // create atoms
		struct atom *atom = &mol->atoms[i];
		AddAtom(mol2, CreateAtom(info, (Integer)(atom->Z-1), (Single)atom->x, 
			(Single)-atom->y, (Single)atom->z, (ShortInt)atom->charge, DEFAULT_STYLE));
	}

	for(i = 0; i < mol->Nbond; ++i){   // create bonds
		struct bond *bond = &mol->bonds[i];
		AddBond(mol2, CreateBond(info, (Integer)bond->a1, (Integer)bond->a2, (Byte)bond->order, DEFAULT_STYLE));
	}

	r.left = (Single) left;
	r.top = (Single) top;
	r.right = (Single) right;
	r.bottom = (Single) bottom;

	SetMolRect(mol2, r, (Single)margin, FALSE);	// adjust mol size and pos

//	addobj_sk2(info->obj_list, CreateLineEx(info, -1, -1, 5, left, top, right, top, right, bottom, left, bottom, left, top), LINE);
				

}

int test_sk2(struct mol *mol)
{
	struct sk2info *info;
	int myarrow;
	GroupFormat *g;

	info = create_sk2("prueba.sk2");
	
	myarrow = addstyle(info, 
		CreateArrowStyle(AS_UPDOWN | AS_FILLED, AS_NULL, 
			AS_NULL , AS_NULL, 0, 8, 3),
		ARROWSTYLE);
	
	putmol_sk2(info, mol, 400, 400, 800, 800, 0);

	addobj_sk2(info->obj_list, g = CreateGroup(), GROUP);
	addobj_sk2(g->ObjectsList, CreateLineEx(info, -1, -1, 2, 900.0, 400.0, 1000.0, 600.0), LINE);
	addobj_sk2(g->ObjectsList, CreateLineEx(info, -1, -1, 2, 900.0, 800.0, 1000.0, 600.0), LINE);
	addobj_sk2(g->ObjectsList, CreateLineEx(info, -1, myarrow, 2, 1000.0, 600.0, 1300.0, 600.0), LINE);

	write_sk2(info);
	return(0);
}


