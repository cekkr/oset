/* _bigsmdb.c
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

#include <ctype.h>

#define BUFFSIZE	1024

void readsmilecas_index(struct struct_info *info)
{
	FILE *fin;
	struct smilecas_index *buff;
	int n = 0;

	buff = NULL;
	if((fin = fopen("dbindex", "rb")) == NULL) {
                seterr("error reading index\n");
                info->numsmilecas = 0;
                info->smilecas_index = NULL;
	} else {
		do {
			buff = realloc(buff, (n + BUFFSIZE) * sizeof(struct smilecas_index));
			n += fread(buff+n, sizeof(struct smilecas_index), BUFFSIZE, fin);
		} while (!feof(fin));
		
		fclose(fin);
		buff = realloc(buff, n * sizeof(struct smilecas_index));
		info->smilecas_index = buff;
		info->numsmilecas = n;
	}
}



int compsmilecas(const void *a1, const void *a2)
{
	struct smilecas_index *e1, *e2;
	e1 = (struct smilecas_index*) a1;
	e2 = (struct smilecas_index*) a2;

	return(e1->formula - e2->formula);
}


struct smilecas_entry *free_smilecas(struct smilecas_entry *entry)
{
	if(entry) {
		free(entry->smiles);
		free(entry->name);
		free(entry->cas);
		free(entry);
		return(NULL);
	}

}


struct smilecas_entry *findsmilecas(struct struct_info *info, char *smilesquery)
{
	struct smilecas_index *key, formula;
	struct smilecas_entry *entry = NULL;
	FILE *fin;
	int len, state, newformula;
	char *buff, *p, smiles[500], name[500], cas[20], formstr[10];
	BOOL end = FALSE;
	struct mol *mol;

        if(info->numsmilecas == 0)
                return(NULL);
	mol = smiles2mol(smilesquery);

	if(mol) {
    	calc_formula(mol);
    	formula.formula = mol->formula[0]*1000000 + mol->formula[1]*10000 + mol->formula[2]*100 + mol->formula[3];
    
    	key = bsearch(&formula, info->smilecas_index, info->numsmilecas, sizeof(struct smilecas_index), compsmilecas);
    	if (key) {
    		if((fin = fopen("dbout.txt", "rb")) == NULL){
                        seterr("error reading dbindex");
    			return(NULL);
    		} else {
    			fseek(fin, key->offset, SEEK_SET);
        		len = 0;
        		state = 0;
    			formstr[0] = smiles[0] = name[0] = cas[0] = 0;
    			p = buff = malloc(BUFFSIZE + 10);
    			assert(buff);
        		do {
        			if (p - buff >= len) {
        				len = fread(buff, 1, BUFFSIZE, fin);
        				buff[len] = 0;
        				p = buff;
        			}
        
        			switch(state) {
        				case 0: //formula
        					if(*p == '\t') {
        						newformula = atoi(formstr);
        						formstr[0] = 0;
        						if(newformula != key->formula) {
    								end = TRUE;
        						}
        
        						state = 1;
        					} else {
        						strncat(formstr, p, 1);
        					}
        					++p;
    	   					break;
        
        				case 1: // smiles
        					if(*p == '\t') {
        						state = 2;
        					} else {
        						strncat(smiles, p, 1);
    						}
        					++p;
        					break;
    
        				case 2: //   CAS
        					if(*p == '\t') {
        						state = 3;
        					} else {
        						strncat(cas, p, 1);
    						}
        					++p;
        					break;
    
        				case 3: //  name
        					if((*p == 10) || (*p == 13)) {
        						state = 4;
        					} else {
        						strncat(name, p, 1);
    						}
        					++p;
        					break;
    
        				case 4: //	eol
        					if((*p != 10) && (*p != 13)) {
    							if(strcmp(smilesquery, smiles) == 0) {
    								entry = malloc(sizeof(struct smilecas_entry));
    								assert(entry);
    								entry->name = strdup(name);
    								entry->smiles = strdup(smiles);
    								entry->cas = strdup(cas);
    								entry->formula = key->formula;
    								end = TRUE;
    							} else {
    	    						state = 0;
    								formstr[0] = smiles[0] = name[0] = cas[0] = 0;
    							}
        					} else
    	    					++p;
        					break;
    
    
    				}
        
        		} while(!end && (!feof(fin) || (p - buff <= len)));
    
    			free(buff);
    			fclose(fin);
    		}
    
    
    	}
		destroy_mol(mol);
	}
	return(entry);
}
