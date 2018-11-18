/* _molring.c
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

BOOL debugrings = FALSE;

extern int primes[];

/*

Hanser et al. A New Algorithm for Exhaustive Ring Perception in a Molecular Graph
J. Chem. Inf. Comput. Sci. 36, 1146-1152 (1996)


FINDALLRINGS(graph):
	rings <- {}
	CONVERT(graph, V, E)
	while V != {}
		choose x in V {lower to higher conectivity}
		REMOVE(x,V,E,rings)


CONVERT(graph,V,E):
	V <- {}; E <- {};
	foreach vertex x in graph
		V <- V U {x}
	for each edge (x,y) in graph
		p(x,y) <- (x,y)
		E <- E U {p(x,y)}


REMOVE(x,V,E,rings):
	foreach p(x,y)|x!=y, p(x,z)|x!=z in E^2
		if p(x,y) and p(x,z) = {x} then
			p(y,z) <- p(x,y).p(x,z)
			E <- E U {p(y,z)}
	foreach p(x,y) in E
		if x=y then 
			rings <- rings U {p(x,y)}
		E <- E - {p(x,y)}
	V <- V - {x}

*/

int numops = 0;

struct ringpathlist {
	int numpaths;
	struct ringpath *paths;
};

struct vertex {
	int num;
	int conn;
};

int compvertex(const void *a1, const void *a2)
{
	struct vertex **v1 = (struct vertex **)a1;
	struct vertex **v2 = (struct vertex **)a2;

	return((*v1)->conn - (*v2)->conn);
}


void convert(struct mol *mol, struct vertex *vertex, struct ringpathlist *paths)
{
	int i;
	struct ringpath *p;

	for (i = 0; i < mol->Natom; ++i) {
		vertex[i].num = i;
		vertex[i].conn = mol->atoms[i].Nbond;
	}

	for (i = 0; i < mol->Nbond; ++i) {
		paths->paths = realloc(paths->paths, sizeof(struct ringpath) * ((paths->numpaths) + 1));
		p = &paths->paths[paths->numpaths];
		memset(p, 0, sizeof(struct ringpath));
		p->len = 2;
		p->path = calloc(p->len, sizeof(int));
		p->path[0] = mol->bonds[i].a1;
		p->path[1] = mol->bonds[i].a2;

		p->nodes = bs_new(mol->Natom);
		bs_set(p->nodes, mol->bonds[i].a1);
		bs_set(p->nodes, mol->bonds[i].a2);
		p->bonds = bs_new(mol->Nbond);
		bs_set(p->bonds, i);

		++paths->numpaths;
	}
}

void printpath(struct ringpath *r)
{
	int j;

	printf("[");
	for (j = 0; j < r->len; ++j)
		printf("%i%c", r->path[j]+1, j < r->len-1 ? '-' : ']');
	printf(" ");
}


void printstatus(struct mol *mol, struct vertex *v, struct ringpathlist *e)
{
	int i;

	printf("Vertices: ");
	for (i = 0; i < mol->Natom; ++i)
		if (v[i].num != -1)
			printf("%i(%i) ", v[i].num+1, v[i].conn);
	printf("\n");

	printf("Edges (%i): ", e->numpaths);
	for (i = 0; i < e->numpaths; ++i) 
		printpath(&e->paths[i]);
	printf("\n");
}

BOOL pathedge(struct ringpath *path, int x)
{
	int b,e;
	b = path->path[0];
	e = path->path[path->len-1];

	return((((b == x) || (e == x)) && (b != e)));
}


int *rcopypath(int *c, struct ringpath *r)
{
	int i;
	for (i = r->len-1; i >= 0; --i) 
		*(c++) = r->path[i];

	return(c);
}

int *fcopypath(int *c, struct ringpath *r)
{
	int i;
	for (i = 0; i < r->len; ++i) 
		*(c++) = r->path[i];

	return(c);
}

void concatpath(struct ringpath *res, struct ringpath *r1, struct ringpath *r2, int x)
{
	int *c;

	assert(res);
	assert(r1);
	assert(r2);

	memset(res, 0, sizeof(struct ringpath));

	res->len = r1->len + r2->len - 1;
	res->path = calloc(res->len, sizeof(int));
	assert(res->path);

	c = res->path;
	if (r1->path[0] == x)
		c = rcopypath(c, r1);
	else
		c = fcopypath(c, r1);
	--c;

	if (r2->path[0] == x)
		c = fcopypath(c, r2);
	else
		c = rcopypath(c, r2);

	res->nodes = bs_or(r1->nodes, r2->nodes);		
	res->bonds = bs_or(r1->bonds, r2->bonds);
}

void free_ringpath(struct ringpath *r)
{
	free(r->path);
	bs_free(r->nodes);
	bs_free(r->bonds);
}


void dup_ringpath(struct mol *mol, struct ringpath *dest, struct ringpath *src)
{
	struct bitset *bs;
	int i;

	memset(dest, 0, sizeof(struct ringpath));
	dest->len = src->len;
	dest->IH = src->IH;
	dest->IA = src->IA;
	dest->ntied = src->ntied;
	dest->type = src->type;
	dest->dependent = src->dependent;

	bs = bs_new(mol->Natom);
	dest->nodes = bs_or(src->nodes, bs);
	bs = bs_free(bs);

	bs = bs_new(mol->Nbond);
	dest->bonds = bs_or(src->bonds, bs);
	bs = bs_free(bs);

	dest->path = calloc(dest->len, sizeof(int));
	for (i = 0; i < dest->len; ++i)
		dest->path[i] = src->path[i];
}


void free_ringpathinlist(struct ringpathlist *e, int pos)
{
	free_ringpath(&e->paths[pos]);
	if (pos < e->numpaths-1) 
		e->paths[pos] = e->paths[e->numpaths-1];
	--e->numpaths;
}


void removenode(struct mol *mol, int x, struct vertex *v, struct ringpathlist *e)
{
	int i,j;
	struct bitset *bs;
	int count;

/*
	foreach p(x,y)|x!=y, p(x,z)|x!=z in E^2
		if p(x,y) and p(x,z) = {x} then
			p(y,z) <- p(x,y).p(x,z)
			E <- E U {p(y,z)}
*/
	for (i = 0; i < e->numpaths; ++i) {
		if (pathedge(&e->paths[i], x)) {
			for (j = i+1; j < e->numpaths; ++j) {
				if (pathedge(&e->paths[j], x)) {
					++numops;
					bs = bs_and(e->paths[i].nodes, e->paths[j].nodes);
					count = bs_count(bs);
					/* if paths join at one endpoint or two endpoints */
					if ((count == 1) || ((count == 2) && pathedge(&e->paths[j], e->paths[i].path[0]) && pathedge(&e->paths[j], e->paths[i].path[e->paths[i].len-1]))) {
						e->paths = realloc(e->paths, sizeof(struct ringpath) * (e->numpaths + 1));
						assert(e->paths);
						concatpath(&e->paths[e->numpaths], &e->paths[i], &e->paths[j], x);
						++e->numpaths;
					}
					bs = bs_free(bs);
				}
			}
		}
	}

/*
	foreach p(x,y) in E
		if x=y then 
			rings <- rings U {p(x,y)}
		E <- E - {p(x,y)}
*/	
	for (i = 0; i < e->numpaths; ++i) {
		if (pathedge(&e->paths[i], x)) {
			free_ringpathinlist(e, i);
			--i;
		}
	}

/*
	V <- V - {x}
*/
	if (mol->atoms[x].Nbond == 1)
		--v[mol->atoms[x].neighbors[0]].conn;
	
	v[x].num = -1;
	v[x].conn = 999;
}

struct ringpathlist *findallrings(struct mol *mol)
{
	struct vertex *v;
	struct vertex **vv;
	struct ringpathlist *e;
	int i;

	v = calloc(mol->Natom, sizeof(struct vertex));
	assert(v);
	vv = calloc(mol->Natom, sizeof(struct vertex *));
	assert(vv);
	e = calloc(1, sizeof(struct ringpathlist));
	assert(e);

	numops = 0;
	convert(mol, v, e);

	for (i = 0; i < mol->Natom; ++i)
		vv[i] = &v[i];

	for (i = 0; i < mol->Natom; ++i) {
		qsort(vv, mol->Natom, sizeof(struct vertex *), compvertex);

/*
		if (debugrings) {
			printf("\nSTEP %i\n", i);
			printstatus(mol, v, e);

			printf("Remove %i\n", vv[0]->num+1);
		}
*/
		removenode(mol, vv[0]->num, v, e);
	}


	if (debugrings) {
		printf("\n\nALL RINGS:\n");
		printstatus(mol,v,e);
		printf("Ops = %i, rings = %i\n", numops, e->numpaths);
	}

	free(v);
	free(vv);

	return(e);
}



/**********************************************************************************
 **********************************************************************************

  Fujita: A New Algorithm for Selection of Synthetically Important Rings
  J. Chem. Inf. Comput. Sci. 28, 78-82 (1988)


1. Find all rings
2. Sort ascending by ring size
3. Calc IH and IA 
4. Detect and exclude tied and multi-tied rings
5,6.7. try to cover with tied rings of the same type 
*/


int compareringpath(const void *a1, const void *a2)
{
	struct ringpath *r1 = (struct ringpath *)a1;
	struct ringpath *r2 = (struct ringpath *)a2;

	return(r1->len - r2->len);
}

void calcindex(struct mol *mol, struct ringpath *r)
{
	int i;
	r->IH = r->IA = 0;
	for (i = 0; i < r->len-1; ++i)
		switch (mol->atoms[r->path[i]].Z) {
			case 6:
				break;
			case 7:
			case 8:
			case 15:
			case 16:
				++r->IH;
				break;
			default:
				++r->IA;
				break;
		}

	if ((r->IH == 0) && (r->IA == 0))
		r->type = RING_NORMAL;
	else if ((r->IH > 0) && (r->IA == 0))
		r->type = RING_HETERO;
	else if (r->IA > 0)
		r->type = RING_ABNORMAL;
}

void calctied(struct mol *mol, struct ringpath *r)
{
	int i,j;
	struct atom *atom;

	r->ntied = 0;
	for (i = 0; i < r->len-1; ++i) {
		atom = &mol->atoms[r->path[i]];

		for (j = 0; j < atom->Nbond; ++j) {
			if (bs_isset(r->nodes, atom->neighbors[j]) && !bs_isset(r->bonds, atom->bonds[j]))
				++r->ntied;
		}
	}
	r->ntied /= 2;
}

void trycover(struct mol *mol, struct ringpathlist *allrings, int pos)
{
	struct bitset *bs, *temp;
	int j;
	int size;
	int count;

	bs = bs_new(mol->Nbond);
	size = allrings->paths[pos].len;

	/* R = cycle we're testing
	   S = tied subcycles we're adding to form R */

	/* rule 2: all S are the the same size or smaller than R */
	for (j = 0; (j < allrings->numpaths) && (allrings->paths[j].len <= size); ++j)
		/* rule 1: S is part of a subset of Tr (tied rings) */
		/* rule 4: All S arte the same class (carbo, hetero, abnormal) as R */
		if ((allrings->paths[j].ntied > 0) && (allrings->paths[pos].type == allrings->paths[j].type)) {
			/* rule 5: all S have the same or smaller IH (in RING_HETERO) or IA (in RING_ABNORMAL) */
			switch (allrings->paths[pos].type) {
				case RING_HETERO:
					if (allrings->paths[j].IH > allrings->paths[pos].IH)
						continue;
					break;
				case RING_ABNORMAL:
					if (allrings->paths[j].IA > allrings->paths[pos].IA)
						continue;
					break;
			}

			/* rule 3: the intersection of S and R involves not less then half the bonds of S */
			temp = bs_and(allrings->paths[j].bonds, allrings->paths[pos].bonds);
			count = bs_count(temp);
			temp = bs_free(temp);
			if (bs_count(allrings->paths[j].bonds) > count * 2)
				continue;

			temp = bs_or(bs, allrings->paths[j].bonds);
			bs_free(bs);
			bs = temp;
		}

	temp = bs_and(bs, allrings->paths[pos].bonds);
	bs_free(bs);
	bs = temp;

	allrings->paths[pos].dependent = (bs_count(bs) == bs_count(allrings->paths[pos].bonds));
	bs_free(bs);
}


void ringtype(struct mol *mol, struct ringpath *r)
{
	int electron = 0;
	int i;

	if (r->len == 7) {		/* six atoms */
		int bond;
		int type;
		int state = 0;
		int ok = TRUE;

		for (i = 0; ok && (i < r->len-1); ++i) {
			bond = are_bonded(mol, r->path[i], r->path[i+1]);

			type = mol->bonds[bond].order;

			switch(state) {
				case 0:
					state = type;
					break;
				case 1:
					if (type == 2)
						state = 2;
					else
						ok = FALSE;
					break;
				case 2:
					if (type == 1)
						state = 1;
					else
						ok = FALSE;
					break;
			}
		}

		if (ok)
			r->type |= RING_BENZENE | RING_AROMATIC;
	}
}


void find_rings(struct mol *mol)
{
	struct ringpathlist *allrings;
	int i;

	destroy_rings(mol);

	/*	1. Find all rings */
	allrings = findallrings(mol);

	/*	2. Sort ascending by ring size */
	qsort(allrings->paths, allrings->numpaths, sizeof(struct ringpath), compareringpath);

	/*  3. Calc IH and IA */
	/*  4. Detect tied and multi-tied rings */
	for (i = 0; i < allrings->numpaths; ++i) {
		calcindex(mol, &allrings->paths[i]);
		calctied(mol, &allrings->paths[i]);
	}

	/*  5,6,7. try to cover with tied rings of the same type */
	for (i = 2; i < allrings->numpaths; ++i) {
		if (allrings->paths[i].ntied == 0)
			trycover(mol, allrings, i);
	}

	if (debugrings) {
		int esercount = 0;
		printf("\n\nCLASSIFY RINGS:\n");

		for (i = 0; i < allrings->numpaths; ++i) {
			printf("Ring %i: ", i);
			printpath(&allrings->paths[i]);
			printf("  IH=%i IA=%i NTIED=%i DEP=%i\n", allrings->paths[i].IH, allrings->paths[i].IA, allrings->paths[i].ntied, allrings->paths[i].dependent);
		}
	}

	for (i = 0; i < allrings->numpaths; ++i) {
		if ((allrings->paths[i].ntied > 0) || (allrings->paths[i].dependent)) {
			free_ringpathinlist(allrings, i);
			--i;
		} else 
			;//ringtype(mol, &allrings->paths[i]);
	}

	if (debugrings) {
		int esercount = 0;
		printf("\n\nESER:");
		for (i = 0; i < allrings->numpaths; ++i) {
			printf("\n");
			printpath(&allrings->paths[i]);
			printf(" SIZE=%i", allrings->paths[i].len-1);
			if (allrings->paths[i].type & RING_BENZENE)
				printf(" BENZENE");

			++esercount;
		}
		printf("\n COUNT = %i\n\n", esercount);
	}

	for (i = 0; i < allrings->numpaths; ++i) {
		int j;

		allrings->paths[i].hash = 1;
		for (j = 0; j < allrings->paths[i].len-1; ++j)
			allrings->paths[i].hash *= primes[mol->atoms[allrings->paths[i].path[j]].Z];
	}

	mol->rings = allrings->paths;
	mol->Nring = allrings->numpaths;
}



/* classify_rings: checks rotated rings against pattern library */

int *ringrotation(struct ringpath *ring, int pos)
{
	int i;
	int ofs = (pos > 0) ? 1 : -1;
	int org = abs(pos) - 1;
	int *ret = calloc(sizeof(int),ring->len-1);
	assert(ret);

	for (i = 0; i < ring->len-1; ++i) {
		ret[i] = ring->path[org];
		org = (org + ring->len-1 + ofs) % (ring->len-1);
	}	

	return(ret);
}

BOOL ringgrpmatch(struct mol *mol, int *ringlist, struct ringgrp *ringgrp) 
{
	int ret = TRUE;
	int i;

	for (i = 0; (i < ringgrp->size) && ret; ++i) 
		ret = ((mol->atoms[ringlist[i]].Z == ringgrp->atoms[i].Z) && (ringgrp->aromatic || (bond_order(mol, ringlist[i], ringlist[(i+1) % ringgrp->size]) == ringgrp->atoms[i].order)));

	return(ret);
}


void findmultigrp(struct struct_info *info, struct mol *mol, struct multiringgrp *mrg);

void classify_rings(struct struct_info *info, struct mol* mol)
{
	int i,j,k;
	struct ringpath *ring;
	struct ringgrp *ringgrp;
	int *ringlist;
	int perms = 0;
	int *permlist = NULL;


	for (i = 0; i < mol->Nring; ++i) {
		ring = &mol->rings[i];
		for (j = 0; (j < info->numvariables); ++j) {
			if (info->variables[j].type == V_RINGGRP) {
				ringgrp = info->variables[j].data.rg.ringgrp;
				if ((ring->len-1 == ringgrp->size) && (((ring->type & RING_AROMATIC) != 0) == ringgrp->aromatic) && (ring->hash == ringgrp->hash)) {
					perms = 0;
					permlist = NULL;

					for (k = -ringgrp->size; k <= ringgrp->size; ++k) {
						if (k != 0) {
							ringlist = ringrotation(ring, k);
							if (ringgrpmatch(mol,ringlist,ringgrp)) {
								assert(ring->ringtype == 0);
								permlist = realloc(permlist, (perms+1) * sizeof(int));
								assert(permlist);
								permlist[perms++] = k;
							}
							free(ringlist);
						}
					}

					if (perms > 0) {
						ring->ringtype = j;
						permlist = realloc(permlist, (perms+1) * sizeof(int));
						permlist[perms++] = 0;
						ring->ringpos = permlist;
						if (debugrings)
							printf("Ring %i, matches %s %i\n", i, info->variables[j].name, perms-1);
					}
				}
			}
		}
	}

	for (j = 0; j < info->numvariables; ++j) {
		if (info->variables[j].type == V_MULTIRINGGRP) 
			findmultigrp(info, mol, info->variables[j].data.mrg.multiringgrp);
	}
}	






/**************************************************************************************/
/*	MULTIRINGS */


void matchmultiringbond(struct struct_info *info, struct mol *mol, struct multiringgrp *mrg, int *foundrings, int *position, struct bitset **matchpos, int numrule)
{
 	int a1 = -1, a2 = -1;
	int j;
	struct mrbond *rule;
	struct ringpath *ring;
	int *ringbonds;
	BOOL fixed = TRUE;

	rule = &mrg->ringfussions[numrule];

	/* for each ring in rule, fix positions */
	for (j = 0; j < MAXRINGMULTIBOND; ++j) {
		if (rule->rbond[j].numring != -1) {
			ring = &mol->rings[foundrings[rule->rbond[j].numring]];
			if (position[rule->rbond[j].numring] == 0) {
				/* try different positions */
				int *rotation;

				fixed = FALSE;

				for (rotation = ring->ringpos; *rotation; ++rotation) {
					BOOL ok;
					ringbonds = ringrotation(ring, *rotation);
					ok = bs_isset(matchpos[numrule],ringbonds[rule->rbond[j].a1]) && bs_isset(matchpos[numrule],ringbonds[rule->rbond[j].a2]);
					free(ringbonds);

					if (ok) {
						/* position matches rule */
						position[rule->rbond[j].numring] = *rotation;
						matchmultiringbond(info, mol, mrg, foundrings, position, matchpos, numrule);
					}
				}
			}
		}
	}
			

	/* if all positions fixed */
	if (fixed) {
		BOOL ok = TRUE;


		printf("\ntrying: ");
		for (j = 0; j < MAXRINGMULTIBOND; ++j) 
			if (rule->rbond[j].numring != -1) 
				printf("%i ", position[rule->rbond[j].numring]);
		printf("\n");

		for (j = 0; j < MAXRINGMULTIBOND && ok; ++j) {			
			if (rule->rbond[j].numring != -1) {
				ring = &mol->rings[foundrings[rule->rbond[j].numring]];
				ringbonds = ringrotation(ring, position[rule->rbond[j].numring]);

				if (a1 == -1)
					a1 = ringbonds[rule->rbond[j].a1];
				else
					ok &= (a1 == (ringbonds[rule->rbond[j].a1]));

				if (a2 == -1)
					a2 = ringbonds[rule->rbond[j].a2];
				else
					ok &= (a2 == (ringbonds[rule->rbond[j].a2]));

				free(ringbonds);
			}
		}

		if (ok) {
			if (numrule < mrg->nummultibonds-1) {
				matchmultiringbond(info, mol, mrg, foundrings, position, matchpos, numrule+1);
			} else {
				int k;
				struct multiring *mr;
				printf("MATCH ");

				mol->mrings = realloc(mol->mrings, sizeof(struct multiring) * (mol->Nmring + 1));
				assert(mol->mrings);
				mr = &mol->mrings[mol->Nmring];
				memset(mr, 0, sizeof(struct multiring));
				++mol->Nmring;
				
				mr->Nring = mrg->numrings;
				mr->ringtype = mrg->ringtype;
				mr->rings = calloc(sizeof(struct multiringmember), mr->Nring);
				assert(mr->rings);

				for (j = 0; j < mrg->numrings; ++j) {
					ring = &mol->rings[foundrings[j]];
					ring->multiring = TRUE;

					mr->rings[j].ringnum = foundrings[j];
					mr->rings[j].rotation = position[j];

					ringbonds = ringrotation(ring, position[j]);

					printf("%i:(", position[j]);
					for (k = 0; k < ring->len-1; ++k)
						printf("%i ", ringbonds[k]+1);
					printf(") ");
					free(ringbonds);
				}
			}
		}
	}
}

/*
	// for each rule 
	for (i = 0; i < mrg->nummultibonds; ++i) {
*/

void matchmultiring(struct struct_info *info, struct mol *mol, struct multiringgrp *mrg, int *pendrings, int *foundrings, int pos)
{
	int i;

	for (i = 0; i < mol->Nring; ++i) {
		if (pendrings[i] == mrg->ringtypes[pos]) {
			foundrings[pos] = i;
			pendrings[i] = 0;

			if (pos < mrg->numrings-1) {
				matchmultiring(info, mol, mrg, pendrings, foundrings, pos+1);
			} else {
				/* found valid combination! */
				int j,k;
				BOOL ok = TRUE;
				struct bitset *temp;
				bitsetp *matchset;
				int *position;

				matchset = calloc(sizeof(struct bitset *), mrg->nummultibonds);
				assert(matchset);
				position = calloc(sizeof(int), mrg->numrings);
				assert(position);

				printf("COMB: ");
				for (j = 0; j < mrg->numrings; ++j) 
					printf("%i ", foundrings[j]);

				for (j = 0; j < mrg->nummultibonds && ok; ++j) {
					matchset[j] = bs_newtrue(mol->Natom);

					for (k = 0; k < MAXRINGMULTIBOND && ok; ++k) {
						if (mrg->ringfussions[j].rbond[k].numring != -1) {
							temp = matchset[j];
							matchset[j] = bs_and(temp, mol->rings[foundrings[mrg->ringfussions[j].rbond[k].numring]].nodes);

							ok = (bs_count(matchset[j]) != 0);
							bs_free(temp);
						}
					}
				}

				if (ok) {
					printf(" INTERSECT ");
					for (j = 0; j < mrg->nummultibonds; ++j) {
						int jj;
						printf("(");
						for (jj = 0; jj < mol->Natom; ++jj)
							if (bs_isset(matchset[j], jj))
								printf("%i ", jj+1);
						printf(")");
					}
					matchmultiringbond(info, mol, mrg, foundrings, position, matchset, 0);
				}
				printf("\n");

				for (j = 0; j < mrg->nummultibonds; ++j)
					if (matchset[j] != NULL)
						free(matchset[j]);
				free(matchset);
				free(position);
			}

			foundrings[pos] = 0;
			pendrings[i] = mrg->ringtypes[pos];
		}
	}
}


void findmultigrp(struct struct_info *info, struct mol *mol, struct multiringgrp *mrg)
{
	int *pendrings, *foundrings;
	int i;

	if (mrg->numrings > 1) {
		pendrings = calloc(1, sizeof(int) * mol->Nring);
		assert(pendrings);
		foundrings = calloc(1, sizeof(int) * mrg->numrings);
		assert(foundrings);

		for (i = 0; i < mol->Nring; ++i)
			pendrings[i] = mol->rings[i].ringtype;

		matchmultiring(info, mol, mrg, pendrings, foundrings, 0);

		free(pendrings);
		free(foundrings);
	}
}


char *list_rings(struct struct_info *info, struct mol *mol)
{
	struct struct_log *buf = init_log(100);
	int i, j, k;
	struct ringpath *ring;
	struct multiring *mring;
	

	for(i = 0; i < mol->Nring; ++i) {
		ring = &mol->rings[i];
		if(!ring->multiring) {
			logadd(buf, "(%i", ring->path[0] + 1);
			for(j = 1; j < ring->len - 1; ++j)
				logadd(buf, " %i", ring->path[j] + 1);
			if(mol->rings[i].type & RING_AROMATIC)
				logadd(buf, ")*");
			else
				logadd(buf, ")");

			if(ring->ringtype) {
				logadd(buf, " %s", info->variables[ring->ringtype].name);
			}
			logadd(buf, "\n");
		}
	}

	for(i = 0; i < mol->Nmring; ++i) {
		mring = &mol->mrings[i];
		logadd(buf, "%s\n", info->variables[mring->ringtype].name);

		for(j = 0; j < mring->Nring; ++j){
			ring = &mol->rings[mring->rings[j].ringnum];

			logadd(buf, "\t(%i", ring->path[0] + 1);
			for(k = 1; k < ring->len - 1; ++k)
				logadd(buf, " %i", ring->path[k] + 1);
			if(mol->rings[j].type & RING_AROMATIC)
				logadd(buf, ")*");
			else
				logadd(buf, ")");

			if(ring->ringtype) {
				logadd(buf, " %s", info->variables[ring->ringtype].name);
			}
			logadd(buf, "\n");
		}
		logadd(buf, "\n");
	}

	return(log2str(buf));
}



