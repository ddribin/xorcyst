/*
 * $Id: codegen.c,v 1.10 2007/08/12 18:58:12 khansen Exp $
 * $Log: codegen.c,v $
 * Revision 1.10  2007/08/12 18:58:12  khansen
 * ability to generate pure 6502 binary (--pure-binary switch)
 *
 * Revision 1.9  2007/08/09 20:33:48  khansen
 * progress
 *
 * Revision 1.8  2007/08/07 22:42:17  khansen
 * make sure a CMD_FILE command is output at the start of each segment
 *
 * Revision 1.7  2007/08/07 21:23:24  khansen
 * const
 *
 * Revision 1.6  2007/07/22 13:33:26  khansen
 * convert tabs to whitespaces
 *
 * Revision 1.5  2004/12/19 19:58:37  kenth
 * xorcyst 1.4.0
 *
 * Revision 1.4  2004/12/18 16:58:52  kenth
 * outputs file and line info if --debug
 * CMD_DSW, CMD_DSD gone
 *
 * Revision 1.3  2004/12/14 01:49:14  kenth
 * xorcyst 1.3.0
 *
 * Revision 1.2  2004/12/06 04:52:36  kenth
 * xorcyst 1.1.0
 *
 * Revision 1.1  2004/06/30 07:55:38  kenth
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

/**
 * This file contains functions for generating object code from an abstract
 * syntax tree (AST).
 * The output of the assembler is not pure 6502 binary code. It is a file in
 * a custom object format meant to be fed to the XORcyst linker. The object
 * file contains information about the symbols it exports (that others may
 * access), the symbols it imports (the linker will look for these in other
 * units and resolve their values), and of course the 6502 instructions.
 * Labels, data and instructions are encoded in a special bytecode language.
 * All operands to instructions and directives which aren't constants are
 * stored as expressions. (It is the linker's job to parse the expressions,
 * compute their values and produce the actual 6502 binary.)
 *
 * The format of resulting file is:
 * 1 word: magic number
 * 1 byte: version
 * 1 word: number of exported constants
 * ? bytes: exported constants descriptors
 * 1 byte: number of units explicitly imported from
 * ? bytes: names of units explicitly imported from
 * 1 word: number of imported symbols (externals)
 * ? bytes: imported symbols descriptors
 * 24-bit word: size of data segment
 * ? bytes: data segment bytecodes
 * 24-bit word: size of code segment
 * ? bytes: code segment bytecodes
 * 1 word: number of expressions
 * ? bytes: expression descriptors
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codegen.h"
#include "symtab.h"
#include "objdef.h"
#include "opcode.h"
#include "xasm.h"
#include "assert.h"

/*---------------------------------------------------------------------------*/

/* Convenient macros to write integers in big-endian form. */
#define put_1(f, b) { fputc((unsigned char)(b), f); }
#define put_2(f, w) { put_1(f, (w) >> 8); put_1(f, w); }
#define put_3(f, t) { put_2(f, (t) >> 8); put_1(f, t); }
#define put_4(f, q) { put_2(f, (q) >> 16); put_2(f, q); }

/* Convenient macros to write length-prepended strings. */
#define put_str_8(f, s) { put_1(f, strlen(s)-1); fprintf(f, "%s", s); }
#define put_str_16(f, s) { put_2(f, strlen(s)-1); fprintf(f, "%s", s); }
#define put_str(f, s) { if (strlen(s) <= 0x100) { put_str_8(f, s); } else { put_str_16(f, s); } }

/**
 * Writes a sequence of bytes to file.
 * @param f File handle
 * @param bytes Array of bytes
 * @param count Number of bytes
 */
static void put_bytes(FILE *f, const unsigned char *bytes, int count)
{
    int i;
    /* Write the bytes */
    for (i=0; i<count; i++) {
        put_1(f, bytes[i]);
    }
}

/*---------------------------------------------------------------------------*/

#define BUF_MAX_SIZE 0x10000
/** Buffer used to collect binary data generated from processing consecutive code statements. */
static unsigned char buf[BUF_MAX_SIZE];
/** Current position in buffer */
static int buf_pos = 0;
/** Buffer file */
static FILE *buf_file = NULL;

#define buf_reset() { buf_pos = 0; }

/**
 * Flushes contents of <code>buf</code> to file.
 * @param f File handle
 */
static void buf_flush(FILE *f)
{
    if (buf_pos > 0) {
        if (buf_pos <= 0x100) {
            /* Use 8-bit form. */
            put_1(f, CMD_BIN8);
            put_1(f, buf_pos - 1);
            put_bytes(f, buf, buf_pos);
        }
        else if (buf_pos <= 0x10000) {
            /* Use 16-bit form. */
            put_1(f, CMD_BIN16);
            put_2(f, buf_pos - 1);
            put_bytes(f, buf, buf_pos);
        }
        else {
            /* Error, buffer overflow. */
            fprintf(stderr, "buf_flush: buffer overflow\n");
        }
        buf_reset();
    }
}

/** Appends single byte to buffer. */
#define buf_append(b) { if (buf_pos >= BUF_MAX_SIZE) buf_flush(buf_file); buf[buf_pos++] = (unsigned char)(b); }

/** Appends n bytes to buffer. */
static void buf_append_n(unsigned char *data, int size)
{
    int i;
    for (i=0; i<size; i++) {
        buf_append(data[i]);
    }
}

/*---------------------------------------------------------------------------*/

/**
 * Converts an arithmetic operator (from ARITHMETIC_NODE) into a byte value.
 * The bytecode language uses a different operator encoding which is why
 * we need this function.
 * @param op Operator to convert
 * @return Bytecode-equivalent operator
 */
static unsigned char operator_to_bytecode(arithmetic_operator op)
{
    switch (op) {
        case PLUS_OPERATOR: return OP_PLUS;
        case MINUS_OPERATOR:    return OP_MINUS;
        case MUL_OPERATOR:  return OP_MUL;
        case DIV_OPERATOR:  return OP_DIV;
        case MOD_OPERATOR:  return OP_MOD;
        case AND_OPERATOR:  return OP_AND;
        case OR_OPERATOR:   return OP_OR;
        case XOR_OPERATOR:  return OP_XOR;
        case SHL_OPERATOR:  return OP_SHL;
        case SHR_OPERATOR:  return OP_SHR;
        case LT_OPERATOR:   return OP_LT;
        case GT_OPERATOR:   return OP_GT;
        case EQ_OPERATOR:   return OP_EQ;
        case NE_OPERATOR:   return OP_NE;
        case LE_OPERATOR:   return OP_LE;
        case GE_OPERATOR:   return OP_GE;
        case NEG_OPERATOR:  return OP_NEG;
        case NOT_OPERATOR:  return OP_NOT;
        case LO_OPERATOR:   return OP_LO;
        case HI_OPERATOR:   return OP_HI;
        case UMINUS_OPERATOR:   return OP_UMINUS;
        case BANK_OPERATOR: return OP_BANK;

        default:
        fprintf(stderr, "operator_to_bytecode(): bad operator\n");
    }
    return 0xFF;
}

/*---------------------------------------------------------------------------*/

/* Maximum number of expressions that the code generator can handle. */
#define MAX_EXPRS 16384

/* List of expressions involved in code statements. */
static astnode *expr_list[MAX_EXPRS];
/* Number of expressions generated so far. */
static int expr_count = 0;

/**
 * Registers an expression with the code generator.
 * Unique 16-bit ID of the expression is returned, which may then be used to refer
 * to the expression in bytecodes.
 * @param expr Expression to register
 * @return Unique ID of expression
 */
static int register_expression(astnode *expr)
{
    int i;
    /* See if an equivalent expression already is registered */
    for (i=0; i<expr_count; i++) {
        if (astnode_equal(expr, expr_list[i])) {
            /* Return ID of equivalent expression. */
            return i;
        }
    }
    /* This is a new expression, add it to end of list if possible. */
    if (expr_count == MAX_EXPRS) {
        fprintf(stderr, "register_expression(): buffer overflow\n");
        return -1;
    }
    else {
        expr_list[expr_count++] = expr;
        return expr_count-1;
    }
}

/**
 * Serializes an expression recursively.
 * The result of serializing a leaf node is always (type, value).
 * The result of serializing an operator node is always (operator, lhs, rhs).
 * @param fp File handle
 * @param expr Expression to serialize
 */
static void put_expr_recursive(FILE *fp, const astnode *expr)
{
    unsigned long v;
    char *s;
    symtab_entry *e;
    if (expr == NULL) { return; }
//  printf("put_expr_recursive(%s)\n", astnode_type_to_string(expr->type) );
    switch (astnode_get_type(expr)) {
        case INTEGER_NODE:
        v = (unsigned long)expr->integer;
        if (v < 0x100) {
            /* Write type */    put_1(fp, INT_8);
            /* Write value */   put_1(fp, v);
        }
        else if (v < 0x10000) {
            /* Write type */    put_1(fp, INT_16);
            /* Write value */   put_2(fp, v);
        }
        else if (v < 0x1000000) {
            /* Write type */    put_1(fp, INT_24);
            /* Write value */   put_3(fp, v);
        }
        else {
            /* Write type */    put_1(fp, INT_32);
            /* Write value */   put_4(fp, v);
        }
        break;

        case STRING_NODE:
        s = expr->string;
        if (strlen(s) <= 0x100) {
            /* Write type */    put_1(fp, STR_8);
            /* Write string */  put_str_8(fp, s);
        }
        else {
            /* Write type */    put_1(fp, STR_16);
            /* Write string */  put_str_16(fp, s);
        }
        break;

        case IDENTIFIER_NODE:
        /* Look it up */
        e = symtab_lookup(expr->ident);
        /* Write type */
        put_1(fp, (e->flags & EXTRN_FLAG) ? EXTRN : LOCAL);
        /* Write ID */
        put_2(fp, e->tag);
        break;

        case ARITHMETIC_NODE:
        /* The operator goes first */
        put_1(fp, operator_to_bytecode(expr->oper) );
        /* Then left-hand side */
        put_expr_recursive(fp, LHS(expr));
        /* Then right-hand side */
        put_expr_recursive(fp, RHS(expr));
        break;

        case CURRENT_PC_NODE:
        put_1(fp, PC);
        break;

        default:
        /* Error */
        fprintf(
            stderr,
            "put_expr_recursive(): unknown expression type (%s)\n",
            astnode_type_to_string(astnode_get_type(expr))
        );
        break;
    }
}

/**
 * Serializes an expression.
 * The expression is written in postfix form so that it can be easily evaluated
 * even as pure binary.
 * @param fp File handle
 * @param expr Expression to serialize
 */
static void put_expr(FILE *fp, const astnode *expr)
{
    put_expr_recursive(fp, expr);
}

/**
 * Writes all expressions registered during code generation to file.
 * @param fp File handle
 */
static void put_expressions(FILE *fp)
{
    int i;
    /* Write expression count */
    put_2(fp, expr_count);
    /* Write serialized expressions */
    for (i=0; i<expr_count; i++) {
        put_expr(fp, expr_list[i]);
    }
}

/*---------------------------------------------------------------------------*/

/**
 * Tests if two file location descriptors are equal.
 */
static int locations_are_equal(const location *l1, const location *l2)
{
    return ((l1->first_line == l2->first_line) && (strcmp(l1->file, l2->file) == 0));
}

/*---------------------------------------------------------------------------*/

/**
 * Writes a statement to file.
 * @param fp File handle
 * @param n Statement
 */
static void put_statement(FILE *fp, const astnode *n, location *loc)
{
    symtab_entry *e;
    astnode *expr;
    astnode *type;
    int len;
    int i;
    int line;
    unsigned char flags;
    static int tag = 0; /* Used to give labels unique IDs */

    /* See if we should embed file+line information */
    if (xasm_args.debug && !locations_are_equal(loc, &n->loc) ) {
        buf_flush(fp);
        if (strcmp(loc->file, n->loc.file) != 0) {
            put_1(fp, CMD_FILE);
            put_str_8(fp, n->loc.file);
        }
        if (loc->first_line != n->loc.first_line) {
            line = n->loc.first_line;
            if (line == loc->first_line + 1) {
                put_1(fp, CMD_LINE_INC);
            } else {
                if (line < 256) {
                    put_1(fp, CMD_LINE8);
                    put_1(fp, line);
                }
                else if (line < 65536) {
                    put_1(fp, CMD_LINE16);
                    put_2(fp, line);
                }
                else {
                    put_1(fp, CMD_LINE24);
                    put_3(fp, line);
                }
            }
        }
    }
    *loc = n->loc;

    switch (astnode_get_type(n)) {

        case LABEL_NODE:
        buf_flush(fp);
        /* Write command */
        put_1(fp, CMD_LABEL);
        /* Look it up in symbol table */
        e = symtab_lookup(n->label);
        /* IMPORTANT: Tag label uniquely so we can refer to it in expressions */
        e->tag = tag++;
        /* Write flag byte */
        flags = 0;
        flags |= (e->flags & PUBLIC_FLAG) ? LABEL_FLAG_EXPORT : 0;
        flags |= (e->flags & ZEROPAGE_FLAG) ? LABEL_FLAG_ZEROPAGE : 0;
        flags |= (e->flags & ALIGN_FLAG) ? LABEL_FLAG_ALIGN : 0;
        flags |= (e->flags & ADDR_FLAG) ? LABEL_FLAG_ADDR : 0;
        put_1(fp, flags);
        /* If exported, write name also */
        if (e->flags & PUBLIC_FLAG) {
            put_str_8(fp, e->id);
        }
        /* if alignment, write that too */
        if (e->flags & ALIGN_FLAG) {
            put_1(fp, e->align);
        }
        /* if address, write that too */
        if (e->flags & ADDR_FLAG) {
            put_2(fp, e->address);
        }
        break;

        case INSTRUCTION_NODE:
        /* Get # of bytes such a 6502 instruction occupies */
        len = opcode_length(n->instr.opcode);
        if (len == 1) {
            /* Instruction has no operand, so append it to binary buffer. */
            buf_append(n->instr.opcode);
        }
        else {
            /* Instruction has an operand.
            It may be a constant or something unresolved. */
            expr = astnode_get_child(n, 0);
            if (astnode_get_type(expr) == INTEGER_NODE) {
                /* Instruction has constant operand, so append it to binary buffer. */
                buf_append(n->instr.opcode);
                /* Lo byte */
                buf_append(expr->integer);
                if (len == 3) {
                    /* Hi byte */
                    buf_append(expr->integer >> 8);
                }
            }
            else {
                /* Instruction has unresolved operand. */
                /* Flush binary buffer to file */
                buf_flush(fp);
                /* Output 4-byte sequence: CMD_INSTR [opcode] [expr-id] */
                put_1(fp, CMD_INSTR);
                put_1(fp, n->instr.opcode);
                put_2(fp, register_expression(expr));
            }
        }
        break;

        case BINARY_NODE:
        /* Append the binary to the buffer */
        buf_append_n(n->binary.data, n->binary.size);
        break;

        case DATA_NODE:
        i = 0;
        type = LHS(n);
        /* Go through all the elements of the data array */
        for (expr = RHS(n); expr != NULL; expr = astnode_get_next_sibling(expr) ) {
            if (astnode_get_type(expr) == INTEGER_NODE) {
                /* Constant, output as binary */
                switch (type->datatype) {
                    case BYTE_DATATYPE:
                    buf_append(expr->integer);
                    break;

                    case WORD_DATATYPE:
                    /* Note: little-endian here (6502) */
                    buf_append(expr->integer);
                    buf_append(expr->integer >> 8);
                    break;

                    case DWORD_DATATYPE:
                    /* Note: little-endian here (6502) */
                    buf_append(expr->integer);
                    buf_append(expr->integer >> 8);
                    buf_append(expr->integer >> 16);
                    buf_append(expr->integer >> 24);
                    break;

                    default:
                    /* Nada */
                    break;
                }
            }
            else {
                /* Unresolved. Linker will handle it. */
                /* Flush binary buffer to file */
                buf_flush(fp);
                /* Output 3-byte sequence: [type-cmd] [expr-id] */
                switch (type->datatype) {
                    case BYTE_DATATYPE: put_1(fp, CMD_DB);  break;
                    case WORD_DATATYPE: put_1(fp, CMD_DW);  break;
                    case DWORD_DATATYPE:    put_1(fp, CMD_DD);  break;
                    default:
                    /* Nada */
                    break;
                }
                put_2(fp, register_expression(expr));
            }
        }
        break;

        case STORAGE_NODE:
        buf_flush(fp);
        type = LHS(n);
        assert(type->datatype == BYTE_DATATYPE);
        /* Get expression which is the count */
        expr = RHS(n);
        /* Write command */
        if (astnode_get_type(expr) == INTEGER_NODE) {
            /* Use the immediate form. */
            /* Calculate the # of bytes. */
            len = expr->integer;
            /* Select bytecode depending on whether count fits in 8 bits or not */
            if (len <= 0x100) {
                /* Write command */
                put_1(fp, CMD_DSI8);
                /* Write count */
                put_1(fp, len-1);
            }
            else {
                /* Write command */
                put_1(fp, CMD_DSI16);
                /* Write count */
                put_2(fp, len-1);
            }
        }
        else {
            /* Use the unresolved form. */
            put_1(fp, CMD_DSB);
            /* Write expression ID */
            put_2(fp, register_expression(expr));
        }
        break;

        case INCSRC_NODE:
        case INCBIN_NODE:
        case CHARMAP_NODE:
        case STRUC_DECL_NODE:
        case UNION_DECL_NODE:
        case ENUM_DECL_NODE:
        case RECORD_DECL_NODE:
        /* Ignore */
        break;

        default:
        /* Nada */
        fprintf(stderr, "put_statement(%s): unsupported type\n", astnode_type_to_string(astnode_get_type(n)));
        break;
    }
}

/*---------------------------------------------------------------------------*/

/**
 * Writes the public constants to file.
 * @param fp File handle
 */
static void put_public_constants(FILE *fp)
{
    symbol_ident_list list;
    unsigned long v;
    char *s;
    symtab_entry *e;
    int i;
    astnode *expr;
    int count;

    /* 16-bit count followed by (name, type, value) triplets */

    symtab_list_type(CONSTANT_SYMBOL, &list);
    /* Make one iteration to look them up and count them */
    count = 0;
    for (i=0; i<list.size; i++) {
        e = symtab_lookup(list.idents[i]);
        if (e->flags & PUBLIC_FLAG) {
            count++;
        }
    }
    /* Write count */
    put_2(fp, count);
    /* Write triplets */
    for (i=0; i<list.size; i++) {
        e = symtab_lookup(list.idents[i]);
        if (e->flags & PUBLIC_FLAG) {
            /* Write the name */
            put_str_8(fp, e->id);
            /* Get the expression which represents the value */
            expr = (astnode *)e->def;
            /* Write the type and value */
            switch (astnode_get_type(expr)) {
                case INTEGER_NODE:
                v = (unsigned long)expr->integer;
                if (v < 0x100) {
                    /* Write type */    put_1(fp, INT_8);
                    /* Write value */   put_1(fp, v);
                }
                else if (v < 0x10000) {
                    /* Write type */    put_1(fp, INT_16);
                    /* Write value */   put_2(fp, v);
                }
                else if (v < 0x1000000) {
                    /* Write type */    put_1(fp, INT_24);
                    /* Write value */   put_3(fp, v);
                }
                else {
                    /* Write type */    put_1(fp, INT_32);
                    /* Write value */   put_4(fp, v);
                }
                break;

                case STRING_NODE:
                s = expr->string;
                if (strlen(s) <= 0x100) {
                    /* Write type */    put_1(fp, STR_8);
                    /* Write value */   put_str_8(fp, s);
                }
                else if (strlen(s) <= 0x10000) {
                    /* Write type */    put_1(fp, STR_16);
                    /* Write value */   put_str_16(fp, s);
                }
                else {
                    fprintf(stderr, "put_public_constants(): string constant too long\n");
                }
                break;

                default:
                fprintf(stderr, "put_public_constants(): expression isn't a constant after all\n");
                break;
            }
        }
    }
    symtab_list_finalize(&list);
}

/**
 * Writes the external symbol specifiers to file.
 * @param fp File handle
 */
static void put_externals(FILE *fp)
{
    symbol_ident_list list;
    symtab_entry *e;
    int i;
    int tag;
    int count;
    /* number of units explicitly imported from */
    put_1(fp, 0x00);    /* TODO */
    /* List of unit names (TODO) */

    /* 16-bit count followed by name list */

    symtab_list_type(ANY_SYMBOL, &list);
    /* One iteration to count them */
    count = 0;
    for (i=0; i<list.size; i++) {
        e = symtab_lookup(list.idents[i]);
        if (e->flags & EXTRN_FLAG) {
            count++;
        }
    }
    put_2(fp, count);
    /* Another iteration to output them */
    tag = 0;
    for (i=0; i<list.size; i++) {
        /* Look it up */
        e = symtab_lookup(list.idents[i]);
        if (e->flags & EXTRN_FLAG) {
            /* IMPORTANT: Set unique tag so we can refer to it in expressions */
            /* (This probably shouldn't be done here though...???) */
            e->tag = tag++;
            /* Write unit (TODO) */
            put_1(fp, 0x00);    /* 0 = Any unit */
            /* Write name */
            put_str_8(fp, list.idents[i]);
        }
    }
    symtab_list_finalize(&list);
}

/**
 * Writes a segment to file.
 * @param fp File handle
 * @param root AST node which has the list of assembly statements as its children
 * @param targetseg Specifies the type of segment (data or code)
 */
static void put_segment(FILE *fp, const astnode *root, int targetseg)
{
    astnode *n;
    int seg;    /* The type of segment we're in */
    fpos_t size_pos, end_pos;
    long start, end;
    location loc = { -1, -1, -1, -1, "" };

    /* Write the size of the segment (backpatched later) */
    fgetpos(fp, &size_pos);
    put_3(fp, 0xDECADE);    /* 24-bit integer */

    seg = 2;    /* Codeseg is default */
    buf_reset();
    start = ftell(fp);
    /* Step through all the nodes in the list */
    for (n = astnode_get_first_child(root); n != NULL; n = astnode_get_next_sibling(n) ) {
        switch (astnode_get_type(n)) {
            case DATASEG_NODE:
            seg = 1;    /* Now we're in a DATA segment */
            break;

            case CODESEG_NODE:
            seg = 2;    /* Now we're in a CODE segment */
            break;

            default:
            /* Only output the statement if we're in the proper segment. */
            if (seg == targetseg) {
                put_statement(fp, n, &loc);
            }
            break;
        }
    }
    buf_flush(fp);
    put_1(fp, CMD_END);
    end = ftell(fp);
    /* Backpatch size */
    fgetpos(fp, &end_pos);  /* First save the current (end) position */
    fsetpos(fp, &size_pos); /* Set position to where the size should be written */
    put_3(fp, end - start); /* Size is difference between end and start offset */
    fsetpos(fp, &end_pos);  /* Restore end position */
}

/**
 * Writes a sequence of parsed assembly statements to file.
 * @param fp File handle
 * @param root AST node which has the assembly statements as its children
 */
static void put_bytecodes(FILE *fp, const astnode *root)
{
    /* Write data segment first */
    put_segment(fp, root, 1);
    /* Write code segment next */
    put_segment(fp, root, 2);
}

/*---------------------------------------------------------------------------*/

/**
 * Writes an object file which captures all the information in a syntax tree.
 * @param root Root of the syntax tree
 * @param filename Name of the file to write to (object file)
 */
void codegen_write(const astnode *root, const char *filename)
{
    /* Attempt to open file for writing */
    FILE *fp = fopen(filename, "wb");
    if (fp == NULL) {
        fprintf(stderr, "codegen_write: could not open '%s' for writing\n", filename);
        /* ### increase error count */
        return;
    }

    expr_count = 0;
    buf_file = fp;

    /* Write magic number */
    put_2(fp, A_MAGIC);
    /* Write version (upper nibble=major, lower nibble=minor) */
    put_1(fp, A_VERSION);

    /* Write exported constants. */
    put_public_constants(fp);

    /* Write imported symbols. */
    put_externals(fp);

    /* Write bytecodes. */
    put_bytecodes(fp, root);

    /* Write expressions */
    put_expressions(fp);

    /* Close the file */
    fclose(fp);
}
