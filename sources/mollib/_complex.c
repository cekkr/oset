/* _complex.c
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

#define	KDBDISCOUNT	(3.0/4.0)

int mol_complexity(struct mol *mol, struct struct_info *info)
{
	int i, res = 0;
	struct cplx_params *params = info->cplx_params;
	
	if(params) {
		for(i = 0; i < mol->Natom; ++i){
			res += params->atom;
			if(isatomtype(mol, i, RW_TERTIARY))
				res += params->tertiary;
			if(isatomtype(mol, i, RW_QUATERNARY))
				res += params->quaternary;
		}
		for(i = 0; i < mol->Nfunc; ++i){
			res += info->variables[mol->gpofunc[i].tipo].data.fg.complex;
		}
	}
//	res += (mol->Nbond - mol->Natom + 1) * params->ring[6];
	for(i = 0; i < mol->Nring; ++i) {
		if(!mol->rings[i].multiring) {
			if(mol->rings[i].ringtype)
				res += info->variables[mol->rings[i].ringtype].data.rg.complex;
			else {
				if(mol->rings[i].len >= MACROCYCLE)
					res += params->ring[MACROCYCLE];
				else
					res += params->ring[mol->rings[i].len - 1];
			}
		}
	}
	for(i = 0; i < mol->Nmring; ++i) {
		res += info->variables[mol->mrings[i].ringtype].data.mrg.complex;
	}
	if (mol->dbname != NULL)
		res = (int)(res * KDBDISCOUNT);

	return(res);
}

void setdef_cplx_params(struct cplx_params *params)
{

	memset(params, 0, sizeof(struct cplx_params));
	params->ring[0] = 0;
	params->ring[1] = 0;
	params->ring[2] = 0;
    params->ring[3] = 25;
	params->ring[4] = 35;
    params->ring[5] = 30;
    params->ring[6] = 25;
    params->ring[7] = 40;
    params->ring[8] = 70;

	params->atom = 5;
	params->tertiary = 6;
	params->quaternary = 17;

}


struct cplx_params *init_cplx_params()
{
	struct cplx_params *params;
	params = malloc(sizeof(struct cplx_params));
	setdef_cplx_params(params);
	return(params);
}
