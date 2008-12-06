/*
 * $Id: opcode.h,v 1.2 2007/07/22 13:35:20 khansen Exp $
 * $Log: opcode.h,v $
 * Revision 1.2  2007/07/22 13:35:20  khansen
 * convert tabs to whitespaces
 *
 * Revision 1.1  2004/06/30 07:56:38  kenth
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

#ifndef OPCODE_H
#define OPCODE_H

#include <stdio.h>
#include "astnode.h"

/* Function prototypes. */

unsigned char opcode_get(instr_mnemonic, addressing_mode);
int opcode_length(unsigned char);
unsigned char opcode_zp_equiv(unsigned char);
const char *opcode_to_string(unsigned char);
addressing_mode opcode_addressing_mode(unsigned char);

#endif  /* !OPCODE_H */
