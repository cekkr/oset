/* caoscomp.h
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

//caoscomp.h

#ifndef _CAOSCOMP
#define	_CAOSCOMP


#define	BOOL	int
#define TRUE	1
#define FALSE	0

#define BUF_SIZE 1024

#define SIZE_IDENT 20



struct token {
	int     tipo;
	void    *valor;
};


struct	reaction {
	char *name;
	int	rating;
	int type;
	int *g1, *g2, *gprec, *gring;
	int *cond;
	char *comments;
	int *mechanism;   // the "compiled" code for the reaction
	int mechsize, mechalloc;
	int ip;		// instruction pointer
	int path;
	char *link;
};


struct variable {
	enum {V_ANY, V_FUNCGRP, V_ATOM, V_BOOL, V_NUM, V_RINGGRP, V_MULTIRINGGRP} type;
	char *name;
	union {
		struct {	/* V_FUNCGRP */
			struct	funcgrpnode *funcgrpnode;
			int		complex;
		}  fg;
		struct {	/* V_RINGGRP */
			struct  ringgrp *ringgrp;
			int		complex;
		} rg;
		struct {	/* V_MULTIRINGGRP */
			struct multiringgrp *multiringgrp;
			int		complex;
		} mrg;
		int		numatom;	/* V_ATOM */
		BOOL	truth;		/* V_BOOL */
		int		num;		/* V_NUM */
	} data;
};


/*struct smiledb {
	char *smile;
	char *name;
};
  */

struct smilecas_index {
	long formula;
	long offset;
};

struct smilecas_entry {
	long formula;
	char *smiles;
	char *name;
	char *cas;
};

#define	MAX_INCLUDE	2

struct include_stack {
		FILE*					f;
		char					fname[80];
		long					pos;
		int						line;
};

struct struct_info {
	FILE*                   f;              // archivo de comandos
	char					fname[80];
	int                     line;

	struct include_stack	include_stack[MAX_INCLUDE];
	int						includelevel;

	int                     readoffs;
	struct  token           token;
	char                    readbuff[BUF_SIZE];
	struct reaction			*reaction;
	int						numreact;
	struct variable			*variables;
	int						numvariables;
	char*					*strings;
	int						numstrings;
	char					**smiledb;		/* smile \0 name \0 */
	int						numsmiledb;
	struct smilecas_index	*smilecas_index;
	int						numsmilecas;
	struct struct_log		*echo;
	struct cplx_params		*cplx_params;
};




#define TOK_DUMMY		0
#define	TOK_EOF			1
#define	TOK_NUMERO		2
#define	TOK_STRING		3
#define	TOK_ID			4
#define	TOK_MUL			5
#define	TOK_ADD			6
#define	TOK_SUB			7
#define	TOK_DIV			8
#define	TOK_COMA		9
#define	TOK_PARENABRE	10
#define	TOK_PARENCIERRA	11
#define	TOK_IGUAL		12
#define	TOK_LT			13	/* less than */
#define TOK_LE			14  /* less equal */
#define TOK_NE			15  /* not equal */
#define	TOK_GE			16  /* greater equal */
#define TOK_GT			17  /* greater than */
#define TOK_OPENBRACK	18
#define TOK_CLOSEBRACK	19

#define	TOK_RESERVED		100
#define	TOK_CHEMRESERVED	1000
#define	TOK_VARIABLE		2000
#define TOK_ECHOSTRING		3000

typedef enum {
RW_RXN = TOK_RESERVED,
RW_NAME,
RW_TYPE,
RW_GP1,
RW_GP2,
RW_G1,
RW_G2,
RW_CONDITIONS,
RW_COMMENTS,
RW_START,
RW_FORATOM,
RW_FORBOND,
RW_FROM,
RW_NEXT,
RW_IF,
RW_THEN,
RW_ELSE,
RW_IS,
RW_ISNOT,
RW_AND,
RW_OR,
RW_NOT,
RW_ENDIF,
RW_BREAKBOND,
RW_MAKEBOND,
RW_ADD,
RW_ELIMINATE,
RW_DONE,
RW_PATH,
RW_LINK,
RW_RINGSIZE,
RW_FGI1,
RW_FGI2,
RW_GPREC,
RW_FGA,
RW_GP0,
RW_NH,
RW_ECHO,
RW_RING,
RW_RINGTYPE,
RW_INCLUDE,
RW_INCBONDORDER,
RW_DECBONDORDER,
RW_GETBONDORDER,
RW_SETBONDORDER,
RW_ISEQ
} RW_ENUM00;

/* to add a new statement type, e.g. ECHO, add in header, scanner,
	parser->statementlist, parser->statement, execute->statementlist, execute->statement */


/* NOTE: this list must be in the same order as the string array in _scanner.c
   AND the string array in _clas2.c. Also, chemical elements on this list should appear
   in parser.c:add_statement()
   All of the types on this list must also appear on the list in _expr.c   */
typedef enum {
RW_METHYL = TOK_CHEMRESERVED,
RW_PRIMARY,
RW_SECONDARY,
RW_TERTIARY,
RW_QUATERNARY,
RW_VINYL,
RW_CARBONYL,
RW_ALKYNYL,
RW_NITRILE,
RW_ALLENE,
RW_ALKYL,
RW_SP3,
RW_SP2,
RW_SP,
RW_PHENYL,
RW_NONCARBON,
RW_HYDROXYL,
RW_PEROXIDE,
RW_ALLYL,
RW_BENZYL,
RW_ALPHA_CARBONYL,
RW_ALPHA_ALKYNYL,
RW_ALPHA_NITRILE,
RW_HYDROGEN,
RW_CARBON,
RW_NITROGEN,
RW_OXYGEN,
RW_HALOGEN,
RW_BROMINE,
RW_CHLORINE,
RW_IODINE,
RW_ALPHA_CH,
RW_EWG,
RW_ALPHA_EWG,
RW_ALPHA_EWG2,
RW_ALPHA_EWG3
} RW_CHEMRESERVED00;

//	_error.c
void err_msg(char *s, ...);
void err_scan(struct struct_info *info, char *s, ...);
void err_parse(struct struct_info *info, char *s, ...);
void err_exec(struct struct_info *info, char *s, ...);
void seterr(char *s, ...);
char *geterr(void);


// _scanner.c
extern char *token_string[];
extern char *reserved_string[];

BOOL initscanner(struct struct_info *info, char *fname);
void deinitscanner(struct struct_info *info);
BOOL scanner(struct struct_info *info);
void	match(struct struct_info *info, int tipo);
char *tokenstring(struct struct_info *info, int tipo);

//	_parser.c
#ifdef _DEBUG
#define DEBUGPARSER(x)	if (debugparser) printf("PARSER:  "x"\n")
#else
#define	DEBUGPARSER(x)	
#endif 

extern	BOOL debugparser;

void	reaction(struct struct_info *info);
BOOL	tokenis(struct struct_info *info, int tipo);
void	matchlist(struct struct_info *info, char *errmsg, int n, ...);
void	matchid(struct struct_info *info, char *name);
BOOL	tokenisinlist(struct struct_info *info, int n, ...);
void	matchandscan(struct struct_info *info, int tipo);
BOOL	tokenisinarray(struct struct_info *info, int *n);
void	matchtype(struct struct_info *info, int type, int expected, int op, ...);

void add_statement(struct struct_info *info);


//	_expr.c
int expression_type(struct struct_info *info);
void expression(struct struct_info *info);

//	_gpofunc.c
void	readfuncgrp(struct struct_info *info);
void	readringgrp(struct struct_info *info);

//	_smdb.c
void readsmiledb(struct struct_info *info);
void sortsmiledb(struct struct_info *info);
char *findsmiledb(struct struct_info *info, char *smile);


//	_sym.c
int getvariable(struct struct_info *info, char *name);
int getvariabletype(struct struct_info *info, char *name);
void *getvariabledata(struct struct_info *info, char *name);
int	createvariable(struct struct_info *info, char *name, int type);
int createstring(struct struct_info *info, char *text);

//	_generat.c
void	generate_code(struct struct_info *info, int code);


// _logfile.c
void logfile(char *s, ...);

//	_try
int _dotry(void);
void _dothrow(int ret);


#endif



