/* gpofunc.h
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

#define CHEMATTRSIZE	4		// 132 bits

#ifndef PI
#define PI 3.1415926535
#endif

#define MAXATOM 255

struct atom
{	
	double x, y, z;	// Coordinates. z is ignored but kept in case the original molfile was 3D
	int Z,			// Atomic number
		charge,		// Charge
		nH,			// Number of implicit hydrogens
		Nbond,		// Number of explicit bonds (double and triple bonds count as one)
		nclass,		// Symmetry class. If two atoms in a mol have the same nclass, they are constitutionally equivalent
		canon_num,	// Canonic atom number. Used in the generation of unique SMILES
		valence;	// Total bonds, counting double bonds as two and triple as three.
	BOOL aromatic;	// Aromatic atom
	int *bonds,		// List of indices of explicit bonds in which the atom participates
		*neighbors;	// List of indices of neighboring explicit atoms, in the same order as *bonds.
	unsigned int chemattr[CHEMATTRSIZE]; // Bitset of non-exclusive chemical attributes
};

struct bond {
	int a1, a2, order;
};

struct gpofunc {
	int tipo;
	int *atomos;
	int repeat;
};


struct  mol
{
	int Natom,Nbond,Nelem,Nfunc,Nring,Nmring;
	struct atom *atoms;
	struct bond *bonds;
	struct gpofunc *gpofunc;
	struct ringpath *rings;
	struct multiring *mrings;
	unsigned char formula[4];
	struct mol_metalist *preclist;
	char *smiles;
	char *dbname;
	int complexity;
};

struct rxn_info
{
	int rxn;
	int rating;
	int simplification;
	int eval;
	char *echo;
};

struct mol_list
{
	int Nmol;
	struct mol **mols;
	struct rxn_info *rxn_info;
};


struct mol_metalist
{
	int Nlist;
	struct mol_list **lists;
};

struct analysisnode
{
	struct mol_metalist *mml;    // Current mol metalist
	struct analysisnode *target; // "parent"
	struct analysisnode ***precs; // precursors
	struct mol_list *orphans;
	int depth;
	struct targetkey {
		int rxn;
		int prec;
	} targetkey;
	void *user;
};


#define	MAXNEIGH	4

#define FGATOM_DONTCARE		0
#define FGATOM_AROMATIC		1
#define FGATOM_ALIPHATIC	2

struct funcgrpnode {
	struct funcgrpnodeatom {
		char Z;
		char charge;
		char aromaticity;	/* 0 = dont care, 1 = aromatic, 2 = aliphatic */
	} atoms[20];
	int		order;
	int		structural;		/* bool */
	int		min,max;
	struct  funcgrpnode *children[MAXNEIGH];
};


#define	MAXRINGGRPSIZE	12
#define MAXRINGMULTIBOND	3	
struct ringgrp {
	int  size;
	BOOL aromatic;
	long		hash;

	struct {
		char Z;
		int  order;
	} atoms[MAXRINGGRPSIZE];
};

struct rbond {
	int numring;
	int a1,a2;
};

struct mrbond {
	char name;
	struct rbond rbond[MAXRINGMULTIBOND];
};

struct multiringgrp {
	int numrings;
	int *ringtypes;
	int nummultibonds;
	struct mrbond *ringfussions;
	int ringtype;
};




#define MACROCYCLE	8

struct cplx_params
{
	int atom;
	int tertiary;
	int quaternary;
	int ring[MACROCYCLE+1];
	int fusion;
	int bridge;
};


#define	BOOL	int
#define	TRUE	1
#define	FALSE	0

#define CH_ERR_INITIAL_BUFSIZE	100
#define CH_ERR_BUFINC			100

struct chem_err
{
	int length;
	int bufsize;
	int num_err;
	char *error_log;
};

enum {H=1,He,Li,Be,B,C,N,O,F,Ne,Na,Mg,Al,Si,P,S,Cl,Ar,K,Ca,Sc,Ti,
 V,Cr,Mn,Fe,Co,Ni,Cu,Zn,Ga,Ge,As,Se,Br,Kr,Rb,Sr,Y,Zr,Nb,Mo,Tc,
 Ru,Rh,Pd,Ag,Cd,In,Sn,Sb,Te,I,Xe,Cs,Ba,La,Ce,Pr,Nd,Pm,Sm,Eu,Gd,
 Tb,Dy,Ho,Er,Tm,Yb,Lu,Hf,Ta,W,Re,Os,Ir,Pt,Au,Hg,Tl,Pb};
 

struct drect {
	double top, bottom, left, right;
};


// _clas2.c
struct chem_err *_init_err();
struct chem_err *_destroy_err(struct chem_err *err);
int __error_log(struct chem_err *err, char *error);
char *list_atomtypes(struct mol *mol);
int classify_atoms(struct mol *mol, struct chem_err *err);
BOOL isatomtype(struct mol *mol, int atom, int type);


// _molmem.c
struct mol *destroy_mol(struct mol *mol);
struct mol_list *destroy_mol_list(struct mol_list *list);
struct mol_metalist *destroy_mol_metalist(struct mol_metalist *metalist);
void reset_mol_list(struct mol_list *list);
void reset_mol(struct mol *mol);
void destroy_rings(struct mol *mol);
void destroy_fg(struct mol *mol);
struct mol *moldup(struct mol *mol);
struct mol *new_mol();
struct mol_list *mol_list_cat(struct mol_list *list1, struct mol_list *list2);
struct mol_metalist *mol_metalist_cat(struct mol_metalist *ml1, struct mol_metalist *ml2);
struct mol_list *append_mol(struct mol_list *list, struct mol *mol);
void delete_mol(struct mol_list *list, int n_mol);
struct mol_list *disband_mol_list(struct mol_list *list);
struct mol_list *disband_mol_metalist(struct mol_metalist *mml);
struct mol_list *new_mol_list(struct mol *mol);
struct mol_list *mol_listdup(struct mol_list *list);
struct mol_list *mol_listcpy(struct mol_list *list);
struct mol_metalist *append_mol_list(struct mol_metalist *mml, struct mol_list *list);
struct mol_metalist *new_mol_metalist(struct mol_list *list);
struct mol_metalist *delete_mol_list(struct mol_metalist *mml, int n_list);


// _moltopo.c : chemistry & topology
int new_bond(struct mol *mol, int a1, int a2);
int new_bond_ex(struct mol *mol, int a1, int a2, int order);
int new_atom(struct mol *mol, int Z, int charge, double x, double y, double z);
void breakbond(struct mol *mol, int nb);
void deleteatom(struct mol *mol, int n_atom);
void rotate_bondorder(struct mol *mol, int n_bond);
int inc_bondorder(struct mol *mol, int n_bond);
void dec_bondorder(struct mol *mol, int n_bond);
int are_bonded(struct mol *mol, int a1, int a2);
int separate_mols(struct mol *mol, struct mol_list *mol_list);
int setatomZ(struct mol *mol, int n_atom, int Z);
void _markatoms(struct mol *mol, int *used, int n_atom, int number);
void calc_formula(struct mol *mol);
void deleteatoms(struct mol *mol, int *used);
int are_bonded(struct mol *mol, int a1, int a2);
int bond_order(struct mol *mol, int a1, int a2);
int set_bondorder(struct mol *mol, int n_bond, int order);



// _molring.c
#define	RING_NORMAL			0x1
#define RING_CARBOCYCLIC	0x1
#define RING_HETERO			0x2
#define	RING_ABNORMAL		0x4

#define	RING_AROMATIC		0x100
#define RING_BENZENE		0x200


struct ringpath {
	int len;
	int *path;
	struct bitset *nodes;
	struct bitset *bonds;
	int IH, IA, ntied, type;
	BOOL dependent;
	BOOL multiring;
	
	long hash;
	int ringtype;
	int *ringpos;			/* rotation/mirror list, ends in 0 */
};

void find_rings(struct mol *mol);
void free_ringpath(struct ringpath *r);
void dup_ringpath(struct mol *mol, struct ringpath *dest, struct ringpath *src);
char *list_rings(struct struct_info *info, struct mol *mol);
void classify_rings(struct struct_info *info, struct mol* mol);
int *ringrotation(struct ringpath *ring, int pos);


struct multiring {
	int Nring;
	int ringtype;
	struct multiringmember {
		int ringnum;
		int rotation;
	} *rings;
};


// _aroma.c
void detect_aromaticity(struct mol *mol);
int aromatize(struct mol *mol);


// _molgeom.c
int sprout_atom(struct mol *mol, int fromatom, int Z);
struct mol *combine_mols(struct mol *mol1, struct mol *mol2, double dx, double dy);
void molorigin(struct mol *mol);
void normalize_mol(struct mol *mol, double bondsize);
struct drect getmolrect(struct mol *mol);
double getmeanbondlength(struct mol *mol);


// _smiles.c
char *mol2smiles(struct mol *mol);
struct mol *smiles2mol(char *smiles);
char *smilescanon(char *smiles);


// _chem.c
int valence(int z);
int group(int z);
int atomnum(char *sym);
int aromatomnum(char **sym, BOOL *arom);

// _fgfind.c
void busca_funcionales(struct struct_info *info, struct mol *mol);
char *list_fgroups(struct struct_info *info, struct mol *mol);

// _molfile.c
struct mol *readmolfile(char *filename);
struct struct_log * writemolfilebuf(struct mol *mol);
int writemolfile(char *filename, struct mol *mol);

// _mol2sk2.c
void putmol_sk2(struct sk2info *info, struct mol *mol, double left, double top, double right, double bottom, double margin);

// _moltran.c
#ifdef _SOCK
void mol2trans(struct struct_info *info, struct sock_data *s,struct mol *mol);
void list2trans(struct struct_info *info, struct sock_data *s, struct mol_list *mol_list);
void meta2trans(struct struct_info *info, struct sock_data *s, struct mol_metalist *mol_metalist);
struct mol *trans2mol(struct sock_data *s);
#endif


// analyze.c
struct mol_metalist *analyze_mol(struct struct_info *info, struct mol *mol);
struct mol_metalist *analyze_molfgi(struct struct_info *info, struct mol *mol, int targetatom, int gprec);
struct mol_metalist *analyze_molfga(struct struct_info *info, struct mol *mol, int targetatom, int gprec);
struct mol_metalist *analyze_molgp0(struct struct_info *info, struct mol *mol, int atom1, int atom2);
char *get_molinfo(struct struct_info *info, struct mol *mol);
struct struct_info *init_comp_info();

// _complex.c
void setdef_cplx_params(struct cplx_params *params);
int mol_complexity(struct mol *mol, struct struct_info *info);
struct cplx_params *init_cplx_params();

// _canon.c
void init_primes();
struct mol *canonicalize(struct mol *mol);
char *list_eq_classes(struct mol *mol);

// _path.c

// _tree.c
struct analysisnode *new_analysisnode(struct analysisnode *target, int rxn, int prec, struct mol_metalist *mml);

// _bigsmdb.c
struct smilecas_entry *findsmilecas(struct struct_info *info, char *smilesquery);
struct smilecas_entry *free_smilecas(struct smilecas_entry *entry);
void readsmilecas_index(struct struct_info *info);


struct path {
	int len;
	int *atoms;
};

struct paths {
	int numpaths;
	struct path *paths;
};

void freepaths(struct paths *paths);
struct paths *findpaths(struct mol *mol, int atom1, int atom2);



#ifdef _CAOSCOMP
int parse_mol(struct struct_info *info, struct mol *mol, struct chem_err *err);
void executegp1(struct struct_info *info, struct mol_list *mol_list, struct gpofunc *gpofunc, struct reaction *reaction);
void executegp2(struct struct_info *info, struct mol_list *mol_list, struct gpofunc *g1, struct gpofunc *g2, struct path *path, struct reaction *reaction);
void executefga(struct struct_info *info, struct mol_list *mol_list, int targetatom, struct reaction *reaction);
void executefg0(struct struct_info *info, struct mol_list *mol_list, int atom1, int atom2, struct reaction *reaction);
void executering(struct struct_info *info, struct mol_list *mol_list, struct ringpath *ring, struct reaction *reaction);
void executemring(struct struct_info *info, struct mol *mol, struct mol_list *mol_list, struct multiring *mring, struct reaction *reaction);

// analyze.c
char *get_rxntext(struct struct_info *info, struct rxn_info *rxn_info);
#endif

