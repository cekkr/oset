/* oset.c
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

#include "../mollib/mollib.h"
int test_sk2(struct mol *mol);

void debug_check(int flag);

/* This is main() for the command line version of the program.
 * The server version uses main() from server.c.
 * The Windows version uses winmain() from molshow.c    */
int main(int argc, char *argv[])
{
	struct struct_info *info;
	struct mol *mol;
	struct chem_err *err;
	char s[100];
	struct mol_metalist *metalist = NULL;
	int i = 0; 

	float x = 2.0;

	
	if(argc > 1) {

		strcpy(s,argv[1]);

		if(strncmp(s, "-s", 2) == 0) {
			mol = smiles2mol(s + 2);
			if(mol == NULL) {
				printf("Error parsing smiles\n");
				printf("%s\n", geterr());	
				exit(1);
			}
		} else {
			mol = readmolfile(s);
			if(mol == NULL){
				printf("Error reading molfile: %s\n\t", s);
				printf("%s\n", geterr());	
				exit(1);
			}
		}

		info = init_comp_info();
		err = _init_err();

		parse_mol(info, mol, err);
		
		test_sk2(mol);
//		printf("%s\n", mol->smiles);

		if (argc == 5) {
			switch (toupper(argv[2][0])) {
				case 'I': {
						int atom = atoi(argv[3])-1;
						int gfunc = getvariable(info, argv[4]);

						printf("FGI MODE: ATOM=%i, GFUNC %s=%i\n", atom+1, argv[4], gfunc);
						metalist = analyze_molfgi(info, mol, atom, gfunc);
					}
					break;
				case 'A': {
						int atom = atoi(argv[3])-1;
						int gfunc = getvariable(info, argv[4]);
						
						printf("FGA MODE: ATOM=%i, GFUNC %s=%i\n", atom+1, argv[4], gfunc);
						metalist = analyze_molfga(info, mol, atom, gfunc);
					}
					break;
				case '0': {
						int atom1 = atoi(argv[3])-1;
						int atom2 = atoi(argv[4])-1;
						printf("GP0 MODE: ATOMS=[%i,%i]\n", atom1+1,atom2+1);
						metalist = analyze_molgp0(info, mol, atom1,atom2);
					}
					break;
				default:
					printf("Error... CAOSCOMP [molfile] [I(fgi)|A(fga)|0(gp0)] [n1] [n2]\n");
					break;
			}
		} else if(argc == 3) {
			if(toupper(argv[2][0]) == 'I'){
				char *mol_info = get_molinfo(info, mol);

				printf("%s", mol_info);
				free(mol_info);
			} else if (toupper(argv[2][0]) == 'N')
				printf("NOP mode\n");
		} else 
			metalist = analyze_mol(info, mol);

		destroy_mol_metalist(metalist);
	
	} else {
		printf("Organic Synthesis Exploration Tool\n");
		printf("Command line version 0.6\n");
		printf("Copyright (C) 2000 Ivan Tubert and Eduardo Tubert\n");
		printf("Contact: tubert@eros.pquim.unam.mx\n\n");
		printf("This program is free software; you can redistribute it and/or\n");
		printf("modify it under the terms of the GNU General Public License\n");
		printf("as published by the Free Software Foundation; either version 2\n");
		printf("of the License, or (at your option) any later version.\n");
		printf("All we ask is that proper credit is given for our work, which includes\n");
		printf("- but is not limited to - adding the above copyright notice to the beginning\n");
		printf("of your source code files, and to any copyright notice that you may distribute\n");
		printf("with programs based on this work.\n");
		printf("This program is distributed in the hope that it will be useful,\n");
		printf("but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
		printf("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n");
		printf("GNU General Public License for more details.\n");
		printf("You should have received a copy of the GNU General Public License\n");
		printf("along with this program; if not, write to the Free Software\n");
		printf("Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.\n\n");

		printf("oset [molfile | -sSMILES] [I(fgi)|A(fga)|0(gp0)] [n1] [n2]\n");
	}

	return(0);
}				
