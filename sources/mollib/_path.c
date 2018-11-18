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

#include "mollib.h"


#define	MAXPATH		10



void findpath(struct mol *mol, int act, int goal, struct paths *paths);
BOOL inpath(struct path *path, int atom);

struct paths *findpaths(struct mol *mol, int atom1, int atom2)
{
	struct paths *paths;

	paths = malloc(sizeof(struct paths));
	assert(paths);
	memset(paths, 0, sizeof(struct paths));

	paths->paths = malloc(sizeof(struct path));
	assert(paths->paths);
	memset(paths->paths, 0, sizeof(struct path));

	paths->paths->atoms = malloc(sizeof(int) * MAXPATH);
	assert(paths->paths->atoms);
	memset(paths->paths->atoms, 0, sizeof(int) * MAXPATH);

	findpath(mol, atom1, atom2, paths);
	free(paths->paths->atoms);
	memmove(paths->paths, &paths->paths[1], sizeof(struct path) * paths->numpaths);

	return(paths);
}


void findpath(struct mol *mol, int act, int goal, struct paths *paths)
{
	struct atom *atom = &mol->atoms[act];
	int i;

	if (paths->paths->len < (MAXPATH-1)) {
		for (i = 0; i < atom->Nbond; ++i) {
			if (!inpath(paths->paths, atom->neighbors[i])) {
				/* add to path */
				paths->paths->atoms[paths->paths->len] = act;
				++paths->paths->len;
				
				if (atom->neighbors[i] == goal) {
					/* emit path */
					paths->paths->atoms[paths->paths->len] = goal;
					++paths->paths->len;

					paths->paths = realloc(paths->paths, sizeof(struct path) * (paths->numpaths + 2));
					assert(paths->paths);
					memset(&paths->paths[paths->numpaths+1], 0, sizeof(struct path));
					paths->paths[paths->numpaths+1].len = paths->paths->len;
					paths->paths[paths->numpaths+1].atoms = malloc(sizeof(int) * paths->paths->len);
					assert(paths->paths[paths->numpaths+1].atoms);
					memmove(paths->paths[paths->numpaths+1].atoms, paths->paths->atoms, sizeof(int) * paths->paths->len);
					++paths->numpaths;

					--paths->paths->len;
				} else {
					/* keep searching */
					findpath(mol, atom->neighbors[i], goal, paths);
				}
				/* remove from path */
				--paths->paths->len;
			}
		}
	}
}



BOOL inpath(struct path *path, int atom) 
{
	int i;
	BOOL ret = FALSE;

	for (i = 0; (i < path->len) && !ret; ++i) 
		ret = path->atoms[i] == atom;

	return(ret);
}



void freepaths(struct paths *paths)
{
	int i;

	for (i = 0; i < paths->numpaths; ++i) 
		free(paths->paths[i].atoms);

	free(paths->paths);
	free(paths);
}

