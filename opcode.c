/*
 * $Id: opcode.c,v 1.2 2007/07/22 13:33:26 khansen Exp $
 * $Log: opcode.c,v $
 * Revision 1.2  2007/07/22 13:33:26  khansen
 * convert tabs to whitespaces
 *
 * Revision 1.1  2004/06/30 07:55:46  kenth
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
 * This file contains functions for converting 6502 instruction mnemonics from
 * an enumerated type to actual 6502 opcodes, getting zeropage-equivalent opcode
 * of an absolute-mode opcode, getting instruction lengths, mnemonic strings ...
 */

#include "opcode.h"

/**
 * Gets the opcode that a pair of (mnemonic, addressing mode) maps to.
 * @param mnc Instruction mnemonic
 * @param amode Addressing mode
 * @return Valid opcode byte, or 0xFF if mnc and amode are an invalid combination
 */
unsigned char opcode_get(instr_mnemonic mnc, addressing_mode amode)
{
    /* 56 instructions, 13 addressing modes. */
    static unsigned char opcode_lookup[56][13] = {
    /* ADC_MNEMONIC */
    {
        /* IMPLIED_MODE */      0xFF,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0x69,
        /* ZEROPAGE_MODE */     0x65,
        /* ZEROPAGE_X_MODE */       0x75,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0x6D,
        /* ABSOLUTE_X_MODE */       0x7D,
        /* ABSOLUTE_Y_MODE */       0x79,
        /* PREINDEXED_INDIRECT_MODE */  0x61,
        /* POSTINDEXED_INDIRECT_MODE */ 0x71,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    },
    /* AND_MNEMONIC */
    {
        /* IMPLIED_MODE */      0xFF,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0x29,
        /* ZEROPAGE_MODE */     0x25,
        /* ZEROPAGE_X_MODE */       0x35,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0x2D,
        /* ABSOLUTE_X_MODE */       0x3D,
        /* ABSOLUTE_Y_MODE */       0x39,
        /* PREINDEXED_INDIRECT_MODE */  0x21,
        /* POSTINDEXED_INDIRECT_MODE */ 0x31,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    },
    /* ASL_MNEMONIC */
    {
        /* IMPLIED_MODE */      0x0A,
        /* ACCUMULATOR_MODE */      0x0A,
        /* IMMEDIATE_MODE */        0xFF,
        /* ZEROPAGE_MODE */     0x06,
        /* ZEROPAGE_X_MODE */       0x16,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0x0E,
        /* ABSOLUTE_X_MODE */       0x1E,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    },
    /* BCC_MNEMONIC */
    {
        /* IMPLIED_MODE */      0xFF,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xFF,
        /* ZEROPAGE_MODE */     0xFF,
        /* ZEROPAGE_X_MODE */       0xFF,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0xFF,
        /* ABSOLUTE_X_MODE */       0xFF,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0x90
    },
    /* BCS_MNEMONIC */
    {
        /* IMPLIED_MODE */      0xFF,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xFF,
        /* ZEROPAGE_MODE */     0xFF,
        /* ZEROPAGE_X_MODE */       0xFF,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0xFF,
        /* ABSOLUTE_X_MODE */       0xFF,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xB0
    },
    /* BEQ_MNEMONIC */
    {
        /* IMPLIED_MODE */      0xFF,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xFF,
        /* ZEROPAGE_MODE */     0xFF,
        /* ZEROPAGE_X_MODE */       0xFF,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0xFF,
        /* ABSOLUTE_X_MODE */       0xFF,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xF0
    },
    /* BIT_MNEMONIC */
    {
        /* IMPLIED_MODE */      0xFF,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xFF,
        /* ZEROPAGE_MODE */     0x24,
        /* ZEROPAGE_X_MODE */       0xFF,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0x2C,
        /* ABSOLUTE_X_MODE */       0xFF,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    },
    /* BMI_MNEMONIC */
    {
        /* IMPLIED_MODE */      0xFF,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xFF,
        /* ZEROPAGE_MODE */     0xFF,
        /* ZEROPAGE_X_MODE */       0xFF,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0xFF,
        /* ABSOLUTE_X_MODE */       0xFF,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0x30
    },
    /* BNE_MNEMONIC */
    {
        /* IMPLIED_MODE */      0xFF,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xFF,
        /* ZEROPAGE_MODE */     0xFF,
        /* ZEROPAGE_X_MODE */       0xFF,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0xFF,
        /* ABSOLUTE_X_MODE */       0xFF,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xD0
    },
    /* BPL_MNEMONIC */
    {
        /* IMPLIED_MODE */      0xFF,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xFF,
        /* ZEROPAGE_MODE */     0xFF,
        /* ZEROPAGE_X_MODE */       0xFF,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0xFF,
        /* ABSOLUTE_X_MODE */       0xFF,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0x10
    },
    /* BRK_MNEMONIC */
    {
        /* IMPLIED_MODE */      0x00,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xFF,
        /* ZEROPAGE_MODE */     0xFF,
        /* ZEROPAGE_X_MODE */       0xFF,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0xFF,
        /* ABSOLUTE_X_MODE */       0xFF,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    },
    /* BVC_MNEMONIC */
    {
        /* IMPLIED_MODE */      0xFF,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xFF,
        /* ZEROPAGE_MODE */     0xFF,
        /* ZEROPAGE_X_MODE */       0xFF,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0xFF,
        /* ABSOLUTE_X_MODE */       0xFF,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0x50
    },
    /* BVS_MNEMONIC */
    {
        /* IMPLIED_MODE */      0xFF,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xFF,
        /* ZEROPAGE_MODE */     0xFF,
        /* ZEROPAGE_X_MODE */       0xFF,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0xFF,
        /* ABSOLUTE_X_MODE */       0xFF,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0x70
    },
    /* CLC_MNEMONIC */
    {
        /* IMPLIED_MODE */      0x18,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xFF,
        /* ZEROPAGE_MODE */     0xFF,
        /* ZEROPAGE_X_MODE */       0xFF,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0xFF,
        /* ABSOLUTE_X_MODE */       0xFF,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    },
    /* CLD_MNEMONIC */
    {
        /* IMPLIED_MODE */      0xD8,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xFF,
        /* ZEROPAGE_MODE */     0xFF,
        /* ZEROPAGE_X_MODE */       0xFF,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0xFF,
        /* ABSOLUTE_X_MODE */       0xFF,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    },
    /* CLI_MNEMONIC */
    {
        /* IMPLIED_MODE */      0x58,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xFF,
        /* ZEROPAGE_MODE */     0xFF,
        /* ZEROPAGE_X_MODE */       0xFF,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0xFF,
        /* ABSOLUTE_X_MODE */       0xFF,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    },
    /* CLV_MNEMONIC */
    {
        /* IMPLIED_MODE */      0xB8,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xFF,
        /* ZEROPAGE_MODE */     0xFF,
        /* ZEROPAGE_X_MODE */       0xFF,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0xFF,
        /* ABSOLUTE_X_MODE */       0xFF,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    },
    /* CMP_MNEMONIC */
    {
        /* IMPLIED_MODE */      0xFF,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xC9,
        /* ZEROPAGE_MODE */     0xC5,
        /* ZEROPAGE_X_MODE */       0xD5,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0xCD,
        /* ABSOLUTE_X_MODE */       0xDD,
        /* ABSOLUTE_Y_MODE */       0xD9,
        /* PREINDEXED_INDIRECT_MODE */  0xC1,
        /* POSTINDEXED_INDIRECT_MODE */ 0xD1,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    },
    /* CPX_MNEMONIC */
    {
        /* IMPLIED_MODE */      0xFF,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xE0,
        /* ZEROPAGE_MODE */     0xE4,
        /* ZEROPAGE_X_MODE */       0xFF,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0xEC,
        /* ABSOLUTE_X_MODE */       0xFF,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    },
    /* CPY_MNEMONIC */
    {
        /* IMPLIED_MODE */      0xFF,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xC0,
        /* ZEROPAGE_MODE */     0xC4,
        /* ZEROPAGE_X_MODE */       0xFF,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0xCC,
        /* ABSOLUTE_X_MODE */       0xFF,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    },
    /* DEC_MNEMONIC */
    {
        /* IMPLIED_MODE */      0xFF,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xFF,
        /* ZEROPAGE_MODE */     0xC6,
        /* ZEROPAGE_X_MODE */       0xD6,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0xCE,
        /* ABSOLUTE_X_MODE */       0xDE,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    },
    /* DEX_MNEMONIC */
    {
        /* IMPLIED_MODE */      0xCA,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xFF,
        /* ZEROPAGE_MODE */     0xFF,
        /* ZEROPAGE_X_MODE */       0xFF,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0xFF,
        /* ABSOLUTE_X_MODE */       0xFF,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    },
    /* DEY_MNEMONIC */
    {
        /* IMPLIED_MODE */      0x88,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xFF,
        /* ZEROPAGE_MODE */     0xFF,
        /* ZEROPAGE_X_MODE */       0xFF,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0xFF,
        /* ABSOLUTE_X_MODE */       0xFF,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    },
    /* EOR_MNEMONIC */
    {
        /* IMPLIED_MODE */      0xFF,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0x49,
        /* ZEROPAGE_MODE */     0x45,
        /* ZEROPAGE_X_MODE */       0x55,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0x4D,
        /* ABSOLUTE_X_MODE */       0x5D,
        /* ABSOLUTE_Y_MODE */       0x59,
        /* PREINDEXED_INDIRECT_MODE */  0x41,
        /* POSTINDEXED_INDIRECT_MODE */ 0x51,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    },
    /* INC_MNEMONIC */
    {
        /* IMPLIED_MODE */      0xFF,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xFF,
        /* ZEROPAGE_MODE */     0xE6,
        /* ZEROPAGE_X_MODE */       0xF6,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0xEE,
        /* ABSOLUTE_X_MODE */       0xFE,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    },
    /* INX_MNEMONIC */
    {
        /* IMPLIED_MODE */      0xE8,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xFF,
        /* ZEROPAGE_MODE */     0xFF,
        /* ZEROPAGE_X_MODE */       0xFF,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0xFF,
        /* ABSOLUTE_X_MODE */       0xFF,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    },
    /* INY_MNEMONIC */
    {
        /* IMPLIED_MODE */      0xC8,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xFF,
        /* ZEROPAGE_MODE */     0xFF,
        /* ZEROPAGE_X_MODE */       0xFF,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0xFF,
        /* ABSOLUTE_X_MODE */       0xFF,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    },
    /* JMP_MNEMONIC */
    {
        /* IMPLIED_MODE */      0xFF,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xFF,
        /* ZEROPAGE_MODE */     0xFF,
        /* ZEROPAGE_X_MODE */       0xFF,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0x4C,
        /* ABSOLUTE_X_MODE */       0xFF,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0x6C,
        /* RELATIVE_MODE */     0xFF
    },
    /* JSR_MNEMONIC */
    {
        /* IMPLIED_MODE */      0xFF,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xFF,
        /* ZEROPAGE_MODE */     0xFF,
        /* ZEROPAGE_X_MODE */       0xFF,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0x20,
        /* ABSOLUTE_X_MODE */       0xFF,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    },
    /* LDA_MNEMONIC */
    {
        /* IMPLIED_MODE */      0xFF,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xA9,
        /* ZEROPAGE_MODE */     0xA5,
        /* ZEROPAGE_X_MODE */       0xB5,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0xAD,
        /* ABSOLUTE_X_MODE */       0xBD,
        /* ABSOLUTE_Y_MODE */       0xB9,
        /* PREINDEXED_INDIRECT_MODE */  0xA1,
        /* POSTINDEXED_INDIRECT_MODE */ 0xB1,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    },
    /* LDX_MNEMONIC */
    {
        /* IMPLIED_MODE */      0xFF,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xA2,
        /* ZEROPAGE_MODE */     0xA6,
        /* ZEROPAGE_X_MODE */       0xFF,
        /* ZEROPAGE_Y_MODE */       0xB6,
        /* ABSOLUTE_MODE */     0xAE,
        /* ABSOLUTE_X_MODE */       0xFF,
        /* ABSOLUTE_Y_MODE */       0xBE,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    },
    /* LDY_MNEMONIC */
    {
        /* IMPLIED_MODE */      0xFF,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xA0,
        /* ZEROPAGE_MODE */     0xA4,
        /* ZEROPAGE_X_MODE */       0xB4,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0xAC,
        /* ABSOLUTE_X_MODE */       0xBC,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    },
    /* LSR_MNEMONIC */
    {
        /* IMPLIED_MODE */      0x4A,
        /* ACCUMULATOR_MODE */      0x4A,
        /* IMMEDIATE_MODE */        0xFF,
        /* ZEROPAGE_MODE */     0x46,
        /* ZEROPAGE_X_MODE */       0x56,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0x4E,
        /* ABSOLUTE_X_MODE */       0x5E,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    },
    /* NOP_MNEMONIC */
    {
        /* IMPLIED_MODE */      0xEA,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xFF,
        /* ZEROPAGE_MODE */     0xFF,
        /* ZEROPAGE_X_MODE */       0xFF,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0xFF,
        /* ABSOLUTE_X_MODE */       0xFF,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    },
    /* ORA_MNEMONIC */
    {
        /* IMPLIED_MODE */      0xFF,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0x09,
        /* ZEROPAGE_MODE */     0x05,
        /* ZEROPAGE_X_MODE */       0x15,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0x0D,
        /* ABSOLUTE_X_MODE */       0x1D,
        /* ABSOLUTE_Y_MODE */       0x19,
        /* PREINDEXED_INDIRECT_MODE */  0x01,
        /* POSTINDEXED_INDIRECT_MODE */ 0x11,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    },
    /* PHA_MNEMONIC */
    {
        /* IMPLIED_MODE */      0x48,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xFF,
        /* ZEROPAGE_MODE */     0xFF,
        /* ZEROPAGE_X_MODE */       0xFF,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0xFF,
        /* ABSOLUTE_X_MODE */       0xFF,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    },
    /* PHP_MNEMONIC */
    {
        /* IMPLIED_MODE */      0x08,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xFF,
        /* ZEROPAGE_MODE */     0xFF,
        /* ZEROPAGE_X_MODE */       0xFF,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0xFF,
        /* ABSOLUTE_X_MODE */       0xFF,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    },
    /* PLA_MNEMONIC */
    {
        /* IMPLIED_MODE */      0x68,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xFF,
        /* ZEROPAGE_MODE */     0xFF,
        /* ZEROPAGE_X_MODE */       0xFF,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0xFF,
        /* ABSOLUTE_X_MODE */       0xFF,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    },
    /* PLP_MNEMONIC */
    {
        /* IMPLIED_MODE */      0x28,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xFF,
        /* ZEROPAGE_MODE */     0xFF,
        /* ZEROPAGE_X_MODE */       0xFF,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0xFF,
        /* ABSOLUTE_X_MODE */       0xFF,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    },
    /* ROL_MNEMONIC */
    {
        /* IMPLIED_MODE */      0x2A,
        /* ACCUMULATOR_MODE */      0x2A,
        /* IMMEDIATE_MODE */        0xFF,
        /* ZEROPAGE_MODE */     0x26,
        /* ZEROPAGE_X_MODE */       0x36,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0x2E,
        /* ABSOLUTE_X_MODE */       0x3E,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    },
    /* ROR_MNEMONIC */
    {
        /* IMPLIED_MODE */      0x6A,
        /* ACCUMULATOR_MODE */      0x6A,
        /* IMMEDIATE_MODE */        0xFF,
        /* ZEROPAGE_MODE */     0x66,
        /* ZEROPAGE_X_MODE */       0x76,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0x6E,
        /* ABSOLUTE_X_MODE */       0x7E,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    },
    /* RTI_MNEMONIC */
    {
        /* IMPLIED_MODE */      0x40,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xFF,
        /* ZEROPAGE_MODE */     0xFF,
        /* ZEROPAGE_X_MODE */       0xFF,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0xFF,
        /* ABSOLUTE_X_MODE */       0xFF,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    },
    /* RTS_MNEMONIC */
    {
        /* IMPLIED_MODE */      0x60,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xFF,
        /* ZEROPAGE_MODE */     0xFF,
        /* ZEROPAGE_X_MODE */       0xFF,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0xFF,
        /* ABSOLUTE_X_MODE */       0xFF,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    },
    /* SBC_MNEMONIC */
    {
        /* IMPLIED_MODE */      0xFF,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xE9,
        /* ZEROPAGE_MODE */     0xE5,
        /* ZEROPAGE_X_MODE */       0xF5,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0xED,
        /* ABSOLUTE_X_MODE */       0xFD,
        /* ABSOLUTE_Y_MODE */       0xF9,
        /* PREINDEXED_INDIRECT_MODE */  0xE1,
        /* POSTINDEXED_INDIRECT_MODE */ 0xF1,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    },
    /* SEC_MNEMONIC */
    {
        /* IMPLIED_MODE */      0x38,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xFF,
        /* ZEROPAGE_MODE */     0xFF,
        /* ZEROPAGE_X_MODE */       0xFF,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0xFF,
        /* ABSOLUTE_X_MODE */       0xFF,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    },
    /* SED_MNEMONIC */
    {
        /* IMPLIED_MODE */      0xF8,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xFF,
        /* ZEROPAGE_MODE */     0xFF,
        /* ZEROPAGE_X_MODE */       0xFF,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0xFF,
        /* ABSOLUTE_X_MODE */       0xFF,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    },
    /* SEI_MNEMONIC */
    {
        /* IMPLIED_MODE */      0x78,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xFF,
        /* ZEROPAGE_MODE */     0xFF,
        /* ZEROPAGE_X_MODE */       0xFF,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0xFF,
        /* ABSOLUTE_X_MODE */       0xFF,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    },
    /* STA_MNEMONIC */
    {
        /* IMPLIED_MODE */      0xFF,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xFF,
        /* ZEROPAGE_MODE */     0x85,
        /* ZEROPAGE_X_MODE */       0x95,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0x8D,
        /* ABSOLUTE_X_MODE */       0x9D,
        /* ABSOLUTE_Y_MODE */       0x99,
        /* PREINDEXED_INDIRECT_MODE */  0x81,
        /* POSTINDEXED_INDIRECT_MODE */ 0x91,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    },
    /* STX_MNEMONIC */
    {
        /* IMPLIED_MODE */      0xFF,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xFF,
        /* ZEROPAGE_MODE */     0x86,
        /* ZEROPAGE_X_MODE */       0xFF,
        /* ZEROPAGE_Y_MODE */       0x96,
        /* ABSOLUTE_MODE */     0x8E,
        /* ABSOLUTE_X_MODE */       0xFF,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    },
    /* STY_MNEMONIC */
    {
        /* IMPLIED_MODE */      0xFF,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xFF,
        /* ZEROPAGE_MODE */     0x84,
        /* ZEROPAGE_X_MODE */       0x94,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0x8C,
        /* ABSOLUTE_X_MODE */       0xFF,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    },
    /* TAX_MNEMONIC */
    {
        /* IMPLIED_MODE */      0xAA,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xFF,
        /* ZEROPAGE_MODE */     0xFF,
        /* ZEROPAGE_X_MODE */       0xFF,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0xFF,
        /* ABSOLUTE_X_MODE */       0xFF,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    },
    /* TAY_MNEMONIC */
    {
        /* IMPLIED_MODE */      0xA8,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xFF,
        /* ZEROPAGE_MODE */     0xFF,
        /* ZEROPAGE_X_MODE */       0xFF,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0xFF,
        /* ABSOLUTE_X_MODE */       0xFF,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    },
    /* TSX_MNEMONIC */
    {
        /* IMPLIED_MODE */      0xBA,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xFF,
        /* ZEROPAGE_MODE */     0xFF,
        /* ZEROPAGE_X_MODE */       0xFF,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0xFF,
        /* ABSOLUTE_X_MODE */       0xFF,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    },
    /* TXA_MNEMONIC */
    {
        /* IMPLIED_MODE */      0x8A,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xFF,
        /* ZEROPAGE_MODE */     0xFF,
        /* ZEROPAGE_X_MODE */       0xFF,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0xFF,
        /* ABSOLUTE_X_MODE */       0xFF,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    },
    /* TXS_MNEMONIC */
    {
        /* IMPLIED_MODE */      0x9A,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xFF,
        /* ZEROPAGE_MODE */     0xFF,
        /* ZEROPAGE_X_MODE */       0xFF,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0xFF,
        /* ABSOLUTE_X_MODE */       0xFF,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    },
    /* TYA_MNEMONIC */
    {
        /* IMPLIED_MODE */      0x98,
        /* ACCUMULATOR_MODE */      0xFF,
        /* IMMEDIATE_MODE */        0xFF,
        /* ZEROPAGE_MODE */     0xFF,
        /* ZEROPAGE_X_MODE */       0xFF,
        /* ZEROPAGE_Y_MODE */       0xFF,
        /* ABSOLUTE_MODE */     0xFF,
        /* ABSOLUTE_X_MODE */       0xFF,
        /* ABSOLUTE_Y_MODE */       0xFF,
        /* PREINDEXED_INDIRECT_MODE */  0xFF,
        /* POSTINDEXED_INDIRECT_MODE */ 0xFF,
        /* INDIRECT_MODE */     0xFF,
        /* RELATIVE_MODE */     0xFF
    }
    };
    /* */
    return opcode_lookup[mnc][amode];
}

/**
 * Gets the length of an instruction, in bytes, given an opcode.
 * The length includes the opcode (1 byte) and operand (0, 1 or 2 bytes).
 * @param op Operation code
 */
int opcode_length(unsigned char op)
{
    /* Lookup-table indexed by opcode */
    static int length_lookup[] = {
        1, 2, 0, 0, 0, 2, 2, 0, 1, 2, 1, 0, 0, 3, 3, 0,
        2, 2, 0, 0, 0, 2, 2, 0, 1, 3, 0, 0, 0, 3, 3, 0,
        3, 2, 0, 0, 2, 2, 2, 0, 1, 2, 1, 0, 3, 3, 3, 0,
        2, 2, 0, 0, 0, 2, 2, 0, 1, 3, 0, 0, 0, 3, 3, 0,
        1, 2, 0, 0, 0, 2, 2, 0, 1, 2, 1, 0, 3, 3, 3, 0,
        2, 2, 0, 0, 0, 2, 2, 0, 1, 3, 0, 0, 0, 3, 3, 0,
        1, 2, 0, 0, 0, 2, 2, 0, 1, 2, 1, 0, 3, 3, 3, 0,
        2, 2, 0, 0, 0, 2, 2, 0, 1, 3, 0, 0, 0, 3, 3, 0,
        0, 2, 0, 0, 2, 2, 2, 0, 1, 1, 1, 0, 3, 3, 3, 0,
        2, 2, 0, 0, 2, 2, 2, 0, 1, 3, 1, 0, 0, 3, 0, 0,
        2, 2, 2, 0, 2, 2, 2, 0, 1, 2, 1, 0, 3, 3, 3, 0,
        2, 2, 0, 0, 2, 2, 2, 0, 1, 3, 1, 0, 3, 3, 3, 0,
        2, 2, 0, 0, 2, 2, 2, 0, 1, 2, 1, 0, 3, 3, 3, 0,
        2, 2, 0, 0, 0, 2, 2, 0, 1, 3, 0, 0, 0, 3, 3, 0,
        2, 2, 0, 0, 2, 2, 2, 0, 1, 2, 1, 0, 3, 3, 3, 0,
        2, 2, 0, 0, 0, 2, 2, 0, 1, 3, 0, 0, 0, 3, 3, 0
    };
    /* */
    return length_lookup[op];
}

/**
 * Gets the zeropage-equivalent opcode of an absolute-mode opcode, if such a
 * conversion is possible.
 * @param op Opcode
 * @return Equivalent opcode, or 0xFF if one does not exist
 */
unsigned char opcode_zp_equiv(unsigned char op)
{
    switch (op) {
        case 0x6D:  return 0x65;    /* ADC oper */
        case 0x7D:  return 0x75;    /* ADC oper,X */
        case 0x2D:  return 0x25;    /* AND oper */
        case 0x3D:  return 0x35;    /* AND oper,X */
        case 0x0E:  return 0x06;    /* ASL oper */
        case 0x1E:  return 0x16;    /* ASL oper,X */
        case 0x2C:  return 0x24;    /* BIT oper */
        case 0xCD:  return 0xC5;    /* CMP oper */
        case 0xDD:  return 0xD5;    /* CMP oper,X */
        case 0xEC:  return 0xE4;    /* CPX oper */
        case 0xCC:  return 0xC4;    /* CPY oper */
        case 0xCE:  return 0xC6;    /* DEC oper */
        case 0xDE:  return 0xD6;    /* DEC oper,X */
        case 0x4D:  return 0x45;    /* EOR oper */
        case 0x5D:  return 0x55;    /* EOR oper,X */
        case 0xEE:  return 0xE6;    /* INC oper */
        case 0xFE:  return 0xF6;    /* INC oper,X */
        case 0xAD:  return 0xA5;    /* LDA oper */
        case 0xBD:  return 0xB5;    /* LDA oper,X */
        case 0xAE:  return 0xA6;    /* LDX oper */
        case 0xBE:  return 0xB6;    /* LDX oper,Y */
        case 0xAC:  return 0xA4;    /* LDY oper */
        case 0xBC:  return 0xB4;    /* LDY oper,X */
        case 0x4E:  return 0x46;    /* LSR oper */
        case 0x5E:  return 0x56;    /* LSR oper,X */
        case 0x0D:  return 0x05;    /* ORA oper */
        case 0x1D:  return 0x15;    /* ORA oper,X */
        case 0x2E:  return 0x26;    /* ROL oper */
        case 0x3E:  return 0x36;    /* ROL oper,X */
        case 0x6E:  return 0x66;    /* ROR oper */
        case 0x7E:  return 0x76;    /* ROR oper,X */
        case 0xED:  return 0xE5;    /* SBC oper */
        case 0xFD:  return 0xF5;    /* SBC oper,X */
        case 0x8D:  return 0x85;    /* STA oper */
        case 0x9D:  return 0x95;    /* STA oper,X */
        case 0x8E:  return 0x86;    /* STX oper */
        case 0x8C:  return 0x84;    /* STY oper */
    }
    return 0xFF;
}

/**
 * Gets the string mnemonic of a 6502 opcode.
 * @param op Opcode
 * @return String representation of opcode
 */
const char *opcode_to_string(unsigned char op)
{
    static char *mnemonic_lookup[] = {
        "BRK","ORA","???","???","???","ORA","ASL","???","PHP","ORA","ASL","???","???","ORA","ASL","???",
        "BPL","ORA","???","???","???","ORA","ASL","???","CLC","ORA","???","???","???","ORA","ASL","???",
        "JSR","AND","???","???","BIT","AND","ROL","???","PLP","AND","ROL","???","BIT","AND","ROL","???",
        "BMI","AND","???","???","???","AND","ROL","???","SEC","AND","???","???","???","AND","ROL","???",
        "RTI","EOR","???","???","???","EOR","LSR","???","PHA","EOR","LSR","???","JMP","EOR","LSR","???",
        "BVC","EOR","???","???","???","EOR","LSR","???","CLI","EOR","???","???","???","EOR","LSR","???",
        "RTS","ADC","???","???","???","ADC","ROR","???","PLA","ADC","ROR","???","JMP","ADC","ROR","???",
        "BVS","ADC","???","???","???","ADC","ROR","???","SEI","ADC","???","???","???","ADC","ROR","???",
        "???","STA","???","???","STY","STA","STX","???","DEY","???","TXA","???","STY","STA","STX","???",
        "BCC","STA","???","???","STY","STA","STX","???","TYA","STA","TXS","???","???","STA","???","???",
        "LDY","LDA","LDX","???","LDY","LDA","LDX","???","TAY","LDA","TAX","???","LDY","LDA","LDX","???",
        "BCS","LDA","???","???","LDY","LDA","LDX","???","CLV","LDA","TSX","???","LDY","LDA","LDX","???",
        "CPY","CMP","???","???","CPY","CMP","DEC","???","INY","CMP","DEX","???","CPY","CMP","DEC","???",
        "BNE","CMP","???","???","???","CMP","DEC","???","CLD","CMP","???","???","???","CMP","DEC","???",
        "CPX","SBC","???","???","CPX","SBC","INC","???","INX","SBC","NOP","???","CPX","SBC","INC","???",
        "BEQ","SBC","???","???","???","SBC","INC","???","SED","SBC","???","???","???","SBC","INC","???"
    };
    return mnemonic_lookup[op];
}
