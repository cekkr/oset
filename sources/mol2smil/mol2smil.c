#include "../mollib/mollib.h"


void main(int argc, char *argv[])
{
	struct mol *mol;
	struct mol_list *mollist;
	int i;

	if (argc < 2) {
		printf("Use: %s [molfilename]\n\n", argv[0]);
		return;
	}
	init_primes();

	if ((mol = readmolfile(argv[1])) != NULL) {
		mollist = new_mol_list(NULL);
		separate_mols(mol, mollist);

		for (i = 0; i < mollist->Nmol; ++i) {
			canonicalize(mollist->mols[i]);
			find_rings(mollist->mols[i]);
			detect_aromaticity(mollist->mols[i]);
			printf("\"%s\"\n", mol2smiles(mollist->mols[i]));
		}
		
		destroy_mol_list(mollist);
		destroy_mol(mol);
	} else
		printf("Error, file %s\n", argv[1]);
}
