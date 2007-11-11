/*
 * $Id: astnode.c,v 1.15 2007/08/12 18:58:12 khansen Exp $
 * $Log: astnode.c,v $
 * Revision 1.15  2007/08/12 18:58:12  khansen
 * ability to generate pure 6502 binary (--pure-binary switch)
 *
 * Revision 1.14  2007/08/10 20:21:02  khansen
 * *** empty log message ***
 *
 * Revision 1.13  2007/08/09 22:05:49  khansen
 * general-purpose flags
 *
 * Revision 1.12  2007/08/07 21:12:16  khansen
 * const
 *
 * Revision 1.11  2007/07/22 13:33:26  khansen
 * convert tabs to whitespaces
 *
 * Revision 1.10  2004/12/29 21:44:04  kenth
 * xorcyst 1.4.2
 * added create_index()
 *
 * Revision 1.9  2004/12/19 19:58:23  kenth
 * xorcyst 1.4.0
 *
 * Revision 1.8  2004/12/19 09:53:46  kenth
 * added create_align()
 *
 * Revision 1.7  2004/12/18 16:56:12  kenth
 * create_extrn() takes unit id
 *
 * Revision 1.6  2004/12/16 13:19:07  kenth
 * astnode_create_label() takes datatype argument
 *
 * Revision 1.5  2004/12/14 01:48:57  kenth
 * xorcyst 1.3.0
 *
 * Revision 1.4  2004/12/11 02:01:10  kenth
 * added forward/backward branching
 *
 * Revision 1.3  2004/12/09 11:17:59  kenth
 * added: warning, error nodes
 *
 * Revision 1.2  2004/12/06 04:52:05  kenth
 * Major updates (xorcyst 1.1.0)
 *
 * Revision 1.1  2004/06/30 07:55:28  kenth
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
 * The result of parsing an assembly file is an Abstract Syntax Tree (AST).
 * Such a tree consists of AST nodes.
 * This file contains the code to manipulate such nodes.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "astnode.h"

#define SAFE_FREE(a) if (a) { free(a); a = NULL; }

/*---------------------------------------------------------------------------*/
/* Functions to convert and print astnodes as string.
   These are useful when debugging syntax trees.
*/

/**
 * Gets string representation of an addressing mode.
 * @param am Addressing mode (enumerated type)
 * @return String representation of am
 */
const char *addressing_mode_to_string(addressing_mode am)
{
    switch (am) {
        case IMPLIED_MODE:      return "IMPLIED_MODE";
        case ACCUMULATOR_MODE:      return "ACCUMULATOR_MODE";
        case IMMEDIATE_MODE:        return "IMMEDIATE_MODE";
        case ZEROPAGE_MODE:     return "ZEROPAGE_MODE";
        case ZEROPAGE_X_MODE:       return "ZEROPAGE_X_MODE";
        case ZEROPAGE_Y_MODE:       return "ZEROPAGE_Y_MODE";
        case ABSOLUTE_MODE:     return "ABSOLUTE_MODE";
        case ABSOLUTE_X_MODE:       return "ABSOLUTE_X_MODE";
        case ABSOLUTE_Y_MODE:       return "ABSOLUTE_Y_MODE";
        case PREINDEXED_INDIRECT_MODE:  return "PREINDEXED_INDIRECT_MODE";
        case POSTINDEXED_INDIRECT_MODE: return "POSTINDEXED_INDIRECT_MODE";
        case INDIRECT_MODE:     return "INDIRECT_MODE";
        case RELATIVE_MODE:     return "RELATIVE_MODE";
    }
    return "addressing_mode_to_string: invalid addressing mode";
}

/**
 * Gets string representation of an instruction mnemonic.
 * @param im Instruction mnemonic (enumerated type)
 * @return String representation of im
 */
const char *instr_mnemonic_to_string(instr_mnemonic im)
{
    switch (im) {
        case ADC_MNEMONIC: return "ADC_MNEMONIC";
        case AND_MNEMONIC: return "AND_MNEMONIC";
        case ASL_MNEMONIC: return "ASL_MNEMONIC";
        case BCC_MNEMONIC: return "BCC_MNEMONIC";
        case BCS_MNEMONIC: return "BCS_MNEMONIC";
        case BEQ_MNEMONIC: return "BEQ_MNEMONIC";
        case BIT_MNEMONIC: return "BIT_MNEMONIC";
        case BMI_MNEMONIC: return "BMI_MNEMONIC";
        case BNE_MNEMONIC: return "BNE_MNEMONIC";
        case BPL_MNEMONIC: return "BPL_MNEMONIC";
        case BRK_MNEMONIC: return "BRK_MNEMONIC";
        case BVC_MNEMONIC: return "BVC_MNEMONIC";
        case BVS_MNEMONIC: return "BVS_MNEMONIC";
        case CLC_MNEMONIC: return "CLC_MNEMONIC";
        case CLD_MNEMONIC: return "CLD_MNEMONIC";
        case CLI_MNEMONIC: return "CLI_MNEMONIC";
        case CLV_MNEMONIC: return "CLV_MNEMONIC";
        case CMP_MNEMONIC: return "CMP_MNEMONIC";
        case CPX_MNEMONIC: return "CPX_MNEMONIC";
        case CPY_MNEMONIC: return "CPY_MNEMONIC";
        case DEC_MNEMONIC: return "DEC_MNEMONIC";
        case DEX_MNEMONIC: return "DEX_MNEMONIC";
        case DEY_MNEMONIC: return "DEY_MNEMONIC";
        case EOR_MNEMONIC: return "EOR_MNEMONIC";
        case INC_MNEMONIC: return "INC_MNEMONIC";
        case INX_MNEMONIC: return "INX_MNEMONIC";
        case INY_MNEMONIC: return "INY_MNEMONIC";
        case JMP_MNEMONIC: return "JMP_MNEMONIC";
        case JSR_MNEMONIC: return "JSR_MNEMONIC";
        case LDA_MNEMONIC: return "LDA_MNEMONIC";
        case LDX_MNEMONIC: return "LDX_MNEMONIC";
        case LDY_MNEMONIC: return "LDY_MNEMONIC";
        case LSR_MNEMONIC: return "LSR_MNEMONIC";
        case NOP_MNEMONIC: return "NOP_MNEMONIC";
        case ORA_MNEMONIC: return "ORA_MNEMONIC";
        case PHA_MNEMONIC: return "PHA_MNEMONIC";
        case PHP_MNEMONIC: return "PHP_MNEMONIC";
        case PLA_MNEMONIC: return "PLA_MNEMONIC";
        case PLP_MNEMONIC: return "PLP_MNEMONIC";
        case ROL_MNEMONIC: return "ROL_MNEMONIC";
        case ROR_MNEMONIC: return "ROR_MNEMONIC";
        case RTI_MNEMONIC: return "RTI_MNEMONIC";
        case RTS_MNEMONIC: return "RTS_MNEMONIC";
        case SBC_MNEMONIC: return "SBC_MNEMONIC";
        case SEC_MNEMONIC: return "SEC_MNEMONIC";
        case SED_MNEMONIC: return "SED_MNEMONIC";
        case SEI_MNEMONIC: return "SEI_MNEMONIC";
        case STA_MNEMONIC: return "STA_MNEMONIC";
        case STX_MNEMONIC: return "STX_MNEMONIC";
        case STY_MNEMONIC: return "STY_MNEMONIC";
        case TAX_MNEMONIC: return "TAX_MNEMONIC";
        case TAY_MNEMONIC: return "TAY_MNEMONIC";
        case TSX_MNEMONIC: return "TSX_MNEMONIC";
        case TXA_MNEMONIC: return "TXA_MNEMONIC";
        case TXS_MNEMONIC: return "TXS_MNEMONIC";
        case TYA_MNEMONIC: return "TYA_MNEMONIC";
    }
    return "instr_mnemonic_to_string: invalid mnemonic";
}

/**
 * Gets string representation of an astnode type.
 * @param at Node type
 * @return String representation of at
 */
const char *astnode_type_to_string(astnode_type at) {
    switch (at) {
        case NULL_NODE:     return "NULL_NODE";
        case INTEGER_NODE:  return "INTEGER_NODE";
        case STRING_NODE:   return "STRING_NODE";
        case IDENTIFIER_NODE:   return "IDENTIFIER_NODE";
        case DATA_NODE:     return "DATA_NODE";
        case STORAGE_NODE:  return "STORAGE_NODE";
        case MACRO_DECL_NODE:   return "MACRO_DECL_NODE";
        case MACRO_NODE:    return "MACRO_NODE";
        case ARITHMETIC_NODE:   return "ARITHMETIC_NODE";
        case IF_NODE:       return "IF_NODE";
        case CASE_NODE:     return "CASE_NODE";
        case DEFAULT_NODE:  return "DEFAULT_NODE";
        case IFDEF_NODE:    return "IFDEF_NODE";
        case IFNDEF_NODE:   return "IFNDEF_NODE";
        case INCSRC_NODE:   return "INCSRC_NODE";
        case INCBIN_NODE:   return "INCBIN_NODE";
        case EQU_NODE:      return "EQU_NODE";
        case ASSIGN_NODE:   return "ASSIGN_NODE";
        case ALIGN_NODE:    return "ALIGN_NODE";
        case INSTRUCTION_NODE:  return "INSTRUCTION_NODE";
        case FILE_PATH_NODE:    return "FILE_PATH_NODE";
        case CURRENT_PC_NODE:   return "CURRENT_PC_NODE";
        case LIST_NODE:     return "LIST_NODE";
        case LABEL_NODE:    return "LABEL_NODE";
        case LOCAL_LABEL_NODE:  return "LOCAL_LABEL_NODE";
        case LOCAL_ID_NODE: return "LOCAL_ID_NODE";
        case BINARY_NODE:   return "BINARY_NODE";
        case PUBLIC_NODE:   return "PUBLIC_NODE";
        case EXTRN_NODE:    return "EXTRN_NODE";
        case DATASEG_NODE:  return "DATASEG_NODE";
        case CODESEG_NODE:  return "CODESEG_NODE";
        case CHARMAP_NODE:  return "CHARMAP_NODE";
        case STRUC_NODE:    return "STRUC_NODE";
        case STRUC_DECL_NODE:   return "STRUC_DECL_NODE";
        case UNION_DECL_NODE:   return "UNION_DECL_NODE";
        case ENUM_DECL_NODE:    return "ENUM_DECL_NODE";
        case RECORD_DECL_NODE:  return "RECORD_DECL_NODE";
        case BITFIELD_DECL_NODE:return "BITFIELD_DECL_NODE";
        case DOT_NODE:      return "DOT_NODE";
        case SIZEOF_NODE:   return "SIZEOF_NODE";
        case DATATYPE_NODE: return "DATATYPE_NODE";
        case VAR_DECL_NODE: return "VAR_DECL_NODE";
        case SCOPE_NODE:    return "SCOPE_NODE";
        case PROC_NODE:     return "PROC_NODE";
        case REPT_NODE:     return "REPT_NODE";
        case WHILE_NODE:    return "WHILE_NODE";
        case MESSAGE_NODE:  return "MESSAGE_NODE";
        case WARNING_NODE:  return "WARNING_NODE";
        case ERROR_NODE:    return "ERROR_NODE";
        case FORWARD_BRANCH_DECL_NODE:  return "FORWARD_BRANCH_DECL_NODE";
        case BACKWARD_BRANCH_DECL_NODE: return "BACKWARD_BRANCH_DECL_NODE";
        case FORWARD_BRANCH_NODE:   return "FORWARD_BRANCH_NODE";
        case BACKWARD_BRANCH_NODE:  return "BACKWARD_BRANCH_NODE";
        case MASK_NODE:     return "MASK_NODE";
        case INDEX_NODE:    return "INDEX_NODE";
        case ORG_NODE:      return "ORG_NODE";
        case TOMBSTONE_NODE:    return "TOMBSTONE_NODE";
    }
    return "astnode_type_to_string: invalid type";
}

/**
 * Gets string representation of a datatype.
 * @param dt Datatype
 * @return String representation of dt
 */
const char *datatype_to_string(const astnode *dt)
{
    switch (dt->datatype) {
        case BYTE_DATATYPE: return "BYTE_DATATYPE";
        case CHAR_DATATYPE: return "CHAR_DATATYPE";
        case WORD_DATATYPE: return "WORD_DATATYPE";
        case DWORD_DATATYPE:    return "DWORD_DATATYPE";
        case USER_DATATYPE: return "USER_DATATYPE"; // astnode_get_child(dt, 0)->ident;
    }
    return "datatype_to_string: invalid datatype";
};

/**
 * Gets string representation of an operator.
 * @param op Operator
 * @return String representation of op
 */
const char *operator_to_string(int op)
{
    switch (op) {
        case PLUS_OPERATOR: return "PLUS_OPERATOR";
        case MINUS_OPERATOR:    return "MINUS_OPERATOR";
        case MUL_OPERATOR:  return "MUL_OPERATOR";
        case DIV_OPERATOR:  return "DIV_OPERATOR";
        case MOD_OPERATOR:  return "MOD_OPERATOR";
        case AND_OPERATOR:  return "AND_OPERATOR";
        case OR_OPERATOR:   return "OR_OPERATOR";
        case XOR_OPERATOR:  return "XOR_OPERATOR";
        case SHL_OPERATOR:  return "SHL_OPERATOR";
        case SHR_OPERATOR:  return "SHR_OPERATOR";
        case LT_OPERATOR:   return "LT_OPERATOR";
        case GT_OPERATOR:   return "GT_OPERATOR";
        case EQ_OPERATOR:   return "EQ_OPERATOR";
        case NE_OPERATOR:   return "NE_OPERATOR";
        case LE_OPERATOR:   return "LE_OPERATOR";
        case GE_OPERATOR:   return "GE_OPERATOR";
        case NEG_OPERATOR:  return "NEG_OPERATOR";
        case NOT_OPERATOR:  return "NOT_OPERATOR";
        case LO_OPERATOR:   return "LO_OPERATOR";
        case HI_OPERATOR:   return "HI_OPERATOR";
        case UMINUS_OPERATOR:   return "UMINUS_OPERATOR";
        case BANK_OPERATOR: return "BANK_OPERATOR";
    }
    return "operator_to_string: invalid operator";
}

/**
 * Indents.
 * @param nlevels Levels
 */
void indent(int nlevels)
{
    int i;
    for (i=0; i<nlevels; i++) {
        printf("  ");
    }
}

/**
 * Prints a node recursively.
 * @param n Node to print
 * @param level Level (depth)
 */
void astnode_print(const astnode *n, int level)
{
    int i;
    /* Indent so it looks pretty */
    indent(level);
    /* Print the node type */
    printf(astnode_type_to_string(astnode_get_type(n)));
    /* Print attributes for those that have */
    switch (astnode_get_type(n)) {
        case INTEGER_NODE:  printf("(%d)", n->integer); break;
        case STRING_NODE:   printf("(\"%s\")", n->string);  break;
        case IDENTIFIER_NODE:   printf("(%s)", n->ident);   break;
        case LOCAL_ID_NODE: printf("(%s)", n->ident);   break;
        case FILE_PATH_NODE:    printf("(%s)", n->file_path);   break;
        case LABEL_NODE:    printf("(%s)", n->label);   break;
        case LOCAL_LABEL_NODE:  printf("(%s)", n->label);   break;
        case BINARY_NODE:   printf("(%d)", n->binary.size); break;

        case ARITHMETIC_NODE:
        printf(
            "(%s)",
            operator_to_string(n->oper)
        );
        break;

        case INSTRUCTION_NODE:
        printf(
            "(%s,%s,%.2X)",
            instr_mnemonic_to_string(n->instr.mnemonic),
            addressing_mode_to_string(n->instr.mode),
            n->instr.opcode
        );
        break;

        case DATATYPE_NODE:
        printf(
            "(%s)",
            datatype_to_string(n)
        );
        break;

        case FORWARD_BRANCH_DECL_NODE:
        case BACKWARD_BRANCH_DECL_NODE:
        case FORWARD_BRANCH_NODE:
        case BACKWARD_BRANCH_NODE:
        printf("(%s)", n->ident);
        break;

        case TOMBSTONE_NODE:
        printf(
            "(%s)",
            astnode_type_to_string(n->param)
        );
        break;

        default:
        /* Has no internal attributes */
        break;
    }
    printf("\n");
    /* Print the children */
    for (i=0; i<astnode_get_child_count(n); i++) {
        astnode_print(astnode_get_child(n, i), level+1);
    }
}

/*---------------------------------------------------------------------------*/
/* Functions for general-purpose node management:
   Creation, destruction, children etc.
*/

/**
 * Creates a new node of the given type.
 * @param type The node's type
 * @param loc File location
 * @return The newly created node
 */
astnode *astnode_create(astnode_type type, location loc)
{
    /* Fix: Sometimes loc.file is NULL for some reason */
    extern const char *yy_current_filename();
    if (loc.file == NULL) {
        loc.file = yy_current_filename();
    }
    /* Allocate memory for node struct */
    astnode *n = (astnode *)malloc(sizeof(astnode));
    /* Fill in struct only if alloc succeeded */
    if (n != NULL) {
        n->type = type;
        n->loc = loc;
        n->flags = 0;
        n->label = NULL;
        n->string = NULL;
        n->parent = n->first_child = n->prev_sibling = n->next_sibling = NULL;
    }
    return n;
}

/**
 * Finalizes a node.
 * Any children of the node are also finalized, recursively.
 * @param n The node to finalize.
 */
void astnode_finalize(astnode *n)
{
    /* Remove the node from the tree it's in. */
    astnode_remove(n);
    /* Finalize all its children recursively. */
    while (astnode_get_first_child(n) != NULL) {
        astnode_finalize(astnode_remove_child_at(n, 0));
    }
    /* Free up memory. */
    switch (astnode_get_type(n)) {
        case LABEL_NODE:    SAFE_FREE(n->label);    break;
        case LOCAL_LABEL_NODE:  SAFE_FREE(n->label);    break;
        case STRING_NODE:   SAFE_FREE(n->string);   break;
        case IDENTIFIER_NODE:   SAFE_FREE(n->ident);    break;
        case LOCAL_ID_NODE: SAFE_FREE(n->ident);    break;
        case FILE_PATH_NODE:    SAFE_FREE(n->file_path);break;
        case BINARY_NODE:   SAFE_FREE(n->binary.data); break;
        case FORWARD_BRANCH_DECL_NODE:
        case BACKWARD_BRANCH_DECL_NODE:
        case FORWARD_BRANCH_NODE:
        case BACKWARD_BRANCH_NODE:
        SAFE_FREE(n->ident);
        break;
        default:
        /* Has no internal attributes that are dynamically allocated */
        break;
    }
    SAFE_FREE(n);
}

/**
 * Gets the node's type.
 * @param n The node whose type to get
 * @return The node's type (astnode_type)
 */
astnode_type astnode_get_type(const astnode *n)
{
    return (n != NULL) ? n->type : NULL_NODE;
}

/**
 * Sets the parent field of all nodes in c to p.
 */
void astnode_set_parent(astnode *c, astnode *p)
{
    astnode *n;
    for (n = c; n != NULL; n = n->next_sibling) {
        n->parent = p;
    }
}

/**
 * Replaces a node with another.
 */
void astnode_replace(astnode *old_node, astnode *new_node)
{
    astnode *p;
    int i;
    /* Get the parent of the node to be replaced */
    p = astnode_get_parent(old_node);
    if (p != NULL) {
        /* Call remove_child on parent */
        i = astnode_remove_child(p, old_node);
        /* Insert new child at old child's position */
        astnode_insert_child(p, new_node, i);
    }
}

/**
 * Removes a node from a tree.
 * @param n The node to remove (can't be the root of the tree)
 */
void astnode_remove(astnode *n)
{
    astnode *p = astnode_get_parent(n);
    if (n && p) {
        astnode_remove_child(p, n);
    }
}

/**
 * Removes a child node.
 * @param p Parent node
 * @param c Child node
 * @return Index of the removed node
 */
int astnode_remove_child(astnode *p, astnode *c)
{
    int i;
    i = astnode_get_child_index(p, c);
    if (i == 0) {
        /* Remove head of list. */
        p->first_child = c->next_sibling;
        if (p->first_child) {
            p->first_child->prev_sibling = NULL;
        }
        c->parent = c->next_sibling = c->prev_sibling = NULL;
    }
    else if (i > 0) {
        c->prev_sibling->next_sibling = c->next_sibling;
        if (c->next_sibling) {
            c->next_sibling->prev_sibling = c->prev_sibling;
        }
        c->parent = c->next_sibling = c->prev_sibling = NULL;
    }
    return i;
}

/**
 * Removes child node at specified index.
 * @param p Parent node
 * @param i Index >= 0
 */
astnode *astnode_remove_child_at(astnode *p, int i)
{
    astnode *c = astnode_get_child(p, i);
    astnode_remove_child(p, c);
    return c;
}

/**
 * Removes all children from a node and returns them as a list.
 * @param p Parent node whose children to remove
 */
astnode *astnode_remove_children(astnode *p)
{
    astnode *c;
    if (p == NULL) { return NULL; }
    if (p->first_child != NULL) {
        c = p->first_child;
        p->first_child = NULL;
        /* Set parent of all siblings to NULL. */
        astnode_set_parent(c, NULL);
        /* Return the list of children */
        return c;
    }
    else {
        /* Has no children. */
        return NULL;
    }
}

/**
 * Inserts a list of nodes as children.
 *
 */
void astnode_insert_child(astnode *p, astnode *c, int i)
{
    astnode *n;
    astnode *x;
    if (p && c) {
        x = astnode_get_child(p, i);    /* Current child at that position */
        if (x == NULL) {
            /* There isn't a node here. Just add to end. */
            astnode_add_child(p, c);
        }
        else {
            n = astnode_get_last_sibling(c);
            /* Make c..n precede x */
            c->prev_sibling = x->prev_sibling;
            if (x->prev_sibling) {
                x->prev_sibling->next_sibling = c;
            }
            n->next_sibling = x;
            x->prev_sibling = n;
        }
        /* Set parent */
        astnode_set_parent(c, p);
        /* Check if head */
        if (i == 0) {
            p->first_child = c;
        }
    }
}

/**
 * Gets the last node in a list.
 */
astnode *astnode_get_last_sibling(const astnode *n)
{
    astnode *s = NULL;
    if (n) {
        for (s = (astnode *)n; s->next_sibling != NULL; s = s->next_sibling) ;
    }
    return s;
}

/**
 * Gets the parent of a node.
 * @param n The node whose parent to get
 * @return The node's parent, or <code>NULL</code> if it has none
 */
astnode *astnode_get_parent(const astnode *n)
{
    return n ? n->parent : NULL;
}

/**
 * Adds child(ren) to a node.
 * @param n The parent-to-be
 * @param new_child List of children-to-be
 */
void astnode_add_child(astnode *n, astnode *new_child)
{
    if (n && new_child) {
        if (n->first_child == NULL) {
            /* This node has no children, add this as the first one */
            n->first_child = new_child;
            astnode_set_parent(new_child, n);
        }
        else {
            astnode_add_sibling(n->first_child, new_child);
        }
    }
}

/**
 * Adds any number of children to a node.
 * @param n The parent-to-be
 */
void astnode_add_children(astnode *n, int count, ...)
{
    int i;
    va_list ap;
    astnode *c;

    va_start(ap, count);
    for (i=0; i<count; i++) {
        c = va_arg(ap, astnode*);
        astnode_add_child(n, c);
    }
    va_end(ap);
}

/**
 * Adds sibling(s) to a node.
 * @param brother List of existing siblings
 * @param sister List of new siblings
 */
void astnode_add_sibling(astnode *brother, astnode *sister)
{
    astnode *n;
    astnode *p;
    if (brother && sister) {
        /* Add to end of list */
        n = astnode_get_last_sibling(brother);
        n->next_sibling = sister;
        sister->prev_sibling = n;
        p = astnode_get_parent(brother);
        astnode_set_parent(sister, p);
    }
}

/**
 * Gets the child node at the specified index.
 * @param n The parent node
 * @param index The index of the desired child node
 */
astnode *astnode_get_child(const astnode *n, int index)
{
    int i;
    astnode *c;
    if (n) {
        c = n->first_child;
        for (i = 0; i != index; i++) {
            if (c == NULL) {
                /* No child at that index. */
                break;
            }
            c = c->next_sibling;
        }
        return c;
    }
    /* Node is NULL, so return NULL */
    return NULL;
}

/**
 * Gets a node's first child.
 * @param n The node
 */
astnode *astnode_get_first_child(const astnode *n)
{
    return (n == NULL) ? NULL : n->first_child;
}

/**
 * Gets the index of a child node.
 * @param p Parent node
 * @param c Child node
 * @return Index of c >= 0, or -1 if invalid input
 */
int astnode_get_child_index(const astnode *p, const astnode *c)
{
    int i;
    astnode *n;
    if (p && c) {
        for (i=0, n=p->first_child; (n != c) && (n != NULL); i++, n=n->next_sibling);
        return n ? i : -1;
    }
    return -1;
}

/**
 * Gets the number of children a node has.
 * @param p Node whose children count to get
 */
int astnode_get_child_count(const astnode *p)
{
    astnode *c;
    int count = 0;
    if (p != NULL) {
        for (c = p->first_child; c != NULL; count++, c = c->next_sibling);
    }
    return count;
}

/**
 * Clones a node and all its children.
 * @param n The node to clone
 * @param loc File location
 */
astnode *astnode_clone(const astnode *n, location loc)
{
    astnode *c;
    astnode *n_c;
    if (n == NULL) { return NULL; }
    /* Create node */
    c = astnode_create(astnode_get_type(n), loc);
    /* Copy attributes */
    switch (astnode_get_type(n)) {
        case INTEGER_NODE:
        c->integer = n->integer;
        break;

        case STRING_NODE:
        case IDENTIFIER_NODE:
        case FILE_PATH_NODE:
        case LABEL_NODE:
        case LOCAL_LABEL_NODE:
        case LOCAL_ID_NODE:
        c->string = (char *)malloc(strlen(n->string)+1);
        if (c->string != NULL) {
            strcpy(c->string, n->string);
        }
        break;

        case ARITHMETIC_NODE:
        c->oper = n->oper;
        break;

        case INSTRUCTION_NODE:
        c->instr = n->instr;
        break;

        case BINARY_NODE:
        c->binary = n->binary;
        break;

        case DATATYPE_NODE:
        c->datatype = n->datatype;
        break;

        default:
        c->param = n->param;
    }
    /* Clone children (TODO: OPTIMIZE THIS) */
    for (n_c=n->first_child; n_c != NULL; n_c=n_c->next_sibling) {
        astnode_add_child(c, astnode_clone(n_c, loc));
    }
    /* Return the clone */
    return c;
}

/**
 * Tests if two nodes are equal.
 */
int astnode_equal(const astnode *n1, const astnode *n2)
{
    int i;
    /* Verify that types are the same */
    if (astnode_get_type(n1) != astnode_get_type(n2)) {
        return 0;   /* Types don't match -- not equal */
    }
    /* Verify that internal data is the same */
    switch (astnode_get_type(n1)) {
        case ARITHMETIC_NODE:   if (n1->oper != n2->oper) return 0; break;
        case INTEGER_NODE:  if (n1->integer != n2->integer) return 0;   break;
        case STRING_NODE:   if (strcmp(n1->string, n2->string)) return 0;   break;
        case IDENTIFIER_NODE:   if (strcmp(n1->ident, n2->ident)) return 0; break;
        case LOCAL_ID_NODE: if (strcmp(n1->ident, n2->ident)) return 0; break;
        case FILE_PATH_NODE:    if (strcmp(n1->file_path, n2->file_path)) return 0; break;
        case LABEL_NODE:    if (strcmp(n1->label, n2->label)) return 0; break;
        case LOCAL_LABEL_NODE:  if (strcmp(n1->label, n2->label)) return 0; break;
        case BINARY_NODE:   if (n1->binary.size != n2->binary.size) return 0;   break;
        case DATATYPE_NODE: if (n1->datatype != n2->datatype) return 0; break;
        case TOMBSTONE_NODE:    if (n1->param != n2->param) return 0;   break;
        case INSTRUCTION_NODE:  if ( (n1->instr.mnemonic != n2->instr.mnemonic) || (n1->instr.mode != n2->instr.mode) ) return 0;   break;
        default:
        /* Has no internal attributes */
        break;
    }
    /* Verify that they have the same number of children */
    if (astnode_get_child_count(n1) != astnode_get_child_count(n2)) {
        return 0;
    }
    /* Verify that children are equal */
    for (i=0; i<astnode_get_child_count(n1); i++) {
        if (!astnode_equal(astnode_get_child(n1, i), astnode_get_child(n2, i))) {
            return 0;
        }
    }
    /* Equal. */
    return 1;
}

/**
 * Gets the ancestor of a node.
 * @param n Node whose ancestor to get
 * @param back How many generations to go back (0=father, 1=grandfather etc.)
 */
astnode *astnode_get_ancestor(const astnode *n, int back)
{
    int i;
    astnode *a = astnode_get_parent(n);
    for (i=0; i<back; i++) {
        a = astnode_get_parent(a);
    }
    return a;
}

/**
 * Tests if a node is a descendant of a node of a particular type.
 * @param n Node
 * @param type Ancestor's type
 * @return 0 if no such ancestor, 1 otherwise
 */
int astnode_has_ancestor_of_type(const astnode *n, astnode_type type)
{
    astnode *a;
    for (a = astnode_get_parent(n); a != NULL; a = astnode_get_parent(a) ) {
        if (astnode_is_type(a, type)) {
            return 1;
        }
    }
    return 0;
}

/**
 * Gets the next sibling of a node.
 * @param n Node
 */
astnode *astnode_get_next_sibling(const astnode *n)
{
    if (n == NULL) { return NULL; }
    return n->next_sibling;
}

/**
 * Gets the previous sibling of a node.
 * @param n Node
 */
astnode *astnode_get_prev_sibling(const astnode *n)
{
    if (n == NULL) { return NULL; }
    return n->prev_sibling;
}

/**
 * Tests if a node is a literal.
 * @param n Node to test
 */
int astnode_is_literal(const astnode *n)
{
    switch (astnode_get_type(n)) {
        case INTEGER_NODE:
        case STRING_NODE:
        /* A literal */
        return 1;

        default:
        /* Not a literal */
        break;
    }
    /* Not a literal */
    return 0;
}

/*---------------------------------------------------------------------------*/
/* Functions for creating AST nodes of specific type.
   1:1 correspondence between astnode_create_* and *_INSTRUCTION.
   Each takes the operands required for that node type,
   calls astnode_create() and then fills in fields and adds children (if any).
*/

astnode *astnode_create_null(location loc)
{
    /* Create the node */
    astnode *n = astnode_create(NULL_NODE, loc);
    /* Return the newly created node */
    return n;
}

/**
 * Creates a CPU instruction node.
 * @param mnemonic The instruction mnemonic
 * @param mode The addressing mode used
 * @param operand The instruction operand (an expression) (can be <code>NULL</code>)
 * @param loc File location
 */
astnode *astnode_create_instruction(int mnemonic, addressing_mode mode, astnode *operand, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(INSTRUCTION_NODE, loc);
    /* Store the mnemonic and addressing mode */
    n->instr.mnemonic = mnemonic;
    n->instr.mode = mode;
    /* This node has one child: The operand, which is an expression */
    astnode_add_child(n, operand);
    /* Return the newly created node */
    return n;
}

/**
 * Creates an identifier node.
 * @param ident The identifier (a string)
 * @param loc File location
 */
astnode *astnode_create_identifier(const char *ident, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(IDENTIFIER_NODE, loc);
    /* Allocate and store text */
    n->ident = (char *)malloc(strlen(ident)+1);
    if (n->ident != NULL) {
        strcpy(n->ident, ident);
    }
    /* Return the newly created node */
    return n;
}

/**
 * Creates an integer literal node.
 * @param value The integer literal
 * @param loc File location
 */
astnode *astnode_create_integer(int value, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(INTEGER_NODE, loc);
    /* Store the integer which this node represents */
    n->integer = value;
    /* Return the newly created node */
    return n;
}

/**
 * Creates a string literal node.
 * @param value The string literal
 * @param loc File location
 */
astnode *astnode_create_string(const char *value, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(STRING_NODE, loc);
    /* Allocate and store text */
    n->string = (char *)malloc(strlen(value)+1);
    if (n->string != NULL) {
        strcpy(n->string, value);
    }
    /* Return the newly created node */
    return n;
}

/**
 * Creates an expression node (unary or binary).
 * @param oper The operator
 * @param left Left operand
 * @param right Right operand (can be <code>NULL</code>)
 * @param loc File location
 */
astnode *astnode_create_arithmetic(arithmetic_operator oper, astnode *left, astnode *right, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(ARITHMETIC_NODE, loc);
    /* Store the operator, which describes the type of expression */
    n->oper = oper;
    /* This node has two children: left-hand side and right-hand side expression */
    /* For unary operators right-hand side should be <code>NULL</code> */
    astnode_add_children(n, 2, left, right);
    /* Return the newly created node */
    return n;
}

/**
 * Creates an if node.
 * @param expr The expression involved in the if
 * @param then The statement(s) to assemble when expr is non-zero
 * @param elif List of CASE nodes (may be <code>NULL</code>)
 * @param els The final else-part (DEFAULT node) (may be <code>NULL</code>)
 * @param loc File location
 */
astnode *astnode_create_if(astnode *expr, astnode *then, astnode *elif, astnode *els, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(IF_NODE, loc);
    /* This node has several children: List of CASE nodes, possibly ended by DEFAULT node */
    astnode_add_child(n, astnode_create_case(expr, then, loc) );
    astnode_add_child(n, elif);
    if (els != NULL) {
        astnode_add_child(n, astnode_create_default(els, loc));
    }
    /* Return the newly created node */
    return n;
}

/**
 * Creates a CASE node.
 * @param expr Expression
 * @param then List of statement to assemble when expr is non-zero (true)
 * @param loc File location
 */
astnode *astnode_create_case(astnode *expr, astnode *then, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(CASE_NODE, loc);
    /* This node has two children: expression to test and list of statements. */
    astnode_add_children(
        n,
        2,
        expr,
        astnode_create_list(then)
    );
    /* Return the newly created node */
    return n;
}

/**
 * Creates a DEFAULT node.
 * @param stmts List of statements
 * @param loc File location
 */
astnode *astnode_create_default(astnode *stmts, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(DEFAULT_NODE, loc);
    /* This node has list of statements as children. */
    astnode_add_child(
        n,
        stmts
    );
    /* Return the newly created node */
    return n;
}

/**
 * Creates an ifdef node.
 * @param ident The identifier to check
 * @param then The statement(s) to assemble when ident is defined
 * @param els The statement(s) to assemble when ident is not defined (can be <code>NULL</code>)
 * @param loc File location
 */
astnode *astnode_create_ifdef(astnode *ident, astnode *then, astnode *els, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(IFDEF_NODE, loc);
    /* This node has three children: identifier to test, then-part, else-part */
    astnode_add_children(
        n,
        3,
        ident,
        astnode_create_list(then),
        astnode_create_list(els)
    );
    /* Return the newly created node */
    return n;
}

/**
 * Creates an ifndef node.
 * @param ident The identifier to check
 * @param then The statement(s) to assemble when ident is not defined
 * @param els The statement(s) to assemble when ident is defined (can be <code>NULL</code>)
 * @param loc File location
 */
astnode *astnode_create_ifndef(astnode *ident, astnode *then, astnode *els, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(IFNDEF_NODE, loc);
    /* This node has three children: identifier to test, then-part, else-part */
    astnode_add_children(
        n,
        3,
        ident,
        astnode_create_list(then),
        astnode_create_list(els)
    );
    /* Return the newly created node */
    return n;
}

/**
 * Creates a macro declaration node.
 * @param ident Name of macro
 * @param params List of parameters (can be <code>NULL</code>)
 * @param body Macro body
 * @param loc File location
 */
astnode *astnode_create_macro_decl(astnode *ident, astnode *params, astnode *body, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(MACRO_DECL_NODE, loc);
    /* This node has three children:
    1) An identifier, which is the name of the macro
    2) List of parameters
    3) List of statements, which is the macro body */
    astnode_add_children(
        n,
        3,
        ident,
        astnode_create_list(params),
        astnode_create_list(body)
    );
    /* Return the newly created node */
    return n;
}

/**
 * Creates a macro node.
 * @param ident Name of macro
 * @param args List of arguments (can be <code>NULL</code>)
 * @param loc File location
 */
astnode *astnode_create_macro(astnode *ident, astnode *args, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(MACRO_NODE, loc);
    /* Add the children */
    astnode_add_children(
        n,
        2,
        ident,
        astnode_create_list(args)
    );
    /* Return the newly created node */
    return n;
}

/**
 * Creates an equ node.
 * @param ident Identifier
 * @param expr Expression
 * @param loc File location
 */
astnode *astnode_create_equ(astnode *ident, astnode *expr, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(EQU_NODE, loc);
    /* Add the children */
    astnode_add_children(n, 2, ident, expr);
    /* Return the newly created node */
    return n;
}

/**
 * Creates an assign node.
 * @param ident Identifier
 * @param expr Expression
 * @param loc File location
 */
astnode *astnode_create_assign(astnode *ident, astnode *expr, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(ASSIGN_NODE, loc);
    /* Add the children */
    astnode_add_children(n, 2, ident, expr);
    /* Return the newly created node */
    return n;
}

/**
 * Creates a storage node.
 * @param type Type of storage
 * @param count Expression with contains count
 * @param loc File location
 */
astnode *astnode_create_storage(astnode *type, astnode *count, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(STORAGE_NODE, loc);
    /* Add the type of data (enumerated or identifier) */
    astnode_add_child(n, type);
    /* Second child: Count */
    if (count == NULL) {
        /* No count given, default=1 */
        count = astnode_create_integer(1, loc);
    }
    astnode_add_child(n, count);
    /* Return the newly created node */
    return n;
}

/**
 * Creates an incsrc node.
 * @param file File specifier
 * @param loc File location
 */
astnode *astnode_create_incsrc(astnode *file, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(INCSRC_NODE, loc);
    /* One child: Path to file */
    astnode_add_child(n, file);
    /* Return the newly created node */
    return n;
}

/**
 * Creates an incbin node.
 * @param file File specifier
 * @param loc File location
 */
astnode *astnode_create_incbin(astnode *file, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(INCBIN_NODE, loc);
    /* One child: Path to file */
    astnode_add_child(n, file);
    /* Return the newly created node */
    return n;
}

/**
 * Creates a charmap node.
 * @param file File specifier
 * @param loc File location
 */
astnode *astnode_create_charmap(astnode *file, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(CHARMAP_NODE, loc);
    /* One child: Path to file */
    astnode_add_child(n, file);
    /* Return the newly created node */
    return n;
}

/**
 * Creates a structure (STRUC) instance node.
 * @param vals Values for the structure fields
 * @param loc File location
 */
astnode *astnode_create_struc(astnode *vals, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(STRUC_NODE, loc);
    /* Children: value list */
    astnode_add_child(n, vals);
    /* Return the newly created node */
    return n;
}
/**
 * Creates a structure (STRUC) declaration node.
 * @param id Structure identifier
 * @param stmts Statements of the structure declaration
 * @param loc File location
 */
astnode *astnode_create_struc_decl(astnode *id, astnode *stmts, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(STRUC_DECL_NODE, loc);
    /* Two children: Identifier, statement list */
    astnode_add_child(n, id);
    astnode_add_child(n, stmts);
    /* Return the newly created node */
    return n;
}

/**
 * Creates a union declaration node.
 * @param id Union identifier
 * @param stmts Statements of the union declaration
 * @param loc File location
 */
astnode *astnode_create_union_decl(astnode *id, astnode *stmts, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(UNION_DECL_NODE, loc);
    /* Two children: Identifier, statement list */
    astnode_add_child(n, id);
    astnode_add_child(n, stmts);
    /* Return the newly created node */
    return n;
}

/**
 * Creates an enum declaration node.
 * @param id Enum identifier
 * @param stmts Statements of the enum declaration
 * @param loc File location
 */
astnode *astnode_create_enum_decl(astnode *id, astnode *stmts, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(ENUM_DECL_NODE, loc);
    /* Two children: Identifier, statement list */
    astnode_add_child(n, id);
    astnode_add_child(n, stmts);
    /* Return the newly created node */
    return n;
}

/**
 * Creates a record declaration node.
 * @param id Record identifier
 * @param fields Fields of the record
 * @param loc File location
 */
astnode *astnode_create_record_decl(astnode *id, astnode *fields, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(RECORD_DECL_NODE, loc);
    /* Two children: Identifier, field list */
    astnode_add_child(n, id);
    astnode_add_child(n, fields);
    /* Return the newly created node */
    return n;
}

/**
 * Creates a bitfield declaration node.
 * @param id Identifier
 * @param width Width of field
 * @param loc Location
 */
astnode *astnode_create_bitfield_decl(astnode *id, astnode *width, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(BITFIELD_DECL_NODE, loc);
    /* Two children: Identifier and width */
    astnode_add_child(n, id);
    astnode_add_child(n, width);
    /* Return the newly created node */
    return n;
}

/**
 * Creates a public node.
 */
astnode *astnode_create_public(astnode *l, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(PUBLIC_NODE, loc);
    /* Add list of identifiers as child */
    astnode_add_child(n, l);
    /* Return the newly created node */
    return n;
}

/**
 * Creates an extrn node.
 * @param l List of identifiers
 * @param t Symbol type specifier
 * @param f From unit (identifier, may be <code>NULL</code>)
 */
astnode *astnode_create_extrn(astnode *l, astnode *t, astnode *f, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(EXTRN_NODE, loc);
    /* Add type specifier as child */
    astnode_add_child(n, t);
    /* Add list of identifiers as child */
    astnode_add_child(n, astnode_create_list(l));
    /* Add from unit identifier */
    astnode_add_child(n, f);
    /* Return the newly created node */
    return n;
}

/**
 * Creates a dataseg node.
 */
astnode *astnode_create_dataseg(int modifiers, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(DATASEG_NODE, loc);
    /* Set modifiers */
    n->modifiers = modifiers;
    /* Return the newly created node */
    return n;
}

/**
 * Creates a codeseg node.
 */
astnode *astnode_create_codeseg(location loc)
{
    /* Create the node */
    astnode *n = astnode_create(CODESEG_NODE, loc);
    /* Return the newly created node */
    return n;
}

/**
 * Creates a data node.
 * @param type Type specifier
 * @param data List of values
 * @param loc File location
 */
astnode *astnode_create_data(astnode *type, astnode *data, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(DATA_NODE, loc);
    /* Add the type of data (enumerated or identifier) */
    astnode_add_child(n, type);
    /* Add list of values */
    astnode_add_child(n, data);
    /* Return the newly created node */
    return n;
}

/**
 * Creates a file path node.
 * This is similar to a string literal node, the only difference is semantics.
 * A file path node implies that the path can be relative to both current
 * directory and any of the directories in the search path.
 * @param path The path this node represents
 * @param loc File location
 */
astnode *astnode_create_file_path(const char *path, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(FILE_PATH_NODE, loc);
    /* Allocate and store text */
    n->file_path = (char *)malloc(strlen(path)+1);
    if (n->file_path != NULL) {
        strcpy(n->file_path, path);
    }
    /* Return the newly created node */
    return n;
}

/**
 * Creates a (global) label node.
 * @param s Name of label
 * @param addr Address
 * @param type Datatype (may be <code>NULL</code>)
 * @param loc Location
 */
astnode *astnode_create_label(const char *s, astnode *addr, astnode *type, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(LABEL_NODE, loc);
    /* Allocate and store text */
    n->label = (char *)malloc(strlen(s)+1);
    if (n->label != NULL) {
        strcpy(n->label, s);
    }
    /* Two children: Datatype and address */
    if (addr == NULL) {
        addr = astnode_create_pc(loc);
    }
    if (type == NULL) {
        type = astnode_create_datatype(BYTE_DATATYPE, NULL, loc);
    }
    astnode_add_child(n, type);
    astnode_add_child(n, addr);
    /* Return the newly created node */
    return n;
}

/**
 * Creates a local label node.
 * @param s Name of label
 * @param loc Location
 */
astnode *astnode_create_local_label(const char *s, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(LOCAL_LABEL_NODE, loc);
    /* Allocate and store text */
    n->label = (char *)malloc(strlen(s)+1);
    if (n->label != NULL) {
        strcpy(n->label, s);
    }
    /* Return the newly created node */
    return n;
}

/**
 * Creates a local identifier node.
 * @param s Identifier
 * @param loc Location
 */
astnode *astnode_create_local_id(const char *s, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(LOCAL_ID_NODE, loc);
    /* Allocate and store text */
    n->ident = (char *)malloc(strlen(s)+1);
    if (n->ident != NULL) {
        strcpy(n->ident, s);
    }
    /* Return the newly created node */
    return n;
}

/**
 * Creates a list node.
 * This is a way to group a list of nodes in a parent node.
 * @param l List of nodes to group in list node
 */
astnode *astnode_create_list(astnode *l)
{
    astnode *n;
    location dummyloc;
    /* Create the node */
    if (l != NULL) {
        n = astnode_create(LIST_NODE, l->loc);
        /* Add list of values */
        astnode_add_child(n, l);
    }
    else {
        /* Make a node with zero children */
        n = astnode_create(LIST_NODE, dummyloc);
    }
    /* Return the newly created node (or NULL) */
    return n;
}

/**
 * Creates a PC node.
 * @param loc File location
 */
astnode *astnode_create_pc(location loc)
{
    return astnode_create(CURRENT_PC_NODE, loc);
}

/**
 * Creates a binary node.
 * @param bin Dynamically allocated (malloc() ) data that this node wraps. Will be freed automatically by astnode_finalize()
 * @param size Size of bin
 * @param loc File location
 */
astnode *astnode_create_binary(unsigned char *bin, int size, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(BINARY_NODE, loc);
    /* Set data */
    n->binary.data = bin;
    n->binary.size = size;
    /* Return the newly created node */
    return n;
}

/**
 * Creates a tombstone node, which is a marker node that says that another node
 * once lived here.
 * @param type The type of node that used to live here
 * @param loc File location
 */
astnode *astnode_create_tombstone(astnode_type type, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(TOMBSTONE_NODE, loc);
    /* Store the type of the old node */
    n->param = (long)type;
    /* Return the newly created node */
    return n;
}

/**
 * Creates a dot operator node.
 * Represents a structure field access of the form 'before.after'.
 * @param before Structure identifier
 * @param after Field identifier (can be another dot op, or an identifier)
 */
astnode *astnode_create_dot(astnode *before, astnode *after, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(DOT_NODE, loc);
    /* Two children: 'before' . 'after' */
    astnode_add_child(n, before);
    astnode_add_child(n, after);
    /* Return the newly created node */
    return n;
}

/**
 * Creates a sizeof operator node.
 * @param expr Expression (datatype?)
 * @param loc Location
 */
astnode *astnode_create_sizeof(astnode *expr, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(SIZEOF_NODE, loc);
    /* One child: expression */
    astnode_add_child(n, expr);
    /* Return the newly created node */
    return n;
}

/**
 * Creates a datatype node.
 * @param t The datatype this node represents
 * @param id If the datatype is a custom one, this is its name
 * @param loc Location
 */
astnode *astnode_create_datatype(datatype t, astnode *id, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(DATATYPE_NODE, loc);
    /* Set the datatype */
    n->datatype = t;
    /* Possibly one child: identifier */
    if (id != NULL) {
        astnode_add_child(n, id);
    }
    /* Return the newly created node */
    return n;
}

/**
 * Creates a variable declaration node.
 * @param modifiers PUBLIC_FLAG | ZEROPAGE_FLAG
 * @param id Identifier
 * @param data Datatype+initializer
 * @param loc Location
 */
astnode *astnode_create_var_decl(int modifiers, astnode *id, astnode *data, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(VAR_DECL_NODE, loc);
    /* Set modifiers */
    n->modifiers = modifiers;
    /* Two children: Identifier and datatype+initializer */
    astnode_add_child(n, id);
    astnode_add_child(n, data);
    /* Return the newly created node */
    return n;
}

/**
 *
 */
astnode *astnode_create_scope(astnode *left, astnode *right, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(SCOPE_NODE, loc);
    /* Two children: left and right */
    astnode_add_child(n, left);
    astnode_add_child(n, right);
    /* Return the newly created node */
    return n;
}

/**
 * Creates a procedure (PROC) node.
 * @param ident Name of procedure
 * @param stmts Procedure statements
 * @param loc File location
 */
astnode *astnode_create_proc(astnode *ident, astnode *stmts, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(PROC_NODE, loc);
    /* This node has two children:
    1) An identifier, which is the name of the procedure
    2) List of statements, which is the procedure body */
    astnode_add_children(
        n,
        2,
        ident,
        astnode_create_list(stmts)
    );
    /* Return the newly created node */
    return n;
}

/**
 * Creates a REPT node.
 * @param expr Number of times to repeat statements
 * @param stmts Statement list
 * @param loc File location
 */
astnode *astnode_create_rept(astnode *expr, astnode *stmts, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(REPT_NODE, loc);
    /* This node has two children:
    1) An expression, which is the repeat count
    2) List of statements, which is the (anonymous) macro body */
    astnode_add_children(
        n,
        2,
        expr,
        astnode_create_list(stmts)
    );
    /* Return the newly created node */
    return n;
}

/**
 * Creates a WHILE node.
 * @param expr Boolean expression
 * @param stmts Statement list
 * @param loc File location
 */
astnode *astnode_create_while(astnode *expr, astnode *stmts, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(WHILE_NODE, loc);
    /* This node has two children:
    1) A boolean expression
    2) List of statements, which is the (anonymous) macro body */
    astnode_add_children(
        n,
        2,
        expr,
        astnode_create_list(stmts)
    );
    /* Return the newly created node */
    return n;
}

/**
 * Creates a MESSAGE node.
 * @param expr Message to print.
 * @param loc File location
 */
astnode *astnode_create_message(astnode *expr, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(MESSAGE_NODE, loc);
    /* This node has one children: An expression, which is the message to print */
    astnode_add_child(n, expr);
    /* Return the newly created node */
    return n;
}

/**
 * Creates a WARNING node.
 * @param str Warning to print.
 * @param loc File location
 */
astnode *astnode_create_warning(astnode *str, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(WARNING_NODE, loc);
    /* This node has one child: A string, which is the warning to print */
    astnode_add_child(n, str);
    /* Return the newly created node */
    return n;
}

/**
 * Creates an ERROR node.
 * @param str Error to print.
 * @param loc File location
 */
astnode *astnode_create_error(astnode *str, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(ERROR_NODE, loc);
    /* This node has one child: A string, which is the error to print */
    astnode_add_child(n, str);
    /* Return the newly created node */
    return n;
}

/**
 * Creates a forward branch declaration node.
 * @param ident Branch name
 * @param loc File location
 */
astnode *astnode_create_forward_branch_decl(const char *ident, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(FORWARD_BRANCH_DECL_NODE, loc);
    /* Allocate and store text */
    n->ident = (char *)malloc(strlen(ident)+1);
    if (n->ident != NULL) {
        strcpy(n->ident, ident);
    }
    /* Return the newly created node */
    return n;
}

/**
 * Creates a backward branch declaration node.
 * @param ident Branch name
 * @param loc File location
 */
astnode *astnode_create_backward_branch_decl(const char *ident, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(BACKWARD_BRANCH_DECL_NODE, loc);
    /* Allocate and store text */
    n->ident = (char *)malloc(strlen(ident)+1);
    if (n->ident != NULL) {
        strcpy(n->ident, ident);
    }
    /* Return the newly created node */
    return n;
}

/**
 * Creates a forward branch reference node.
 * @param ident Branch name
 * @param loc File location
 */
astnode *astnode_create_forward_branch(const char *ident, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(FORWARD_BRANCH_NODE, loc);
    /* Allocate and store text */
    n->ident = (char *)malloc(strlen(ident)+1);
    if (n->ident != NULL) {
        strcpy(n->ident, ident);
    }
    /* Return the newly created node */
    return n;
}

/**
 * Creates a backward branch reference node.
 * @param ident Branch name
 * @param loc File location
 */
astnode *astnode_create_backward_branch(const char *ident, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(BACKWARD_BRANCH_NODE, loc);
    /* Allocate and store text */
    n->ident = (char *)malloc(strlen(ident)+1);
    if (n->ident != NULL) {
        strcpy(n->ident, ident);
    }
    /* Return the newly created node */
    return n;
}

/**
 * Creates a mask operator node.
 * @param expr Expression
 * @param loc Location
 */
astnode *astnode_create_mask(astnode *expr, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(MASK_NODE, loc);
    /* One child: expression */
    astnode_add_child(n, expr);
    /* Return the newly created node */
    return n;
}

/**
 * Creates an ALIGN node.
 * @param idents List of identifiers
 * @param expr Expression
 * @param loc File location
 */
astnode *astnode_create_align(astnode *idents, astnode *expr, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(ALIGN_NODE, loc);
    /* This node has two children: List of identifiers and alignment constraint */
    astnode_add_child(n, astnode_create_list(idents) );
    astnode_add_child(n, expr);
    /* Return the newly created node */
    return n;
}

/**
 * Creates an INDEX node.
 * @param ident Identifier being indexed
 * @param expr Index expression
 * @param loc File location
 */
astnode *astnode_create_index(astnode *ident, astnode *expr, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(INDEX_NODE, loc);
    /* This node has two children: Identifier and expression */
    astnode_add_child(n, ident);
    astnode_add_child(n, expr);
    /* Return the newly created node */
    return n;
}

/**
 * Creates an ORG node.
 * @param addr Address
 * @param loc File location
 */
astnode *astnode_create_org(astnode *addr, location loc)
{
    /* Create the node */
    astnode *n = astnode_create(ORG_NODE, loc);
    /* This node has one child: The address */
    astnode_add_child(n, addr);
    /* Return the newly created node */
    return n;
}

/*---------------------------------------------------------------------------*/
/* Functions for (de)serializing a node; probably not complete since it didn't
   end up being used by any program.
*/

#define put_byte(f, b) { fputc((unsigned char)(b), f); }
#define put_short(f, w) { put_byte(f, w >> 8); put_byte(f, w); }
#define put_int(f, q) { put_short(f, q >> 16); put_short(f, q); }

/**
 * Serializes a node.
 * @param n The node to serialize
 * @param f The file to write to
 */
void astnode_serialize(const astnode *n, FILE *f)
{
    int i;
    /* Node type: 1 byte */
    put_byte(f, astnode_get_type(n));
    /* Internal node data */
    switch (astnode_get_type(n)) {
        case INTEGER_NODE:
        /* Assumes that sizeof(long) is same as for int */
        put_int(f, n->integer);
        break;

        case STRING_NODE:
        case IDENTIFIER_NODE:
        case LOCAL_ID_NODE:
        case FILE_PATH_NODE:
        case LABEL_NODE:
        case LOCAL_LABEL_NODE:
        /* Put length first */
        put_int(f, strlen(n->string));
        /* Put characters */
        for (i=0; i<strlen(n->string); i++) {
            put_byte(f, n->string[i]);
        }
        break;

        case DATA_NODE:
        /* Datatype: 1 byte */
        put_byte(f, n->datatype);
        break;

        case BINARY_NODE:
        /* Size: 4 bytes */
        put_int(f, n->binary.size);
        /* Put data bytes */
        for (i=0; i<n->binary.size; i++) {
            put_byte(f, n->binary.data[i]);
        }
        break;

        case ARITHMETIC_NODE:
        // TODO
        break;

        case INSTRUCTION_NODE:
        // TODO
        break;

        default:
        /* No internal attributes */
        break;
    }
    /* Child count */
    put_int(f, astnode_get_child_count(n));
    /* Serialize children */
    for (i=0; i<astnode_get_child_count(n); i++) {
        astnode_serialize(astnode_get_child(n, i), f);
    }
}

#define get_byte(f) 0
#define get_short(f) 0
#define get_int(f) 0

/**
 * Deserializes a node.
 * @param f The file to read from
 */
astnode *astnode_deserialize(FILE *f)
{
    int i;
    int len;
    astnode *n;
    astnode_type t;
    location loc;
    /* Node type: 1 byte */
    t = (astnode_type)get_byte(f);
    /* Create the node */
    n = astnode_create(t, loc);
    /* Internal node data */
    switch (astnode_get_type(n)) {
        case INTEGER_NODE:
        /* Assumes that sizeof(long) is same as for int */
        n->integer = get_int(f);
        break;

        case STRING_NODE:
        case IDENTIFIER_NODE:
        case LOCAL_ID_NODE:
        case FILE_PATH_NODE:
        case LABEL_NODE:
        case LOCAL_LABEL_NODE:
        /* Get length */
        len = get_int(f);
        /* Create the character array */
        n->string = malloc(len+1);
        if (n->string != NULL) {
            /* Get characters */
            for (i=0; i<len; i++) {
                n->string[i] = get_byte(f);
            }
        }
        break;

        case DATA_NODE:
        /* Datatype: 1 byte */
        n->datatype = get_byte(f);
        break;

        case BINARY_NODE:
        /* Size: 4 bytes */
        n->binary.size = get_int(f);
        /* Allocate storage */
        n->binary.data = (unsigned char *)malloc(n->binary.size);
        if (n->binary.data != NULL) {
            /* Get data bytes */
            for (i=0; i<n->param; i++) {
                n->binary.data[i] = get_byte(f);
            }
        }
        break;

        case ARITHMETIC_NODE:
        // TODO
        break;

        case INSTRUCTION_NODE:
        // TODO
        break;

        default:
        /* No internal attributes */
        break;
    }
    /* Child count */
    len = get_int(f);
    /* Deserialize children */
    for (i=0; i<len; i++) {
        astnode_add_child(n, astnode_deserialize(f));
    }

    /* Return the deserialized node */
    return n;
}
