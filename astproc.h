/*
 * $Id: astproc.h,v 1.5 2007/08/12 18:59:10 khansen Exp $
 * $Log: astproc.h,v $
 * Revision 1.5  2007/08/12 18:59:10  khansen
 * ability to generate pure 6502 binary
 *
 * Revision 1.4  2007/08/12 02:42:46  khansen
 * prettify, const
 *
 * Revision 1.3  2007/07/22 13:35:20  khansen
 * convert tabs to whitespaces
 *
 * Revision 1.2  2004/12/06 04:54:00  kenth
 * xorcyst 1.1.0
 *
 * Revision 1.1  2004/06/30 07:56:12  kenth
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

#ifndef ASTPROC_H
#define ASTPROC_H

#include "astnode.h"

/**
 * Signature for procedure to process an AST node.
 * @param node The node to process
 * @param arg General-purpose argument
 * @param next Next node to process
 * @return 0 if node children should not be processed, 1 otherwise
 */
typedef int (*astnodeproc)(astnode *, void *, astnode **);

/**
 * Structure that represents a mapping from node type to processor function.
 * All AST traversal functions rely on tables of such mappings.
 */
struct tag_astnodeprocmap {
    astnode_type type;  /* The AST node type (*_NODE, see header file) */
    astnodeproc proc;   /* The function that will process nodes of type */
};

typedef struct tag_astnodeprocmap astnodeprocmap;

/* Function prototypes */
void astproc_first_pass(astnode *);
void astproc_second_pass(astnode *);
void astproc_third_pass(astnode *);
void astproc_fourth_pass(astnode *);
void astproc_fifth_pass(astnode *, FILE *);
void astproc_walk(astnode *, void *, const astnodeprocmap *);
int astproc_err_count();

#endif  /* !ASTPROC_H */
