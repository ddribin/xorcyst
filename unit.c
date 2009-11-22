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
    int len = get_1(fp) + 1;
    s = (char *)malloc(len + 1);
    if (s != NULL) {
        fread(s, 1, len, fp);
        s[len] = '\0';
    }
    return s;
}
/* Reads a string prepended by 16-bit length */
static char *get_str_16(FILE *fp)
{
    char *s;
    int len = get_2(fp) + 1;
    s = (char *)malloc(len + 1);
    if (s != NULL) {
        fread(s, 1, len, fp);
        s[len] = '\0';
    }
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
static void get_const(FILE *fp, xasm_unit *u, int i)
{
    xasm_constant *cnst = &u->constants[i];
    cnst->name = get_str_8(fp);
    cnst->type = get_1(fp);
    switch (cnst->type) {
        case XASM_INT_8: cnst->integer = get_1(fp);  cnst->type = XASM_INTEGER_CONSTANT;  break;
        case XASM_INT_16:    cnst->integer = get_2(fp);  cnst->type = XASM_INTEGER_CONSTANT;  break;
        case XASM_INT_24:    cnst->integer = get_3(fp);  cnst->type = XASM_INTEGER_CONSTANT;  break;
        case XASM_INT_32:    cnst->integer = get_4(fp);  cnst->type = XASM_INTEGER_CONSTANT;  break;
        case XASM_STR_8: cnst->string = get_str_8(fp);   cnst->type = XASM_STRING_CONSTANT;   break;
        case XASM_STR_16:    cnst->string = get_str_16(fp);  cnst->type = XASM_STRING_CONSTANT;   break;

        default:
        fprintf(stderr, "%s(0x%lx): get_const(): bad constant type (%.2X)\n", u->name, ftell(fp), cnst->type);
        break;
    }
    cnst->unit = u;
}

/**
 * Reads constant array from file.
 * @param fp File handle
 * @param u Unit whose constants array will be populated
 */
static void get_constants(FILE *fp, xasm_unit *u)
{
    int i;
    int count = get_2(fp);
    if (count > 0) {
        u->constants = (xasm_constant *)malloc(sizeof(xasm_constant) * count);
    } else {
        u->constants = NULL;
    }
    for (i=0; i<count; i++) {
        get_const(fp, u, i);
    }
    u->const_count = count;
}

/**
 * Reads imported symbol from file.
 * @param fp File handle
 * @param u Unit
 * @param i External index
 */
static void get_ext(FILE *fp, xasm_unit *u, int i)
{
    xasm_external *ext = &u->externals[i];
    ext->unit = get_1(fp);
    ext->name = get_str_8(fp);
    ext->from = u;
}

/**
 * Reads imported symbol array from file.
 * @param fp File handle
 * @param u Unit whose externals array will be populated
 */
static void get_externals(FILE *fp, xasm_unit *u)
{
    int i;
    int count;
    count = get_2(fp);
    if (count > 0) {
        u->externals = (xasm_external *)malloc(sizeof(xasm_external) * count);
    } else {
        u->externals = NULL;
    }
    for (i=0; i<count; i++) {
        get_ext(fp, u, i);
    }
    u->ext_count = count;
}

/**
 * Reads an expression from file.
 * @param fp File handle
 * @param dest Pointer to pointer where expression should be stored
 * @param u Owner unit
 */
static void get_expr_recursive(FILE *fp, xasm_expression **dest, xasm_unit *u)
{
    unsigned char type;
    xasm_expression *exp = (xasm_expression *)malloc( sizeof(xasm_expression) );
    if (exp != NULL) {
        type = get_1(fp);
        switch (type) {
            case XASM_INT_8: exp->integer = get_1(fp);   exp->type = XASM_INTEGER_EXPRESSION; break;
            case XASM_INT_16:    exp->integer = get_2(fp);   exp->type = XASM_INTEGER_EXPRESSION; break;
            case XASM_INT_24:    exp->integer = get_3(fp);   exp->type = XASM_INTEGER_EXPRESSION; break;
            case XASM_INT_32:    exp->integer = get_4(fp);   exp->type = XASM_INTEGER_EXPRESSION; break;
            case XASM_STR_8: exp->string = get_str_8(fp);    exp->type = XASM_STRING_EXPRESSION;  break;
            case XASM_STR_16:    exp->string = get_str_16(fp);   exp->type = XASM_STRING_EXPRESSION;  break;

            case XASM_LOCAL: exp->local_id = get_2(fp);  exp->type = XASM_LOCAL_EXPRESSION;   break;
            case XASM_EXTRN: exp->extrn_id = get_2(fp);  exp->type = XASM_EXTERNAL_EXPRESSION;break;

            case XASM_PC:    ;               exp->type = XASM_PC_EXPRESSION;  break;

            case XASM_OP_PLUS:
            case XASM_OP_MINUS:
            case XASM_OP_MUL:
            case XASM_OP_DIV:
            case XASM_OP_MOD:
            case XASM_OP_SHL:
            case XASM_OP_SHR:
            case XASM_OP_AND:
            case XASM_OP_OR:
            case XASM_OP_XOR:
            case XASM_OP_EQ:
            case XASM_OP_NE:
            case XASM_OP_LT:
            case XASM_OP_GT:
            case XASM_OP_LE:
            case XASM_OP_GE:
            get_expr_recursive(fp, &exp->op_expr.lhs, u);
            get_expr_recursive(fp, &exp->op_expr.rhs, u);
            exp->op_expr.operator = type;
            exp->type = XASM_OPERATOR_EXPRESSION;
            break;

            case XASM_OP_NOT:
            case XASM_OP_NEG:
            case XASM_OP_LO:
            case XASM_OP_HI:
            case XASM_OP_UMINUS:
            case XASM_OP_BANK:
            get_expr_recursive(fp, &exp->op_expr.lhs, u);
            exp->op_expr.rhs = NULL;
            exp->op_expr.operator = type;
            exp->type = XASM_OPERATOR_EXPRESSION;
            break;

            default:
            fprintf(stderr, "%s(0x%lx): get_expr(): invalid expression type (%.2X)\n", u->name, ftell(fp), type);
            exp->integer = 0;
            exp->type = XASM_INTEGER_EXPRESSION;
            break;
        }
    }
    exp->unit = u;
    *dest = exp;
}

/**
 * Reads expressions from file.
 * @param fp File handle
 * @param u Unit whose expressions array to populate
 */
static void get_expressions(FILE *fp, xasm_unit *u)
{
    int i;
    int count;
    count = get_2(fp);
    if (count > 0) {
        u->expressions = (xasm_expression **)malloc(sizeof(xasm_expression *) * count);
    } else {
        u->expressions = NULL;
    }
    for (i=0; i<count; i++) {
        get_expr_recursive(fp, &u->expressions[i], u);
    }
    u->expr_count = count;
}

/**
 * Reads a segment from file.
 * @param fp File handle
 * @param seg Where to store segment
 */
static void get_segment(FILE *fp, xasm_segment *seg)
{
    seg->size = get_3(fp);
    if (seg->size > 0) {
        seg->bytes = (unsigned char *)malloc(seg->size);
        if (seg->bytes != NULL) {
            fread(seg->bytes, 1, seg->size, fp);
        }
    } else {
        seg->bytes = NULL;
    }
}

/*--------------------------------------------------------------------------*/

/**
 * Reads a unit from file.
 * @param filename Name of the unit
 * @param u Pointer to struct to fill in
 */
int xasm_unit_read(const char *filename, xasm_unit *u)
{
    FILE *fp;
    int count, i;
    unsigned short magic;
    unsigned char version;

    u->name = filename;
    fp = fopen(filename, "rb");
    if (fp == NULL) {
        u->const_count = 0;
        u->ext_count = 0;
        u->expr_count = 0;
        u->dataseg.size = 0;
        u->dataseg.bytes = NULL;
        u->codeseg.size = 0;
        u->codeseg.bytes = NULL;
        return 0;
    }

    magic = get_2(fp);
    if (magic != XASM_MAGIC) {
        /* Error, bad magic number */
        return 0;
    }

    version = get_1(fp);
    if (version != XASM_OBJ_VERSION) {
        /* Error, bad version */
        return 0;
    }

    get_constants(fp, u);

    /* Read # of units explicitly imported from */
    count = get_1(fp);
    /* Read unit names */
    for (i=0; i<count; i++) {
        get_str_8(fp);
    }

    get_externals(fp, u);
    get_segment(fp, &u->dataseg);
    get_segment(fp, &u->codeseg);
    get_expressions(fp, u);

    fclose(fp);

    /* Success */
    return 1;
}

/*--------------------------------------------------------------------------*/

/**
 * Finalizes a constant.
 * @param c Constant to finalize
 */
static void finalize_constant(xasm_constant *c)
{
    if (c->type == XASM_STRING_CONSTANT) {
        SAFE_FREE(c->string);
    }
    SAFE_FREE(c->name);
}

/**
 * Finalizes an external.
 * @param e External to finalize
 */
static void finalize_external(xasm_external *e)
{
    SAFE_FREE(e->name);
}

/**
 * Finalizes an expression.
 * @param e Expression to finalize
 */
static void finalize_expression(xasm_expression *e)
{
    if (e == NULL) { return; }
    switch (e->type) {
        case XASM_STRING_EXPRESSION:
        SAFE_FREE(e->string);
        break;

        case XASM_OPERATOR_EXPRESSION:
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
static void finalize_segment(xasm_segment *s)
{
    SAFE_FREE(s->bytes);
}

/*--------------------------------------------------------------------------*/

/**
 * Finalizes a unit.
 * @param u Unit to finalize
 */
void xasm_unit_finalize(xasm_unit *u)
{
    int i;
    for (i=0; i<u->const_count; i++) {
        finalize_constant(&u->constants[i]);
    }
    SAFE_FREE(u->constants);

    for (i=0; i<u->ext_count; i++) {
        finalize_external(&u->externals[i]);
    }
    SAFE_FREE(u->externals);

    for (i=0; i<u->expr_count; i++) {
        finalize_expression(u->expressions[i]);
    }
    SAFE_FREE(u->expressions);

    finalize_segment(&u->dataseg);
    finalize_segment(&u->codeseg);
}

/**
 * Gets string representation of an operator (OP_*, see objdef.h).
 * @param op Operator
 * @return String representation of operator
 */
const char *xasm_operator_to_string(int op)
{
    switch (op) {
        case XASM_OP_PLUS:   return "+";
        case XASM_OP_MINUS:  return "-";
        case XASM_OP_MUL:    return "*";
        case XASM_OP_DIV:    return "/";
        case XASM_OP_MOD:    return "%";
        case XASM_OP_SHL:    return "<<";
        case XASM_OP_SHR:    return ">>";
        case XASM_OP_AND:    return "&";
        case XASM_OP_OR: return "|";
        case XASM_OP_XOR:    return "^";
        case XASM_OP_EQ: return "==";
        case XASM_OP_NE: return "!=";
        case XASM_OP_LT: return "<";
        case XASM_OP_GT: return ">";
        case XASM_OP_LE: return "<=";
        case XASM_OP_GE: return ">=";
        case XASM_OP_NOT:    return "!";
        case XASM_OP_NEG:    return "~";
        case XASM_OP_LO: return "<";
        case XASM_OP_HI: return ">";
        case XASM_OP_UMINUS: return "-";
        case XASM_OP_BANK:   return "^";
    }
    return "";
}
