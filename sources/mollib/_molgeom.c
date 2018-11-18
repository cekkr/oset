/* _molgeom.c
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


double bondlength(struct mol *mol, int n_bond);

	

/*	scales the molecule so that its average bond length equals bondsize
 *	it also moves it to the origin*/
void normalize_mol(struct mol *mol, double bondsize)
{
	int i;
	double avglength, scale, deltax, deltay;
	struct atom *atom;

	if(mol->Natom > 1)
	{
		avglength = 0.0;
		for(i=0; i < mol->Nbond; ++i)
			avglength += bondlength(mol, i);
		avglength /= mol->Nbond;
		scale = bondsize / avglength;
	
		deltax = mol->atoms[0].x;
		deltay = mol->atoms[0].y;
		for(i=1; i < mol->Natom; ++i)
		{
			atom = &mol->atoms[i];
			if(atom->x < deltax)
				deltax = atom->x;
			if(atom->y > deltay)
				deltay = atom->y;
		}

		for(i=0; i < mol->Natom; ++i)
		{
			atom = &mol->atoms[i];
			atom->x = (atom->x - deltax) * scale;
			atom->y = (atom->y - deltay) * scale;
		}
	}
	else if(mol->Natom == 1) {
		mol->atoms[0].x = mol->atoms[0].y = 0.0;
	}
	
}

double getmeanbondlength(struct mol *mol)
{
	int i;
	double avglength = 0.0;

	if(mol->Natom > 1)
	{
		avglength = 0.0;
		for(i=0; i < mol->Nbond; ++i)
			avglength += bondlength(mol, i);
		avglength /= mol->Nbond;
	}

	return(avglength);
}


/*	moves a molecule so that the upper left corner of the enclosing
 *	rectangle lies at the origin */
void molorigin(struct mol *mol)
{
	int i;
	double deltax, deltay;
	struct atom *atom;

	if(mol->Natom > 0)
	{
		deltax = mol->atoms[0].x;
		deltay = mol->atoms[0].y;
		for(i=1; i < mol->Natom; ++i)
		{
			atom = &mol->atoms[i];
			if(atom->x < deltax)
				deltax = atom->x;
			if(atom->y > deltay)
				deltay = atom->y;
		}

		for(i=0; i < mol->Natom; ++i)
		{
			atom = &mol->atoms[i];
			atom->x = atom->x - deltax;
			atom->y = atom->y - deltay;
		}
	}
}

struct drect getmolrect(struct mol *mol)
{
	struct drect r = {0.0, 0.0, 0.0, 0.0};
	struct atom *atom;
	int i;

	if(mol->Natom > 0) {
		r.top = mol->atoms[0].y;
		r.bottom = mol->atoms[0].y;
		r.right = mol->atoms[0].x;
		r.left = mol->atoms[0].x;


		for(i=1; i < mol->Natom; ++i)
		{
			atom = &mol->atoms[i];
			if(atom->x < r.left)
				r.left = atom->x;
			else if(atom->x > r.right)
				r.right = atom->x;
			if(atom->y > r.top)
				r.top = atom->y;
			else if(atom->y < r.bottom)
				r.bottom = atom->y;
		}
	}

	return(r);
}

/*	returns the length of bond n_bond */
double bondlength(struct mol *mol, int n_bond)
{
	double dx, dy;
	struct bond *bond;

	bond = &mol->bonds[n_bond];

	dx = mol->atoms[bond->a1].x - mol->atoms[bond->a2].x;
	dy = mol->atoms[bond->a1].y - mol->atoms[bond->a2].y;
	return(sqrt(dx*dx + dy*dy));
}

/*	returns the angle of the line that passes through a1 and a2 with respect
 *	to the x-axis	*/
double interatomic_angle(struct mol *mol, int a1, int a2)
{
	struct atom *atom1, *atom2;

	atom1 = &mol->atoms[a1];
	atom2 = &mol->atoms[a2];

	return(atan2(atom2->y - atom1->y, atom2->x - atom1->x));
}

/*	returns the shortest angle between two bonds.
 *	The bonds MUST share an atom. */
double interbond_angle(struct mol *mol, int b1, int b2)
{
	double v1[2], v2[2];
	struct atom *a1, *a2, *a3;
	struct bond *bond1, *bond2;

	bond1 = &mol->bonds[b1];
	bond2 = &mol->bonds[b2];

	if(bond1->a1 == bond2->a1) {
		a2 = &mol->atoms[bond1->a1];
		a1 = &mol->atoms[bond1->a2];
		a3 = &mol->atoms[bond2->a2];
	} else if(bond1->a2 == bond2->a2) {
		a2 = &mol->atoms[bond1->a2];
		a1 = &mol->atoms[bond1->a1];
		a3 = &mol->atoms[bond2->a1];
	} else if(bond1->a1 == bond2->a2) {
		a2 = &mol->atoms[bond1->a1];
		a1 = &mol->atoms[bond1->a2];
		a3 = &mol->atoms[bond2->a1];
	} else if(bond1->a2 == bond2->a1) {
		a2 = &mol->atoms[bond1->a2];
		a1 = &mol->atoms[bond1->a1];
		a3 = &mol->atoms[bond2->a2];
	} else assert(0);
		
	v1[0] = a1->x - a2->x;
	v1[1] = a1->y - a2->y;
	v2[0] = a3->x - a2->x;
	v2[1] = a3->y - a2->y;

	return(acos((v1[0]*v2[0]+v1[1]*v2[1])/bondlength(mol, b1)/bondlength(mol, b2)));
}


/* Returns TRUE if there is an atom within 'range'. It only takes into account
 * atoms which are connected by a path to 'startatom' (i.e. belong to the
 * same 'molecule').
 * NOTE: currently the range test is rectangular. It seems to work well, but
 * it would be more logical for it to be circular    */
BOOL near_atom(struct mol *mol, double x, double y, double range, int startatom)
{
	int i;
	int *used = calloc(mol->Natom, sizeof(int));
	assert(used);

	_markatoms(mol, used, startatom, TRUE);

	for(i = 0; i < mol->Natom; ++i) {
		struct atom *atom = &mol->atoms[i];
		if(used[i] && (fabs(x - atom->x) < range) && (fabs(y - atom->y) < range))
			break;
	}
	free(used);
	if(i < mol->Natom)
		return(TRUE);
	else
		return(FALSE);
}


void _rand_sprout(struct mol *mol, int fromatom, double length, double *x, double *y)
{
	BOOL ok=FALSE;
	double th;
	int n = 0;
	double l = length;
	struct atom *atom = &mol->atoms[fromatom];

	while (!ok) {
		++n;
		if((n % 50) == 0)
			l += length;
		th = (rand()*PI*2)/RAND_MAX;
		*x = atom->x + l*cos(th);
		*y = atom->y + l*sin(th);
		if (!near_atom(mol, *x, *y, length / 2.0, fromatom))
			ok = TRUE;
	}
}

void _sprout_0(struct mol *mol, int fromatom, double *x, double *y)
{
	struct atom *atom = &mol->atoms[fromatom];

	*x = atom->x + cos(30*PI/180);
	*y = atom->y + sin(30*PI/180);
}

void _sprout_1(struct mol *mol, int fromatom, double *x, double *y)
{
	struct atom *atom = &mol->atoms[fromatom];
	double l = bondlength(mol, atom->bonds[0]);
	struct bond *bond = &mol->bonds[atom->bonds[0]];
	double th = interatomic_angle(mol, fromatom, (bond->a1 == fromatom) ? bond->a2 : bond->a1);
	struct atom *neigh = &mol->atoms[(bond->a1 == fromatom) ? bond->a2 : bond->a1];
	int n = 0;
	BOOL ok = FALSE;
	double th2;

	while(!ok) {
		if(n < 2) {
			if(th > 0) { 
				if(atom->x < neigh->x)
					th2 = th + PI * 2 / 3 * ((n == 0)?1.0:-1.0);
				else
					th2 = th - PI * 2 / 3 * ((n == 0)?1.0:-1.0);
			}
			else {
				if(atom->x < neigh->x)
					th2 = th - PI * 2 / 3 * ((n == 0)?1.0:-1.0);
				else
					th2 = th + PI * 2 / 3 * ((n == 0)?1.0:-1.0);
			}
			*x = atom->x + l*cos(th2);
			*y = atom->y + l*sin(th2);
			if (!near_atom(mol, *x, *y, l/2.0, fromatom))
				ok = TRUE;
		} else {
			_rand_sprout(mol, fromatom, l, x, y);
			ok = TRUE;
		}
		++n;
	}

}

void _sprout_2(struct mol *mol, int fromatom, double *x, double *y)
{
	struct atom *atom = &mol->atoms[fromatom];
	double l = 0.5*(bondlength(mol, atom->bonds[0]) + bondlength(mol, atom->bonds[1]));
	struct bond *bond = &mol->bonds[atom->bonds[0]];
	double th = interatomic_angle(mol, fromatom, (bond->a1 == fromatom) ? bond->a2 : bond->a1);
	double th2;

	bond = &mol->bonds[atom->bonds[1]];
	th2 = 0.5*(th + interatomic_angle(mol, fromatom, (bond->a1 == fromatom) ? bond->a2 : bond->a1));
	if(fabs(th2 - th) < PI/2)
		th2 += PI;
	*x = atom->x + l*cos(th2);
	*y = atom->y + l*sin(th2);

	if(near_atom(mol, *x, *y, l/2.0, fromatom)) {
		th2 += PI;
		*x = atom->x + l*cos(th2);
		*y = atom->y + l*sin(th2);
		if(near_atom(mol, *x, *y, l/2.0, fromatom))
			_rand_sprout(mol, fromatom, l, x, y);
	}
}

void _sprout_3(struct mol *mol, int fromatom, double *x, double *y)
{
	struct atom *atom = &mol->atoms[fromatom];
	struct atom *antiatom;
	struct bond *bond;
	int antibonds[4], i, j;
	double th[4];

	for(i=0; i<3; ++i) {
		th[i] = interbond_angle(mol, atom->bonds[i], atom->bonds[(i+1)%3]);
		antibonds[i] = atom->bonds[(i+2)%3];
	}
	for(i = 0; i < 3; ++i)  // sort
		for(j = 1; j < 3; ++j)
			if(th[i] < th[j]) {
				antibonds[3] = antibonds[i];
				antibonds[i] = antibonds[j];
				antibonds[j] = antibonds[3];
				th[3] = th[i];
				th[i] = th[j];
				th[j] = th[3];
			}

	for(i = 0; i < 3; ++i) {
		bond = &mol->bonds[antibonds[i]];
		antiatom = (bond->a1 == fromatom) ? &mol->atoms[bond->a2] : &mol->atoms[bond->a1];
		*x = 2*atom->x - antiatom->x;
		*y = 2*atom->y - antiatom->y;
		if(!near_atom(mol, *x, *y, bondlength(mol, antibonds[i])/2.0, fromatom))
			break;
	}
	if(i == 3) 
		_rand_sprout(mol, fromatom, 
		(bondlength(mol, antibonds[0])+bondlength(mol, antibonds[1])+bondlength(mol, antibonds[2]))/3.0,
		 x, y);
}


/*	Creates a new atom bonded to fromatom, with atomic number Z. This
 *	function calculates the coordinates of the new atom to make them
 *	"aesthetically pleasing"
 *	returns the new atom's number */
int sprout_atom(struct mol *mol, int fromatom, int Z)
{
	int toatom;
	struct atom *atom = &mol->atoms[fromatom];
	double x, y;

	switch(atom->Nbond){
		case 0:
			_sprout_0(mol, fromatom, &x, &y);
			break;

		case 1:
			_sprout_1(mol, fromatom, &x, &y);
			break;

		case 2:
			_sprout_2(mol, fromatom, &x, &y);
			break;

		case 3:
			_sprout_3(mol, fromatom, &x, &y);
			break;

		default:
			_rand_sprout(mol, fromatom, 1.0, &x, &y);
			break;
	}

	toatom = new_atom(mol, Z, 0, x, y, 0.0);
	new_bond(mol, fromatom, toatom);

//	classify_atoms(mol, NULL); 

	return(toatom);

}

