/* _moltran.c
 *
 * Copyright (C) 2000 Ivan Tubert and Eduardo Tubert
 * 
 * Contact: tubert@eros.pquim.unam.mx
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * All I ask is that proper credit is given for my work, which includes
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

#include "server.h"

extern char *symbol[84];

void mol2trans(struct struct_info *info, struct sock_data *s,struct mol *mol)
{
	int i;

	assert(mol);
	assert(s);

	sockprintf(s, "STARTMOL\n");
	sockprintf(s, "ATOMS %i\n", mol->Natom);
	for (i = 0; i < mol->Natom; ++i)
		sockprintf(s, "\t%10.4lf %10.4lf %s %i\n", mol->atoms[i].x, mol->atoms[i].y, symbol[mol->atoms[i].Z], mol->atoms[i].charge);

	sockprintf(s, "BONDS %i\n", mol->Nbond);
	for (i = 0; i < mol->Nbond; ++i)
		sockprintf(s, "\t%i %i %i\n", mol->bonds[i].a1, mol->bonds[i].a2, mol->bonds[i].order);

	sockprintf(s, "SMILES \"%s\"\n", mol->smiles);
	printf("\"%s\"", mol->smiles);

	sockprintf(s, "CMPLX %i\n", mol->complexity);

	if (mol->dbname != NULL) {
		sockprintf(s, "NAME \"%s\"\n", mol->dbname);
		printf(" = %s", mol->dbname);
	}

	printf("\n");

	sockprintf(s, "ENDMOL\n\n");
}


void list2trans(struct struct_info *info, struct sock_data *s, struct mol_list *mol_list)
{
	int i;
	assert(s);

	if (mol_list) {
		sockprintf(s, "STARTTRANSFORM\n");
		sockprintf(s, "HEADER\n");
		sockprintf(s, "\tRXN %i\n\tRATING %i\n\tSIMPLIFICATION %i\n", mol_list->rxn_info->rxn+1,mol_list->rxn_info->rating,mol_list->rxn_info->simplification);
		if (mol_list->rxn_info->echo) 
			sockprintf(s, "\tECHO \"%s\"\n", mol_list->rxn_info->echo);

		sockprintf(s, "MOLS %i\n", mol_list->Nmol);
		for (i = 0; i < mol_list->Nmol; ++i)
			mol2trans(info, s, mol_list->mols[i]);
	
		sockprintf(s, "ENDTRANSFORM\n\n");
	}
}


void meta2trans(struct struct_info *info, struct sock_data *s, struct mol_metalist *mol_metalist)
{
	int i;
	assert(s);

	if (mol_metalist) {
		sockprintf(s, "STARTTRANSFORMLIST\n");
		sockprintf(s, "TRANSFORMS %i\n", mol_metalist->Nlist);
		for (i = 0; i < mol_metalist->Nlist; ++i)
			list2trans(info, s, mol_metalist->lists[i]);
	
		sockprintf(s, "ENDTRANSFORMLIST\n\n");
	}
}


int strnncmp(char *buff, char *patt)
{
	return(strnicmp(buff, patt, strlen(patt)));
}



struct mol *trans2mol(struct sock_data *s)
{
	char buff[200];
	int i,j;
	int natom, nbond;
	int Z, charge;
	int a1,a2,type;
	double x,y;
	char *aux;
	int bond;
	struct mol *mol;
	
	sockgets(s, buff, sizeof(buff));
	logsockfile(s, "> %s", buff);
	if (strnncmp(buff, "STARTMOL")) {
		seterr("Expected STARTMOL");
		return(NULL);
	}

	sockgets(s, buff, sizeof(buff));
	logsockfile(s, "> %s", buff);
	if (strnncmp(buff, "ATOMS")) {
		seterr("Expected ATOMS");
		return(NULL);
	}

	mol = calloc(1,sizeof(struct mol));
	natom = atoi(buff+5);

	for (i = 0; i < natom; ++i) {
		sockgets(s, buff, sizeof(buff));
		logsockfile(s, "> %s", buff);
		aux = strtok(buff, " \t");
		if (!aux) {
			seterr("Expected ATOM x");
			destroy_mol(mol);
			return(NULL);
		}
		x = atof(aux);

		aux = strtok(NULL, " \t");
		if (!aux) {
			seterr("Expected ATOM y");
			destroy_mol(mol);
			return(NULL);
		}
		y = atof(aux);

		aux = strtok(NULL, " \t");
		if (!aux || ((Z = atomnum(aux)) == -1)) {
			seterr("Expected ATOM sym");
			destroy_mol(mol);
			return(NULL);
		}

		aux = strtok(NULL, " \t");
		if (!aux) {
			seterr("Expected ATOM charge");
			destroy_mol(mol);
			return(NULL);
		}
		charge = atoi(aux);

		new_atom(mol, Z, charge, x, y, 0);
	}

	sockgets(s, buff, sizeof(buff));
	logsockfile(s, "> %s", buff);
	if (strnncmp(buff, "BONDS")) {
		seterr("Expected BONDS");
		return(NULL);
	}

	nbond = atoi(buff+5);

	for (i = 0; i < nbond; ++i) {
		sockgets(s, buff, sizeof(buff));
		logsockfile(s, "> %s", buff);
		aux = strtok(buff, " \t");
		if (!aux) {
			seterr("Expected BOND a1");
			destroy_mol(mol);
			return(NULL);
		}
		a1 = atoi(aux);

		aux = strtok(NULL, " \t");
		if (!aux) {
			seterr("Expected BOND a2");
			destroy_mol(mol);
			return(NULL);
		}
		a2 = atoi(aux);

		aux = strtok(NULL, " \t");
		if (!aux) {
			seterr("Expected BOND type");
			destroy_mol(mol);
			return(NULL);
		}
		type = atoi(aux);

		bond = new_bond(mol, a1, a2);
		for (j = 1; j < type; ++j)
			inc_bondorder(mol, bond);
	}

	sockgets(s, buff, sizeof(buff));
	if (strnncmp(buff, "ENDMOL")) {
		seterr("Expected ENDMOL");
		return(NULL);
	}

	return(mol);
}

