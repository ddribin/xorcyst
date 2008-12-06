/*
 * $Id: astnode.h,v 1.14 2007/08/12 18:59:10 khansen Exp $
 * $Log: astnode.h,v $
 * Revision 1.14  2007/08/12 18:59:10  khansen
 * ability to generate pure 6502 binary
 *
 * Revision 1.13  2007/08/09 22:06:00  khansen
 * general-purpose flags
 *
 * Revision 1.12  2007/08/07 21:12:16  khansen
 * const
 *
 * Revision 1.11  2007/07/22 13:35:20  khansen
 * convert tabs to whitespaces
 *
 * Revision 1.10  2004/12/29 21:44:23  kenth
 * xorcyst 1.4.2
 * added create_index()
 *
 * Revision 1.9  2004/12/19 20:46:49  kenth
 * xorcyst 1.4.0
 *
 * Revision 1.8  2004/12/19 09:53:56  kenth
 * added create_align()
 *
 * Revision 1.7  2004/12/18 16:56:55  kenth
 * create_extrn() takes unit id
 *
 * Revision 1.6  2004/12/16 13:19:28  kenth
 * changed astnode_create_label() signature
 *
 * Revision 1.5  2004/12/14 01:51:22  kenth
 * xorcyst 1.3.0
 *
 * Revision 1.4  2004/12/11 02:01:44  kenth
 * added forward/backward branching
 *
 * Revision 1.3  2004/12/09 11:17:41  kenth
 * added: warning, error nodes
 *
 * Revision 1.2  2004/12/06 04:54:00  kenth
 * xorcyst 1.1.0
 *
 * Revision 1.1  2004/06/30 07:56:09  kenth
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
 * Header file with definitions for Abstract Syntax Tree (AST) nodes.
 */
#ifndef ASTNODE_H
#define ASTNODE_H

#include "loc.h"

#define LHS(e) astnode_get_child(e, 0)
#define RHS(e) astnode_get_child(e, 1)

/**
 * The possible addressing modes for a parsed instruction.
 * Note that there is not a 1:1 correspondence between these and
 * the "real" 6502 addressing modes. Specifically, the parser doesn't
 * distinguish between absolute and relative mode, or between
 * 8-bit (zeropage) absolute addressing and 16-bit addressing.
 * That's for the code generator to figure out.
 */
enum tag_addressing_mode {
    IMPLIED_MODE=0,
    ACCUMULATOR_MODE,
    IMMEDIATE_MODE,
    ZEROPAGE_MODE,
    ZEROPAGE_X_MODE,
    ZEROPAGE_Y_MODE,
    ABSOLUTE_MODE,
    ABSOLUTE_X_MODE,
    ABSOLUTE_Y_MODE,
    PREINDEXED_INDIRECT_MODE,
    POSTINDEXED_INDIRECT_MODE,
    INDIRECT_MODE,
    RELATIVE_MODE,
    INVALID_MODE
};

typedef enum tag_addressing_mode addressing_mode;

/**
 * The possible instruction mnemonics.
 */
enum tag_instr_mnemonic {
    ADC_MNEMONIC=0,
    AND_MNEMONIC,
    ASL_MNEMONIC,
    BCC_MNEMONIC,
    BCS_MNEMONIC,
    BEQ_MNEMONIC,
    BIT_MNEMONIC,
    BMI_MNEMONIC,
    BNE_MNEMONIC,
    BPL_MNEMONIC,
    BRK_MNEMONIC,
    BVC_MNEMONIC,
    BVS_MNEMONIC,
    CLC_MNEMONIC,
    CLD_MNEMONIC,
    CLI_MNEMONIC,
    CLV_MNEMONIC,
    CMP_MNEMONIC,
    CPX_MNEMONIC,
    CPY_MNEMONIC,
    DEC_MNEMONIC,
    DEX_MNEMONIC,
    DEY_MNEMONIC,
    EOR_MNEMONIC,
    INC_MNEMONIC,
    INX_MNEMONIC,
    INY_MNEMONIC,
    JMP_MNEMONIC,
    JSR_MNEMONIC,
    LDA_MNEMONIC,
    LDX_MNEMONIC,
    LDY_MNEMONIC,
    LSR_MNEMONIC,
    NOP_MNEMONIC,
    ORA_MNEMONIC,
    PHA_MNEMONIC,
    PHP_MNEMONIC,
    PLA_MNEMONIC,
    PLP_MNEMONIC,
    ROL_MNEMONIC,
    ROR_MNEMONIC,
    RTI_MNEMONIC,
    RTS_MNEMONIC,
    SBC_MNEMONIC,
    SEC_MNEMONIC,
    SED_MNEMONIC,
    SEI_MNEMONIC,
    STA_MNEMONIC,
    STX_MNEMONIC,
    STY_MNEMONIC,
    TAX_MNEMONIC,
    TAY_MNEMONIC,
    TSX_MNEMONIC,
    TXA_MNEMONIC,
    TXS_MNEMONIC,
    TYA_MNEMONIC
};

typedef enum tag_instr_mnemonic instr_mnemonic;

/**
 * The possible types of a node in the abstract syntax tree.
 */
enum tag_astnode_type {
    NULL_NODE=0,
    INTEGER_NODE,
    STRING_NODE,
    IDENTIFIER_NODE,
    DATA_NODE,
    STORAGE_NODE,
    MACRO_DECL_NODE,
    MACRO_NODE,
    ARITHMETIC_NODE,
    IF_NODE,
    CASE_NODE,
    DEFAULT_NODE,
    IFDEF_NODE,
    IFNDEF_NODE,
    INCSRC_NODE,
    INCBIN_NODE,
    EQU_NODE,
    ASSIGN_NODE,
    ALIGN_NODE,
    INSTRUCTION_NODE,
    FILE_PATH_NODE,
    CURRENT_PC_NODE,
    LIST_NODE,
    LABEL_NODE,
    LOCAL_LABEL_NODE,
    LOCAL_ID_NODE,
    BINARY_NODE,
    PUBLIC_NODE,
    EXTRN_NODE,
    DATASEG_NODE,
    CODESEG_NODE,
    CHARMAP_NODE,
    STRUC_NODE,
    STRUC_DECL_NODE,
    UNION_DECL_NODE,
    ENUM_DECL_NODE,
    RECORD_DECL_NODE,
    BITFIELD_DECL_NODE,
    DOT_NODE,
    SIZEOF_NODE,
    DATATYPE_NODE,
    VAR_DECL_NODE,
    SCOPE_NODE,
    PROC_NODE,
    REPT_NODE,
    WHILE_NODE,
    MESSAGE_NODE,
    WARNING_NODE,
    ERROR_NODE,
    FORWARD_BRANCH_DECL_NODE,
    BACKWARD_BRANCH_DECL_NODE,
    FORWARD_BRANCH_NODE,
    BACKWARD_BRANCH_NODE,
    MASK_NODE,
    INDEX_NODE,
    ORG_NODE,
    TOMBSTONE_NODE
};

typedef enum tag_astnode_type astnode_type;

/**
 * The possible data types.
 */
enum tag_datatype {
    BYTE_DATATYPE = 0,
    CHAR_DATATYPE,
    WORD_DATATYPE,
    DWORD_DATATYPE,
    USER_DATATYPE   /* i.e. structure, union, enumeration, ... */
};

typedef enum tag_datatype datatype;

/**
 * The possible types of operators for an ARITHMETIC_NODE.
 */
enum tag_arithmetic_operator {
    PLUS_OPERATOR=0,
    MINUS_OPERATOR,
    MUL_OPERATOR,
    DIV_OPERATOR,
    MOD_OPERATOR,
    AND_OPERATOR,
    OR_OPERATOR,
    XOR_OPERATOR,
    SHL_OPERATOR,
    SHR_OPERATOR,
    LT_OPERATOR,
    GT_OPERATOR,
    EQ_OPERATOR,
    NE_OPERATOR,
    LE_OPERATOR,
    GE_OPERATOR,
    NEG_OPERATOR,
    NOT_OPERATOR,
    LO_OPERATOR,
    HI_OPERATOR,
    UMINUS_OPERATOR,
    BANK_OPERATOR
};

typedef enum tag_arithmetic_operator arithmetic_operator;

/**
 * Instruction attributes.
 */
struct tag_instruction_attribs {
    int mnemonic;
    addressing_mode mode;
    unsigned char opcode;
};

typedef struct tag_instruction_attribs instruction_attribs;

/**
 * Binary (byte buffer) attributes.
 */
struct tag_binary_attribs {
    unsigned char *data;
    int size;
};

typedef struct tag_binary_attribs binary_attribs;

/**
 * Structure that defines content of a node in the abstract syntax tree.
 */
struct tag_astnode {
    astnode_type type;
    union {
        int integer;    /* type == INTEGER_NODE */
        char *string;   /* type == STRING_NODE */
        char *ident;    /* type == IDENTIFIER_NODE */
        char *label;    /* type == LABEL_NODE */
        char *file_path;    /* type == FILE_PATH_NODE */
        binary_attribs binary;  /* type == BINARY_NODE */
        instruction_attribs instr;  /* type == INSTRUCTION_NODE */
        arithmetic_operator oper;   /* type == ARITHMETIC_NODE */
        datatype datatype;  /* type == DATATYPE_NODE */
        int modifiers;  /* type == DATASEG_NODE, VAR_DECL_NODE */
    /* The other node types have attributes stored as children,
    or can use this general-purpose field: */
        long param;
    };
    int flags;
    location loc;   /* File location where node was parsed */
    struct tag_astnode *prev_sibling;
    struct tag_astnode *next_sibling;
    struct tag_astnode *first_child;
    struct tag_astnode *parent;
};

typedef struct tag_astnode astnode;

/**
 * Function prototypes.
 */
astnode *astnode_create(astnode_type, location);
void astnode_finalize(astnode *);
void astnode_replace(astnode *, astnode *);
void astnode_remove(astnode *);
void astnode_insert_child(astnode *, astnode *, int);
int astnode_remove_child(astnode *, astnode *);
astnode *astnode_remove_child_at(astnode *, int);
astnode *astnode_remove_children(astnode *);
void astnode_add_sibling(astnode *, astnode *);
void astnode_add_child(astnode *, astnode *);
void astnode_add_children(astnode *, int, ...);
astnode *astnode_get_child(const astnode *, int);
astnode *astnode_get_first_child(const astnode *);
int astnode_get_child_count(const astnode *);
astnode *astnode_get_parent(const astnode *);
astnode *astnode_get_ancestor(const astnode *, int);
int astnode_has_ancestor_of_type(const astnode *, astnode_type);
int astnode_get_child_index(const astnode *, const astnode *);
astnode *astnode_get_last_sibling(const astnode *);
astnode_type astnode_get_type(const astnode *);
astnode *astnode_get_next_sibling(const astnode *);
astnode *astnode_get_prev_sibling(const astnode *);
void astnode_print(const astnode *, int);
astnode *astnode_clone(const astnode *, location);
//void astnode_serialize(astnode *, FILE *);
//astnode *astnode_deserialize(FILE *);
int astnode_equal(const astnode *, const astnode *);
int astnode_is_literal(const astnode *);
const char *astnode_type_to_string(astnode_type);
#define astnode_is_type(n, t) (astnode_get_type(n) == (t))

astnode *astnode_create_null(location);
astnode *astnode_create_instruction(int, addressing_mode, astnode *, location);
astnode *astnode_create_identifier(const char *, location);
astnode *astnode_create_integer(int, location);
astnode *astnode_create_string(const char *, location);
astnode *astnode_create_file_path(const char *, location);
astnode *astnode_create_arithmetic(arithmetic_operator, astnode *, astnode *, location);
astnode *astnode_create_if(astnode *, astnode *, astnode *, astnode *, location);
astnode *astnode_create_case(astnode *, astnode *, location);
astnode *astnode_create_default(astnode *, location);
astnode *astnode_create_ifdef(astnode *, astnode *, astnode *, location);
astnode *astnode_create_ifndef(astnode *, astnode *, astnode *, location);
astnode *astnode_create_macro_decl(astnode *, astnode *, astnode *, location);
astnode *astnode_create_macro(astnode *, astnode *, location);
astnode *astnode_create_equ(astnode *, astnode *, location);
astnode *astnode_create_assign(astnode *, astnode *, location);
astnode *astnode_create_incsrc(astnode *, location);
astnode *astnode_create_incbin(astnode *, location);
astnode *astnode_create_public(astnode *, location);
astnode *astnode_create_extrn(astnode *, astnode *, astnode *, location);
astnode *astnode_create_charmap(astnode *, location);
astnode *astnode_create_struc(astnode *, location);
astnode *astnode_create_struc_decl(astnode *, astnode *, location);
astnode *astnode_create_union_decl(astnode *, astnode *, location);
astnode *astnode_create_enum_decl(astnode *, astnode *, location);
astnode *astnode_create_record_decl(astnode *, astnode *, location);
astnode *astnode_create_bitfield_decl(astnode *, astnode *, location);
astnode *astnode_create_dot(astnode *, astnode *, location);
astnode *astnode_create_data(astnode *, astnode *, location);
astnode *astnode_create_storage(astnode *, astnode *, location);
astnode *astnode_create_dataseg(int, location);
astnode *astnode_create_codeseg(location);
astnode *astnode_create_pc(location);
astnode *astnode_create_label(const char *, astnode *, astnode *, location);
astnode *astnode_create_local_label(const char *, location);
astnode *astnode_create_local_id(const char *, location);
astnode *astnode_create_binary(unsigned char *, int, location);
astnode *astnode_create_list(astnode *);
astnode *astnode_create_sizeof(astnode *, location);
astnode *astnode_create_datatype(datatype, astnode *, location);
astnode *astnode_create_var_decl(int, astnode *, astnode *, location);
astnode *astnode_create_scope(astnode *, astnode *, location);
astnode *astnode_create_proc(astnode *, astnode *, location);
astnode *astnode_create_rept(astnode *, astnode *, location);
astnode *astnode_create_while(astnode *, astnode *, location);
astnode *astnode_create_message(astnode *, location);
astnode *astnode_create_warning(astnode *, location);
astnode *astnode_create_error(astnode *, location);
astnode *astnode_create_forward_branch_decl(const char *, location);
astnode *astnode_create_backward_branch_decl(const char *, location);
astnode *astnode_create_forward_branch(const char *, location);
astnode *astnode_create_backward_branch(const char *, location);
astnode *astnode_create_mask(astnode *, location);
astnode *astnode_create_align(astnode *, astnode *, location);
astnode *astnode_create_index(astnode *, astnode *, location);
astnode *astnode_create_org(astnode *, location);
astnode *astnode_create_tombstone(astnode_type type, location);

#endif  /* !ASTNODE_H */
