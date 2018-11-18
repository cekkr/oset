/* _clas2.c
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


int _classify_carbon(struct mol *mol, int atom_number, struct chem_err *err);
int _classify_nitrogen(struct mol *mol, int atom_number, struct chem_err *err);
int _classify_oxygen(struct mol *mol, int atom_number, struct chem_err *err);
int _classify_halogen(struct mol *mol, int atom_number, struct chem_err *err);
//int _classify_sulphur(struct mol *mol, int atom_number, struct chem_err *err);
void _find_ewg(struct mol *mol);
void setattr(unsigned int *arr, int pos);
BOOL noattr(unsigned int *arr);


// NOTE: this list must be in the same order as the one in caoscomp.h and the list in
// _scanner.c These are the displayed names and they are not used for parsing.
// All of the types on this list must also appear on the list in _expr.c  

char *chemtypes[] = {
"methyl", "primary", "secondary", "tertiary", "quaternary", "vinyl", "carbonyl", "alkynyl",
"nitrile", "allene", "alkyl", "sp3", "sp2", "sp", "phenyl", "noncarbon", 
"hydroxyl", "peroxide", "allyl", "benzyl", 
"alpha_carbonyl", "alpha_alkynyl", "alpha_nitrile", "","","","","X","","", "", "alpha_CH",
"EWG", "alpha_EWG", "alpha_EWG2", "alpha_EWG3"};

int classify_atoms(struct mol *mol, struct chem_err *err)
{
	int i,j, neighnum, errors = 0;
	struct atom *atom, *neighbor;
	struct bond *bond;

	for(i=0; i < mol->Natom; ++i)	  //First pass; carbon only
	{
		atom = &mol->atoms[i];
		memset(atom->chemattr, 0, sizeof(atom->chemattr));
		if(atom->charge)
			setattr(atom->chemattr, RW_NONCARBON);
		if(atom->Z == C)
			errors += _classify_carbon(mol, i, err);
	}

	for(i=0; i < mol->Natom; ++i)	  //second pass; hetero
	{
		atom = &mol->atoms[i];
		if(atom->Z != C){
			switch(atom->Z)	{
				case N:
					errors += _classify_nitrogen(mol, i, err);
					break;

				case O:
					errors += _classify_oxygen(mol, i, err);
					setattr(atom->chemattr, RW_OXYGEN);
					break;

				case Cl:
				case Br:
				case I:
					errors += _classify_halogen(mol, i, err);
					break;
//				case S:
					//_classify_sulphur();
					break;

			}
		}
	}
	
	_find_ewg(mol);


	for(i=0; i< mol->Natom; ++i) //third pass: "alpha to..."
	{
		atom = &(mol->atoms[i]);
		if((atom->Z == C))
		{
			for(j=0; j < atom->Nbond; ++j)
			{
				bond = &(mol->bonds[atom->bonds[j]]);
				if(bond->order == 1)
				{
					neighnum = ((bond->a1 == i) ? bond->a2 : bond->a1);
					neighbor = &mol->atoms[neighnum];
					if (neighbor->Z == C)	
					{
						if(isatomtype(mol, neighnum, RW_PHENYL) && !isatomtype(mol, i, RW_PHENYL))
							setattr(atom->chemattr, RW_BENZYL);
						if(isatomtype(mol, neighnum, RW_VINYL))
							setattr(atom->chemattr, RW_ALLYL);
						if(isatomtype(mol, neighnum, RW_CARBONYL))
							setattr(atom->chemattr, RW_ALPHA_CARBONYL);
						if(isatomtype(mol, neighnum, RW_ALKYNYL))
							setattr(atom->chemattr, RW_ALPHA_ALKYNYL);
						if(isatomtype(mol, neighnum, RW_NITRILE))
							setattr(atom->chemattr, RW_ALPHA_NITRILE);
					}
					if(isatomtype(mol, neighnum, RW_EWG)){
						if(isatomtype(mol, i, RW_ALPHA_EWG2))
							setattr(atom->chemattr, RW_ALPHA_EWG3);
						else if(isatomtype(mol, i, RW_ALPHA_EWG))
							setattr(atom->chemattr, RW_ALPHA_EWG2);
						else
							setattr(atom->chemattr, RW_ALPHA_EWG);
					}
				}
			}
		}
	}

	return(errors);
}


int _classify_carbon(struct mol *mol, int atom_number, struct chem_err *err)
{	
	int j, Cvec, noCvec, errors = 0;
	struct bond *bond;
	struct atom *atom;
	char s[100];

	atom = &(mol->atoms[atom_number]);
// if nH < 0... ?
	switch(atom->Nbond+atom->nH)
	{
		case 4:    //sp3
			if(atom->Nbond <= 4)  {
				Cvec = noCvec = 0;
				for(j=0; j < atom->Nbond; ++j) {
					bond = &(mol->bonds[atom->bonds[j]]);
					if((mol->atoms[(bond->a1 == atom_number) ? bond->a2 : bond->a1].Z) == C)
						++Cvec;
					else
						++noCvec;
				}
				
				switch(Cvec)
				{
					case 0:
						setattr(atom->chemattr, RW_METHYL);
						break;

					case 1:
						setattr(atom->chemattr, RW_PRIMARY);
						break;

					case 2:
						setattr(atom->chemattr, RW_SECONDARY);
						break;

					case 3:
						setattr(atom->chemattr, RW_TERTIARY);
						break;

					case 4:
						setattr(atom->chemattr, RW_QUATERNARY);
						break;
				}
				setattr(atom->chemattr, RW_ALKYL);
				setattr(atom->chemattr, RW_SP3);
				if(noCvec == 0)
					setattr(atom->chemattr, RW_ALPHA_CH);
			}
			else
			{
				sprintf(s, "Error: carbon atom %i has a valence of %i\n", 
						atom_number+1, atom->Nbond);
				__error_log(err, s);
				++errors;
			}
			break;

		case 3:	  //sp2
			setattr(atom->chemattr, RW_SP2);
			for(j=0; j < mol->Nring; ++j) {
   				if(mol->rings[j].type & RING_AROMATIC)
				  	if(bs_isset(mol->rings[j].nodes, atom_number)){
						setattr(atom->chemattr, RW_PHENYL);
					}
			}

			for(j=0; j < atom->Nbond; ++j)
			{
				bond = &(mol->bonds[atom->bonds[j]]);
				if(bond->order == 2)
					switch(mol->atoms[(bond->a1 == atom_number) ? bond->a2 : bond->a1].Z)
					{
						case C:
							setattr(atom->chemattr, RW_VINYL);
							break;
						case O:
							setattr(atom->chemattr, RW_CARBONYL);
							break;
					}
			}
			break;

		case 2:		   //sp
			for(j=0; j < atom->Nbond; ++j)
			{
				bond = &(mol->bonds[atom->bonds[j]]);
				switch(bond->order)
				{
					case 3:
						switch(mol->atoms[(bond->a1 == atom_number) ? bond->a2 : bond->a1].Z)
						{
							case C:
								setattr(atom->chemattr, RW_ALKYNYL);
								break;
							case N:
								setattr(atom->chemattr, RW_NITRILE);
								break;
						}
						break;
					case 2: 
						setattr(atom->chemattr, RW_ALLENE);//falta detalle para reconocer cetenas, etc.
						break;
				}
			}
			break;
	}
	if((noattr(atom->chemattr)) && (!errors)){
		sprintf(s, "Error: carbon atom %i is unidentified\n", 
						atom_number+1);
		__error_log(err, s);
		++errors;
	}
	setattr(atom->chemattr, RW_CARBON);
	
	return(errors);
}


int _classify_nitrogen(struct mol *mol, int atom_number, struct chem_err *err)
{
	int j, Cvec, errors = 0;
	struct bond *bond;
	struct atom *atom;
	char s[100];

	atom = &(mol->atoms[atom_number]);

	switch(atom->Nbond+atom->nH)
	{
		case 4:    //sp3, amonio cuaternario
			Cvec = 0;
			
			for(j=0; j < atom->Nbond; ++j)
			{
				bond = &(mol->bonds[atom->bonds[j]]);
				if((mol->atoms[(bond->a1 == atom_number) ? bond->a2 : bond->a1].Z) == C)
					++Cvec;
			}
			if((Cvec + atom->nH == 4) && (atom->nH >= 0) && (atom->charge == +1))
				setattr(atom->chemattr, RW_QUATERNARY);
			break;				 

		case 3:	  //amina sp3
			Cvec = 0;
			for(j=0; j < atom->Nbond; ++j)
			{
				bond = &(mol->bonds[atom->bonds[j]]);
				if(bond->order == 1)
					switch(mol->atoms[(bond->a1 == atom_number) ? bond->a2 : bond->a1].Z)
					{
						case C:
							++Cvec;
							break;
						default:
							Cvec = -999;
					}
			}

			switch(Cvec)
			{
				case 1:
					setattr(atom->chemattr, RW_PRIMARY);
					break;

				case 2:
					setattr(atom->chemattr, RW_SECONDARY);
					break;

				case 3:
					setattr(atom->chemattr, RW_TERTIARY);
					break;
			}
			break;

		case 2:		   //sp2
/*			for(j=0; j < atom->Nbond; ++j)
			{
				bond = &(mol->bonds[atom->bonds[j]]);
				switch(bond->type)
				{
					case 3:
						switch(mol->atoms[(bond->a1 == atom_number) ? bond->a2 : bond->a1].Z)
						{
							case C:
								atom->type = CT_ALKYNYL;
								break;
							case N:
								atom->type = CT_NITRILE;
								break;
							default:
								atom->type = CT_SP;
						}
						break;
					case 2: 
						atom->type = CT_ALLENE;//falta detalle para reconocer cetenas, etc.
						break;
				}
			}*/
			break;

		case 1:  //sp, nitrilos
			if(atom->charge == 0) {
				bond = &(mol->bonds[atom->bonds[0]]);
				if(bond->order == 3)
					if((mol->atoms[(bond->a1 == atom_number) ? bond->a2 : bond->a1].Z) == C)
						setattr(atom->chemattr, RW_NITRILE);
			}
			break;

	}

	if(noattr(atom->chemattr)){
		sprintf(s, "Error: nitrogen atom %i is unidentified\n", 
						atom_number+1);
		__error_log(err, s);
		++errors;
	}

	setattr(atom->chemattr, RW_NITROGEN);
	return(errors);
}


int _classify_oxygen(struct mol *mol, int atom_number, struct chem_err *err) 
{
	int j, Cvec, Ovec, errors = 0, neighbor;
	struct bond *bond;
	struct atom *atom;
	char s[100];

	atom = &(mol->atoms[atom_number]);

	switch(atom->Nbond+atom->nH-atom->charge)
	{
		case 2:	  //sp3, hidroxilo o eter
			Cvec = 0;
			Ovec = 0;
			for(j=0; j < atom->Nbond; ++j)
			{
				bond = &(mol->bonds[atom->bonds[j]]);
				if(bond->order == 1)
					switch(mol->atoms[(bond->a1 == atom_number) ? bond->a2 : bond->a1].Z)
					{
						case C:
							++Cvec;
							break;

						case O:
							++Ovec;
							break;

						default:
							Cvec = -999;
					}
			}

			switch(Ovec){
				case 1:
					setattr(atom->chemattr, RW_PEROXIDE);
					break;

				case 0:
					switch(Cvec)
					{
						case 1:
							setattr(atom->chemattr, RW_HYDROXYL);
							neighbor = atom->neighbors[0];
							
							if(isatomtype(mol, neighbor, RW_VINYL))
								setattr(atom->chemattr, RW_VINYL);
							else if(isatomtype(mol, neighbor, RW_PRIMARY))
								setattr(atom->chemattr, RW_PRIMARY);
							else if(isatomtype(mol, neighbor, RW_SECONDARY))
								setattr(atom->chemattr, RW_SECONDARY);
							else if(isatomtype(mol, neighbor, RW_TERTIARY))
								setattr(atom->chemattr, RW_TERTIARY);
	//						else if(isatomtype(mol, neighnum, RW_CARBONYL))
	//							setattr(atom->chemattr, RW_CARBOXYL);

							break;

					}
					break;

				default:
					sprintf(s, "Error: three oxygen atoms in a row at atom %i\n", 
							atom_number+1);
					__error_log(err, s);
					++errors;

					break;
			}
			break;

		case 1:  //sp2, carbonyl
			bond = &(mol->bonds[atom->bonds[0]]);
			if(bond->order == 2)
				if((mol->atoms[(bond->a1 == atom_number) ? bond->a2 : bond->a1].Z) == C)
					setattr(atom->chemattr, RW_CARBONYL);
			break;

	}

	if(noattr(atom->chemattr)){
		sprintf(s, "Error: oxygen atom %i is unidentified\n", 
						atom_number+1);
		__error_log(err, s);
		++errors;
	}
	setattr(atom->chemattr, RW_OXYGEN);
	
	return(errors);
}


int _classify_halogen(struct mol *mol, int atom_number, struct chem_err *err) 
{
	int neighbor;
	struct atom *atom;
	char s[100];
	int errors = 0;

	atom = &(mol->atoms[atom_number]);

	switch(atom->Nbond){
		case 0:
			if(atom->charge != -1){
			sprintf(s, "Error: isolated halogen atom %i\n", atom_number+1);
			__error_log(err, s);
			}
				
			break;

		case 1:
			setattr(atom->chemattr, RW_HALOGEN);
			switch(atom->Z){
				case Cl:
					setattr(atom->chemattr, RW_CHLORINE);
					break;

				case Br:
					setattr(atom->chemattr, RW_BROMINE);
					break;

				case I:
					setattr(atom->chemattr, RW_IODINE);
					break;
			}

			neighbor = atom->neighbors[0];
			if((mol->atoms[neighbor].Z) != C){
				sprintf(s, "Halogen atom %i is bonded to a noncarbon atom\n", atom_number+1);
				__error_log(err, s);
			}
			else {
				if(isatomtype(mol, neighbor, RW_VINYL))
					setattr(atom->chemattr, RW_VINYL);
				else if(isatomtype(mol, neighbor, RW_PRIMARY))
					setattr(atom->chemattr, RW_PRIMARY);
				else if(isatomtype(mol, neighbor, RW_SECONDARY))
					setattr(atom->chemattr, RW_SECONDARY);
				else if(isatomtype(mol, neighbor, RW_TERTIARY))
					setattr(atom->chemattr, RW_TERTIARY);
			}
			break;

		default:
			sprintf(s, "Error: halogen atom %i has a valence of %i\n", 
					atom_number+1, atom->Nbond);
			__error_log(err, s);
			break;


	}
	if(noattr(atom->chemattr)){
		sprintf(s, "Error: halogen atom %i is unidentified\n", 
						atom_number+1);
		__error_log(err, s);
		++errors;
	}
	setattr(atom->chemattr, RW_HALOGEN);
	return(errors);
}

/*

int _classify_sulphur(struct mol *mol, int atom_number, struct chem_err *err) 
{
	int j, Cvec, Ovec, Svec, errors = 0, neighbor;
	struct bond *bond;
	struct atom *atom;
	char s[100];

	atom = &(mol->atoms[atom_number]);

	switch(atom->Nbond+atom->nH)
	{
		case 2:	  //sp3, hidroxilo o eter
			Cvec = 0;
			Ovec = 0;
			for(j=0; j < atom->Nbond; ++j)
			{
				bond = &(mol->bonds[atom->bonds[j]]);
				if(bond->order == 1)
					switch(mol->atoms[(bond->a1 == atom_number) ? bond->a2 : bond->a1].Z)
					{
						case C:
							++Cvec;
							break;

						case O:
							++Ovec;
							break;

						default:
							Cvec = -999;
					}
			}

			switch(Ovec){
				case 1:
					setattr(atom->chemattr, RW_PEROXIDE);
					break;

				case 0:
					switch(Cvec)
					{
						case 1:
							setattr(atom->chemattr, RW_HYDROXYL);
							neighbor = atom->neighbors[0];
							
							if(isatomtype(mol, neighbor, RW_VINYL))
								setattr(atom->chemattr, RW_VINYL);
							else if(isatomtype(mol, neighbor, RW_PRIMARY))
								setattr(atom->chemattr, RW_PRIMARY);
							else if(isatomtype(mol, neighbor, RW_SECONDARY))
								setattr(atom->chemattr, RW_SECONDARY);
							else if(isatomtype(mol, neighbor, RW_TERTIARY))
								setattr(atom->chemattr, RW_TERTIARY);
	//						else if(isatomtype(mol, neighnum, RW_CARBONYL))
	//							setattr(atom->chemattr, RW_CARBOXYL);

							break;

						case 2:
							switch(isatomtype(mol, atom->neighbors[0], RW_CARBONYL) +
								isatomtype(mol, atom->neighbors[1], RW_CARBONYL)){
								
								case 0:
								case 1:
								case 2: 
									setattr(atom->chemattr, RW_ETHER);
									break;

							}
							break;
					}
					break;

				default:
					sprintf(s, "Error: three oxygen atoms in a row at atom %i\n", 
							atom_number+1);
					__error_log(err, s);
					++errors;

					break;
			}
			break;

		case 1:  //sp2, carbonilos
			bond = &(mol->bonds[atom->bonds[0]]);
			if(bond->order == 2)
				if((mol->atoms[(bond->a1 == atom_number) ? bond->a2 : bond->a1].Z) == C)
					setattr(atom->chemattr, RW_CARBONYL);
			break;

	}

	if(noattr(atom->chemattr)){
		sprintf(s, "Error: oxygen atom %i is unidentified\n", 
						atom_number+1);
		__error_log(err, s);
		++errors;
	}
	setattr(atom->chemattr, RW_OXYGEN);
	
	return(errors);
}

*/


void _find_ewg(struct mol *mol)
{
	int i, j, Z, n;  // n 
	struct atom *atom;

	for(i = 0; i < mol->Natom; ++i){
		atom = &mol->atoms[i];
		for(j = 0, n = 0; j < atom->Nbond; ++j){
			Z = mol->atoms[atom->neighbors[j]].Z; 
			if((Z == O) || (Z == Cl) || (Z == Br) || (Z == N) || (Z == F)) {
				n += mol->bonds[atom->bonds[j]].order*mol->bonds[atom->bonds[j]].order;
			}
		}
		if(n >= 3)
			setattr(atom->chemattr, RW_EWG);
	}


}
 
int __error_log(struct chem_err *err, char *error)
{
	int l;

	if(err) {
		l = strlen(error);
		while(err->length+l > err->bufsize)
		{
			err->bufsize += CH_ERR_BUFINC;
			err->error_log = realloc(err->error_log, err->bufsize+1);
			assert(err->error_log);
		}

		strcat(err->error_log, error);
		err->length += l;
		++(err->num_err);
	}
	return(0);
}




struct chem_err * _init_err()
{
	struct chem_err *err;

	err = malloc(sizeof(struct chem_err));
	assert(err);
	err->error_log = malloc(CH_ERR_INITIAL_BUFSIZE+1);
	err->error_log[0] = 0;
	err->length = 0;
	err->num_err = 0;
	err->bufsize = CH_ERR_INITIAL_BUFSIZE;
	
	return(err);
}

struct chem_err *_destroy_err(struct chem_err *err)
{
	if(err){
		free(err->error_log);
		free(err);
	}
	return(NULL);
}

char *list_atomtypes(struct mol *mol)
{
	int i,j;
	char s3[300], *list;
	struct chem_err *types;
	extern char *symbol[];

	types = _init_err();
	

	for(i=0; i < mol->Natom; ++i) {
		sprintf(s3, "Atom %i: %s; ", i+1, symbol[mol->atoms[i].Z]);
		for(j=0; j < CHEMATTRSIZE*32; ++j){
			if(mol->atoms[i].chemattr[j / 32] & (1 << (j % 32))){
				if(chemtypes[j][0]) {
					strcat(s3, chemtypes[j]);
					strcat(s3, "; ");
				}
			}
		}

		strcat(s3, "\n");
		__error_log(types, s3);
	}
	list = strdup(types->error_log);

	_destroy_err(types);
	
	return(list);

}


BOOL isatomtype(struct mol *mol, int atomnum, int type)
{
	struct atom *atom;

	atom = &mol->atoms[atomnum];
	type -= TOK_CHEMRESERVED;
	if(atom->chemattr[type / 32] & (1 << (type % 32)))
		return(TRUE);
	else
		return(FALSE);

}

void setattr(unsigned int *arr, int pos)
{
	pos -= TOK_CHEMRESERVED;
	arr[pos / 32] |= 1 << (pos % 32);
}

BOOL noattr(unsigned int *arr)
{
	int i;

	for(i=0; i < CHEMATTRSIZE; ++i)
		if(arr[i])
			return(FALSE);

	return(TRUE);	
}
