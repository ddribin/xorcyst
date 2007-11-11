/* A Bison parser, made by GNU Bison 1.875.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software Foundation, Inc.

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




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 65 "parser.y"
typedef union YYSTYPE {
    long integer;
    int mnemonic;
    const char *string;
    const char *label;
    const char *ident;
    astnode *node;
} YYSTYPE;
/* Line 1248 of yacc.c.  */
#line 179 "parser.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

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

extern YYLTYPE yylloc;


