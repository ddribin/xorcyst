/*
 * $Id: symtab.h,v 1.8 2007/08/12 18:59:10 khansen Exp $
 * $Log: symtab.h,v $
 * Revision 1.8  2007/08/12 18:59:10  khansen
 * ability to generate pure 6502 binary
 *
 * Revision 1.7  2007/08/09 20:34:09  khansen
 * progress
 *
 * Revision 1.6  2007/07/22 13:35:20  khansen
 * convert tabs to whitespaces
 *
 * Revision 1.5  2005/01/05 02:27:23  kenth
 * ordered_field_list has pointer to symtab_entry instead of id
 *
 * Revision 1.4  2004/12/19 20:47:04  kenth
 * xorcyst 1.4.0
 *
 * Revision 1.3  2004/12/16 13:21:47  kenth
 * added RECORD_SYMBOL
 *
 * Revision 1.2  2004/12/06 04:54:00  kenth
 * xorcyst 1.1.0
 *
 * Revision 1.1  2004/06/30 07:56:45  kenth
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

#ifndef SYMTAB_H
#define SYMTAB_H

#include "astnode.h"

/**
 * The possible types of symbol.
 */
enum tag_symbol_type {
    LABEL_SYMBOL=0,     /* A label */
    CONSTANT_SYMBOL,    /* A constant */
    MACRO_SYMBOL,       /* A macro */
    STRUC_SYMBOL,       /* A structure */
    UNION_SYMBOL,       /* A union */
    RECORD_SYMBOL,      /* A record */
    ENUM_SYMBOL,        /* An enumeration */
    VAR_SYMBOL,     /* A variable */
    PROC_SYMBOL,        /* A procedure */
    ANY_SYMBOL
};

typedef enum tag_symbol_type symbol_type;

typedef struct tag_label_attribs label_attribs;

/* Possible bits for entry.flags */
#define PUBLIC_FLAG 0x01    /* Symbol is public (exported) */
#define EXTRN_FLAG  0x02    /* Symbol is external */
#define DATA_FLAG   0x04    /* Symbol is in dataseg */
#define VOLATILE_FLAG   0x08    /* Symbol is volatile */
#define ZEROPAGE_FLAG   0x10    /* Symbol should be mapped to ZP */
#define ALIGN_FLAG  0x20    /* Symbol has alignment constraint */
#define ADDR_FLAG   0x40    /* Symbol has hardcoded address */
#define ERROR_UNDEFINED_FLAG  0x80
/**
 * Must keep a definition-ordered list of struct or union's fields.
 */
struct tag_ordered_field_list {
    struct tag_symtab_entry *entry;
    struct tag_ordered_field_list *next;
};

typedef struct tag_ordered_field_list ordered_field_list;

/**
 * Structure that describes a struct or union.
 */
struct tag_struc_attribs {
    astnode *size;  /* total size of the struct or union */
    ordered_field_list *fields;
};

typedef struct tag_struc_attribs struc_attribs;

struct tag_field_attribs {
    astnode *offset;    /* offset from start of structure, in bytes */
    astnode *size;      /* size of field, in bytes */
};

typedef struct tag_field_attribs field_attribs;

/**
 * Structure that describes a symbol table entry.
 */
struct tag_symtab_entry {
    symbol_type type;
    char *id;
    astnode *def;   /* Pointer to something that describes this entry more */
    int flags;
    int tag;
    int ref_count;
    int align;  /* 2^align = boundary */
    int address;
    struct tag_symtab *symtab;  /* Child symbols (STRUC|UNION|ENUM_SYMBOL) */
    union {
        field_attribs field;    /* type == VAR_SYMBOL */
        struc_attribs struc;    /* type == UNION|STRUC_SYMBOL */
    };
    struct tag_symtab_entry *left;
    struct tag_symtab_entry *right;
    struct tag_symtab_entry *parent;
};

typedef struct tag_symtab_entry symtab_entry;

/**
 * Structure that describes a symbol table.
 */
struct tag_symtab {
    struct tag_symtab *parent;
    symtab_entry *root;
};

typedef struct tag_symtab symtab;

/**
 * Structure that describes a list of identifiers.
 */
struct tag_symbol_ident_list {
    char **idents;
    int size;
};

typedef struct tag_symbol_ident_list symbol_ident_list;

/* Function prototypes */
symtab *symtab_create();
void symtab_push(symtab *);
symtab *symtab_pop();
symtab *symtab_tos();
symtab *symtab_parent();
symtab_entry *symtab_enter(const char *, symbol_type, astnode *, int);
symtab_entry *symtab_lookup(const char *);
symtab_entry *symtab_lookup_recursive(const char *);
symtab_entry *symtab_global_lookup(const char *);
int symtab_size();
int symtab_type_count(symbol_type);
void symtab_finalize(symtab *);
void symtab_remove(const char *);
void symtab_remove_by_type(symbol_type);
int symtab_list(symbol_ident_list *);
int symtab_list_type(symbol_type, symbol_ident_list *);
void symtab_list_finalize(symbol_ident_list *);
void symtab_print();

#endif  /* !SYMTAB_H */
