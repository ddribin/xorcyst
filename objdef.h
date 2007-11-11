/*
 * $Id: objdef.h,v 1.5 2007/07/22 13:35:20 khansen Exp $
 * $Log: objdef.h,v $
 * Revision 1.5  2007/07/22 13:35:20  khansen
 * convert tabs to whitespaces
 *
 * Revision 1.4  2004/12/19 20:46:59  kenth
 * xorcyst 1.4.0
 *
 * Revision 1.3  2004/12/18 17:02:58  kenth
 * added some LINE commands
 *
 * Revision 1.2  2004/12/14 01:51:29  kenth
 * CMD_LINE and CMD_FILE defines
 *
 * Revision 1.1  2004/06/30 07:56:32  kenth
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

#ifndef OBJDEF_H
#define OBJDEF_H

/* "It's a kind of magic" */
#define A_MAGIC 0xFACE

/* Version number; major in high nibble, minor in low nibble */
#define A_VERSION 0x14

/* Possible kinds of datatypes/symbols/etc */
#define INT_8   0x01
#define INT_16  0x02
#define INT_24  0x03
#define INT_32  0x04
#define STR_8   0x05
#define STR_16  0x06
#define EXTRN   0x07
#define LOCAL   0x08
#define PC  0x09

/* Expression operators */
#define OP_PLUS     0x10
#define OP_MINUS    0x11
#define OP_MUL      0x12
#define OP_DIV      0x13
#define OP_MOD      0x14
#define OP_SHL      0x15
#define OP_SHR      0x16
#define OP_AND      0x17
#define OP_OR       0x18
#define OP_XOR      0x19
#define OP_EQ       0x1A
#define OP_NE       0x1B
#define OP_LT       0x1C
#define OP_GT       0x1D
#define OP_LE       0x1E
#define OP_GE       0x1F
#define OP_NOT      0x20
#define OP_NEG      0x21
#define OP_LO       0x22
#define OP_HI       0x23
#define OP_UMINUS   0x24
#define OP_BANK     0x25

/* Bytecode commands */
#define CMD_FILE    0xEE    /* Operands: 8-bit count, string (filename) */
#define CMD_LINE8   0xEF    /* Operands: 8-bit line number */
#define CMD_LINE16  0xF0    /* Operands: 16-bit line number */
#define CMD_LINE24  0xF1    /* Operands: 24-bit line number */
#define CMD_LINE_INC    0xF2    /* Operands: None */
#define CMD_END     0xF3    /* Operands: None */
#define CMD_BIN8    0xF4    /* Operands: 8-bit count, string of bytes */
#define CMD_BIN16   0xF5    /* Operands: 16-bit count, string of bytes */
#define CMD_LABEL   0xF6    /* Operands: export flag byte, name (if export flag set) */
#define CMD_INSTR   0xF7    /* Operands: 6502 opcode, 16-bit expression ID */
#define CMD_DB      0xF8    /* Operands: 16-bit expression ID */
#define CMD_DW      0xF9    /* Operands: 16-bit expression ID */
#define CMD_DD      0xFA    /* Operands: 16-bit expression ID */
#define CMD_DSI8    0xFB    /* Operands: 8-bit count */
#define CMD_DSI16   0xFC    /* Operands: 16-bit count */
#define CMD_DSB     0xFD    /* Operands: 16-bit expression ID */
#define CMD_RSV1    0xFE    /* Reserved 1 */
#define CMD_RSV2    0xFF    /* Reserved 2 */

/* Bitmasks for CMD_LABEL flag byte */
#define LABEL_FLAG_EXPORT   1
#define LABEL_FLAG_ZEROPAGE 2
#define LABEL_FLAG_ALIGN    4
#define LABEL_FLAG_ADDR     8

#endif  /* !OBJDEF_H */
