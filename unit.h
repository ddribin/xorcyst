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

#ifndef XASM_UNIT_H
#define XASM_UNIT_H

/*---------------------------------------------------------------------------*/
/* Data structures. */

enum tag_xasm_constant_type {
    XASM_INTEGER_CONSTANT=0,
    XASM_STRING_CONSTANT
};

typedef enum tag_xasm_constant_type xasm_constant_type;

/**
 * Describes a constant exported from unit.
 */
struct tag_xasm_constant {
    xasm_constant_type type; /* *_CONSTANT */
    char *name;
    union {
        long integer;   /* type == INTEGER_CONSTANT */
        char *string;   /* type == STRING_CONSTANT */
    };
    struct tag_xasm_unit *unit;  /* Owner unit */
};

typedef struct tag_xasm_constant xasm_constant;

/**
 * Describes a symbol located in another unit that this unit references.
 */
struct tag_xasm_external
{
    unsigned char unit; /* Unit to import from (0=any unit) (currently not used) */
    char *name;
    struct tag_xasm_unit *from;  /* Unit exported from */
};

typedef struct tag_xasm_external xasm_external;

/* The possible kinds of expression */
enum tag_xasm_expr_type
{
    XASM_OPERATOR_EXPRESSION=0,
    XASM_INTEGER_EXPRESSION,
    XASM_STRING_EXPRESSION,
    XASM_LOCAL_EXPRESSION,
    XASM_EXTERNAL_EXPRESSION,
    XASM_PC_EXPRESSION
};

typedef enum tag_xasm_expr_type xasm_expr_type;

/* Attributes of operator expression */
struct tag_xasm_operator_expr
{
    int operator;   /* See bytecode header for valid values */
    struct tag_xasm_expression *lhs;
    struct tag_xasm_expression *rhs;
};

typedef struct tag_xasm_operator_expr xasm_operator_expr;

/** Describes an expression. */
struct tag_xasm_expression
{
    xasm_expr_type type;
    union {
        xasm_operator_expr op_expr;  /* type == OPERATOR_EXPRESSION */
        unsigned long integer;  /* type == INTEGER_EXPRESSION */
        char *string;       /* type == STRING_EXPRESSION */
        int local_id;       /* type == LOCAL_EXPRESSION */
        int extrn_id;       /* type == EXTERNAL_EXPRESSION */
    };
    struct tag_xasm_unit *unit;  /* Owner unit */
};

typedef struct tag_xasm_expression xasm_expression;

/** The possible kinds of segment. */
enum tag_xasm_segment_type {
    XASM_DATA_SEGMENT=0,
    XASM_CODE_SEGMENT
};

typedef enum tag_xasm_segment_type xasm_segment_type;

/**
 * A segment of a unit.
 */
struct tag_xasm_segment {
    int size;   /* Size in bytes */
    unsigned char *bytes;
};

typedef struct tag_xasm_segment xasm_segment;

/**
 * Describes a unit.
 */
struct tag_xasm_unit {
    const char *name;     /* Name of unit */
    xasm_constant *constants;    /* Array of exported constants */
    int const_count;    /* Number of exported constants */
    xasm_external *externals;    /* Array of imported symbols */
    int ext_count;      /* Number of imported symbols */
    xasm_expression **expressions;   /* Array of expressions */
    int expr_count;     /* Number of expressions */
    xasm_segment dataseg;    /* The data segment */
    xasm_segment codeseg;    /* The code segment */
};

typedef struct tag_xasm_unit xasm_unit;

/** Signature for procedure to process a bytecode */
typedef void (*xasm_bytecodeproc)(const unsigned char *, void *);

/*---------------------------------------------------------------------------*/
/* Function prototypes. */

int xasm_unit_read(const char *, xasm_unit *);
void xasm_unit_finalize(xasm_unit *);
const char *xasm_operator_to_string(int op);

#endif  /* !UNIT_H */
