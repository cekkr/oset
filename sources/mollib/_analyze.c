/* caoscomp.c
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
#include <limits.h>
#include <setjmp.h>

void try_gp1(struct struct_info *info, struct mol *mol, struct mol_list *mol_list, int g1, int rxn);
void try_gp2(struct struct_info *info, struct mol *mol, struct mol_list *mol_list, int g1, int rxn);
void try_fga(struct struct_info *info, struct mol *mol, struct mol_list *mol_list, int targetatom, int rxn);
void try_gp0(struct struct_info *info, struct mol *mol, struct mol_list *mol_list, int atom1, int atom2, int rxn);
void try_ring(struct struct_info *info, struct mol *mol, struct mol_list *mol_list, int g1, int rxn);
void try_mring(struct struct_info *info, struct mol *mol, struct mol_list *mol_list, int ring, int rxn);
struct mol_metalist *postprocess_mol_list(struct struct_info *info, struct mol_list *list);
int compstring(void *a1, void *a2);

extern jmp_buf mark;

struct struct_info * init_comp_info()
{
	struct struct_info *info;
	char buff[10];
	int i;

	init_primes();

	info = calloc(1, sizeof(struct struct_info));
	assert(info);

	info->line = 0;
	if (setjmp(mark) == 0) {
		// reads functional groups
		if (initscanner(info, "funcgrp.chm")) {
			while (tokenis(info, TOK_ID))
				readfuncgrp(info);
			if (!tokenis(info, TOK_EOF))
				match(info, TOK_ID);
		}
	} else
		logfile("Parse error in funcgrp.chm\n");

	// reads smiles db
	info->line = 0;
	if (setjmp(mark) == 0) {
		if (initscanner(info, "sm.chm")) {
			while (tokenis(info, TOK_STRING))
				readsmiledb(info);
			if (!tokenis(info, TOK_EOF))
				match(info, TOK_STRING);

			sortsmiledb(info);
		}
	} else
		logfile("Parse error in sm.chm\n");
	deinitscanner(info);

	// reads rings db
	info->line = 0;
	if (setjmp(mark) == 0) {
		if (initscanner(info, "rings.chm")) {
			while (tokenis(info, TOK_ID))
				readringgrp(info);
			if (!tokenis(info, TOK_EOF))
				match(info, TOK_ID);
		}
	} else
		logfile("Parse error in rings.chm\n");
	deinitscanner(info);


	createvariable(info, "RATING", V_NUM);
	for (i = 1; i <= 9; ++i) {
		sprintf(buff, "A%i", i);	
		createvariable(info, buff, V_ATOM);
		sprintf(buff, "B%i", i);	
		createvariable(info, buff, V_ATOM);
		sprintf(buff, "P%i", i);	
		createvariable(info, buff, V_ATOM);
		sprintf(buff, "R%i", i);	
		createvariable(info, buff, V_ATOM);
		sprintf(buff, "S%i", i);	
		createvariable(info, buff, V_ATOM);
		sprintf(buff, "T%i", i);	
		createvariable(info, buff, V_ATOM);
		sprintf(buff, "U%i", i);	
		createvariable(info, buff, V_ATOM);
	}

	// reads reaction database
	info->line = 0;
	if (initscanner(info,"rxns.chm")) {
		while (tokenis(info, RW_RXN))
			reaction(info);
	}
	deinitscanner(info);

	// read smilecas database
	readsmilecas_index(info);

	info->cplx_params = init_cplx_params();


	return(info);
}



/* 1- classifies the atoms in a molecule
 * 2- finds funcitional groups
 * 3- finds rings (not yet)
 * 4- Canonicalizes (not yet)
 * returns zero if no error and non-zero otherwise (not yet)
 * *mol must be allocated and consistent
 * *err may be null; if not, the chemical error log will be stored there by classify_atoms()
 * *info may be null; if it is null, this function does not find functional groups
 * else it must be allocated and contain functional group information:
 * call init_compinfo() to initialize
 */
int parse_mol(struct struct_info *info, struct mol *mol, struct chem_err *err)
{	
//	int i, j;

	
	assert(mol);

	find_rings(mol);
	detect_aromaticity(mol);
	classify_rings(info, mol);

	canonicalize(mol);
	if(mol->smiles)
		free(mol->smiles);

	mol->smiles = mol2smiles(mol);
	mol->dbname = findsmiledb(info, mol->smiles);

	classify_atoms(mol, err);

	calc_formula(mol);

	if (info) {
		busca_funcionales(info, mol);
/*		printf("\n\nGrupos funcionales\n");
		if(debug_mol) {
			for (i = 0; i < mol->Nfunc; ++i) {
				printf("%i (", mol->gpofunc[i].tipo);		
				for (j = 0; mol->gpofunc[i].atomos[j] != -1; ++j)
					printf("%i ", mol->gpofunc[i].atomos[j] + 1);
				printf(")\n");									 
			}      
		}              */
	}

	mol->complexity = mol_complexity(mol, info);

	return(0);
}






struct mol_metalist *analyze_mol(struct struct_info *info, struct mol *mol)
{
	int i,j;
	int file = 0;
	struct mol_list *mol_list;
	struct mol_metalist *metalist;

	mol_list = new_mol_list(NULL);
	append_mol(mol_list, moldup(mol));
	
	for (i = 0; i < mol->Nfunc; ++i) {
		//printf("\n\nAnalyzing funcgrp %i\n", mol->gpofunc[i].tipo);
		for (j = 0; j < info->numreact; ++j) {
			switch (info->reaction[j].type) {
				case RW_GP1:  
					try_gp1(info, mol, mol_list, i, j);
					break;

				case RW_GP2:
					try_gp2(info, mol, mol_list, i, j);
					break;

			}
		}
	}

	for (i = 0; i < mol->Nring; ++i) {
		for (j = 0; j < info->numreact; ++j) {
			switch (info->reaction[j].type) {
				case RW_RING:
					try_ring(info, mol, mol_list, i, j);
					break;
			}
		}
	}

	for (i = 0; i < mol->Nmring; ++i) {
		for (j = 0; j < info->numreact; ++j) {
			switch (info->reaction[j].type) {
				case RW_RING:
					try_mring(info, mol, mol_list, i, j);
					break;
			}
		}
	}


	metalist = postprocess_mol_list(info, mol_list);
	destroy_mol_list(mol_list);
	return(metalist);
}



struct mol_metalist *analyze_molfgi(struct struct_info *info, struct mol *mol, int targetatom, int gprec)
{
	int i,j;
	int file = 0;
	struct mol_list *mol_list;
	struct mol_metalist *metalist;
	int *x, *k;
	BOOL doexec;

	mol_list = new_mol_list(moldup(mol));
	
	for (i = 0; i < mol->Nfunc; ++i) {
		for (x = mol->gpofunc[i].atomos; x && (*x != -1); ++x) {
			if (*x == targetatom) {
				for (j = 0; j < info->numreact; ++j) {
					if ((info->reaction[j].gprec) && ((info->reaction[j].type == RW_FGI1) || (info->reaction[j].type == RW_FGI2))) {
						doexec = gprec == -1;	

						for (k = info->reaction[j].gprec; !doexec && k && (*k != -1); ++k) 
							doexec = gprec == *k;

						if (doexec) {
							switch (info->reaction[j].type) {
								case RW_FGI1:  
									try_gp1(info, mol, mol_list, i, j);
									break;

								case RW_FGI2: {
										int *temp;
										
										try_gp2(info, mol, mol_list, i, j);
										temp = info->reaction[j].g1;
										info->reaction[j].g1 = info->reaction[j].g2;
										info->reaction[j].g2 = temp;

										try_gp2(info, mol, mol_list, i, j);
										temp = info->reaction[j].g1;
										info->reaction[j].g1 = info->reaction[j].g2;
										info->reaction[j].g2 = temp;
									}
									break;
							}
						}
					}
				}
			}
		}

		//printf("\n\nAnalyzing funcgrp %i\n", mol->gpofunc[i].tipo);
	}

	metalist = postprocess_mol_list(info, mol_list);
	destroy_mol_list(mol_list);
	return(metalist);
}


struct mol_metalist *analyze_molfga(struct struct_info *info, struct mol *mol, int targetatom, int gprec)
{
	int j;
	struct mol_list *mol_list;
	struct mol_metalist *metalist;
	int *k;
	BOOL doexec;

	mol_list = new_mol_list(moldup(mol));
	
	for (j = 0; j < info->numreact; ++j) {
		if ((info->reaction[j].gprec) && (info->reaction[j].type == RW_FGA)) {
			doexec = gprec == -1;	

			for (k = info->reaction[j].gprec; !doexec && k && (*k != -1); ++k) 
				doexec = gprec == *k;

			if (doexec) 
				try_fga(info, mol, mol_list, targetatom, j);
		}
	}

	metalist = postprocess_mol_list(info, mol_list);
	destroy_mol_list(mol_list);
	return(metalist);
}


struct mol_metalist *analyze_molgp0(struct struct_info *info, struct mol *mol, int atom1, int atom2)
{
	int j;
	struct mol_list *mol_list;
	struct mol_metalist *metalist;
	BOOL doexec = FALSE;

	mol_list = new_mol_list(moldup(mol));

	for (j = 0; (j < mol->atoms[atom1].Nbond) && !doexec; ++j)
		if (mol->atoms[atom1].neighbors[j] == atom2)
			doexec=TRUE;
	
	if (doexec) {
		for (j = 0; j < info->numreact; ++j) {
			if (info->reaction[j].type == RW_GP0) 
				try_gp0(info, mol, mol_list, atom1, atom2, j);
		}
	}

	metalist = postprocess_mol_list(info, mol_list);
	destroy_mol_list(mol_list);
	return(metalist);
}



void try_gp1(struct struct_info *info, struct mol *mol, struct mol_list *mol_list, int g1, int rxn)
{
	int *k;

	for (k = info->reaction[rxn].g1; k && (*k != -1); ++k) {
		if (mol->gpofunc[g1].tipo == *k) {
			//printf(" -> Can use rxn %s\n", info->reaction[rxn].name);
			executegp1(info, mol_list, &mol->gpofunc[g1], &info->reaction[rxn]);
		}
	}
}



void try_gp2(struct struct_info *info, struct mol *mol, struct mol_list *mol_list, int g1, int rxn)
{
	int *k,*l;
	int g2;
	int i;
	struct paths *paths;

	for (k = info->reaction[rxn].g1; k && (*k != -1); ++k) {
		if (mol->gpofunc[g1].tipo == *k) {
			for (g2 = 0; g2 < mol->Nfunc; ++g2) {
				if (g2 == g1)
					continue;

				for (l = info->reaction[rxn].g2; l && (*l != -1); ++l) {
					if (mol->gpofunc[g2].tipo == *l) {
						paths = findpaths(mol, mol->gpofunc[g1].atomos[0], mol->gpofunc[g2].atomos[0]);

						for (i = 0; i < paths->numpaths; ++i) {
							if (paths->paths[i].len == info->reaction[rxn].path) {
								//printf(" -> Can use rxn %s\n", info->reaction[rxn].name);
								executegp2(info, mol_list, &mol->gpofunc[g1], &mol->gpofunc[g2], &paths->paths[i], &info->reaction[rxn]);
							}
						}
						freepaths(paths);
					}
				}
			}
		}
	}
}

void try_ring(struct struct_info *info, struct mol *mol, struct mol_list *mol_list, int ring, int rxn)
{
	int *k;

	if (mol->rings[ring].ringtype) {
		for (k = info->reaction[rxn].gring; k && (*k != -1); ++k) {
			if ((mol->rings[ring].ringtype == *k) && (!mol->rings[ring].multiring)) {
				//printf(" -> Can use rxn %s\n", info->reaction[rxn].name);
				executering(info, mol_list, &mol->rings[ring], &info->reaction[rxn]);
			}
		}
	}
}


void try_mring(struct struct_info *info, struct mol *mol, struct mol_list *mol_list, int ring, int rxn)
{
	int *k;

	for (k = info->reaction[rxn].gring; k && (*k != -1); ++k) {
		if (mol->mrings[ring].ringtype == *k) {
			//printf(" -> Can use rxn %s\n", info->reaction[rxn].name);
			executemring(info, mol, mol_list, &mol->mrings[ring], &info->reaction[rxn]);
		}
	}
}




void try_fga(struct struct_info *info, struct mol *mol, struct mol_list *mol_list, int targetatom, int rxn)
{
	mol;
	executefga(info, mol_list, targetatom, &info->reaction[rxn]);
}

void try_gp0(struct struct_info *info, struct mol *mol, struct mol_list *mol_list, int atom1, int atom2, int rxn)
{
	mol;
	executefg0(info, mol_list, atom1, atom2, &info->reaction[rxn]);
}



static int compare_eval(const void *a1, const void* a2)
{
    struct mol_list **arg1 = (struct mol_list **)a1;
    struct mol_list **arg2 = (struct mol_list **)a2;
    int ret;

    if ((ret = arg2[0]->rxn_info->eval - arg1[0]->rxn_info->eval) != 0)
    	return(ret);

    if ((ret = arg2[0]->rxn_info->rxn - arg1[0]->rxn_info->rxn) != 0)
    	return(ret);

 	return(0);

}


static int compare_molsmiles(const void *a1, const void* a2)
{
    struct mol **mol1 = (struct mol **)a1;
    struct mol **mol2 = (struct mol **)a2;

	return(strcmp(mol1[0]->smiles, mol2[0]->smiles));
}



/* Processes a mol_list returned by analyze_mol(), creating a mol_metalist
 * 1. separates the molecules in each of the mols of the mol_list
 * 2. parses each precursor
 * 3. calculates the complexity of the target molecule and of each precursor
 * 4. calculates the simplification and evaluation function for each transform 
 * Note: *params may be NULL; in that case the simplification is always 100
 * Does not affect *mol_list   */
struct mol_metalist *postprocess_mol_list(struct struct_info *info, struct mol_list *list)
{
	struct mol_list *newlist;
	struct mol_metalist *metalist;
	int i, j, cplx, tgtcplx, simp;

	metalist = malloc(sizeof(struct mol_metalist));
	assert(metalist);
	metalist->Nlist = list->Nmol-1;
	if(metalist->Nlist == 0){
		metalist->lists = NULL;
	} else {
		metalist->lists = malloc(metalist->Nlist*sizeof(struct mol_list*));
		assert(metalist->lists);

		tgtcplx = list->mols[0]->complexity;
		for(i = 1; i < list->Nmol; ++i){
			printf("\n");
			newlist = calloc(1, sizeof(struct mol_list));
			assert(newlist);
			separate_mols(list->mols[i], newlist);
			metalist->lists[i-1] = newlist;
			newlist->rxn_info = malloc(sizeof(struct rxn_info));
			assert(newlist->rxn_info);
			*(newlist->rxn_info) = list->rxn_info[i];
			newlist->rxn_info->simplification = INT_MAX;
			for(j = 0; j < newlist->Nmol; ++j){
				parse_mol(info, newlist->mols[j], NULL);
				cplx = newlist->mols[j]->complexity;
				simp = cplx ? (100 * tgtcplx)/cplx : 100;
				if(simp < newlist->rxn_info->simplification)
					newlist->rxn_info->simplification = simp;
				
				printf("%s\n", newlist->mols[j]->smiles);
			}
			newlist->rxn_info->eval = newlist->rxn_info->simplification * newlist->rxn_info->rating;
			qsort(newlist->mols, newlist->Nmol, sizeof(struct mol*), compare_molsmiles);
		}

    	qsort(metalist->lists, metalist->Nlist, sizeof(struct mol_list*), compare_eval);

		for(i = 1; i < metalist->Nlist; ++i) {
           if((metalist->lists[i]->rxn_info->rxn == metalist->lists[i-1]->rxn_info->rxn)
				&& (metalist->lists[i]->Nmol == metalist->lists[i-1]->Nmol)) {
				for(j = 0; j < metalist->lists[i]->Nmol; ++j) {
    	           	if(strcmp(metalist->lists[i]->mols[j]->smiles, metalist->lists[i-1]->mols[j]->smiles) != 0)
        	           	break;
				}
               	if(j == metalist->lists[i]->Nmol) {
                	delete_mol_list(metalist, i);
                    printf("Terminating duplicate...\n");
                    --i;
                };
           }

        }


	}

	return(metalist);
}


char *get_rxntext(struct struct_info *info, struct rxn_info *rxn_info)
{
	struct struct_log *buf = init_log(100);
	char *p, *s = malloc(1000);

	sprintf(s, "%s\r\nRating = %i\r\nSimplification = %i\r\nEvaluation = %i\r\n",
		info->reaction[rxn_info->rxn].name, rxn_info->rating, rxn_info->simplification, rxn_info->eval);
	logadd(buf, s);

	logadd(buf, info->reaction[rxn_info->rxn].comments);
	
	p = buf->buffer;
	while((p = strstr(p, "\\n")) != NULL) {
		p[0] = '\r';
		p[1] = '\n';
	}

	free(s);
	return(log2str(buf));
}


char *get_molinfo(struct struct_info *info, struct mol *mol) 
{
	struct struct_log *buf = init_log(128);
	char *aux;

	assert(buf);
	logadd(buf, "%s\n", mol->smiles);

	if(mol->dbname) 
		logadd(buf, "%s\n", mol->dbname);

	logadd(buf, "\nATOM TYPES:\n%s", aux = list_atomtypes(mol));
	free(aux);

	logadd(buf, "\nFUNCTIONAL GROUPS:\n%s", aux = list_fgroups(info, mol));
	free(aux);

	logadd(buf, "\nRINGS:\n%s", aux = list_rings(info, mol));
	free(aux);

	logadd(buf, "\nCOMPLEXITY = %i", mol->complexity);
	
	return(log2str(buf));
}

