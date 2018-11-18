/* server.c
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

#include "server.h"
#include <time.h>

#define DEFAULT_PORT	8000
#define QLEN			5

void comm_server(struct sock_data *t);

static struct struct_info *info;

int main(int argc, char *argv[])
{
	unsigned short port = DEFAULT_PORT;
	FILE *f = NULL;

	logfile("Starting OSET Server\n");
	printf("Organic Sythesis Exploration Tool\n\n");

	if (argc > 1)
		port = atoi(argv[1]);

	init_primes();

	#ifdef LINUX
		if(f = fopen("/home/httpd/cgi-bin/caos/port", "w")) {
			fprintf(f, "%i\n", port);	
			fclose(f);
		} else {
			logfile("Fatal error: Could not write port file.\n");
			return(1);
		}
	#endif 

	info = init_comp_info();

	logfile("Read %i reactions\n", info->numreact);

	sock_server(port, QLEN, comm_server);
	return(0);
}


/* server functions */
BOOL do_quit(struct sock_data *s, char *buff);
BOOL do_help(struct sock_data *s, char *buff);
BOOL do_load(struct sock_data *s, char *buff);
BOOL do_analyze(struct sock_data *s, char *buff);
BOOL do_analyzeaux(struct sock_data *s, char *buff);
BOOL do_analyzefg0(struct sock_data *s, char *buff);
BOOL do_rxninfo(struct sock_data *s, char *buff);
BOOL do_cas(struct sock_data *s, char *buff);
BOOL do_savetree(struct sock_data *s, char *buff);
BOOL do_start(struct sock_data *s, char *buff);

static struct functabl {
	char *command;
	BOOL (*func)(struct sock_data *, char *);
	char *params;
	char *comments;
} functabl[] = {
	{"QUIT", do_quit, "", "kills the server process"},
	{"HELP", do_help, "[func]", "shows info on a function"},
	{"LOAD", do_load, "fname", "returns the fname mol from server"},
	{"ANALYZE", do_analyze, "mol", "returns a molmetalist with possible precursors of mol"},
	{"ANALAUX", do_analyzeaux, "atom mol", "returns a molmetalist with possible precursors of mol by FGI, FGA"},
	{"ANALFG0", do_analyzefg0, "bond mol", "returns a molmetalist with possible precursors of mol by FG0"},
	{"RXNINFO", do_rxninfo, "rxnnum", "returns description, comments, etc. for rxnnum"},
	{"CAS", do_cas, "mol", "returns name and Chemical Abstracs Registry Number (CAS) from mol"},
	{"SAVETREE", do_savetree, "tree", "writes tree to output.sk2 file"},
	{"START", do_start, "molfile", "saves molfile and then executes LOAD function"}
};


BOOL do_quit(struct sock_data *s, char *buff)
{
	sockprintf(s, "BYE\n");
	return(FALSE);
}


BOOL do_help(struct sock_data *s, char *buff)
{
	int i;

	buff = strtok(buff, " \r\n");

	if (buff) {
		for (i = 0; i < sizeof(functabl)/sizeof(functabl[0]); ++i) {
			if (!strnicmp(buff, functabl[i].command, strlen(functabl[i].command))) {
				sockprintf(s, "%s %s\n%s\n\n", functabl[i].command, functabl[i].params, functabl[i].comments);
				break;
			}
		}

		if (i == sizeof(functabl)/sizeof(functabl[0])) 
			sockprintf(s, "No info on '%s'\n\n", buff);
	} else {
		sockprintf(s, "CAOSSERVER COMMANDS\n");
		for (i = 0; i < sizeof(functabl)/sizeof(functabl[0]); ++i) 
			sockprintf(s, "%s ", functabl[i].command);
		sockprintf(s, "\n\nUse help [command] for more info\n\n");
	}

	return(TRUE);
}

BOOL do_load(struct sock_data *s, char *buff)
{
	char *aux;
	//FILE *f;
	//char linea[200];

	aux = strtok(buff, " \r\n");

	if (!aux) {
		sockprintf(s, "- Missing arg: fname\n");
	} else {
		struct mol *mol;
		/* mutex */
		mutex_lock(s->mutex);
		mol = readmolfile(aux);
		mutex_unlock(s->mutex);

		if (mol != NULL) {
			parse_mol(info, mol, NULL);
			/*
			canonicalize(mol);
			find_rings(mol);
			detect_aromaticity(mol);
			mol->smiles = mol2smiles(mol);
			*/
			sockprintf(s, "+ OK\n");
			mol2trans(info,s,mol);
			destroy_mol(mol);			
		} else
			sockprintf(s, "- Err: %s\n", geterr());
	}
	
	
/*	
	if ((f = fopen(aux, "rt")) != NULL) {
		sockprintf(s, "+ OK\n");
		while (fgets(linea, sizeof(linea), f))		
			sockprintf(s, "%s", linea);
		sockprintf(s,".\n");
		fclose(f);
	} else
		sockprintf(s, "- File not found\n");
*/
	return(TRUE);
}


BOOL do_start(struct sock_data *s, char *b)
{
	char *fname = tmpnam(NULL);
	FILE *f;
	char buff[200];

    if ((f = fopen(fname, "w")) != NULL) {
		do {
			sockgets(s, buff, sizeof(buff));
			if (strstr(buff, "M  END"))
				break;
			fwrite(buff, strlen(buff), 1, f);
		} while (1);
		fclose(f);

		return(do_load(s, fname));
	} else
		sockprintf(s, "-Err: can't write file");
	return(TRUE);
}


BOOL do_cas(struct sock_data *s, char *buff)
{
	struct smilecas_entry *entry = NULL;
	char *aux;

	aux = strtok(buff, " \r\n");

	entry = findsmilecas(info, aux);
	if (entry != NULL) {
		sockprintf(s, "+ OK\n\t\"%s\" \"%s\"\n", entry->name, entry->cas);
		free_smilecas(entry);
	} else {
		printf("CAS not found\n");
		sockprintf(s, "- NOT FOUND\n");
	}
	
	return(TRUE);
}

BOOL do_analyze(struct sock_data *s, char *buff)
{
	struct mol *mol;
	struct mol_metalist *molmetalist;
	clock_t c1, c2, c3, c4;

	c1 = clock();
	sockprintf(s, "+ OK, waiting for mol\n");
	
	if ((mol = trans2mol(s)) != NULL) {
		c2 = clock();
		parse_mol(info,mol,NULL);
		/* mutex */
		mutex_lock(s->mutex);
		molmetalist = analyze_mol(info,mol);
		mutex_unlock(s->mutex);

		if (molmetalist != NULL) {
			c3 = clock();
			sockprintf(s, "+ OK\n");
			meta2trans(info, s, molmetalist);
			c4 = clock();

			destroy_mol_metalist(molmetalist);

			logfile("Elapsed time: %.2lfs communications / %.2lfs processing\n", (double)(c2-c1+c4-c3) / CLOCKS_PER_SEC, (double)(c3-c2) / CLOCKS_PER_SEC);
		}
		destroy_mol(mol);
	} else
		sockprintf(s, "- Err: %s\n", geterr());
	
	return(TRUE);
}


BOOL do_analyzeaux(struct sock_data *s, char *buff)
{
	struct mol *mol;
	struct mol_metalist *mml1, *mml2;
	int		atom;
	char *aux;

	aux = strtok(buff, " \r\n");

	if (!aux) {
		sockprintf(s, "- Missing arg: atom\n");
	} else {
		sockprintf(s, "+ OK, waiting for mol\n");

		atom = atoi(aux);
		
		if ((mol = trans2mol(s)) != NULL) {
			parse_mol(info,mol,NULL);
			/* mutex */
			mutex_lock(s->mutex);
			mml1 = analyze_molfgi(info, mol,atom,-1);
			mml2 = analyze_molfga(info, mol,atom,-1);
			mutex_unlock(s->mutex);

			if ((mml1 != NULL) && (mml2 != NULL)) {
				mol_metalist_cat(mml1, mml2);

				sockprintf(s, "+ OK\n");
				meta2trans(info, s, mml1);

				destroy_mol_metalist(mml1);
				disband_mol_metalist(mml2);
			}
			destroy_mol(mol);
		} else
			sockprintf(s, "- Err: %s\n", geterr());
		
		return(TRUE);
	}

	return(FALSE);
}


BOOL do_analyzefg0(struct sock_data *s, char *buff)
{
	struct mol *mol;
	struct mol_metalist *mml;
	int		bond;
	char	*aux;

	aux = strtok(buff, " \r\n");

	if (!aux) {
		sockprintf(s, "- Missing arg: atom\n");
	} else {
		sockprintf(s, "+ OK, waiting for mol\n");

		bond = atoi(aux);
		
		if ((mol = trans2mol(s)) != NULL) {
			parse_mol(info,mol,NULL);
			/* mutex */
			mutex_lock(s->mutex);
			mml = analyze_molgp0(info, mol,mol->bonds[bond].a1, mol->bonds[bond].a2);
			mutex_unlock(s->mutex);

			if (mml != NULL) {
				sockprintf(s, "+ OK\n");
				meta2trans(info, s, mml);

				destroy_mol_metalist(mml);
			}
			destroy_mol(mol);
		} else
			sockprintf(s, "- Err: %s\n", geterr());
		
		return(TRUE);
	}

	return(FALSE);
}


BOOL do_rxninfo(struct sock_data *s, char *buff)
{
	int rxnnum = atoi(buff);
	struct reaction *rxn;

	if ((rxnnum > 0) && (rxnnum <= info->numreact)) {
		rxn = &info->reaction[rxnnum-1];
		sockprintf(s, "+ OK\n");
		sockprintf(s, "\tNAME \"%s\"\n",  rxn->name);
		if (rxn->link)
			sockprintf(s, "\tLINK\"%s\"\n", rxn->link);
		sockprintf(s, "\tCOMMENTS \"%s\"\n", rxn->comments ? rxn->comments : "");
		sockprintf(s, "\n");
	} else
		sockprintf(s, "- Err: No info on rxnnum %i\n", rxnnum);

	return(TRUE);	
}


BOOL do_savetree(struct sock_data *s, char *b)
{
	char buff[200];

	sockprintf(s, "+ OK, waiting for tree\n");

	sockgets(s, buff, sizeof(buff));
	logsockfile(s, "> %s", buff);
	if (strnncmp(buff, "STARTTREE")) {
		seterr("Expected STARTTREE");
		return(FALSE);
	}

	writesk(s);
	
	return(TRUE);
}





void comm_server(struct sock_data *s)
{

	int  i;
	char buff[100];
	BOOL ok = TRUE;

	/* conversation */
	logsockfile(s, "Accepted connection\n");

	sockprintf(s, "CAOSSERVER\n");

	while (ok) {
		sockgets(s, buff, sizeof(buff));
		logsockfile(s, "%s", buff);

		for (i = 0; i < sizeof(functabl)/sizeof(functabl[0]); ++i) {
			if (!strnicmp(buff, functabl[i].command, strlen(functabl[i].command))) {
				ok = functabl[i].func(s, buff+strlen(functabl[i].command));
				break;
			}
		}

		if (i == sizeof(functabl)/sizeof(functabl[0]))
			sockprintf(s, "Unknown command! Use HELP\n");
	}
}


