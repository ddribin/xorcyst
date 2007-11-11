/*
 * $Id: unit.c,v 1.4 2007/07/22 13:33:26 khansen Exp $
 * $Log: unit.c,v $
 * Revision 1.4  2007/07/22 13:33:26  khansen
 * convert tabs to whitespaces
 *
 * Revision 1.3  2004/12/18 17:00:35  kenth
 * improved error reporting slightly
 *
 * Revision 1.2  2004/12/16 13:20:07  kenth
 * xorcyst 1.3.5
 *
 * Revision 1.1  2004/06/30 07:56:00  kenth
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

#include <stdio.h>
#include <stdlib.h>
#include "unit.h"
#include "objdef.h"

#define SAFE_FREE(m) if ((m) != NULL) { free(m); m = NULL; }

/*---------------------------------------------------------------------------*/

/* Reads a byte */
#define get_1(f) (unsigned char)fgetc(fp)
/* Reads a short (big-endian) */
static unsigned short get_2(FILE *fp)
{
    unsigned short result;
    result = get_1(fp) << 8;    /* High byte */
    result |= get_1(fp);        /* Low byte */
    return result;
}
/* Reads a 24-bit integer (big-endian) */
static unsigned int get_3(FILE *fp)
{
    unsigned int result;
    result = get_2(fp) << 8;    /* High 16 bits */
    result |= get_1(fp);        /* Low byte */
    return result;
}
/* Reads an int (big-endian) */
static unsigned int get_4(FILE *fp)
{
    unsigned int result;
    /* Assumes little-endian machine!! */
    result = get_2(fp) << 16;   /* High 16 bits */
    result |= get_2(fp);        /* Low 16 bits */
    return result;
}
/* Reads a string prepended by 8-bit length */
static char *get_str_8(FILE *fp)
{
    char *s;
    /* Read length */
    int len = get_1(fp) + 1;
    /* Allocate space for string */
    s = (char *)malloc(len + 1);
    if (s != NULL) {
        /* Read string from file */
        fread(s, 1, len, fp);
        /* 0-terminate string */
        s[len] = '\0';
    }
    /* Return the new string */
    return s;
}
/* Reads a string prepended by 16-bit length */
static char *get_str_16(FILE *fp)
{
    char *s;
    /* Read length */
    int len = get_2(fp) + 1;
    /* Allocate space for string */
    s = (char *)malloc(len + 1);
    if (s != NULL) {
        /* Read string from file */
        fread(s, 1, len, fp);
    }
    /* Return the new string */
    return s;
}
#define get_bytes_8(f) (unsigned char *)get_str_8(f)
#define get_bytes_16(f) (unsigned char *)get_str_16(f)

/*--------------------------------------------------------------------------*/

/**
 * Reads a constant from file.
 * @param fp File handle
 * @param u Unit in which constant is loaded
 * @param i Index of constant in unit's constant array
 */
static void get_const(FILE *fp, unit *u, int i)
{
    constant *cnst = &u->constants[i];
    /* Read name */
    cnst->name = get_str_8(fp);
    /* Read type */
    cnst->type = get_1(fp);
    /* Read value */
    switch (cnst->type) {
        case INT_8: cnst->integer = get_1(fp);  cnst->type = INTEGER_CONSTANT;  break;
        case INT_16:    cnst->integer = get_2(fp);  cnst->type = INTEGER_CONSTANT;  break;
        case INT_24:    cnst->integer = get_3(fp);  cnst->type = INTEGER_CONSTANT;  break;
        case INT_32:    cnst->integer = get_4(fp);  cnst->type = INTEGER_CONSTANT;  break;
        case STR_8: cnst->string = get_str_8(fp);   cnst->type = STRING_CONSTANT;   break;
        case STR_16:    cnst->string = get_str_16(fp);  cnst->type = STRING_CONSTANT;   break;

        default:
        /* Error, invalid type */
        fprintf(stderr, "%s(0x%lx): get_const(): bad constant type (%.2X)\n", u->name, ftell(fp), cnst->type);
        break;
    }
    /* Set owner */
    cnst->unit = u;
}

/**
 * Reads constant array from file.
 * @param fp File handle
 * @param u Unit whose constants array will be populated
 */
static void get_constants(FILE *fp, unit *u)
{
    int i;
    /* Read the number of constants */
    int count = get_2(fp);
    /* Allocate the list */
    if (count > 0) {
        u->constants = (constant *)malloc(sizeof(constant) * count);
    }
    else {
        u->constants = NULL;
    }
    /* Read constants */
    for (i=0; i<count; i++) {
        get_const(fp, u, i);
    }
    /* Set count field */
    u->const_count = count;
}

/**
 * Reads imported symbol from file.
 * @param fp File handle
 * @param u Unit
 * @param i External index
 */
static void get_ext(FILE *fp, unit *u, int i)
{
    external *ext = &u->externals[i];
    /* Read unit # */
    ext->unit = get_1(fp);
    /* Read name */
    ext->name = get_str_8(fp);
    /* Set owner */
    ext->from = u;
}

/**
 * Reads imported symbol array from file.
 * @param fp File handle
 * @param u Unit whose externals array will be populated
 */
static void get_externals(FILE *fp, unit *u)
{
    int i;
    int count;
    /* Read the number of imported symbols */
    count = get_2(fp);
    /* Allocate the list */
    if (count > 0) {
        u->externals = (external *)malloc(sizeof(external) * count);
    }
    else {
        u->externals = NULL;
    }
    /* Read imported symbols */
    for (i=0; i<count; i++) {
        get_ext(fp, u, i);
    }
    /* Set count field */
    u->ext_count = count;
}

/**
 * Reads an expression from file.
 * @param fp File handle
 * @param dest Pointer to pointer where expression should be stored
 * @param u Owner unit
 */
static void get_expr_recursive(FILE *fp, expression **dest, unit *u)
{
    unsigned char type;
    /* Allocate space for expression */
    expression *exp = (expression *)malloc( sizeof(expression) );
    if (exp != NULL) {
        /* Read expression type */
        type = get_1(fp);
        /* Read the value */
        switch (type) {
            /* Literals */
            case INT_8: exp->integer = get_1(fp);   exp->type = INTEGER_EXPRESSION; break;
            case INT_16:    exp->integer = get_2(fp);   exp->type = INTEGER_EXPRESSION; break;
            case INT_24:    exp->integer = get_3(fp);   exp->type = INTEGER_EXPRESSION; break;
            case INT_32:    exp->integer = get_4(fp);   exp->type = INTEGER_EXPRESSION; break;
            case STR_8: exp->string = get_str_8(fp);    exp->type = STRING_EXPRESSION;  break;
            case STR_16:    exp->string = get_str_16(fp);   exp->type = STRING_EXPRESSION;  break;

            /* Identifiers */
            case LOCAL: exp->local_id = get_2(fp);  exp->type = LOCAL_EXPRESSION;   break;
            case EXTRN: exp->extrn_id = get_2(fp);  exp->type = EXTERNAL_EXPRESSION;break;

            /* Current address */
            case PC:    ;               exp->type = PC_EXPRESSION;  break;

            /* Arithmetic, binary */
            case OP_PLUS:
            case OP_MINUS:
            case OP_MUL:
            case OP_DIV:
            case OP_MOD:
            case OP_SHL:
            case OP_SHR:
            case OP_AND:
            case OP_OR:
            case OP_XOR:
            case OP_EQ:
            case OP_NE:
            case OP_LT:
            case OP_GT:
            case OP_LE:
            case OP_GE:
            get_expr_recursive(fp, &exp->op_expr.lhs, u);
            get_expr_recursive(fp, &exp->op_expr.rhs, u);
            exp->op_expr.operator = type;
            exp->type = OPERATOR_EXPRESSION;
            break;

            /* Arithmetic, unary */
            case OP_NOT:
            case OP_NEG:
            case OP_LO:
            case OP_HI:
            case OP_UMINUS:
            case OP_BANK:
            get_expr_recursive(fp, &exp->op_expr.lhs, u);
            exp->op_expr.rhs = NULL;
            exp->op_expr.operator = type;
            exp->type = OPERATOR_EXPRESSION;
            break;

            default:
            fprintf(stderr, "%s(0x%lx): get_expr(): invalid expression type (%.2X)\n", u->name, ftell(fp), type);
            exp->integer = 0;
            exp->type = INTEGER_EXPRESSION;
            break;
        }
    }
    /* Set owner */
    exp->unit = u;
    /* Set result pointer */
    *dest = exp;
}

/**
 * Reads expressions from file.
 * @param fp File handle
 * @param u Unit whose expressions array to populate
 */
static void get_expressions(FILE *fp, unit *u)
{
    int i;
    int count;
    /* Read the number of expressions */
    count = get_2(fp);
    /* Allocate the list */
    if (count > 0) {
        u->expressions = (expression **)malloc(sizeof(expression *) * count);
    }
    else {
        u->expressions = NULL;
    }
    /* Read expressions */
    for (i=0; i<count; i++) {
        get_expr_recursive(fp, &u->expressions[i], u);
    }
    /* Store count */
    u->expr_count = count;
}

/**
 * Reads a segment from file.
 * @param fp File handle
 * @param seg Where to store segment
 */
static void get_segment(FILE *fp, segment *seg)
{
    /* Read size */
    seg->size = get_3(fp);
    /* Allocate space */
    if (seg->size > 0) {
        /* Allocate mem for bytecodes */
        seg->bytes = (unsigned char *)malloc(seg->size);
        if (seg->bytes != NULL) {
            /* Read contents */
            fread(seg->bytes, 1, seg->size, fp);
        }
    }
    else {
        seg->bytes = NULL;
    }
}

/*--------------------------------------------------------------------------*/

/**
 * Reads a unit from file.
 * @param filename Name of the unit
 * @param u Pointer to struct to fill in
 */
int unit_read(char *filename, unit *u)
{
    FILE *fp;
    int count, i;
    unsigned short magic;
    unsigned char version;

    /* Store name */
    u->name = filename;
    /* Attempt to open file */
    fp = fopen(filename, "rb");
    if (fp == NULL) {
        /* Error, couldn't open it */
        u->const_count = 0;
        u->ext_count = 0;
        u->expr_count = 0;
        u->dataseg.size = 0;
        u->dataseg.bytes = NULL;
        u->codeseg.size = 0;
        u->codeseg.bytes = NULL;
        return 0;
    }
    /* Read magic number */
    magic = get_2(fp);
    /* Verify magic number */
    if (magic != A_MAGIC) {
        /* Error, bad magic number */
        return 0;
    }
    /* Read version */
    version = get_1(fp);
    /* Verify version: should be 1.0 */
    if (version != A_VERSION) {
        /* Error, bad version */
        return 0;
    }
    /* Read exported constants */
    get_constants(fp, u);
    /* Read # of units explicitly imported from */
    count = get_1(fp);
    /* Read unit names */
    for (i=0; i<count; i++) {
        get_str_8(fp);
    }
    /* Read imported symbols */
    get_externals(fp, u);
    /* Read dataseg */
    get_segment(fp, &u->dataseg);
    /* Read codeseg */
    get_segment(fp, &u->codeseg);
    /* Read expressions */
    get_expressions(fp, u);

    /* Close the file */
    fclose(fp);

    /* Success */
    return 1;
}

/*--------------------------------------------------------------------------*/

/**
 * Finalizes a constant.
 * @param c Constant to finalize
 */
static void finalize_constant(constant *c)
{
    if (c->type == STRING_CONSTANT) {
        SAFE_FREE(c->string);
    }
    SAFE_FREE(c->name);
}

/**
 * Finalizes an external.
 * @param e External to finalize
 */
static void finalize_external(external *e)
{
    SAFE_FREE(e->name);
}

/**
 * Finalizes an expression.
 * @param e Expression to finalize
 */
static void finalize_expression(expression *e)
{
    if (e == NULL) { return; }
    switch (e->type) {
        case STRING_EXPRESSION:
        SAFE_FREE(e->string);
        break;

        case OPERATOR_EXPRESSION:
        finalize_expression(e->op_expr.lhs);
        finalize_expression(e->op_expr.rhs);
        break;

        default:
        /* Nada */
        break;
    }
    SAFE_FREE(e);
}

/**
 * Finalizes a bytecode segment.
 * @param s Segment to finalize
 */
static void finalize_segment(segment *s)
{
    SAFE_FREE(s->bytes);
}

/*--------------------------------------------------------------------------*/

/**
 * Finalizes a unit.
 * @param u Unit to finalize
 */
void unit_finalize(unit *u)
{
    int i;
    /* Finalize the constants */
    for (i=0; i<u->const_count; i++) {
        finalize_constant(&u->constants[i]);
    }
    SAFE_FREE(u->constants);
    /* Finalize the externals */
    for (i=0; i<u->ext_count; i++) {
        finalize_external(&u->externals[i]);
    }
    SAFE_FREE(u->externals);
    /* Finalize expressions */
    for (i=0; i<u->expr_count; i++) {
        finalize_expression(u->expressions[i]);
    }
    SAFE_FREE(u->expressions);
    /* Finalize bytecode segments */
    finalize_segment(&u->dataseg);
    finalize_segment(&u->codeseg);
}
