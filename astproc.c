/*
 * $Id: astproc.c,v 1.21 2007/11/11 22:35:22 khansen Exp $
 * $Log: astproc.c,v $
 * Revision 1.21  2007/11/11 22:35:22  khansen
 * compile on mac
 *
 * Revision 1.20  2007/08/19 10:17:39  khansen
 * allow symbols to be used without having been declared
 *
 * Revision 1.19  2007/08/12 18:58:12  khansen
 * ability to generate pure 6502 binary (--pure-binary switch)
 *
 * Revision 1.18  2007/08/12 02:42:46  khansen
 * prettify, const
 *
 * Revision 1.17  2007/08/09 22:06:10  khansen
 * ability to pass in reference to local label as argument to macro
 *
 * Revision 1.16  2007/08/09 20:48:46  khansen
 * disable buggy code that can cause crash
 *
 * Revision 1.15  2007/08/09 20:33:40  khansen
 * progress
 *
 * Revision 1.14  2007/08/08 22:40:01  khansen
 * improved symbol lookup, definitions must precede usage
 *
 * Revision 1.13  2007/07/22 13:33:26  khansen
 * convert tabs to whitespaces
 *
 * Revision 1.12  2005/01/09 11:17:57  kenth
 * xorcyst 1.4.5
 * fixed bug in process_data(), merge_data()
 * no longer truncation warning when fits in signed byte/word
 *
 * Revision 1.11  2005/01/05 02:28:13  kenth
 * xorcyst 1.4.3
 * support for anonymous unions
 * fixed sizeof bug
 *
 * Revision 1.10  2004/12/29 21:44:41  kenth
 * xorcyst 1.4.2
 * static indexing, sizeof improved
 *
 * Revision 1.9  2004/12/25 02:22:35  kenth
 * fixed bug in reduce_user_storage()
 *
 * Revision 1.8  2004/12/19 19:58:29  kenth
 * xorcyst 1.4.0
 *
 * Revision 1.7  2004/12/18 16:57:39  kenth
 * STORAGE_NODE(WORD/DWORD_DATATYPE) converts to BYTE
 *
 * Revision 1.6  2004/12/16 13:19:47  kenth
 * xorcyst 1.3.5
 *
 * Revision 1.5  2004/12/14 01:49:05  kenth
 * xorcyst 1.3.0
 *
 * Revision 1.4  2004/12/11 02:01:25  kenth
 * added forward/backward branching
 *
 * Revision 1.3  2004/12/09 11:18:13  kenth
 * added: warning, error node processing
 *
 * Revision 1.2  2004/12/06 04:52:24  kenth
 * Major updates (xorcyst 1.1.0)
 *
 * Revision 1.1  2004/06/30 07:55:31  kenth
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
 * This file contains functions that process the Abstract Syntax Tree (AST).
 * After the assembly file has been parsed into an AST, a number of passes are
 * made on it to process it and transform it. The functions here are
 * concerned with things like
 * - macro expansion
 * - symbol table generation
 * - equates substitution
 * - constant folding
 * - code and symbol validation
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include "astproc.h"
#include "symtab.h"
#include "opcode.h"
#include "charmap.h"
#include "xasm.h"

#define IS_SIGNED_BYTE_VALUE(v) (((v) >= -128) && ((v) <= 127))
#define IS_UNSIGNED_BYTE_VALUE(v) (((v) >= 0) && ((v) <= 255))
#define IS_BYTE_VALUE(v) (IS_SIGNED_BYTE_VALUE(v) || IS_UNSIGNED_BYTE_VALUE(v))

#define IS_SIGNED_WORD_VALUE(v) (((v) >= -32768) && ((v) <= 32767))
#define IS_UNSIGNED_WORD_VALUE(v) (((v) >= 0) && ((v) <= 65535))
#define IS_WORD_VALUE(v) (IS_SIGNED_WORD_VALUE(v) || IS_UNSIGNED_WORD_VALUE(v))

/*---------------------------------------------------------------------------*/

/** Number of errors issued during processing. */
static int err_count = 0;

/** Number of warnings issued during processing. */
static int warn_count = 0;

/* Keeps track of number of global labels encountered. */
static int label_count = 0;

/* Keeps track of whether statement is in dataseg or codeseg. */
static int in_dataseg = 0;

/* Default symbol modifiers, i.e. ZEROPAGE_FLAG, PUBLIC_FLAG */
static int modifiers = 0;

/* Used when we are outputting pure 6502 binary */
static int dataseg_pc;
static int codeseg_pc;

/*---------------------------------------------------------------------------*/

/** Mapping from regular ASCII characters to custom character values.
 * Used to transform .char arrays to regular .db arrays.
 */
static unsigned char charmap[256];

/**
 * Resets the custom character map.
 * Every ASCII character is mapped to itself.
 */
static void reset_charmap()
{
    int i;
    for (i=0; i<256; i++) {
        charmap[i] = (char)i;
    }
}

/*---------------------------------------------------------------------------*/
/* Forward/backward branching stuff */

struct tag_forward_branch_info {
    astnode *refs[128];
    int index;  /* Index into refs */
    int counter;
};

typedef struct tag_forward_branch_info forward_branch_info;

struct tag_backward_branch_info {
    astnode *decl;
    int counter;
};

typedef struct tag_backward_branch_info backward_branch_info;

#define BRANCH_MAX 8

static forward_branch_info forward_branch[BRANCH_MAX];

static backward_branch_info backward_branch[BRANCH_MAX];

/**
 * Zaps forward/backward branch data.
 */
static void branch_init()
{
    int i, j;
    for (i=0; i<BRANCH_MAX; i++) {
        for (j=0; j<128; j++) {
            forward_branch[i].refs[j] = NULL;
        }
        forward_branch[i].index = 0;
        forward_branch[i].counter = 0;
        backward_branch[i].decl = NULL;
        backward_branch[i].counter = 0;
    }
}

/*---------------------------------------------------------------------------*/

/**
 * Issues an error.
 * @param loc File location of error
 * @param fmt printf-style format string
 */
static void err(location loc, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    /* Print error message w/ location info */
    fprintf(stderr, "error: %s:", loc.file);
    LOCATION_PRINT(stderr, loc);
    fprintf(stderr, ": ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");

    va_end(ap);

    /* Increase total error count */
    err_count++;
}

/**
 * Issues a warning.
 * @param loc File location of warning
 * @param fmt printf-style format string
 */
static void warn(location loc, const char *fmt, ...)
{
    va_list ap;
    if (!xasm_args.no_warn) {
        va_start(ap, fmt);
        /* Print warning message w/ location info */
        fprintf(stderr, "warning: %s:", loc.file);
        LOCATION_PRINT(stderr, loc);
        fprintf(stderr, ": ");
        vfprintf(stderr, fmt, ap);
        fprintf(stderr, "\n");
        va_end(ap);
    }

    /* Increase total warning count */
    warn_count++;
}

/**
 * Gets the number of errors encountered during processing.
 * @return Number of errors
 */
int astproc_err_count()
{
    return err_count;
}

/*---------------------------------------------------------------------------*/

/**
 * Gets the processor function for a node type from a map.
 * Used by astproc_walk().
 * @param type The node type
 * @param map A mapping from node types to processor functions
 */
static astnodeproc astproc_node_type_to_proc(astnode_type type, const astnodeprocmap *map)
{
    /* Try all map entries */
    for (; map->proc != NULL; map += 1) {
        if (map->type == type) {
            return map->proc;   /* Match */
        }
    }
    /* No match */
    return NULL;
}

/*---------------------------------------------------------------------------*/

/**
 * Walks an abstract syntax tree recursively.
 * @param n Node to walk
 * @param arg Optional argument to pass to processor function
 * @param map Mapping of node types to processor functions
 */
static void astproc_walk_recursive(astnode *n, void *arg, const astnodeprocmap *map, astnode **next)
{
    astnode *c;
    astnode *t;
    if (n == NULL) { return; }
    /* Process this node if it has a processor function */
    astnodeproc p = astproc_node_type_to_proc(astnode_get_type(n), map);
    if (p != NULL) {
        if (!p(n, arg, next)) return;   /* Don't walk children */
    }
    /* Walk the node's children recursively */
    for (c=n->first_child; c != NULL; c = t) {
        t = c->next_sibling;    /* default next node */
        astproc_walk_recursive(c, arg, map, &t);
    }
}

/**
 * Generic tree walker function.
 * @param n Root
 * @param arg General-purpose argument passed to each node handler function
 * @param map Array of (nodetype, handler function) tuples
 */
void astproc_walk(astnode *n, void *arg, const astnodeprocmap *map)
{
    astnode *dummy;
    astproc_walk_recursive(n, arg, map, &dummy);
}

/*---------------------------------------------------------------------------*/

/**
 * Don't do any processing of this node or its children on this pass.
 */
static int noop(astnode *n, void *arg, astnode **next)
{
    return 0;
}

/**
 * Substitutes an identifier node with subst_expr if the id is equal to subst_id.
 * @param n A node of type IDENTIFIER_NODE
 * @param arg Array of length 2, containing (expr, id) pair
 */
static int substitute_id(astnode *n, void *arg, astnode **next)
{
    /* arg is array containing expression and identifier */
    astnode **array = (astnode **)arg;
    astnode *subst_expr = array[0];
    astnode *subst_id = array[1];
    /* Test if this node and the identifier to replace are equal */
    if (astnode_equal(n, subst_id)) {
        /* They're equal, replace it by expression. */
        astnode *cl = astnode_clone(subst_expr, n->loc);
        /* ### Generalize: traverse all children, set the flag */
        if (astnode_get_type(cl) == LOCAL_ID_NODE) {
            cl->flags |= 0x80; /* don't globalize it */
        }
        astnode_replace(n, cl);
        astnode_finalize(n);
        *next = cl;
        return 0;
    } else {
        return 1;
    }
}

/**
 * Substitutes expr for id in list.
 * Used by macro expander to substitute a macro body parameter name with the
 * actual expression used in the macro expansion.
 * @param expr An expression
 * @param id An identifier
 * @param list A list of statements (macro body)
 */
static void substitute_expr_for_id(astnode *expr, astnode *id, astnode *list)
{
    /* Prepare argument to astproc_walk */
    astnode *array[2];
    array[0] = expr;
    array[1] = id;
    /* Table of callback functions for our purpose. */
    static astnodeprocmap map[] = {
        { IDENTIFIER_NODE, substitute_id },
        { 0, NULL }
    };
    /* Do the walk. */
    astproc_walk(list, array, map);
}

/*---------------------------------------------------------------------------*/

/**
 * Globalizes a macro expanded local.
 * This is done simply by concatenating the local label identifier with the
 * global macro invocation counter.
 * @param n A node of type LOCAL_LABEL_NODE or LOCAL_ID_NODE
 * @param arg Namespace counter (int)
 */
static int globalize_macro_expanded_local(astnode *n, void *arg, astnode **next)
{
    /* Only globalize if it's a reference to a label defined in the macro */
    if (!(n->flags & 0x80)) {
        char str[16];
        int count;
        /* Make it global by appending the macro expansion counter to the id */
        count = (int)arg;
        sprintf(str, "#%d", count);
        if (astnode_is_type(n, LOCAL_LABEL_NODE)) {
            /* LOCAL_LABEL_NODE, use label field */
            n->label = realloc(n->label, strlen(n->label)+strlen(str)+1);
            strcat(n->label, str);
        } else {
            /* LOCAL_ID_NODE, use ident field */
            assert(astnode_is_type(n, LOCAL_ID_NODE));
            n->ident = realloc(n->ident, strlen(n->ident)+strlen(str)+1);
            strcat(n->ident, str);
        }
    }
    /* */
    return 1;
}

/**
 * Globalizes all locals in the body of a macro expansion.
 * Used by the macro expander to ensure that local labels in macro expansions
 * are unique.
 * @param exp_body The expanded macro body
 * @param count Unique macro namespace counter
 */
static void globalize_macro_expanded_locals(astnode *exp_body, int count)
{
    /* Table of callback functions for our purpose. */
    static astnodeprocmap map[] = {
        { LOCAL_ID_NODE, globalize_macro_expanded_local },
        { LOCAL_LABEL_NODE, globalize_macro_expanded_local },
        { 0, NULL }
    };
    /* Do the walk. */
    astproc_walk(exp_body, (void *)count, map);
}

/**
 * Expands a macro; that is, replaces a macro invocation in the AST with the
 * macro body. Substitutes parameter names for values.
 * @param n Must be a node of type MACRO_NODE
 * @param arg Not used
 */
static int expand_macro(astnode *n, void *arg, astnode **next)
{
    astnode *decl;
    astnode *decl_body;
    astnode *exp_body;
    astnode *formals;
    astnode *actuals;
    astnode *id;
    astnode *expr;
    int i;
    /* Keeps track of the current/total number of macro expansions */
    static int count = 0;
    /* Get the name of the macro to expand */
    id = astnode_get_child(n, 0);
    /* Look up its definition in symbol table */
    symtab_entry *e = symtab_lookup(id->ident);
    /* If it's not in the symbol table, error. */
    if (e == NULL) {
        err(n->loc, "unknown macro or directive `%s'", id->ident);
        /* Remove from AST */
        astnode_remove(n);
        astnode_finalize(n);
        return 0;
    }
    else if (e->type != MACRO_SYMBOL) {
        err(n->loc, "cannot expand `%s'; not a macro", e->id);
        /* Remove from AST */
        astnode_remove(n);
        astnode_finalize(n);
        return 0;
    }
    else {
        /* e->def has pointer to proper MACRO_DECL_NODE */
        decl = (astnode *)e->def;
        /* Get the lists of formals and actuals */
        formals = astnode_get_child(decl, 1);
        actuals = astnode_get_child(n, 1);
        /* Verify that argument count is correct */
        if (astnode_get_child_count(formals) != astnode_get_child_count(actuals)) {
            err(n->loc, "macro `%s' does not take %d argument(s)", id->ident, astnode_get_child_count(actuals) );
            /* Remove from AST */
            astnode_remove(n);
            astnode_finalize(n);
            return 0;
        }
        /* Expand the body */
        decl_body = astnode_get_child(decl, 2);
        exp_body = astnode_clone(decl_body, n->loc);
        assert(astnode_get_type(exp_body) == LIST_NODE);
        /* Substitute actuals for formals */
        for (i=0; i<astnode_get_child_count(actuals); i++) {
            /* The id to substitute */
            id = astnode_get_child(formals, i);
            /* The expression to substitute it with */
            expr = astnode_get_child(actuals, i);
            /* Do it! */
            substitute_expr_for_id(expr, id, exp_body);
        }
        /* Make locals a bit more global */
        globalize_macro_expanded_locals(exp_body, count);
        /* Replace MACRO_NODE by the macro body instance */
        {
            astnode *stmts = astnode_remove_children(exp_body);
            astnode_replace(n, stmts);
            *next = stmts;
            astnode_finalize(exp_body);
        }
        /* Discard the replaced node */
        astnode_finalize(n);
        /* Increase macro expansion counter */
        count++;
    }
    /* */
    return 0;
}

/*---------------------------------------------------------------------------*/

/**
 * Does constant folding of expression.
 * If the expression can be folded, the original expression is replaced by the
 * new one, and the original expression is finalized.
 * @param expr Expression
 * @return Original expression, if couldn't fold, otherwise new, folded expression
 */
astnode *astproc_fold_constants(astnode *expr)
{
    astnode *folded;
    astnode *lhs;
    astnode *rhs;
    if (expr == NULL) { return NULL; }
    folded = NULL;
    if (astnode_is_type(expr, ARITHMETIC_NODE)) {
        /* Fold operands recursively */
        lhs = astproc_fold_constants(LHS(expr));
        rhs = astproc_fold_constants(RHS(expr));
        switch (expr->oper) {
            /* Binary ops */
            case PLUS_OPERATOR:
            case MINUS_OPERATOR:
            case MUL_OPERATOR:
            case DIV_OPERATOR:
            case MOD_OPERATOR:
            case AND_OPERATOR:
            case OR_OPERATOR:
            case XOR_OPERATOR:
            case SHL_OPERATOR:
            case SHR_OPERATOR:
            case LT_OPERATOR:
            case GT_OPERATOR:
            case EQ_OPERATOR:
            case NE_OPERATOR:
            case LE_OPERATOR:
            case GE_OPERATOR:
            /* See if it can be folded */
            if ( (astnode_is_type(lhs, INTEGER_NODE)) &&
                (astnode_is_type(rhs, INTEGER_NODE)) ) {
                /* Both sides are integer literals, so fold. */
                switch (expr->oper) {
                    case PLUS_OPERATOR: folded = astnode_create_integer(lhs->integer + rhs->integer, expr->loc);    break;
                    case MINUS_OPERATOR:    folded = astnode_create_integer(lhs->integer - rhs->integer, expr->loc);    break;
                    case MUL_OPERATOR:  folded = astnode_create_integer(lhs->integer * rhs->integer, expr->loc);    break;
                    case DIV_OPERATOR:  folded = astnode_create_integer(lhs->integer / rhs->integer, expr->loc);    break;
                    case MOD_OPERATOR:  folded = astnode_create_integer(lhs->integer % rhs->integer, expr->loc);    break;
                    case AND_OPERATOR:  folded = astnode_create_integer(lhs->integer & rhs->integer, expr->loc);    break;
                    case OR_OPERATOR:   folded = astnode_create_integer(lhs->integer | rhs->integer, expr->loc);    break;
                    case XOR_OPERATOR:  folded = astnode_create_integer(lhs->integer ^ rhs->integer, expr->loc);    break;
                    case SHL_OPERATOR:  folded = astnode_create_integer(lhs->integer << rhs->integer, expr->loc);   break;
                    case SHR_OPERATOR:  folded = astnode_create_integer(lhs->integer >> rhs->integer, expr->loc);   break;
                    case LT_OPERATOR:   folded = astnode_create_integer(lhs->integer < rhs->integer, expr->loc);    break;
                    case GT_OPERATOR:   folded = astnode_create_integer(lhs->integer > rhs->integer, expr->loc);    break;
                    case EQ_OPERATOR:   folded = astnode_create_integer(lhs->integer == rhs->integer, expr->loc);   break;
                    case NE_OPERATOR:   folded = astnode_create_integer(lhs->integer != rhs->integer, expr->loc);   break;
                    case LE_OPERATOR:   folded = astnode_create_integer(lhs->integer <= rhs->integer, expr->loc);   break;
                    case GE_OPERATOR:   folded = astnode_create_integer(lhs->integer >= rhs->integer, expr->loc);   break;

                    default:
                    fprintf(stderr, "internal error: operator not handled in astproc_fold_constants()\n");
                    assert(0);
                    folded = expr;
                    break;
                }
                if (folded != expr) {
                    /* Replace expression by folded one. */
                    astnode_replace(expr, folded);
                    astnode_finalize(expr);
                    return folded;
                }
            }
            else if ( (astnode_is_type(lhs, STRING_NODE)) &&
                (astnode_is_type(rhs, STRING_NODE)) ) {
                /* Both sides are string literals. */
                /* Folding is defined only for certain operators. */
                switch (expr->oper) {
                    case PLUS_OPERATOR:
                    /* String concatenation. */
                    folded = astnode_create(STRING_NODE, expr->loc);
                    folded->string = (char *)malloc(strlen(lhs->string) + strlen(rhs->string) + 1);
                    if (folded->string != NULL) {
                        strcpy(folded->string, lhs->string);
                        strcat(folded->string, rhs->string);
                    }
                    break;

                    /* String comparison. */
                    case LT_OPERATOR:   folded = astnode_create_integer(strcmp(lhs->string, rhs->string) < 0, expr->loc);   break;
                    case GT_OPERATOR:   folded = astnode_create_integer(strcmp(lhs->string, rhs->string) > 0, expr->loc);   break;
                    case EQ_OPERATOR:   folded = astnode_create_integer(strcmp(lhs->string, rhs->string) == 0, expr->loc);  break;
                    case NE_OPERATOR:   folded = astnode_create_integer(strcmp(lhs->string, rhs->string) != 0, expr->loc);  break;
                    case LE_OPERATOR:   folded = astnode_create_integer(strcmp(lhs->string, rhs->string) <= 0, expr->loc);  break;
                    case GE_OPERATOR:   folded = astnode_create_integer(strcmp(lhs->string, rhs->string) >= 0, expr->loc);  break;

                    default:
                    folded = expr;
                    break;
                }
                if (folded != expr) {
                    /* Replace expression by folded one. */
                    astnode_replace(expr, folded);
                    astnode_finalize(expr);
                    return folded;
                }
            }
            else if ((astnode_get_type(lhs) == STRING_NODE) &&
                (astnode_get_type(rhs) == INTEGER_NODE) &&
                (expr->oper == PLUS_OPERATOR)) {
                /* Left side is string and right side is integer.
                Result is a string. */
                char str[32];
                sprintf(str, "%d", rhs->integer);
                folded = astnode_create(STRING_NODE, expr->loc);
                folded->string = (char *)malloc(strlen(lhs->string) + strlen(str) + 1);
                if (folded->string != NULL) {
                    strcpy(folded->string, lhs->string);
                    strcat(folded->string, str);
                }
                /* Replace expression by folded one. */
                astnode_replace(expr, folded);
                astnode_finalize(expr);
                return folded;
            }
            else if ((astnode_get_type(rhs) == STRING_NODE) &&
                (astnode_get_type(lhs) == INTEGER_NODE) &&
                (expr->oper == PLUS_OPERATOR)) {
                /* Left side is integer and right side is string.
                Result is a string. */
                char str[32];
                sprintf(str, "%d", lhs->integer);
                folded = astnode_create(STRING_NODE, expr->loc);
                folded->string = (char *)malloc(strlen(str) + strlen(rhs->string) + 1);
                if (folded->string != NULL) {
                    strcpy(folded->string, str);
                    strcat(folded->string, rhs->string);
                }
                /* Replace expression by folded one. */
                astnode_replace(expr, folded);
                astnode_finalize(expr);
                return folded;
            }
            /* Use some mathematical identities... */
            else if ((astnode_is_type(lhs, INTEGER_NODE) && (lhs->integer == 0))
                && (expr->oper == PLUS_OPERATOR)) {
                /* 0+expr == expr */
                astnode_remove_child(expr, rhs);
                astnode_replace(expr, rhs);
                astnode_finalize(expr);
                return rhs;
            }
            else if ((astnode_is_type(rhs, INTEGER_NODE) && (rhs->integer == 0))
                && (expr->oper == PLUS_OPERATOR)) {
                /* expr+0 == expr */
                astnode_remove_child(expr, lhs);
                astnode_replace(expr, lhs);
                astnode_finalize(expr);
                return lhs;
            }
            else if ((astnode_is_type(lhs, INTEGER_NODE) && (lhs->integer == 1))
                && (expr->oper == MUL_OPERATOR)) {
                /* 1*expr == expr */
                astnode_remove_child(expr, rhs);
                astnode_replace(expr, rhs);
                astnode_finalize(expr);
                return rhs;
            }
            else if ((astnode_is_type(rhs, INTEGER_NODE) && (rhs->integer == 1))
                && ((expr->oper == MUL_OPERATOR) || (expr->oper == DIV_OPERATOR)) ) {
                /* expr*1 == expr */
                /* expr/1 == expr */
                astnode_remove_child(expr, lhs);
                astnode_replace(expr, lhs);
                astnode_finalize(expr);
                return lhs;
            }
            else {
                /* No chance of folding this one. */
            }
            break;

            /* Unary ops */
            case NEG_OPERATOR:
            case NOT_OPERATOR:
            case LO_OPERATOR:
            case HI_OPERATOR:
            case UMINUS_OPERATOR:
            case BANK_OPERATOR:
            /* See if it can be folded */
            if (astnode_is_type(lhs, INTEGER_NODE)) {
                /* Fold it. */
                switch (expr->oper) {
                    case NEG_OPERATOR:  folded = astnode_create_integer(~lhs->integer, expr->loc);  break;
                    case NOT_OPERATOR:  folded = astnode_create_integer(!lhs->integer, expr->loc);  break;
                    case LO_OPERATOR:   folded = astnode_create_integer(lhs->integer & 0xFF, expr->loc);    break;
                    case HI_OPERATOR:   folded = astnode_create_integer((lhs->integer >> 8) & 0xFF, expr->loc); break;
                    case UMINUS_OPERATOR:   folded = astnode_create_integer(-lhs->integer, expr->loc);  break;
                    default: break;
                }
                /* Replace expression by folded one. */
                astnode_replace(expr, folded);
                astnode_finalize(expr);
                return folded;
            }
            else {
                /* Couldn't fold this one. */
            }
            break;
        }
    }
    /* Couldn't fold it, return original expression */
    return expr;
}

/*---------------------------------------------------------------------------*/

/**
 * Substitutes identifier if it has a constant definition in symbol table.
 * @param expr Node of type IDENTIFIER_NODE
 */
static astnode *substitute_ident(astnode *expr)
{
    astnode *c;
    symtab_entry *e;
    /* Look it up in symbol table */
    e = symtab_lookup(expr->ident);
    if (e != NULL) {
        /* Found it. Test if it's a define. */
        if (e->type == CONSTANT_SYMBOL) {
            /* This is a defined symbol that should be
            replaced by the expression it stands for */
            c = astnode_clone((astnode *)e->def, expr->loc);
            astnode_replace(expr, c);
            astnode_finalize(expr);
            expr = c;
        }
    }
    else {
        /* Didn't find it in symbol table. */
    }
    return expr;
}

/**
 * Substitutes sizeof with proper constant.
 * @param expr Node of type SIZEOF_NODE
 */
static astnode *reduce_sizeof(astnode *expr)
{
    int ok;
    astnode *c;
    astnode *id;
    astnode *type;
    astnode *count;
    symtab_entry *e;

    count = NULL;
    if (astnode_is_type(LHS(expr), IDENTIFIER_NODE)) {
        /* Identifier might be the name of a user-defined type, OR
           it might be the name of a variable of a user-defined type */
        type = NULL;
        /* Look it up */
        id = LHS(expr);
        e = symtab_global_lookup(id->ident);
        if (e != NULL) {
            switch (e->type) {
                case STRUC_SYMBOL:
                case UNION_SYMBOL:
                case RECORD_SYMBOL:
                case ENUM_SYMBOL:
                type = astnode_create_datatype(USER_DATATYPE, astnode_clone(id, id->loc), id->loc);
                break;

                case VAR_SYMBOL:
                type = astnode_clone(LHS(e->def), id->loc);
                if (astnode_is_type(e->def, STORAGE_NODE)) {
                    count = astnode_clone(RHS(e->def), id->loc);
                }
                else {
                    count = astnode_create_integer(astnode_get_child_count(e->def)-1, id->loc);
                }
                break;

                default:
                /* Can't take sizeof of this symbol type */
                break;
            }
        }
        if (type == NULL) {
            /* Unknown */
            type = astnode_create_datatype(USER_DATATYPE, astnode_clone(id, id->loc), id->loc);
        }
        /* Replace identifier by datatype node */
        astnode_replace(id, type);
        astnode_finalize(id);
    }
    type = LHS(expr);
    switch (type->datatype) {
        case BYTE_DATATYPE:
        case CHAR_DATATYPE:
        c = astnode_create_integer(1, expr->loc);
        astnode_replace(expr, c);
        astnode_finalize(expr);
        expr = c;
        break;

        case WORD_DATATYPE:
        c = astnode_create_integer(2, expr->loc);
        astnode_replace(expr, c);
        astnode_finalize(expr);
        expr = c;
        break;

        case DWORD_DATATYPE:
        c = astnode_create_integer(4, expr->loc);
        astnode_replace(expr, c);
        astnode_finalize(expr);
        expr = c;
        break;

        case USER_DATATYPE:
        /* Look up the data type in symbol table */
        id = LHS(type);
        e = symtab_global_lookup(id->ident);
        ok = 0;
        if (e != NULL) {
            switch (e->type) {
                case STRUC_SYMBOL:
                case UNION_SYMBOL:
                /* Datatype is defined, replace sizeof with proper expression */
                c = astnode_clone((astnode *)(e->struc.size), ((astnode *)(e->struc.size))->loc);
                astnode_replace(expr, c);
                astnode_finalize(expr);
                expr = c;
                ok = 1;
                break;

                case RECORD_SYMBOL:
                case ENUM_SYMBOL:
                /* 1 byte */
                c = astnode_create_integer(1, expr->loc);
                astnode_replace(expr,  c);
                astnode_finalize(expr);
                expr = c;
                ok = 1;
                break;

                default:
                /* Dunno the size of this symbol type */
                break;
            }
        }
        if (!ok) {
            /* Datatype not defined, error */
            err(expr->loc, "size of `%s' is unknown", id->ident);
            /* Replace by 1 */
            c = astnode_create_integer(1, expr->loc);
            astnode_replace(expr,  c);
            astnode_finalize(expr);
            return c;
        }
        break;

        default:
        err(expr->loc, "substitute_sizeof(): unknown type");
        break;
    }
    if (count != NULL) {
        c = astnode_create_arithmetic(
            MUL_OPERATOR,
            astnode_clone(expr, expr->loc),
            count,
            expr->loc
        );
        astnode_replace(expr,  c);
        astnode_finalize(expr);
        expr = c;
    }
    return expr;
}

/**
 * Substitutes A::B with an expression.
 * If A is a struct: substitute with offset of B
 * If A is a union: substitute with 0
 * If A is an enumeration: substitute with value for B
 * @param expr Node of type SCOPE_NODE
 */
static astnode *reduce_scope(astnode *expr)
{
    symtab_entry *ns;
    symtab_entry *sym;
    astnode *c;
    astnode *namespace;
    astnode *symbol;
    /* Look up the namespace */
    namespace = LHS(expr);
    ns = symtab_lookup(namespace->ident);
    if (ns != NULL) {
        /* Look up the local symbol */
        symtab_push(ns->symtab);
        symbol = RHS(expr);
        sym = symtab_lookup(symbol->ident);
        if (sym != NULL) {
            /* See if we can replace it */
            switch (ns->type) {
                case STRUC_SYMBOL:
                case UNION_SYMBOL:
                case RECORD_SYMBOL:
                /* Replace with field offset */
                c = astnode_clone(sym->field.offset, sym->field.offset->loc);
                astnode_replace(expr, c);
                astnode_finalize(expr);
                expr = c;
                break;

                case ENUM_SYMBOL:
                /* Replace with enum entry value */
                c = astnode_clone(sym->def, sym->def->loc);
                astnode_replace(expr, c);
                astnode_finalize(expr);
                expr = c;
                break;

                default:
                break;
            }
        }
        symtab_pop();
    }
    return expr;
}

static astnode *reduce_expression(astnode *expr);

/**
 * Handles remainder of fields in A.B.C.D . ..., where one or more fields may be indexed.
 * @param expr Node of type DOT_NODE, INDEX_NODE or IDENTIFIER_NODE
 */
static astnode *reduce_dot_recursive(astnode *expr)
{
    astnode *term;
    astnode *offset;
    astnode *left;
    astnode *right;
    astnode *type;
    symtab_entry *field;
    symtab_entry *def;
    astnode *index = NULL;
    /* Get identifiers involved: 'right' is field in 'left' */
    left = LHS(expr);
    if (astnode_is_type(left, INDEX_NODE)) {
        left = LHS(left);   /* Need identifier */
    }
    right = RHS(expr);
    if (astnode_is_type(right, DOT_NODE)) {
        right = LHS(right); /* Need identifier */
    }
    if (astnode_is_type(right, INDEX_NODE)) {
        index = RHS(right);
        right = LHS(right); /* Need identifier */
    }
    /* Lookup 'right' in 'left's symbol table (on stack) */
    field = symtab_lookup(right->ident);
    /* Look up variable's type definition */
    type = LHS(field->def);
    /* Copy its offset */
    offset = astnode_clone(field->field.offset, right->loc);
    if (index != NULL) {
        /* Create expression: identifier + sizeof(datatype) * index */
        offset = astnode_create_arithmetic(
            PLUS_OPERATOR,
            offset,
            astnode_create_arithmetic(
                MUL_OPERATOR,
                astnode_create_sizeof(astnode_clone(type, type->loc), expr->loc),
                astnode_clone(index, index->loc),
                index->loc
            ),
            expr->loc
        );
    }
    /* See if more subfields to process */
    expr = RHS(expr);
    if (astnode_is_type(expr, DOT_NODE)) {
        /* Next field */
        def = symtab_global_lookup(LHS(type)->ident);
        symtab_push(def->symtab);
        term = reduce_dot_recursive(expr);
        symtab_pop();
        /* Construct sum */
        offset = astnode_create_arithmetic(
            PLUS_OPERATOR,
            offset,
            term,
            expr->loc
        );
    }
    return offset;
}

/**
 * Transforms A.B.C.D . ... to A + offset(B) + offset(C) + ...
 * No error checking, since validate_dotref() should have been called previously.
 * @param expr Node of type DOT_NODE
 */
static astnode *reduce_dot(astnode *expr)
{
    symtab_entry *father;
    symtab_entry *def;
    astnode *type;
    astnode *left;
    astnode *term1;
    astnode *term2;
    astnode *sum;
    astnode *index = NULL;
    /* Look up parent in global symbol table */
    left = LHS(expr);   /* expr := left . right */
    if (astnode_is_type(left, INDEX_NODE)) {
        index = RHS(left);
        left = LHS(left);   /* Need identifier */
    }
    father = symtab_lookup(left->ident);
    /* Look up variable's type definition */
    type = LHS(father->def);    /* DATATYPE_NODE */
    def = symtab_lookup(LHS(type)->ident);
    /* 1st term of sum is the leftmost structure identifier */
    term1 = astnode_clone(left, left->loc);
    if (index != NULL) {
        /* Create expression: identifier + sizeof(datatype) * index */
        term1 = astnode_create_arithmetic(
            PLUS_OPERATOR,
            term1,
            astnode_create_arithmetic(
                MUL_OPERATOR,
                astnode_create_sizeof(astnode_clone(type, type->loc), expr->loc),
                astnode_clone(index, index->loc),
                index->loc
            ),
            expr->loc
        );
    }
    /* Add offsets recursively */
    symtab_push(def->symtab);
    term2 = reduce_dot_recursive(expr);
    symtab_pop();
    /* Calculate final sum */
    sum = astnode_create_arithmetic(
        PLUS_OPERATOR,
        term1,
        term2,
        expr->loc
    );
    sum = reduce_expression(sum);
    /* Replace dotted expression by sum */
    astnode_replace(expr, sum);
    astnode_finalize(expr);
    return sum;
}

/**
 * Reduces MASK operation to a field mask.
 * @param mask A node of type MASK_NODE
 */
static astnode *reduce_mask(astnode *mask)
{
    symtab_entry *ns;
    symtab_entry *sym;
    astnode *c;
    astnode *namespace;
    astnode *symbol;
    astnode *expr;
    /* Child is a scope node, record::field */
    expr = LHS(mask);
    /* Look up the namespace */
    namespace = LHS(expr);
    ns = symtab_lookup(namespace->ident);
    if (ns != NULL) {
        /* Make sure it's a record */
        if (ns->type != RECORD_SYMBOL) {
            err(expr->loc, "`%s' is not a record");
            /* Replace by 0 */
            c = astnode_create_integer(0, expr->loc);
            astnode_replace(mask, c);
            astnode_finalize(mask);
            expr = c;
        }
        else {
            /* Look up the local symbol */
            symtab_push(ns->symtab);
            symbol = RHS(expr);
            sym = symtab_lookup(symbol->ident);
            if (sym != NULL) {
                /* Calculate field mask */
                // mask = ((1 << width) - 1) << offset
                c = astnode_create_arithmetic(
                    SHL_OPERATOR,
                    astnode_create_arithmetic(
                        MINUS_OPERATOR,
                        astnode_create_arithmetic(
                            SHL_OPERATOR,
                            astnode_create_integer(1, expr->loc),
                            astnode_clone(sym->field.size, expr->loc),
                            expr->loc
                            ),
                        astnode_create_integer(1, expr->loc),
                        expr->loc
                    ),
                    astnode_clone(sym->field.offset, expr->loc),
                    expr->loc
                );
                c = reduce_expression(c);
                astnode_replace(mask, c);
                astnode_finalize(mask);
                expr = c;
            }
            symtab_pop();
        }
    }
    return expr;
}

/**
 * Reduces identifier[expression] to identifier + sizeof(identifier type) * expression
 */
static astnode *reduce_index(astnode *expr)
{
    symtab_entry *e;
    astnode *c;
    astnode *type;
    astnode *id;
    astnode *index;
    id = LHS(expr);
    index = reduce_expression(RHS(expr));
    /* Lookup identifier */
    e = symtab_lookup(id->ident);
    /* Get its datatype */
    type = LHS(e->def);
    /* Create expression: identifier + sizeof(datatype) * index */
    c = astnode_create_arithmetic(
        PLUS_OPERATOR,
        astnode_clone(id, id->loc),
        astnode_create_arithmetic(
            MUL_OPERATOR,
            astnode_create_sizeof(astnode_clone(type, type->loc), expr->loc),
            astnode_clone(index, index->loc),
            index->loc
        ),
        expr->loc
    );
    /* Replace index expression */
    astnode_replace(expr, c);
    astnode_finalize(expr);
    /* Return the new expression */
    return c;
}

/**
 * Substitutes all identifiers that represent EQU defines with their
 * corresponding expression.
 * @param expr The expression whose defines to substitute
 */
static astnode *substitute_defines(astnode *expr)
{
    switch (astnode_get_type(expr)) {
        case ARITHMETIC_NODE:
        substitute_defines(LHS(expr));
        substitute_defines(RHS(expr));
        break;

        case IDENTIFIER_NODE:
        expr = substitute_ident(expr);
        break;

        case SIZEOF_NODE:
        expr = reduce_sizeof(expr);
        break;

        case MASK_NODE:
        expr = reduce_mask(expr);
        break;

        case INDEX_NODE:
        substitute_defines(LHS(expr));
        substitute_defines(RHS(expr));
        break;

        case DOT_NODE:
        substitute_defines(LHS(expr));
        substitute_defines(RHS(expr));
        break;

        default:
        /* Nada */
        break;
    }
    return expr;
}

/**
 *
 */
static astnode *reduce_highlevel_constructs(astnode *expr)
{
    switch (astnode_get_type(expr)) {
        case ARITHMETIC_NODE:
        reduce_highlevel_constructs(LHS(expr));
        reduce_highlevel_constructs(RHS(expr));
        break;

        case SCOPE_NODE:
        expr = reduce_scope(expr);
        break;

        case DOT_NODE:
        expr = reduce_dot(expr);
        break;

        case INDEX_NODE:
        expr = reduce_index(expr);
        break;

        default:
        /* Nada */
        break;
    }
    return expr;
}

/**
 * Really reduces an expression.
 * @param expr Expression to attempt to reduce
 */
static astnode *reduce_expression_complete(astnode *expr)
{
    return astproc_fold_constants( reduce_highlevel_constructs( substitute_defines(expr) ) );
}

/**
 * Reduces an expression.
 * It does two things:
 * 1. Substitute all equates by their value
 * 2. Folds constants in the resulting expression
 * If the expression is reduced, the original expression is replaced by the
 * new one, the original is finalized, and a pointer to the new expression
 * is returned.
 * If the expression is not reduced, the original pointer is returned.
 */
static astnode *reduce_expression(astnode *expr)
{
    return astproc_fold_constants( substitute_defines(expr) );
}

/**
 * Reduces RECORD instance to a single byte (DB statement).
 * @param r Record's symbol table entry
 * @param expr Record initializer
 * @param flat List on which to append the reduced form
 */
static void reduce_record(symtab_entry *r, astnode *init, astnode *flat)
{
    ordered_field_list *list;
    symtab_entry *e;
    astnode *val;
    astnode *term;
    astnode *result;
    astnode *mask;
    astnode *repl;
    /* Validate initializer */
    if (!astnode_is_type(init, STRUC_NODE)) {
        err(init->loc, "record initializer expected");
        return;
    }
    /* Go through fields */
    symtab_push(r->symtab);
    result = astnode_create_integer(0, init->loc);
    for (val = init->first_child, list = r->struc.fields; (val != NULL) && (list != NULL); list = list->next, val = val->next_sibling) {
        if (astnode_is_type(val, NULL_NODE)) {
            continue;
        }
        if (astnode_is_type(val, STRUC_NODE)) {
            err(init->loc, "record field initializer expected");
            continue;
        }
        /* Get field definition */
        e = list->entry;
        /* Calculate field mask */
        // mask = ((1 << width) - 1) << offset
        mask = astnode_create_arithmetic(
            SHL_OPERATOR,
            astnode_create_arithmetic(
                MINUS_OPERATOR,
                astnode_create_arithmetic(
                    SHL_OPERATOR,
                    astnode_create_integer(1, val->loc),
                    astnode_clone(e->field.size, val->loc),
                    val->loc
                    ),
                astnode_create_integer(1, val->loc),
                val->loc
            ),
            astnode_clone(e->field.offset, val->loc),
            val->loc
        );
        /* Shift val left e->field.offset bits, AND with mask */
        term = astnode_create_arithmetic(
            AND_OPERATOR,
            astnode_create_arithmetic(
                SHL_OPERATOR,
                astnode_clone(val, val->loc),
                astnode_clone(e->field.offset, val->loc),
                val->loc
            ),
            mask,
            val->loc
        );
        /* OR the value with the result so far */
        result = astnode_create_arithmetic(
            OR_OPERATOR,
            result,
            term,
            val->loc
        );
        result = reduce_expression(result);
    }
    /* Determine reason for stopping loop */
    if (val != NULL) {
        err(init->loc, "too many field initializers");
    }
    /* Make byte data node (packed record value) */
    repl = astnode_create_data(
        astnode_create_datatype(BYTE_DATATYPE, NULL, init->loc),
        result,
        init->loc
    );
    /* Add to list */
    astnode_add_child(flat, repl);
    /* Restore old symbol table */
    symtab_pop();
}

/**
 * Reduces ENUM instance to DB.
 * @param e Enumeration's symbol table entry
 * @param expr Expression
 * @param flat List on which to append the reduced form
 */
static void reduce_enum(symtab_entry *e, astnode *expr, astnode *list)
{
    symtab_entry *sym;
    astnode *repl;
    if (!astnode_is_type(expr, IDENTIFIER_NODE)) {
        err(expr->loc, "identifier expected");
    }
    else {
        /* Look up the enumeration symbol */
        symtab_push(e->symtab);
        sym = symtab_lookup(expr->ident);
        symtab_pop();
        /* Make byte data node (symbol value) */
        repl = astnode_create_data(
            astnode_create_datatype(BYTE_DATATYPE, NULL, expr->loc),
            astnode_clone(sym->def, expr->loc),
            expr->loc
        );
        /* Add to list */
        astnode_add_child(list, repl);
    }
}

static void flatten_struc_recursive(symtab_entry *s, astnode *init, astnode *flat);

/**
 * Flattens a union initializer to a sequence of native data values.
 * Verify similar to flattening of structure, but only single field allowed.
 * @param s Union's symbol table definition
 * @param init Union initializer
 * @param flat List on which to append the flattened form
 */
static void flatten_union_recursive(symtab_entry *s, astnode *init, astnode *flat)
{
    astnode *fill;
    astnode *type;
    astnode *count;
    symtab_entry *e;
    symtab_entry *t;
    astnode *val;
    astnode *valvals;
    astnode *temp;
    ordered_field_list *list;
    int num;
    /* Validate initializer */
    if (!astnode_is_type(init, STRUC_NODE)) {
        err(init->loc, "union initializer expected");
        return;
    }
    /* Go through fields */
    symtab_push(s->symtab);
    fill = astnode_clone(s->struc.size, flat->loc);
    for (val = init->first_child, list = s->struc.fields; (val != NULL) && (list != NULL); list = list->next, val = val->next_sibling) {
        if (astnode_is_type(val, NULL_NODE)) {
            continue;
        }
        if (!astnode_equal(fill, s->struc.size)) {
            err(init->loc, "only one field of union can be initialized");
            continue;
        }
        /* Get field definition */
        e = list->entry;
        /* Symbol definition is STORAGE_NODE w/ two children: type and count */
        type = LHS(e->def);
        count = RHS(e->def);
        /* Decide what to do based on field type and value */
        switch (type->datatype) {
            case BYTE_DATATYPE:
            case CHAR_DATATYPE:
            case WORD_DATATYPE:
            case DWORD_DATATYPE:
            if (astnode_is_type(val, STRUC_NODE)) {
                /* Handle multi-value array */
                temp = astnode_clone(val, val->loc);
                valvals = astnode_remove_children(temp);
                astnode_finalize(temp);
                astnode_add_child(flat,
                    astnode_create_data(
                        astnode_create_datatype(type->datatype, NULL, type->loc),
                        valvals,
                        val->loc
                    )
                );
                num = astnode_get_child_count(val);
            } else {
                /* Output single value */
                astnode_add_child(flat,
                    astnode_create_data(
                        astnode_create_datatype(type->datatype, NULL, type->loc),
                        astnode_clone(val, val->loc),
                        val->loc
                    )
                );
                num = astnode_is_type(val, STRING_NODE) ? strlen(val->string) : 1;
            }
            if (num > count->integer) {
                err(val->loc, "initializer for field `%s' exceeds field size", e->id);
            }
            /* Fill in remainder of field if necessary: count - 1 */
            else if (count->integer > num) {
                astnode_add_child(flat,
                    astnode_create_storage(
                        astnode_create_datatype(type->datatype, NULL, type->loc),
                        astproc_fold_constants(
                            astnode_create_arithmetic(
                                MINUS_OPERATOR,
                                astnode_clone(count, count->loc),
                                astnode_create_integer(num, flat->loc),
                                count->loc
                            )
                        ),
                        val->loc
                    )
                );
            }
            break;

            case USER_DATATYPE:
            /* Look up user type definition */
            t = symtab_global_lookup(LHS(type)->ident);
            switch (t->type) {
                case STRUC_SYMBOL:
                flatten_struc_recursive(t, val, flat);
                break;

                case UNION_SYMBOL:
                flatten_union_recursive(t, val, flat);
                break;

                case RECORD_SYMBOL:
                reduce_record(t, val, flat);
                break;

                case ENUM_SYMBOL:
                reduce_enum(t, val, flat);
                break;

                default:
                break;
            }
            break;
        }
        /* Decrease fill amount according to field size */
        fill = astproc_fold_constants(
            astnode_create_arithmetic(
                MINUS_OPERATOR,
                fill,
                astnode_clone(e->field.size, flat->loc),
                flat->loc
            )
        );
    }
    /* Determine reason for stopping loop */
    if (val != NULL) {
        err(init->loc, "too many field initializers");
    }
    if (fill->integer > 0) {
        /* Fill remainder of union with zeroes */
        astnode_add_child(flat,
            astnode_create_storage(
                astnode_create_datatype(BYTE_DATATYPE, NULL, flat->loc),
                fill,
                flat->loc
            )
        );
    }
    symtab_pop();
}

/**
 * Flattens a structure initializer to a sequence of native data values.
 * @param s Structure's symbol table definition
 * @param init Structure initializer
 * @param flat List on which to append the flattened form
 */
static void flatten_struc_recursive(symtab_entry *s, astnode *init, astnode *flat)
{
    astnode *fill;
    astnode *type;
    astnode *count;
    astnode *temp;
    symtab_entry *e;
    symtab_entry *t;
    astnode *val;
    astnode *valvals;
    ordered_field_list *list;
    int num;
    /* Validate initializer */
    if (!astnode_is_type(init, STRUC_NODE)) {
        err(init->loc, "structure initializer expected");
        return;
    }
    /* Go through fields */
    symtab_push(s->symtab);
    fill = astnode_clone(s->struc.size, flat->loc);
    for (val = init->first_child, list = s->struc.fields; (val != NULL) && (list != NULL); list = list->next, val = val->next_sibling) {
        /* Get field definition */
        e = list->entry;
        /* Check if normal field or anonymous union */
        if (e->type == UNION_SYMBOL) {
            if (astnode_is_type(val, NULL_NODE)) {
                /* Output union size bytes to fill in field */
                astnode_add_child(flat,
                    astnode_create_storage(
                        astnode_create_datatype(BYTE_DATATYPE, NULL, val->loc),
                        astnode_clone(e->struc.size, val->loc),
                        val->loc
                    )
                );
            } else {
                flatten_union_recursive(e, val, flat);
                /* Decrease fill amount according to union size */
                fill = astproc_fold_constants(
                    astnode_create_arithmetic(
                        MINUS_OPERATOR,
                        fill,
                        astnode_clone(e->struc.size, flat->loc),
                        flat->loc
                    )
                );
            }
        } else {
            /* VAR_SYMBOL */
            /* Symbol definition is STORAGE_NODE w/ two children: type and count */
            type = LHS(e->def);
            count = RHS(e->def);
            /* Decide what to do based on field type and value */
            switch (type->datatype) {
                case BYTE_DATATYPE:
                case CHAR_DATATYPE:
                case WORD_DATATYPE:
                case DWORD_DATATYPE:
                if (astnode_is_type(val, NULL_NODE)) {
                    /* Output field_size bytes to fill in field */
                    astnode_add_child(flat,
                        astnode_create_storage(
                            astnode_create_datatype(type->datatype, NULL, type->loc),
                            astnode_clone(count, count->loc),
                            val->loc
                        )
                    );
                } else {
                    if (astnode_is_type(val, STRUC_NODE)) {
                        /* Handle multi-value array */
                        temp = astnode_clone(val, val->loc);
                        valvals = astnode_remove_children(temp);
                        astnode_finalize(temp);
                        astnode_add_child(flat,
                            astnode_create_data(
                                astnode_create_datatype(type->datatype, NULL, type->loc),
                                valvals,
                                val->loc
                            )
                        );
                        num = astnode_get_child_count(val);
                    } else {
                        /* Output single value */
                        astnode_add_child(flat,
                            astnode_create_data(
                                astnode_create_datatype(type->datatype, NULL, type->loc),
                                astnode_clone(val, val->loc),
                                val->loc
                            )
                        );
                        num = astnode_is_type(val, STRING_NODE) ? strlen(val->string) : 1;
                    }
                    if (astnode_is_type(count, INTEGER_NODE) && (count->integer < num)) {
                        err(val->loc, "initializer for field `%s' exceeds field size", e->id);
                    }
                    /* Fill in remainder of field if necessary: count - 1 */
                    else if ( (astnode_is_type(count, INTEGER_NODE) && (count->integer > num))
                    || !astnode_is_type(count, INTEGER_NODE) ) {
                        astnode_add_child(flat,
                            astnode_create_storage(
                                astnode_create_datatype(type->datatype, NULL, flat->loc),
                                astproc_fold_constants(
                                    astnode_create_arithmetic(
                                        MINUS_OPERATOR,
                                        astnode_clone(count, flat->loc),
                                        astnode_create_integer(num, flat->loc),
                                        flat->loc
                                    )
                                ),
                                flat->loc
                            )
                        );
                    }
                }
                break;

                case USER_DATATYPE:
                /* Look up user type definition */
                t = symtab_global_lookup(LHS(type)->ident);
                if (astnode_is_type(val, NULL_NODE)) {
                    /* Output sizeof(type) bytes to fill in */
                    astnode_add_child(flat,
                        astnode_create_storage(
                            astnode_create_datatype(BYTE_DATATYPE, NULL, val->loc),
                            astnode_clone(t->struc.size, val->loc),
                            val->loc
                        )
                    );
                } else {
                    switch (t->type) {
                        case STRUC_SYMBOL:
                        flatten_struc_recursive(t, val, flat);
                        break;

                        case UNION_SYMBOL:
                        flatten_union_recursive(t, val, flat);
                        break;

                        case RECORD_SYMBOL:
                        reduce_record(t, val, flat);
                        break;

                        case ENUM_SYMBOL:
                        reduce_enum(t, val, flat);
                        break;

                        default:
                        break;
                    }
                }
                break;
            }
            /* Decrease fill amount according to field size */
            fill = astproc_fold_constants(
                astnode_create_arithmetic(
                    MINUS_OPERATOR,
                    fill,
                    astnode_clone(e->field.size, flat->loc),
                    flat->loc
                )
            );
        }
    }
    /* Determine reason for stopping loop */
    if (val != NULL) {
        err(init->loc, "too many field initializers");
    }
    else if (list != NULL) {
        /* All fields not initialized; fill remainder of struc with zeroes */
        astnode_add_child(flat,
            astnode_create_storage(
                astnode_create_datatype(BYTE_DATATYPE, NULL, flat->loc),
                fill,
                flat->loc
            )
        );
    } else {
        astnode_finalize(fill);
    }
    symtab_pop();
}

/**
 * Converts data that is expressed in a high-level form (such as structure initializers)
 * to a simple sequence of bytes.
 * @param n The source node to flatten
 * @param type The type of data that n is an instance of
 * @param list List on which to append the resulting sequence of items (bytes/words/dwords)
 */
static void flatten_user_data(astnode *n, astnode *type, astnode *list)
{
    symtab_entry *def;
    /* Look up type definition */
    def = symtab_global_lookup(LHS(type)->ident);
    if (def != NULL) {
        switch (def->type) {
            case STRUC_SYMBOL:
            /* Flatten structure initializer to series of simple data statements */
            flatten_struc_recursive(def, n, list);
            break;

            case UNION_SYMBOL:
            /* Flatten union initializer to series of simple data statements */
            flatten_union_recursive(def, n, list);
            break;

            case RECORD_SYMBOL:
            reduce_record(def, n, list);
            break;

            case ENUM_SYMBOL:
            reduce_enum(def, n, list);
            break;

            default:
            break;
        }
    }
}

/*---------------------------------------------------------------------------*/

/**
 * Loads the character map specified by the node.
 * @param n Node of type CHARMAP_NODE
 */
static int load_charmap(astnode *n, void *arg, astnode **next)
{
    /* TODO: should probably be done in the parsing phase (same path resolution as for INCSRC and INCBIN) */
    astnode *file;
    /* Get file descriptor */
    file = astnode_get_child(n, 0);
    /* Try to load the charmap */
    if (charmap_parse(file->file_path, charmap) == 0) {
        err(n->loc, "could not open `%s' for reading", file->file_path);
    }
    return 0;
}

/**
 * First-time processing of instruction node.
 * @param n Node of type INSTRUCTION_NODE
 * @param arg Not used
 */
static int process_instruction(astnode *n, void *arg, astnode **next)
{
    astnode *expr;
    if (in_dataseg) {
        err(n->loc, "instructions not allowed in data segment");
        /* Remove from AST */
        astnode_remove(n);
        astnode_finalize(n);
        return 0;
    }
    else {
        /* The instruction operand */
        expr = astnode_get_child(n, 0);
        /* Substitute defines and fold constants */
        reduce_expression(expr);
        return 1;
    }
}

/**
 * First-time processing of data node.
 * @param n Node of type DATA_NODE
 * @param arg Not used
 */
static int process_data(astnode *n, void *arg, astnode **next)
{
    int j;
    int k;
    astnode *type;
    astnode *expr;
    astnode *list;
    astnode *stmts;
    type = astnode_get_child(n, 0);
    assert(astnode_is_type(type, DATATYPE_NODE));
    if (in_dataseg) {
        err(n->loc, "value not allowed in data segment");
        /* Replace with storage node  */
        astnode_replace(
            n,
            astnode_create_storage(
                astnode_create_datatype(BYTE_DATATYPE, NULL, n->loc),
                astnode_create_integer(1, n->loc),
                n->loc
            )
        );
        astnode_finalize(n);
        return 0;
    }
    if (type->datatype == USER_DATATYPE) {
        /* Make sure the type exists */
        if (symtab_global_lookup(LHS(type)->ident) == NULL) {
            err(n->loc, "unknown type `%s'", LHS(type)->ident);
            /* Remove from AST */
            astnode_remove(n);
            astnode_finalize(n);
            return 0;
        } else {
            /* Attempt to reduce user data to native data */
            list = astnode_create(LIST_NODE, n->loc);
            for (expr = type->next_sibling; expr != NULL; expr = expr->next_sibling) {
                flatten_user_data(expr, type, list);
            }
            /* Replace initializers with generated list */
            stmts = astnode_remove_children(list);
            astnode_replace(n, stmts);
            astnode_finalize(n);
            astnode_finalize(list);
            *next = stmts;
            return 0;
        }
    }
    /* Go through the list of data values, replacing defines and folding constants */
    for (j=1; j<astnode_get_child_count(n); j++) {
        expr = astnode_get_child(n, j);
        /* Substitute defines and fold constants */
        expr = reduce_expression(expr);
        /* If it's a string, replace by array of integers */
        /* (makes it easier to process later... favour regularity) */
        if (astnode_is_type(expr, STRING_NODE)) {
            astnode_remove_child(n, expr);  /* Remove string */
            for (k=strlen(expr->string)-1; k>=0; k--) {
                /* Check if we should map character from custom charmap */
                if (type->datatype == CHAR_DATATYPE) {
                    expr->string[k] = charmap[(unsigned)expr->string[k]];
                }
                /* Append character value to array */
                astnode_insert_child(n, astnode_create_integer((unsigned char)expr->string[k], n->loc), j);
            }
            if (type->datatype == CHAR_DATATYPE) {
                /* It's normal byte array now */
                type->datatype = BYTE_DATATYPE;
            }
            j += strlen(expr->string)-1;
            astnode_finalize(expr);
        }
    }
    return 1;
}

/**
 * First-time processing of storage node.
 * @param n Node of type STORAGE_NODE
 * @param arg Not used
 */
static int process_storage(astnode *n, void *arg, astnode **next)
{
    int item_size;
    astnode *type;
    astnode *expr;
    astnode *new_expr;
    type = LHS(n);
    expr = RHS(n);
    /* If not BYTE_DATATYPE, multiply by word/dword-size */
    switch (type->datatype) {
        case BYTE_DATATYPE:
        case CHAR_DATATYPE: item_size = 1; break;
        case WORD_DATATYPE: item_size = 2;  break;
        case DWORD_DATATYPE:    item_size = 4;  break;
        default:  item_size = 1; break; // ### Hmmm...
    }
    if (item_size != 1) {
        new_expr = astnode_create_arithmetic(
            MUL_OPERATOR,
            astnode_clone(expr, expr->loc),
            astnode_create_integer(item_size, expr->loc),
            expr->loc
        );
        astnode_replace(expr, new_expr);
        astnode_finalize(expr);
        expr = new_expr;
        type->datatype = BYTE_DATATYPE;
    }
    /* Substitute defines and fold constants */
    expr = reduce_expression(expr);
    // TODO: Validate range somewhere else than here please... ???
    if (astnode_is_type(expr, INTEGER_NODE)) {
        if ((expr->integer <= 0) || (expr->integer >= 0x10000)) {
            err(n->loc, "operand out of range");
        }
    }
    return 1;
}

/**
 * Process EQU node.
 * @param n Node of type EQU_NODE
 * @param arg Not used
 */
static int process_equ(astnode *n, void *arg, astnode **next)
{
    symtab_entry *e;
    astnode *id;
    astnode *expr;
    /* The expression which describes the value */
    expr = astnode_clone(astnode_get_child(n, 1), n->loc);
    /* Substitute defines and fold constants */
    expr = reduce_expression(expr);
    /* The identifier which is being defined */
    id = astnode_get_child(n, 0);
    /* Look up in symbol table */
    e = symtab_lookup(id->ident);
    if (e == NULL) {
        /* Symbol is being defined */
        // TODO: Check that expression is a constant?
        /* Enter it in symbol table */
        symtab_enter(id->ident, CONSTANT_SYMBOL, expr, 0);
    } else {
        /* Symbol is being redefined */
        /* This is not allowed for EQU equate! */
        if (!astnode_equal((astnode *)(e->def), expr)) {
            warn(n->loc, "redefinition of `%s' is not identical; ignored", id->ident);
        }
    }
    /* Remove the equate node from the tree. */
    astnode_remove(n);
    astnode_finalize(n);
    return 0;
}

/**
 * Process '=' node.
 * @param n Node of type ASSIGN_NODE
 * @param arg Not used
 */
static int process_assign(astnode *n, void *arg, astnode **next)
{
    symtab_entry *e;
    astnode *id;
    astnode *expr;
    /* If it's part of ENUM declaration, don't touch */
    if (astnode_has_ancestor_of_type(n, ENUM_DECL_NODE)) {
        return 0;
    }
    /* Very similar to EQU, except symbol 1) can be
    redefined and 2) is volatile (see end of proc) */
    /* The expression which describes the value */
    expr = astnode_clone(astnode_get_child(n, 1), n->loc);
    /* Substitute defines and fold constants */
    expr = reduce_expression(expr);
    /* The identifier which is being (re)defined */
    id = astnode_get_child(n, 0);
    /* Look up in symbol table */
    e = symtab_lookup(id->ident);
    if (e == NULL) {
        /* Symbol is being defined for the first time */
        /* Note that the VOLATILE_FLAG is set */
        symtab_enter(id->ident, CONSTANT_SYMBOL, expr, VOLATILE_FLAG);
    } else {
        /* Symbol is being redefined */
        /* This is OK for ASSIGN equate, simply replace definition */
        // ### store a list of definitions
        expr->loc = e->def->loc;
        e->def = expr;
    }
    /* Remove the equate node from the tree. */
    astnode_remove(n);
    astnode_finalize(n);
    return 0;
}

/**
 * Process IFDEF-node.
 * @param n Node of type IFDEF_NODE
 * @param arg Not used
 */
static int process_ifdef(astnode *n, void *arg, astnode **next)
{
    symtab_entry *e;
    astnode *id;
    astnode *stmts;
    /* The identifier which is being tested */
    id = astnode_get_child(n, 0);
    e = symtab_lookup(id->ident);
    if (e != NULL) {
        /* Symbol is defined. */
        /* Replace IFDEF node by the true-branch statement list */
        stmts = astnode_remove_children(astnode_get_child(n, 1));
        astnode_replace(n, stmts);
        *next = stmts;
    } else {
        /* Symbol is not defined. */
        /* Replace IFDEF node by the false-branch statement list (if any) */
        stmts = astnode_remove_children( astnode_get_child(n, 2));
        if (stmts != NULL) {
            astnode_replace(n, stmts);
            *next = stmts;
        } else {
            astnode_remove(n);
        }
    }
    /* Discard the original node */
    astnode_finalize(n);
    return 0;
}

/**
 * Process IFNDEF-node.
 * @param n Node of type IFNDEF_NODE
 * @param arg Not used
 */
static int process_ifndef(astnode *n, void *arg, astnode **next)
{
    symtab_entry *e;
    astnode *id;
    astnode *stmts;
    /* The identifier which is being tested */
    id = astnode_get_child(n, 0);
    e = symtab_lookup(id->ident);
    if (e == NULL) {
        /* Symbol is not defined. */
        /* Replace IFNDEF node by the true-branch statement list */
        stmts = astnode_remove_children(astnode_get_child(n, 1));
        astnode_replace(n, stmts);
        *next = stmts;
    } else {
        /* Symbol is defined. */
        /* Replace IFNDEF node by the false-branch statement list, if any */
        stmts = astnode_remove_children(astnode_get_child(n, 2));
        if (stmts != NULL) {
            astnode_replace(n, stmts);
            *next = stmts;
        } else {
            astnode_remove(n);
        }
    }
    /* Discard the original node */
    astnode_finalize(n);
    return 0;
}

/**
 * Process IF-node.
 * @param n Node of type IF_NODE
 * @param arg Not used
 */
static int process_if(astnode *n, void *arg, astnode **next)
{
    astnode *expr;
    astnode *stmts;
    astnode *c;
    int ret = 0;
    /* IF_NODE has a list of CASE, DEFAULT nodes as children */
    for (c = astnode_get_first_child(n); c != NULL; c = astnode_get_next_sibling(c) ) {
        if (astnode_is_type(c, CASE_NODE)) {
            /* The expression which is being tested */
            expr = astnode_get_child(c, 0);
            /* Try to reduce expression to literal */
            expr = reduce_expression(expr);
            /* Resulting expression must be an integer literal,
            since this is static evaluation.
            In other words, it can't contain label references.
            */
            if (astnode_is_type(expr, INTEGER_NODE)) {
                /* Non-zero is true, zero is false */
                if (expr->integer) {
                    /* Replace IF node by the true-branch statement list */
                    stmts = astnode_remove_children( astnode_get_child(c, 1) );
                    astnode_replace(n, stmts);
                    astnode_finalize(n);
                    *next = stmts;
                    return ret;
                }
            } else {
                /* Error, expression is not constant */
                err(expr->loc, "conditional expression does not evaluate to literal");
            }
        } else {  /* DEFAULT_NODE */
            /* Replace IF node by the false-branch statement list */
            stmts = astnode_remove_children(c);
            astnode_replace(n, stmts);
            astnode_finalize(n);
            *next = stmts;
            return ret;
        }
    }
    /* No match, remove IF node from AST */
    astnode_remove(n);
    astnode_finalize(n);
    return ret;
}

/**
 * Process dataseg-node.
 * @param n Node of type DATASEG_NODE
 * @param arg Not used
 */
static int process_dataseg(astnode *n, void *arg, astnode **next)
{
    modifiers = n->modifiers;
    in_dataseg = 1; /* true */
    return 0;
}

/**
 * Process codeseg-node.
 * @param n Node of type CODESEG_NODE
 * @param arg Not used
 */
static int process_codeseg(astnode *n, void *arg, astnode **next)
{
    modifiers = 0;
    in_dataseg = 0; /* false */
    return 0;
}

/**
 * Process org-node.
 * @param n Node of type ORG_NODE
 * @param arg Not used
 */
static int process_org(astnode *n, void *arg, astnode **next)
{
    if (!xasm_args.pure_binary) {
        err(n->loc, "org directive can only be used when output format is pure 6502 binary");
    } else {
        astnode *addr = astnode_get_child(n, 0);
        addr = reduce_expression_complete(addr);
        if (astnode_is_type(addr, INTEGER_NODE)) {
            /* Range check */
            if ((addr->integer < 0) || (addr->integer >= 0x10000)) {
                err(n->loc, "org address out of 64K range");
            }
        } else {
            err(n->loc, "org address does not evaluate to literal");
            /* Remove from AST */
            astnode_remove(n);
            astnode_finalize(n);
        }
    }
    return 0;
}

/**
 * Process REPT node.
 * @param n Node of type REPT_NODE
 * @param arg Not used
 */
static int process_rept(astnode *n, void *arg, astnode **next)
{
    astnode *count;
    astnode *stmts;
    astnode *list;
    /* The repeat count */
    count = astnode_get_child(n, 0);
    /* Try to reduce count expression to literal */
    count = reduce_expression_complete(count);
    /* Resulting expression must be an integer literal,
    since this is static evaluation.
    */
    if (astnode_is_type(count, INTEGER_NODE)) {
        if (count->integer < 0) {
            warn(n->loc, "REPT ignored; negative repeat count (%d)", count->integer);
            /* Remove from AST */
            astnode_remove(n);
            astnode_finalize(n);
        } else if (count->integer > 0) {
            /* Expand body <count> times */
            list = astnode_clone(astnode_get_child(n, 1), n->loc);
            stmts = astnode_remove_children(list);
            astnode_finalize(list);
            while (--count->integer > 0) {
                list = astnode_clone(astnode_get_child(n, 1), n->loc);
                astnode_add_sibling(stmts, astnode_remove_children(list) );
                astnode_finalize(list);
            }
            astnode_replace(n, stmts);
            astnode_finalize(n);
            *next = stmts;
        } else {
            /* count == 0, remove from AST */
            astnode_remove(n);
            astnode_finalize(n);
        }
    } else {
        err(n->loc, "repeat count does not evaluate to literal");
        /* Remove from AST */
        astnode_remove(n);
        astnode_finalize(n);
    }
    return 0;
}

/**
 * Process WHILE node.
 * @param n Node of type WHILE_NODE
 * @param arg Not used
 */
static int process_while(astnode *n, void *arg, astnode **next)
{
    astnode *expr;
    astnode *stmts;
    astnode *list;
    /* The boolean expression */
    expr = astnode_get_child(n, 0);
    /* Try to reduce expression to literal */
    expr = reduce_expression(astnode_clone(expr, expr->loc));
    /* Resulting expression must be an integer literal,
    since this is static evaluation.
    */
    if (astnode_is_type(expr, INTEGER_NODE)) {
        /* Expand body if the expression is true */
        if (expr->integer) {
            list = astnode_clone(astnode_get_child(n, 1), n->loc);
            stmts = astnode_remove_children(list);
            astnode_finalize(list);
            astnode_replace(n, stmts);
            astnode_add_sibling(stmts, n);  /* Clever huh? */
            *next = stmts;
        } else {
            /* Remove WHILE node from AST */
            astnode_remove(n);
            astnode_finalize(n);
        }
    } else {
        err(n->loc, "while expression does not evaluate to literal");
        /* Remove WHILE node from AST */
        astnode_remove(n);
        astnode_finalize(n);
    }
    astnode_finalize(expr);
    return 0;
}

/*---------------------------------------------------------------------------*/

/**
 * Enters a macro into the symbol table.
 * @param n Must be a node of type MACRO_DECL_NODE
 * @param arg Not used
 */
static int enter_macro(astnode *n, void *arg, astnode **next)
{
    astnode *id = astnode_get_child(n, 0);  /* Child 0 is macro identifier */
    assert(astnode_get_type(id) == IDENTIFIER_NODE);
    if (symtab_enter(id->ident, MACRO_SYMBOL, n, 0) == NULL) {
        /* ### This could be allowed, you know... */
        err(n->loc, "duplicate symbol `%s'", id->ident);
    }
    /* Remove from AST */
    astnode_remove(n);
    // ### n is not finalized???
    return 0;
}

/**
 * Enters a label into the symbol table.
 * @param n Must be a node of type LABEL_NODE
 */
static int enter_label(astnode *n, void *arg, astnode **next)
{
    symtab_entry *e;
    astnode *addr;
    /* Make sure it's unique first */
    if (symtab_lookup(n->ident)) {
        err(n->loc, "duplicate symbol `%s'", n->ident);
        /* Remove from AST */
        astnode_remove(n);
        astnode_finalize(n);
    } else {
        /* Enter it! */
        e = symtab_enter(n->ident, LABEL_SYMBOL, n, (in_dataseg ? DATA_FLAG : 0) | modifiers );
        /* Check if hardcoded address */
        addr = reduce_expression_complete(RHS(n));
        if (astnode_is_type(addr, INTEGER_NODE)) {
            /* Store it */
            e->address = addr->integer;
            e->flags |= ADDR_FLAG;
        } else if (!astnode_is_type(addr, CURRENT_PC_NODE)) {
            err(n->loc, "label address does not evaluate to literal");
        }
        /* Increase namespace counter */
        label_count++;
    }
    /* */
    return 0;
}

/**
 * Enters a variable declaration in symbol table.
 * @param n Must be a node of type VAR_DECL_NODE
 */
static int enter_var(astnode *n, void *arg, astnode **next)
{
    astnode *id = LHS(n);   /* Variable identifier */
    assert(astnode_get_type(id) == IDENTIFIER_NODE);
    /* Make sure it's unique first */
    if (symtab_lookup(id->ident)) {
        err(n->loc, "duplicate symbol `%s'", id->ident);
        /* Remove from AST */
        astnode_remove(n);
        astnode_finalize(n);
        return 0;
    } else {
        /* Validate modifiers */
        if ((n->modifiers & ZEROPAGE_FLAG) && !in_dataseg) {
            warn(n->loc, "zeropage modifier has no effect in code segment");
            n->modifiers &= ~ZEROPAGE_FLAG;
        }
        /* Enter it! */
        symtab_enter(id->ident, VAR_SYMBOL, astnode_clone(RHS(n), n->loc), (in_dataseg ? DATA_FLAG : 0) | n->modifiers | modifiers);
        /* */
        return 1;
    }
}

/**
 * Enters a procedure declaration in symbol table.
 * @param n Must be a node of type PROC_NODE
 */
static int enter_proc(astnode *n, void *arg, astnode **next)
{
    astnode *id;
    if (in_dataseg) {
        err(n->loc, "procedures not allowed in data segment");
        /* Remove from AST */
        astnode_remove(n);
        astnode_finalize(n);
        return 0;
    }
    id = LHS(n);    /* Procedure identifier */
    assert(astnode_get_type(id) == IDENTIFIER_NODE);
    /* Make sure it's unique first */
    if (symtab_lookup(id->ident)) {
        err(n->loc, "duplicate symbol `%s'", id->ident);
        /* Remove from AST */
        astnode_remove(n);
        astnode_finalize(n);
        return 0;
    } else {
        /* Enter it! RHS(n) is the list of procedure statements */
        symtab_enter(id->ident, PROC_SYMBOL, RHS(n), (in_dataseg ? DATA_FLAG : 0) );
        /* Increase global namespace counter */
        label_count++;
        /* */
        return 1;
    }
}

/**
 * Enters a simple <identifier> <storage> structure member.
 * @param c Node of type VAR_DECL_NODE
 * @param offset Offset of this field
 * @param plist List of symbol table's entries
 * @param struc_id Structure identifier (for error messages)
 * @return New offset (old offset + size of this field)
 */
static astnode *enter_struc_atomic_field(astnode *c, astnode *offset, ordered_field_list ***plist, astnode *struc_id)
{
    astnode *field_id;
    astnode *field_data;
    astnode *field_size;
    symtab_entry *fe;
    /* c has two children: id and STORAGE_NODE */
    field_id = LHS(c);
    assert(astnode_get_type(field_id) == IDENTIFIER_NODE);
    field_data = RHS(c);
    reduce_expression(RHS(field_data));
    /* Validate the declaration -- no data initialized */
    if (astnode_is_type(field_data, DATA_NODE)) {
        err(c->loc, "data initialization not allowed here");
        return(offset);
    }
    /* Try to enter field in structure's symbol table */
    fe = symtab_enter(
        field_id->ident,
        VAR_SYMBOL,
        astnode_clone(field_data, field_data->loc),
        0
    );
    if (fe == NULL) {
        err(c->loc, "duplicate symbol `%s' in structure `%s'", field_id->ident, struc_id->ident);
        return(offset);
    }
    /* Add to ordered list of fields */
    (**plist) = malloc(sizeof(ordered_field_list));
    (**plist)->entry = fe;
    (**plist)->next = NULL;
    *plist = &((**plist)->next);
    /* Set field offset */
    fe->field.offset = astnode_clone(offset, offset->loc);
    /* Calculate field size in bytes: sizeof(datatype) * count */
    field_size = astnode_create_arithmetic(
        MUL_OPERATOR,
        astnode_create_sizeof(astnode_clone(LHS(field_data), field_data->loc), field_data->loc),
        astnode_clone(RHS(field_data), field_data->loc),
        field_data->loc
    );
    field_size = reduce_expression(field_size);
    /* Set field size */
    fe->field.size = astnode_clone(field_size, field_size->loc);
    /* Add field size to total offset */
    offset = astnode_create_arithmetic(
        PLUS_OPERATOR,
        offset,
        field_size,
        offset->loc
    );
    offset = reduce_expression(offset);
    return(offset);
}

static void enter_union_fields(symtab_entry *, astnode *);

/**
 * Attempts to enter an (anonymous) union's members into structure's symbol table.
 * @param n Node of type UNION_DECL_NODE
 * @param offset Current parent structure offset
 * @param plist Ordered list of parent structure's fields
 */
astnode *enter_struc_union_field(astnode *n, astnode *offset, ordered_field_list ***plist, astnode *struc_id)
{
    ordered_field_list *ls;
    symtab_entry *se;
    symtab_entry *fe;
    static int id = 0;
    char id_str[16];
    astnode *union_id;
    union_id = LHS(n);
    if (astnode_is_type(union_id, IDENTIFIER_NODE)) {
        err(n->loc, "anonymous union expected");
        return(offset);
    }
    /* Put UNION in symbol table */
    sprintf(id_str, "%d", id++);
    se = symtab_enter(id_str, UNION_SYMBOL, n, 0);
    enter_union_fields(se, n);
    /* Add to ordered list of fields */
    (**plist) = malloc(sizeof(ordered_field_list));
    (**plist)->entry = se;
    (**plist)->next = NULL;
    *plist = &((**plist)->next);
    /* Add to parent structure as well, with same offsets */
    for (ls = se->struc.fields; ls != NULL; ls = ls->next) {
        /* Try to enter field in structure's symbol table */
        fe = symtab_enter(
            ls->entry->id,
            VAR_SYMBOL,
            astnode_clone(ls->entry->def, ls->entry->def->loc),
            0
        );
        if (fe == NULL) {
            err(ls->entry->def->loc, "duplicate symbol `%s' in structure `%s'", ls->entry->id, struc_id->ident);
            continue;
        }
        /* Set field offset */
        fe->field.offset = astnode_clone(offset, offset->loc);
        /* Set field size */
        fe->field.size = astnode_clone(se->struc.size, offset->loc);
    }
    /* Advance offset by size of union */
    offset = astnode_create_arithmetic(
        PLUS_OPERATOR,
        offset,
        astnode_clone(se->struc.size, offset->loc),
        offset->loc
    );
    offset = reduce_expression(offset);
    return(offset);
}

/**
 * Enters struc type into symbol table based on AST node.
 * - Creates a symbol table for the structure
 * - Validates and enters all its fields
 * - Calculates offset of each field in the structure, and total size
 * @param n Node of type STRUC_DECL_NODE
 */
static int enter_struc(astnode *n, void *arg, astnode **next)
{
    ordered_field_list **plist;
    symtab_entry *se;
    astnode *c;
    astnode *offset;
    astnode *struc_id = LHS(n); /* Child 0 is struc identifier */
    /* Put STRUC in symbol table */
    se = symtab_enter(struc_id->ident, STRUC_SYMBOL, n, 0);
    if (se == NULL) {
        err(n->loc, "duplicate symbol `%s'", struc_id->ident);
    } else {
        /* Put the fields of the structure in local symbol table */
        se->symtab = symtab_create();
        offset = astnode_create_integer(0, n->loc); /* offset = 0 */
        plist = &se->struc.fields;
        for (c = struc_id->next_sibling; c != NULL; c = c->next_sibling) {
            /* Check if it's a field declaration */
            if (astnode_is_type(c, VAR_DECL_NODE)) {
                offset = enter_struc_atomic_field(c, offset, &plist, struc_id);
            }
            /* Check if (anonymous) union */
            else if (astnode_is_type(c, UNION_DECL_NODE)) {
                offset = enter_struc_union_field(c, offset, &plist, struc_id);
            } else {
                err(c->loc, "field declaration expected");
                continue;
            }
        }
        /* Store total size of structure */
        se->struc.size = offset;
        /* Restore previous symbol table */
        symtab_pop();
    }
    /* ### Remove STRUC node from AST */
//    astnode_remove(n);
//    astnode_finalize(n);
    return 0;
}

/**
 * Enters fields of union into its symbol table.
 */
static void enter_union_fields(symtab_entry *se, astnode *n)
{
    ordered_field_list **plist;
    astnode *c;
    astnode *field_id;
    astnode *field_data;
    astnode *field_size;
    symtab_entry *fe;

    se->symtab = symtab_create();
    se->struc.size = astnode_create_integer(0, n->loc);
    plist = &se->struc.fields;
    /* Process field declarations */
    for (c = RHS(n); c != NULL; c = c->next_sibling) {
        /* Make sure it's a field declaration */
        if (!astnode_is_type(c, VAR_DECL_NODE)) {
            err(c->loc, "field declaration expected");
            continue;
        }
        /* c has two children: id and STORAGE_NODE */
        field_id = LHS(c);
        assert(astnode_get_type(field_id) == IDENTIFIER_NODE);
        field_data = RHS(c);
        reduce_expression(RHS(field_data));
        /* Validate the declaration -- no data initialized */
        if (astnode_is_type(field_data, DATA_NODE)) {
            err(c->loc, "data initialization not allowed here");
            continue;
        }
        /* Calculate field size in bytes: sizeof(datatype) * count */
        field_size = astnode_create_arithmetic(
            MUL_OPERATOR,
            astnode_create_sizeof(astnode_clone(LHS(field_data), field_data->loc), field_data->loc),
            astnode_clone(RHS(field_data), field_data->loc),
            field_data->loc
        );
        field_size = reduce_expression(field_size);
        /* Make sure field size is a constant */
        if (!astnode_is_type(field_size, INTEGER_NODE)) {
            err(c->loc, "union member must be of constant size");
            astnode_finalize(field_size);
            /* Use default size: 1 byte */
            field_size = astnode_create_integer(1, field_data->loc);
        }
        /* Try to enter field in structure's symbol table */
        fe = symtab_enter(
            field_id->ident,
            VAR_SYMBOL,
            astnode_clone(field_data, field_data->loc),
            0
        );
        if (fe == NULL) {
            err(c->loc, "duplicate symbol `%s' in union `%s'", field_id->ident, se->id);
            astnode_finalize(field_size);
            continue;
        }
        /* Add to ordered list of fields */
        (*plist) = malloc(sizeof(ordered_field_list));
        (*plist)->entry = fe;
        (*plist)->next = NULL;
        plist = &((*plist)->next);
        /* Set field offset (0 for all) and size */
        fe->field.offset = astnode_create_integer(0, n->loc);
        fe->field.size = astnode_clone(field_size, field_size->loc);
        /* See if field size of this member is largest so far */
        if (se->struc.size->integer < field_size->integer) {
            astnode_finalize(se->struc.size);
            se->struc.size = field_size;
        } else {
            astnode_finalize(field_size);
        }
    }
    symtab_pop();
}

/**
 * Enters union type into symbol table based on AST node.
 * @param n Node of type UNION_DECL_NODE
 */
static int enter_union(astnode *n, void *arg, astnode **next)
{
    symtab_entry *se;
    astnode *union_id = astnode_get_child(n, 0);    /* Child 0 is union identifier */
    /* Check for anonymous union */
    if (astnode_is_type(union_id, NULL_NODE)) {
        err(n->loc, "anonymous union not allowed in global scope");
    } else {
        /* Put UNION in symbol table */
        assert(astnode_get_type(union_id) == IDENTIFIER_NODE);
        se = symtab_enter(union_id->ident, UNION_SYMBOL, n, 0);
        if (se == NULL) {
            err(n->loc, "duplicate symbol `%s'", union_id->ident);
        } else {
            /* Put the fields of the union in local symbol table */
            enter_union_fields(se, n);
        }
    }
    /* ### Remove UNION node from AST */
//    astnode_remove(n);
//    astnode_finalize(n);
    return 0;
}

/**
 * Enters enumerated type into symbol table based on AST node.
 * @param n Node of type ENUM_DECL_NODE
 */
static int enter_enum(astnode *n, void *arg, astnode **next)
{
    astnode *c;
    astnode *id;
    astnode *val;
    symtab_entry *se;
    astnode *enum_id = astnode_get_child(n, 0); /* Child 0 is enum identifier */
    /* Enter in global symbol table */
    assert(astnode_get_type(enum_id) == IDENTIFIER_NODE);
    se = symtab_enter(enum_id->ident, ENUM_SYMBOL, n, 0);
    if (se == NULL) {
        err(n->loc, "duplicate symbol `%s'", enum_id->ident);
    } else {
        /* Add all the enum symbols to its own symbol table */
        se->symtab = symtab_create();
        val = NULL;
        for (c = enum_id->next_sibling; c != NULL; c = c->next_sibling) {
            if (astnode_is_type(c, IDENTIFIER_NODE)) {
                id = c;
                if (val == NULL) {
                    val = astnode_create_integer(0, c->loc);
                } else {
                    val = astnode_create_integer(val->integer+1, c->loc);
                }
            } else {
                id = LHS(c);
                val = reduce_expression_complete(astnode_clone(RHS(c), RHS(c)->loc));
                if (!astnode_is_type(val, INTEGER_NODE)) {
                    err(c->loc, "initializer does not evaluate to integer literal");
                    astnode_finalize(val);
                    /* Use default value */
                    val = astnode_create_integer(0, c->loc);
                }
            }
            if (symtab_enter(id->ident, CONSTANT_SYMBOL, val, 0) == NULL) {
                err(c->loc, "duplicate symbol `%s' in enumeration `%s'", id->ident, enum_id->ident);
                continue;
            }
        }
        symtab_pop();
    }
    /* ### Remove ENUM node from AST */
//    astnode_remove(n);
//    astnode_finalize(n);
    return 0;
}

/**
 * Enters record type into symbol table based on AST node.
 * @param n Node of type RECORD_DECL_NODE
 */
static int enter_record(astnode *n, void *arg, astnode **next)
{
    ordered_field_list **plist;
    astnode *c;
    astnode *field_id;
    astnode *field_width;
    int size;
    int offset;
    symtab_entry *se;
    symtab_entry *fe;
    astnode *record_id = astnode_get_child(n, 0);   /* Child 0 is record identifier */
    assert(astnode_get_type(record_id) == IDENTIFIER_NODE);
    /* Enter in global symbol table */
    se = symtab_enter(record_id->ident, RECORD_SYMBOL, n, 0);
    if (se == NULL) {
        err(n->loc, "duplicate symbol `%s'", record_id->ident);
    }
    else {
        /* Add all the record fields to record's own symbol table */
        se->symtab = symtab_create();
        offset = 8;
        plist = &se->struc.fields;
        for (c = record_id->next_sibling; c != NULL; c = c->next_sibling) {
            /* c has two children: field identifier and its width */
            field_id = LHS(c);
            field_width = astnode_clone(reduce_expression(RHS(c)), RHS(c)->loc);
            /* Validate the width -- must be positive integer literal */
            if (!astnode_is_type(field_width, INTEGER_NODE)) {
                err(c->loc, "record member `%s' is not of constant size", field_id->ident);
                continue;
            }
            if ((field_width->integer <= 0) || (field_width->integer >= 8)) {
                err(c->loc, "width of record member `%s' is out of range (%d)", field_id->ident, field_width->integer);
                continue;
            }
            /* Attempt to enter field in record's symbol table */
            fe = symtab_enter(field_id->ident, VAR_SYMBOL, c, 0);
            if (fe == NULL) {
                err(c->loc, "duplicate symbol `%s' in record `%s'", field_id->ident, record_id->ident);
                continue;
            }
            /* Add to ordered list of fields */
            (*plist) = malloc(sizeof(ordered_field_list));
            (*plist)->entry = fe;
            (*plist)->next = NULL;
            plist = &((*plist)->next);
            /* Set field offset */
            offset = offset - field_width->integer;
            fe->field.offset = astnode_create_integer(offset, c->loc);
            /* Set field size (width) */
            fe->field.size = field_width;
        }
        size = 8 - offset;
        if (size > 8) {
            err(n->loc, "size of record `%s' (%d) exceeds 8 bits", record_id->ident, size);
        } else {
            /* Set size of record (in bits) */
            se->struc.size = astnode_create_integer(size, n->loc);
        }
        symtab_pop();
    }
    /* ### Remove RECORD node from AST */
//    astnode_remove(n);
//    astnode_finalize(n);
    return 0;
}

/**
 * Globalizes a local.
 * The node is morphed into its global equivalent (LABEL_NODE or IDENTIFIER_NODE).
 * @param n A node of type LOCAL_LABEL_NODE or LOCAL_ID_NODE
 * @param arg Pointer to namespace counter
 */
static int globalize_local(astnode *n, void *arg, astnode **next)
{
    char str[32];
    /* Make it global by appending namespace counter to the id */
    sprintf(str, "#%d", label_count);
    if (astnode_is_type(n, LOCAL_LABEL_NODE)) {
        /* Local label definition, use label field */
        n->label = realloc(n->label, strlen(n->label)+strlen(str)+1);
        strcat(n->label, str);
        /* This node is now a unique, global label */
        n->type = LABEL_NODE;
        /* Make sure it's unique */
        if (symtab_lookup(n->label)) {
            err(n->loc, "duplicate symbol `%s'", n->label);
            /* Remove from AST */
            astnode_remove(n);
            astnode_finalize(n);
            return 0;
        } else {
            /* Enter it in symbol table */
            symtab_enter(n->label, LABEL_SYMBOL, n, (in_dataseg ? DATA_FLAG : 0) );
        }
    } else {
        /* Local label reference, use ident field */
        n->ident = realloc(n->ident, strlen(n->ident)+strlen(str)+1);
        strcat(n->ident, str);
        /* This node is now a unique, global identifier */
        n->type = IDENTIFIER_NODE;
    }
    return 1;
}

/**
 * Tags symbols as extrn.
 * @param n A node of type EXTRN_NODE
 */
static int tag_extrn_symbols(astnode *n, void *arg, astnode **next)
{
    astnode *id;
    astnode *type;
    astnode *list;
    symtab_entry *e;
    /* Get symbol type specifier */
    type = astnode_get_child(n, 0);
    /* Go through the list of identifiers */
    list = astnode_get_child(n, 1);
    for (id=astnode_get_first_child(list); id != NULL; id=astnode_get_next_sibling(id) ) {
        /* Look up identifier in symbol table */
        e = symtab_lookup(id->ident);
        if (e != NULL) {
            if (!(e->flags & EXTRN_FLAG)) {
                /* Error, can't import a symbol that's defined locally! */
                // TODO: this is okay?
                err(n->loc, "`%s' declared as extrn but is defined locally", id->ident);
            }
        }
        else {
            // TODO: store external unit name
            switch (astnode_get_type(type)) {
                case DATATYPE_NODE:
                /* Put it in symbol table */
                symtab_enter(id->ident, VAR_SYMBOL, astnode_create_data(astnode_clone(type, n->loc), NULL, n->loc), EXTRN_FLAG);
                break;

                case INTEGER_NODE:
                /* type->integer is (LABEL|PROC)_SYMBOL */
                symtab_enter(id->ident, type->integer, NULL, EXTRN_FLAG);
                break;

                default:
                break;
            }
        }
    }
    /* Remove extrn node from AST */
    astnode_remove(n);
    astnode_finalize(n);
    //
    return 0;
}

/**
 *
 */
static int process_message(astnode *n, void *arg, astnode **next)
{
    astnode *mesg = reduce_expression_complete(LHS(n));
    if (astnode_is_type(mesg, STRING_NODE)) {
        printf("%s\n", mesg->string);
    }
    else if (astnode_is_type(mesg, INTEGER_NODE)) {
        printf("%d\n", mesg->integer);
    }
    else {
        err(mesg->loc, "string or integer argument expected");
    }
    astnode_remove(n);
    astnode_finalize(n);
    return 0;
}

/**
 *
 */
static int process_warning(astnode *n, void *arg, astnode **next)
{
    astnode *mesg = reduce_expression_complete(LHS(n));
    if (astnode_is_type(mesg, STRING_NODE)) {
        warn(mesg->loc, mesg->string);
    }
    else {
        err(mesg->loc, "string argument expected");
    }
    astnode_remove(n);
    astnode_finalize(n);
    return 0;
}

/**
 *
 */
static int process_error(astnode *n, void *arg, astnode **next)
{
    astnode *mesg = reduce_expression_complete(LHS(n));
    if (astnode_is_type(mesg, STRING_NODE)) {
        err(mesg->loc, mesg->string);
    }
    else {
        err(mesg->loc, "string argument expected");
    }
    astnode_remove(n);
    astnode_finalize(n);
    return 0;
}

/**
 * Processes a forward branch declaration.
 * @param n Node of type FORWARD_BRANCH_DECL_NODE
 * @param arg Not used
 */
static int process_forward_branch_decl(astnode *n, void *arg, astnode **next)
{
    astnode *l;
    int i;
    char str[32];
    assert(!strchr(n->ident, '#'));
    /* Get branch info structure for label (+, ++, ...) */
    forward_branch_info *fwd = &forward_branch[strlen(n->ident)-1];
    /* Morph n to globally unique label */
    sprintf(str, "#%d", fwd->counter);
    n->label = (char *)realloc(n->ident, strlen(n->ident)+strlen(str)+1);
    strcat(n->label, str);
    n->type = LABEL_NODE;
    symtab_enter(n->label, LABEL_SYMBOL, n, 0);
    /* Fix reference identifiers */
    for (i=0; i<fwd->index; i++) {
        l = fwd->refs[i];
        l->ident = (char *)realloc(l->ident, strlen(n->ident)+1);
        strcpy(l->ident, n->ident);
    }
    /* Prepare for next declaration */
    fwd->index = 0;
    fwd->counter++;
    return 0;
}

/**
 * Processes a backward branch declaration.
 * @param n Node of type BACKWARD_BRANCH_DECL_NODE
 * @param arg Not used
 */
static int process_backward_branch_decl(astnode *n, void *arg, astnode **next)
{
    char str[32];
    assert(!strchr(n->ident, '#'));
    /* Get branch info */
    backward_branch_info *bwd = &backward_branch[strlen(n->ident)-1];
    bwd->decl = n;
    /* Morph n to globally unique label */
    sprintf(str, "#%d", bwd->counter);
    n->label = (char *)realloc(n->ident, strlen(n->ident)+strlen(str)+1);
    strcat(n->label, str);
    n->type = LABEL_NODE;
    symtab_enter(n->label, LABEL_SYMBOL, n, 0);
    /* Prepare for next declaration */
    bwd->counter++;
    return 0;
}

/**
 * Processes a forward branch label reference.
 * @param n Node of type FORWARD_BRANCH_NODE
 * @param arg Not used
 */
static int process_forward_branch(astnode *n, void *arg, astnode **next)
{
    /* Add n to proper forward_branch array */
    forward_branch_info *fwd = &forward_branch[strlen(n->ident)-1];
    fwd->refs[fwd->index++] = n;
    /* Change to identifier node */
    n->type = IDENTIFIER_NODE;
    return 0;
}

/**
 * Processes a backward branch label reference.
 * @param n Node of type BACKWARD_BRANCH_NODE
 * @param arg Not used
 */
static int process_backward_branch(astnode *n, void *arg, astnode **next)
{
    /* Get branch info */
    backward_branch_info *bwd = &backward_branch[strlen(n->ident)-1];
    /* Make sure it's a valid reference */
    if (bwd->decl != NULL) {
        /* Fix n->ident */
        n->ident = (char *)realloc(n->ident, strlen(bwd->decl->ident)+1);
        strcpy(n->ident, bwd->decl->ident);
    }
    /* Change to identifier node */
    n->type = IDENTIFIER_NODE;
    return 0;
}

/*---------------------------------------------------------------------------*/

static int is_field_ref(astnode *n)
{
    astnode *p = astnode_get_parent(n);
    /* Case 1: id.id */
    if (astnode_is_type(p, DOT_NODE)) return 1;
    /* Case 2: id.id[expr] */
    if (astnode_is_type(p, INDEX_NODE) && (n == LHS(p)) && astnode_is_type(astnode_get_parent(p), DOT_NODE) ) return 1;
    return 0;
}

/**
 * Checks that the given identifier node is present in symbol table.
 * Issues error if it is not, and replaces with integer 0.
 * @param n A node of type IDENTIFIER_NODE
 */
static int validate_ref(astnode *n, void *arg, astnode **next)
{
    int i;
    symbol_ident_list list;
    symtab_entry *enum_def;
    int ret = 1;
    if (is_field_ref(n)) {
        return 1;   /* Validated by validate_dotref() */
    }
    /* Look it up in symbol table */
    symtab_entry * e = symtab_lookup(n->ident);
    if (e == NULL) {
        /* This identifier is unknown */
        /* Maybe it is part of an enumeration */
        symtab_list_type(ENUM_SYMBOL, &list);
        for (i=0; i<list.size; i++) {
            enum_def = symtab_lookup(list.idents[i]);
            symtab_push(enum_def->symtab);
            e = symtab_lookup(n->ident);
            symtab_pop();
            if (e != NULL) {
                /* Found it */
                /* Replace id by SCOPE_NODE */
                astnode *scope = astnode_create_scope(
                    astnode_create_identifier(enum_def->id, n->loc),
                    astnode_clone(n, n->loc), n->loc);
                astnode_replace(n, scope);
                astnode_finalize(n);
                *next = scope;
                ret = 0;
                break;
            }
        }
        symtab_list_finalize(&list);
        /* If still not found, error */
        if (e == NULL) {
            strtok(n->ident, "#");  /* Remove globalize junk */
//            err(n->loc, "unknown symbol `%s'", n->ident);
            /* ### Replace by integer 0 */
            //astnode_replace(n, astnode_create_integer(0, n->loc) );
            //astnode_finalize(n);
            warn(n->loc, "`%s' undeclared; assuming external label", n->ident);
            e = symtab_enter(n->ident, LABEL_SYMBOL, NULL, EXTRN_FLAG);
        }
    }
    assert(e);
    /* Increase reference count */
    e->ref_count++;
    return ret;
}

/**
 * Validates top-level (not part of structure) indexed identifier.
 * @param n Node of type INDEX_NODE
 * @param arg Not used
 */
static int validate_index(astnode *n, void *arg, astnode **next)
{
    symtab_entry *e;
    astnode *id;
    astnode *type;
    if (is_field_ref(LHS(n))) {
        return 1;   /* Validated by validate_dotref() */
    }
    id = LHS(n);
    if (!astnode_is_type(id, IDENTIFIER_NODE)) {
        err(n->loc, "identifier expected");
        astnode_replace(n, astnode_create_integer(0, n->loc) );
        astnode_finalize(n);
        return 0;
    }
    e = symtab_lookup(id->ident);
    if (e != NULL) {
        type = LHS(e->def);
        if (!astnode_is_type(type, DATATYPE_NODE)) {
            err(n->loc, "`%s' cannot be indexed", id->ident);
            astnode_replace(n, astnode_create_integer(0, n->loc) );
            astnode_finalize(n);
            return 0;
        } else {
            // TODO: bounds check
            reduce_index(n);
        }
    } else {
        err(n->loc, "unknown symbol `%s'", id->ident);
        astnode_replace(n, astnode_create_integer(0, n->loc) );
        astnode_finalize(n);
        return 0;
    }
    return 1;
}

/**
 * Checks that A::B is valid.
 * If it's not valid it is replaced by integer 0.
 * @param n Node of type SCOPE_NODE
 */
static int validate_scoperef(astnode *n, void *arg, astnode **next)
{
    astnode *symbol;
    astnode *namespace = LHS(n);
    /* Look up namespace in global symbol table */
    symtab_entry * e = symtab_lookup(namespace->ident);
    if (e == NULL) {
        /* Error, this identifier is unknown */
        err(n->loc, "unknown namespace `%s'", namespace->ident);
        /* Replace by integer 0 */
        astnode_replace(n, astnode_create_integer(0, n->loc) );
        astnode_finalize(n);
        return 0;
    } else {
        /* Get symbol on right of :: operator */
        symbol = RHS(n);
        /* Namespace was found, check its type */
        switch (e->type) {
            case STRUC_SYMBOL:
            case UNION_SYMBOL:
            case RECORD_SYMBOL:
            case ENUM_SYMBOL:
            /* OK, check the symbol */
            symtab_push(e->symtab);
            e = symtab_lookup(symbol->ident);
            if (e == NULL) {
                /* Error, symbol is not in namespace */
                err(n->loc, "unknown symbol `%s' in namespace `%s'", symbol->ident, namespace->ident);
                /* Replace by integer 0 */
                astnode_replace(n, astnode_create_integer(0, n->loc) );
                astnode_finalize(n);
            }
            symtab_pop();
            break;

            default:
            err(n->loc, "`%s' is not a namespace", namespace->ident);
            /* Replace by integer 0 */
            astnode_replace(n, astnode_create_integer(0, n->loc) );
            astnode_finalize(n);
            break;
        }
    }
    return 0;
}

/**
 * Validates right part of dotted reference recursively.
 * Assumes that left part's symbol table is on stack.
 * @param n Node of type DOT_NODE
 */
static void validate_dotref_recursive(astnode *n, astnode *top)
{
    astnode *left;
    astnode *right;
    astnode *type;
    symtab_entry *field;
    symtab_entry *def;
    left = LHS(n);
    if (astnode_is_type(left, INDEX_NODE)) {
        left = LHS(left);   /* Need identifier */
    }
    right = RHS(n);
    if (astnode_is_type(right, DOT_NODE)) {
        right = LHS(right); /* Need identifier */
    }
    if (astnode_is_type(right, INDEX_NODE)) {
        right = LHS(right); /* Need identifier */
    }
    /* Lookup 'right' in 'left's symbol table */
    assert(astnode_get_type(right) == IDENTIFIER_NODE);
    field = symtab_lookup(right->ident);
    if (field == NULL) {
        /* Error, this symbol is unknown */
        err(n->loc, "`%s' is not a member of `%s'", right->ident, left->ident);
        /* Replace by integer 0 */
        astnode_replace(top, astnode_create_integer(0, top->loc) );
        astnode_finalize(top);
    } else {
        /* See if more subfields to process */
        n = RHS(n);
        if (astnode_is_type(n, DOT_NODE)) {
            /* Verify the variable's type -- should be user-defined */
            type = LHS(field->def);
            if ((type == NULL) || (type->datatype != USER_DATATYPE)) {
                err(n->loc, "member `%s' of `%s' is not a structure", right->ident, left->ident);
                /* Replace by integer 0 */
                astnode_replace(top, astnode_create_integer(0, top->loc) );
                astnode_finalize(top);
            } else {
                /* Look up variable's type definition and verify it's a structure */
                def = symtab_global_lookup(LHS(type)->ident);
                if (def == NULL) {
                    err(n->loc, "member '%s' of '%s' is of unknown type (`%s')", right->ident, left->ident, LHS(type)->ident);
                    /* Replace by integer 0 */
                    astnode_replace(top, astnode_create_integer(0, top->loc) );
                    astnode_finalize(top);
                } else if ( !((def->type == STRUC_SYMBOL) || (def->type == UNION_SYMBOL)) ) {
                    err(n->loc, "member `%s' of `%s' is not a structure", right->ident, left->ident);
                    /* Replace by integer 0 */
                    astnode_replace(top, astnode_create_integer(0, top->loc) );
                    astnode_finalize(top);
                } else {
                    /* Next field */
                    symtab_push(def->symtab);
                    validate_dotref_recursive(n, top);
                    symtab_pop();
                }
            }
        }
    }
}

/**
 * Validates A.B.C.D. . ...
 * Replaces the whole thing with integer 0 if not.
 * @param n Node of type DOT_NODE
 */
static int validate_dotref(astnode *n, void *arg, astnode **next)
{
    symtab_entry *father;
    symtab_entry *def;
    astnode *type;
    astnode *left;
    if (astnode_has_ancestor_of_type(n, DOT_NODE)) {
        return 1;   /* Already validated, since this function is recursive */
    }
    /* Look up parent in global symbol table */
    left = LHS(n);  /* n := left . right */
    if (astnode_is_type(left, INDEX_NODE)) {
        left = LHS(left);   /* Need identifier */
    }
    father = symtab_lookup(left->ident);
    if (father == NULL) {
        /* Error, this symbol is unknown */
        err(n->loc, "unknown symbol `%s'", left->ident);
        /* Replace by integer 0 */
        astnode_replace(n, astnode_create_integer(0, n->loc) );
        astnode_finalize(n);
        return 0;
    } else {
        /* Increase reference count */
        father->ref_count++;
        /* Verify the variable's type -- should be user-defined */
        type = LHS(father->def);
        if ((type == NULL) || (type->datatype != USER_DATATYPE)) {
            err(n->loc, "`%s' is not a structure", left->ident);
            /* Replace by integer 0 */
            astnode_replace(n, astnode_create_integer(0, n->loc) );
            astnode_finalize(n);
            return 0;
        } else {
            /* Look up variable's type definition and verify it's a structure */
            def = symtab_lookup(LHS(type)->ident);
            if (def == NULL) {
                err(n->loc, "'%s' is of unknown type (`%s')", left->ident, LHS(type)->ident);
                /* Replace by integer 0 */
                astnode_replace(n, astnode_create_integer(0, n->loc) );
                astnode_finalize(n);
                return 0;
            } else if ( !((def->type == STRUC_SYMBOL) || (def->type == UNION_SYMBOL)) ) {
                err(n->loc, "`%s' is not a structure", left->ident);
                /* Replace by integer 0 */
                astnode_replace(n, astnode_create_integer(0, n->loc) );
                astnode_finalize(n);
                return 0;
            } else {
                /* Verify fields recursively */
                symtab_push(def->symtab);
                validate_dotref_recursive(n, n);
                symtab_pop();
            }
        }
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

/**
 * Evaluates expressions involved in conditional assembly, and removes the
 * appropriate branches from the AST.
 * Does some other stuff too, such as substitute equates and fold constants.
 */
void astproc_first_pass(astnode *root)
{
    /* Table of callback functions for our purpose. */
    static astnodeprocmap map[] = {
        { LABEL_NODE, enter_label },
        { VAR_DECL_NODE, enter_var },
        { PROC_NODE, enter_proc },
        { STRUC_DECL_NODE, enter_struc },
        { UNION_DECL_NODE, enter_union },
        { ENUM_DECL_NODE, enter_enum },
        { RECORD_DECL_NODE, enter_record },
        { LOCAL_LABEL_NODE, globalize_local },
        { LOCAL_ID_NODE, globalize_local },
        { MACRO_DECL_NODE, enter_macro },
        { MACRO_NODE, expand_macro },
        { REPT_NODE, process_rept },
        { WHILE_NODE, process_while },
        { DATASEG_NODE, process_dataseg },
        { CODESEG_NODE, process_codeseg },
        { ORG_NODE, process_org },
        { CHARMAP_NODE, load_charmap },
        { INSTRUCTION_NODE, process_instruction },
        { DATA_NODE, process_data },
        { STORAGE_NODE, process_storage },
        { EQU_NODE, process_equ },
        { ASSIGN_NODE, process_assign },
        { IFDEF_NODE, process_ifdef },
        { IFNDEF_NODE, process_ifndef },
        { IF_NODE, process_if },
        { EXTRN_NODE, tag_extrn_symbols },
        { MESSAGE_NODE, process_message },
        { WARNING_NODE, process_warning },
        { ERROR_NODE, process_error },
        { FORWARD_BRANCH_DECL_NODE, process_forward_branch_decl },
        { BACKWARD_BRANCH_DECL_NODE, process_backward_branch_decl },
        { FORWARD_BRANCH_NODE, process_forward_branch },
        { BACKWARD_BRANCH_NODE, process_backward_branch },
        { 0, NULL }
    };
    reset_charmap();
    branch_init();
    in_dataseg = 0; /* codeseg is default */
    /* Do the walk. */
    astproc_walk(root, NULL, map);
    /* Remove all the volatile constants from the symbol table */
    /* These are the ones defined with the '=' operator, whose identifiers should
    all have been replaced by their value in the syntax tree now. Since
    they're not referenced anywhere we can safely dispose of them.
    The EQUates on the other hand should be kept, since they will
    possibly be exported. */
#ifdef ENABLE_BUGGY_THING // ### FIXME
    {
        int i;
        symbol_ident_list list;
        symtab_entry *e;
        symtab_list_type(CONSTANT_SYMBOL, &list);
        for (i = 0; i < list.size; ++i) {
            e = symtab_lookup(list.idents[i]);
            if (e->flags & VOLATILE_FLAG) {
                symtab_remove(list.idents[i]);
            }
        }
        symtab_list_finalize(&list);
    }
#endif
}

/*---------------------------------------------------------------------------*/

/**
 * Tags labels as public.
 * @param n A node of type PUBLIC_NODE
 */
static int tag_public_symbols(astnode *n, void *arg, astnode **next)
{
    astnode *id;
    symtab_entry *e;
    /* Go through the list of identifiers */
    for (id=astnode_get_first_child(n); id != NULL; id = astnode_get_next_sibling(id) ) {
        /* Look up identifier in symbol table */
        e = symtab_lookup(id->ident);
        if (e != NULL) {
            /* Symbol exists. Set the proper flag unless ambiguous. */
            if (e->flags & EXTRN_FLAG) {
                err(n->loc, "`%s' already declared extrn", id->ident);
            } else {
                switch (e->type) {
                    case LABEL_SYMBOL:
                    case CONSTANT_SYMBOL:
                    case VAR_SYMBOL:
                    case PROC_SYMBOL:
                    /* GO! */
                    e->flags |= PUBLIC_FLAG;
                    break;

                    default:
                    err(n->loc, "`%s' is of non-exportable type", id->ident);
                    break;
                }
            }
        } else {
            /* Warning, can't export a symbol that's not defined. */
            warn(n->loc, "`%s' declared as public but is not defined", id->ident);
        }
    }
    /* Remove PUBLIC_NODE from AST */
    astnode_remove(n);
    astnode_finalize(n);
    //
    return 0;
}

/**
 * Sets alignment for a set of (data) labels.
 * @param n A node of type ALIGN_NODE
 */
static int tag_align_symbols(astnode *n, void *arg, astnode **next)
{
    int pow;
    astnode *id;
    astnode *idents;
    astnode *expr;
    symtab_entry *e;
    /* Go through the list of identifiers */
    idents = LHS(n);
    for (id=astnode_get_first_child(idents); id != NULL; id = astnode_get_next_sibling(id) ) {
        /* Look up identifier in symbol table */
        e = symtab_lookup(id->ident);
        if (e != NULL) {
            /* Symbol exists. Set the proper flag unless ambiguous. */
            if (!(e->flags & DATA_FLAG)) {
                err(n->loc, "cannot align a code symbol (`%s')", id->ident);
            } else {
                switch (e->type) {
                    case LABEL_SYMBOL:
                    case VAR_SYMBOL:
                    expr = reduce_expression(RHS(n));
                    if (!astnode_is_type(expr, INTEGER_NODE)) {
                        err(n->loc, "alignment expression must be an integer literal");
                    } else if ((expr->integer < 0) || (expr->integer >= 0x10000)) {
                        err(n->loc, "alignment expression out of range");
                    } else if (expr->integer > 1) {
                        pow = 0;
                        switch (expr->integer) {
                            case 32768: pow++;
                            case 16384: pow++;
                            case 8192:  pow++;
                            case 4096:  pow++;
                            case 2048:  pow++;
                            case 1024:  pow++;
                            case 512:   pow++;
                            case 256:   pow++;
                            case 128:   pow++;
                            case 64:    pow++;
                            case 32:    pow++;
                            case 16:    pow++;
                            case 8:     pow++;
                            case 4:     pow++;
                            case 2:     pow++;
                            /* GO! */
                            e->flags |= ALIGN_FLAG;
                            e->align = pow;
                            break;

                            default:
                            err(n->loc, "alignment expression must be a power of 2");
                            break;
                        }
                    }
                    break;

                    default:
                    err(n->loc, "`%s' cannot be aligned", id->ident);
                    break;
                }
            }
        }
        else {
            /* Warning, can't align a symbol that's not defined. */
            warn(n->loc, "alignment ignored for undefined symbol `%s'", id->ident);
        }
    }
    /* Remove ALIGN_NODE from AST */
    astnode_remove(n);
    astnode_finalize(n);
    //
    return 0;
}

/*---------------------------------------------------------------------------*/

/**
 * Removes unused labels from a syntax tree (and symbol table).
 * Unused labels are labels that are defined but not referenced anywhere.
 * This function assumes that the reference counts have already been calculated.
 */
void remove_unused_labels()
{
    int i;
    char *id;
    astnode *n;
    symbol_ident_list list;
    symtab_list_type(LABEL_SYMBOL, &list);
    for (i=0; i<list.size; i++) {
        /* Look up label in symbol table */
        id = list.idents[i];
        symtab_entry * e = symtab_lookup(id);
        /* If reference count is zero, AND label isn't declared public, remove it. */
        if ((e->ref_count == 0) && ((e->flags & PUBLIC_FLAG) == 0)) {
            n = e->def;
            strtok(n->label, "#");  /* Remove globalize junk */
            warn(n->loc, "`%s' defined but not used", n->label);
            /* Remove label from AST */
            astnode_remove(n);
            astnode_finalize(n);
            //symtab_remove(n->label); ### FIXME leads to crash sometimes...
        }
    }
    symtab_list_finalize(&list);
}

/**
 * If the storage is of user-defined type, replaces it with
 * .DSB sizeof(type) * count
 */
static int reduce_user_storage(astnode *n, void *arg, astnode **next)
{
    astnode *type;
    astnode *count;
    astnode *byte_storage;
    symtab_entry *e;
    type = LHS(n);
    if (type->datatype == USER_DATATYPE) {
        /* Look it up */
        e = symtab_lookup(LHS(type)->ident);
        if (e != NULL) {
            /* Replace by DSB */
            count = RHS(n);
            byte_storage = astnode_create_storage(
                astnode_create_datatype(BYTE_DATATYPE, NULL, type->loc),
                astnode_create_arithmetic(
                    MUL_OPERATOR,
                    astnode_create_sizeof(
                        astnode_create_identifier(LHS(type)->ident, n->loc),
                        n->loc
                    ),
                    astnode_clone(count, n->loc),
                    n->loc
                ),
                n->loc
            );
            astnode_replace(n, byte_storage);
            astnode_finalize(n);
            *next = byte_storage;
            return 0;
        } else {
            err(n->loc, "unknown symbol `%s'", LHS(type)->ident);
            /* Remove from AST */
            astnode_remove(n);
            astnode_finalize(n);
            return 0;
        }
    }
    return 1;
}

/**
 * Second major pass over AST.
 */
void astproc_second_pass(astnode *root)
{
    /* Table of callback functions for our purpose. */
    static astnodeprocmap map[] = {
        { IDENTIFIER_NODE, validate_ref },
        { SCOPE_NODE, validate_scoperef },
        { DOT_NODE, validate_dotref },
        { INDEX_NODE, validate_index },
        { PUBLIC_NODE, tag_public_symbols },
        { STORAGE_NODE, reduce_user_storage },
        { ALIGN_NODE, tag_align_symbols },
        { STRUC_DECL_NODE, noop },
        { UNION_DECL_NODE, noop },
        { ENUM_DECL_NODE, noop },
        { RECORD_DECL_NODE, noop },
        { 0, NULL }
    };
    in_dataseg = 0; /* codeseg is default */
    /* Do the walk. */
    astproc_walk(root, NULL, map);
    /* */
    remove_unused_labels();
}

/*---------------------------------------------------------------------------*/

/**
 * Translates a single instruction.
 * @param n A node of type INSTRUCTION_NODE
 */
static int translate_instruction(astnode *n, void *arg, astnode **next)
{
    unsigned char c;
    /* Put the operand in final form */
    astnode *o = reduce_expression_complete( LHS(n) );
    assert(o == LHS(n));
    /* Convert (mnemonic, addressing mode) pair to opcode */
    n->instr.opcode = opcode_get(n->instr.mnemonic, n->instr.mode);
    /* Test if opcode is invalid */
    if (n->instr.opcode == 0xFF) {
        /* Check for the special cases */
        if ((n->instr.mnemonic == STX_MNEMONIC) && (n->instr.mode == ABSOLUTE_Y_MODE)) {
            /* Doesn't have absolute version, "scale down" to zeropage */
            n->instr.mode = ZEROPAGE_Y_MODE;
            n->instr.opcode = opcode_get(n->instr.mnemonic, n->instr.mode);
        } else if ((n->instr.mnemonic == STY_MNEMONIC) && (n->instr.mode == ABSOLUTE_X_MODE)) {
            /* Doesn't have absolute version, "scale down" to zeropage */
            n->instr.mode = ZEROPAGE_X_MODE;
            n->instr.opcode = opcode_get(n->instr.mnemonic, n->instr.mode);
        } else if (n->instr.mode == ABSOLUTE_MODE) {
            /* Check for relative addressing (these are parsed as absolute mode) */
            switch (n->instr.mnemonic) {
                case BCC_MNEMONIC:
                case BCS_MNEMONIC:
                case BEQ_MNEMONIC:
                case BMI_MNEMONIC:
                case BNE_MNEMONIC:
                case BPL_MNEMONIC:
                case BVC_MNEMONIC:
                case BVS_MNEMONIC:
                /* Fix addressing mode and opcode */
                n->instr.mode = RELATIVE_MODE;
                n->instr.opcode = opcode_get(n->instr.mnemonic, n->instr.mode);
                break;
            }
        }
    }
    if (n->instr.opcode != 0xFF) {
        /* If the operand is a constant, see if we can "reduce" from
        absolute mode to zeropage mode */
        if ((astnode_is_type(o, INTEGER_NODE)) &&
        ((unsigned long)o->integer < 256) &&
        ((c = opcode_zp_equiv(n->instr.opcode)) != 0xFF)) {
            /* Switch to the zeromode version */
            n->instr.opcode = c;
            switch (n->instr.mode) {
                case ABSOLUTE_MODE: n->instr.mode = ZEROPAGE_MODE;  break;
                case ABSOLUTE_X_MODE:   n->instr.mode = ZEROPAGE_X_MODE;break;
                case ABSOLUTE_Y_MODE:   n->instr.mode = ZEROPAGE_Y_MODE;break;
                default: /* Impossible to get here, right? */ break;
            }
        }
        /* If the operand is a constant, make sure it fits */
        if (astnode_is_type(o, INTEGER_NODE)) {
            switch (n->instr.mode) {
                case IMMEDIATE_MODE:
                case ZEROPAGE_MODE:
                case ZEROPAGE_X_MODE:
                case ZEROPAGE_Y_MODE:
                case PREINDEXED_INDIRECT_MODE:
                case POSTINDEXED_INDIRECT_MODE:
                /* Operand must fit in 8 bits */
                if (!IS_BYTE_VALUE(o->integer)) {
                    warn(o->loc, "operand out of range; truncated");
                    o->integer &= 0xFF;
                }
                break;

                case ABSOLUTE_MODE:
                case ABSOLUTE_X_MODE:
                case ABSOLUTE_Y_MODE:
                case INDIRECT_MODE:
                /* Operand must fit in 8 bits */
                if ((unsigned long)o->integer >= 0x10000) {
                    warn(o->loc, "operand out of range; truncated");
                    o->integer &= 0xFFFF;
                }
                break;

                case RELATIVE_MODE:
                /* Constant isn't allowed here is it? */
                break;

                default:
                break;
            }
        }
        else if (astnode_is_type(o, STRING_NODE)) {
            /* String operand doesn't make sense here */
            err(n->loc, "invalid operand");
        }
    } else {
        /* opcode_get() returned 0xFF */
        err(n->loc, "invalid addressing mode");
    }
    return 0;
}

/**
 * ### Is this really such a good idea?
 */
static int maybe_merge_data(astnode *n, void *arg, astnode **next)
{
    astnode *temp;
    astnode *type;
    type = LHS(n);
    /* Only merge if no debugging, otherwise line information is lost. */
    if (!xasm_args.debug && astnode_is_type(*next, DATA_NODE) &&
    astnode_equal(type, LHS(*next)) ) {
        /* Merge ahead */
        temp = *next;
        astnode_finalize( astnode_remove_child_at(temp, 0) );   /* Remove datatype node */
        astnode_add_child(n, astnode_remove_children(temp) );
        astnode_finalize(temp);
        *next = n;
    } else {
        /* Reduce expressions to final form */
        for (n = n->first_child; n != NULL; n = temp->next_sibling) {
            temp = reduce_expression_complete(n);
            if (astnode_is_type(temp, INTEGER_NODE)) {
                /* Check that value fits according to datatype */
                switch (type->datatype) {
                    case BYTE_DATATYPE:
                    if (!IS_BYTE_VALUE(temp->integer)) {
                        warn(temp->loc, "operand out of range; truncated");
                        temp->integer &= 0xFF;
                    }
                    break;

                    case WORD_DATATYPE:
                    if (!IS_WORD_VALUE(temp->integer)) {
                        warn(temp->loc, "operand out of range; truncated");
                        temp->integer &= 0xFFFF;
                    }
                    break;

                    case DWORD_DATATYPE:
                    break;

                    default:
                    break;
                }
            }
        }
    }
    return 0;
}

/**
 *
 */
static int maybe_merge_storage(astnode *n, void *arg, astnode **next)
{
    astnode *temp;
    astnode *new_count;
    astnode *old_count;
    if (astnode_is_type(*next, STORAGE_NODE) &&
    astnode_equal(LHS(n), LHS(*next)) ) {
        /* Merge ahead */
        temp = *next;
        astnode_finalize( astnode_remove_child_at(temp, 0) );   /* Remove datatype node */
        old_count = RHS(n);
        /* Calculate new count */
        new_count = astnode_create_arithmetic(
            PLUS_OPERATOR,
            astnode_remove_child_at(temp, 0),
            astnode_clone(old_count, n->loc),
            n->loc
        );
        new_count = reduce_expression_complete(new_count);
        astnode_replace(old_count, new_count);
        astnode_finalize(old_count);
        astnode_finalize(temp);
        *next = n;
    } else {
        reduce_expression_complete(RHS(n));
    }
    return 0;
}

/**
 * Replaces .proc by its label followed by statements.
 */
static int flatten_proc(astnode *n, void *arg, astnode **next)
{
    astnode *id = LHS(n);
    astnode *list = RHS(n);
    astnode_remove(id);
    id->type = LABEL_NODE;
    astnode_insert_child(list, id, 0);
    astnode *stmts = astnode_remove_children(list);
    astnode_replace(n, stmts);
    astnode_finalize(n);
    *next = stmts;
    return 0;
}

/**
 *
 */
static int flatten_var_decl(astnode *n, void *arg, astnode **next)
{
    astnode *stmts = LHS(n);
    astnode_remove_children(n);
    stmts->type = LABEL_NODE;
    astnode_replace(n, stmts);
    astnode_finalize(n);
    *next = stmts;
    return 0;
}

/**
 * Third and final pass (if the output isn't pure 6502).
 * Translates instructions, merges data and storage nodes,
 * and reduces their operands to final form on the way.
 */
void astproc_third_pass(astnode *root)
{
    /* Table of callback functions for our purpose. */
    static astnodeprocmap map[] = {
        { INSTRUCTION_NODE, translate_instruction },
        { DATA_NODE, maybe_merge_data },
        { STORAGE_NODE, maybe_merge_storage },
        { VAR_DECL_NODE, flatten_var_decl },
        { PROC_NODE, flatten_proc },
        { STRUC_DECL_NODE, noop },
        { UNION_DECL_NODE, noop },
        { ENUM_DECL_NODE, noop },
        { RECORD_DECL_NODE, noop },
        { 0, NULL }
    };
    in_dataseg = 0; /* codeseg is default */
    /* Do the walk. */
    astproc_walk(root, NULL, map);
}

/*---------------------------------------------------------------------------*/

/**
 * Evaluates the given expression, _without_ replacing it in the AST
 * (unlike astproc_reduce_expression() and friends).
 */
static astnode *eval_expression(astnode *expr)
{
    switch (astnode_get_type(expr)) {

        case ARITHMETIC_NODE: {
        astnode *lhs = eval_expression(LHS(expr));
        astnode *rhs = eval_expression(RHS(expr));
        switch (expr->oper) {
            /* Binary ops */
            case PLUS_OPERATOR:
            case MINUS_OPERATOR:
            case MUL_OPERATOR:
            case DIV_OPERATOR:
            case MOD_OPERATOR:
            case AND_OPERATOR:
            case OR_OPERATOR:
            case XOR_OPERATOR:
            case SHL_OPERATOR:
            case SHR_OPERATOR:
            case LT_OPERATOR:
            case GT_OPERATOR:
            case EQ_OPERATOR:
            case NE_OPERATOR:
            case LE_OPERATOR:
            case GE_OPERATOR:
            if (astnode_is_type(lhs, INTEGER_NODE)
                && astnode_is_type(rhs, INTEGER_NODE)) {
                /* Both sides are integer literals. */
                switch (expr->oper) {
                    case PLUS_OPERATOR:  return astnode_create_integer(lhs->integer + rhs->integer, expr->loc);
                    case MINUS_OPERATOR: return astnode_create_integer(lhs->integer - rhs->integer, expr->loc);
                    case MUL_OPERATOR:   return astnode_create_integer(lhs->integer * rhs->integer, expr->loc);
                    case DIV_OPERATOR:   return astnode_create_integer(lhs->integer / rhs->integer, expr->loc);
                    case MOD_OPERATOR:   return astnode_create_integer(lhs->integer % rhs->integer, expr->loc);
                    case AND_OPERATOR:   return astnode_create_integer(lhs->integer & rhs->integer, expr->loc);
                    case OR_OPERATOR:    return astnode_create_integer(lhs->integer | rhs->integer, expr->loc);
                    case XOR_OPERATOR:   return astnode_create_integer(lhs->integer ^ rhs->integer, expr->loc);
                    case SHL_OPERATOR:   return astnode_create_integer(lhs->integer << rhs->integer, expr->loc);
                    case SHR_OPERATOR:   return astnode_create_integer(lhs->integer >> rhs->integer, expr->loc);
                    case LT_OPERATOR:    return astnode_create_integer(lhs->integer < rhs->integer, expr->loc);
                    case GT_OPERATOR:    return astnode_create_integer(lhs->integer > rhs->integer, expr->loc);
                    case EQ_OPERATOR:    return astnode_create_integer(lhs->integer == rhs->integer, expr->loc);
                    case NE_OPERATOR:    return astnode_create_integer(lhs->integer != rhs->integer, expr->loc);
                    case LE_OPERATOR:    return astnode_create_integer(lhs->integer <= rhs->integer, expr->loc);
                    case GE_OPERATOR:    return astnode_create_integer(lhs->integer >= rhs->integer, expr->loc);

                    default:    /* ### Error, actually */
                    break;
                }
            }
            /* Use some mathematical identities... */
            else if ((astnode_is_type(lhs, INTEGER_NODE) && (lhs->integer == 0))
                && (expr->oper == PLUS_OPERATOR)) {
                /* 0+expr == expr */
                return astnode_clone(rhs, rhs->loc);
            } else if ((astnode_is_type(rhs, INTEGER_NODE) && (rhs->integer == 0))
                && (expr->oper == PLUS_OPERATOR)) {
                /* expr+0 == expr */
                return astnode_clone(lhs, lhs->loc);
            } else if ((astnode_is_type(lhs, INTEGER_NODE) && (lhs->integer == 1))
                && (expr->oper == MUL_OPERATOR)) {
                /* 1*expr == expr */
                return astnode_clone(rhs, rhs->loc);
            } else if ((astnode_is_type(rhs, INTEGER_NODE) && (rhs->integer == 1))
                && ((expr->oper == MUL_OPERATOR) || (expr->oper == DIV_OPERATOR)) ) {
                /* expr*1 == expr */
                /* expr/1 == expr */
                return astnode_clone(lhs, lhs->loc);
            }
            break;

            /* Unary ops */
            case NEG_OPERATOR:
            case NOT_OPERATOR:
            case LO_OPERATOR:
            case HI_OPERATOR:
            case UMINUS_OPERATOR:
            case BANK_OPERATOR:
            if (astnode_is_type(lhs, INTEGER_NODE)) {
                switch (expr->oper) {
                    case NEG_OPERATOR:  return astnode_create_integer(~lhs->integer, expr->loc);
                    case NOT_OPERATOR:  return astnode_create_integer(!lhs->integer, expr->loc);
                    case LO_OPERATOR:   return astnode_create_integer(lhs->integer & 0xFF, expr->loc);
                    case HI_OPERATOR:   return astnode_create_integer((lhs->integer >> 8) & 0xFF, expr->loc);
                    case UMINUS_OPERATOR: return astnode_create_integer(-lhs->integer, expr->loc);
                    default: break;
                }
            }
            break;
          } /* switch */
        } break;

        case INTEGER_NODE:
        return astnode_clone(expr, expr->loc);

        case IDENTIFIER_NODE: {
            symtab_entry *e = symtab_lookup(expr->ident);
// ###        assert(e->type == LABEL_SYMBOL);
            if (e->flags & ADDR_FLAG)
                return astnode_create_integer(e->address, expr->loc);
        } break;

        case CURRENT_PC_NODE:
        return astnode_create_integer(in_dataseg ? dataseg_pc : codeseg_pc, expr->loc);

        default:
        break;
    } /* switch */
    return 0;
}

/**
 * Sets the address of the label to be the currently calculated PC.
 */
static int set_label_address(astnode *label, void *arg, astnode **next)
{
    symtab_entry *e = symtab_lookup(label->ident);
// ###   assert(e && (e->type == LABEL_SYMBOL));
    e->address = in_dataseg ? dataseg_pc : codeseg_pc;
    e->flags |= ADDR_FLAG;
    return 0;
}

/**
 * Sets the current PC to the address specified by the ORG node.
 */
static int set_pc_from_org(astnode *org, void *arg, astnode **next)
{
    astnode *addr = LHS(org);
    assert(astnode_is_type(addr, INTEGER_NODE));
    if (in_dataseg)
        dataseg_pc = addr->integer;
    else
        codeseg_pc = addr->integer;
    return 0;
}

/**
 * Ensures that the given symbol is defined.
 */
static int ensure_symbol_is_defined(astnode *id, void *arg, astnode **next)
{
    symtab_entry *e = symtab_lookup(id->ident);
    assert(e);
    if ((e->flags & EXTRN_FLAG) && !(e->flags & ERROR_UNDEFINED_FLAG)) {
        err(id->loc, "cannot generate pure binary because `%s' is not defined", id->ident);
        e->flags |= ERROR_UNDEFINED_FLAG;
    }
    return 0;
}

/**
 * Increments PC according to the size of the instruction.
 */
static int inc_pc_by_instruction(astnode *instr, void *arg, astnode **next)
{
    assert(!in_dataseg);
    if (LHS(instr)) {
        /* Has operand */
        unsigned char zp_op = opcode_zp_equiv(instr->instr.opcode);
        if (zp_op != 0xFF) {
            /* See if we can optimize this to a ZP-instruction */
            astnode *operand = eval_expression(LHS(instr));
            if (operand && astnode_is_type(operand, INTEGER_NODE)) {
                if ((operand->integer >= 0) && (operand->integer < 256)) {
                    instr->instr.opcode = zp_op;
                }
                astnode_finalize(operand);
            }
        }
    }
    codeseg_pc += opcode_length(instr->instr.opcode);
    return 1;
}

/**
 * Increments PC according to the size of the defined data.
 */
static int inc_pc_by_data(astnode *data, void *arg, astnode **next)
{
    astnode *type = LHS(data);
    int count = astnode_get_child_count(data) - 1;
    int nbytes;
    assert(!in_dataseg);
    switch (type->datatype) {
        case BYTE_DATATYPE:  nbytes = count; break;
        case WORD_DATATYPE:  nbytes = count * 2; break;
        case DWORD_DATATYPE: nbytes = count * 4; break;
        default:
        assert(0);
        break;
    }
    codeseg_pc += nbytes;
    return 0;
}

/**
 * Increments PC according to the size of the included binary.
 */
static int inc_pc_by_binary(astnode *node, void *arg, astnode **next)
{
    assert(!in_dataseg);
    codeseg_pc += node->binary.size;
    return 0;
}

/**
 * Increments PC according to the size of the storage.
 */
static int inc_pc_by_storage(astnode *storage, void *arg, astnode **next)
{
    astnode *type = LHS(storage);
    assert(type->datatype == BYTE_DATATYPE);
    astnode *count = eval_expression(RHS(storage));
    if (count) {
        if (astnode_get_type(count) == INTEGER_NODE) {
            if (in_dataseg)
                dataseg_pc += count->integer;
            else
                codeseg_pc += count->integer;
        }
        astnode_finalize(count);
    }
    return 1;
}

/**
 * This pass is only performed if the output format is pure 6502.
 * It ensures that it is actually possible to generate pure 6502
 * for this syntax tree (i.e. no external symbols).
 * Furthermore, it calculates the address of all labels, so that
 * everything is ready for the final output phase.
 */
void astproc_fourth_pass(astnode *root)
{
    int x;
    /* ### Should loop while there's a change in the address of
       one or more labels */
    for (x = 0; x < 2; ++x) {
        in_dataseg = 0; /* codeseg is default */
        dataseg_pc = 0;
        codeseg_pc = 0;
        /* Table of callback functions for our purpose. */
        static astnodeprocmap map[] = {
            { DATASEG_NODE, process_dataseg },
            { CODESEG_NODE, process_codeseg },
            { ORG_NODE, set_pc_from_org },
            { LABEL_NODE, set_label_address },
            { IDENTIFIER_NODE, ensure_symbol_is_defined },
            { INSTRUCTION_NODE, inc_pc_by_instruction },
            { DATA_NODE, inc_pc_by_data },
            { STORAGE_NODE, inc_pc_by_storage },
            { BINARY_NODE, inc_pc_by_binary },
            { STRUC_DECL_NODE, noop },
            { UNION_DECL_NODE, noop },
            { ENUM_DECL_NODE, noop },
            { RECORD_DECL_NODE, noop },
            { 0, NULL }
        };
        /* Do the walk. */
        astproc_walk(root, NULL, map);
    }
}

/*---------------------------------------------------------------------------*/

/**
 * Writes an instruction.
 */
static int write_instruction(astnode *instr, void *arg, astnode **next)
{
    FILE *fp = (FILE *)arg;
    unsigned char op = instr->instr.opcode;
    int len = opcode_length(op);
    fputc(op, fp);
    if (len > 1) {
        /* Write operand */
        astnode *operand = eval_expression(LHS(instr));
        if(!astnode_is_type(operand, INTEGER_NODE)) {
            /* ### This is rather fatal, it should be a literal by this point */
            err(instr->loc, "operand does not evaluate to literal");
        } else {
            int value = operand->integer;
            if (len == 2) {
                /* Check if it's a relative jump */
                switch (op) {
                    case 0x10:
                    case 0x30:
                    case 0x50:
                    case 0x70:
                    case 0x90:
                    case 0xB0:
                    case 0xD0:
                    case 0xF0:
                    /* Calculate difference between target and address of next instruction */
                    value = value - (codeseg_pc + 2);
                    if (!IS_BYTE_VALUE(value)) {
                        err(operand->loc, "branch out of range");
                        value &= 0xFF;
                    }
                    break;

                    default:
                    if (!IS_BYTE_VALUE(value)) {
                        warn(operand->loc, "operand out of range; truncated");
                        value &= 0xFF;
                    }
                    break;
                }
                fputc((unsigned char)value, fp);
            } else {
                assert(len == 3);
                if (!IS_WORD_VALUE(value)) {
                    warn(operand->loc, "operand out of range; truncated");
                    value &= 0xFFFF;
                }
                fputc((unsigned char)value, fp);
                fputc((unsigned char)(value >> 8), fp);
            }
        }
        astnode_finalize(operand);
    }
    codeseg_pc += opcode_length(instr->instr.opcode);
    return 0;
}

/**
 * Writes data.
 */
static int write_data(astnode *data, void *arg, astnode **next)
{
    FILE *fp = (FILE *)arg;
    astnode *type = LHS(data);
    astnode *expr;
    assert(!in_dataseg);
    for (expr = RHS(data); expr != NULL; expr = astnode_get_next_sibling(expr) ) {
        int value;
        astnode *e = eval_expression(expr);
        assert(e->type == INTEGER_NODE);
        value = e->integer;
        switch (type->datatype) {
            case BYTE_DATATYPE:
            if (!IS_BYTE_VALUE(value)) {
                warn(expr->loc, "operand out of range; truncated");
                value &= 0xFF;
            }
            fputc((unsigned char)value, fp);
            codeseg_pc += 1;
            break;

            case WORD_DATATYPE:
            if (!IS_WORD_VALUE(value)) {
                warn(expr->loc, "operand out of range; truncated");
                value &= 0xFFFF;
            }
            fputc((unsigned char)value, fp);
            fputc((unsigned char)(value >> 8), fp);
            codeseg_pc += 2;
            break;

            case DWORD_DATATYPE:
            fputc((unsigned char)value, fp);
            fputc((unsigned char)(value >> 8), fp);
            fputc((unsigned char)(value >> 16), fp);
            fputc((unsigned char)(value >> 24), fp);
            codeseg_pc += 4;
            break;

            default:
            assert(0);
            break;
        }
        astnode_finalize(e);
    }
    return 0;
}

/**
 * Writes storage (padding).
 */
static int write_storage(astnode *storage, void *arg, astnode **next)
{
    FILE *fp = (FILE *)arg;
    astnode *type = LHS(storage);
    astnode *count = eval_expression(RHS(storage));
    assert(type->datatype == BYTE_DATATYPE);
    assert(!in_dataseg);
    if (count) {
        int i;
        assert(astnode_get_type(count) == INTEGER_NODE);
        for (i = 0; i < count->integer; ++i)
            fputc(0, fp);
        codeseg_pc += count->integer;
        astnode_finalize(count);
    }
    return 0;
}

/**
 * Writes binary.
 */
static int write_binary(astnode *node, void *arg, astnode **next)
{
    FILE *fp = (FILE *)arg;
    assert(!in_dataseg);
    fwrite(node->binary.data, 1, node->binary.size, fp);
    codeseg_pc += node->binary.size;
    return 0;
}

/**
 * This pass is only performed if the output format is pure 6502.
 * It writes the binary code.
 */
void astproc_fifth_pass(astnode *root)
{
    FILE *fp = fopen(xasm_args.output_file, "wb");
    if (!fp) {
        fprintf(stderr, "could not open '%s' for writing\n", xasm_args.output_file);
        ++err_count;
        return;
    }
    /* Table of callback functions for our purpose. */
    static astnodeprocmap map[] = {
        { DATASEG_NODE, process_dataseg },
        { CODESEG_NODE, process_codeseg },
        { ORG_NODE, set_pc_from_org },
        { INSTRUCTION_NODE, write_instruction },
        { DATA_NODE, write_data },
        { STORAGE_NODE, write_storage },
        { BINARY_NODE, write_binary },
        { STRUC_DECL_NODE, noop },
        { UNION_DECL_NODE, noop },
        { ENUM_DECL_NODE, noop },
        { RECORD_DECL_NODE, noop },
        { 0, NULL }
    };
    in_dataseg = 0; /* codeseg is default */
    dataseg_pc = 0;
    codeseg_pc = 0;
    /* Do the walk. */
    astproc_walk(root, fp, map);
    fclose(fp);
}
