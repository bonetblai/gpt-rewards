/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     INTTOK = 1,
     FLOATTOK = 2,
     COLONTOK = 3,
     MINUSTOK = 4,
     PLUSTOK = 5,
     STRINGTOK = 6,
     ASTERICKTOK = 7,
     DISCOUNTTOK = 8,
     VALUESTOK = 9,
     STATETOK = 10,
     ACTIONTOK = 11,
     OBSTOK = 12,
     TTOK = 13,
     OTOK = 14,
     RTOK = 15,
     UNIFORMTOK = 16,
     IDENTITYTOK = 17,
     REWARDTOK = 18,
     COSTTOK = 19,
     RESETTOK = 20,
     STARTTOK = 21,
     INCLUDETOK = 22,
     EXCLUDETOK = 23,
     GOALTOK = 24,
     EOFTOK = 258
   };
#endif
/* Tokens.  */
#define INTTOK 1
#define FLOATTOK 2
#define COLONTOK 3
#define MINUSTOK 4
#define PLUSTOK 5
#define STRINGTOK 6
#define ASTERICKTOK 7
#define DISCOUNTTOK 8
#define VALUESTOK 9
#define STATETOK 10
#define ACTIONTOK 11
#define OBSTOK 12
#define TTOK 13
#define OTOK 14
#define RTOK 15
#define UNIFORMTOK 16
#define IDENTITYTOK 17
#define REWARDTOK 18
#define COSTTOK 19
#define RESETTOK 20
#define STARTTOK 21
#define INCLUDETOK 22
#define EXCLUDETOK 23
#define GOALTOK 24
#define EOFTOK 258




/* Copy the first part of user declarations.  */
#line 1 "parser.y"

/* parser.y

 * Copyright 1996,1997, Brown University, Providence, RI.
 * 
 *                         All Rights Reserved
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose other than its incorporation into a
 * commercial product is hereby granted without fee, provided that the
 * above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Brown University not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.
 * 
 * BROWN UNIVERSITY DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR ANY
 * PARTICULAR PURPOSE.  IN NO EVENT SHALL BROWN UNIVERSITY BE LIABLE FOR
 * ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
#include <stdio.h>
#include <stdlib.h>
#include "parse_err.h"
#include "mdp.h"
#include "parse_hash.h"
#include "parse_constant.h"
#include "sparse-matrix.h"
#include "imm-reward.h"

#define YACCtrace(X)       /*   printf(X);fflush(stdout)    */ 

/* When reading in matrices we need to know what type we are reading
   and also we need to keep track of where in the matrix we are
   including how to update the row and col after each entry is read. */
typedef enum { mc_none, mc_trans_single, mc_trans_row, mc_trans_all,
               mc_obs_single, mc_obs_row, mc_obs_all,
               mc_reward_single, mc_reward_row, 
               mc_reward_all, mc_reward_mdp_only,
               mc_start_belief, mc_mdp_start, 
               mc_start_include, mc_start_exclude } Matrix_Context;

#ifdef __cplusplus
int yyparse();
extern int yylex();
extern void yyerror(char *string);
#else
extern int yylex();
extern void yyerror(char *string);
#endif

/* Forward declaration for action routines which appear at end of file */
void checkMatrix();
void enterString( Constant_Block *block );
void enterUniformMatrix( );
void enterIdentityMatrix( );
void enterResetMatrix( );
void enterMatrix( double value );
void setMatrixContext( Matrix_Context context, 
                      int a, int i, int j, int obs );
void enterStartState( int i );
void setStartStateUniform();
void endStartStates();
void verifyPreamble();
void checkProbs();

/*  Helps to give more meaningful error messages */
long currentLineNumber = 1;

/* This sets the context needed when names are given the the states, 
   actions and/or observations */
Mnemonic_Type curMnemonic = nt_unknown;

Matrix_Context curMatrixContext = mc_none;

/* These variable are used to keep track what type of matrix is being entered and
   which element is currently being processed.  They are initialized by the
   setMatrixContext() routine and updated by the enterMatrix() routine. */
int curRow;
int curCol;
int minA, maxA;
int minI, maxI;
int minJ, maxJ;
int minObs, maxObs;

/*  These variables will keep the intermediate representation for the
    matrices.  We cannot know how to set up the sparse matrices until
    all entries are read in, so we must have this intermediate 
    representation, which will will convert when it has all been read in.
    We allocate this memory once we know how big they must be and we
    will free all of this when we convert it to its final sparse format.
    */
//I_Matrix *IP;   /* For transition matrices. */
//I_Matrix *IR;   /* For observation matrices. */
//I_Matrix **IW;  /* For reward matrices */

/* These variables are used by the parser only, to keep some state
   information. 
*/
/* These are set when the appropriate preamble line is encountered.  This will
   allow us to check to make sure each is specified.  If observations are not
   defined then we will assume it is a regular MDP, and otherwise assume it 
   is a POMDP
   */
int discountDefined = 0;
int valuesDefined = 0;
int statesDefined = 0;
int actionsDefined = 0;
int observationsDefined = 0;
int goalsDefined = 0;

/* We only want to check when observation probs. are specified, but
   there was no observations in preamble. */
int observationSpecDefined = 0;

/* When we encounter a matrix with too many entries.  We would like
   to only generate one error message, instead of one for each entry.
   This variable is cleared at the start of reading  a matrix and
   set when there are too many entries. */
int gTooManyEntries = 0;

/* These will hold the goal states. */
int gGoalList[100];
int gNumGoals = 0;



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 140 "parser.y"
{
  Constant_Block *constBlk;
  int i_num;
  double f_num;
}
/* Line 187 of yacc.c.  */
#line 286 "y.tab.c"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 299 "y.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   122

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  28
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  56
/* YYNRULES -- Number of rules.  */
#define YYNRULES  100
/* YYNRULES -- Number of states.  */
#define YYNSTATES  142

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   259

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,    27,     2
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     4,     5,    11,    14,    15,    17,    19,
      21,    23,    25,    27,    31,    35,    37,    39,    40,    45,
      47,    49,    50,    55,    57,    59,    60,    65,    67,    69,
      70,    75,    77,    79,    80,    85,    89,    90,    96,    97,
     103,   104,   107,   109,   112,   113,   115,   117,   119,   123,
     124,   132,   133,   139,   140,   144,   148,   149,   157,   158,
     164,   165,   169,   173,   174,   184,   185,   193,   194,   200,
     201,   205,   207,   209,   211,   213,   215,   217,   220,   222,
     225,   227,   229,   231,   233,   235,   237,   239,   241,   243,
     245,   248,   250,   253,   255,   257,   259,   262,   265,   267,
     269
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      29,     0,    -1,    -1,    -1,    32,    30,    49,    31,    54,
      -1,    32,    33,    -1,    -1,    34,    -1,    35,    -1,    37,
      -1,    40,    -1,    43,    -1,    46,    -1,    10,     5,    82,
      -1,    11,     5,    36,    -1,    20,    -1,    21,    -1,    -1,
      12,     5,    38,    39,    -1,     3,    -1,    79,    -1,    -1,
      13,     5,    41,    42,    -1,     3,    -1,    79,    -1,    -1,
      14,     5,    44,    45,    -1,     3,    -1,    79,    -1,    -1,
      26,     5,    47,    48,    -1,    80,    -1,    79,    -1,    -1,
      23,     5,    50,    73,    -1,    23,     5,     8,    -1,    -1,
      23,    24,     5,    51,    53,    -1,    -1,    23,    25,     5,
      52,    53,    -1,    -1,    53,    76,    -1,    76,    -1,    54,
      55,    -1,    -1,    56,    -1,    61,    -1,    66,    -1,    15,
       5,    57,    -1,    -1,    77,     5,    76,     5,    76,    58,
      81,    -1,    -1,    77,     5,    76,    59,    73,    -1,    -1,
      77,    60,    72,    -1,    16,     5,    62,    -1,    -1,    77,
       5,    76,     5,    78,    63,    81,    -1,    -1,    77,     5,
      76,    64,    73,    -1,    -1,    77,    65,    73,    -1,    17,
       5,    67,    -1,    -1,    77,     5,    76,     5,    76,     5,
      78,    68,    82,    -1,    -1,    77,     5,    76,     5,    76,
      69,    75,    -1,    -1,    77,     5,    76,    70,    75,    -1,
      -1,    77,    71,    75,    -1,    18,    -1,    19,    -1,    74,
      -1,    18,    -1,    22,    -1,    74,    -1,    74,    81,    -1,
      81,    -1,    75,    82,    -1,    82,    -1,     3,    -1,     8,
      -1,     9,    -1,     3,    -1,     8,    -1,     9,    -1,     3,
      -1,     8,    -1,     9,    -1,    79,     8,    -1,     8,    -1,
      80,     3,    -1,     3,    -1,     3,    -1,     4,    -1,    83,
       3,    -1,    83,     4,    -1,     7,    -1,     6,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   153,   153,   178,   152,   213,   217,   219,   220,   221,
     222,   223,   224,   226,   240,   246,   257,   263,   262,   281,
     300,   304,   303,   316,   335,   339,   338,   351,   370,   374,
     373,   386,   387,   391,   390,   407,   444,   443,   450,   449,
     457,   461,   465,   470,   471,   473,   474,   500,   502,   508,
     507,   514,   513,   518,   518,   523,   529,   528,   535,   534,
     539,   539,   544,   552,   551,   561,   560,   567,   566,   574,
     573,   580,   584,   588,   594,   598,   602,   607,   611,   616,
     620,   625,   637,   651,   656,   669,   683,   688,   700,   714,
     719,   727,   735,   740,   746,   755,   768,   776,   785,   789,
     794
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "INTTOK", "FLOATTOK", "COLONTOK",
  "MINUSTOK", "PLUSTOK", "STRINGTOK", "ASTERICKTOK", "DISCOUNTTOK",
  "VALUESTOK", "STATETOK", "ACTIONTOK", "OBSTOK", "TTOK", "OTOK", "RTOK",
  "UNIFORMTOK", "IDENTITYTOK", "REWARDTOK", "COSTTOK", "RESETTOK",
  "STARTTOK", "INCLUDETOK", "EXCLUDETOK", "GOALTOK", "EOFTOK", "$accept",
  "pomdp_file", "@1", "@2", "preamble", "param_type", "discount_param",
  "value_param", "value_tail", "state_param", "@3", "state_tail",
  "action_param", "@4", "action_tail", "obs_param", "@5", "obs_param_tail",
  "goal_param", "@6", "goal_param_tail", "start_state", "@7", "@8", "@9",
  "start_state_list", "param_list", "param_spec", "trans_prob_spec",
  "trans_spec_tail", "@10", "@11", "@12", "obs_prob_spec", "obs_spec_tail",
  "@13", "@14", "@15", "reward_spec", "reward_spec_tail", "@16", "@17",
  "@18", "@19", "ui_matrix", "u_matrix", "prob_matrix", "num_matrix",
  "state", "action", "obs", "ident_list", "goal_list", "prob", "number",
  "optional_sign", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   259,     1,     2,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,   258
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    28,    30,    31,    29,    32,    32,    33,    33,    33,
      33,    33,    33,    34,    35,    36,    36,    38,    37,    39,
      39,    41,    40,    42,    42,    44,    43,    45,    45,    47,
      46,    48,    48,    50,    49,    49,    51,    49,    52,    49,
      49,    53,    53,    54,    54,    55,    55,    55,    56,    58,
      57,    59,    57,    60,    57,    61,    63,    62,    64,    62,
      65,    62,    66,    68,    67,    69,    67,    70,    67,    71,
      67,    72,    72,    72,    73,    73,    73,    74,    74,    75,
      75,    76,    76,    76,    77,    77,    77,    78,    78,    78,
      79,    79,    80,    80,    81,    81,    82,    82,    83,    83,
      83
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     0,     5,     2,     0,     1,     1,     1,
       1,     1,     1,     3,     3,     1,     1,     0,     4,     1,
       1,     0,     4,     1,     1,     0,     4,     1,     1,     0,
       4,     1,     1,     0,     4,     3,     0,     5,     0,     5,
       0,     2,     1,     2,     0,     1,     1,     1,     3,     0,
       7,     0,     5,     0,     3,     3,     0,     7,     0,     5,
       0,     3,     3,     0,     9,     0,     7,     0,     5,     0,
       3,     1,     1,     1,     1,     1,     1,     2,     1,     2,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     1,     2,     1,     1,     1,     2,     2,     1,     1,
       0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       6,     0,     2,     1,     0,     0,     0,     0,     0,     0,
      40,     5,     7,     8,     9,    10,    11,    12,   100,     0,
      17,    21,    25,    29,     0,     3,    99,    98,    13,     0,
      15,    16,    14,     0,     0,     0,     0,    33,     0,     0,
      44,    96,    97,    19,    91,    18,    20,    23,    22,    24,
      27,    26,    28,    93,    30,    32,    31,    35,     0,    36,
      38,     4,    90,    92,    94,    95,    74,    75,    34,    76,
      78,     0,     0,     0,     0,     0,    43,    45,    46,    47,
      77,    81,    82,    83,    37,    42,    39,     0,     0,     0,
      41,    84,    85,    86,    48,    53,    55,    60,    62,    69,
       0,     0,     0,     0,     0,   100,    51,    71,    72,    54,
      73,    58,    61,    67,    70,    80,     0,     0,     0,     0,
       0,   100,    79,    49,    52,    87,    88,    89,    56,    59,
      65,    68,     0,     0,     0,   100,    50,    57,    63,    66,
     100,    64
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,    10,    40,     2,    11,    12,    13,    32,    14,
      33,    45,    15,    34,    48,    16,    35,    51,    17,    36,
      54,    25,    58,    71,    72,    84,    61,    76,    77,    94,
     132,   117,   101,    78,    96,   133,   119,   103,    79,    98,
     140,   135,   121,   105,   109,    68,    69,   114,    85,    95,
     128,    46,    56,    70,   115,    29
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -115
static const yytype_int8 yypact[] =
{
    -115,    10,     2,  -115,    28,    34,    58,    61,    71,    72,
      55,  -115,  -115,  -115,  -115,  -115,  -115,  -115,    30,    38,
    -115,  -115,  -115,  -115,     0,  -115,  -115,  -115,  -115,    67,
    -115,  -115,  -115,     3,    43,    44,    47,    73,    74,    75,
    -115,  -115,  -115,  -115,  -115,  -115,    76,  -115,  -115,    76,
    -115,  -115,    76,  -115,  -115,    76,    79,  -115,     5,  -115,
    -115,    45,  -115,  -115,  -115,  -115,  -115,  -115,  -115,    69,
    -115,    23,    23,    78,    80,    81,  -115,  -115,  -115,  -115,
    -115,  -115,  -115,  -115,    23,  -115,    23,    32,    32,    32,
    -115,  -115,  -115,  -115,  -115,    82,  -115,    83,  -115,    84,
      23,    26,    23,     5,    23,    30,    85,  -115,  -115,  -115,
      69,    86,  -115,    87,    50,  -115,    23,     5,    40,     5,
      23,    30,  -115,  -115,  -115,  -115,  -115,  -115,  -115,  -115,
      88,    50,    69,    69,    40,    30,  -115,  -115,  -115,    50,
      30,  -115
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
    -115,  -115,  -115,  -115,  -115,  -115,  -115,  -115,  -115,  -115,
    -115,  -115,  -115,  -115,  -115,  -115,  -115,  -115,  -115,  -115,
    -115,  -115,  -115,  -115,  -115,    22,  -115,  -115,  -115,  -115,
    -115,  -115,  -115,  -115,  -115,  -115,  -115,  -115,  -115,  -115,
    -115,  -115,  -115,  -115,  -115,  -100,    -6,  -114,   -82,   -14,
     -37,    33,  -115,   -68,   -18,  -115
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -101
static const yytype_int16 yytable[] =
{
      28,    80,    90,   112,    90,    37,    43,   131,    64,    65,
       3,    44,     4,     5,     6,     7,     8,   124,   106,   129,
     111,   139,   113,    66,    38,    39,    81,    67,     9,    64,
      65,    82,    83,    18,   123,    91,    26,    27,   130,    19,
      92,    93,    80,   125,   107,   108,    47,    50,   126,   127,
      53,    44,    44,  -100,  -100,    44,    26,    27,    30,    31,
      73,    74,    75,    20,   136,   137,    21,    49,    52,    55,
      41,    42,    64,    65,    97,    99,    22,    23,    24,    59,
      60,    57,    63,    87,    62,    88,    89,   100,   102,   104,
     116,   118,   120,   134,    86,   110,   122,   138,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   122,     0,     0,     0,     0,     0,     0,
       0,   122,   141
};

static const yytype_int16 yycheck[] =
{
      18,    69,    84,   103,    86,     5,     3,   121,     3,     4,
       0,     8,    10,    11,    12,    13,    14,   117,   100,   119,
     102,   135,   104,    18,    24,    25,     3,    22,    26,     3,
       4,     8,     9,     5,   116,     3,     6,     7,   120,     5,
       8,     9,   110,     3,    18,    19,     3,     3,     8,     9,
       3,     8,     8,     3,     4,     8,     6,     7,    20,    21,
      15,    16,    17,     5,   132,   133,     5,    34,    35,    36,
       3,     4,     3,     4,    88,    89,     5,     5,    23,     5,
       5,     8,     3,     5,     8,     5,     5,     5,     5,     5,
       5,     5,     5,     5,    72,   101,   114,   134,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   131,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   139,   140
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    29,    32,     0,    10,    11,    12,    13,    14,    26,
      30,    33,    34,    35,    37,    40,    43,    46,     5,     5,
       5,     5,     5,     5,    23,    49,     6,     7,    82,    83,
      20,    21,    36,    38,    41,    44,    47,     5,    24,    25,
      31,     3,     4,     3,     8,    39,    79,     3,    42,    79,
       3,    45,    79,     3,    48,    79,    80,     8,    50,     5,
       5,    54,     8,     3,     3,     4,    18,    22,    73,    74,
      81,    51,    52,    15,    16,    17,    55,    56,    61,    66,
      81,     3,     8,     9,    53,    76,    53,     5,     5,     5,
      76,     3,     8,     9,    57,    77,    62,    77,    67,    77,
       5,    60,     5,    65,     5,    71,    76,    18,    19,    72,
      74,    76,    73,    76,    75,    82,     5,    59,     5,    64,
       5,    70,    82,    76,    73,     3,     8,     9,    78,    73,
      76,    75,    58,    63,     5,    69,    81,    81,    78,    75,
      68,    82
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 153 "parser.y"
    { 
		    /* The preamble is a section of the file which */
		    /* must come first and whcih contains some global */
		    /* properties of the MDP that the file */
		    /* specifies. (e.g., number of states).  The */
		    /* observations are optional and its presence or */
		    /* absence is what first tells the parser whether */
		    /* it is parsing an MDP or a POMDP. */

		    verifyPreamble();  /* make sure all things are */
				       /* defined */

		    /* While we parse we use an intermediate */
		    /* representation which will be converted to the */
		    /* sparse representation when we are finished */
		    /* parsing.  After the preamble we are ready to */
		    /* start filling in values and we know how big the */
		    /* problem is, so we allocate the space for the */
		    /* intermediate forms */

		    allocateIntermediateMDP();  
                    initializeImmRewards();
		  }
    break;

  case 3:
#line 178 "parser.y"
    { 
		    /* Some type of algorithms want a place to start */
		    /* off the problem, especially when doing */
		    /* simulation type experiments.  This is an */
		    /* optional argument that allows specification of */
		    /* this.   In a POMDP this is a belief state, but */
		    /* in an MDP this is a single state.  If none is */
		    /* specified for a POMDP, then the uniform */
		    /* distribution over all states is used.  If none */
		    /* is specified for an MDP, then random states */
		    /* will be assumed. */

		    endStartStates(); 
		  }
    break;

  case 4:
#line 197 "parser.y"
    {
		    /* This is the very last thing we do while */
		    /* parsing.  Even though the file may conform to */
		    /* the syntax, the semantics of the problem */
		    /* specification requires probability */
		    /* distributions.  This routine will make sure */
		    /* that the appropriate things sum to 1.0 to make */
		    /* a valid probability distribution. This will */
		    /* also generate the error message when */
		    /* observation probabilities are specified in an */
		    /* MDP problem, since this is illegal. */

                     checkProbs();
		     YACCtrace("pomdp_file -> preamble params\n");
                  }
    break;

  case 5:
#line 214 "parser.y"
    {
		   YACCtrace("preamble -> preamble param_type\n");
		}
    break;

  case 13:
#line 227 "parser.y"
    {
		  /* The discount factor only makes sense when in the */
		  /* range 0 to 1, so it is an error to specify */
		  /* anything outside this range. */

                   gDiscount = (yyvsp[(3) - (3)].f_num);
                   if(( gDiscount < 0.0 ) || ( gDiscount > 1.0 ))
                      ERR_enter("Parser<ytab>:", currentLineNumber,
                                BAD_DISCOUNT_VAL, "");
                   discountDefined = 1;
		   YACCtrace("discount_param -> DISCOUNTTOK COLONTOK number\n");
	        }
    break;

  case 14:
#line 241 "parser.y"
    {
                   valuesDefined = 1;
		   YACCtrace("value_param -> VALUESTOK COLONTOK value_tail\n");
	        }
    break;

  case 15:
#line 254 "parser.y"
    {
                   gValueType = REWARD_value_type;
		}
    break;

  case 16:
#line 258 "parser.y"
    {
                   gValueType = COST_value_type;
		}
    break;

  case 17:
#line 263 "parser.y"
    { 
		  /* Since are able to enumerate the states and refer */
		  /* to them by identifiers, we will need to set the */
		  /* current state to indicate that we are parsing */
		  /* states.  This is important, since we will parse */
		  /* observatons and actions in exactly the same */
		  /* manner with the same code.  */
 
		  curMnemonic = nt_state; 

		}
    break;

  case 18:
#line 275 "parser.y"
    {
                   statesDefined = 1;
                   curMnemonic = nt_unknown;
		   YACCtrace("state_param -> STATETOK COLONTOK state_tail\n");
		}
    break;

  case 19:
#line 282 "parser.y"
    {

		  /*  For the number of states, we can just have a */
		  /*  number indicating how many there are, or ... */

                   gNumStates = (yyvsp[(1) - (1)].constBlk)->theValue.theInt;
                   if( gNumStates < 1 ) {
                      ERR_enter("Parser<ytab>:", currentLineNumber, 
                                BAD_NUM_STATES, "");
                      gNumStates = 1;
                   }

 		   /* Since we use some temporary storage to hold the
		      integer as we parse, we free the memory when we
		      are done with the value */

                   free( (yyvsp[(1) - (1)].constBlk) );
		}
    break;

  case 21:
#line 304 "parser.y"
    {
		  /* See state_param for explanation of this */

		  curMnemonic = nt_action;  
		}
    break;

  case 22:
#line 310 "parser.y"
    {
                   actionsDefined = 1;
                   curMnemonic = nt_unknown;
		   YACCtrace("action_param -> ACTIONTOK COLONTOK action_tail\n");
		}
    break;

  case 23:
#line 317 "parser.y"
    {

		  /*  For the number of actions, we can just have a */
		  /*  number indicating how many there are, or ... */

                   gNumActions = (yyvsp[(1) - (1)].constBlk)->theValue.theInt;
                   if( gNumActions < 1 ) {
                      ERR_enter("Parser<ytab>:", currentLineNumber, 
                                BAD_NUM_ACTIONS, "" );
                      gNumActions = 1;
                   }
		   
		   /* Since we use some temporary storage to hold the
		      integer as we parse, we free the memory when we
		      are done with the value */

                   free( (yyvsp[(1) - (1)].constBlk) );
		}
    break;

  case 25:
#line 339 "parser.y"
    { 
		  /* See state_param for explanation of this */

		  curMnemonic = nt_observation; 
		}
    break;

  case 26:
#line 345 "parser.y"
    {
                   observationsDefined = 1;
                   curMnemonic = nt_unknown;
		   YACCtrace("obs_param -> OBSTOK COLONTOK obs_param_tail\n");
		}
    break;

  case 27:
#line 352 "parser.y"
    {

		  /*  For the number of observation, we can just have a */
		  /*  number indicating how many there are, or ... */

                   gNumObservations = (yyvsp[(1) - (1)].constBlk)->theValue.theInt;
                   if( gNumObservations < 1 ) {
                      ERR_enter("Parser<ytab>:", currentLineNumber, 
                                BAD_NUM_OBS, "" );
                      gNumObservations = 1;
                   }

		   /* Since we use some temporary storage to hold the
		      integer as we parse, we free the memory when we
		      are done with the value */

                   free( (yyvsp[(1) - (1)].constBlk) );
		}
    break;

  case 29:
#line 374 "parser.y"
    { 
		  /* See state_param for explanation of this */
		  curMnemonic = nt_goal; 
		}
    break;

  case 30:
#line 379 "parser.y"
    {
                   goalsDefined = 1;
		   gGoalList[gNumGoals] = -1;
		   curMnemonic = nt_unknown;
		   YACCtrace("goal_param -> GOALTOK COLONTOK goal_param_tail\n");
		}
    break;

  case 33:
#line 391 "parser.y"
    { 
		  /* There are a number of different formats for the */
		  /* start state.  This one is valid for either a */
		  /* POMDP or an MDP.  With a POMDP it will expect a */
		  /* list of probabilities, one for each state, */
		  /* representing the initial belief state.  For an */
		  /* MDP there can be only a single integer */
		  /* representing the starting state. */

		  if( gProblemType == POMDP_problem_type )
		    setMatrixContext(mc_start_belief, 0, 0, 0, 0); 
		  else
		    setMatrixContext(mc_mdp_start, 0, 0, 0, 0); 
		}
    break;

  case 35:
#line 424 "parser.y"
    {
                   int num;

		   num = H_lookup( (yyvsp[(3) - (3)].constBlk)->theValue.theString, nt_state );
		   if(( num < 0 ) || (num >= gNumStates )) {
		     ERR_enter("Parser<ytab>:", currentLineNumber, 
				 BAD_STATE_STR, (yyvsp[(3) - (3)].constBlk)->theValue.theString );
                   }
                   else
		     if( gProblemType == MDP_problem_type )
		       gInitialState = num;
		     else
		       gInitialBelief[num] = 1.0;


                   free( (yyvsp[(3) - (3)].constBlk)->theValue.theString );
                   free( (yyvsp[(3) - (3)].constBlk) );
                }
    break;

  case 36:
#line 444 "parser.y"
    { 
		  setMatrixContext(mc_start_include, 0, 0, 0, 0); 
		}
    break;

  case 38:
#line 450 "parser.y"
    { 
		  setMatrixContext(mc_start_exclude, 0, 0, 0, 0); 
		}
    break;

  case 40:
#line 457 "parser.y"
    { 
		  setStartStateUniform(); 
		}
    break;

  case 41:
#line 462 "parser.y"
    {
		  enterStartState( (yyvsp[(2) - (2)].i_num) );
                }
    break;

  case 42:
#line 466 "parser.y"
    {
		  enterStartState( (yyvsp[(1) - (1)].i_num) );
                }
    break;

  case 46:
#line 475 "parser.y"
    {
		    /* If there are observation specifications defined,
		       but no observations listed in the preamble, then
		       this is an error, since regular MDPs don't have
		       the concept of observations.  However, it could 
		       be a POMDP that was just missing the preamble 
		       part.  The way we handle this is to go ahead 
		       and parse the observation specifications, but
		       always check before we actually enter values in
		       a matrix (see the enterMatrix() routine.)  This
		       way we can determine if there are any problems 
		       with the observation specifications.  We cannot
		       add entries to the matrices since there will be
		       no memory allocated for it.  We want to
		       generate an error for this case, but don't want
		       a separate error for each observation
		       specification, so we define a variable that is
		       just a flag for whether or not any observation
		       specificiations have been defined.  After we
		       are all done parsing we will check this flag
		       and generate an error if needed.
		       */

		      observationSpecDefined = 1;
		  }
    break;

  case 48:
#line 503 "parser.y"
    {
		   YACCtrace("trans_prob_spec -> TTOK COLONTOK trans_spec_tail\n");
		}
    break;

  case 49:
#line 508 "parser.y"
    { setMatrixContext(mc_trans_single, (yyvsp[(1) - (5)].i_num), (yyvsp[(3) - (5)].i_num), (yyvsp[(5) - (5)].i_num), 0); }
    break;

  case 50:
#line 509 "parser.y"
    {
                   enterMatrix( (yyvsp[(7) - (7)].f_num) );
		   YACCtrace("trans_spec_tail -> action COLONTOK state COLONTOK state prob \n");
		}
    break;

  case 51:
#line 514 "parser.y"
    { setMatrixContext(mc_trans_row, (yyvsp[(1) - (3)].i_num), (yyvsp[(3) - (3)].i_num), 0, 0); }
    break;

  case 52:
#line 515 "parser.y"
    {
		   YACCtrace("trans_spec_tail -> action COLONTOK state ui_matrix \n");
		}
    break;

  case 53:
#line 518 "parser.y"
    { setMatrixContext(mc_trans_all, (yyvsp[(1) - (1)].i_num), 0, 0, 0); }
    break;

  case 54:
#line 519 "parser.y"
    {
		   YACCtrace("trans_spec_tail -> action ui_matrix\n");
		}
    break;

  case 55:
#line 524 "parser.y"
    {
		   YACCtrace("obs_prob_spec -> OTOK COLONTOK  obs_spec_tail\n");
		}
    break;

  case 56:
#line 529 "parser.y"
    { setMatrixContext(mc_obs_single, (yyvsp[(1) - (5)].i_num), 0, (yyvsp[(3) - (5)].i_num), (yyvsp[(5) - (5)].i_num)); }
    break;

  case 57:
#line 530 "parser.y"
    {
                   enterMatrix( (yyvsp[(7) - (7)].f_num) );
		   YACCtrace("obs_spec_tail -> action COLONTOK state COLONTOK obs prob \n");
		}
    break;

  case 58:
#line 535 "parser.y"
    { setMatrixContext(mc_obs_row, (yyvsp[(1) - (3)].i_num), 0, (yyvsp[(3) - (3)].i_num), 0); }
    break;

  case 59:
#line 536 "parser.y"
    {
		   YACCtrace("obs_spec_tail -> action COLONTOK state COLONTOK u_matrix\n");
		}
    break;

  case 60:
#line 539 "parser.y"
    { setMatrixContext(mc_obs_all, (yyvsp[(1) - (1)].i_num), 0, 0, 0); }
    break;

  case 61:
#line 540 "parser.y"
    {
		   YACCtrace("obs_spec_tail -> action u_matrix\n");
		}
    break;

  case 62:
#line 545 "parser.y"
    {
		   YACCtrace("reward_spec -> RTOK COLONTOK  reward_spec_tail\n");
		}
    break;

  case 63:
#line 552 "parser.y"
    { setMatrixContext(mc_reward_single, (yyvsp[(1) - (7)].i_num), (yyvsp[(3) - (7)].i_num), (yyvsp[(5) - (7)].i_num), (yyvsp[(7) - (7)].i_num)); }
    break;

  case 64:
#line 553 "parser.y"
    {
                   enterMatrix( (yyvsp[(9) - (9)].f_num) );

		   /* Only need this for the call to doneImmReward */
		   checkMatrix();  
		   YACCtrace("reward_spec_tail -> action COLONTOK state COLONTOK state COLONTOK obs number\n");
		}
    break;

  case 65:
#line 561 "parser.y"
    { setMatrixContext(mc_reward_row, (yyvsp[(1) - (5)].i_num), (yyvsp[(3) - (5)].i_num), (yyvsp[(5) - (5)].i_num), 0); }
    break;

  case 66:
#line 562 "parser.y"
    {
                   checkMatrix();
		   YACCtrace("reward_spec_tail -> action COLONTOK state COLONTOK state num_matrix\n");
		 }
    break;

  case 67:
#line 567 "parser.y"
    { setMatrixContext(mc_reward_all, (yyvsp[(1) - (3)].i_num), (yyvsp[(3) - (3)].i_num), 0, 0); }
    break;

  case 68:
#line 568 "parser.y"
    {
                   checkMatrix();
		   YACCtrace("reward_spec_tail -> action COLONTOK state num_matrix\n");
		}
    break;

  case 69:
#line 574 "parser.y"
    { setMatrixContext(mc_reward_mdp_only, (yyvsp[(1) - (1)].i_num), 0, 0, 0); }
    break;

  case 70:
#line 575 "parser.y"
    {
                   checkMatrix();
		   YACCtrace("reward_spec_tail -> action num_matrix\n");
                }
    break;

  case 71:
#line 581 "parser.y"
    {
                   enterUniformMatrix();
                }
    break;

  case 72:
#line 585 "parser.y"
    {
                   enterIdentityMatrix();
                }
    break;

  case 73:
#line 589 "parser.y"
    {
                   checkMatrix();
                }
    break;

  case 74:
#line 595 "parser.y"
    {
                   enterUniformMatrix();
                }
    break;

  case 75:
#line 599 "parser.y"
    {
		  enterResetMatrix();
		}
    break;

  case 76:
#line 603 "parser.y"
    {
                   checkMatrix();
                }
    break;

  case 77:
#line 608 "parser.y"
    {
                   enterMatrix( (yyvsp[(2) - (2)].f_num) );
                }
    break;

  case 78:
#line 612 "parser.y"
    {
                   enterMatrix( (yyvsp[(1) - (1)].f_num) );
                }
    break;

  case 79:
#line 617 "parser.y"
    {
                   enterMatrix( (yyvsp[(2) - (2)].f_num) );
                }
    break;

  case 80:
#line 621 "parser.y"
    {
                   enterMatrix( (yyvsp[(1) - (1)].f_num) );
                }
    break;

  case 81:
#line 626 "parser.y"
    {
                   if(( (yyvsp[(1) - (1)].constBlk)->theValue.theInt < 0 ) 
                      || ((yyvsp[(1) - (1)].constBlk)->theValue.theInt >= gNumStates )) {
                      ERR_enter("Parser<ytab>:", currentLineNumber, 
                                BAD_STATE_VAL, "");
                      (yyval.i_num) = 0;
                   }
                   else
                      (yyval.i_num) = (yyvsp[(1) - (1)].constBlk)->theValue.theInt;
                   free( (yyvsp[(1) - (1)].constBlk) );
                }
    break;

  case 82:
#line 638 "parser.y"
    {
                   int num;
                   num = H_lookup( (yyvsp[(1) - (1)].constBlk)->theValue.theString, nt_state );
                   if(( num < 0 ) || (num >= gNumStates )) {
                      ERR_enter("Parser<ytab>:", currentLineNumber, 
                                BAD_STATE_STR, (yyvsp[(1) - (1)].constBlk)->theValue.theString );
                      (yyval.i_num) = 0;
                   }
                   else
                      (yyval.i_num) = num;
                   free( (yyvsp[(1) - (1)].constBlk)->theValue.theString );
                   free( (yyvsp[(1) - (1)].constBlk) );
                }
    break;

  case 83:
#line 652 "parser.y"
    {
                   (yyval.i_num) = WILDCARD_SPEC;
                }
    break;

  case 84:
#line 657 "parser.y"
    {
                   (yyval.i_num) = (yyvsp[(1) - (1)].constBlk)->theValue.theInt;
                   if(( (yyvsp[(1) - (1)].constBlk)->theValue.theInt < 0 ) 
                      || ((yyvsp[(1) - (1)].constBlk)->theValue.theInt >= gNumActions )) {
                      ERR_enter("Parser<ytab>:", currentLineNumber, 
                                BAD_ACTION_VAL, "" );
                      (yyval.i_num) = 0;
                   }
                   else
                      (yyval.i_num) = (yyvsp[(1) - (1)].constBlk)->theValue.theInt;
                   free( (yyvsp[(1) - (1)].constBlk) );
                }
    break;

  case 85:
#line 670 "parser.y"
    {
                   int num;
                   num = H_lookup( (yyvsp[(1) - (1)].constBlk)->theValue.theString, nt_action );
                   if(( num < 0 ) || (num >= gNumActions )) {
                      ERR_enter("Parser<ytab>:", currentLineNumber, 
                                BAD_ACTION_STR, (yyvsp[(1) - (1)].constBlk)->theValue.theString );
                      (yyval.i_num) = 0;
                   }
                   else
                      (yyval.i_num) = num;
                   free( (yyvsp[(1) - (1)].constBlk)->theValue.theString );
                   free( (yyvsp[(1) - (1)].constBlk) );
                }
    break;

  case 86:
#line 684 "parser.y"
    {
                   (yyval.i_num) = WILDCARD_SPEC;
                }
    break;

  case 87:
#line 689 "parser.y"
    {
                   if(( (yyvsp[(1) - (1)].constBlk)->theValue.theInt < 0 ) 
                      || ((yyvsp[(1) - (1)].constBlk)->theValue.theInt >= gNumObservations )) {
                      ERR_enter("Parser<ytab>:", currentLineNumber, 
                                BAD_OBS_VAL, "");
                      (yyval.i_num) = 0;
                   }
                   else
                      (yyval.i_num) = (yyvsp[(1) - (1)].constBlk)->theValue.theInt;
                   free( (yyvsp[(1) - (1)].constBlk) );
                }
    break;

  case 88:
#line 701 "parser.y"
    {
                   int num;
                   num = H_lookup( (yyvsp[(1) - (1)].constBlk)->theValue.theString, nt_observation );
                   if(( num < 0 ) || (num >= gNumObservations )) { 
                      ERR_enter("Parser<ytab>:", currentLineNumber, 
                                BAD_OBS_STR, (yyvsp[(1) - (1)].constBlk)->theValue.theString);
                      (yyval.i_num) = 0;
                   }
                   else
                      (yyval.i_num) = num;
                   free( (yyvsp[(1) - (1)].constBlk)->theValue.theString );
                   free( (yyvsp[(1) - (1)].constBlk) );
               }
    break;

  case 89:
#line 715 "parser.y"
    {
                   (yyval.i_num) = WILDCARD_SPEC;
                }
    break;

  case 90:
#line 720 "parser.y"
    {
		   if (curMnemonic != nt_goal)
		     enterString( (yyvsp[(2) - (2)].constBlk) );
		   else
		     gGoalList[gNumGoals++] = H_lookup( (yyvsp[(2) - (2)].constBlk)->theValue.theString, nt_state );
		   
                }
    break;

  case 91:
#line 728 "parser.y"
    {
		   if (curMnemonic != nt_goal)
		     enterString( (yyvsp[(1) - (1)].constBlk) );
		   else
		     gGoalList[gNumGoals++] = H_lookup( (yyvsp[(1) - (1)].constBlk)->theValue.theString, nt_state );
                }
    break;

  case 92:
#line 736 "parser.y"
    {
		   gGoalList[gNumGoals++] = (yyvsp[(2) - (2)].constBlk)->theValue.theInt;
                   free((yyvsp[(2) - (2)].constBlk));
		}
    break;

  case 93:
#line 741 "parser.y"
    {
		   gGoalList[gNumGoals++] = (yyvsp[(1) - (1)].constBlk)->theValue.theInt;
                   free((yyvsp[(1) - (1)].constBlk));
		}
    break;

  case 94:
#line 747 "parser.y"
    {
		  (yyval.f_num) = (yyvsp[(1) - (1)].constBlk)->theValue.theInt;
		  if( curMatrixContext != mc_mdp_start )
		    if(( (yyval.f_num) < 0 ) || ((yyval.f_num) > 1 ))
		      ERR_enter("Parser<ytab>:", currentLineNumber, 
				BAD_PROB_VAL, "");
		  free( (yyvsp[(1) - (1)].constBlk) );
		}
    break;

  case 95:
#line 756 "parser.y"
    {
                   (yyval.f_num) = (yyvsp[(1) - (1)].constBlk)->theValue.theFloat;
		   if( curMatrixContext == mc_mdp_start )
		     ERR_enter("Parser<ytab>:", currentLineNumber, 
			       BAD_START_STATE_TYPE, "" );
		   else
		     if(( (yyval.f_num) < 0.0 ) || ((yyval.f_num) > 1.0 ))
		       ERR_enter("Parser<ytab>:", currentLineNumber, 
				 BAD_PROB_VAL, "" );
                   free( (yyvsp[(1) - (1)].constBlk) );
		}
    break;

  case 96:
#line 769 "parser.y"
    {
                   if( (yyvsp[(1) - (2)].i_num) )
                      (yyval.f_num) = (yyvsp[(2) - (2)].constBlk)->theValue.theInt * -1.0;
                   else
                      (yyval.f_num) = (yyvsp[(2) - (2)].constBlk)->theValue.theInt;
                   free( (yyvsp[(2) - (2)].constBlk) );
                }
    break;

  case 97:
#line 777 "parser.y"
    {
                   if( (yyvsp[(1) - (2)].i_num) )
                      (yyval.f_num) = (yyvsp[(2) - (2)].constBlk)->theValue.theFloat * -1.0;
                   else
                      (yyval.f_num) = (yyvsp[(2) - (2)].constBlk)->theValue.theFloat;
                   free( (yyvsp[(2) - (2)].constBlk) );
                }
    break;

  case 98:
#line 786 "parser.y"
    {
                   (yyval.i_num) = 0;
                }
    break;

  case 99:
#line 790 "parser.y"
    {
                   (yyval.i_num) = 1;
                }
    break;

  case 100:
#line 794 "parser.y"
    {
                   (yyval.i_num) = 0;
                }
    break;


/* Line 1267 of yacc.c.  */
#line 2437 "y.tab.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


#line 801 "parser.y"


/********************************************************************/
/*              External Routines                                   */
/********************************************************************/

#define EPSILON  0.00001  /* tolerance for sum of probs == 1 */

Constant_Block *aConst;

/******************************************************************************/
void yyerror(char *string)
{
   ERR_enter("Parser<yyparse>", currentLineNumber, PARSE_ERR,"");
}  /* yyerror */
/******************************************************************************/
void checkMatrix() {
/* When a matrix is finished being read for the exactly correct number of
   values, curRow should be 0 and curCol should be -1.  For the cases
   where we are only interested in a row of entries curCol should be -1.
   If we get too many entries, then we will catch this as we parse the 
   extra entries.  Therefore, here we only need to check for too few 
   entries.
   */

   switch( curMatrixContext ) {
   case mc_trans_row:
      if( curCol < gNumStates )
         ERR_enter("Parser<checkMatrix>:", currentLineNumber, 
                   TOO_FEW_ENTRIES, "");
      break;
   case mc_trans_all:
      if((curRow < (gNumStates-1) )
	 || ((curRow == (gNumStates-1))
	     && ( curCol < gNumStates ))) 
	ERR_enter("Parser<checkMatrix>:", currentLineNumber,  
                   TOO_FEW_ENTRIES, "" );
      break;
   case mc_obs_row:
      if( curCol < gNumObservations )
         ERR_enter("Parser<checkMatrix>:", currentLineNumber, 
                   TOO_FEW_ENTRIES, "");
      break;
   case mc_obs_all:
      if((curRow < (gNumStates-1) )
	 || ((curRow == (gNumStates-1))
	     && ( curCol < gNumObservations ))) 
         ERR_enter("Parser<checkMatrix>:", currentLineNumber,  
                   TOO_FEW_ENTRIES, "" );
      break;
   case mc_start_belief:
      if( curCol < gNumStates )
	ERR_enter("Parser<checkMatrix>:", currentLineNumber, 
		  TOO_FEW_ENTRIES, "");
      break;

    case mc_mdp_start:
      /* We will check for invalid multiple entries for MDP in 
	 enterMatrix() */
      break;

    case mc_reward_row:
      if( gProblemType == POMDP_problem_type )
	if( curCol < gNumObservations )
	  ERR_enter("Parser<checkMatrix>:", currentLineNumber, 
		    TOO_FEW_ENTRIES, "");
      break;

    case mc_reward_all:
      if( gProblemType == POMDP_problem_type ) {
	if((curRow < (gNumStates-1) )
	   || ((curRow == (gNumStates-1))
	       && ( curCol < gNumObservations ))) 
	  ERR_enter("Parser<checkMatrix>:", currentLineNumber,  
		    TOO_FEW_ENTRIES, "" );
      }
      else
	if( curCol < gNumStates )
	  ERR_enter("Parser<checkMatrix>:", currentLineNumber, 
		    TOO_FEW_ENTRIES, "");
      
      break;
    case mc_reward_single:
      /* Don't need to do anything */
      break;

    case mc_reward_mdp_only:
      if((curRow < (gNumStates-1) )
	 || ((curRow == (gNumStates-1))
	     && ( curCol < gNumStates ))) 
	ERR_enter("Parser<checkMatrix>:", currentLineNumber,  
		  TOO_FEW_ENTRIES, "" );
      break;

   default:
      ERR_enter("Parser<checkMatrix>:", currentLineNumber, 
                BAD_MATRIX_CONTEXT, "" );
      break;
   }  /* switch */

   if( gTooManyEntries )
     ERR_enter("Parser<checkMatrix>:", currentLineNumber, 
	       TOO_MANY_ENTRIES, "" );

   /* After reading a line for immediate rewards for a pomdp, we must tell
      the data structures for the special representation that we are done */
   switch( curMatrixContext ) {
   case mc_reward_row:
   case mc_reward_all:
   case mc_reward_mdp_only:
     doneImmReward();
     break;

     /* This case is only valid for POMDPs, so if we have an MDP, we
	never would have started a new immediate reward, so calling 
	the doneImmReward will be in error.  */
   case mc_reward_single:
     if( gProblemType == POMDP_problem_type )
       doneImmReward();
     break;
   default:
     break;
   }  /* switch */
   

   curMatrixContext = mc_none;  /* reset this as a safety precaution */
}  /* checkMatrix */
/******************************************************************************/
void enterString( Constant_Block *block ) {
   
   if( H_enter( block->theValue.theString, curMnemonic ) == 0 )
      ERR_enter("Parser<enterString>:", currentLineNumber, 
                DUPLICATE_STRING, block->theValue.theString );

   free( block->theValue.theString );
   free( block );
}  /* enterString */
/******************************************************************************/
void enterUniformMatrix( ) {
   int a, i, j, obs;
   double prob;

   switch( curMatrixContext ) {
   case mc_trans_row:
      prob = 1.0/gNumStates;
      for( a = minA; a <= maxA; a++ )
         for( i = minI; i <= maxI; i++ )
            for( j = 0; j < gNumStates; j++ )
	       addEntryToIMatrix( IP[a], i, j, prob );
      break;
   case mc_trans_all:
      prob = 1.0/gNumStates;
      for( a = minA; a <= maxA; a++ )
         for( i = 0; i < gNumStates; i++ )
            for( j = 0; j < gNumStates; j++ )
 	       addEntryToIMatrix( IP[a], i, j, prob );
      break;
   case mc_obs_row:
      prob = 1.0/gNumObservations;
      for( a = minA; a <= maxA; a++ )
         for( j = minJ; j <= maxJ; j++ )
            for( obs = 0; obs < gNumObservations; obs++ )
 	       addEntryToIMatrix( IR[a], j, obs, prob );
      break;
   case mc_obs_all:
      prob = 1.0/gNumObservations;
      for( a = minA; a <= maxA; a++ )
         for( j = 0; j < gNumStates; j++ )
            for( obs = 0; obs < gNumObservations; obs++ )
 	       addEntryToIMatrix( IR[a], j, obs, prob );
      break;
   case mc_start_belief:
      setStartStateUniform();
      break;
   case mc_mdp_start:
      /* This is meaning less for an MDP */
      ERR_enter("Parser<enterUniformMatrix>:", currentLineNumber, 
                BAD_START_STATE_TYPE, "" );
      break;
   default:
      ERR_enter("Parser<enterUniformMatrix>:", currentLineNumber, 
                BAD_MATRIX_CONTEXT, "" );
      break;
   }  /* switch */
}  /* enterUniformMatrix */
/******************************************************************************/
void enterIdentityMatrix( ) {
   int a, i,j;

   switch( curMatrixContext ) {
   case mc_trans_all:
      for( a = minA; a <= maxA; a++ )
         for( i = 0; i < gNumStates; i++ )
            for( j = 0; j < gNumStates; j++ )
               if( i == j )
		 addEntryToIMatrix( IP[a], i, j, 1.0 );
               else
		 addEntryToIMatrix( IP[a], i, j, 0.0 );
      break;
   default:
      ERR_enter("Parser<enterIdentityMatrix>:", currentLineNumber, 
                BAD_MATRIX_CONTEXT, "" );
      break;
   }  /* switch */
}  /* enterIdentityMatrix */
/******************************************************************************/
void enterResetMatrix( ) {
  int a, i, j;

  if( curMatrixContext != mc_trans_row ) {
    ERR_enter("Parser<enterMatrix>:", currentLineNumber, 
	      BAD_RESET_USAGE, "" );
    return;
  }

  if( gProblemType == POMDP_problem_type )
    for( a = minA; a <= maxA; a++ )
      for( i = minI; i <= maxI; i++ )
	for( j = 0; j < gNumStates; j++ )
	  addEntryToIMatrix( IP[a], i, j, gInitialBelief[j] );
  
  else  /* It is an MDP */
    for( a = minA; a <= maxA; a++ )
      for( i = minI; i <= maxI; i++ )
	addEntryToIMatrix( IP[a], i, gInitialState, 1.0 );
  

}  /* enterResetMatrix */
/******************************************************************************/
void enterMatrix( double value ) {
/*
  For the '_single' context types we never have to worry about setting or 
  checking the bounds on the current row or col.  For all other we do and
  how this is done depends on the context.  Notice that we are filling in the 
  elements in reverse order due to the left-recursive grammar.  Thus
  we need to update the col and row backwards 
  */
   int a, i, j, obs;

   switch( curMatrixContext ) {
   case mc_trans_single:
      for( a = minA; a <= maxA; a++ )
         for( i = minI; i <= maxI; i++ )
            for( j = minJ; j <= maxJ; j++ )
	      addEntryToIMatrix( IP[a], i, j, value );
      break;
   case mc_trans_row:
      if( curCol < gNumStates ) {
         for( a = minA; a <= maxA; a++ )
            for( i = minI; i <= maxI; i++ )
	      addEntryToIMatrix( IP[a], i, curCol, value );
         curCol++;
      }
      else
	gTooManyEntries = 1;

      break;
   case mc_trans_all:
      if( curCol >= gNumStates ) {
         curRow++;
         curCol = 0;;
      }

      if( curRow < gNumStates ) {
         for( a = minA; a <= maxA; a++ )
	   addEntryToIMatrix( IP[a], curRow, curCol, value );
         curCol++;
      }
      else
	gTooManyEntries = 1;

      break;

   case mc_obs_single:

      if( gProblemType == POMDP_problem_type )
	/* We ignore this if it is an MDP */

	for( a = minA; a <= maxA; a++ )
	  for( j = minJ; j <= maxJ; j++ )
            for( obs = minObs; obs <= maxObs; obs++ )
	      addEntryToIMatrix( IR[a], j, obs, value );
      break;

   case mc_obs_row:
      if( gProblemType == POMDP_problem_type )
	/* We ignore this if it is an MDP */

	if( curCol < gNumObservations ) {

	  for( a = minA; a <= maxA; a++ )
            for( j = minJ; j <= maxJ; j++ )
	      addEntryToIMatrix( IR[a], j, curCol, value );
	  
	  curCol++;
	}
	else
	  gTooManyEntries = 1;

      break;

   case mc_obs_all:
      if( curCol >= gNumObservations ) {
         curRow++;
         curCol = 0;
      }

      if( gProblemType == POMDP_problem_type )
	/* We ignore this if it is an MDP */

	if( curRow < gNumStates ) {
	  for( a = minA; a <= maxA; a++ )
	    addEntryToIMatrix( IR[a], curRow, curCol, value );
	  
	  curCol++;
	}
	else
	  gTooManyEntries = 1;

      break;

/* This is a special case for POMDPs, since we need a special 
   representation for immediate rewards for POMDP's.  Note that this 
   is not valid syntax for an MDP, but we flag this error when we set 
   the matrix context, so we ignore the MDP case here.
   */
   case mc_reward_single:
      if( gProblemType == POMDP_problem_type ) {

	if( curCol == 0 ) {
	  enterImmReward( 0, 0, 0, value );
	  curCol++;
	}
	else
	  gTooManyEntries = 1;

      }
     break;

    case mc_reward_row:
      if( gProblemType == POMDP_problem_type ) {

	/* This is a special case for POMDPs, since we need a special 
	   representation for immediate rewards for POMDP's */
   
	if( curCol < gNumObservations ) {
	  enterImmReward( 0, 0, curCol, value );
	  curCol++;
	}
	else
	  gTooManyEntries = 1;

      }  /* if POMDP problem */

      else /* we are dealing with an MDP, so there should only be 
	      a single entry */
	if( curCol == 0 ) {
	  enterImmReward( 0, 0, 0, value );
	  curCol++;
	}
	else
	  gTooManyEntries = 1;


     break;

   case mc_reward_all:

      /* This is a special case for POMDPs, since we need a special 
	 representation for immediate rewards for POMDP's */

      if( gProblemType == POMDP_problem_type ) {
	if( curCol >= gNumObservations ) {
	  curRow++;
	  curCol = 0;
	}
	if( curRow < gNumStates ) {
	  enterImmReward( 0, curRow, curCol, value );
	  curCol++;
	}
	else
	  gTooManyEntries = 1;

      }  /* If POMDP problem */

      /* Otherwise it is an MDP and we should be expecting an entire
	 row of rewards. */

      else  /* MDP */
	if( curCol < gNumStates ) {
	  enterImmReward( 0, curCol, 0, value );
	  curCol++;
	}
	else
	  gTooManyEntries = 1;

      break;

      /* This is a special case for an MDP only where we specify
	 the entire matrix of rewards. If we are erroneously 
	 definining a POMDP, this error will be flagged in the 
	 setMatrixContext() routine.
	 */

    case mc_reward_mdp_only:
      if( gProblemType == MDP_problem_type ) {
	if( curCol >= gNumStates ) {
	  curRow++;
	  curCol = 0;
	}
	if( curRow < gNumStates ) {
	  enterImmReward( curRow, curCol, 0, value );
	  curCol++;
	}
	else
	  gTooManyEntries = 1;

      }
      break;

    case mc_mdp_start:

      /* For an MDP we only want to see a single value and */
      /* we want it to correspond to a valid state number. */

      if( curCol > 0 )
	gTooManyEntries = 1;

      else {
	gInitialState = (int) value;
	curCol++;
      }
      break;
	  
   case mc_start_belief:

      /* This will process the individual entries when a starting */
      /* belief state is fully specified.  When it is a POMDP, we need */
      /* an entry for each state, so we keep the curCol variable */
      /* updated.  */

      if( curCol < gNumStates ) {
	gInitialBelief[curCol] = value;
	curCol++;
      }
      else
	gTooManyEntries = 1;

      break;

   default:
      ERR_enter("Parser<enterMatrix>:", currentLineNumber, 
                BAD_MATRIX_CONTEXT, "");
      break;
   }  /* switch */

}  /* enterMatrix */
/******************************************************************************/
void setMatrixContext( Matrix_Context context, 
                      int a, int i, int j, int obs ) {
/* 
   Note that we must enter the matrix entries in reverse order because
   the matrices are defined with left-recursive rules.  Set the a, i,
   and j parameters to be less than zero when you want to define it
   for all possible values.  

   Rewards for MDPs and POMDPs differ since in the former, rewards are not
   based upon an observations.  This complicates things since not only is one 
   of the reward syntax options not valid, but the semantics of all the
   rewards change as well.  I have chosen to handle this in this routine.  
   I will check for the appropriate type and set the context to handle the
   proper amount of entries.
*/
  int state;

   curMatrixContext = context;
   gTooManyEntries = 0;  /* Clear this out before reading any */

   curRow = 0;  /* This is ignored for some contexts */
   curCol = 0;

   switch( curMatrixContext ) {

   mc_start_belief:
     
     break;

   case mc_start_include:

     /* When we specify the starting belief state as a list of states */
     /* to include, we initialize all state to 0.0, since as we read */
     /* the states we will set that particular value to 1.0.  After it */
     /* is all done we can then just normalize the belief state */

     if( gProblemType == POMDP_problem_type )
       for( state = 0; state < gNumStates; state++ )
	 gInitialBelief[state] = 0.0;

     else  /* It is an MDP which is not valid */
       ERR_enter("Parser<setMatrixContext>:", currentLineNumber, 
		 BAD_START_STATE_TYPE, "");
      
     break;

   case mc_start_exclude:

     /* When we are specifying the starting belief state as a a list */
     /* of states, we initialize all states to 1.0 and as we read each */
     /* in the list we clear it out to be zero.  fter it */
     /* is all done we can then just normalize the belief state */

     if( gProblemType == POMDP_problem_type )
       for( state = 0; state < gNumStates; state++ )
	 gInitialBelief[state] = 1.0;

     else  /* It is an MDP which is not valid */
       ERR_enter("Parser<setMatrixContext>:", currentLineNumber, 
		 BAD_START_STATE_TYPE, "");

     break;

  /* We need a special representation for the immediate rewards.
     These four cases initialize the data structure that will be
     needed for immediate rewards by calling newImmReward.  Note that
     the arguments will differe depending upon whether it is an
     MDP or POMDP.
     */
  case mc_reward_mdp_only:
    if( gProblemType == POMDP_problem_type )  {
       ERR_enter("Parser<setMatrixContext>:", currentLineNumber, 
		 BAD_REWARD_SYNTAX, "");
    }
    else {
      newImmReward( a, NOT_PRESENT, NOT_PRESENT, 0 );
    } 
    break;
 
  case mc_reward_all:	
    if( gProblemType == POMDP_problem_type ) 
      newImmReward( a, i, NOT_PRESENT, NOT_PRESENT );

    else {
      newImmReward( a, i, NOT_PRESENT, 0 );
    }
    break;
  case mc_reward_row:
    if( gProblemType == POMDP_problem_type ) 
      newImmReward( a, i, j, NOT_PRESENT );
    
    else {
      newImmReward( a, i, j, 0 );
    } 
    break;
  case mc_reward_single:

    if( gProblemType == MDP_problem_type ) {
       ERR_enter("Parser<setMatrixContext>:", currentLineNumber, 
		 BAD_REWARD_SYNTAX, "");
    }
    else {
       newImmReward( a, i, j, obs );
     }
    break;

   default:
     break;
   }

  /* These variable settings will define the range over which the current 
     matrix context will have effect.  This accounts for wildcards by
     setting the range to include everything.  When a single entry was
     specified, the range is that single number.  When we actually 
     start to read the matrix, each entry we see will apply for the
     entire range specified, though for specific entries the range 
     will be a single number.
     */
   if( a < 0 ) {
      minA = 0;
      maxA = gNumActions - 1;
   }
   else
      minA = maxA = a;

   if( i < 0 ) {
      minI = 0;
      maxI = gNumStates - 1;
   }
   else
      minI = maxI = i;

   if( j < 0 ) {
      minJ = 0;
      maxJ = gNumStates - 1;
   }
   else
      minJ = maxJ = j;

   if( obs < 0 ) {
      minObs = 0;
      maxObs = gNumObservations - 1;
   }
   else
      minObs = maxObs = obs;

}  /* setMatrixContext */
/******************************************************************************/
void enterStartState( int i ) {
/*
   This is not valid for an MDP, but the error has already been flagged
   in the setMatrixContext() routine.  Therefore, if just igore this if 
   it is an MDP.
*/

  if( gProblemType == MDP_problem_type )
    return;

  switch( curMatrixContext ) {
  case mc_start_include:
    gInitialBelief[i] = 1.0;
    break;
  case mc_start_exclude:
    gInitialBelief[i] = 0.0;
    break;
  default:
    ERR_enter("Parser<enterStartState>:", currentLineNumber, 
	      BAD_MATRIX_CONTEXT, "");
      break;
  } /* switch */
}  /* enterStartState */
/******************************************************************************/
void setStartStateUniform() {
  int i;
  double prob;

  if( gProblemType != POMDP_problem_type )
    return;

  prob = 1.0/gNumStates;
  for( i = 0; i < gNumStates; i++ )
    gInitialBelief[i] = prob;

}  /*  setStartStateUniform*/
/******************************************************************************/
void endStartStates() {
/*
   There are a few cases where the matrix context will not be
   set at this point.  When there is a list of probabilities
   or if it is an MDP the context will have been cleared.
   */
  int i;
  double prob;

  if( gProblemType == MDP_problem_type ) {
    curMatrixContext = mc_none;  /* just to be sure */
    return;
  }
    
  switch( curMatrixContext ) {
  case mc_start_include:
  case mc_start_exclude:
    /* At this point gInitialBelief should be a vector of 1.0's and 0.0's
       being set as each is either included or excluded.  Now we need to
       normalized them to make it a true probability distribution */
    prob = 0.0;
    for( i = 0; i < gNumStates; i++ )
      prob += gInitialBelief[i];
    if( prob <= 0.0 ) {
      ERR_enter("Parser<endStartStates>:", currentLineNumber, 
                BAD_START_PROB_SUM, "" );
      return;
    }
    for( i = 0; i < gNumStates; i++ )
      gInitialBelief[i] /= prob;
    break;

  default:  /* Make sure we have a valid prob. distribution */
    prob = 0.0;
    for( i = 0; i < gNumStates; i++ ) 
      prob += gInitialBelief[i];
    if((prob < ( 1.0 - EPSILON)) || (prob > (1.0 + EPSILON))) {
      ERR_enter("Parser<endStartStates>:", NO_LINE, 
		BAD_START_PROB_SUM, "" );
    }
    break;
  }  /* switch */

  curMatrixContext = mc_none;

}  /* endStartStates */
/******************************************************************************/
void verifyPreamble() {
/* 
   When a param is not defined, set these to non-zero so parsing can
   proceed even in the absence of specifying these values.  When an
   out of range value is encountered the parser will flag the error,
   but return 0 so that more errors can be detected 
   */

   if( discountDefined == 0 )
      ERR_enter("Parser<verifyPreamble>:", currentLineNumber, 
                MISSING_DISCOUNT, "" );
   if( valuesDefined == 0 )
      ERR_enter("Parser<verifyPreamble>:", currentLineNumber,
                MISSING_VALUES, "" );
   if( statesDefined == 0 ) {
      ERR_enter("Parser<verifyPreamble>:", currentLineNumber, 
                MISSING_STATES, "" );
      gNumStates = 1;
   }
   if( actionsDefined == 0 ) {
      ERR_enter("Parser<verifyPreamble>:", currentLineNumber, 
                MISSING_ACTIONS, "" );
      gNumActions = 1;
   }

   /* If we do not see this, them we must be parsing an MDP */
   if( observationsDefined == 0 ) {
     gNumObservations = 0;
     gProblemType = MDP_problem_type;
   }

   else
     gProblemType = POMDP_problem_type;

#if 0
   if( goalsDefined == 0 ) {
      ERR_enter("Parser<verifyPreamble>:", currentLineNumber, 
                MISSING_GOALS, "" );
      gNumGoals = 0;
   }
#endif

}  /* verifyPreamble */
/******************************************************************************/
void checkProbs() {
   int a,i,j,obs;
   double sum;
   char str[40];

   
   for( a = 0; a < gNumActions; a++ )
      for( i = 0; i < gNumStates; i++ ) {
	 sum = sumIMatrixRowValues( IP[a], i );
         if((sum < ( 1.0 - EPSILON)) || (sum > (1.0 + EPSILON))) {
            sprintf( str, "action=%d, state=%d (%.5lf)", a, i, sum );
            ERR_enter("Parser<checkProbs>:", NO_LINE, 
                      BAD_TRANS_PROB_SUM, str );
         }
      } /* for i */

   if( gProblemType == POMDP_problem_type )
     for( a = 0; a < gNumActions; a++ )
       for( j = 0; j < gNumStates; j++ ) {
	 sum = sumIMatrixRowValues( IR[a], j );
         if((sum < ( 1.0 - EPSILON)) || (sum > (1.0 + EPSILON))) {
	   sprintf( str, "action=%d, state=%d (%.5lf)", a, j, sum );
	   ERR_enter("Parser<checkProbs>:", NO_LINE, 
		     BAD_OBS_PROB_SUM, str );
         } /* if sum not == 1 */
       }  /* for j */

   /* Now see if we had observation specs defined in an MDP */

   if( observationSpecDefined && (gProblemType == MDP_problem_type))
     ERR_enter("Parser<checkProbs>:", NO_LINE, 
	       OBS_IN_MDP_PROBLEM, "" );

}  /* checkProbs */
/************************************************************************/
void initParser() {
/*
   This routine will reset all the state variables used by the parser
   in case it will parse multiple files.
*/
   observationSpecDefined = 0;
   discountDefined = 0;
   valuesDefined = 0;
   statesDefined = 0;
   actionsDefined = 0;
   observationsDefined = 0;
   observationSpecDefined = 0;
   currentLineNumber = 1;
   curMnemonic = nt_unknown;
   curMatrixContext = mc_none;

}  /* initParser */
/************************************************************************/
int readMDPFile( FILE *file ) {
   int returnValue;
   extern FILE *yyin;

   initParser();

   ERR_initialize();
   H_create();
   yyin = file;

   returnValue = yyparse();

   /* If there are syntax errors, then we have to do something if we 
      want to parse another file without restarting.  It seems that
      a syntax error bombs the code out, but leaves the file pointer
      at the place it bombed.  Thus, another call to yyparse() will
      pick up where it left off and not necessarily at the start of a 
      new file.

      Unfortunately, I do not know how to do this yet.
      */
   if (returnValue != 0) {
      printf("\nParameter file contains syntax errors!\n");
    }

   if (ERR_dump() || returnValue ) 
      return( 0 );

   ERR_cleanUp();
   H_destroy();

   /* This is where intermediate matrix representation are
      converted into their final representation */
   convertMatrices();
   //deallocateIntermediateMDP();

   return( 1 );
}  /* readPomdpFile */
/************************************************************************/

