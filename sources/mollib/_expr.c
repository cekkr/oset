/* _expr.c
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


void ringsize_function(struct struct_info *info);
void add_function(struct struct_info *info);
void nH_function(struct struct_info *info);
void getbondorder_function(struct struct_info *info);


void simple_expression(struct struct_info *info);
void term(struct struct_info *info);
void factor(struct struct_info *info);

//NOTE: this list does not need to be in the same order as that in caoscomp.h
//or _scanner.c
int atomtype[] = {RW_METHYL,RW_PRIMARY,RW_SECONDARY,RW_TERTIARY,RW_QUATERNARY,
RW_VINYL,RW_CARBONYL,RW_ALKYNYL,RW_NITRILE,RW_ALLENE,RW_ALKYL,RW_SP3,RW_SP2,RW_SP,RW_PHENYL,
RW_NONCARBON,RW_HYDROXYL,
RW_PEROXIDE,RW_ALLYL,RW_BENZYL,RW_ALPHA_CARBONYL,RW_ALPHA_ALKYNYL,RW_ALPHA_NITRILE,
RW_HYDROGEN,RW_CARBON,RW_NITROGEN,RW_OXYGEN,RW_HALOGEN,RW_BROMINE,RW_CHLORINE,RW_IODINE,
RW_ALPHA_CH, RW_ALPHA_EWG, RW_ALPHA_EWG2, RW_ALPHA_EWG3, 0};


int	type;
//	enum {V_ANY, V_FUNCGRP, V_ATOM, V_BOOL, V_NUM} type;


int expression_type(struct struct_info *info)
{
	type = V_ANY;
	expression(info);
	return(type);
}



void expression(struct struct_info *info)
{
	int op;
	int lefttype;
	DEBUGPARSER("expression");

	simple_expression(info);
	if (tokenisinlist(info, RW_IS, RW_ISNOT, RW_ISEQ, TOK_IGUAL, TOK_LT, TOK_LE, TOK_NE, TOK_GE, TOK_GT, 0)) {
		op = info->token.tipo;
		matchtype(info, type, V_ATOM, op, RW_IS, RW_ISNOT, RW_ISEQ, 0);
		matchtype(info, type, V_NUM, op, TOK_LT, TOK_LE, TOK_GE, TOK_GT, 0);
		if (op == TOK_IGUAL)
			lefttype = type;
		scanner(info);

		if ((op == RW_IS) || (op == RW_ISNOT)) {
			if (tokenisinarray(info, atomtype)) {
				generate_code(info, info->token.tipo);
				scanner(info);
			} else {
				err_parse(info, "Expected atom type");
			}
		} else if (op == RW_ISEQ) {
			simple_expression(info);
			matchtype(info, type, V_ATOM, op, RW_ISEQ, 0);
		} else {
			simple_expression(info);
			matchtype(info, type, V_NUM, op, TOK_LT, TOK_LE, TOK_GE, TOK_GT, 0);
			if ((op == TOK_IGUAL) && (lefttype != type))
				err_parse(info, "Type mismatch");
		}

		generate_code(info, op);
		type = V_BOOL;
	}
}



void simple_expression(struct struct_info *info)
{
	int op;

	DEBUGPARSER("simple_expression");

	term(info);

	while (tokenisinlist(info, RW_OR, TOK_ADD, TOK_SUB, 0)) {
		op = info->token.tipo;
		matchtype(info, type, V_BOOL, op, RW_OR, 0);
		matchtype(info, type, V_NUM, op, TOK_ADD, TOK_SUB, 0);
		scanner(info);
		term(info);
		matchtype(info, type, V_BOOL, op, RW_OR, 0);
		matchtype(info, type, V_NUM, op, TOK_ADD, TOK_SUB, 0);
		generate_code(info, op);
	}
}


void term(struct struct_info *info)
{
	int op;

	DEBUGPARSER("term");

	factor(info);

	while (tokenisinlist(info, RW_AND, TOK_MUL, TOK_DIV, 0)) {
		op = info->token.tipo;
		matchtype(info, type, V_BOOL, op, RW_AND, 0);
		matchtype(info, type, V_NUM, op, TOK_MUL, TOK_DIV, 0);
		scanner(info);
		factor(info);
		matchtype(info, type, V_BOOL, op, RW_AND, 0);
		matchtype(info, type, V_NUM, op, TOK_MUL, TOK_DIV, 0);
		generate_code(info, op);
	}
}

void factor(struct struct_info *info)
{
	DEBUGPARSER("factor");

	switch (info->token.tipo) {
		case TOK_NUMERO:
			generate_code(info, TOK_NUMERO);
			generate_code(info, *((int *)info->token.valor));
			type = V_NUM;
			scanner(info);
			break;
		case TOK_ID: {	/* variable? */
				int var;

				if ((var = getvariable(info, info->token.valor)) == -1)
					err_parse(info, "Undefined variable: %s", info->token.valor);
				type = info->variables[var].type;
				generate_code(info, createvariable(info, info->token.valor, V_ANY));
			}
			scanner(info);
			break;
		case TOK_PARENABRE:
			scanner(info);
			expression(info);
			matchandscan(info, TOK_PARENCIERRA);
			break;
		case RW_NOT:
			scanner(info);
			factor(info);
			generate_code(info, RW_NOT);

			matchtype(info, type, V_BOOL, RW_NOT, RW_NOT, 0);
			break;
		case RW_RINGSIZE:
			ringsize_function(info);
			type = V_NUM;
			break;
		case RW_NH:
			nH_function(info);
			type = V_NUM;
			break;
		case RW_ADD:
			add_function(info);
			type = V_ATOM;
			break;
		case RW_GETBONDORDER:
			getbondorder_function(info);
			type = V_NUM;
			break;
		default:
			err_parse(info, "Sintax error %s", (info->token.tipo < TOK_RESERVED) ? token_string[info->token.tipo] : reserved_string[info->token.tipo - TOK_RESERVED]);
			break;
	}
}



void ringsize_function(struct struct_info *info)
{
	int var;

	DEBUGPARSER("ringsize_function");
	scanner(info);
	matchandscan(info, TOK_PARENABRE);
	match(info, TOK_ID);

	if ((var = getvariabletype(info, info->token.valor)) == -1)
		err_parse(info, "Undefined variable: %s", info->token.valor);
	matchtype(info, var, V_ATOM, RW_RINGSIZE, RW_RINGSIZE, 0);
	generate_code(info, createvariable(info, info->token.valor, V_ANY));

	scanner(info);
	matchandscan(info, TOK_COMA);

	if ((var = getvariabletype(info, info->token.valor)) == -1)
		err_parse(info, "Undefined variable: %s", info->token.valor);
	matchtype(info, var, V_ATOM, RW_RINGSIZE, RW_RINGSIZE, 0);
	generate_code(info, createvariable(info, info->token.valor, V_ANY));

	generate_code(info, RW_RINGSIZE);

	scanner(info);
	matchandscan(info, TOK_PARENCIERRA);
}

void nH_function(struct struct_info *info)
{
	int var;

	DEBUGPARSER("nH_function");
	scanner(info);
	matchandscan(info, TOK_PARENABRE);
	match(info, TOK_ID);

	if ((var = getvariabletype(info, info->token.valor)) == -1)
		err_parse(info, "Undefined variable: %s", info->token.valor);
	matchtype(info, var, V_ATOM, RW_NH, RW_NH, 0);
	generate_code(info, createvariable(info, info->token.valor, V_ANY));
	generate_code(info, RW_NH);

	scanner(info);
	matchandscan(info, TOK_PARENCIERRA);
}

void getbondorder_function(struct struct_info *info)
{
	int var;

	DEBUGPARSER("getbondorder_function");
	scanner(info);
	matchandscan(info, TOK_PARENABRE);
	match(info, TOK_ID);

	if ((var = getvariabletype(info, info->token.valor)) == -1)
		err_parse(info, "Undefined variable: %s", info->token.valor);
	matchtype(info, var, V_ATOM, RW_GETBONDORDER, RW_GETBONDORDER, 0);
	generate_code(info, createvariable(info, info->token.valor, V_ANY));

	scanner(info);
	matchandscan(info, TOK_COMA);

	if ((var = getvariabletype(info, info->token.valor)) == -1)
		err_parse(info, "Undefined variable: %s", info->token.valor);
	matchtype(info, var, V_ATOM, RW_GETBONDORDER, RW_GETBONDORDER, 0);
	generate_code(info, createvariable(info, info->token.valor, V_ANY));
	generate_code(info, RW_GETBONDORDER);

	scanner(info);
	matchandscan(info, TOK_PARENCIERRA);
}

void add_function(struct struct_info *info)
{
	int var;

	DEBUGPARSER("add_function");
	scanner(info);
	matchandscan(info, TOK_PARENABRE);
	match(info, TOK_ID);

	if ((var = getvariabletype(info, info->token.valor)) == -1)
		err_parse(info, "Undefined variable: %s", info->token.valor);
	matchtype(info, var, V_ATOM, RW_ADD, RW_ADD, 0);
	generate_code(info, createvariable(info, info->token.valor, V_ANY));

	scanner(info);
	matchandscan(info, TOK_COMA);

	matchlist(info, "Expected atom", RW_HYDROGEN,RW_CARBON,RW_NITROGEN,RW_OXYGEN,RW_HALOGEN,RW_BROMINE,0);
	generate_code(info, info->token.tipo);

	generate_code(info, RW_ADD);

	scanner(info);
	matchandscan(info, TOK_PARENCIERRA);
}
