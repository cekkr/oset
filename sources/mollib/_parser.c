/* _parser.c
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
#include <stdarg.h>
#include <setjmp.h>

#ifdef _DEBUG
BOOL debugparser = FALSE;
#endif

void free_rxnaux(struct reaction *reaction);

void rxnblock(struct struct_info *info, struct reaction *reaction);
int * fglist(struct struct_info *info);
int * ringlist(struct struct_info *info);
void condlist(struct struct_info *info);
void conditionblock(struct struct_info *info);
void commentblock(struct struct_info *info, struct reaction *reaction);
void linkblock(struct struct_info *info, struct reaction *reaction);
void startblock(struct struct_info *info);
void statementlist(struct struct_info *info);
void statement(struct struct_info *info);
void foratom_statement(struct struct_info *info);
void if_statement(struct struct_info *info);
void add_statement(struct struct_info *info);
void breakbond_statement(struct struct_info *info);
void eliminate_statement(struct struct_info *info);
void makebond_statement(struct struct_info *info);
void done_statement(struct struct_info *info);
void assignment_statement(struct struct_info *info);
void echo_statement(struct struct_info *info);
void incbondorder_statement(struct struct_info *info);
void decbondorder_statement(struct struct_info *info);
void setbondorder_statement(struct struct_info *info);


jmp_buf mark;


void reaction(struct struct_info *info)
{
	BOOL end = FALSE;
	struct reaction *reaction;
	int stat;

	info->reaction = realloc(info->reaction, (info->numreact+1)*sizeof(struct reaction));
	assert(info->reaction);
	reaction = &info->reaction[info->numreact];
	memset(reaction, 0, sizeof(struct reaction));

	if ((stat = setjmp(mark)) == 0) {
		DEBUGPARSER("reaction");
		rxnblock(info, reaction);

		while (!end) {
			matchlist(info, "Expected section", RW_CONDITIONS, RW_COMMENTS, RW_LINK, RW_START, 0);
			switch (info->token.tipo) {
				case RW_CONDITIONS:
					conditionblock(info);
					break;
				case RW_COMMENTS:
					commentblock(info, reaction);
					break;
				case RW_LINK:
					linkblock(info, reaction);
					break;
				case RW_START:
					startblock(info);
					end = TRUE;
					break;
			}
		}

		matchlist(info, "Expected reaction", RW_RXN, TOK_EOF, 0);

		++info->numreact;
	} else {
		printf("Aborting reaction %s...\n", reaction->name ? reaction->name : "unknown");

		free_rxnaux(reaction);

		while (!tokenis(info, RW_RXN) && !tokenis(info, TOK_EOF))
			scanner(info);
	}
}





void free_rxnaux(struct reaction *reaction)
{
	if (reaction->name)
		free(reaction->name);
	if (reaction->g1)
		free(reaction->g1);
	if (reaction->g2)
		free(reaction->g2);
	if (reaction->gprec)
		free(reaction->gprec);
	if (reaction->cond)
		free(reaction->cond);
	if (reaction->comments)
		free(reaction->comments);
	if (reaction->mechanism)
		free(reaction->mechanism);
	if (reaction->link)
		free(reaction->link);
}

void rxnblock(struct struct_info *info, struct reaction *reaction)
{
	BOOL end = FALSE;

	DEBUGPARSER("rxnblock");
	match(info, RW_RXN);

	scanner(info);

	while (!end) {
		switch(info->token.tipo) {
			case RW_NAME:
				DEBUGPARSER("rxnblock name");
				scanner(info);
				match(info, TOK_STRING);
				if (reaction->name)
					err_parse(info, "Reaction already has a name");
				reaction->name = strdup(info->token.valor);
				scanner(info);
				break;

			case RW_TYPE: {
					BOOL done;

					DEBUGPARSER("rxnblock type");
					scanner(info);
					matchlist(info, "Expected reaction type", RW_GP1, RW_GP2, RW_FGI1, RW_FGI2, RW_FGA, RW_GP0, RW_RING, 0);
					reaction->type = info->token.tipo;
					scanner(info);
					do {
						done = FALSE;

						switch (info->token.tipo) {
							case RW_G1:
								scanner(info);
								reaction->g1 = fglist(info);
								break;
							case RW_G2:
								scanner(info);
								reaction->g2 = fglist(info);
								break;
							case RW_GPREC:
								scanner(info);
								reaction->gprec = fglist(info);
								break;
							case RW_RINGTYPE:
								scanner(info);
								reaction->gring = ringlist(info);
								break;
							default:
								done = TRUE;
								break;
						}
					} while (!done);


					switch (reaction->type) {
						case RW_GP0:
							if (reaction->g1)
								err_parse(info, "Reaction can't have G1");
							if (reaction->g2)
								err_parse(info, "Reaction can't have G2");
							if (reaction->gprec)
								err_parse(info, "Reaction can't have GPREC");
							break;
						case RW_GP1:
							if (!reaction->g1)
								err_parse(info, "Reaction needs G1");
							if (reaction->g2)
								err_parse(info, "Reaction can't have G2");
							if (reaction->gprec)
								err_parse(info, "Reaction can't have GPREC");
							break;
						case RW_GP2:
							if (!reaction->g1)
								err_parse(info, "Reaction needs G1");
							if (!reaction->g2)
								err_parse(info, "Reaction needs G2");
							if (reaction->gprec)
								err_parse(info, "Reaction can't have GPREC");
							break;
						case RW_FGI1:
							if (!reaction->g1)
								err_parse(info, "Reaction needs G1");
							if (reaction->g2)
								err_parse(info, "Reaction can't have G2");
							if (!reaction->gprec)
								err_parse(info, "Reaction needs GPREC");
							break;
						case RW_FGI2:
							if (!reaction->g1)
								err_parse(info, "Reaction needs G1");
							if (!reaction->g2)
								err_parse(info, "Reaction needs G2");
							if (!reaction->gprec)
								err_parse(info, "Reaction needs GPREC");
							break;
						case RW_FGA:
							if (reaction->g1)
								err_parse(info, "Reaction can't have G1");
							if (reaction->g2)
								err_parse(info, "Reaction can't have G2");
							if (!reaction->gprec)
								err_parse(info, "Reaction needs GPREC");
							break;
						case RW_RING:
							if (reaction->g1)
								err_parse(info, "Reaction can't have G1");
							if (reaction->g2)
								err_parse(info, "Reaction can't have G2");
							if (reaction->gprec)
								err_parse(info, "Reaction can't have GPREC");
							if (!reaction->gring)
								err_parse(info, "Reaction needs RINGTYPE");
							break;
					}
	/*
					match(info, RW_G1);
					scanner(info);
					reaction->g1 = fglist(info);
					if (tokenis(info, RW_G2)) {
						scanner(info);
						reaction->g2 = fglist(info);
					}
					if (tokenis(info, RW_GPREC)) {
						scanner(info);
						reaction->gprec = fglist(info);
					}
*/
				}
				break;

			case RW_PATH:
				DEBUGPARSER("rxnblock path");
				scanner(info);
				match(info, TOK_IGUAL);
				scanner(info);
				match(info, TOK_NUMERO);
				reaction->path = *((int *)info->token.valor);
				scanner(info);
				break;
				

			case TOK_ID:
				DEBUGPARSER("rxnblock id");
				matchid(info, "RATING");
				scanner(info);
				match(info, TOK_IGUAL);
				scanner(info);
				match(info, TOK_NUMERO);
				reaction->rating = *((int *)info->token.valor);
				scanner(info);
				break;

			default:
				end = TRUE;
				break;
		}
	}

	if (!reaction->name)
		err_parse(info,"Missing reaction name");
	if (!reaction->type)
		err_parse(info,"Missing reaction type");
}


int * fglist(struct struct_info *info)
{
	BOOL done = FALSE;
	int var;
	int size = 0;
	int *ret = NULL;

	DEBUGPARSER("fglist");
	do {
		match(info, TOK_ID);

		var = getvariable(info, info->token.valor);
		if ((var == -1) || (info->variables[var].type != V_FUNCGRP))
			err_parse(info, "Expected FUNCGRP");

		ret = realloc(ret, (size+1)*sizeof(int));
		assert(ret);
		ret[size] = var;
		++size;

		scanner(info);
		if (tokenis(info, TOK_COMA)) {
			scanner(info);
		} else
			done = TRUE;
	} while (!done);

	ret = realloc(ret, (size+1)*sizeof(int));
	assert(ret);
	ret[size] = -1;

	return(ret);
}


int * ringlist(struct struct_info *info)
{
	BOOL done = FALSE;
	int var;
	int size = 0;
	int *ret = NULL;

	DEBUGPARSER("ringlist");
	do {
		match(info, TOK_ID);

		var = getvariable(info, info->token.valor);
		if ((var == -1) || ((info->variables[var].type != V_RINGGRP) && (info->variables[var].type != V_MULTIRINGGRP)))
			err_parse(info, "Expected RINGGRP");

		ret = realloc(ret, (size+1)*sizeof(int));
		assert(ret);
		ret[size] = var;
		++size;

		scanner(info);
		if (tokenis(info, TOK_COMA)) {
			scanner(info);
		} else
			done = TRUE;
	} while (!done);

	ret = realloc(ret, (size+1)*sizeof(int));
	assert(ret);
	ret[size] = -1;

	return(ret);
}



void condlist(struct struct_info *info)
{
	BOOL done = FALSE;

	DEBUGPARSER("fglist");
	do {
		match(info, TOK_ID);
		scanner(info);
		if (tokenis(info, TOK_COMA)) {
			scanner(info);
		} else
			done = TRUE;
	} while (!done);
}

void conditionblock(struct struct_info *info)
{
	DEBUGPARSER("conditionblock");
	scanner(info);
	condlist(info);
}


void commentblock(struct struct_info *info, struct reaction *reaction)
{
	int len;

	DEBUGPARSER("commentblock");
	scanner(info);
	do {
		match(info, TOK_STRING);
		if (reaction->comments) {
			len = strlen(reaction->comments);

			reaction->comments = realloc(reaction->comments, len + strlen(info->token.valor) + 2);
			assert(reaction->comments);
			sprintf(reaction->comments+len, "%s", info->token.valor);
		} else {
			reaction->comments = malloc(strlen(info->token.valor) + 1);
			assert(reaction->comments);
			strcpy(reaction->comments, info->token.valor);
		}

		scanner(info);
	} while (tokenis(info, TOK_STRING));
}

void linkblock(struct struct_info *info, struct reaction *reaction)
{
	DEBUGPARSER("linkblock");
	scanner(info);

	if (reaction->link)
		err_parse(info, "Reaction already has a link");

	match(info, TOK_STRING);
	reaction->link = strdup(info->token.valor);
	scanner(info);
}

void startblock(struct struct_info *info)
{
	DEBUGPARSER("startblock");
	scanner(info);
	statementlist(info);
	generate_code(info,RW_RXN);
}

void statementlist(struct struct_info *info)
{
	DEBUGPARSER("statementlist");
	statement(info);

	while (tokenisinlist(info, RW_FORATOM, RW_IF, RW_ADD, RW_BREAKBOND, RW_ELIMINATE, RW_MAKEBOND, RW_DONE, RW_ECHO, RW_INCBONDORDER, RW_DECBONDORDER, RW_SETBONDORDER, TOK_ID, 0)) 
		statement(info);
}

void statement(struct struct_info *info)
{
	DEBUGPARSER("statement");
	matchlist(info, "Expected statement", RW_FORATOM, RW_IF, RW_ADD, RW_BREAKBOND, RW_ELIMINATE, RW_MAKEBOND, RW_DONE, RW_ECHO, RW_INCBONDORDER, RW_DECBONDORDER, RW_SETBONDORDER, TOK_ID, 0);

	switch (info->token.tipo) {
		case RW_FORATOM:
			foratom_statement(info);
			break;
		case RW_IF:
			if_statement(info);
			break;
		case RW_ADD:
			add_statement(info);
			break;
		case RW_BREAKBOND:
			breakbond_statement(info);
			break;
		case RW_ELIMINATE:
			eliminate_statement(info);
			break;
		case RW_MAKEBOND:
			makebond_statement(info);
			break;
		case RW_INCBONDORDER:
			incbondorder_statement(info);
			break;
		case RW_DECBONDORDER:
			decbondorder_statement(info);
			break;
		case RW_SETBONDORDER:
			setbondorder_statement(info);
			break;
		case RW_DONE:
			done_statement(info);
			break;
		case RW_ECHO:
			echo_statement(info);
			break;
		case TOK_ID:
			assignment_statement(info);
			break;
	}
}

void foratom_statement(struct struct_info *info)
{
	DEBUGPARSER("foratom_statement");
	generate_code(info, RW_FORATOM);

	scanner(info);
	matchandscan(info,TOK_PARENABRE);
	match(info,TOK_ID);
	generate_code(info, createvariable(info, info->token.valor, V_ATOM));

	scanner(info);
	matchandscan(info,RW_FROM);
	match(info,TOK_ID);
	generate_code(info, createvariable(info, info->token.valor, V_ATOM));

	scanner(info);
	matchandscan(info,TOK_PARENCIERRA);
	statementlist(info);
	matchandscan(info, RW_NEXT);
	generate_code(info, RW_NEXT);
}

void if_statement(struct struct_info *info)
{
	int type;

	DEBUGPARSER("if_statement");
	generate_code(info, RW_IF);

	scanner(info);
	matchandscan(info, TOK_PARENABRE);
	type = expression_type(info);
	matchtype(info, type, V_BOOL, RW_IF, RW_IF, 0);
	matchandscan(info, TOK_PARENCIERRA);
	matchandscan(info, RW_THEN);
	generate_code(info, RW_THEN);
	statementlist(info);

	if (tokenis(info, RW_ELSE)) {
		generate_code(info, RW_ELSE);
		scanner(info);
		statementlist(info);
	}

	matchandscan(info, RW_ENDIF);
	generate_code(info, RW_ENDIF);
}

void add_statement(struct struct_info *info)
{
	int var;

	DEBUGPARSER("add_statement");
	generate_code(info, RW_ADD);
	scanner(info);
	matchandscan(info, TOK_PARENABRE);
	match(info, TOK_ID);

	if ((var = getvariabletype(info, info->token.valor)) == -1)
		err_parse(info, "Undefined variable: %s", info->token.valor);
	matchtype(info, var, V_ATOM, RW_ADD, RW_ADD, 0);
	generate_code(info, createvariable(info, info->token.valor, V_ANY));

	scanner(info);
	matchandscan(info, TOK_COMA);

	matchlist(info, "Expected atom", RW_HYDROGEN,RW_CARBON,RW_NITROGEN,RW_OXYGEN,RW_HALOGEN,
		RW_BROMINE, RW_CHLORINE, RW_IODINE, 0);
	generate_code(info, info->token.tipo);

	scanner(info);
	matchandscan(info, TOK_PARENCIERRA);
}

void breakbond_statement(struct struct_info *info)
{
	int var;

	DEBUGPARSER("breakbond_statement");
	generate_code(info, RW_BREAKBOND);

	scanner(info);
	matchandscan(info, TOK_PARENABRE);
	match(info, TOK_ID);

	if ((var = getvariabletype(info, info->token.valor)) == -1)
		err_parse(info, "Undefined variable: %s", info->token.valor);
	matchtype(info, var, V_ATOM, RW_BREAKBOND, RW_BREAKBOND, 0);
	generate_code(info, createvariable(info, info->token.valor, V_ANY));

	scanner(info);
	matchandscan(info, TOK_COMA);
	match(info, TOK_ID);

	if ((var = getvariabletype(info, info->token.valor)) == -1)
		err_parse(info, "Undefined variable: %s", info->token.valor);
	matchtype(info, var, V_ATOM, RW_BREAKBOND, RW_BREAKBOND, 0);
	generate_code(info, createvariable(info, info->token.valor, V_ANY));

	scanner(info);
	matchandscan(info, TOK_PARENCIERRA);
}

void echo_statement(struct struct_info *info)
{
	DEBUGPARSER("echo_statement");
	generate_code(info, RW_ECHO);

	scanner(info);
	matchandscan(info, TOK_PARENABRE);
	match(info, TOK_STRING);	//string

	generate_code(info, createstring(info, info->token.valor));

	scanner(info);
	matchandscan(info, TOK_PARENCIERRA);
}

void eliminate_statement(struct struct_info *info)
{
	int var;

	DEBUGPARSER("eliminate_statement");
	generate_code(info, RW_ELIMINATE);
	scanner(info);
	matchandscan(info, TOK_PARENABRE);
	match(info, TOK_ID);

	if ((var = getvariabletype(info, info->token.valor)) == -1)
		err_parse(info, "Undefined variable: %s", info->token.valor);
	matchtype(info, var, V_ATOM, RW_ELIMINATE, RW_ELIMINATE, 0);
	generate_code(info, createvariable(info, info->token.valor, V_ANY));

	scanner(info);
	matchandscan(info, TOK_COMA);
	match(info, TOK_ID);

	matchlist(info, "Expected atom", RW_HYDROGEN,RW_CARBON,RW_NITROGEN,RW_OXYGEN,RW_HALOGEN,RW_BROMINE,0);
	generate_code(info, info->token.tipo);

	scanner(info);
	matchandscan(info, TOK_PARENCIERRA);
}

void makebond_statement(struct struct_info *info)
{
	int var;

	DEBUGPARSER("makebond_statement");
	generate_code(info, RW_MAKEBOND);
	scanner(info);
	matchandscan(info, TOK_PARENABRE);
	match(info, TOK_ID);

	if ((var = getvariabletype(info, info->token.valor)) == -1)
		err_parse(info, "Undefined variable: %s", info->token.valor);
	matchtype(info, var, V_ATOM, RW_MAKEBOND, RW_MAKEBOND, 0);
	generate_code(info, createvariable(info, info->token.valor, V_ANY));

	scanner(info);
	matchandscan(info, TOK_COMA);
	match(info, TOK_ID);

	if ((var = getvariabletype(info, info->token.valor)) == -1)
		err_parse(info, "Undefined variable: %s", info->token.valor);
	matchtype(info, var, V_ATOM, RW_MAKEBOND, RW_MAKEBOND, 0);
	generate_code(info, createvariable(info, info->token.valor, V_ANY));

	scanner(info);
	matchandscan(info, TOK_PARENCIERRA);
}


void incbondorder_statement(struct struct_info *info)
{
	int var;

	DEBUGPARSER("incbondorder_statement");
	generate_code(info, RW_INCBONDORDER);
	scanner(info);
	matchandscan(info, TOK_PARENABRE);
	match(info, TOK_ID);

	if ((var = getvariabletype(info, info->token.valor)) == -1)
		err_parse(info, "Undefined variable: %s", info->token.valor);
	matchtype(info, var, V_ATOM, RW_INCBONDORDER, RW_INCBONDORDER, 0);
	generate_code(info, createvariable(info, info->token.valor, V_ANY));

	scanner(info);
	matchandscan(info, TOK_COMA);
	match(info, TOK_ID);

	if ((var = getvariabletype(info, info->token.valor)) == -1)
		err_parse(info, "Undefined variable: %s", info->token.valor);
	matchtype(info, var, V_ATOM, RW_INCBONDORDER, RW_INCBONDORDER, 0);
	generate_code(info, createvariable(info, info->token.valor, V_ANY));

	scanner(info);
	matchandscan(info, TOK_PARENCIERRA);
}

void decbondorder_statement(struct struct_info *info)
{
	int var;

	DEBUGPARSER("decbondorder_statement");
	generate_code(info, RW_DECBONDORDER);
	scanner(info);
	matchandscan(info, TOK_PARENABRE);
	match(info, TOK_ID);

	if ((var = getvariabletype(info, info->token.valor)) == -1)
		err_parse(info, "Undefined variable: %s", info->token.valor);
	matchtype(info, var, V_ATOM, RW_DECBONDORDER, RW_DECBONDORDER, 0);
	generate_code(info, createvariable(info, info->token.valor, V_ANY));

	scanner(info);
	matchandscan(info, TOK_COMA);
	match(info, TOK_ID);

	if ((var = getvariabletype(info, info->token.valor)) == -1)
		err_parse(info, "Undefined variable: %s", info->token.valor);
	matchtype(info, var, V_ATOM, RW_DECBONDORDER, RW_DECBONDORDER, 0);
	generate_code(info, createvariable(info, info->token.valor, V_ANY));

	scanner(info);
	matchandscan(info, TOK_PARENCIERRA);
}


void setbondorder_statement(struct struct_info *info)
{
	int var;

	DEBUGPARSER("setbondorder_statement");
	generate_code(info, RW_SETBONDORDER);
	scanner(info);
	matchandscan(info, TOK_PARENABRE);
	match(info, TOK_ID);

	if ((var = getvariabletype(info, info->token.valor)) == -1)
		err_parse(info, "Undefined variable: %s", info->token.valor);
	matchtype(info, var, V_ATOM, RW_SETBONDORDER, RW_SETBONDORDER, 0);
	generate_code(info, createvariable(info, info->token.valor, V_ANY));

	scanner(info);
	matchandscan(info, TOK_COMA);
	match(info, TOK_ID);

	if ((var = getvariabletype(info, info->token.valor)) == -1)
		err_parse(info, "Undefined variable: %s", info->token.valor);
	matchtype(info, var, V_ATOM, RW_SETBONDORDER, RW_SETBONDORDER, 0);
	generate_code(info, createvariable(info, info->token.valor, V_ANY));

	scanner(info);
	matchandscan(info, TOK_COMA);

	matchlist(info, "Expected number", TOK_ID, TOK_NUMERO, 0);
	if (tokenis(info, TOK_ID)) {
		if ((var = getvariabletype(info, info->token.valor)) == -1)
			err_parse(info, "Undefined variable: %s", info->token.valor);
		matchtype(info, var, V_NUM, RW_SETBONDORDER, RW_SETBONDORDER, 0);
		generate_code(info, createvariable(info, info->token.valor, V_ANY));
	} else if (tokenis(info, TOK_NUMERO)) {
		generate_code(info, TOK_NUMERO);
		generate_code(info, *(int *)info->token.valor);
	}

	scanner(info);
	matchandscan(info, TOK_PARENCIERRA);
}



void done_statement(struct struct_info *info)
{
	DEBUGPARSER("done_statement");
	generate_code(info, RW_DONE);
	scanner(info);

	if (info->token.tipo == TOK_PARENABRE) {
		int var;

		generate_code(info, TOK_PARENABRE);

		do {
			scanner(info);

			match(info, TOK_ID);

			if ((var = getvariabletype(info, info->token.valor)) == -1)
				err_parse(info, "Undefined variable: %s", info->token.valor);
			matchtype(info, var, V_ATOM, RW_DONE, RW_DONE, 0);
			generate_code(info, createvariable(info, info->token.valor, V_ANY));

			scanner(info);
		} while (info->token.tipo == TOK_COMA);

		matchandscan(info, TOK_PARENCIERRA);
		generate_code(info, TOK_PARENCIERRA);
	}
}

void assignment_statement(struct struct_info *info)
{
	char vname[100];
	int type;

	DEBUGPARSER("assignment_statement");
	generate_code(info, TOK_IGUAL);

	strcpy(vname, info->token.valor);
	scanner(info);
	matchandscan(info, TOK_IGUAL);
	type = expression_type(info);
	generate_code(info, TOK_IGUAL);

	generate_code(info, createvariable(info, vname, type));
}




BOOL	tokenis(struct struct_info *info, int tipo)
{
	return(info->token.tipo == tipo);
}

void	match(struct struct_info *info, int tipo)
{
	if (info->token.tipo != tipo)
		err_parse(info, "Expected %s", (tipo < TOK_RESERVED) ? token_string[tipo] : reserved_string[tipo - TOK_RESERVED]);
}

void	matchtype(struct struct_info *info, int type, int expected, int op, ...)
{
	va_list marker;
	BOOL hasmatch = FALSE;
	int n;

	va_start(marker, op);
	n = va_arg(marker, int);

	while (n && !hasmatch) {
		hasmatch = op == n;
		n = va_arg(marker, int);
	}
	va_end(marker);

	if (hasmatch && (type != expected))
		err_parse(info, "Type mismatch %s", (op < TOK_RESERVED) ? token_string[op] : reserved_string[op - TOK_RESERVED]);
}


BOOL	tokenisinlist(struct struct_info *info, int n, ...)
{
	va_list marker;
	BOOL hasmatch = FALSE;

	va_start(marker, n);
	while (n && !hasmatch) {
		hasmatch = info->token.tipo == n;
		n = va_arg(marker, int);
	}
	va_end(marker);

	return(hasmatch);
}


void	matchlist(struct struct_info *info, char *errmsg, int n, ...)
{
	va_list marker;
	BOOL hasmatch = FALSE;

	va_start(marker, n);
	while (n && !hasmatch) {
		hasmatch = info->token.tipo == n;
		n = va_arg(marker, int);
	}
	va_end(marker);

	if (!hasmatch) 
		err_parse(info, errmsg);
}

void	matchid(struct struct_info *info, char *name)
{
	if (!((info->token.tipo == TOK_ID) && info->token.valor && !strcmp(name, info->token.valor))) 
		err_parse(info, "Expected %s", name);
}


void	matchandscan(struct struct_info *info, int tipo)
{
	match(info, tipo);
	scanner(info);
}

BOOL	tokenisinarray(struct struct_info *info, int *n)
{
	BOOL hasmatch = FALSE;

	while (*n && !hasmatch) {
		hasmatch = info->token.tipo == *n;
		++n;
	}
	return(hasmatch);
}

