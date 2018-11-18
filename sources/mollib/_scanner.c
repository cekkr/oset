/* _scanner.c
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
#include <limits.h>

#ifdef _DEBUG
BOOL debugscanner = FALSE;

char *tipo_tok[] = {
	"DUMMY", "EOF", "TOK_NUMERO", "TOK_STRING", "TOK_ID", "TOK_MUL", "TOK_ADD",
		"TOK_SUB", "TOK_DIV", "TOK_COMA", "TOK_PARENABRE", "TOK_PARENCIERRA",
		"TOK_IGUAL", "TOK_LT", "TOK_LE", "TOK_NE", "TOK_GE", "TOK_GT"
};
#endif 

char *token_string[] = {
	"EOF", "number", "string", "identifier", "*", "+",
		"-", "/", ",", "(", ")", "=", "<", "<=", "<>", ">=", ">", "[", "]"
};

char *reserved_string[] = {
".RXN", "NAME", "TYPE", "GP1", "GP2", "G1", "G2", ".CONDITIONS", ".COMMENTS",
".START", "FORATOM", "FORBOND", "FROM", "NEXT", "IF", "THEN", "ELSE", "IS", "ISNOT", "AND", "OR", "NOT", 
"ENDIF", "BREAKBOND", "MAKEBOND", "ADD", "ELIMINATE", "DONE", "PATH", "LINK", "RINGSIZE", "FGI1", "FGI2",
"GPREC", "FGA", "GP0", "NH", "ECHO", "RING", "RINGTYPE", "INCLUDE", "INCBONDORDER", "DECBONDORDER", "GETBONDORDER", "SETBONDORDER",
"ISEQ"};


//NOTE: this list must be in the same order as the enum in caoscomp.h and the string list
//in _clas2.c
//Also, chemical elements on this list should also appear in parser.c:add_statement()
char *reserved_chem[] = {
"METHYL", "PRIMARY", "SECONDARY", "TERTIARY", "QUATERNARY", "VINYL", "CARBONYL", "ALKYNYL",
"NITRILE", "ALLENE", "ALKYL", "SP3", "SP2", "SP", "PHENYL", "NONCARBON", 
 "HYDROXYL", "PEROXIDE", "ALLYL", "BENZYL", 
"ALPHA_CARBONYL", "ALPHA_ALKYNYL", "ALPHA_NITRILE", "H","C","N","O","X","Br", "Cl", "I",
"ALPHA_CH", "EWG", "ALPHA_EWG", "ALPHA_EWG2", "ALPHA_EWG3"};

long tellscanner(struct struct_info *info);


BOOL initscanner(struct struct_info *info, char *fname)
{
	BOOL ret = FALSE;
	FILE *f;

	if ((f = fopen(fname, "rb")) != NULL) {
		if (info->f) {
			/* push include */
			memmove(&info->include_stack[1], &info->include_stack[0], sizeof(struct include_stack) * (MAX_INCLUDE-1));
			info->include_stack[0].pos = tellscanner(info);
			info->include_stack[0].f = info->f;
			strcpy(info->include_stack[0].fname, info->fname);
			info->include_stack[0].line = info->line;
			++info->includelevel;
		}

		info->readoffs = BUF_SIZE+1;
		strcpy(info->fname, fname);
		info->f = f;
		scanner(info);				// pumps first token 
		ret = TRUE;
	} else
		logfile("Error opening file '%s'\n", fname);

	return(ret);
}


void deinitscanner(struct struct_info *info)
{

	/* pop include */
	if (info->f != NULL) {
		fclose(info->f);
		info->f = NULL;

		if (info->includelevel > 0) {
			int pos;

			info->f = info->include_stack[0].f;
			strcpy(info->fname, info->include_stack[0].fname);


			fseek(info->f, (((info->include_stack[0].pos + BUF_SIZE - 1) / BUF_SIZE) - 1) * BUF_SIZE, SEEK_SET);
			pos = fread(info->readbuff, 1, BUF_SIZE, info->f);
			if (pos < BUF_SIZE)
				info->readbuff[pos] = 26;		/* eof */

			info->readoffs = info->include_stack[0].pos % BUF_SIZE;
			info->line = info->include_stack[0].line;

			--info->includelevel;
			memmove(&info->include_stack[0], &info->include_stack[1], sizeof(struct include_stack) * (MAX_INCLUDE-1));

			scanner(info);				// pumps first token 
		}
	}
}


char readscanner(struct struct_info *info)
{
	char ret = 0;
	int  pos = 0;

	if (info->readoffs >= BUF_SIZE) {
		pos = fread(info->readbuff, 1, BUF_SIZE, info->f);
		info->readoffs = 0;
		if (pos < BUF_SIZE)
			info->readbuff[pos] = 26;		/* eof */
	}

	ret = info->readbuff[info->readoffs];
	++info->readoffs;

	return(ret);
}

void retractscanner(struct struct_info *info)
{
	if (info->readoffs <= 0) {
		fseek(info->f,-(BUF_SIZE * 2),SEEK_CUR);
		fread(info->readbuff, BUF_SIZE, 1, info->f);
		info->readoffs = BUF_SIZE;
	}

	--info->readoffs;
}

/*
void resetscanner(struct struct_info *info, long pos)
{
	_llseek(info->f,(pos / 1024) * 1024,SEEK_SET);

	_lread(info->f,info->readbuff,BUF_SIZE);
	info->readoffs = (int)(pos % 1024L);
}
*/

long tellscanner(struct struct_info *info)
{                 
	long pos = ftell(info->f);
	
	return((((pos + BUF_SIZE - 1) / BUF_SIZE) - 1) * BUF_SIZE + info->readoffs);
}




BOOL scanner_int(struct struct_info *info)
{											
	char ch;
	BOOL end = FALSE;
	BOOL eof = FALSE;
	int	state = 0;
	char s[1000];						   
	int ofs;
	int n;

	assert(info);


	if (info->token.valor)
		free(info->token.valor);
	memset(&info->token, 0, sizeof(struct token));

	while (!end) {
		ch = readscanner(info);

		switch (state) {
			case 0: 
				switch (ch) {
					case 26:
						/* eof */
						info->token.tipo = TOK_EOF;
						eof = TRUE;
						end = TRUE;
						break;

					case '\n':
						/* new line */
						++info->line;
						break;

					case ' ':
					case '\t':
					case '\r':
						/* whitespace */
						break;

					case ';':
						state = 1;	/* comment */
						break;

					case '"':
						info->token.tipo = TOK_STRING;
						ofs = 0;
						state = 2;
						break;

					case '*':
						info->token.tipo = TOK_MUL;
						end = TRUE;
						break;

					case '+':
						info->token.tipo = TOK_ADD;
						end = TRUE;
						break;

					case '-':
						info->token.tipo = TOK_SUB;
						end = TRUE;
						break;

					case '/':
						state = 7;
						break;

					case ',':
						info->token.tipo = TOK_COMA;
						end = TRUE;
						break;
					
					case '(':
						info->token.tipo = TOK_PARENABRE;
						end = TRUE;
						break;
					
					case ')':
						info->token.tipo = TOK_PARENCIERRA;
						end = TRUE;
						break;
					
					case '=':
						info->token.tipo = TOK_IGUAL;
						end = TRUE;
						break;

					case '[':
						info->token.tipo = TOK_OPENBRACK;
						end = TRUE;
						break;

					case ']':
						info->token.tipo = TOK_CLOSEBRACK;
						end = TRUE;
						break;

					case '<':
						state = 5;
						break;

					case '>':
						state = 6;
						break;


					default:
						if (isdigit(ch)) {
							info->token.tipo = TOK_NUMERO;
							state = 3;
							n = 0;
							retractscanner(info);							
						} else if (isalpha(ch) || (ch == '.') || (ch == '_')) {
							info->token.tipo = TOK_ID;
							state = 4;
							s[0] = ch;
							ofs = 1;
						}
						break;
				}
				break;

			case 1:		/* comment */
				if (ch == '\n') {
					++info->line;
					state = 0;
				}
				break;

			case 2:		/* string */
				if (ch == '"') {
					/* fin del string */
					s[ofs] = 0;
					info->token.valor = strdup(s);
					assert(info->token.valor);
					end = TRUE;
				} else if ((ch == '\r') || (ch == '\n')) {
					err_scan(info, "Endless string");
				} else {
					if(ofs < sizeof(s)-1) 
						s[ofs++] = ch;
					else {
						err_scan(info, "String too long");
						end = TRUE;
					}
				}
				break;

			case 3:		/* number */
				if (isdigit(ch)) {
					if (n >= INT_MAX / 10 - 9)
						err_scan(info, "Number too big");
					n *= 10;
					n += (ch - '0');
				} else {
					retractscanner(info);
					info->token.valor = malloc(sizeof(int));
					assert(info->token.valor);
					*((int *)info->token.valor) = n;
					end = TRUE;
				}
				break;

			case 4:
				if (isalpha(ch) || isdigit(ch) || (ch == '_')) {
					if (ofs < SIZE_IDENT) 
						s[ofs++] = ch;
					else
						err_scan(info, "Identifier too long");
				} else {
					int i;

					end = TRUE;

					retractscanner(info);
					s[ofs] = 0;

					//strupr(s);
					{ char *p = s;
						while(*p){
							*p = (char)toupper(*p);
							++p;
						}
					}

					for (i = 0; i < sizeof(reserved_string) / sizeof(reserved_string[0]); ++i) {
						if (!stricmp(s, reserved_string[i])) {
							info->token.tipo = i + TOK_RESERVED;
							break;
						}
					}

					if (i != sizeof(reserved_string) / sizeof(reserved_string[0])) 
						break;


					for (i = 0; i < sizeof(reserved_chem) / sizeof(reserved_chem[0]); ++i) {
						if (!stricmp(s, reserved_chem[i])) {
							info->token.tipo = i + TOK_CHEMRESERVED;
							break;
						}
					}

					if (i != sizeof(reserved_chem) / sizeof(reserved_string[0])) 
						break;

					info->token.valor = strdup(s);
					assert(info->token.valor);
				}
				break;

			case 5:				/* < */
				switch (ch) {
					case '=':
						info->token.tipo = TOK_LE;
						break;
					case '>':
						info->token.tipo = TOK_NE;
						break;
					default:
						retractscanner(info);
						info->token.tipo = TOK_LT;
						break;
				}
				end = TRUE;
				break;

			case 6:				/* > */
				switch (ch) {
					case '=':
						info->token.tipo = TOK_GE;
						break;
					default:
						retractscanner(info);
						info->token.tipo = TOK_GT;
						break;
				}
				end = TRUE;
				break;

			case 7:				/* / */
				switch (ch) {
					case '*':
						state = 8;
						break;
					default:
						retractscanner(info);
						info->token.tipo = TOK_DIV;
						end = TRUE;
						break;
				}
				break;

			case 8:				/* comment */
				switch (ch) {
					case '*':
						state = 9;
						break;
					case '\n':
						/* new line */
						++info->line;
						break;
				}
				break;

			case 9:				/* comment */
				switch (ch) {
					case '/':
						state = 0;
						break;
					case '\n':
						/* new line */
						++info->line;
						// fall-through
					default:
						state = 8;
						break;
				}
				break;

			default:
				err_scan(info, "Unknown state %i !", state);
				break;
		}
	}

#ifdef _DEBUG
	if (debugscanner) {
		printf("SCANNER: tipo = %s", tokenstring(info, info->token.tipo));
		switch (info->token.tipo) {
			case TOK_ID:
			case TOK_STRING:
				printf(" valor = [%08X] %s", info->token.valor, info->token.valor);
				break;
			case TOK_NUMERO:
				printf(" valor = [%08X] %i", info->token.valor, *((int *)info->token.valor));
				break;
		}
		printf("\n");
	}
#endif


	return(!eof);
}



BOOL scanner(struct struct_info *info)
{
	BOOL ret = scanner_int(info);

	if (!ret) {
		/* EOF */
		deinitscanner(info);
	} else if (info->token.tipo == RW_INCLUDE) {
		ret = scanner_int(info);

		if (info->includelevel < MAX_INCLUDE) {
			/* push include */
			ret = initscanner(info, info->token.valor);
		} else {
			logfile("Too many includes file %s line %i, %s\n", info->fname, info->line, info->token.valor);
			/* try to recover reading next token */
			ret = scanner(info);
		}
	}

	return(ret);
}



char *tokenstring(struct struct_info *info, int tipo) 
{
	if (tipo < TOK_RESERVED)
		return(tipo_tok[tipo]);
	else if (tipo < TOK_CHEMRESERVED)
		return(reserved_string[tipo - TOK_RESERVED]);
	else if (tipo < TOK_VARIABLE)
		return(reserved_chem[tipo - TOK_CHEMRESERVED]);
	else if (tipo < TOK_ECHOSTRING)
		return(info->variables[tipo - TOK_VARIABLE].name);
	else
		return(info->strings[tipo - TOK_STRING]);
}

