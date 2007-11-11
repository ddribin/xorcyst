/*
 * $Id: unit.h,v 1.3 2007/07/22 13:35:20 khansen Exp $
 * $Log: unit.h,v $
 * Revision 1.3  2007/07/22 13:35:20  khansen
 * convert tabs to whitespaces
 *
 * Revision 1.2  2004/12/16 13:21:07  kenth
 * added unit parent pointer to some structs
 *
 * Revision 1.1  2004/06/30 07:56:49  kenth
 * Initial revision
 *
 */

/**
 *    (C) 2004 Kent Hansen
 *
 *    The XORcyst is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    The XORcyst is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with The XORcyst; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef UNIT_H
#define UNIT_H

/*---------------------------------------------------------------------------*/
/* Data structures. */

enum tag_constant_type {
    INTEGER_CONSTANT=0,
    STRING_CONSTANT
};

typedef enum tag_constant_type constant_type;

/**
 * Describes a constant exported from unit.
 */
struct tag_constant {
    constant_type type; /* *_CONSTANT */
    char *name;
    union {
        long integer;   /* type == INTEGER_CONSTANT */
        char *string;   /* type == STRING_CONSTANT */
    };
    struct tag_unit *unit;  /* Owner unit */
};

typedef struct tag_constant constant;

/**
 * Describes a symbol located in another unit that this unit references.
 */
struct tag_external
{
    unsigned char unit;
    char *name;
    struct tag_unit *from;  /* Unit exported from */
};

typedef struct tag_external external;

/* The possible kinds of expression */
enum tag_expr_type
{
    OPERATOR_EXPRESSION=0,
    INTEGER_EXPRESSION,
    STRING_EXPRESSION,
    LOCAL_EXPRESSION,
    EXTERNAL_EXPRESSION,
    PC_EXPRESSION
};

typedef enum tag_expr_type expr_type;

/* Attributes of operator expression */
struct tag_operator_expr
{
    int operator;   /* See bytecode header for valid values */
    struct tag_expression *lhs;
    struct tag_expression *rhs;
};

typedef struct tag_operator_expr operator_expr;

/** Describes an expression. */
struct tag_expression
{
    expr_type type;
    union {
        operator_expr op_expr;  /* type == OPERATOR_EXPRESSION */
        unsigned long integer;  /* type == INTEGER_EXPRESSION */
        char *string;       /* type == STRING_EXPRESSION */
        int local_id;       /* type == LOCAL_EXPRESSION */
        int extrn_id;       /* type == EXTERNAL_EXPRESSION */
    };
    struct tag_unit *unit;  /* Owner unit */
};

typedef struct tag_expression expression;

/** The possible kinds of segment. */
enum tag_segment_type {
    DATA_SEGMENT=0,
    CODE_SEGMENT
};

typedef enum tag_segment_type segment_type;

/**
 * A segment of a unit.
 */
struct tag_segment {
    int size;   /* Size in bytes */
    unsigned char *bytes;
};

typedef struct tag_segment segment;

/**
 * Describes a unit.
 */
struct tag_unit {
    char *name;     /* Name of unit */
    constant *constants;    /* Array of exported constants */
    int const_count;    /* Number of exported constants */
    external *externals;    /* Array of imported symbols */
    int ext_count;      /* Number of imported symbols */
    expression **expressions;   /* Array of expressions */
    int expr_count;     /* Number of expressions */
    segment dataseg;    /* The data segment */
    segment codeseg;    /* The code segment */
};

typedef struct tag_unit unit;

/*---------------------------------------------------------------------------*/
/* Function prototypes. */

int unit_read(char *, unit *);
void unit_finalize(unit *);

#endif  /* !UNIT_H */
