/* _execute.c
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

#include <setjmp.h>

#ifdef _DEBUG
BOOL debugexec = FALSE;
BOOL debugexpr = FALSE;
BOOL debugatoms = FALSE;
#endif

#ifdef _DEBUG
#define DEBUGEXEC(x)	if (debugexec) printf("EXEC:  "x"\n")
#else
#define	DEBUGEXEC(x)	
#endif 


extern char *reserved_chem[];


void scanner_exec(struct struct_info *info, struct reaction *reaction);
void execstatementlist(struct struct_info *info, struct mol_list *mol_list, struct reaction *reaction);
void execstatement(struct struct_info *info, struct mol_list *mol_list, struct reaction *reaction);
void execforatom_statement(struct struct_info *info, struct mol_list *mol_list, struct reaction *reaction);
void execif_statement(struct struct_info *info, struct mol_list *mol_list, struct reaction *reaction);
int  execadd_statement(struct struct_info *info, struct mol_list *mol_list, struct reaction *reaction);
void execeliminate_statement(struct struct_info *info, struct mol_list *mol_list, struct reaction *reaction);
void execmakebond_statement(struct struct_info *info, struct mol_list *mol_list, struct reaction *reaction);
void execbreakbond_statement(struct struct_info *info, struct mol_list *mol_list, struct reaction *reaction);
void execdone_statement(struct struct_info *info, struct mol_list *mol_list, struct reaction *reaction);
void execassignment_statement(struct struct_info *info, struct mol_list *mol_list, struct reaction *reaction);
int  execexpression(struct struct_info *info, struct mol_list *mol_list, struct reaction *reaction);
void exececho_statement(struct struct_info *info, struct mol_list *mol_list, struct reaction *reaction);
void execincbondorder_statement(struct struct_info *info, struct mol_list *mol_list, struct reaction *reaction);
void execdecbondorder_statement(struct struct_info *info, struct mol_list *mol_list, struct reaction *reaction);
void execsetbondorder_statement(struct struct_info *info, struct mol_list *mol_list, struct reaction *reaction);

void skiptoendif(struct struct_info *info, struct reaction *reaction);
void skiptoelse(struct struct_info *info, struct reaction *reaction);

extern int atomtype[];
jmp_buf env;

void execute(struct struct_info *info, struct mol_list *mol_list, struct reaction *reaction);


void executegp1(struct struct_info *info, struct mol_list *mol_list, struct gpofunc *gpofunc, struct reaction *reaction)
{
	int i, *aux;
	char buff[20];
	int *numatom;

	logfile("Executing reaction %s\n", reaction->name);

	/* assigns A1..A9 funcgrp atoms */
	for (i = 0, aux = gpofunc->atomos; *aux != -1; ++i, ++aux) {
		assert(i < 10);
		sprintf(buff, "A%i", i+1);

		numatom = (int *)getvariabledata(info, buff);
		assert(numatom);

		if (debugatoms)
			printf("%s = %i\n", buff, *aux + 1);

		*numatom = *aux;
	}

	execute(info, mol_list, reaction);
}



void executegp2(struct struct_info *info, struct mol_list *mol_list, struct gpofunc *g1, struct gpofunc *g2, struct path *path, struct reaction *reaction)
{
	int i, *aux;
	char buff[20];
	int *numatom;

	logfile("Executing reaction %s\n", reaction->name);

	/* assigns A1..A9 funcgrp atoms */
	for (i = 0, aux = g1->atomos; *aux != -1; ++i, ++aux) {
		assert(i < 10);
		sprintf(buff, "A%i", i+1);

		numatom = (int *)getvariabledata(info, buff);
		assert(numatom);

		if (debugatoms)
			printf("%s = %i\n", buff, *aux + 1);

		*numatom = *aux;
	}

	/* assigns B1..B9 funcgrp atoms */
	for (i = 0, aux = g2->atomos; *aux != -1; ++i, ++aux) {
		assert(i < 10);
		sprintf(buff, "B%i", i+1);

		numatom = (int *)getvariabledata(info, buff);
		assert(numatom);

		if (debugatoms)
			printf("%s = %i\n", buff, *aux + 1);

		*numatom = *aux;
	}

	/* assigns P1..P9 path atoms */
	for (i = 0; i < path->len; ++i) {
		assert(i < 10);
		sprintf(buff, "P%i", i+1);

		numatom = (int *)getvariabledata(info, buff);
		assert(numatom);

		if (debugatoms)
			printf("%s = %i\n", buff, path->atoms[i] + 1);

		*numatom = path->atoms[i];
	}

	execute(info, mol_list, reaction);
}


void executering(struct struct_info *info, struct mol_list *mol_list, struct ringpath *ring, struct reaction *reaction)
{
	int *ringpath;
	int *i,j;
	char buff[20];
	int *numatom;

	logfile("Executing reaction %s\n", reaction->name);

	for (i = ring->ringpos; *i; ++i) {
		ringpath = ringrotation(ring, *i);

		/* assigns R1..R9 ring atoms */
		for (j = 0; j < ring->len-1; ++j) {
			sprintf(buff, "R%i", j+1);
			numatom = (int *)getvariabledata(info, buff);
			assert(numatom);

			if (debugatoms)
				printf("%s = %i\n", buff, ringpath[j] + 1);

			*numatom = ringpath[j];
		}
		execute(info, mol_list, reaction);

		free(ringpath);
	}
}


void executemring(struct struct_info *info, struct mol *mol, struct mol_list *mol_list, struct multiring *mring, struct reaction *reaction)
{
	int *ringpath;
	int i,j;
	char buff[20];
	int *numatom;
	struct ringpath *ring;

	logfile("Executing reaction %s\n", reaction->name);

	for (i = 0; i < mring->Nring; ++i) {
		ring = &mol->rings[mring->rings[i].ringnum];
		ringpath = ringrotation(ring, mring->rings[i].rotation);

		/* assigns R1..R9, S1..S9, etc. ring atoms */
		for (j = 0; j < ring->len-1; ++j) {
			sprintf(buff, "%c%i", 'R'+i, j+1);
			numatom = (int *)getvariabledata(info, buff);
			assert(numatom);

			if (debugatoms)
				printf("%s = %i\n", buff, ringpath[j] + 1);

			*numatom = ringpath[j];
		}

		free(ringpath);
	}

	execute(info, mol_list, reaction);
}


void executefga(struct struct_info *info, struct mol_list *mol_list, int targetatom, struct reaction *reaction)
{
	int *numatom;

	logfile("Executing reaction %s\n", reaction->name);

	/* assigns A1 atom */
	numatom = (int *)getvariabledata(info, "A1");
	assert(numatom);

	if (debugatoms)
		printf("A1 = %i\n", targetatom + 1);

	*numatom = targetatom;

	execute(info, mol_list, reaction);
}

void executefg0(struct struct_info *info, struct mol_list *mol_list, int atom1, int atom2, struct reaction *reaction)
{
	int *numatom;

	logfile("Executing reaction %s\n", reaction->name);

	/* assigns A1 atom */
	numatom = (int *)getvariabledata(info, "A1");
	assert(numatom);
	if (debugatoms)
		printf("A1 = %i\n", atom1 + 1);
	*numatom = atom1;

	/* assigns A2 atom */
	numatom = (int *)getvariabledata(info, "A2");
	assert(numatom);
	if (debugatoms)
		printf("A1 = %i\n", atom2 + 1);
	*numatom = atom2;

	execute(info, mol_list, reaction);
}



void execute(struct struct_info *info, struct mol_list *mol_list, struct reaction *reaction)
{
	int *numatom;

	numatom = (int *)getvariabledata(info, "RATING");
	assert(numatom);
	*numatom = reaction->rating;

	/* create working mol */
	append_mol(mol_list, moldup(mol_list->mols[0]));
	mol_list->rxn_info = realloc(mol_list->rxn_info, mol_list->Nmol * sizeof(struct rxn_info));
	assert(mol_list->rxn_info);
	info->echo = init_log(1024);

	reaction->ip = 0;

	/* pump first instruction into info->token */
	scanner_exec(info, reaction);
	
	if (!setjmp(env)) 
		execstatementlist(info, mol_list, reaction);

	/* destroy working mol */
	destroy_mol(mol_list->mols[mol_list->Nmol-1]);
	info->echo = destroy_log(info->echo);
//	free(mol_list->mols[mol_list->Nmol-1]);
	--mol_list->Nmol;

	//printf("END\n\n");
}

static int lasttok = -1;

void scanner_exec(struct struct_info *info, struct reaction *reaction)
{
	memset(&info->token, 0, sizeof(struct token));
	if (reaction->ip >= reaction->mechsize)
		longjmp(env, 1);
	info->token.tipo = reaction->mechanism[reaction->ip++];

	if (debugexec) {

		if (lasttok != TOK_NUMERO) {
			printf("SCANEX: %i %s\n", info->token.tipo, tokenstring(info, info->token.tipo));
			lasttok = info->token.tipo;
		} else {
			printf("SCANEX: %i\n", info->token.tipo);
			lasttok = -1;
		}
		getchar();
	}

}

int scanner_exec_ext(struct struct_info *info, struct reaction *reaction)
{
	int ret = 0;

	scanner_exec(info, reaction);
	switch (info->token.tipo) {
		case TOK_NUMERO:
			ret = reaction->mechanism[reaction->ip++];		
			lasttok = -1;
			break;

		default:
			if (info->token.tipo >= TOK_ECHOSTRING) {
				ret = info->token.tipo - TOK_ECHOSTRING;
				info->token.tipo = TOK_STRING;
			} else if (info->token.tipo > TOK_VARIABLE) {
				ret = info->token.tipo - TOK_VARIABLE;
				info->token.tipo = TOK_VARIABLE;
			}
			break;
	}
	return(ret);
}


void execstatementlist(struct struct_info *info, struct mol_list *mol_list, struct reaction *reaction)
{
	DEBUGEXEC("execstatementlist");

	execstatement(info, mol_list, reaction);

	while (tokenisinlist(info, RW_FORATOM, RW_IF, RW_ADD, RW_BREAKBOND, RW_ELIMINATE, RW_MAKEBOND, RW_DONE, RW_ECHO, RW_INCBONDORDER, RW_DECBONDORDER, RW_SETBONDORDER, TOK_IGUAL,  0)) 
		execstatement(info, mol_list, reaction);
}

void execstatement(struct struct_info *info, struct mol_list *mol_list, struct reaction *reaction)
{
	DEBUGEXEC("execstatement");

	switch (info->token.tipo) {
		case RW_FORATOM:
			execforatom_statement(info, mol_list, reaction);
			break;
		case RW_IF:
			execif_statement(info, mol_list, reaction);
			break;
		case RW_ADD:
			execadd_statement(info, mol_list, reaction);
			break;
		case RW_BREAKBOND:
			execbreakbond_statement(info, mol_list, reaction);
			break;
		case RW_ELIMINATE:
			execeliminate_statement(info, mol_list, reaction);
			break;
		case RW_MAKEBOND:
			execmakebond_statement(info, mol_list, reaction);
			break;
		case RW_INCBONDORDER:
			execincbondorder_statement(info, mol_list, reaction);
			break;
		case RW_DECBONDORDER:
			execdecbondorder_statement(info, mol_list, reaction);
			break;
		case RW_SETBONDORDER:
			execsetbondorder_statement(info, mol_list, reaction);
			break;
		case RW_DONE:
			execdone_statement(info, mol_list, reaction);
			break;
		case RW_ECHO:
			exececho_statement(info, mol_list, reaction);
			break;
		case TOK_IGUAL:
			execassignment_statement(info, mol_list, reaction);
			break;
		default:
			err_msg("Expected statement");
			break;
	}
}

void execforatom_statement(struct struct_info *info, struct mol_list *mol_list, struct reaction *reaction)
{
	int	*varatom;
	int *fromatom;
	struct atom *atom;
	struct bond *bond;
	int	startip;
	int i;
	struct mol *mol = mol_list->mols[mol_list->Nmol - 1];

	DEBUGEXEC("foratom_statement");

/*
	RW_FORATOM
	variable (VAR)
	variable (FROM)
		statementlist
	RW_NEXT
*/
	scanner_exec(info, reaction);	
	varatom = (int *)&info->variables[info->token.tipo - TOK_VARIABLE].data;
	scanner_exec(info, reaction);	
	fromatom = (int *)&info->variables[info->token.tipo - TOK_VARIABLE].data;

	startip = reaction->ip;

	atom = &mol->atoms[*fromatom];

	for (i = 0; i < atom->Nbond; ++i) {
		bond = &mol->bonds[atom->bonds[i]];
		*varatom = (bond->a1 == *fromatom) ? bond->a2 : bond->a1;

		reaction->ip = startip;
		scanner_exec(info, reaction);

		execstatementlist(info, mol_list, reaction);
		mol = mol_list->mols[mol_list->Nmol - 1];
		atom = &mol->atoms[*fromatom];


		assert (info->token.tipo == RW_NEXT);
	}

	scanner_exec(info, reaction);
}

void execif_statement(struct struct_info *info, struct mol_list *mol_list, struct reaction *reaction)
{
	DEBUGEXEC("if_statement");

/*
	RW_IF
		expr
		statementlist
	[RW_ELSE
		statementlist]
	RW_ENDIF
*/

	if (execexpression(info, mol_list, reaction)) {
		assert(info->token.tipo == RW_THEN);
		scanner_exec(info, reaction);
		execstatementlist(info, mol_list, reaction);
		if (info->token.tipo == RW_ELSE)
			skiptoendif(info, reaction);
	} else {
		skiptoelse(info, reaction);
 		if (info->token.tipo == RW_ELSE) {
			scanner_exec(info, reaction);
			execstatementlist(info, mol_list, reaction);
		}
	}
	assert(info->token.tipo == RW_ENDIF);
	scanner_exec(info, reaction);
}

int execadd_statement(struct struct_info *info, struct mol_list *mol_list, struct reaction *reaction)
{
	int atomv, atomc, ret;
	struct mol *mol = mol_list->mols[mol_list->Nmol - 1];

	DEBUGEXEC("add_statement");

/*
	RW_ADD
		ATOM_VAR
		ATOM_CONST
*/

	atomv = scanner_exec_ext(info, reaction);
	assert(info->token.tipo == TOK_VARIABLE);

	scanner_exec(info, reaction);
	atomc = info->token.tipo;
	assert((atomc >= TOK_CHEMRESERVED) && (atomc < TOK_VARIABLE));

	atomc = atomnum(reserved_chem[atomc - TOK_CHEMRESERVED]);
	assert(atomc != -1);

	ret = sprout_atom(mol, info->variables[atomv].data.numatom, atomc);

	scanner_exec(info, reaction);

	return(ret);
}

void execbreakbond_statement(struct struct_info *info, struct mol_list *mol_list, struct reaction *reaction)
{
	int atom1, atom2;
	int bond;
	struct mol *mol = mol_list->mols[mol_list->Nmol - 1];

/*
	RW_BREAKBOND
		ATOM1
		ATOM2
*/

  	DEBUGEXEC("breakbond_statement");

	atom1 = scanner_exec_ext(info, reaction);
	assert(info->token.tipo == TOK_VARIABLE);
	atom2 = scanner_exec_ext(info, reaction);
	assert(info->token.tipo == TOK_VARIABLE);

	bond = are_bonded(mol, info->variables[atom1].data.numatom, info->variables[atom2].data.numatom);
	if (bond == -1)
		err_exec(info, "Atoms are not bonded");
	breakbond(mol, bond);

	scanner_exec(info, reaction);
}

void execeliminate_statement(struct struct_info *info, struct mol_list *mol_list, struct reaction *reaction)
{
	int atomv, atomc;
	struct mol *mol = mol_list->mols[mol_list->Nmol - 1];

	DEBUGEXEC("eliminate_statement");

/*
	RW_ELIMINATE
		ATOM_VAR
		ATOM_CONST
*/

	atomv = scanner_exec_ext(info, reaction);
	assert(info->token.tipo == TOK_VARIABLE);

	scanner_exec(info, reaction);
	atomc = info->token.tipo;
	assert((atomc >= TOK_CHEMRESERVED) && (atomc < TOK_VARIABLE));

	atomc = atomnum(reserved_chem[atomc - TOK_CHEMRESERVED]);
	assert(atomc != -1);

	//eliminate_atom(mol, info->variables[atomv].data.numatom, atomc);

	scanner_exec(info, reaction);
}

void execmakebond_statement(struct struct_info *info, struct mol_list *mol_list, struct reaction *reaction)
{
	int atom1, atom2;
	struct mol *mol = mol_list->mols[mol_list->Nmol - 1];

/*
	RW_MAKEBOND
		ATOM1
		ATOM2
*/

	DEBUGEXEC("makebond_statement");

	atom1 = scanner_exec_ext(info, reaction);
	assert(info->token.tipo == TOK_VARIABLE);
	atom2 = scanner_exec_ext(info, reaction);
	assert(info->token.tipo == TOK_VARIABLE);

	new_bond(mol, info->variables[atom1].data.numatom, info->variables[atom2].data.numatom);

	scanner_exec(info, reaction);
}

void execincbondorder_statement(struct struct_info *info, struct mol_list *mol_list, struct reaction *reaction)
{
	int atom1, atom2;
	int bond;
	struct mol *mol = mol_list->mols[mol_list->Nmol - 1];

/*
	RW_INCBONDORDER
		ATOM1
		ATOM2
*/

	DEBUGEXEC("incbondorder_statement");

	atom1 = scanner_exec_ext(info, reaction);
	assert(info->token.tipo == TOK_VARIABLE);
	atom2 = scanner_exec_ext(info, reaction);
	assert(info->token.tipo == TOK_VARIABLE);

	if ((bond = are_bonded(mol, info->variables[atom1].data.numatom, info->variables[atom2].data.numatom)) != -1)
		inc_bondorder(mol, bond);

	scanner_exec(info, reaction);
}

void execdecbondorder_statement(struct struct_info *info, struct mol_list *mol_list, struct reaction *reaction)
{
	int atom1, atom2;
	int bond;
	struct mol *mol = mol_list->mols[mol_list->Nmol - 1];

/*
	RW_DECBONDORDER
		ATOM1
		ATOM2
*/

	DEBUGEXEC("decbondorder_statement");

	atom1 = scanner_exec_ext(info, reaction);
	assert(info->token.tipo == TOK_VARIABLE);
	atom2 = scanner_exec_ext(info, reaction);
	assert(info->token.tipo == TOK_VARIABLE);

	if ((bond = are_bonded(mol,info->variables[atom1].data.numatom, info->variables[atom2].data.numatom)) != -1)
		dec_bondorder(mol, bond);

	scanner_exec(info, reaction);
}

void execsetbondorder_statement(struct struct_info *info, struct mol_list *mol_list, struct reaction *reaction)
{
	int atom1, atom2;
	int order;
	int bond;
	struct mol *mol = mol_list->mols[mol_list->Nmol - 1];

/*
	RW_SETBONDORDER
		ATOM1
		ATOM2
*/

	DEBUGEXEC("setbondorder_statement");

	atom1 = scanner_exec_ext(info, reaction);
	assert(info->token.tipo == TOK_VARIABLE);
	atom2 = scanner_exec_ext(info, reaction);
	assert(info->token.tipo == TOK_VARIABLE);
	order = scanner_exec_ext(info, reaction);
	assert((info->token.tipo == TOK_VARIABLE) || (info->token.tipo == TOK_NUMERO));

	if ((bond = are_bonded(mol,info->variables[atom1].data.numatom, info->variables[atom2].data.numatom)) != -1)
		set_bondorder(mol, bond, order);

	scanner_exec(info, reaction);
}




void execdone_statement(struct struct_info *info, struct mol_list *mol_list, struct reaction *reaction)
{
//	char fname[_MAX_PATH];
	static int result = 0;
	struct mol *mol = mol_list->mols[mol_list->Nmol - 1];

	info, reaction;

	DEBUGEXEC("done_statement");

	scanner_exec(info, reaction);
	if (info->token.tipo == TOK_PARENABRE) {
		int atomv;
		int *used;

		used = calloc(mol->Natom, sizeof(int));
		assert(used);

		//printf("Borrando atomos: ");


		atomv = scanner_exec_ext(info, reaction);
		while (info->token.tipo != TOK_PARENCIERRA) {
			//printf("%i ", info->variables[atomv].data.numatom);
			_markatoms(mol, used, info->variables[atomv].data.numatom, 1);
			atomv = scanner_exec_ext(info, reaction);
		} 

		deleteatoms(mol, used);

		printf("\n");
		free(used);
		scanner_exec(info, reaction);
	}



	logfile(" -> Done, rating = %i\n",  *(int *)getvariabledata(info, "RATING"));
//	sprintf(fname, "RESULT%02i.MOL", ++result);
//	printf(" -> Writing result to %s, rating = %i\n", fname, *(int *)getvariabledata(info, "RATING"));
//	writemolfile(fname, mol);

	mol_list->rxn_info[mol_list->Nmol-1].rating = *(int *)getvariabledata(info, "RATING");
	mol_list->rxn_info[mol_list->Nmol-1].simplification = 0;
	mol_list->rxn_info[mol_list->Nmol-1].rxn = reaction - info->reaction;
	mol_list->rxn_info[mol_list->Nmol-1].echo = log2str(info->echo);
	info->echo = init_log(1024);

	/* create new working mol */
	append_mol(mol_list, moldup(mol_list->mols[0]));
	mol_list->rxn_info = realloc(mol_list->rxn_info, mol_list->Nmol * sizeof(struct rxn_info));
	assert(mol_list->rxn_info);
//	mol_list->rxn_info = realloc(mol_list->rxn_info, mol_list->Nmol*sizeof(struct rxn_info));
}

void exececho_statement(struct struct_info *info, struct mol_list *mol_list, struct reaction *reaction)
{
	int pos;
	struct mol *mol = mol_list->mols[mol_list->Nmol - 1];

	info, reaction;

	DEBUGEXEC("echo_statement");

	pos = scanner_exec_ext(info, reaction);
	assert(info->token.tipo == TOK_STRING);
	assert(pos < info->numstrings);

	printf(" ->> %s\n", info->strings[pos]);
	logadd(info->echo, info->strings[pos]);
	logadd(info->echo, "\\n");

	scanner_exec(info, reaction);
}

void execassignment_statement(struct struct_info *info, struct mol_list *mol_list, struct reaction *reaction)
{
	int res;
	int val;
	struct variable *var;

	DEBUGEXEC("assignment_statement");

/*	
	TOK_IGUAL
	expresion
	TOK_IGUAL
	variable
*/

	res = execexpression(info, mol_list, reaction);
	
 	val = scanner_exec_ext(info, reaction);
	assert((val >= 0) && (val < info->numvariables));

	var = &info->variables[val];

	switch(var->type) {
		case V_NUM:
			var->data.num = res;
			break;
		case V_ATOM:
			var->data.numatom = res;
			break;
	}

	scanner_exec(info, reaction);
}


int exec_ringsizefunction(struct mol_list *mol_list, int a1, int a2)
{
	struct mol *mol = mol_list->mols[mol_list->Nmol - 1];
	int i;
	int size = 999;

	DEBUGEXEC("ringsize_function");

	for (i = 0; i < mol->Nring; ++i)
		if (bs_isset(mol->rings[i].nodes, a1) && bs_isset(mol->rings[i].nodes, a2))
			if (mol->rings[i].len-1 < size)
				size = mol->rings[i].len-1;

	return(size == 999 ? 0 : size);
}

int exec_nHfunction(struct mol_list *mol_list, int a1)
{
	struct mol *mol = mol_list->mols[mol_list->Nmol - 1];

	DEBUGEXEC("nH");

	return(mol->atoms[a1].nH);
}

int exec_addfunction(struct mol_list *mol_list, int a1, int a2)
{
	int atomc = a2;
	struct mol *mol = mol_list->mols[mol_list->Nmol - 1];

	assert((atomc >= TOK_CHEMRESERVED) && (atomc < TOK_VARIABLE));

	atomc = atomnum(reserved_chem[atomc - TOK_CHEMRESERVED]);
	assert(atomc != -1);

	return(sprout_atom(mol, a1, atomc));
}

int exec_getbondorderfunction(struct mol_list *mol_list, int a1, int a2)
{
	struct mol *mol = mol_list->mols[mol_list->Nmol - 1];

	return(bond_order(mol, a1, a2));
}




int  execexpression(struct struct_info *info, struct mol_list *mol_list, struct reaction *reaction)
{
	int val;
	int stack[100];
	int tos;
	struct variable *var;
	int end = 0;

	tos = 0;

	do {
  		val = scanner_exec_ext(info, reaction);
		switch(info->token.tipo) {
			case TOK_NUMERO:
				stack[tos++] = val;
				#ifdef _DEBUG
				if (debugexpr) printf("EXPR:  NUM = %i\n", val);
				#endif
				break;

			case TOK_VARIABLE:
				assert((val >= 0) && (val < info->numvariables));
				var = &info->variables[val];
				switch(var->type) {
					case V_NUM:
						stack[tos++] = var->data.num;
						#ifdef _DEBUG
						if (debugexpr) printf("EXPR:  NUM = %i\n", var->data.num);
						#endif
						break;
					case V_ATOM:
						stack[tos++] = var->data.numatom;
						#ifdef _DEBUG
						if (debugexpr) printf("EXPR:  ATOM = %i\n", var->data.numatom);
						#endif
						break;
					default:
						err_exec(info, "Invalid type");
						break;
				}
				break;

			case RW_IS: {
					struct mol *mol = mol_list->mols[mol_list->Nmol - 1];
					assert(tos >= 2);
					stack[tos-2] = isatomtype(mol, stack[tos-2], stack[tos-1]);
					#ifdef _DEBUG
					if (debugexpr) printf("EXPR:  IS = %i\n", stack[tos-2]);
					#endif
					--tos;
				}
				break;


			case RW_ISNOT: {
					struct mol *mol = mol_list->mols[mol_list->Nmol - 1];
					assert(tos >= 2);
					stack[tos-2] = !isatomtype(mol, stack[tos-2], stack[tos-1]);
					#ifdef _DEBUG
					if (debugexpr) printf("EXPR:  ISNOT = %i\n", stack[tos-2]);
					#endif
					--tos;
				}
				break;

			case RW_ISEQ: {
					struct mol *mol = mol_list->mols[mol_list->Nmol - 1];
					assert(tos >= 2);
					stack[tos-2] = mol->atoms[stack[tos-2]].nclass == mol->atoms[stack[tos-1]].nclass;
					#ifdef _DEBUG
					if (debugexpr) printf("EXPR:  ISEQ = %i\n", stack[tos-2]);
					#endif
					--tos;
				}
				break;

			case TOK_IGUAL: 
				if (tos >= 2) {
					stack[tos-2] = (stack[tos-2] == stack[tos-1]);
					#ifdef _DEBUG
					if (debugexpr) printf("EXPR:  EQ = %i\n", stack[tos-2]);
					#endif
					--tos;
				} else 
					end = 1;
				break;

			case TOK_LT: 
				assert(tos >= 2);
				stack[tos-2] = (stack[tos-2] < stack[tos-1]);
				#ifdef _DEBUG
				if (debugexpr) printf("EXPR:  LT = %i\n", stack[tos-2]);
				#endif
				--tos;
				break;

			case TOK_LE: 
				assert(tos >= 2);
				stack[tos-2] = (stack[tos-2] <= stack[tos-1]);
				#ifdef _DEBUG
				if (debugexpr) printf("EXPR:  LE = %i\n", stack[tos-2]);
				#endif
				--tos;
				break;

			case TOK_NE: 
				assert(tos >= 2);
				stack[tos-2] = (stack[tos-2] != stack[tos-1]);
				#ifdef _DEBUG
				if (debugexpr) printf("EXPR:  NE = %i\n", stack[tos-2]);
				#endif
				--tos;
				break;

			case TOK_GE: 
				assert(tos >= 2);
				stack[tos-2] = (stack[tos-2] >= stack[tos-1]);
				#ifdef _DEBUG
				if (debugexpr) printf("EXPR:  GE = %i\n", stack[tos-2]);
				#endif
				--tos;
				break;

			case TOK_GT: 
				assert(tos >= 2);
				stack[tos-2] = (stack[tos-2] > stack[tos-1]);
				#ifdef _DEBUG
				if (debugexpr) printf("EXPR:  GT = %i\n", stack[tos-2]);
				#endif
				--tos;
				break;

			case RW_OR: 
				assert(tos >= 2);
				stack[tos-2] = (stack[tos-2] || stack[tos-1]);
				#ifdef _DEBUG
				if (debugexpr) printf("EXPR:  OR = %i\n", stack[tos-2]);
				#endif
				--tos;
				break;

			case TOK_ADD: 
				assert(tos >= 2);
				stack[tos-2] = (stack[tos-2] + stack[tos-1]);
				#ifdef _DEBUG
				if (debugexpr) printf("EXPR:  ADD = %i\n", stack[tos-2]);
				#endif
				--tos;
				break;

			case TOK_SUB: 
				assert(tos >= 2);
				stack[tos-2] = (stack[tos-2] - stack[tos-1]);
				#ifdef _DEBUG
				if (debugexpr) printf("EXPR:  SUB = %i\n", stack[tos-2]);
				#endif
				--tos;
				break;

			case RW_AND: 
				assert(tos >= 2);
				stack[tos-2] = (stack[tos-2] && stack[tos-1]);
				#ifdef _DEBUG
				if (debugexpr) printf("EXPR:  AND = %i\n", stack[tos-2]);
				#endif
				--tos;
				break;

			case TOK_MUL: 
				assert(tos >= 2);
				stack[tos-2] = (stack[tos-2] * stack[tos-1]);
				#ifdef _DEBUG
				if (debugexpr) printf("EXPR:  MUL = %i\n", stack[tos-2]);
				#endif
				--tos;
				break;

			case TOK_DIV: 
				assert(tos >= 2);
				if (!stack[tos-1])
					err_exec(info, "Division by zero");
				stack[tos-2] = (stack[tos-2] / stack[tos-1]);
				#ifdef _DEBUG
				if (debugexpr) printf("EXPR:  DIV = %i\n", stack[tos-2]);
				#endif
				--tos;
				break;
			
			case RW_NOT:
				assert(tos >= 1);
				stack[tos-1] = !stack[tos-1];
				#ifdef _DEBUG
				if (debugexpr) printf("EXPR:  NOT = %i\n", stack[tos-1]);
				#endif
				break;

			case RW_RINGSIZE:
				assert(tos >= 2);
				stack[tos-2] = exec_ringsizefunction(mol_list, stack[tos-2], stack[tos-1]);
				#ifdef _DEBUG
				if (debugexpr) printf("EXPR:  RINGSIZE = %i\n", stack[tos-2]);
				#endif
				--tos;
				break;

			case RW_NH:
				assert(tos >= 1);
				stack[tos-1] = exec_nHfunction(mol_list, stack[tos-1]);
				#ifdef _DEBUG
				if (debugexpr) printf("EXPR:  NH = %i\n", stack[tos-1]);
				#endif
				break;

			case RW_ADD: 
				assert(tos >= 2);
				stack[tos-2] = exec_addfunction(mol_list, stack[tos-2], stack[tos-1]);
				#ifdef _DEBUG
				if (debugexpr) printf("EXPR:  ADD = %i\n", stack[tos-2]);
				#endif
				--tos;
				break;

			case RW_GETBONDORDER:
				assert(tos >= 2);
				stack[tos-2] = exec_getbondorderfunction(mol_list, stack[tos-2], stack[tos-1]);
				#ifdef _DEBUG
				if (debugexpr) printf("EXPR:  GETBONDORDER = %i\n", stack[tos-2]);
				#endif
				--tos;
				break;

			default:
				if (tokenisinarray(info, atomtype))
					stack[tos++] = info->token.tipo;
				else 
					end = 1;
				break;
		}
	} while (!end);

	assert(tos == 1);

	#ifdef _DEBUG
	if (debugexpr) printf("EXPR:  RESULT = %i\n", stack[0]);
	#endif

	return(stack[0]);
}



void skiptoendif(struct struct_info *info, struct reaction *reaction)
{
	int ifs = 1;

	DEBUGEXEC(">>> SKIP TO ENDIF");
	do {
		switch(info->token.tipo) {
			case RW_IF:
				++ifs;
				break;
			case RW_ENDIF:
				--ifs;
				break;
		}
		if (ifs)
			scanner_exec_ext(info, reaction);
	} while (ifs);
	DEBUGEXEC(">>> DONE SKIP TO ENDIF");
}

void skiptoelse(struct struct_info *info, struct reaction *reaction)
{
	int ifs = 1;

	DEBUGEXEC(">>> SKIP TO ELSE OR ENDIF");
	do {
		switch(info->token.tipo) {
			case RW_IF:
				++ifs;
				break;
			case RW_ENDIF:
				--ifs;
				break;
			case RW_ELSE:
				if (ifs == 1)
					--ifs;
				break;
		}
		if (ifs)
			scanner_exec_ext(info, reaction);
	} while (ifs);
	DEBUGEXEC(">>> DONE SKIP TO ELSE OR ENDIF");
}

