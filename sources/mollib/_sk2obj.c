/* _sk2obj.c
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

#include <stdarg.h>
#include "sk2.h"

int fputGeomShape(FILE *fp, GeomShapeFormat *g);
GeomShapeFormat *DestroyGeomShape(GeomShapeFormat *shape);

int fputObject(FILE *fp, struct object *obj)
{
	int sizepos, beginpos, endpos, size;

	fputByte(fp, obj->type);  
	fputByte(fp, 0);			
	sizepos = ftell(fp);
	fputLongInt(fp, 0);
	beginpos = ftell(fp);

	switch(obj->type){
		case MOLECULE:{
			int sizepos2, beginpos2, endpos2, size2;
			int i;
			MoleculeFormat *mol = (MoleculeFormat *) obj->content;

			fputByte(fp, mol->Version);
			fputByte(fp, mol->NewProperties);
			fputInteger(fp, mol->AtomsCount);

			for(i = 0; i < mol->AtomsCount; ++i){  // put atoms
				AtomDataFormat *atom = (AtomDataFormat *) mol->Atoms[i];

				sizepos2 = ftell(fp);
				fputInteger(fp, 0); // 	 atom record size
				beginpos2 = ftell(fp);

				fputSingle(fp, atom->AutoScale);
				fputByte(fp, atom->NewProperties);
				fputInteger(fp, atom->Style);
				fputInteger(fp, atom->ElementNum);
				fputShortInt(fp, atom->Valence);
				fputShortInt(fp, atom->Charge);
				fputInteger(fp, atom->Isotope);
				fputPChar(fp, atom->NumStr);
				fputSingle(fp, atom->X);
				fputSingle(fp, atom->Y);
				fputSingle(fp, atom->Z);
				fputByte(fp, atom->HydrogenPosition);
				fputByte(fp, atom->PersonalShowState);
				fputByte(fp, atom->FixStates);
				fputByte(fp, atom->ShowStates);
				fputBoolean(fp, atom->SubStructureExist);
				fputWord(fp, atom->AtomLabel);

				endpos2 = ftell(fp);
				size2 = endpos2-beginpos2;
				fseekputInteger(fp, (Integer)size2, sizepos2);
			}

			fputInteger(fp, mol->BondsCount);

			for(i = 0; i < mol->BondsCount; ++i){  // put bonds
				BondDataFormat *bond = (BondDataFormat *) mol->Bonds[i];

				sizepos2 = ftell(fp);
				fputInteger(fp, 0); // 	 bond record size
				beginpos2 = ftell(fp);

				fputSingle(fp, bond->AutoScale);
				fputByte(fp, bond->NewProperties);
				fputInteger(fp, bond->Style);
				fputInteger(fp, bond->IndexOfFirstAtom);
				fputInteger(fp, bond->IndexOfSecondAtom);
				fputByte(fp, bond->BondType);
				fputTPoint(fp, bond->Reserved[0]);
				fputTPoint(fp, bond->Reserved[1]);
				fputByte(fp, bond->StereoView);
				fputByte(fp, bond->SymmetryOfDoubleBond);
				fputByte(fp, bond->Reservedb);
				fputByte(fp, bond->RingState);

				endpos2 = ftell(fp);
				size2 = endpos2-beginpos2;
				fseekputInteger(fp, (Integer)size2, sizepos2);
			}

			fputInteger(fp, mol->NumberofRadicalAtom);
			fputInteger(fp, mol->AromaRingsCount);

			}break;

		case LINE:
		case POLYLINE: {
			LineFormat *line = (LineFormat *)obj->content;
			fputByte(fp, line->Version);
			fputInteger(fp, line->PenStyleIndex);
			fputInteger(fp, line->BrushStyleIndex);
			fputInteger(fp, line->ArrowStyleIndex);
			fputGeomShape(fp, line->GeomShape);
		}break;

		case GROUP: {
			GroupFormat *group = (GroupFormat *)obj->content;
			int i;
			fputByte(fp, group->Version);
			fputInteger(fp, (Integer)group->ObjectsList->Nobj);
			for(i = 0; i < group->ObjectsList->Nobj; ++i)   // ObjectsList
				fputObject(fp, &group->ObjectsList->objs[i]);
		}break;

	}
	endpos = ftell(fp);
	size = endpos-beginpos;
	fseekputLongInt(fp, size, sizepos);

	return(size);

}

void *DestroyObject(struct object *obj)
{
	switch(obj->type){
		case MOLECULE:{
			MoleculeFormat *mol = (MoleculeFormat *) obj->content;
			int i;

			for(i = 0; i < mol->AtomsCount; ++i){  // destroy atoms
				AtomDataFormat *atom = mol->Atoms[i];
				if(atom->NumStr)
				 free(atom->NumStr);
				free(atom);
			}
			free(mol->Atoms);

			for(i = 0; i < mol->BondsCount; ++i)  // destroy bonds
				free(mol->Bonds[i]);
			free(mol->Bonds);
			free(mol);
			}break;

		case LINE:
		case POLYLINE: {
			LineFormat *line = (LineFormat *)obj->content;
			DestroyGeomShape(line->GeomShape);
			free(line);
		}break;

		case GROUP: {
			GroupFormat *group = (GroupFormat *)obj->content;
			int i;
			for(i = 0; i < group->ObjectsList->Nobj; ++i)   // ObjectsList
				DestroyObject(&group->ObjectsList->objs[i]);
			free(group->ObjectsList->objs);
			free(group->ObjectsList);
			free(group);
		}break;

	}
	return(NULL);
}

GeomShapeFormat *DestroyGeomShape(GeomShapeFormat *shape)
{
	int i;

	for(i = 0; i < shape->NodesCount; ++i)
		free(shape->Nodes[i]);
	free(shape->Nodes);
	free(shape);
	return(NULL);
}

int fputGeomShape(FILE *fp, GeomShapeFormat *g)
{
	int i;

	fputInteger(fp, g->NodesCount);
	fputBoolean(fp, g->IsShapeClosed);

	for(i = 0; i < g->NodesCount; ++i){
		fputByte(fp, g->Nodes[i]->NodeType);
		fputTRPoint(fp, g->Nodes[i]->Pos);
	}

	return(0);
}

ObjectList *CreateObjList()
{
	ObjectList *list;

	list = calloc(1, sizeof(ObjectList));
	assert(list);
	return(list);
}


GroupFormat *CreateGroup()
{
	GroupFormat *group;

	group = calloc(1, sizeof(GroupFormat));
	assert(group);
	group->ObjectsList = CreateObjList();
	return(group);
}



int addobj_sk2(struct object_list *list, void *obj, int type)
{
	list->objs = realloc(list->objs, sizeof(struct object) * (list->Nobj + 1));
	assert(list->objs);
	list->objs[list->Nobj].content = obj;
	list->objs[list->Nobj].type = type;
	++list->Nobj;
	return(list->Nobj);
}


MoleculeFormat *CreateMolecule()
{
	MoleculeFormat *mol;

	mol = malloc(sizeof(MoleculeFormat));
	assert(mol);

	mol->Version = 0;	   				// Should be zero
	mol->NewProperties = 0;				// Should be zero
	mol->AtomsCount = 0;
	mol->Atoms = NULL;
	mol->Bonds = NULL;
	mol->BondsCount = 0;
	mol->NumberofRadicalAtom = -1;	// For independent structures should be < 0
	mol->AromaRingsCount  = 0;		// For the moment should be zero

	return(mol);
}

/* Returns the valence of element with atomic number z. Only works for
 * "representative elements" (blocks s and p) */
int Valence(int z)
{
	if (z==1) 
		return(1);
	if (z<=20) 
		return(4-abs((z-2)%8-4));
	if ((z<=30) || ((z>=39) && (z<=48))) 
		return(0);
	if (z<=56) 
		return(4-abs((z-4)%8-4));
	return(0);
}


AtomDataFormat *CreateAtom(struct sk2info *info, Integer ElementNum, Single X, Single Y, Single Z, ShortInt Charge, Integer Style)
{
	AtomDataFormat *atom;

	if(Style < 0) { // use default style
		if(info->defatomstyle < 0) // if the default style does not exist, create it.
			info->defatomstyle = addstyle(info, CreateDefAtomStyle(), ATOMSTYLE);
		Style = info->defatomstyle;
	}

	atom = malloc(sizeof(AtomDataFormat));
	assert(atom);

	atom->AutoScale = 1.0;
	atom->NewProperties = 0;
	atom->Style = Style;
	atom->ElementNum = ElementNum;
	atom->Valence = Valence(ElementNum + 1);
	atom->Charge = Charge;
	atom->Isotope = 32766;
	atom->NumStr = NULL;
	atom->X = X;
	atom->Y = Y;
	atom->Z = Z;
	atom->HydrogenPosition = HPOS_AUTO;
	atom->PersonalShowState = PSHOWSTATE_UNDEFINED;
	atom->FixStates = 0;
	atom->ShowStates = SHOWSTATES_CROSSOUT_INVALID_ATOM | SHOWSTATES_SHOW_HYDROGENS;
	atom->SubStructureExist = FALSE;
	atom->AtomLabel = 0;

	return(atom);
}

BondDataFormat *CreateBond(struct sk2info *info, Integer a1, Integer a2, Byte Type, Integer Style)
{
	BondDataFormat *bond;

	if(Style < 0) { // use default style
		if(info->defbondstyle < 0) // if the default style does not exist, create it.
			info->defbondstyle = addstyle(info, CreateDefBondStyle(), BONDSTYLE);
		Style = info->defbondstyle;
	}

	bond = malloc(sizeof(BondDataFormat));
	assert(bond);

	bond->AutoScale = 1.0;
	bond->NewProperties = 0;
	bond->Style = Style;
	bond->IndexOfFirstAtom = a1;
	bond->IndexOfSecondAtom = a2;
	bond->BondType = Type;
	memset(bond->Reserved, 0, sizeof(bond->Reserved));
	bond->StereoView = 0;
	bond->SymmetryOfDoubleBond = 0;
	bond->Reservedb = 1;
	bond->RingState = 0;

	return(bond);
}

int AddAtom(MoleculeFormat *mol, AtomDataFormat *atom)
{
	mol->Atoms = realloc(mol->Atoms, sizeof(AtomDataFormat *) * (mol->AtomsCount + 1));
	assert(mol->Atoms);
	mol->Atoms[mol->AtomsCount] = atom;
	++mol->AtomsCount;
	return(mol->AtomsCount - 1);
}


int AddBond(MoleculeFormat *mol, BondDataFormat *bond)
{
	mol->Bonds = realloc(mol->Bonds, sizeof(BondDataFormat *) * (mol->BondsCount + 1));
	assert(mol->Bonds);
	mol->Bonds[mol->BondsCount] = bond;
	++mol->BondsCount;
	return(mol->BondsCount - 1);
}


TRRect GetMolRect(MoleculeFormat *mol)
{
	TRRect r = {0.0, 0.0, 0.0, 0.0};
	AtomDataFormat *atom;
	int i;

	if(mol->AtomsCount > 0) {
		r.top = mol->Atoms[0]->Y;
		r.bottom = mol->Atoms[0]->Y;
		r.right = mol->Atoms[0]->X;
		r.left = mol->Atoms[0]->X;

		for(i=1; i < mol->AtomsCount; ++i)
		{
			atom = mol->Atoms[i];
			if(atom->X < r.left)
				r.left = atom->X;
			else if(atom->X > r.right)
				r.right = atom->X;
			if(atom->Y < r.top)
				r.top = atom->Y;
			else if(atom->Y > r.bottom)
				r.bottom = atom->Y;
		}
	}

	return(r);
}

void SetMolRect(MoleculeFormat *mol, TRRect r, Single margin, BOOL change_aspect)
{
	TRRect oldrect;
	TRPoint scale, delta;
	int i;

	oldrect = GetMolRect(mol);
	if(r.left < r.right) {
		r.left += margin;
		r.right -= margin;
	} else {
		r.left -= margin;
		r.right += margin;
	}
	if(r.top < r.bottom) {
		r.top += margin;
		r.bottom -= margin;
	} else {
		r.top -= margin;
		r.bottom += margin;
	}

	scale.X = (r.right-r.left)/(oldrect.right-oldrect.left);
	scale.Y = (r.bottom-r.top)/(oldrect.bottom-oldrect.top);
	
	if(!change_aspect){
		if(scale.X > scale.Y) {
			scale.X = scale.Y;  // center x
			delta.X = ((r.right-r.left) - scale.X * (oldrect.right-oldrect.left)) / 2;
		} else {
			scale.Y = scale.X; // center y
			delta.Y = ((r.bottom-r.top) - scale.Y * (oldrect.bottom-oldrect.top)) / 2;
		}
	}

	for(i = 0; i < mol->AtomsCount; ++i){
		mol->Atoms[i]->X = (mol->Atoms[i]->X - oldrect.left) * scale.X + r.left + delta.X;
		mol->Atoms[i]->Y = (mol->Atoms[i]->Y - oldrect.top) * scale.Y + r.top + delta.Y;
	}
	
}

GeomNodeFormat *CreateNode(Single X, Single Y)
{
	GeomNodeFormat *n;

	n = malloc(sizeof(GeomNodeFormat));
	assert(n);

	n->NodeType = 0;
	n->Pos.X = X;
	n->Pos.Y = Y;
	return(n);
}

GeomShapeFormat *CreateGeomShape()
{
	GeomShapeFormat *g;

	g = calloc(1, sizeof(GeomShapeFormat));
	assert(g);
	return(g);
}

int AddNode(GeomShapeFormat *g, GeomNodeFormat *node)
{
	g->Nodes = realloc(g->Nodes, (g->NodesCount + 1) * sizeof(GeomNodeFormat *));
	assert(g->Nodes);
	g->Nodes[g->NodesCount] = node;
	++g->NodesCount;
	return(g->NodesCount-1);
}


LineFormat *CreateLine(struct sk2info *info, int Pen, int Arrow)
{
	LineFormat *line;

	if(Pen < 0) { // use default style
		if(info->defpenstyle < 0) // if the default style does not exist, create it.
			info->defpenstyle = addstyle(info, CreateDefPenStyle(), PENSTYLE);
		Pen = info->defpenstyle;
	}

	if(Arrow < 0) { // use default style
		if(info->defarrowstyle < 0) // if the default style does not exist, create it.
			info->defarrowstyle = addstyle(info, CreateDefArrowStyle(), ARROWSTYLE);
		Arrow = info->defarrowstyle;
	}

	line = malloc(sizeof(LineFormat));
	assert(line);

	line->Version = 0;
	line->PenStyleIndex = Pen;
	line->BrushStyleIndex = -1;
	line->ArrowStyleIndex = Arrow;
	line->GeomShape = CreateGeomShape();

	return(line);
}


LineFormat *CreateLineEx(struct sk2info *info, int Pen, int Arrow, int Npoint, double x, ...)
{
	va_list marker;
	LineFormat *line;
	Single p[2];
	int i;

	line = CreateLine(info, Pen, Arrow);
	va_start(marker, x);
	for(i = 0; i < (Npoint*2); ++i) {
		p[i % 2] = (Single)x;
		if((i % 2) == 1)
			AddNode(line->GeomShape, CreateNode(p[0], p[1]));
		x = va_arg(marker, double);
	}
	va_end(marker);
	
	return(line);
}



