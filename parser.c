/* A Bison parser, made by GNU Bison 2.0.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004 Free Software Foundation, Inc.

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
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 1



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     INTEGER_LITERAL = 258,
     STRING_LITERAL = 259,
     FILE_PATH = 260,
     IDENTIFIER = 261,
     LOCAL_ID = 262,
     FORWARD_BRANCH = 263,
     BACKWARD_BRANCH = 264,
     LABEL = 265,
     LOCAL_LABEL = 266,
     MNEMONIC = 267,
     _LABEL_ = 268,
     BYTE = 269,
     CHAR = 270,
     WORD = 271,
     DWORD = 272,
     DSB = 273,
     DSW = 274,
     DSD = 275,
     DATASEG = 276,
     CODESEG = 277,
     IF = 278,
     IFDEF = 279,
     IFNDEF = 280,
     ELSE = 281,
     ELIF = 282,
     ENDIF = 283,
     INCSRC = 284,
     INCBIN = 285,
     MACRO = 286,
     REPT = 287,
     WHILE = 288,
     ENDM = 289,
     ALIGN = 290,
     EQU = 291,
     DEFINE = 292,
     END = 293,
     PUBLIC = 294,
     EXTRN = 295,
     CHARMAP = 296,
     STRUC = 297,
     UNION = 298,
     ENDS = 299,
     RECORD = 300,
     ENUM = 301,
     ENDE = 302,
     PROC = 303,
     ENDP = 304,
     SIZEOF = 305,
     MASK = 306,
     TAG = 307,
     MESSAGE = 308,
     WARNING = 309,
     ERROR = 310,
     ZEROPAGE = 311,
     ORG = 312,
     SCOPE_OP = 313,
     HI_OP = 314,
     LO_OP = 315,
     NE_OP = 316,
     EQ_OP = 317,
     GE_OP = 318,
     LE_OP = 319,
     SHR_OP = 320,
     SHL_OP = 321,
     UMINUS = 322
   };
#endif
#define INTEGER_LITERAL 258
#define STRING_LITERAL 259
#define FILE_PATH 260
#define IDENTIFIER 261
#define LOCAL_ID 262
#define FORWARD_BRANCH 263
#define BACKWARD_BRANCH 264
#define LABEL 265
#define LOCAL_LABEL 266
#define MNEMONIC 267
#define _LABEL_ 268
#define BYTE 269
#define CHAR 270
#define WORD 271
#define DWORD 272
#define DSB 273
#define DSW 274
#define DSD 275
#define DATASEG 276
#define CODESEG 277
#define IF 278
#define IFDEF 279
#define IFNDEF 280
#define ELSE 281
#define ELIF 282
#define ENDIF 283
#define INCSRC 284
#define INCBIN 285
#define MACRO 286
#define REPT 287
#define WHILE 288
#define ENDM 289
#define ALIGN 290
#define EQU 291
#define DEFINE 292
#define END 293
#define PUBLIC 294
#define EXTRN 295
#define CHARMAP 296
#define STRUC 297
#define UNION 298
#define ENDS 299
#define RECORD 300
#define ENUM 301
#define ENDE 302
#define PROC 303
#define ENDP 304
#define SIZEOF 305
#define MASK 306
#define TAG 307
#define MESSAGE 308
#define WARNING 309
#define ERROR 310
#define ZEROPAGE 311
#define ORG 312
#define SCOPE_OP 313
#define HI_OP 314
#define LO_OP 315
#define NE_OP 316
#define EQ_OP 317
#define GE_OP 318
#define LE_OP 319
#define SHR_OP 320
#define SHL_OP 321
#define UMINUS 322




/* Copy the first part of user declarations.  */
#line 1 "parser.y"

/*
 * $Id: parser.y,v 1.14 2007/11/11 22:35:51 khansen Exp $
 * $Log: parser.y,v $
 * Revision 1.14  2007/11/11 22:35:51  khansen
 * compile on mac
 *
 * Revision 1.13  2007/08/19 11:19:47  khansen
 * --case-insensitive option
 *
 * Revision 1.12  2007/08/12 18:59:00  khansen
 * ability to generate pure 6502 binary
 *
 * Revision 1.11  2007/08/11 01:25:50  khansen
 * includepaths support (-I option)
 *
 * Revision 1.10  2007/07/22 13:34:38  khansen
 * convert tabs to whitespaces
 *
 * Revision 1.9  2005/01/05 02:28:40  kenth
 * anonymous union parsing
 *
 * Revision 1.8  2004/12/29 21:45:26  kenth
 * xorcyst 1.4.2
 * static indexing
 *
 * Revision 1.7  2004/12/19 19:59:14  kenth
 * xorcyst 1.4.0
 *
 * Revision 1.6  2004/12/16 13:22:27  kenth
 * xorcyst 1.3.5
 *
 * Revision 1.5  2004/12/14 01:50:42  kenth
 * xorcyst 1.3.0
 *
 * Revision 1.4  2004/12/11 02:12:41  kenth
 * xorcyst 1.2.0
 *
 * Revision 1.3  2004/12/09 11:17:15  kenth
 * added: warning_statement, error_statement
 *
 * Revision 1.2  2004/12/06 05:06:21  kenth
 * xorcyst 1.1.0
 *
 * Revision 1.1  2004/06/30 07:57:03  kenth
 * Initial revision
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "symtab.h"
#include "loc.h"
#include "xasm.h"
//#define YYDEBUG 1
int yyparse(void);
void yyerror(const char *);   /* In lexer */
int yylex(void);    /* In lexer */
int yypushandrestart(const char *, int);   /* In lexer */
void __yy_memcpy(char *, char *, int);
extern char *yytext;    /* In lexer */
extern YYLTYPE yylloc;  /* In lexer */
char *scan_include(int); /* In lexer */
extern astnode *root_node;  /* Root of the generated parse tree */
void handle_incsrc(astnode *);  /* See below */
void handle_incbin(astnode *);  /* See below */


/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 69 "parser.y"
typedef union YYSTYPE {
    long integer;
    int mnemonic;
    const char *string;
    const char *label;
    const char *ident;
    astnode *node;
} YYSTYPE;
/* Line 185 of yacc.c.  */
#line 287 "parser.c"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

#if ! defined (YYLTYPE) && ! defined (YYLTYPE_IS_DECLARED)
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


/* Copy the second part of user declarations.  */


/* Line 213 of yacc.c.  */
#line 311 "parser.c"

#if ! defined (yyoverflow) || YYERROR_VERBOSE

# ifndef YYFREE
#  define YYFREE free
# endif
# ifndef YYMALLOC
#  define YYMALLOC malloc
# endif

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   else
#    define YYSTACK_ALLOC alloca
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (defined (YYLTYPE_IS_TRIVIAL) && YYLTYPE_IS_TRIVIAL \
             && defined (YYSTYPE_IS_TRIVIAL) && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short int yyss;
  YYSTYPE yyvs;
    YYLTYPE yyls;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short int) + sizeof (YYSTYPE) + sizeof (YYLTYPE))	\
      + 2 * YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined (__GNUC__) && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
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
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short int yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  160
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1727

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  97
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  84
/* YYNRULES -- Number of rules. */
#define YYNRULES  217
/* YYNRULES -- Number of states. */
#define YYNSTATES  381

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   322

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      58,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    93,     2,    73,    66,    92,    78,     2,
      95,    96,    90,    87,    71,    88,    72,    91,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    59,     2,
      82,    65,    81,     2,    60,    62,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    63,    64,
       2,    69,     2,    70,    77,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    67,    76,    68,    94,     2,     2,     2,
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
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    61,    74,    75,    79,    80,    83,    84,
      85,    86,    89
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short int yyprhs[] =
{
       0,     0,     3,     6,     9,    10,    12,    15,    17,    18,
      21,    23,    25,    27,    29,    31,    33,    35,    37,    39,
      41,    43,    45,    47,    49,    51,    53,    55,    57,    59,
      61,    63,    65,    67,    69,    71,    73,    75,    77,    79,
      81,    83,    85,    87,    90,    94,    99,   103,   107,   111,
     117,   120,   121,   124,   125,   132,   139,   146,   153,   160,
     167,   169,   172,   174,   177,   182,   184,   188,   192,   196,
     199,   203,   206,   208,   210,   212,   214,   216,   219,   221,
     224,   228,   231,   236,   241,   248,   255,   260,   262,   265,
     267,   269,   271,   273,   275,   277,   279,   283,   285,   287,
     289,   291,   294,   296,   301,   306,   308,   310,   312,   314,
     316,   317,   321,   325,   329,   333,   335,   336,   338,   342,
     344,   345,   347,   351,   355,   359,   363,   367,   371,   375,
     379,   383,   387,   390,   393,   396,   399,   402,   405,   409,
     413,   417,   421,   425,   429,   431,   433,   435,   437,   439,
     441,   443,   445,   446,   448,   450,   459,   461,   462,   464,
     467,   472,   476,   477,   485,   493,   496,   499,   502,   506,
     510,   515,   520,   523,   525,   530,   532,   534,   536,   538,
     541,   544,   546,   550,   554,   558,   560,   562,   570,   572,
     573,   577,   579,   580,   582,   586,   591,   596,   600,   605,
     609,   616,   619,   620,   622,   624,   626,   628,   630,   632,
     635,   639,   643,   648,   653,   657,   659,   661
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short int yyrhs[] =
{
      98,     0,    -1,   100,    99,    -1,    38,   128,    -1,    -1,
     102,    -1,   100,   102,    -1,   100,    -1,    -1,   127,   103,
      -1,   103,    -1,   150,    -1,   155,    -1,   156,    -1,   165,
      -1,   167,    -1,   162,    -1,   163,    -1,   170,    -1,   171,
      -1,   173,    -1,   174,    -1,   130,    -1,   157,    -1,   177,
      -1,   124,    -1,   125,    -1,   123,    -1,   115,    -1,   116,
      -1,   117,    -1,   120,    -1,   114,    -1,   113,    -1,   112,
      -1,   109,    -1,   108,    -1,   106,    -1,   107,    -1,   172,
      -1,   105,    -1,   104,    -1,   126,    -1,     1,   128,    -1,
      57,   132,   128,    -1,    35,   169,   132,   128,    -1,    54,
     132,   128,    -1,    55,   132,   128,    -1,    53,   132,   128,
      -1,    13,   147,   110,   111,   128,    -1,    65,   132,    -1,
      -1,    59,   176,    -1,    -1,    33,   132,   128,   101,    34,
     128,    -1,    32,   132,   128,   101,    34,   128,    -1,    48,
     147,   128,   101,    49,   128,    -1,    42,   147,   128,   101,
      44,   128,    -1,    43,   148,   128,   101,    44,   128,    -1,
      46,   147,   128,   118,    47,   128,    -1,   119,    -1,   118,
     119,    -1,   171,    -1,   147,   128,    -1,    45,   147,   121,
     128,    -1,   122,    -1,   121,    71,   122,    -1,   147,    59,
     132,    -1,    41,   164,   128,    -1,    21,   128,    -1,    21,
      56,   128,    -1,    22,   128,    -1,   128,    -1,   146,    -1,
     129,    -1,    59,    -1,    58,    -1,   131,   128,    -1,    12,
      -1,    12,    62,    -1,    12,    73,   132,    -1,    12,   132,
      -1,    12,   132,    71,    63,    -1,    12,   132,    71,    64,
      -1,    12,    69,   132,    71,    63,    70,    -1,    12,    69,
     132,    70,    71,    64,    -1,    12,    69,   132,    70,    -1,
     133,    -1,    50,   135,    -1,   137,    -1,   138,    -1,   143,
      -1,   149,    -1,    66,    -1,   144,    -1,   145,    -1,    95,
     132,    96,    -1,    87,    -1,    88,    -1,     8,    -1,     9,
      -1,    51,   137,    -1,   147,    -1,   147,    69,   132,    70,
      -1,   147,    95,   132,    96,    -1,   132,    -1,   139,    -1,
     147,    -1,   160,    -1,   132,    -1,    -1,   147,    61,   147,
      -1,   133,    72,   138,    -1,   133,    72,   133,    -1,    67,
     140,    68,    -1,   141,    -1,    -1,   142,    -1,   141,    71,
     142,    -1,   134,    -1,    -1,     7,    -1,   132,    87,   132,
      -1,   132,    88,   132,    -1,   132,    90,   132,    -1,   132,
      91,   132,    -1,   132,    92,   132,    -1,   132,    78,   132,
      -1,   132,    76,   132,    -1,   132,    77,   132,    -1,   132,
      86,   132,    -1,   132,    85,   132,    -1,    94,   132,    -1,
      93,   132,    -1,    77,   147,    -1,    82,   132,    -1,    81,
     132,    -1,    88,   132,    -1,   132,    80,   132,    -1,   132,
      79,   132,    -1,   132,    81,   132,    -1,   132,    82,   132,
      -1,   132,    83,   132,    -1,   132,    84,   132,    -1,    10,
      -1,    11,    -1,    87,    -1,    88,    -1,     8,    -1,     9,
      -1,     6,    -1,   147,    -1,    -1,     3,    -1,     4,    -1,
      23,   132,   128,   101,   151,   154,    28,   128,    -1,   152,
      -1,    -1,   153,    -1,   152,   153,    -1,    27,   132,   128,
     101,    -1,    26,   128,   101,    -1,    -1,    24,   147,   128,
     101,   154,    28,   128,    -1,    25,   147,   128,   101,   154,
      28,   128,    -1,   158,   128,    -1,   159,   128,    -1,   147,
     159,    -1,    56,   147,   159,    -1,    39,   147,   159,    -1,
      56,    39,   147,   159,    -1,    39,    56,   147,   159,    -1,
     160,   161,    -1,   160,    -1,   160,    69,   132,    70,    -1,
      14,    -1,    15,    -1,    16,    -1,    17,    -1,    52,   147,
      -1,    72,   147,    -1,   134,    -1,   161,    71,   134,    -1,
      29,   164,   128,    -1,    30,   164,   128,    -1,     4,    -1,
      82,    -1,    31,   147,   166,   128,   101,    34,   128,    -1,
     169,    -1,    -1,   147,   168,   128,    -1,   161,    -1,    -1,
     147,    -1,   169,    71,   147,    -1,   147,    36,   134,   128,
      -1,   147,    65,   134,   128,    -1,    37,   147,   128,    -1,
      37,   147,   134,   128,    -1,    39,   169,   128,    -1,    40,
     169,    59,   176,   175,   128,    -1,    60,   147,    -1,    -1,
     160,    -1,   147,    -1,    48,    -1,    13,    -1,   178,    -1,
     179,    -1,   147,   179,    -1,    56,   147,   179,    -1,    39,
     147,   179,    -1,    56,    39,   147,   179,    -1,    39,    56,
     147,   179,    -1,   180,   136,   128,    -1,    18,    -1,    19,
      -1,    20,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short int yyrline[] =
{
       0,   123,   123,   127,   128,   132,   133,   140,   141,   145,
     146,   150,   151,   152,   153,   154,   155,   156,   157,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,   182,   186,   190,   194,   198,   202,   206,
     210,   211,   215,   216,   220,   224,   228,   232,   236,   240,
     244,   245,   249,   250,   254,   258,   259,   263,   267,   271,
     272,   275,   279,   283,   287,   288,   292,   296,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   330,   331,   332,   336,   337,   341,   342,   346,
     347,   351,   355,   356,   360,   364,   365,   369,   370,   374,
     375,   379,   383,   384,   385,   386,   387,   388,   389,   390,
     391,   392,   393,   394,   395,   396,   397,   398,   402,   403,
     404,   405,   406,   407,   411,   412,   413,   414,   415,   416,
     420,   424,   425,   429,   430,   434,   438,   439,   443,   444,
     448,   452,   453,   457,   461,   465,   466,   470,   471,   472,
     473,   474,   478,   479,   480,   484,   485,   486,   487,   488,
     489,   493,   494,   498,   502,   506,   507,   511,   515,   516,
     520,   524,   525,   529,   530,   534,   538,   542,   543,   547,
     551,   555,   556,   560,   561,   562,   563,   567,   568,   572,
     573,   574,   575,   576,   580,   584,   585,   586
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "INTEGER_LITERAL", "STRING_LITERAL",
  "FILE_PATH", "IDENTIFIER", "LOCAL_ID", "FORWARD_BRANCH",
  "BACKWARD_BRANCH", "LABEL", "LOCAL_LABEL", "MNEMONIC", "_LABEL_", "BYTE",
  "CHAR", "WORD", "DWORD", "DSB", "DSW", "DSD", "DATASEG", "CODESEG", "IF",
  "IFDEF", "IFNDEF", "ELSE", "ELIF", "ENDIF", "INCSRC", "INCBIN", "MACRO",
  "REPT", "WHILE", "ENDM", "ALIGN", "EQU", "DEFINE", "END", "PUBLIC",
  "EXTRN", "CHARMAP", "STRUC", "UNION", "ENDS", "RECORD", "ENUM", "ENDE",
  "PROC", "ENDP", "SIZEOF", "MASK", "TAG", "MESSAGE", "WARNING", "ERROR",
  "ZEROPAGE", "ORG", "'\\n'", "':'", "'@'", "SCOPE_OP", "'A'", "'X'",
  "'Y'", "'='", "'$'", "'{'", "'}'", "'['", "']'", "','", "'.'", "'#'",
  "HI_OP", "LO_OP", "'|'", "'^'", "'&'", "NE_OP", "EQ_OP", "'>'", "'<'",
  "GE_OP", "LE_OP", "SHR_OP", "SHL_OP", "'+'", "'-'", "UMINUS", "'*'",
  "'/'", "'%'", "'!'", "'~'", "'('", "')'", "$accept", "assembly_unit",
  "end_opt", "statement_list", "statement_list_opt", "labelable_statement",
  "statement", "org_statement", "align_statement", "warning_statement",
  "error_statement", "message_statement", "label_statement",
  "label_addr_part_opt", "label_type_part_opt", "while_statement",
  "rept_statement", "proc_statement", "struc_decl_statement",
  "union_decl_statement", "enum_decl_statement", "enum_item_list",
  "enum_item", "record_decl_statement", "record_field_list",
  "record_field", "charmap_statement", "dataseg_statement",
  "codeseg_statement", "null_statement", "label_decl", "line_tail",
  "newline", "instruction_statement", "instruction", "expression",
  "indexed_identifier", "extended_expression", "sizeof_arg",
  "expression_opt", "scope_access", "struc_access", "struc_initializer",
  "field_initializer_list_opt", "field_initializer_list",
  "field_initializer", "local_id", "arithmetic_expression",
  "comparison_expression", "label", "identifier", "identifier_opt",
  "literal", "if_statement", "elif_statement_list_opt",
  "elif_statement_list", "elif_statement", "else_part_opt",
  "ifdef_statement", "ifndef_statement", "data_statement",
  "named_data_statement", "unnamed_data_statement", "datatype",
  "expression_list", "incsrc_statement", "incbin_statement",
  "file_specifier", "macro_decl_statement", "param_list_opt",
  "macro_statement", "arg_list_opt", "identifier_list", "equ_statement",
  "assign_statement", "define_statement", "public_statement",
  "extrn_statement", "from_part_opt", "symbol_type", "storage_statement",
  "named_storage_statement", "unnamed_storage_statement", "storage", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short int yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,    10,    58,
      64,   313,    65,    88,    89,    61,    36,   123,   125,    91,
      93,    44,    46,    35,   314,   315,   124,    94,    38,   316,
     317,    62,    60,   318,   319,   320,   321,    43,    45,   322,
      42,    47,    37,    33,   126,    40,    41
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    97,    98,    99,    99,   100,   100,   101,   101,   102,
     102,   103,   103,   103,   103,   103,   103,   103,   103,   103,
     103,   103,   103,   103,   103,   103,   103,   103,   103,   103,
     103,   103,   103,   103,   103,   103,   103,   103,   103,   103,
     103,   103,   103,   103,   104,   105,   106,   107,   108,   109,
     110,   110,   111,   111,   112,   113,   114,   115,   116,   117,
     118,   118,   119,   119,   120,   121,   121,   122,   123,   124,
     124,   125,   126,   127,   128,   128,   129,   130,   131,   131,
     131,   131,   131,   131,   131,   131,   131,   132,   132,   132,
     132,   132,   132,   132,   132,   132,   132,   132,   132,   132,
     132,   132,   133,   133,   133,   134,   134,   135,   135,   136,
     136,   137,   138,   138,   139,   140,   140,   141,   141,   142,
     142,   143,   144,   144,   144,   144,   144,   144,   144,   144,
     144,   144,   144,   144,   144,   144,   144,   144,   145,   145,
     145,   145,   145,   145,   146,   146,   146,   146,   146,   146,
     147,   148,   148,   149,   149,   150,   151,   151,   152,   152,
     153,   154,   154,   155,   156,   157,   157,   158,   158,   158,
     158,   158,   159,   159,   159,   160,   160,   160,   160,   160,
     160,   161,   161,   162,   163,   164,   164,   165,   166,   166,
     167,   168,   168,   169,   169,   170,   171,   172,   172,   173,
     174,   175,   175,   176,   176,   176,   176,   177,   177,   178,
     178,   178,   178,   178,   179,   180,   180,   180
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     2,     2,     0,     1,     2,     1,     0,     2,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     3,     4,     3,     3,     3,     5,
       2,     0,     2,     0,     6,     6,     6,     6,     6,     6,
       1,     2,     1,     2,     4,     1,     3,     3,     3,     2,
       3,     2,     1,     1,     1,     1,     1,     2,     1,     2,
       3,     2,     4,     4,     6,     6,     4,     1,     2,     1,
       1,     1,     1,     1,     1,     1,     3,     1,     1,     1,
       1,     2,     1,     4,     4,     1,     1,     1,     1,     1,
       0,     3,     3,     3,     3,     1,     0,     1,     3,     1,
       0,     1,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     2,     2,     2,     2,     2,     2,     3,     3,
       3,     3,     3,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     0,     1,     1,     8,     1,     0,     1,     2,
       4,     3,     0,     7,     7,     2,     2,     2,     3,     3,
       4,     4,     2,     1,     4,     1,     1,     1,     1,     2,
       2,     1,     3,     3,     3,     1,     1,     7,     1,     0,
       3,     1,     0,     1,     3,     4,     4,     3,     4,     3,
       6,     2,     0,     1,     1,     1,     1,     1,     1,     2,
       3,     3,     4,     4,     3,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,     0,   150,   148,   149,   144,   145,    78,     0,   175,
     176,   177,   178,   215,   216,   217,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   152,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    76,    75,     0,   146,   147,     0,     0,     5,
      10,    41,    40,    37,    38,    36,    35,    34,    33,    32,
      28,    29,    30,    31,    27,    25,    26,    42,     0,    72,
      74,    22,     0,    73,   192,    11,    12,    13,    23,     0,
       0,   173,    16,    17,    14,    15,    18,    19,    39,    20,
      21,    24,   207,   208,   110,    43,   153,   154,   121,    99,
     100,     0,     0,    79,    93,     0,     0,     0,     0,     0,
      97,    98,     0,     0,     0,    81,    87,    89,    90,    91,
      94,    95,   102,    92,    51,     0,    69,    71,     0,     0,
       0,   185,   186,     0,     0,   189,     0,     0,   193,     0,
       0,     0,   193,     0,     0,     0,     0,   151,     0,     0,
       0,     0,   179,     0,     0,     0,     0,     0,     0,   180,
       1,     0,     2,     6,     9,    77,     0,     0,   116,   105,
     181,   106,   167,   191,     0,   209,   165,   166,     0,   172,
     109,     0,    88,   107,   108,   101,     0,     0,    80,   134,
     136,   135,   137,   133,   132,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    53,    70,
       0,     0,     0,   183,   184,     0,   188,     0,     0,     0,
       0,   197,     0,     0,   169,   211,   199,     0,    68,     0,
       0,     0,    65,     0,     0,     0,    48,    46,    47,     0,
     168,   210,    44,     3,     0,     0,   119,     0,   115,   117,
       0,   190,     0,   214,    86,     0,    96,    82,    83,   128,
     129,   127,   139,   138,   140,   141,   142,   143,   131,   130,
     122,   123,   124,   125,   126,   113,   112,   102,   111,     0,
       0,    50,     0,     0,     0,   157,   162,   162,     0,     0,
       0,   194,    45,   198,   171,   213,   206,   205,   204,   203,
     202,     0,     0,     0,    64,     0,     0,    60,     0,    62,
       0,   170,   212,   195,   196,   114,   120,   182,   174,     0,
       0,   103,   104,    52,    49,     0,   162,   156,   158,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    66,
      67,     0,    61,    63,     0,   118,    85,    84,     0,     0,
     159,     0,     0,     0,     0,    55,    54,   201,   200,    57,
      58,    59,    56,     0,     0,   161,   163,   164,   187,   160,
     155
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short int yydefgoto[] =
{
      -1,    47,   162,   294,   295,    49,    50,    51,    52,    53,
      54,    55,    56,   218,   293,    57,    58,    59,    60,    61,
      62,   316,   317,    63,   241,   242,    64,    65,    66,    67,
      68,    69,    70,    71,    72,   169,   116,   170,   182,   181,
     117,   118,   171,   257,   258,   259,   119,   120,   121,    73,
     122,   148,   123,    75,   336,   337,   338,   340,    76,    77,
      78,    79,    80,    81,   173,    82,    83,   133,    84,   225,
      85,   174,   139,    86,    87,    88,    89,    90,   346,   310,
      91,    92,    93,    94
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -290
static const short int yypact[] =
{
    1502,     8,  -290,  -290,  -290,  -290,  -290,   378,    38,  -290,
    -290,  -290,  -290,  -290,  -290,  -290,   -17,     8,   748,    38,
      38,     2,     2,    38,   748,   748,    38,    38,     1,    38,
       2,    38,    38,    38,    38,    38,    38,   748,   748,   748,
      13,   748,  -290,  -290,    38,  -290,  -290,    49,   838,  -290,
    -290,  -290,  -290,  -290,  -290,  -290,  -290,  -290,  -290,  -290,
    -290,  -290,  -290,  -290,  -290,  -290,  -290,  -290,  1563,  -290,
    -290,  -290,     8,  -290,   501,  -290,  -290,  -290,  -290,     8,
       8,   536,  -290,  -290,  -290,  -290,  -290,  -290,  -290,  -290,
    -290,  -290,  -290,  -290,   748,  -290,  -290,  -290,  -290,  -290,
    -290,   160,    38,  -290,  -290,   748,   748,    38,   748,   748,
    -290,    14,   748,   748,   748,  1597,   -16,  -290,  -290,  -290,
    -290,  -290,   -26,  -290,    -2,     8,  -290,  -290,   682,     8,
       8,  -290,  -290,     8,     8,    38,   682,   682,  -290,   636,
     192,    38,   246,   -45,   -21,     8,     8,  -290,     8,    38,
       8,     8,  -290,   682,   682,   682,    38,   246,   682,  -290,
    -290,     8,  -290,  -290,  -290,  -290,   655,   655,   603,  1635,
    -290,  -290,  -290,    -3,     8,  -290,  -290,  -290,   748,    -3,
    1635,     8,  -290,  -290,  -290,  -290,    15,   705,  1635,  -290,
    1635,  1635,   -32,  -290,  -290,   725,     9,   748,   748,   748,
     748,   748,   748,   748,   748,   748,   748,   748,   748,   748,
     748,   748,   748,    38,    38,   748,   748,   748,    16,  -290,
    1004,  1087,  1087,  -290,  -290,     8,     3,  1170,  1170,    38,
     682,  -290,     8,   246,  -290,  -290,  -290,   200,  -290,  1253,
    1253,   -34,  -290,    27,    38,  1336,  -290,  -290,  -290,   246,
    -290,  -290,  -290,  -290,     8,     8,  -290,    19,    28,  -290,
     655,  -290,  1560,  -290,    30,    40,  -290,  -290,  -290,   821,
     330,   249,   441,   441,   135,   135,   135,   135,    26,    26,
     -32,   -32,  -290,  -290,  -290,   -16,  -290,   -59,  -290,  1579,
    1614,  1635,   200,     8,   921,    78,    86,    86,  1170,    85,
      87,  -290,  -290,  -290,  -290,  -290,  -290,  -290,  -290,  -290,
      60,    81,    82,    38,  -290,   748,     6,  -290,    -4,  -290,
      73,  -290,  -290,  -290,  -290,  -290,   655,  -290,  -290,    67,
      64,  -290,  -290,  -290,  -290,   748,    86,    78,  -290,     8,
     109,   112,   107,     8,     8,    38,     8,     8,     8,  -290,
    1635,     8,  -290,  -290,     8,  -290,  -290,  -290,   682,   115,
    -290,  1419,     8,     8,     8,  -290,  -290,  -290,  -290,  -290,
    -290,  -290,  -290,  1004,     8,  -290,  -290,  -290,  -290,  -290,
    -290
};

/* YYPGOTO[NTERM-NUM].  */
static const short int yypgoto[] =
{
    -290,  -290,  -290,   146,  -194,   -47,    80,  -290,  -290,  -290,
    -290,  -290,  -290,  -290,  -290,  -290,  -290,  -290,  -290,  -290,
    -290,  -290,  -165,  -290,  -290,  -158,  -290,  -290,  -290,  -290,
    -290,    -1,  -290,  -290,  -290,   286,   -57,  -137,  -290,  -290,
      56,   -54,  -290,  -290,  -290,  -164,  -290,  -290,  -290,  -290,
      62,  -290,  -290,  -290,  -290,  -290,  -176,  -289,  -290,  -290,
    -290,  -290,   -65,   -99,    84,  -290,  -290,    18,  -290,  -290,
    -290,  -290,   -24,  -290,  -233,  -290,  -290,  -290,  -290,  -122,
    -290,  -290,   -42,  -290
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -121
static const short int yytable[] =
{
      95,   163,   184,   232,   143,   144,   131,     2,   341,   172,
     215,   319,     2,    42,    43,   126,   127,    96,    97,     2,
       2,    98,    99,   100,    42,    43,   229,   296,   297,   254,
     255,   256,   175,   299,   300,   214,   216,   313,   237,   125,
     134,    42,    43,   215,     2,   311,   312,   359,   145,   160,
     229,   320,   156,   351,    42,    43,   213,   141,   210,   211,
     212,   167,    74,   217,   101,   102,    42,    43,   260,   216,
     124,   165,   267,   268,   229,   292,   214,   234,   176,   177,
     104,   129,   130,   319,   132,   135,   315,   325,   138,   140,
     142,   138,   250,   146,   147,   149,   150,   151,   152,   326,
     235,   329,   157,   330,   342,   335,   159,   112,   113,   114,
      74,   226,   339,   208,   209,   251,   210,   211,   212,   343,
     345,   344,   354,   327,   219,   347,   348,   220,   221,   222,
      74,   356,   223,   224,   357,   227,   228,   362,   309,   231,
     363,   364,   236,   374,   238,   239,    48,   240,   164,   244,
     245,   352,   246,   247,   248,   349,   285,   252,   185,   286,
     253,   360,   355,   183,   186,   179,     2,   375,   304,   189,
     333,     0,     0,   261,     9,    10,    11,    12,     0,   379,
     263,     0,     0,     0,   321,     0,     0,     0,     0,   256,
       0,   305,     0,   309,     0,    96,    97,   138,     2,    98,
      99,   100,     0,   233,     0,     0,     2,   322,     0,     0,
       0,   243,    36,   306,     9,    10,    11,    12,   249,     0,
     206,   207,   208,   209,   298,   210,   211,   212,     0,   302,
       0,   303,    44,     0,     0,     0,     0,     0,     0,     0,
     314,     0,   101,   102,     0,     0,     0,   163,   307,     0,
      42,    43,    36,   323,   324,     0,     0,     0,   104,   168,
       9,    10,    11,    12,    13,    14,    15,     0,     0,   107,
       0,     0,    44,   108,   109,   287,   288,     0,     0,   110,
     111,     0,    74,    74,    74,   112,   113,   114,     0,    74,
      74,   301,   334,   115,     0,     0,     0,     0,    36,   308,
       0,    74,    74,     0,   128,     0,   318,    74,     0,     0,
     136,   137,     0,     0,     0,     0,     0,   353,    44,     0,
       0,     0,     0,   153,   154,   155,     0,   158,   200,   201,
     202,   203,   204,   205,   206,   207,   208,   209,   361,   210,
     211,   212,   365,   366,     0,   368,   369,   370,     0,     0,
     371,     0,     0,   372,   308,     0,    74,   373,     0,     0,
      74,   376,   377,   378,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   380,     0,   243,     0,     0,   318,     0,
     180,    96,    97,     0,     2,    98,    99,   100,     0,     0,
       0,   187,   188,     0,   190,   191,     0,   192,   193,   194,
     195,     0,     0,     0,     0,     0,     0,   367,   199,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   209,     0,
     210,   211,   212,    74,     0,   230,     0,     0,   101,   102,
       0,     0,     0,     0,     0,    74,     0,     0,     0,     0,
     103,     0,     0,     0,   104,     0,     0,   105,     0,     0,
       0,   106,     0,     0,     0,   107,     0,     0,     0,   108,
     109,     0,     0,     0,   262,   110,   111,     0,     0,     0,
       0,   112,   113,   114,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   269,   270,   271,   272,   273,   274,   275,
     276,   277,   278,   279,   280,   281,   282,   283,   284,     0,
       0,   289,   290,   291,    96,    97,     0,     2,    98,    99,
     100,     0,     0,     0,     0,     9,    10,    11,    12,    13,
      14,    15,   202,   203,   204,   205,   206,   207,   208,   209,
       0,   210,   211,   212,     0,     0,     0,   166,     0,    96,
      97,     0,     2,    98,    99,   100,     0,     0,     0,     0,
       0,   101,   102,    36,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   167,   104,   168,     0,
       0,     0,     0,    44,     0,     0,     0,     0,   107,     0,
       0,     0,   108,   109,     0,     0,   101,   102,   110,   111,
       0,     0,     0,     0,   112,   113,   114,     0,     0,     0,
       0,   350,   104,   168,     0,   178,    96,    97,     0,     2,
      98,    99,   100,   107,     0,     0,     0,   108,   109,     0,
       0,   358,     0,   110,   111,     0,     0,     0,     0,   112,
     113,   114,     0,     0,     0,     0,     0,     0,     0,    96,
      97,     0,     2,    98,    99,   100,     0,     0,     0,     0,
       0,     0,     0,   101,   102,     0,     0,     0,    96,    97,
       0,     2,    98,    99,   100,     0,     0,     0,     0,   104,
     168,     0,     0,     0,  -120,     0,     0,     0,     0,     0,
     107,     0,     0,     0,   108,   109,   101,   102,     0,     0,
     110,   111,     0,     0,     0,     0,   112,   113,   114,     0,
       0,     0,   104,     0,     0,   101,   102,   229,     0,     0,
       0,     0,     0,   107,     0,     0,     0,   108,   109,     0,
       0,   104,   168,   110,   111,     0,     0,     0,     0,   112,
     113,   114,   107,     0,     0,     0,   108,   109,     0,     0,
      42,    43,   110,   111,     0,     0,     0,     0,   112,   113,
     114,    96,    97,     0,     2,    98,    99,   100,   197,   198,
     199,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,     0,   210,   211,   212,   264,   265,     0,     0,     0,
       0,   197,   198,   199,   200,   201,   202,   203,   204,   205,
     206,   207,   208,   209,     0,   210,   211,   212,   101,   102,
       0,   197,   198,   199,   200,   201,   202,   203,   204,   205,
     206,   207,   208,   209,   104,   210,   211,   212,     0,     0,
       0,   266,     0,     0,     0,   107,     0,     0,     0,   108,
     109,     0,     0,     0,     0,   110,   111,     0,    -4,     1,
       0,   112,   113,   114,     2,     0,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,     0,     0,     0,    21,    22,    23,
      24,    25,     0,    26,     0,    27,   161,    28,    29,    30,
      31,    32,     0,    33,    34,     0,    35,     0,     0,     0,
      36,    37,    38,    39,    40,    41,    42,    43,   198,   199,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
      44,   210,   211,   212,     0,     0,     0,     0,     0,     0,
       0,     0,     1,     0,     0,    45,    46,     2,     0,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    -7,    -7,    -7,
      21,    22,    23,    24,    25,    -7,    26,     0,    27,     0,
      28,    29,    30,    31,    32,    -7,    33,    34,     0,    35,
      -7,     0,     0,    36,    37,    38,    39,    40,    41,    42,
      43,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    44,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     1,     0,     0,    45,    46,
       2,     0,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      -8,    -8,    -8,    21,    22,    23,    24,    25,     0,    26,
       0,    27,     0,    28,    29,    30,    31,    32,     0,    33,
      34,     0,    35,     0,     0,     0,    36,    37,    38,    39,
      40,    41,    42,    43,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    44,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     1,     0,
       0,    45,    46,     2,     0,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    -8,     0,    -8,    21,    22,    23,    24,
      25,     0,    26,     0,    27,     0,    28,    29,    30,    31,
      32,     0,    33,    34,     0,    35,     0,     0,     0,    36,
      37,    38,    39,    40,    41,    42,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    44,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     1,     0,     0,    45,    46,     2,     0,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,     0,     0,     0,    21,
      22,    23,    24,    25,    -8,    26,     0,    27,     0,    28,
      29,    30,    31,    32,     0,    33,    34,     0,    35,     0,
       0,     0,    36,    37,    38,    39,    40,    41,    42,    43,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    44,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     1,     0,     0,    45,    46,     2,
       0,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,     0,
       0,     0,    21,    22,    23,    24,    25,     0,    26,     0,
      27,     0,    28,    29,    30,    31,    32,    -8,    33,    34,
       0,    35,     0,     0,     0,    36,    37,    38,    39,    40,
      41,    42,    43,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    44,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     1,     0,     0,
      45,    46,     2,     0,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,     0,     0,     0,    21,    22,    23,    24,    25,
       0,    26,     0,    27,     0,    28,    29,    30,    31,    32,
       0,    33,    34,     0,    35,    -8,     0,     0,    36,    37,
      38,    39,    40,    41,    42,    43,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    44,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       1,     0,     0,    45,    46,     2,     0,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,     0,     0,    -8,    21,    22,
      23,    24,    25,     0,    26,     0,    27,     0,    28,    29,
      30,    31,    32,     0,    33,    34,     0,    35,     0,     0,
       0,    36,    37,    38,    39,    40,    41,    42,    43,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    44,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     1,     0,     0,    45,    46,     2,     0,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,     0,     0,
       0,    21,    22,    23,    24,    25,     0,    26,     0,    27,
       0,    28,    29,    30,    31,    32,     0,    33,    34,     0,
      35,     0,     0,     0,    36,    37,    38,    39,    40,    41,
      42,    43,     0,     0,     1,     0,     0,     0,     0,     2,
       0,     0,     0,     0,    44,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    45,
      46,     0,    21,    22,    23,    24,    25,     0,    26,     0,
      27,     0,    28,    29,    30,    31,    32,     0,    33,    34,
       0,    35,     0,     0,     0,    36,    37,    38,    39,    40,
      41,    42,    43,     0,     0,     0,     0,     0,     0,     0,
     328,     0,     0,     0,     0,    44,   197,   198,   199,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   209,   331,
     210,   211,   212,     0,     0,   197,   198,   199,   200,   201,
     202,   203,   204,   205,   206,   207,   208,   209,   196,   210,
     211,   212,     0,   197,   198,   199,   200,   201,   202,   203,
     204,   205,   206,   207,   208,   209,     0,   210,   211,   212,
     197,   198,   199,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   209,     0,   210,   211,   212,     0,     0,     0,
     332,   197,   198,   199,   200,   201,   202,   203,   204,   205,
     206,   207,   208,   209,     0,   210,   211,   212
};

static const short int yycheck[] =
{
       1,    48,   101,   140,    28,    29,     4,     6,   297,    74,
      69,   244,     6,    58,    59,    16,    17,     3,     4,     6,
       6,     7,     8,     9,    58,    59,    71,   221,   222,   166,
     167,   168,    74,   227,   228,    61,    95,    71,    59,    56,
      22,    58,    59,    69,     6,   239,   240,   336,    30,     0,
      71,   245,    39,    47,    58,    59,    72,    56,    90,    91,
      92,    65,     0,    65,    50,    51,    58,    59,    71,    95,
       8,    72,    63,    64,    71,    59,    61,   142,    79,    80,
      66,    19,    20,   316,    82,    23,    59,    68,    26,    27,
      28,    29,   157,    31,    32,    33,    34,    35,    36,    71,
     142,    71,    40,    63,   298,    27,    44,    93,    94,    95,
      48,   135,    26,    87,    88,   157,    90,    91,    92,    34,
      60,    34,    49,   260,   125,    44,    44,   128,   129,   130,
      68,    64,   133,   134,    70,   136,   137,    28,   237,   140,
      28,    34,   143,    28,   145,   146,     0,   148,    68,   150,
     151,   316,   153,   154,   155,   313,   213,   158,   102,   213,
     161,   337,   326,   101,   102,    81,     6,   361,   233,   107,
     292,    -1,    -1,   174,    14,    15,    16,    17,    -1,   373,
     181,    -1,    -1,    -1,   249,    -1,    -1,    -1,    -1,   326,
      -1,   233,    -1,   292,    -1,     3,     4,   135,     6,     7,
       8,     9,    -1,   141,    -1,    -1,     6,   249,    -1,    -1,
      -1,   149,    52,    13,    14,    15,    16,    17,   156,    -1,
      85,    86,    87,    88,   225,    90,    91,    92,    -1,   230,
      -1,   232,    72,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     241,    -1,    50,    51,    -1,    -1,    -1,   294,    48,    -1,
      58,    59,    52,   254,   255,    -1,    -1,    -1,    66,    67,
      14,    15,    16,    17,    18,    19,    20,    -1,    -1,    77,
      -1,    -1,    72,    81,    82,   213,   214,    -1,    -1,    87,
      88,    -1,   220,   221,   222,    93,    94,    95,    -1,   227,
     228,   229,   293,     7,    -1,    -1,    -1,    -1,    52,   237,
      -1,   239,   240,    -1,    18,    -1,   244,   245,    -1,    -1,
      24,    25,    -1,    -1,    -1,    -1,    -1,   318,    72,    -1,
      -1,    -1,    -1,    37,    38,    39,    -1,    41,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,   339,    90,
      91,    92,   343,   344,    -1,   346,   347,   348,    -1,    -1,
     351,    -1,    -1,   354,   292,    -1,   294,   358,    -1,    -1,
     298,   362,   363,   364,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   374,    -1,   313,    -1,    -1,   316,    -1,
      94,     3,     4,    -1,     6,     7,     8,     9,    -1,    -1,
      -1,   105,   106,    -1,   108,   109,    -1,   111,   112,   113,
     114,    -1,    -1,    -1,    -1,    -1,    -1,   345,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    -1,
      90,    91,    92,   361,    -1,   139,    -1,    -1,    50,    51,
      -1,    -1,    -1,    -1,    -1,   373,    -1,    -1,    -1,    -1,
      62,    -1,    -1,    -1,    66,    -1,    -1,    69,    -1,    -1,
      -1,    73,    -1,    -1,    -1,    77,    -1,    -1,    -1,    81,
      82,    -1,    -1,    -1,   178,    87,    88,    -1,    -1,    -1,
      -1,    93,    94,    95,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   197,   198,   199,   200,   201,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,    -1,
      -1,   215,   216,   217,     3,     4,    -1,     6,     7,     8,
       9,    -1,    -1,    -1,    -1,    14,    15,    16,    17,    18,
      19,    20,    81,    82,    83,    84,    85,    86,    87,    88,
      -1,    90,    91,    92,    -1,    -1,    -1,    36,    -1,     3,
       4,    -1,     6,     7,     8,     9,    -1,    -1,    -1,    -1,
      -1,    50,    51,    52,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    66,    67,    -1,
      -1,    -1,    -1,    72,    -1,    -1,    -1,    -1,    77,    -1,
      -1,    -1,    81,    82,    -1,    -1,    50,    51,    87,    88,
      -1,    -1,    -1,    -1,    93,    94,    95,    -1,    -1,    -1,
      -1,   315,    66,    67,    -1,    69,     3,     4,    -1,     6,
       7,     8,     9,    77,    -1,    -1,    -1,    81,    82,    -1,
      -1,   335,    -1,    87,    88,    -1,    -1,    -1,    -1,    93,
      94,    95,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,
       4,    -1,     6,     7,     8,     9,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    50,    51,    -1,    -1,    -1,     3,     4,
      -1,     6,     7,     8,     9,    -1,    -1,    -1,    -1,    66,
      67,    -1,    -1,    -1,    71,    -1,    -1,    -1,    -1,    -1,
      77,    -1,    -1,    -1,    81,    82,    50,    51,    -1,    -1,
      87,    88,    -1,    -1,    -1,    -1,    93,    94,    95,    -1,
      -1,    -1,    66,    -1,    -1,    50,    51,    71,    -1,    -1,
      -1,    -1,    -1,    77,    -1,    -1,    -1,    81,    82,    -1,
      -1,    66,    67,    87,    88,    -1,    -1,    -1,    -1,    93,
      94,    95,    77,    -1,    -1,    -1,    81,    82,    -1,    -1,
      58,    59,    87,    88,    -1,    -1,    -1,    -1,    93,    94,
      95,     3,     4,    -1,     6,     7,     8,     9,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    -1,    90,    91,    92,    70,    71,    -1,    -1,    -1,
      -1,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    -1,    90,    91,    92,    50,    51,
      -1,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    66,    90,    91,    92,    -1,    -1,
      -1,    96,    -1,    -1,    -1,    77,    -1,    -1,    -1,    81,
      82,    -1,    -1,    -1,    -1,    87,    88,    -1,     0,     1,
      -1,    93,    94,    95,     6,    -1,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    -1,    -1,    -1,    29,    30,    31,
      32,    33,    -1,    35,    -1,    37,    38,    39,    40,    41,
      42,    43,    -1,    45,    46,    -1,    48,    -1,    -1,    -1,
      52,    53,    54,    55,    56,    57,    58,    59,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      72,    90,    91,    92,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     1,    -1,    -1,    87,    88,     6,    -1,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    -1,    37,    -1,
      39,    40,    41,    42,    43,    44,    45,    46,    -1,    48,
      49,    -1,    -1,    52,    53,    54,    55,    56,    57,    58,
      59,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    72,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     1,    -1,    -1,    87,    88,
       6,    -1,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    -1,    35,
      -1,    37,    -1,    39,    40,    41,    42,    43,    -1,    45,
      46,    -1,    48,    -1,    -1,    -1,    52,    53,    54,    55,
      56,    57,    58,    59,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    72,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     1,    -1,
      -1,    87,    88,     6,    -1,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    -1,    28,    29,    30,    31,    32,
      33,    -1,    35,    -1,    37,    -1,    39,    40,    41,    42,
      43,    -1,    45,    46,    -1,    48,    -1,    -1,    -1,    52,
      53,    54,    55,    56,    57,    58,    59,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    72,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     1,    -1,    -1,    87,    88,     6,    -1,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    -1,    -1,    -1,    29,
      30,    31,    32,    33,    34,    35,    -1,    37,    -1,    39,
      40,    41,    42,    43,    -1,    45,    46,    -1,    48,    -1,
      -1,    -1,    52,    53,    54,    55,    56,    57,    58,    59,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    72,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     1,    -1,    -1,    87,    88,     6,
      -1,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    -1,
      -1,    -1,    29,    30,    31,    32,    33,    -1,    35,    -1,
      37,    -1,    39,    40,    41,    42,    43,    44,    45,    46,
      -1,    48,    -1,    -1,    -1,    52,    53,    54,    55,    56,
      57,    58,    59,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    72,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     1,    -1,    -1,
      87,    88,     6,    -1,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    -1,    -1,    -1,    29,    30,    31,    32,    33,
      -1,    35,    -1,    37,    -1,    39,    40,    41,    42,    43,
      -1,    45,    46,    -1,    48,    49,    -1,    -1,    52,    53,
      54,    55,    56,    57,    58,    59,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    72,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       1,    -1,    -1,    87,    88,     6,    -1,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    -1,    -1,    28,    29,    30,
      31,    32,    33,    -1,    35,    -1,    37,    -1,    39,    40,
      41,    42,    43,    -1,    45,    46,    -1,    48,    -1,    -1,
      -1,    52,    53,    54,    55,    56,    57,    58,    59,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    72,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     1,    -1,    -1,    87,    88,     6,    -1,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    -1,    -1,
      -1,    29,    30,    31,    32,    33,    -1,    35,    -1,    37,
      -1,    39,    40,    41,    42,    43,    -1,    45,    46,    -1,
      48,    -1,    -1,    -1,    52,    53,    54,    55,    56,    57,
      58,    59,    -1,    -1,     1,    -1,    -1,    -1,    -1,     6,
      -1,    -1,    -1,    -1,    72,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    87,
      88,    -1,    29,    30,    31,    32,    33,    -1,    35,    -1,
      37,    -1,    39,    40,    41,    42,    43,    -1,    45,    46,
      -1,    48,    -1,    -1,    -1,    52,    53,    54,    55,    56,
      57,    58,    59,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      70,    -1,    -1,    -1,    -1,    72,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    70,
      90,    91,    92,    -1,    -1,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    71,    90,
      91,    92,    -1,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    -1,    90,    91,    92,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    -1,    90,    91,    92,    -1,    -1,    -1,
      96,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    -1,    90,    91,    92
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     1,     6,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    29,    30,    31,    32,    33,    35,    37,    39,    40,
      41,    42,    43,    45,    46,    48,    52,    53,    54,    55,
      56,    57,    58,    59,    72,    87,    88,    98,   100,   102,
     103,   104,   105,   106,   107,   108,   109,   112,   113,   114,
     115,   116,   117,   120,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   146,   147,   150,   155,   156,   157,   158,
     159,   160,   162,   163,   165,   167,   170,   171,   172,   173,
     174,   177,   178,   179,   180,   128,     3,     4,     7,     8,
       9,    50,    51,    62,    66,    69,    73,    77,    81,    82,
      87,    88,    93,    94,    95,   132,   133,   137,   138,   143,
     144,   145,   147,   149,   147,    56,   128,   128,   132,   147,
     147,     4,    82,   164,   164,   147,   132,   132,   147,   169,
     147,    56,   147,   169,   169,   164,   147,   147,   148,   147,
     147,   147,   147,   132,   132,   132,    39,   147,   132,   147,
       0,    38,    99,   102,   103,   128,    36,    65,    67,   132,
     134,   139,   159,   161,   168,   179,   128,   128,    69,   161,
     132,   136,   135,   147,   160,   137,   147,   132,   132,   147,
     132,   132,   132,   132,   132,   132,    71,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      90,    91,    92,    72,    61,    69,    95,    65,   110,   128,
     128,   128,   128,   128,   128,   166,   169,   128,   128,    71,
     132,   128,   134,   147,   159,   179,   128,    59,   128,   128,
     128,   121,   122,   147,   128,   128,   128,   128,   128,   147,
     159,   179,   128,   128,   134,   134,   134,   140,   141,   142,
      71,   128,   132,   128,    70,    71,    96,    63,    64,   132,
     132,   132,   132,   132,   132,   132,   132,   132,   132,   132,
     132,   132,   132,   132,   132,   133,   138,   147,   147,   132,
     132,   132,    59,   111,   100,   101,   101,   101,   128,   101,
     101,   147,   128,   128,   159,   179,    13,    48,   147,   160,
     176,   101,   101,    71,   128,    59,   118,   119,   147,   171,
     101,   159,   179,   128,   128,    68,    71,   134,    70,    71,
      63,    70,    96,   176,   128,    27,   151,   152,   153,    26,
     154,   154,   101,    34,    34,    60,   175,    44,    44,   122,
     132,    47,   119,   128,    49,   142,    64,    70,   132,   154,
     153,   128,    28,    28,    34,   128,   128,   147,   128,   128,
     128,   128,   128,   128,    28,   101,   128,   128,   128,   101,
     128
};

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

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
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");\
      YYERROR;							\
    }								\
while (0)


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (N)								\
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
    while (0)
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
} while (0)

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr, 					\
                  Type, Value, Location);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short int *bottom, short int *top)
#else
static void
yy_stack_print (bottom, top)
    short int *bottom;
    short int *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname [yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname [yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

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
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep, yylocationp)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
    YYLTYPE *yylocationp;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;
  (void) yylocationp;

  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  fprintf (yyoutput, ": ");

# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, yylocationp)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
    YYLTYPE *yylocationp;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;
  (void) yylocationp;

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
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
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
/* Location data for the look-ahead symbol.  */
YYLTYPE yylloc;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short int yyssa[YYINITDEPTH];
  short int *yyss = yyssa;
  register short int *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;

  /* The location stack.  */
  YYLTYPE yylsa[YYINITDEPTH];
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;
  /* The locations where the error started and ended. */
  YYLTYPE yyerror_range[2];

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

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
  yylsp = yyls;
#if YYLTYPE_IS_TRIVIAL
  /* Initialize the default location before parsing starts.  */
  yylloc.first_line   = yylloc.last_line   = 1;
  yylloc.first_column = yylloc.last_column = 0;
#endif


  yyvsp[0] = yylval;
    yylsp[0] = yylloc;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short int *yyss1 = yyss;
	YYLTYPE *yyls1 = yyls;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yyls1, yysize * sizeof (*yylsp),
		    &yystacksize);
	yyls = yyls1;
	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short int *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);
	YYSTACK_RELOCATE (yyls);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

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

/* Do appropriate processing given the current state.  */
/* Read a look-ahead token if we need one and don't already have one.  */
/* yyresume: */

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

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
  *++yylsp = yylloc;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
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

  /* Default location. */
  YYLLOC_DEFAULT (yyloc, yylsp - yylen, yylen);
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 123 "parser.y"
    { root_node = astnode_create_list((yyvsp[-1].node)); }
    break;

  case 3:
#line 127 "parser.y"
    { ; }
    break;

  case 5:
#line 132 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 6:
#line 133 "parser.y"
    {
         if ((yyvsp[-1].node) != NULL) { (yyval.node) = (yyvsp[-1].node); astnode_add_sibling((yyval.node), (yyvsp[0].node)); }
         else { (yyval.node) = (yyvsp[0].node); }
        }
    break;

  case 7:
#line 140 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 8:
#line 141 "parser.y"
    { (yyval.node) = NULL; }
    break;

  case 9:
#line 145 "parser.y"
    { (yyval.node) = (yyvsp[-1].node); astnode_add_sibling((yyval.node), (yyvsp[0].node)); }
    break;

  case 10:
#line 146 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 11:
#line 150 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 12:
#line 151 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 13:
#line 152 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 14:
#line 153 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 15:
#line 154 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 16:
#line 155 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 17:
#line 156 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 18:
#line 157 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 19:
#line 158 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 20:
#line 159 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 21:
#line 160 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 22:
#line 161 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 23:
#line 162 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 24:
#line 163 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 25:
#line 164 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 26:
#line 165 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 27:
#line 166 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 28:
#line 167 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 29:
#line 168 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 30:
#line 169 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 31:
#line 170 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 32:
#line 171 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 33:
#line 172 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 34:
#line 173 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 35:
#line 174 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 36:
#line 175 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 37:
#line 176 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 38:
#line 177 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 39:
#line 178 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 40:
#line 179 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 41:
#line 180 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 42:
#line 181 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 43:
#line 182 "parser.y"
    { (yyval.node) = NULL; }
    break;

  case 44:
#line 186 "parser.y"
    { (yyval.node) = astnode_create_org((yyvsp[-1].node), (yyloc)); }
    break;

  case 45:
#line 190 "parser.y"
    { (yyval.node) = astnode_create_align((yyvsp[-2].node), (yyvsp[-1].node), (yyloc)); }
    break;

  case 46:
#line 194 "parser.y"
    { (yyval.node) = astnode_create_warning((yyvsp[-1].node), (yyloc)); }
    break;

  case 47:
#line 198 "parser.y"
    { (yyval.node) = astnode_create_error((yyvsp[-1].node), (yyloc)); }
    break;

  case 48:
#line 202 "parser.y"
    { (yyval.node) = astnode_create_message((yyvsp[-1].node), (yyloc)); }
    break;

  case 49:
#line 206 "parser.y"
    { (yyval.node) = astnode_create_label((yyvsp[-3].node)->label, (yyvsp[-2].node), (yyvsp[-1].node), (yyloc)); astnode_finalize((yyvsp[-3].node)); }
    break;

  case 50:
#line 210 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 51:
#line 211 "parser.y"
    { (yyval.node) = NULL; }
    break;

  case 52:
#line 215 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 53:
#line 216 "parser.y"
    { (yyval.node) = NULL; }
    break;

  case 54:
#line 220 "parser.y"
    { (yyval.node) = astnode_create_while((yyvsp[-4].node), (yyvsp[-2].node), (yyloc)); }
    break;

  case 55:
#line 224 "parser.y"
    { (yyval.node) = astnode_create_rept((yyvsp[-4].node), (yyvsp[-2].node), (yyloc)); }
    break;

  case 56:
#line 228 "parser.y"
    { (yyval.node) = astnode_create_proc((yyvsp[-4].node), (yyvsp[-2].node), (yyloc)); }
    break;

  case 57:
#line 232 "parser.y"
    { (yyval.node) = astnode_create_struc_decl((yyvsp[-4].node), (yyvsp[-2].node), (yyloc)); }
    break;

  case 58:
#line 236 "parser.y"
    { (yyval.node) = astnode_create_union_decl((yyvsp[-4].node), (yyvsp[-2].node), (yyloc)); }
    break;

  case 59:
#line 240 "parser.y"
    { (yyval.node) = astnode_create_enum_decl((yyvsp[-4].node), (yyvsp[-2].node), (yyloc)); }
    break;

  case 60:
#line 244 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 61:
#line 245 "parser.y"
    { (yyval.node) = (yyvsp[-1].node); astnode_add_sibling((yyval.node), (yyvsp[0].node)); }
    break;

  case 62:
#line 249 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 63:
#line 250 "parser.y"
    { (yyval.node) = (yyvsp[-1].node); }
    break;

  case 64:
#line 254 "parser.y"
    { (yyval.node) = astnode_create_record_decl((yyvsp[-2].node), (yyvsp[-1].node), (yyloc)); }
    break;

  case 65:
#line 258 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 66:
#line 259 "parser.y"
    { (yyval.node) = (yyvsp[-2].node); astnode_add_sibling((yyval.node), (yyvsp[0].node)); }
    break;

  case 67:
#line 263 "parser.y"
    { (yyval.node) = astnode_create_bitfield_decl((yyvsp[-2].node), (yyvsp[0].node), (yyloc)); }
    break;

  case 68:
#line 267 "parser.y"
    { (yyval.node) = astnode_create_charmap((yyvsp[-1].node), (yyloc)); }
    break;

  case 69:
#line 271 "parser.y"
    { (yyval.node) = astnode_create_dataseg(0, (yyloc)); }
    break;

  case 70:
#line 272 "parser.y"
    { (yyval.node) = astnode_create_dataseg(ZEROPAGE_FLAG, (yyloc)); }
    break;

  case 71:
#line 275 "parser.y"
    { (yyval.node) = astnode_create_codeseg((yyloc)); }
    break;

  case 72:
#line 279 "parser.y"
    { (yyval.node) = NULL; }
    break;

  case 73:
#line 283 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 74:
#line 287 "parser.y"
    { ; }
    break;

  case 75:
#line 288 "parser.y"
    { ; }
    break;

  case 76:
#line 292 "parser.y"
    { ; }
    break;

  case 77:
#line 296 "parser.y"
    { (yyval.node) = (yyvsp[-1].node); }
    break;

  case 78:
#line 300 "parser.y"
    { (yyval.node) = astnode_create_instruction((yyvsp[0].mnemonic), IMPLIED_MODE, NULL, (yyloc)); }
    break;

  case 79:
#line 301 "parser.y"
    { (yyval.node) = astnode_create_instruction((yyvsp[-1].mnemonic), ACCUMULATOR_MODE, NULL, (yyloc)); }
    break;

  case 80:
#line 302 "parser.y"
    { (yyval.node) = astnode_create_instruction((yyvsp[-2].mnemonic), IMMEDIATE_MODE, (yyvsp[0].node), (yyloc)); }
    break;

  case 81:
#line 303 "parser.y"
    { (yyval.node) = astnode_create_instruction((yyvsp[-1].mnemonic), ABSOLUTE_MODE, (yyvsp[0].node), (yyloc)); }
    break;

  case 82:
#line 304 "parser.y"
    { (yyval.node) = astnode_create_instruction((yyvsp[-3].mnemonic), ABSOLUTE_X_MODE, (yyvsp[-2].node), (yyloc)); }
    break;

  case 83:
#line 305 "parser.y"
    { (yyval.node) = astnode_create_instruction((yyvsp[-3].mnemonic), ABSOLUTE_Y_MODE, (yyvsp[-2].node), (yyloc)); }
    break;

  case 84:
#line 306 "parser.y"
    { (yyval.node) = astnode_create_instruction((yyvsp[-5].mnemonic), PREINDEXED_INDIRECT_MODE, (yyvsp[-3].node), (yyloc)); }
    break;

  case 85:
#line 307 "parser.y"
    { (yyval.node) = astnode_create_instruction((yyvsp[-5].mnemonic), POSTINDEXED_INDIRECT_MODE, (yyvsp[-3].node), (yyloc)); }
    break;

  case 86:
#line 308 "parser.y"
    { (yyval.node) = astnode_create_instruction((yyvsp[-3].mnemonic), INDIRECT_MODE, (yyvsp[-1].node), (yyloc)); }
    break;

  case 87:
#line 312 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 88:
#line 313 "parser.y"
    { (yyval.node) = astnode_create_sizeof((yyvsp[0].node), (yyloc)); }
    break;

  case 89:
#line 314 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 90:
#line 315 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 91:
#line 316 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 92:
#line 317 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 93:
#line 318 "parser.y"
    { (yyval.node) = astnode_create_pc((yyloc)); }
    break;

  case 94:
#line 319 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 95:
#line 320 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 96:
#line 321 "parser.y"
    { (yyval.node) = (yyvsp[-1].node); }
    break;

  case 97:
#line 322 "parser.y"
    { (yyval.node) = astnode_create_forward_branch("+", (yyloc)); }
    break;

  case 98:
#line 323 "parser.y"
    { (yyval.node) = astnode_create_backward_branch("-", (yyloc)); }
    break;

  case 99:
#line 324 "parser.y"
    { (yyval.node) = astnode_create_forward_branch((yyvsp[0].ident), (yyloc)); }
    break;

  case 100:
#line 325 "parser.y"
    { (yyval.node) = astnode_create_backward_branch((yyvsp[0].ident), (yyloc)); }
    break;

  case 101:
#line 326 "parser.y"
    { (yyval.node) = astnode_create_mask((yyvsp[0].node), (yyloc)); }
    break;

  case 102:
#line 330 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 103:
#line 331 "parser.y"
    { (yyval.node) = astnode_create_index((yyvsp[-3].node), (yyvsp[-1].node), (yyloc)); }
    break;

  case 104:
#line 332 "parser.y"
    { (yyval.node) = astnode_create_index((yyvsp[-3].node), (yyvsp[-1].node), (yyloc)); }
    break;

  case 105:
#line 336 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 106:
#line 337 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 107:
#line 341 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 108:
#line 342 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 109:
#line 346 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 110:
#line 347 "parser.y"
    { (yyval.node) = NULL; }
    break;

  case 111:
#line 351 "parser.y"
    { (yyval.node) = astnode_create_scope((yyvsp[-2].node), (yyvsp[0].node), (yyloc)); }
    break;

  case 112:
#line 355 "parser.y"
    { (yyval.node) = astnode_create_dot((yyvsp[-2].node), (yyvsp[0].node), (yyloc)); }
    break;

  case 113:
#line 356 "parser.y"
    { (yyval.node) = astnode_create_dot((yyvsp[-2].node), (yyvsp[0].node), (yyloc)); }
    break;

  case 114:
#line 360 "parser.y"
    { (yyval.node) = astnode_create_struc((yyvsp[-1].node), (yyloc)); }
    break;

  case 115:
#line 364 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 116:
#line 365 "parser.y"
    { (yyval.node) = NULL; }
    break;

  case 117:
#line 369 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 118:
#line 370 "parser.y"
    { (yyval.node) = (yyvsp[-2].node); astnode_add_sibling((yyval.node), (yyvsp[0].node)); }
    break;

  case 119:
#line 374 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 120:
#line 375 "parser.y"
    { (yyval.node) = astnode_create_null((yyloc)); }
    break;

  case 121:
#line 379 "parser.y"
    { (yyval.node) = astnode_create_local_id((yyvsp[0].ident), (yyloc)); }
    break;

  case 122:
#line 383 "parser.y"
    { (yyval.node) = astnode_create_arithmetic(PLUS_OPERATOR, (yyvsp[-2].node), (yyvsp[0].node), (yyloc)); }
    break;

  case 123:
#line 384 "parser.y"
    { (yyval.node) = astnode_create_arithmetic(MINUS_OPERATOR, (yyvsp[-2].node), (yyvsp[0].node), (yyloc)); }
    break;

  case 124:
#line 385 "parser.y"
    { (yyval.node) = astnode_create_arithmetic(MUL_OPERATOR, (yyvsp[-2].node), (yyvsp[0].node), (yyloc)); }
    break;

  case 125:
#line 386 "parser.y"
    { (yyval.node) = astnode_create_arithmetic(DIV_OPERATOR, (yyvsp[-2].node), (yyvsp[0].node), (yyloc)); }
    break;

  case 126:
#line 387 "parser.y"
    { (yyval.node) = astnode_create_arithmetic(MOD_OPERATOR, (yyvsp[-2].node), (yyvsp[0].node), (yyloc)); }
    break;

  case 127:
#line 388 "parser.y"
    { (yyval.node) = astnode_create_arithmetic(AND_OPERATOR, (yyvsp[-2].node), (yyvsp[0].node), (yyloc)); }
    break;

  case 128:
#line 389 "parser.y"
    { (yyval.node) = astnode_create_arithmetic(OR_OPERATOR, (yyvsp[-2].node), (yyvsp[0].node), (yyloc)); }
    break;

  case 129:
#line 390 "parser.y"
    { (yyval.node) = astnode_create_arithmetic(XOR_OPERATOR, (yyvsp[-2].node), (yyvsp[0].node), (yyloc)); }
    break;

  case 130:
#line 391 "parser.y"
    { (yyval.node) = astnode_create_arithmetic(SHL_OPERATOR, (yyvsp[-2].node), (yyvsp[0].node), (yyloc)); }
    break;

  case 131:
#line 392 "parser.y"
    { (yyval.node) = astnode_create_arithmetic(SHR_OPERATOR, (yyvsp[-2].node), (yyvsp[0].node), (yyloc)); }
    break;

  case 132:
#line 393 "parser.y"
    { (yyval.node) = astnode_create_arithmetic(NEG_OPERATOR, (yyvsp[0].node), NULL, (yyloc)); }
    break;

  case 133:
#line 394 "parser.y"
    { (yyval.node) = astnode_create_arithmetic(NOT_OPERATOR, (yyvsp[0].node), NULL, (yyloc)); }
    break;

  case 134:
#line 395 "parser.y"
    { (yyval.node) = astnode_create_arithmetic(BANK_OPERATOR, (yyvsp[0].node), NULL, (yyloc)); }
    break;

  case 135:
#line 396 "parser.y"
    { (yyval.node) = astnode_create_arithmetic(LO_OPERATOR, (yyvsp[0].node), NULL, (yyloc)); }
    break;

  case 136:
#line 397 "parser.y"
    { (yyval.node) = astnode_create_arithmetic(HI_OPERATOR, (yyvsp[0].node), NULL, (yyloc)); }
    break;

  case 137:
#line 398 "parser.y"
    { (yyval.node) = astnode_create_arithmetic(UMINUS_OPERATOR, (yyvsp[0].node), NULL, (yyloc)); }
    break;

  case 138:
#line 402 "parser.y"
    { (yyval.node) = astnode_create_arithmetic(EQ_OPERATOR, (yyvsp[-2].node), (yyvsp[0].node), (yyloc)); }
    break;

  case 139:
#line 403 "parser.y"
    { (yyval.node) = astnode_create_arithmetic(NE_OPERATOR, (yyvsp[-2].node), (yyvsp[0].node), (yyloc)); }
    break;

  case 140:
#line 404 "parser.y"
    { (yyval.node) = astnode_create_arithmetic(GT_OPERATOR, (yyvsp[-2].node), (yyvsp[0].node), (yyloc)); }
    break;

  case 141:
#line 405 "parser.y"
    { (yyval.node) = astnode_create_arithmetic(LT_OPERATOR, (yyvsp[-2].node), (yyvsp[0].node), (yyloc)); }
    break;

  case 142:
#line 406 "parser.y"
    { (yyval.node) = astnode_create_arithmetic(GE_OPERATOR, (yyvsp[-2].node), (yyvsp[0].node), (yyloc)); }
    break;

  case 143:
#line 407 "parser.y"
    { (yyval.node) = astnode_create_arithmetic(LE_OPERATOR, (yyvsp[-2].node), (yyvsp[0].node), (yyloc)); }
    break;

  case 144:
#line 411 "parser.y"
    { (yyval.node) = astnode_create_label((yyvsp[0].label), NULL, NULL, (yyloc)); }
    break;

  case 145:
#line 412 "parser.y"
    { (yyval.node) = astnode_create_local_label((yyvsp[0].label), (yyloc)); }
    break;

  case 146:
#line 413 "parser.y"
    { (yyval.node) = astnode_create_forward_branch_decl("+", (yyloc)); }
    break;

  case 147:
#line 414 "parser.y"
    { (yyval.node) = astnode_create_backward_branch_decl("-", (yyloc)); }
    break;

  case 148:
#line 415 "parser.y"
    { (yyval.node) = astnode_create_forward_branch_decl((yyvsp[0].ident), (yyloc)); }
    break;

  case 149:
#line 416 "parser.y"
    { (yyval.node) = astnode_create_backward_branch_decl((yyvsp[0].ident), (yyloc)); }
    break;

  case 150:
#line 420 "parser.y"
    { (yyval.node) = astnode_create_identifier((yyvsp[0].ident), (yyloc)); }
    break;

  case 151:
#line 424 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 152:
#line 425 "parser.y"
    { (yyval.node) = astnode_create_null((yyloc)); }
    break;

  case 153:
#line 429 "parser.y"
    { (yyval.node) = astnode_create_integer((yyvsp[0].integer), (yyloc)); }
    break;

  case 154:
#line 430 "parser.y"
    { (yyval.node) = astnode_create_string((yyvsp[0].string), (yyloc)); }
    break;

  case 155:
#line 434 "parser.y"
    { (yyval.node) = astnode_create_if((yyvsp[-6].node), (yyvsp[-4].node), (yyvsp[-3].node), (yyvsp[-2].node), (yyloc)); }
    break;

  case 156:
#line 438 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 157:
#line 439 "parser.y"
    { (yyval.node) = NULL; }
    break;

  case 158:
#line 443 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 159:
#line 444 "parser.y"
    { (yyval.node) = (yyvsp[-1].node); astnode_add_sibling((yyval.node), (yyvsp[0].node)); }
    break;

  case 160:
#line 448 "parser.y"
    { (yyval.node) = astnode_create_case((yyvsp[-2].node), (yyvsp[0].node), (yyloc)); }
    break;

  case 161:
#line 452 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 162:
#line 453 "parser.y"
    { (yyval.node) = NULL; }
    break;

  case 163:
#line 457 "parser.y"
    { (yyval.node) = astnode_create_ifdef((yyvsp[-5].node), (yyvsp[-3].node), (yyvsp[-2].node), (yyloc)); }
    break;

  case 164:
#line 461 "parser.y"
    { (yyval.node) = astnode_create_ifndef((yyvsp[-5].node), (yyvsp[-3].node), (yyvsp[-2].node), (yyloc)); }
    break;

  case 165:
#line 465 "parser.y"
    { (yyval.node) = (yyvsp[-1].node); }
    break;

  case 166:
#line 466 "parser.y"
    { (yyval.node) = (yyvsp[-1].node); }
    break;

  case 167:
#line 470 "parser.y"
    { (yyval.node) = astnode_create_var_decl(0, (yyvsp[-1].node), (yyvsp[0].node), (yyloc)); }
    break;

  case 168:
#line 471 "parser.y"
    { (yyval.node) = astnode_create_var_decl(ZEROPAGE_FLAG, (yyvsp[-1].node), (yyvsp[0].node), (yyloc)); }
    break;

  case 169:
#line 472 "parser.y"
    { (yyval.node) = astnode_create_var_decl(PUBLIC_FLAG, (yyvsp[-1].node), (yyvsp[0].node), (yyloc)); }
    break;

  case 170:
#line 473 "parser.y"
    { (yyval.node) = astnode_create_var_decl(ZEROPAGE_FLAG | PUBLIC_FLAG, (yyvsp[-1].node), (yyvsp[0].node), (yyloc)); }
    break;

  case 171:
#line 474 "parser.y"
    { (yyval.node) = astnode_create_var_decl(PUBLIC_FLAG | ZEROPAGE_FLAG, (yyvsp[-1].node), (yyvsp[0].node), (yyloc)); }
    break;

  case 172:
#line 478 "parser.y"
    { (yyval.node) = astnode_create_data((yyvsp[-1].node), (yyvsp[0].node), (yyloc)); }
    break;

  case 173:
#line 479 "parser.y"
    { (yyval.node) = astnode_create_storage((yyvsp[0].node), NULL, (yyloc)); }
    break;

  case 174:
#line 480 "parser.y"
    { (yyval.node) = astnode_create_storage((yyvsp[-3].node), (yyvsp[-1].node), (yyloc)); }
    break;

  case 175:
#line 484 "parser.y"
    { (yyval.node) = astnode_create_datatype(BYTE_DATATYPE, NULL, (yyloc)); }
    break;

  case 176:
#line 485 "parser.y"
    { (yyval.node) = astnode_create_datatype(CHAR_DATATYPE, NULL, (yyloc)); }
    break;

  case 177:
#line 486 "parser.y"
    { (yyval.node) = astnode_create_datatype(WORD_DATATYPE, NULL, (yyloc)); }
    break;

  case 178:
#line 487 "parser.y"
    { (yyval.node) = astnode_create_datatype(DWORD_DATATYPE, NULL, (yyloc)); }
    break;

  case 179:
#line 488 "parser.y"
    { (yyval.node) = astnode_create_datatype(USER_DATATYPE, (yyvsp[0].node), (yyloc)); }
    break;

  case 180:
#line 489 "parser.y"
    { (yyval.node) = astnode_create_datatype(USER_DATATYPE, (yyvsp[0].node), (yyloc)); }
    break;

  case 181:
#line 493 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 182:
#line 494 "parser.y"
    { (yyval.node) = (yyvsp[-2].node); astnode_add_sibling((yyval.node), (yyvsp[0].node)); }
    break;

  case 183:
#line 498 "parser.y"
    { (yyval.node) = astnode_create_incsrc((yyvsp[-1].node), (yyloc)); handle_incsrc((yyval.node)); }
    break;

  case 184:
#line 502 "parser.y"
    { (yyval.node) = astnode_create_incbin((yyvsp[-1].node), (yyloc)); handle_incbin((yyval.node)); }
    break;

  case 185:
#line 506 "parser.y"
    { (yyval.node) = astnode_create_string((yyvsp[0].string), (yyloc)); }
    break;

  case 186:
#line 507 "parser.y"
    { (yyval.node) = astnode_create_file_path(scan_include('>'), (yyloc)); }
    break;

  case 187:
#line 511 "parser.y"
    { (yyval.node) = astnode_create_macro_decl((yyvsp[-5].node), (yyvsp[-4].node), (yyvsp[-2].node), (yyloc)); }
    break;

  case 188:
#line 515 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 189:
#line 516 "parser.y"
    { (yyval.node) = NULL; }
    break;

  case 190:
#line 520 "parser.y"
    { (yyval.node) = astnode_create_macro((yyvsp[-2].node), (yyvsp[-1].node), (yyloc)); }
    break;

  case 191:
#line 524 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 192:
#line 525 "parser.y"
    { (yyval.node) = NULL; }
    break;

  case 193:
#line 529 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 194:
#line 530 "parser.y"
    { (yyval.node) = (yyvsp[-2].node); astnode_add_sibling((yyval.node), (yyvsp[0].node)); }
    break;

  case 195:
#line 534 "parser.y"
    { (yyval.node) = astnode_create_equ((yyvsp[-3].node), (yyvsp[-1].node), (yyloc)); }
    break;

  case 196:
#line 538 "parser.y"
    { (yyval.node) = astnode_create_assign((yyvsp[-3].node), (yyvsp[-1].node), (yyloc)); }
    break;

  case 197:
#line 542 "parser.y"
    { (yyval.node) = astnode_create_equ((yyvsp[-1].node), astnode_create_integer(0, (yyloc)), (yyloc)); }
    break;

  case 198:
#line 543 "parser.y"
    { (yyval.node) = astnode_create_equ((yyvsp[-2].node), (yyvsp[-1].node), (yyloc)); }
    break;

  case 199:
#line 547 "parser.y"
    { (yyval.node) = astnode_create_public((yyvsp[-1].node), (yyloc)); }
    break;

  case 200:
#line 551 "parser.y"
    { (yyval.node) = astnode_create_extrn((yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[-1].node), (yyloc)); }
    break;

  case 201:
#line 555 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 202:
#line 556 "parser.y"
    { (yyval.node) = NULL; }
    break;

  case 203:
#line 560 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 204:
#line 561 "parser.y"
    { (yyval.node) = astnode_create_datatype(USER_DATATYPE, (yyvsp[0].node), (yyloc)); }
    break;

  case 205:
#line 562 "parser.y"
    { (yyval.node) = astnode_create_integer(PROC_SYMBOL, (yyloc)); }
    break;

  case 206:
#line 563 "parser.y"
    { (yyval.node) = astnode_create_integer(LABEL_SYMBOL, (yyloc)); }
    break;

  case 207:
#line 567 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 208:
#line 568 "parser.y"
    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 209:
#line 572 "parser.y"
    { (yyval.node) = astnode_create_var_decl(0, (yyvsp[-1].node), (yyvsp[0].node), (yyloc)); }
    break;

  case 210:
#line 573 "parser.y"
    { (yyval.node) = astnode_create_var_decl(ZEROPAGE_FLAG, (yyvsp[-1].node), (yyvsp[0].node), (yyloc)); }
    break;

  case 211:
#line 574 "parser.y"
    { (yyval.node) = astnode_create_var_decl(PUBLIC_FLAG, (yyvsp[-1].node), (yyvsp[0].node), (yyloc)); }
    break;

  case 212:
#line 575 "parser.y"
    { (yyval.node) = astnode_create_var_decl(ZEROPAGE_FLAG | PUBLIC_FLAG, (yyvsp[-1].node), (yyvsp[0].node), (yyloc)); }
    break;

  case 213:
#line 576 "parser.y"
    { (yyval.node) = astnode_create_var_decl(PUBLIC_FLAG | ZEROPAGE_FLAG, (yyvsp[-1].node), (yyvsp[0].node), (yyloc)); }
    break;

  case 214:
#line 580 "parser.y"
    { (yyval.node) = astnode_create_storage((yyvsp[-2].node), (yyvsp[-1].node), (yyloc)); }
    break;

  case 215:
#line 584 "parser.y"
    { (yyval.node) = astnode_create_datatype(BYTE_DATATYPE, NULL, (yyloc)); }
    break;

  case 216:
#line 585 "parser.y"
    { (yyval.node) = astnode_create_datatype(WORD_DATATYPE, NULL, (yyloc)); }
    break;

  case 217:
#line 586 "parser.y"
    { (yyval.node) = astnode_create_datatype(DWORD_DATATYPE, NULL, (yyloc)); }
    break;


    }

/* Line 1037 of yacc.c.  */
#line 2982 "parser.c"

  yyvsp -= yylen;
  yyssp -= yylen;
  yylsp -= yylen;

  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;
  *++yylsp = yyloc;

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
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  int yytype = YYTRANSLATE (yychar);
	  const char* yyprefix;
	  char *yymsg;
	  int yyx;

	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  int yyxbegin = yyn < 0 ? -yyn : 0;

	  /* Stay within bounds of both yycheck and yytname.  */
	  int yychecklim = YYLAST - yyn;
	  int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
	  int yycount = 0;

	  yyprefix = ", expecting ";
	  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      {
		yysize += yystrlen (yyprefix) + yystrlen (yytname [yyx]);
		yycount += 1;
		if (yycount == 5)
		  {
		    yysize = 0;
		    break;
		  }
	      }
	  yysize += (sizeof ("syntax error, unexpected ")
		     + yystrlen (yytname[yytype]));
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yyprefix = ", expecting ";
		  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			yyp = yystpcpy (yyp, yyprefix);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yyprefix = " or ";
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("syntax error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror ("syntax error");
    }

  yyerror_range[0] = yylloc;

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* If at end of input, pop the error token,
	     then the rest of the stack, then return failure.  */
	  if (yychar == YYEOF)
	     for (;;)
	       {
                 yyerror_range[0] = *yylsp;
		 YYPOPSTACK;
		 if (yyssp == yyss)
		   YYABORT;
		 yydestruct ("Error: popping",
                             yystos[*yyssp], yyvsp, yylsp);
	       }
        }
      else
	{
	  yydestruct ("Error: discarding", yytoken, &yylval, &yylloc);
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

#ifdef __GNUC__
  /* Pacify GCC when the user code never invokes YYERROR and the label
     yyerrorlab therefore never appears in user code.  */
  if (0)
     goto yyerrorlab;
#endif

  yyerror_range[0] = yylsp[1-yylen];
  yylsp -= yylen;
  yyvsp -= yylen;
  yyssp -= yylen;
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

      yyerror_range[0] = *yylsp;
      yydestruct ("Error: popping", yystos[yystate], yyvsp, yylsp);
      YYPOPSTACK;
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;

  yyerror_range[1] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the look-ahead.  YYLOC is available though. */
  YYLLOC_DEFAULT (yyloc, yyerror_range - 1, 2);
  *++yylsp = yyloc;

  /* Shift the error token. */
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
  yydestruct ("Error: discarding lookahead",
              yytoken, &yylval, &yylloc);
  yychar = YYEMPTY;
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 588 "parser.y"

/**
 * Takes care of switching to a new scanner input stream when a "incsrc" statement
 * has been encountered.
 * The current stream is pushed on a stack, and will be popped when EOF is reached
 * in the new stream.
 * @param n A node of type INCSRC_NODE
 */
void handle_incsrc(astnode *n)
{
    char errs[512];
    /* Get the node which describes the file to include */
    astnode *file = astnode_get_child(n, 0);
    int quoted_form = (astnode_get_type(file) == STRING_NODE) ? 1 : 0;
    switch (yypushandrestart(file->string, quoted_form)) {
        case 0:
        /* Success */
        break;
        case 1:
        /* Failed to open file */
        sprintf(errs, "could not open '%s' for reading", file->string);
        yyerror(errs);
        break;
        case 2:
        /* Stack overflow */
        yyerror("Maximum include nesting level reached");
        break;
    }
}

// TODO: This shouldn't be done here but rather in astproc module.

FILE *open_included_file(const char *, int, char **);

/**
 * Takes care of including the binary contents of the file specified by a parsed
 * "incbin" statement.
 * Calls yyerror() if the file can't be included for some reason.
 * @param n A node of type INCBIN_NODE
 */
void handle_incbin(astnode *n)
{
    FILE *fp;
    unsigned char *data;
    int size;
    char errs[512];
    /* Get the node which describes the file to include */
    astnode *file = astnode_get_child(n, 0);
    const char *filename = file->string;
    int quoted_form = (astnode_get_type(file) == STRING_NODE) ? 1 : 0;
    /* Try to open it */
    fp = open_included_file(filename, quoted_form, NULL);
    if (fp) {
        /* Get filesize */
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        rewind(fp);
        if (size > 0) {
            /* Allocate buffer to hold file contents */
            data = (unsigned char *)malloc(size);
            /* Read file contents into buffer */
            fread(data, 1, size, fp);
            /* Insert binary node */
            astnode_add_sibling(n, astnode_create_binary(data, size, n->loc) );
        }
        /* Close file */
        fclose(fp);
    } else {
        /* Couldn't open file */
        sprintf(errs, "could not open '%s' for reading", file->string);
        yyerror(errs);
    }
}

