/*
 * $Id: loc.h,v 1.3 2007/08/10 20:21:26 khansen Exp $
 * $Log: loc.h,v $
 * Revision 1.3  2007/08/10 20:21:26  khansen
 * *** empty log message ***
 *
 * Revision 1.2  2007/07/22 13:35:20  khansen
 * convert tabs to whitespaces
 *
 * Revision 1.1  2004/06/30 07:56:29  kenth
 * Initial revision
 *
 */

/**
 * Definitions for flex location tracking.
 */

#ifndef LOC_H
#define LOC_H

#if ! defined (YYLTYPE) && ! defined (YYLTYPE_IS_DECLARED)
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
  const char *file;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif

#ifndef YYRHSLOC
# define YYRHSLOC(Rhs, K) ((Rhs)[K])
#endif
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)         \
    do									\
      if (N)								\
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
          (Current).file         = YYRHSLOC (Rhs, 1).file;              \
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
          (Current).file         = YYRHSLOC (Rhs, 0).file;              \
	}								\
    while (0)
#endif

const char *yy_current_filename();

typedef struct YYLTYPE location;

/* Initialize LOC. */
     # define LOCATION_RESET(Loc)                  \
       (Loc).first_column = (Loc).first_line = 1;  \
       (Loc).last_column =  (Loc).last_line = 1; \
       (Loc).file = yy_current_filename();

     /* Advance of NUM lines. */
     # define LOCATION_LINES(Loc, Num)             \
       (Loc).last_column = 1;                      \
       (Loc).last_line += Num; \
       (Loc).file = (Loc).file;

     /* Restart: move the first cursor to the last position. */
     # define LOCATION_STEP(Loc)                   \
       (Loc).first_column = (Loc).last_column;     \
       (Loc).first_line = (Loc).last_line; \
       (Loc).file = (Loc).file;

     /* Output LOC on the stream OUT. */
     # define LOCATION_PRINT(Out, Loc)                               \
         fprintf (Out, "%d", (Loc).first_line)
    /*
     # define LOCATION_PRINT(Out, Loc)                               \
       if ((Loc).first_line != (Loc).last_line)                      \
         fprintf (Out, "%d.%d-%d.%d",                                \
                  (Loc).first_line, (Loc).first_column,              \
                  (Loc).last_line, (Loc).last_column - 1);           \
       else if ((Loc).first_column < (Loc).last_column - 1)          \
         fprintf (Out, "%d.%d-%d", (Loc).first_line,                 \
                  (Loc).first_column, (Loc).last_column - 1);        \
       else                                                          \
         fprintf (Out, "%d.%d", (Loc).first_line, (Loc).first_column)
    */

#endif  /* !LOC_H */
