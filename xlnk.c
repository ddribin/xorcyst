/*
 * $Id: xlnk.c,v 1.20 2007/11/11 22:35:22 khansen Exp $
 * $Log: xlnk.c,v $
 * Revision 1.20  2007/11/11 22:35:22  khansen
 * compile on mac
 *
 * Revision 1.19  2007/08/12 19:01:11  khansen
 * xorcyst 1.5.0
 *
 * Revision 1.18  2007/08/07 22:43:01  khansen
 * version
 *
 * Revision 1.17  2007/07/22 13:33:26  khansen
 * convert tabs to whitespaces
 *
 * Revision 1.16  2005/01/09 11:19:41  kenth
 * xorcyst 1.4.5
 * prints code/data addresses of public symbols when --verbose
 *
 * Revision 1.15  2005/01/05 09:33:37  kenth
 * xorcyst 1.4.4
 * fixed RAM allocator bug
 * print RAM statistics when --verbose
 *
 * Revision 1.14  2005/01/05 01:52:19  kenth
 * xorcyst 1.4.3
 *
 * Revision 1.13  2005/01/04 21:35:58  kenth
 * return error code from main() when error count > 0
 *
 * Revision 1.12  2004/12/29 21:43:55  kenth
 * xorcyst 1.4.2
 *
 * Revision 1.11  2004/12/27 06:42:05  kenth
 * fixed bug in alloc_ram()
 *
 * Revision 1.10  2004/12/25 02:23:28  kenth
 * xorcyst 1.4.1
 *
 * Revision 1.9  2004/12/19 19:58:54  kenth
 * xorcyst 1.4.0
 *
 * Revision 1.8  2004/12/18 19:10:39  kenth
 * relocation improved (I hope)
 *
 * Revision 1.7  2004/12/18 17:02:00  kenth
 * CMD_LINE*, CMD_FILE handling, error location printing
 * CMD_DSW, CMD_DSD gone
 *
 * Revision 1.6  2004/12/16 13:20:41  kenth
 * xorcyst 1.3.5
 *
 * Revision 1.5  2004/12/14 01:50:21  kenth
 * xorcyst 1.3.0
 *
 * Revision 1.4  2004/12/11 02:06:18  kenth
 * xorcyst 1.2.0
 *
 * Revision 1.3  2004/12/06 04:53:18  kenth
 * xorcyst 1.1.0
 *
 * Revision 1.2  2004/06/30 23:38:22  kenth
 * replaced argp with something else
 *
 * Revision 1.1  2004/06/30 07:56:04  kenth
 * Initial revision
 *
 * Revision 1.1  2004/06/30 07:42:03  kenth
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
 * This is the 6502 linker program. It takes one or more object files generated
 * by the 6502 assembler program, and then (rough list)
 * - maps all data labels to physical 6502 RAM
 * - relocates code to 6502 address space
 * - resolves external references
 * - writes everything to a final binary
 *
 * The input to the linker is a script file which describes the layout and
 * contents of the final binary.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include "getopt.h"
#include "objdef.h"
#include "opcode.h"
#include "script.h"
#include "unit.h"
#include "hashtab.h"

#define SAFE_FREE(m) if ((m) != NULL) { free(m); m = NULL; }

/**
 * Parses a string to an integer.
 * @param s String
 * @return Integer
 */
static int str_to_int(const char *s)
{
    if (s[0] == '$') {
        return strtol(&s[1], NULL, 16);
    }
    else if (s[0] == '%') {
        return strtol(&s[1], NULL, 2);
    }
    return strtol(s, NULL, 0);
}

/*--------------------------------------------------------------------------*/
/* Argument parsing stuff. */

static char program_version[] = "xlnk 1.5.2";

struct tag_arguments {
    char *input_file;
    int silent;
    int verbose;
};

typedef struct tag_arguments arguments;

/* Argument variables set by arg parser. */
static arguments program_args;

/* Long options for getopt_long(). */
static struct option long_options[] = {
  { "quiet",    no_argument, 0, 'q' },
  { "silent",   no_argument, 0, 's' },
  { "verbose",  no_argument, 0, 'v' },
  { "help", no_argument, 0, 0 },
  { "usage",    no_argument, 0, 0 },
  { "version",  no_argument, 0, 'V' },
  { 0 }
};

/* Prints usage message and exits. */
static void usage()
{
    printf("\
Usage: xlnk [-qsvV] [--quiet] [--silent] [--verbose] [--help] [--usage]\n\
            [--version] FILE\n\
");
    exit(0);
}

/* Prints help message and exits. */
static void help()
{
    printf("\
Usage: xlnk [OPTION...] FILE\n\
The XORcyst Linker -- it creates quite a stir\n\
\n\
  -q, -s, --quiet, --silent  Don't produce any output\n\
  -v, --verbose              Produce verbose output\n\
      --help                 Give this help list\n\
      --usage                Give a short usage message\n\
  -V, --version              Print program version\n\
\n\
Report bugs to <kentmhan@gmail.com>.\n\
");
    exit(0);
}

/* Prints version and exits. */
static void version()
{
    printf("%s\n", program_version);
    exit(0);
}

/* Parses program arguments. */
static void
parse_arguments (int argc, char **argv)
{
    int key;
    /* getopt_long stores the option index here. */
    int index = 0;

    /* Set default values. */
    program_args.silent = 0;
    program_args.verbose = 0;
    program_args.input_file = NULL;

    while ((key = getopt_long(argc, argv, "qsvV", long_options, &index)) != -1) {
        switch (key) {
            case 'q': case 's':
            program_args.silent = 1;
            break;

            case 'v':
            ++program_args.verbose;
            break;

            case 0:
            /* Use index to differentiate between options */
            if (strcmp(long_options[index].name, "usage") == 0) {
                usage();
            }
            else if (strcmp(long_options[index].name, "help") == 0) {
                help();
            }
            break;

            case 'V':
            version();
            break;

            case '?':
            /* Error message has been printed by getopt_long */
            exit(1);
            break;

            default:
            /* Forgot to handle a short option, most likely */
            exit(1);
            break;
        }
    }

    /* Must be one additional argument, which is the input file. */
    if (argc-1 != optind) {
        printf("Usage: xlnk [OPTION...] FILE\nTry `xlnk --help' or `xlnk --usage' for more information.\n");
        exit(1);
    }
    else {
        program_args.input_file = argv[optind];
    }
}

/*--------------------------------------------------------------------------*/
/* Data structures. */

/* Describes a local label in the unit. */
struct tag_local
{
    char *name; /* NULL if not exported */
    int resolved;   /* 0 initially, set to 1 when phys_addr has been assigned */
    int virt_addr;
    int phys_addr;
    int ref_count;
    int size;
    struct tag_xunit *owner;
    unsigned short align;
    unsigned char flags;
};

typedef struct tag_local local;

/* Describes an array of local labels. */
struct tag_local_array
{
    local *entries;
    int size;
};

typedef struct tag_local_array local_array;

/**
 * eXtended unit, has extra info built from basic unit ++
 */
struct tag_xunit
{
    xasm_unit _unit_;    /* NB!!! "Superclass", must be first field for casting to work */
    local_array data_locals;
    local_array code_locals;
    int bank_id;
    int data_size;
    int code_origin;
    int code_size;
    int loaded;
};

typedef struct tag_xunit xunit;

/**
 * Describes a 6502 RAM block available for allocation.
 */
struct tag_avail_ram_block
{
    int start;  /* Start address in 6502 space */
    int end;    /* End address in 6502 space (not inclusive) */
    struct tag_avail_ram_block *next;
};

typedef struct tag_avail_ram_block avail_ram_block;

/** */
struct tag_calc_address_args
{
    xunit *xu;
    int index;
};

typedef struct tag_calc_address_args calc_address_args;

/** */
struct tag_write_binary_args
{
    xunit *xu;
    FILE *fp;
};

typedef struct tag_write_binary_args write_binary_args;

/*--------------------------------------------------------------------------*/

/** Array containing the units to link. */
static xunit *units;
/* Number of units in above array. */
static int unit_count;

/** Holds the current memory address. */
static int pc;

/** Hash tables used to lookup symbols. */
static hashtab *label_hash;
static hashtab *constant_hash;
static hashtab *unit_hash;

/** Number of errors and warnings during linking */
static int err_count;
static int warn_count;

static int suppress;

/* Head of the list of available 6502 RAM blocks (for data allocation). */
static avail_ram_block *ram_block_head = NULL;

/* Total amount of 6502 RAM (bytes) that was registered */
static int total_ram = 0;

/* Bank info */
static int bank_offset;
static int bank_size;
static int bank_origin;
static int bank_id;

/* Debug info */
static const unsigned char *unit_file = NULL; /* length byte followed by chars */
static int unit_line = -1;

/* Turn on to produce flat (dis)assembly code. The resulting code can be
   assembled by xasm using the --pure-binary switch.
   It's useful for checking that the linker doesn't do anything stupid
   in binary output mode.
*/
static const int generate_assembly = 0;

/*--------------------------------------------------------------------------*/

/**
 * If the object file contains FILE and LINE bytecodes (assembled with
 * --debug switch), unit_file and unit_line will contain the current
 * source location. In that case, this function prints the location.
 */
static void maybe_print_location()
{
    char *str;
    int len;
    if (unit_file != NULL) {
        len = unit_file[0] + 1;
        str = (char *)malloc(len + 1);
        strncpy(str, (char *)&unit_file[1], len);
        str[len] = '\0';
        fprintf(stderr, "%s:%d: ", str, unit_line);
        free(str);
    }
}

/**
 * If the object doesn't contain FILE and LINE bytecodes,
 * unit_file will be <code>NULL</code>. In that case, this
 * function prints a tip about reassembling with --debug switch.
 */
static void maybe_print_debug_tip()
{
    if (unit_file == NULL) {
        fprintf(stderr, "\treassemble with --debug switch to obtain source location\n");
    }
}

/**
 * Issues an error.
 * @param fmt format string for printf
 */
static void err(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    if (!suppress) {
        maybe_print_location();
        fprintf(stderr, "error: ");
        vfprintf(stderr, fmt, ap);
        fprintf(stderr, "\n");
        maybe_print_debug_tip();
        err_count++;
    }
    va_end(ap);
}

/**
 * Issues a warning.
 * @param fmt format string for printf
 */
static void warn(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    if (!suppress) {
        maybe_print_location();
        fprintf(stderr, "warning: ");
        vfprintf(stderr, fmt, ap);
        fprintf(stderr, "\n");
        maybe_print_debug_tip();
        warn_count++;
    }
    va_end(ap);
}

/**
 * Prints a message if --verbose switch was given.
 * @param level verbosity level
 * @param fmt format string for printf
 */
static void verbose(int level, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    if (!suppress && program_args.verbose >= level) {
        vfprintf(stdout, fmt, ap);
        fprintf(stdout, "\n");
    }
    va_end(ap);
}

/*--------------------------------------------------------------------------*/
/* Functions to manage 6502 RAM blocks. */
/* The RAM allocator maintains a list of these blocks that are used to
   map the contents of the units' data segments to memory.
*/

/**
 * Calculates number of bytes of 6502 RAM left for allocation.
 */
static int ram_left()
{
    int sum;
    avail_ram_block *b;
    for (sum = 0, b = ram_block_head; b != NULL; b = b->next) {
        sum += b->end - b->start;
    }
    return sum;
}

/**
 * Adds a block of 6502 memory to the list of available memory regions.
 * When adding multiple blocks they should be added in prioritized order.
 * @param start Start address of the block
 * @param end End address of the block (non-inclusive!)
 */
static void add_ram_block(int start, int end)
{
    avail_ram_block *b;
    avail_ram_block *new_block = (avail_ram_block *)malloc( sizeof(avail_ram_block) );
    if (new_block != NULL) {
        new_block->start = start;
        new_block->end = end;
        new_block->next = NULL;
        if (ram_block_head == NULL) {
            /* Start the list */
            ram_block_head = new_block;
        } else {
            /* Add to end */
            for (b = ram_block_head; b->next != NULL; b = b->next) ;
            b->next = new_block;
        }
        verbose(1, "  added RAM block: %.4X-%.4X", new_block->start, new_block->end);
    }
}

/**
 * Allocates a chunk of 6502 RAM to a local.
 * @param l Local
 * @return 0 if there isn't enough RAM to satisfy the request (fail), 1 otherwise (success)
 */
static int alloc_ram(local *l)
{
    /* Try the available blocks in order. */
    /* Use the first one that's sufficient. */
    avail_ram_block *b;
    avail_ram_block *p = NULL;
    for (b = ram_block_head; b != NULL; p = b, b = b->next) {
        int left;
        int pad;
        avail_ram_block *n;
        if (l->flags & XASM_LABEL_FLAG_ZEROPAGE) {
            if (b->start >= 0x100) {
                continue;   /* This block is no good */
            }
        }
        left = b->end - b->start;
        if (left < l->size) {
            continue;   /* Not enough, sorry */
        }
        if (l->flags & XASM_LABEL_FLAG_ALIGN) {
            pad = b->start & ((1 << l->align) - 1);
            if (pad != 0) {
                /* This block doesn't match the alignment */
                /* Break it into two blocks if possible */
                pad = (1 << l->align) - pad;
                pad = (left < pad) ? left : pad;
                if (pad < left) {
                    n = (avail_ram_block *)malloc(sizeof(avail_ram_block));
                    n->start = b->start;
                    n->end = n->start + pad;
                    b->start += pad;
                    n->next = b;
                    if (b == ram_block_head) {
                        ram_block_head = n;    /* New head */
                    }
                    b = n;
                }
                continue;
            }
        }
        /* Pick this one. */
        l->phys_addr = b->start;
        /* Decrease block size by moving start address ahead */
        b->start += l->size;
        /* If there's no more space left in this block, discard it */
        if (left == l->size) {
            /* Remove from linked list */
            if (p == NULL) {
                /* Set successor block as new head */
                ram_block_head = b->next;
            }
            else {
                /* Squeeze out */
                p->next = b->next;
            }
            SAFE_FREE(b);
        }
        return 1;
    }
    return 0;
}

/**
 * Frees up memory associated with list of RAM blocks.
 */
static void finalize_ram_blocks()
{
    avail_ram_block *b;
    avail_ram_block *t;
    for (b = ram_block_head; b != NULL; b = t) {
        t = b->next;
        SAFE_FREE(b);
    }
}

/*--------------------------------------------------------------------------*/
/* Functions to get big-endian values from byte buffer. */

/* Gets single byte from buffer and increments index. */
static unsigned char get_1(const unsigned char *b, int *i)
{
    return b[(*i)++];
}
/* Gets big-endian short from buffer and increments index. */
static unsigned short get_2(const unsigned char *b, int *i)
{
    unsigned short result = get_1(b, i) << 8;
    result |= get_1(b, i);
    return result;
}
/* Gets big-endian 24-bit integer from buffer and increments index. */
static unsigned int get_3(const unsigned char *b, int *i)
{
    unsigned int result = get_2(b, i) << 8;
    result |= get_1(b, i);
    return result;
}
/* Gets big-endian int from buffer and increments index. */
/*static unsigned int get_4(unsigned char *b, int *i)
{
    unsigned int result = get_2(b, i) << 16;
    result |= get_2(b, i);
    return result;
}
*/
/*--------------------------------------------------------------------------*/

/**
 * Calculates the storage occupied by a CMD_LABEL bytecode's arguments.
 */
static int label_cmd_args_size(const unsigned char *bytes)
{
    int size = 1;   /* Smallest possible: flag byte */
    int flags = bytes[0];
    if (flags & XASM_LABEL_FLAG_EXPORT) { size += bytes[1] + 1 + 1; }    /* Length byte + string */
    if (flags & XASM_LABEL_FLAG_ALIGN) { size += 1; }            /* Alignment */
    if (flags & XASM_LABEL_FLAG_ADDR) { size += 2; }         /* Address */
    return size;
}

/**
 * Walks an array of bytecodes, calling corresponding bytecode handlers
 * along the way.
 * @param bytes Array of bytecodes, terminated by CMD_END
 * @param handlers Array of bytecode handlers (entries can be NULL)
 * @param arg Argument passed to bytecode handler, can be anything
 */
static void bytecode_walk(const unsigned char *bytes, xasm_bytecodeproc *handlers, void *arg)
{
    int i;
    unsigned char cmd;
    unit_file = NULL;
    unit_line = -1;
    if (bytes == NULL) { return; }
    i = 0;
    do {
        cmd = get_1(bytes, &i);

        /* Check if debug command */
        if (cmd < XASM_CMD_END) {
            switch (cmd) {
                case XASM_CMD_FILE:
                unit_file = &bytes[i];
                i += get_1(bytes, &i) + 1;  /* Skip count and array of bytes */
                break;
                case XASM_CMD_LINE8:  unit_line = get_1(bytes, &i);  break;
                case XASM_CMD_LINE16: unit_line = get_2(bytes, &i);  break;
                case XASM_CMD_LINE24: unit_line = get_3(bytes, &i);  break;
                case XASM_CMD_LINE_INC: unit_line++; break;
            }
            continue;
        }

        if (handlers[cmd-XASM_CMD_END] != NULL) {
            handlers[cmd-XASM_CMD_END](&bytes[i-1], arg);
        }
        /* Skip any bytecode arguments */
        switch (cmd) {
            case XASM_CMD_END:   break;
            case XASM_CMD_BIN8:  i += get_1(bytes, &i) + 1;  break;  /* Skip count and array of bytes */
            case XASM_CMD_BIN16: i += get_2(bytes, &i) + 1;  break;  /* Skip count and array of bytes */
            case XASM_CMD_LABEL: i += label_cmd_args_size(&bytes[i]); break; /* Skip flag byte and possibly name and alignment */
            case XASM_CMD_INSTR: i += 3; break;  /* Skip 6502 opcode and 16-bit expr id */
            case XASM_CMD_DB:    i += 2; break;  /* Skip 16-bit expr id */
            case XASM_CMD_DW:    i += 2; break;  /* Skip 16-bit expr id */
            case XASM_CMD_DD:    i += 2; break;  /* Skip 16-bit expr id */
            case XASM_CMD_DSI8:  i += 1; break;  /* Skip 8-bit count */
            case XASM_CMD_DSI16: i += 2; break;  /* Skip 16-bit count */
            case XASM_CMD_DSB:   i += 2; break;  /* Skip 16-bit expr id */

            default:
            err("invalid bytecode");
            break;
        }
    } while (cmd != XASM_CMD_END);
}

/*--------------------------------------------------------------------------*/
/* Functions for expression evaluation. */

/**
 * Finalizes a constant.
 * @param c Constant to finalize
 */
/* ### Merge with finalize_constant() in unit.c? */
static void finalize_constant(xasm_constant *c)
{
    if (c->type == XASM_STRING_CONSTANT) {
        SAFE_FREE(c->string);
    }
}

/**
 * Evaluates an expression recursively.
 * The result will either be a integer or string literal, indicating successful
 * evaluation; or an invalid type indicating that a symbol could not be translated
 * to a constant (in other words, it could not be resolved). In this case,
 * result->string contains the name of the symbol which couldn't be evaluated.
 * @param u The unit where the expression is contained
 * @param e The expression to evaluate
 * @param result Pointer to resulting value
 */
static void eval_recursive(xunit *u, xasm_expression *e, xasm_constant *result)
{
    char *s;
    local *l;
    xasm_constant *c;
    xasm_constant lhs_result, rhs_result;
    switch (e->type) {
        case XASM_OPERATOR_EXPRESSION:
        switch (e->op_expr.operator) {
            /* Binary operators */
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
            /* Evaluate both sides */
            eval_recursive(u, e->op_expr.lhs, &lhs_result);
            eval_recursive(u, e->op_expr.rhs, &rhs_result);
            /* If either side is unresolved, then result is unresolved. */
            if ((lhs_result.type == -1) || (rhs_result.type == -1)) {
                result->type = -1;
            }
            /* If both sides are integer, then result is integer. */
            else if ((lhs_result.type == XASM_INTEGER_CONSTANT) &&
            (rhs_result.type == XASM_INTEGER_CONSTANT)) {
                result->type = XASM_INTEGER_CONSTANT;
                /* Perform the proper operation to obtain result. */
                switch (e->op_expr.operator) {
                    case XASM_OP_PLUS:   result->integer = lhs_result.integer + rhs_result.integer;  break;
                    case XASM_OP_MINUS:  result->integer = lhs_result.integer - rhs_result.integer;  break;
                    case XASM_OP_MUL:    result->integer = lhs_result.integer * rhs_result.integer;  break;
                    case XASM_OP_DIV:    result->integer = lhs_result.integer / rhs_result.integer;  break;
                    case XASM_OP_MOD:    result->integer = lhs_result.integer % rhs_result.integer;  break;
                    case XASM_OP_SHL:    result->integer = lhs_result.integer << rhs_result.integer; break;
                    case XASM_OP_SHR:    result->integer = lhs_result.integer >> rhs_result.integer; break;
                    case XASM_OP_AND:    result->integer = lhs_result.integer & rhs_result.integer;  break;
                    case XASM_OP_OR: result->integer = lhs_result.integer | rhs_result.integer;  break;
                    case XASM_OP_XOR:    result->integer = lhs_result.integer ^ rhs_result.integer;  break;
                    case XASM_OP_EQ: result->integer = lhs_result.integer == rhs_result.integer; break;
                    case XASM_OP_NE: result->integer = lhs_result.integer != rhs_result.integer; break;
                    case XASM_OP_LT: result->integer = lhs_result.integer < rhs_result.integer;  break;
                    case XASM_OP_GT: result->integer = lhs_result.integer > rhs_result.integer;  break;
                    case XASM_OP_LE: result->integer = lhs_result.integer <= rhs_result.integer; break;
                    case XASM_OP_GE: result->integer = lhs_result.integer >= rhs_result.integer; break;
                }
            }
            /* If both sides are string... */
            else if ((lhs_result.type == XASM_STRING_CONSTANT) &&
            (rhs_result.type == XASM_STRING_CONSTANT)) {
                switch (e->op_expr.operator) {
                    case XASM_OP_PLUS:
                    /* Concatenate */
                    result->string = (char *)malloc(strlen(lhs_result.string)+strlen(rhs_result.string)+1);
                    if (result->string != NULL) {
                        strcpy(result->string, lhs_result.string);
                        strcat(result->string, rhs_result.string);
                        result->type = XASM_STRING_CONSTANT;
                    }
                    break;

                    /* String comparison: using strcmp() */
                    case XASM_OP_EQ: result->integer = strcmp(lhs_result.string, rhs_result.string) == 0;    break;
                    case XASM_OP_NE: result->integer = strcmp(lhs_result.string, rhs_result.string) != 0;    break;
                    case XASM_OP_LT: result->integer = strcmp(lhs_result.string, rhs_result.string) < 0; break;
                    case XASM_OP_GT: result->integer = strcmp(lhs_result.string, rhs_result.string) > 0; break;
                    case XASM_OP_LE: result->integer = strcmp(lhs_result.string, rhs_result.string) <= 0;    break;
                    case XASM_OP_GE: result->integer = strcmp(lhs_result.string, rhs_result.string) >= 0;    break;

                    default:
                    /* Not defined operator for string operation... */
                    assert(0);
                    break;
                }
            }
            else {
                result->type = -1;
                err("incompatible operands to `%s' in expression", xasm_operator_to_string(e->op_expr.operator) );
            }
            /* Discard the operands */
            finalize_constant(&lhs_result);
            finalize_constant(&rhs_result);
            break;  /* Binary operator */

            /* Unary operators */
            case XASM_OP_NOT:
            case XASM_OP_NEG:
            case XASM_OP_LO:
            case XASM_OP_HI:
            case XASM_OP_UMINUS:
            /* Evaluate the single operand */
            eval_recursive(u, e->op_expr.lhs, &lhs_result);
            /* If operand is unresolved then result is unresolved. */
            if (lhs_result.type == -1) {
                result->type = -1;
            }
            /* If operand is integer then result is integer. */
            else if (lhs_result.type == XASM_INTEGER_CONSTANT) {
                result->type = XASM_INTEGER_CONSTANT;
                /* Perform the proper operation to obtain result. */
                switch (e->op_expr.operator) {
                    case XASM_OP_NOT:    result->integer = !lhs_result.integer;      break;
                    case XASM_OP_NEG:    result->integer = ~lhs_result.integer;      break;
                    case XASM_OP_LO: result->integer = lhs_result.integer & 0xFF;    break;
                    case XASM_OP_HI: result->integer = (lhs_result.integer >> 8) & 0xFF; break;
                    case XASM_OP_UMINUS: result->integer = -lhs_result.integer;      break;
                }
            }
            else {
                /* Error, invalid operand */
                err("incompatible operand to `%s' in expression", xasm_operator_to_string(e->op_expr.operator) );
                result->type = -1;
            }
            /* Discard the operand */
            finalize_constant(&lhs_result);
            break;  /* Unary operator */

            case XASM_OP_BANK:
            switch (e->op_expr.lhs->type) {
                case XASM_LOCAL_EXPRESSION:
                /* Simple, it must be in the same (current) bank */
                result->integer = bank_id;
                result->type = XASM_INTEGER_CONSTANT;
                break;

                case XASM_EXTERNAL_EXPRESSION:
                s = u->_unit_.externals[e->op_expr.lhs->extrn_id].name;
                if ((l = (local *)hashtab_get(label_hash, s)) != NULL) {
                    /* It's a label */
                    result->integer = l->owner->bank_id;
                    result->type = XASM_INTEGER_CONSTANT;
                }
                else if ((c = (xasm_constant *)hashtab_get(constant_hash, s)) != NULL) {
                    /* It's a constant */
                    result->integer = ((xunit *)c->unit)->bank_id;
                    result->type = XASM_INTEGER_CONSTANT;
                }
                else {
                    result->type = -1;
                }
                break;

                default:
                result->type = -1;
                break;
            }
            break;
        }
        break;

        case XASM_INTEGER_EXPRESSION:
        result->type = XASM_INTEGER_CONSTANT;
        result->integer = e->integer;
        break;

        case XASM_STRING_EXPRESSION:
        result->string = (char *)malloc(strlen(e->string) + 1);
        if (result->string != NULL) {
            strcpy(result->string, e->string);
            result->type = XASM_STRING_CONSTANT;
        }
        break;

        case XASM_LOCAL_EXPRESSION:
        if (e->local_id >= u->data_locals.size) {
            /* It's a code local */
            l = &u->code_locals.entries[e->local_id - u->data_locals.size];
        }
        else {
            /* It's a data local */
            l = &u->data_locals.entries[e->local_id];
        }
        if (l->resolved) {
            result->type = XASM_INTEGER_CONSTANT;
            result->integer = l->phys_addr;
        }
        else {
            /* Not resolved (yet, at least) */
            result->type = -1;
        }
        break;

        case XASM_EXTERNAL_EXPRESSION:
        s = u->_unit_.externals[e->extrn_id].name;
        if ((l = (local *)hashtab_get(label_hash, s)) != NULL) {
            /* It's a label */
            if (l->resolved) {
                result->type = XASM_INTEGER_CONSTANT;
                result->integer = l->phys_addr;
            }
            else {
                /* Not resolved (yet) */
                result->type = -1;
            }
        }
        else if ((c = (xasm_constant *)hashtab_get(constant_hash, s)) != NULL) {
            switch (c->type) {
                case XASM_INTEGER_CONSTANT:
                result->type = XASM_INTEGER_CONSTANT;
                result->integer = c->integer;
                break;

                case XASM_STRING_CONSTANT:
                result->string = (char *)malloc(strlen(c->string) + 1);
                if (result->string != NULL) {
                    strcpy(result->string, c->string);
                    result->type = XASM_STRING_CONSTANT;
                }
                break;
            }
        }
        else {
            result->type = -1;
            err("unknown symbol `%s' referenced from %s", s, u->_unit_.name);
        }
        break;

        case XASM_PC_EXPRESSION:
        result->type = XASM_INTEGER_CONSTANT;
        result->integer = pc;
        break;
    }
}

/**
 * Evaluates an expression.
 * @param u The unit where the expression is contained
 * @param exid The unique ID of the expression
 * @param result Where to store the result of the evaluation
 */
static void eval_expression(xunit *u, int exid, xasm_constant *result)
{
    xasm_expression *exp = u->_unit_.expressions[exid];
    eval_recursive(u, exp, result);
}

/*--------------------------------------------------------------------------*/
/* Functions for incrementing PC, with error handling for wraparound. */

/**
 * Increases PC by amount.
 * Issues error if the PC wraps around.
 */
static void inc_pc(int amount, void *arg)
{
    calc_address_args *aargs;
    if ((pc <= 0x10000) && ((pc+amount) > 0x10000)) {
        aargs = (calc_address_args *)arg;
        err("PC went beyond 64K when linking `%s'", aargs->xu->_unit_.name);
    }
    pc += amount;
}

/**
 * Increases PC by 8-bit value immediately following bytecode command.
 */
static void inc_pc_count8(const unsigned char *b, void *arg)
{
    int i = 1;
    inc_pc( get_1(b, &i) + 1, arg );
}

/**
 * Increases PC by 16-bit value immediately following bytecode command.
 */
static void inc_pc_count16(const unsigned char *b, void *arg)
{
    int i = 1;
    inc_pc( get_2(b, &i) + 1, arg );
}

/**
 * Increases PC by 1.
 */
static void inc_pc_1(const unsigned char *b, void *arg)
{
    inc_pc( 1, arg );
}

/**
 * Increases PC by 2.
 */
static void inc_pc_2(const unsigned char *b, void *arg)
{
    inc_pc( 2, arg );
}

/**
 * Increases PC by 4.
 */
static void inc_pc_4(const unsigned char *b, void *arg)
{
    inc_pc( 4, arg );
}

/**
 * Increases PC according to size of define data command.
 */
static void inc_pc_dsb(const unsigned char *b, void *arg)
{
    xasm_constant c;
    int exid;
    calc_address_args *args = (calc_address_args *)arg;
    int i = 1;
    /* Get expression ID */
    exid = get_2(b, &i);
    /* Evaluate expression */
    eval_expression(args->xu, exid, &c);
    /* Handle the result */
    if (c.type == XASM_INTEGER_CONSTANT) {
        /* An array of bytes will be located here */
        /* Advance PC appropriately */
        inc_pc( c.integer, arg );
    }
    else if (c.type == XASM_STRING_CONSTANT) {
        err("unexpected string operand (`%s') to storage directive", c.string);
    }
    else {
        //err("unresolved symbol");
        assert(0);
    }

    finalize_constant(&c);
}

/**
 * Increments PC according to the length of this instruction.
 */
static void inc_pc_instr(const unsigned char *b, void *arg)
{
    xasm_constant c;
    unsigned char op, t;
    int exid;
    calc_address_args *args = (calc_address_args *)arg;
    /* Get opcode */
    int i = 1;
    op = get_1(b, &i);
    /* Get expression ID */
    exid = get_2(b, &i);
    /* Evaluate it */
    eval_expression(args->xu, exid, &c);
    /* Handle the result */
    if (c.type == XASM_INTEGER_CONSTANT) {
        /* See if it can be reduced to ZP instruction */
        if ((c.integer < 0x100) &&
        ((t = opcode_zp_equiv(op)) != 0xFF)) {
            /* replace op by ZP-version */
            op = t;
            ((unsigned char*)b)[1] = t;
        }
    }
    else if (c.type == XASM_STRING_CONSTANT) {
        err("invalid instruction operand (string)");
    }
    else {
        /* Address not available yet (forward reference). */
        //err("unresolved symbol");
    }
    /* Advance PC */
    inc_pc( opcode_length(op), arg );
}

/*--------------------------------------------------------------------------*/
/* Functions for writing pure 6502 binary from bytecodes. */

/**
 * Writes an array of bytes.
 */
static void write_bin8(const unsigned char *b, void *arg)
{
    int count;
    int i;
    write_binary_args *args = (write_binary_args *)arg;
    i = 1;
    count = get_1(b, &i) + 1;
    fwrite(&b[i], 1, count, args->fp);
    inc_pc( count, arg );
}

/**
 * Writes an array of bytes.
 */
static void write_bin16(const unsigned char *b, void *arg)
{
    int count;
    int i;
    write_binary_args *args = (write_binary_args *)arg;
    i = 1;
    count = get_2(b, &i) + 1;
    fwrite(&b[i], 1, count, args->fp);
    inc_pc( count, arg );
}

/**
 * Writes an instruction.
 */
static void write_instr(const unsigned char *b, void *arg)
{
    xasm_constant c;
    unsigned char op;
    int i;
    int exid;
    write_binary_args *args = (write_binary_args *)arg;
    /* Get opcode */
    i = 1;
    op = get_1(b, &i);
    assert(opcode_length(op) > 1);
    /* Get expression ID */
    exid = get_2(b, &i);
    /* Evaluate expression */
    eval_expression(args->xu, exid, &c);
    assert(c.type == XASM_INTEGER_CONSTANT);
    /* Write the opcode */
    fputc(op, args->fp);
    if (opcode_length(op) == 2) {
        /* Operand must fit in 1 byte */
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
            c.integer = c.integer - (pc + 2);
            /* Make sure jump is in range */
            if ( (c.integer < -128) || (c.integer > 127) ) {
                err("branch out of range");
            }
            /* Make it a byte value */
            c.integer &= 0xFF;
            break;
        }
        if (c.integer >= 0x100) {
            err("instruction operand doesn't fit in 1 byte");
        }
        else {
            /* Write it */
            fputc(c.integer, args->fp);
        }
    } else {
        assert(opcode_length(op) == 3);
        /* Operand must fit in 2 bytes */
        if (c.integer >= 0x10000) {
            err("instruction operand doesn't fit in 2 bytes");
        }
        else {
            /* Write it, low byte first */
            fputc(c.integer, args->fp);
            fputc(c.integer >> 8, args->fp);
        }
    }
    inc_pc( opcode_length(op), arg );
}

/**
 * Writes a byte, word or dword.
 */
static void write_dx(const unsigned char *b, void *arg)
{
    xasm_constant c;
    int i;
    int exid;
    write_binary_args *args = (write_binary_args *)arg;
    /* Get expression ID */
    i = 1;
    exid = get_2(b, &i);
    /* Evaluate expression */
    eval_expression(args->xu, exid, &c);

    if (c.type == XASM_INTEGER_CONSTANT) {
        /* Write low byte */
        fputc(c.integer, args->fp);
        /* If 2+ bytes, write high ones */
        switch (b[0]) {
            case XASM_CMD_DB:
            if (c.integer > 0xFF) {
                warn("`.DB' operand $%X out of range; truncated", c.integer);
            }
            break;

            case XASM_CMD_DW:
            fputc(c.integer >> 8, args->fp);
            if (c.integer > 0xFFFF) {
                warn("`.DW' operand $%X out of range; truncated", c.integer);
            }
            break;

            case XASM_CMD_DD:
            fputc(c.integer >> 8, args->fp);
            fputc(c.integer >> 16, args->fp);
            fputc(c.integer >> 24, args->fp);
            break;
        }
        /* Advance PC */
        switch (b[0]) {
            case XASM_CMD_DB:    inc_pc( 1, arg );   break;
            case XASM_CMD_DW:    inc_pc( 2, arg );   break;
            case XASM_CMD_DD:    inc_pc( 4, arg );   break;
        }
    }
    else if (c.type == XASM_STRING_CONSTANT) {
        for (i=0; i<strlen(c.string); i++) {
            /* Write low byte */
            fputc(c.string[i], args->fp);
            /* If 2+ bytes, write high ones */
            switch (b[0]) {
                case XASM_CMD_DW:
                fputc(0, args->fp);
                break;

                case XASM_CMD_DD:
                fputc(0, args->fp);
                fputc(0, args->fp);
                fputc(0, args->fp);
                break;
            }
            /* Advance PC */
            switch (b[0]) {
                case XASM_CMD_DB:    inc_pc( 1, arg );   break;
                case XASM_CMD_DW:    inc_pc( 2, arg );   break;
                case XASM_CMD_DD:    inc_pc( 4, arg );   break;
            }
        }
    } else {
        assert(0);
    }

    finalize_constant(&c);
}

/**
 * Writes a series of zeroes.
 */
static void write_dsi8(const unsigned char *b, void *arg)
{
    int count;
    int i;
    write_binary_args *args = (write_binary_args *)arg;
    i = 1;
    count = get_1(b, &i) + 1;
    for (i=0; i<count; i++) {
        fputc(0, args->fp);
    }
    inc_pc( count, arg );
}

/**
 * Writes a series of zeroes.
 */
static void write_dsi16(const unsigned char *b, void *arg)
{
    int count;
    int i;
    write_binary_args *args = (write_binary_args *)arg;
    i = 1;
    count = get_2(b, &i) + 1;
    for (i=0; i<count; i++) {
        fputc(0, args->fp);
    }
    inc_pc( count, arg );
}

/**
 * Writes a series of zeroes.
 */
static void write_dsb(const unsigned char *b, void *arg)
{
    xasm_constant c;
    int i;
    int exid;
    write_binary_args *args = (write_binary_args *)arg;
    /* Get expression ID */
    i = 1;
    exid = get_2(b, &i);
    /* Evaluate expression */
    eval_expression(args->xu, exid, &c);
    assert(c.type == XASM_INTEGER_CONSTANT);
    if (c.integer < 0) {
        err("negative count");
    } else if (c.integer > 0) {
        for (i=0; i<c.integer; i++) {
            fputc(0, args->fp);
        }
        inc_pc( c.integer, arg );
    }
}

/**
 * Writes a code segment as fully native 6502 code.
 * @param fp File handle
 * @param u Unit whose code to write
 */
static void write_as_binary(FILE *fp, xunit *u)
{
    write_binary_args args;
    /* Table of callback functions for our purpose. */
    static xasm_bytecodeproc handlers[] =
    {
        NULL,       /* CMD_END */
        write_bin8, /* CMD_BIN8 */
        write_bin16,    /* CMD_BIN16 */
        NULL,       /* CMD_LABEL */
        write_instr,    /* CMD_INSTR */
        write_dx,   /* CMD_DB */
        write_dx,   /* CMD_DW */
        write_dx,   /* CMD_DD */
        write_dsi8, /* CMD_DSI8 */
        write_dsi16,    /* CMD_DSI16 */
        write_dsb   /* CMD_DSB */
    };
    /* Fill in args */
    args.xu = u;
    args.fp = fp;
    /* Reset PC */
    pc = u->code_origin;
    /* Do the walk */
    bytecode_walk(u->_unit_.codeseg.bytes, handlers, (void *)&args);
}

/*--------------------------------------------------------------------------*/
/* Functions for writing 6502 assembly from bytecodes. */

/**
  Prints \a size bytes of data defined by \a buf to \a out.
*/
static void print_chunk(FILE *out, const char *label,
                        const unsigned char *buf, int size, int cols)
{
    int i, j, m;
    int pos = 0;
    if (label)
        fprintf(out, "%s:\n", label);
    for (i = 0; i < size / cols; ++i) {
        fprintf(out, ".DB ");
        for (j = 0; j < cols-1; ++j)
            fprintf(out, "$%.2X,", buf[pos++]);
        fprintf(out, "$%.2X\n", buf[pos++]);
    }
    m = size % cols;
    if (m > 0) {
        fprintf(out, ".DB ");
        for (j = 0; j < m-1; ++j)
            fprintf(out, "$%.2X,", buf[pos++]);
        fprintf(out, "$%.2X\n", buf[pos++]);
    }
}

/**
 * Writes an array of bytes.
 */
static void asm_write_bin8(const unsigned char *b, void *arg)
{
    int count;
    int i;
    write_binary_args *args = (write_binary_args *)arg;
    i = 1;
    count = get_1(b, &i) + 1;
    //    fprintf(args->fp, "; %d byte(s)\n", count);
    print_chunk(args->fp, /*label=*/0, &b[i], count, /*cols=*/16);
    inc_pc( count, arg );
}

/**
 * Writes an array of bytes.
 */
static void asm_write_bin16(const unsigned char *b, void *arg)
{
    int count;
    int i;
    write_binary_args *args = (write_binary_args *)arg;
    i = 1;
    count = get_2(b, &i) + 1;
    //    fprintf(args->fp, "; %d byte(s)\n", count);
    print_chunk(args->fp, /*label=*/0, &b[i], count, /*cols=*/16);
    inc_pc( count, arg );
}

/**
 * Writes a label.
 */
static void asm_write_label(const unsigned char *b, void *arg)
{
    unsigned char flags;
    int i= 1;
    write_binary_args *args = (write_binary_args *)arg;
    fprintf(args->fp, "; label");
    flags = get_1(b, &i);
    if (flags & XASM_LABEL_FLAG_EXPORT) {
        char *name;
        int len = get_1(b, &i) + 1;
        name = (char *)malloc( len + 1 );
        assert(name != 0);
        memcpy(name, &b[i], len);
        name[len] = '\0';
        i += len;
        fprintf(args->fp, " %s (PC=$%.4X)", name, pc);
        free(name);
    } else {
        fprintf(args->fp, " PC=$%.4X", pc);
    }
    fprintf(args->fp, "\n");
}

/**
 * Writes an instruction.
 */
static void asm_write_instr(const unsigned char *b, void *arg)
{
    xasm_constant c;
    unsigned char op;
    addressing_mode mode;
    int i;
    int exid;
    write_binary_args *args = (write_binary_args *)arg;
    /* Get opcode */
    i = 1;
    op = get_1(b, &i);
    assert(opcode_length(op) > 1);
    mode = opcode_addressing_mode(op);
    assert(mode != INVALID_MODE);
    /* Get expression ID */
    exid = get_2(b, &i);
    /* Evaluate expression */
    eval_expression(args->xu, exid, &c);
    assert(c.type == XASM_INTEGER_CONSTANT);
    /* Write the opcode */
    fprintf(args->fp, "%s", opcode_to_string(op));
    switch (mode) {
        case IMPLIED_MODE:
        case ACCUMULATOR_MODE:
        break;
        case IMMEDIATE_MODE:
        fprintf(args->fp, " #$");
        break;
        case ZEROPAGE_MODE:
        case ZEROPAGE_X_MODE:
        case ZEROPAGE_Y_MODE:
        case ABSOLUTE_MODE:
        case ABSOLUTE_X_MODE:
        case ABSOLUTE_Y_MODE:
        fprintf(args->fp, " $");
        break;
        case PREINDEXED_INDIRECT_MODE:
        case POSTINDEXED_INDIRECT_MODE:
        case INDIRECT_MODE:
        fprintf(args->fp, " [$");
        break;
        case RELATIVE_MODE:
        fprintf(args->fp, " $");
        break;
        case INVALID_MODE:
        break;
    }
    /* Write the operand */
    fprintf(args->fp, "%.4X", (unsigned)c.integer);
    switch (mode) {
        case IMPLIED_MODE:
        case ACCUMULATOR_MODE:
        case IMMEDIATE_MODE:
        case ZEROPAGE_MODE:
	break;
        case ZEROPAGE_X_MODE:
        fprintf(args->fp, ",X");
	break;
        case ZEROPAGE_Y_MODE:
        fprintf(args->fp, ",Y");
	break;
        case ABSOLUTE_MODE:
	break;
        case ABSOLUTE_X_MODE:
        fprintf(args->fp, ",X");
	break;
        case ABSOLUTE_Y_MODE:
        fprintf(args->fp, ",Y");
        break;
        case PREINDEXED_INDIRECT_MODE:
        fprintf(args->fp, ",X]");
	break;
        case POSTINDEXED_INDIRECT_MODE:
        fprintf(args->fp, "],Y");
	break;
        case INDIRECT_MODE:
        fprintf(args->fp, "]");
        break;
        case RELATIVE_MODE:
        break;
        case INVALID_MODE:
        break;
    }
    fprintf(args->fp, "\n");
    inc_pc( opcode_length(op), arg );
}

/**
 * Writes a byte, word or dword.
 */
static void asm_write_dx(const unsigned char *b, void *arg)
{
    xasm_constant c;
    int i;
    int exid;
    write_binary_args *args = (write_binary_args *)arg;
    /* Get expression ID */
    i = 1;
    exid = get_2(b, &i);
    /* Evaluate expression */
    eval_expression(args->xu, exid, &c);
    if (c.type == XASM_INTEGER_CONSTANT) {
        switch (b[0]) {
            case XASM_CMD_DB:
            fprintf(args->fp, ".DB $%.2X", (unsigned)c.integer);
            break;
            case XASM_CMD_DW:
       	    fprintf(args->fp, ".DW $%.4X", (unsigned)c.integer);
            break;
            case XASM_CMD_DD:
       	    fprintf(args->fp, ".DD $%.8X", (unsigned)c.integer);
            break;
        }
        /* Advance PC */
        switch (b[0]) {
            case XASM_CMD_DB:    inc_pc( 1, arg );   break;
            case XASM_CMD_DW:    inc_pc( 2, arg );   break;
            case XASM_CMD_DD:    inc_pc( 4, arg );   break;
        }
    } else if (c.type == XASM_STRING_CONSTANT) {
        int count = strlen(c.string);
        switch (b[0]) {
            case XASM_CMD_DB:
            fprintf(args->fp, ".DB");
            break;
            case XASM_CMD_DW:
            fprintf(args->fp, ".DW");
            break;
            case XASM_CMD_DD:
            fprintf(args->fp, ".DD");
            break;
        }
        fprintf(args->fp, " \"%s\"", c.string);
        /* Advance PC */
        switch (b[0]) {
            case XASM_CMD_DB:    inc_pc( count * 1, arg );   break;
            case XASM_CMD_DW:    inc_pc( count * 2, arg );   break;
            case XASM_CMD_DD:    inc_pc( count * 4, arg );   break;
        }
    } else {
        assert(0);
    }
    fprintf(args->fp, "\n");
    finalize_constant(&c);
}

/**
 * Writes a series of zeroes.
 */
static void asm_write_dsi8(const unsigned char *b, void *arg)
{
    int count;
    int i;
    write_binary_args *args = (write_binary_args *)arg;
    i = 1;
    count = get_1(b, &i) + 1;
    fprintf(args->fp, ".DSB $%X\n", count);
    inc_pc( count, arg );
}

/**
 * Writes a series of zeroes.
 */
static void asm_write_dsi16(const unsigned char *b, void *arg)
{
    int count;
    int i;
    write_binary_args *args = (write_binary_args *)arg;
    i = 1;
    count = get_2(b, &i) + 1;
    fprintf(args->fp, ".DSB $%X\n", count);
    inc_pc( count, arg );
}

/**
 * Writes a series of zeroes.
 */
static void asm_write_dsb(const unsigned char *b, void *arg)
{
    xasm_constant c;
    int i;
    int exid;
    write_binary_args *args = (write_binary_args *)arg;
    /* Get expression ID */
    i = 1;
    exid = get_2(b, &i);
    /* Evaluate expression */
    eval_expression(args->xu, exid, &c);
    assert(c.type == XASM_INTEGER_CONSTANT);
    if (c.integer < 0) {
        err("negative count");
    }
    else if (c.integer > 0) {
        fprintf(args->fp, ".DSB $%X\n", (unsigned)c.integer);
        inc_pc( c.integer, arg );
    }
}

/**
 * Writes a code segment as fully native 6502 code.
 * @param fp File handle
 * @param u Unit whose code to write
 */
static void write_as_assembly(FILE *fp, xunit *u)
{
    write_binary_args args;
    /* Table of callback functions for our purpose. */
    static xasm_bytecodeproc handlers[] =
    {
        NULL,       /* CMD_END */
        asm_write_bin8, /* CMD_BIN8 */
        asm_write_bin16,    /* CMD_BIN16 */
        asm_write_label,    /* CMD_LABEL */
        asm_write_instr,    /* CMD_INSTR */
        asm_write_dx,   /* CMD_DB */
        asm_write_dx,   /* CMD_DW */
        asm_write_dx,   /* CMD_DD */
        asm_write_dsi8, /* CMD_DSI8 */
        asm_write_dsi16,    /* CMD_DSI16 */
        asm_write_dsb   /* CMD_DSB */
    };
    /* Fill in args */
    args.xu = u;
    args.fp = fp;
    /* Reset PC */
    pc = u->code_origin;
    fprintf(fp, "; ***************************************\n");
    fprintf(fp, "; * %s, PC=$%.4X\n", u->_unit_.name, pc);
    fprintf(fp, "; ***************************************\n");
    /* Do the walk */
    bytecode_walk(u->_unit_.codeseg.bytes, handlers, (void *)&args);
}

#define XLNK_NO_DEBUG
#ifndef XLNK_NO_DEBUG

/*--------------------------------------------------------------------------*/
/* Functions for debugging bytecodes. */

/**
 * Gets string representation of bytecode command.
 * @param cmd CMD_*
 * @return String representation ("CMD_*")
 */
static const char *bytecode_to_string(unsigned char cmd)
{
    switch (cmd) {
        case XASM_CMD_FILE:  return "CMD_FILE";
        case XASM_CMD_LINE8: return "CMD_LINE8";
        case XASM_CMD_LINE16:return "CMD_LINE16";
        case XASM_CMD_LINE24:return "CMD_LINE24";
        case XASM_CMD_LINE_INC:  return "CMD_LINE_INC";
        case XASM_CMD_END:   return "CMD_END";
        case XASM_CMD_BIN8:  return "CMD_BIN8";
        case XASM_CMD_BIN16: return "CMD_BIN16";
        case XASM_CMD_LABEL: return "CMD_LABEL";
        case XASM_CMD_INSTR: return "CMD_INSTR";
        case XASM_CMD_DB:    return "CMD_DB";
        case XASM_CMD_DW:    return "CMD_DW";
        case XASM_CMD_DD:    return "CMD_DD";
        case XASM_CMD_DSI8:  return "CMD_DSI8";
        case XASM_CMD_DSI16: return "CMD_DSI16";
        case XASM_CMD_DSB:   return "CMD_DSB";
    }
    return "bytecode_to_string: invalid bytecode";
}

/**
 * Print a bytecode.
 * @param b Bytecodes
 * @param arg Not used
 */
static void print_it(const unsigned char *b, void *arg)
{
    printf("%s\n", bytecode_to_string(b[0]) );
}

/**
 * Prints bytecodes.
 * @param bytes Bytecodes
 */
static void print_bytecodes(const unsigned char *bytes)
{
    static xasm_bytecodeproc handlers[] =
    {
        print_it,print_it,print_it,print_it,print_it,
        print_it,print_it,print_it,print_it,print_it,
        print_it,print_it,print_it
    };
    bytecode_walk(bytes, handlers, NULL);
}

/**
 * Prints a unit.
 * @param u Unit
 */
static void print_unit(xasm_unit *u)
{
    print_bytecodes(u->dataseg.bytes);
    print_bytecodes(u->codeseg.bytes);
}

#endif /* !XLNK_NO_DEBUG */

/*--------------------------------------------------------------------------*/
/* Functions for managing arrays of unit locals. */

/**
 * Creates array of locals.
 * @param size Number of locals
 * @param la Local array
 */
static void create_local_array(int size, local_array *la)
{
    la->size = size;
    if (size > 0) {
        la->entries = (local *)malloc(sizeof(local) * size);
    }
    else {
        la->entries = NULL;
    }
}

/**
 * Finalizes array of locals.
 */
static void finalize_local_array(local_array *la)
{
    int i;
    for (i=0; i<la->size; i++) {
        SAFE_FREE(la->entries[i].name);
    }
    SAFE_FREE(la->entries);
}

/*--------------------------------------------------------------------------*/
/* Functions for counting and registering locals in a unit. */
/* In bytecode expressions, locals are referred to by their index.
   In order to not have to go through the bytecodes every time to
   find a label definition, the following functions build an array
   of structures that can be indexed by the local ID to obtain its
   information.
*/

/**
 * Counts this local.
 */
static void count_one_local(const unsigned char *b, void *arg)
{
    /* Argument points to the counter */
    int *count = (int *)arg;
    (*count)++;
}

/**
 * Counts the number of locals (labels) in an array of bytecodes.
 * @param b Bytecodes, terminated by CMD_END
 * @return Number of locals counted
 */
static int count_locals(const unsigned char *b)
{
    int count;
    /* Table of callback functions for our purpose. */
    static xasm_bytecodeproc handlers[] =
    {
        NULL,   /* CMD_END */
        NULL,   /* CMD_BIN8 */
        NULL,   /* CMD_BIN16 */
        count_one_local,    /* CMD_LABEL */
        NULL,   /* CMD_INSTR */
        NULL,   /* CMD_DB */
        NULL,   /* CMD_DW */
        NULL,   /* CMD_DD */
        NULL,   /* CMD_DSI8 */
        NULL,   /* CMD_DSI16 */
        NULL    /* CMD_DSB */
    };
    count = 0;
    bytecode_walk(b, handlers, (void *)&count);
    return count;
}

/**
 * Variable that points to the unit that locals are being registered for.
 */
static xunit *reg_unit = NULL;

/**
 * Puts this local into array of locals for current unit.
 */
static void register_one_local(const unsigned char *b, void *arg)
{
    int len;
    int i= 1;
    /* Argument points to a pointer which points to the local struct to fill in */
    local **lpptr = (local **)arg;
    local *lptr = *lpptr;
    /* Initialize some fields */
    lptr->resolved = 0;
    lptr->ref_count = 0;
    lptr->name = NULL;
    lptr->align = 1;
    lptr->owner = reg_unit;
    /* Get flag byte */
    lptr->flags = get_1(b, &i);
    /* Test export flag */
    if (lptr->flags & XASM_LABEL_FLAG_EXPORT) {
        /* Get the length of the name */
        len = get_1(b, &i) + 1;
        /* Allocate space for name */
        lptr->name = (char *)malloc( len + 1 );
        if (lptr->name != NULL) {
            /* Copy name from bytecodes */
            memcpy(lptr->name, &b[i], len);
            /* Zero-terminate string */
            lptr->name[len] = '\0';
        }
        i += len;
    }
    if (lptr->flags & XASM_LABEL_FLAG_ALIGN) {
        lptr->align = get_1(b, &i);
    }
    if (lptr->flags & XASM_LABEL_FLAG_ADDR) {
        lptr->phys_addr = get_2(b, &i);
        lptr->resolved = 1;
    }
#if 0
    if (program_args.verbose) {
        verbose(1, "      %s align=%d resolved=%d",
                lptr->name ? lptr->name : "(anonymous)",
                lptr->align, lptr->resolved);
    }
#endif
    /* Point to next local in array */
    *lpptr += 1;
}

/**
 * Puts all locals found in the array of bytecodes into array.
 * @param b Bytecodes, terminated by CMD_END
 * @param la Pointer to array to receive locals
 * @param xu Owner unit
 */
static void register_locals(const unsigned char *b, local_array *la, xunit *xu)
{
    local *lptr;
    local **lpptr;
    /* Table of callback functions for our purpose. */
    static xasm_bytecodeproc handlers[] =
    {
        NULL,   /* CMD_END */
        NULL,   /* CMD_BIN8 */
        NULL,   /* CMD_BIN16 */
        register_one_local, /* CMD_LABEL */
        NULL,   /* CMD_INSTR */
        NULL,   /* CMD_DB */
        NULL,   /* CMD_DW */
        NULL,   /* CMD_DD */
        NULL,   /* CMD_DSI8 */
        NULL,   /* CMD_DSI16 */
        NULL    /* CMD_DSB */
    };
    /* Create array of locals */
    create_local_array(count_locals(b), la);
    /* Prepare args */
    lptr = la->entries;
    lpptr = &lptr;
    reg_unit = xu;
    /* Go! */
    bytecode_walk(b, handlers, (void *)lpptr);
}

/*--------------------------------------------------------------------------*/
/* Functions for entering exported symbols into proper hash table. */

/**
 * Enters an exported symbol into a hash table.
 * @param tab Hash table to enter it into
 * @param key Key
 * @param data Data
 * @param u Owner unit
 */
static void enter_exported_symbol(hashtab *tab, void *key, void *data, xasm_unit *u)
{
    if ((hashtab_get(label_hash, key) != NULL)
        || (hashtab_get(constant_hash, key) != NULL) ) {
        err("duplicate symbol `%s' exported from unit `%s'", (char *)key, u->name);
    }
    else {
        verbose(1, "      %s", (char*)key);
        hashtab_put(tab, key, data);
    }
}

/**
 * Enters all constants in a unit into the proper hash table.
 * @param u Unit whose constants to enter
 */
static void enter_exported_constants(xasm_unit *u)
{
    int i;
    xasm_constant *c;
    for (i=0; i<u->const_count; i++) {
        c = &u->constants[i];
        enter_exported_symbol(constant_hash, (void *)c->name, (void *)c, u);
    }
}

/**
 * Enters locals which should be globally visible into the proper hash table.
 * @param la Array of locals
 * @param u Owner unit
 */
static void enter_exported_locals(local_array *la, xasm_unit *u)
{
    int i;
    local *l;
    for (i=0; i<la->size; i++) {
        l = &la->entries[i];
        /* If it has a name, it is exported */
        if (l->name != NULL) {
            enter_exported_symbol(label_hash, (void *)l->name, (void *)l, u);
        }
    }
}

/*--------------------------------------------------------------------------*/
/* Functions for calculating addresses of data labels in a unit. */

/**
 * Sets the virtual address of this local to current PC value.
 */
static void set_data_address(const unsigned char *b, void *arg)
{
    calc_address_args *args = (calc_address_args *)arg;
    local *l = &args->xu->data_locals.entries[args->index];
    if (!l->resolved) {
        l->virt_addr = pc;
        verbose(2, "    %.4X %s", l->virt_addr, l->name ? l->name : "");
    }
    /* Increase label index */
    args->index++;
}

/**
 * Calculates addresses of labels in a data segment relative to 0.
 * Only a small set of bytecode commands are allowed in a data segment:
 * - label (which we want to assign a virtual address)
 * - storage (constant or variable)
 */
static void calc_data_addresses(xunit *u)
{
    calc_address_args args;
    /* Table of callback functions for our purpose. */
    static xasm_bytecodeproc handlers[] =
    {
        NULL,       /* CMD_END */
        NULL,       /* CMD_BIN8 */
        NULL,       /* CMD_BIN16 */
        set_data_address,   /* CMD_LABEL */
        NULL,       /* CMD_INSTR */
        NULL,       /* CMD_DB */
        NULL,       /* CMD_DW */
        NULL,       /* CMD_DD */
        inc_pc_count8,  /* CMD_DSI8 */
        inc_pc_count16, /* CMD_DSI16 */
        inc_pc_dsb  /* CMD_DSB */
    };
    /* Fill in args */
    args.xu = u;
    args.index = 0;
    /* Reset PC */
    pc = 0;
    verbose(1, "  %s", u->_unit_.name);
    /* Map away! */
    bytecode_walk(u->_unit_.dataseg.bytes, handlers, (void *)&args);
    /* Store the end address, which is the total size of data */
    u->data_size = pc;
}

/*--------------------------------------------------------------------------*/

/* Constructs 32-bit sort key for local. */
#define SORT_KEY(l) (unsigned long)((((l)->flags & XASM_LABEL_FLAG_ZEROPAGE) << 30) | ((l)->align << 24) | (0x10000-(l)->size))

/**
 * Array is sorted from high to low value.
 */
static int label_partition(local **a, int p, int r)
{
    int x;
    int i;
    int j;
    x = SORT_KEY(a[r]);
    i = p - 1;
    local *temp;
    for (j=p; j<r; j++) {
        if (SORT_KEY(a[j]) >= x) {
            i = i + 1;
            temp = a[i];
            a[i] = a[j];
            a[j] = temp;
        }
    }
    temp = a[i+1];
    a[i+1] = a[r];
    a[r] = temp;
    return i + 1;
}

/**
 * Quicksort implementation used to sort array of pointers to locals.
 */
static void label_qsort(local **a, int p, int r)
{
    int q;
    if (p < r) {
        q = label_partition(a, p, r);
        label_qsort(a, p, q-1);
        label_qsort(a, q+1, r);
    }
}

/**
 * Maps all data labels to 6502 RAM locations.
 * This is a very important function. It takes all the data labels from all
 * the loaded units and attempts to assign them unique physical addresses.
 * The list of target RAM blocks given in the linker script is the premise.
 */
static void map_data_to_ram()
{
    int i, k;
    local **total_order;
    local *l;
    int count;
    /* Use a bit array to keep track of allocations,
       to ensure that there is no overlap */
    unsigned char *allocated;
    int ram_base, ram_end;
    if (ram_block_head == NULL)
        return; /* Nothing to do. */
    {
        avail_ram_block *b;
        ram_base = 10000000;
        ram_end = -10000000;
        for (b = ram_block_head; b != NULL; b = b->next) {
            if (b->start < ram_base)
                ram_base = b->start;
            if (b->end > ram_end)
                ram_end = b->end;
        }
    }
    allocated = (unsigned char *)malloc(((ram_end - ram_base) + 7) / 8);
    memset(allocated, 0, ((ram_end - ram_base) + 7) / 8);
    /* Calculate total number of labels to map */
    count = 0;
    for (i=0; i<unit_count; i++) {
        count += units[i].data_locals.size;
    }
    /* Put pointers to all data labels in one big array */
    total_order = (local **)malloc( count * sizeof(local *) );
    for (i=0, k=0; i<unit_count; i++) {
        int j;
        local_array *la;
        la = &units[i].data_locals;
        for (j=0; j<la->size; j++) {
            int size;
            /* Use virtual addresses to calculate size from this label to next */
            if (j == la->size-1) {
                size = units[i].data_size;
            }
            else {
                size = la->entries[j+1].virt_addr;
            }
            la->entries[j].size = size - la->entries[j].virt_addr;
            /* Put pointer in array */
            total_order[k++] = &la->entries[j];
        }
    }
    /* Sort them */
    label_qsort(total_order, 0, count-1);
    /* Map them */
    for (i=0; i<count; i++) {
        l = total_order[i];
        /* Try to allocate it */
        if (alloc_ram(l) == 1) {
            /* Good, label mapped successfully */
            l->resolved = 1;
            verbose(1, "  %.4X-%.4X %s (%s)", l->phys_addr,
                    l->phys_addr + l->size-1, l->name ? l->name : "",
                    l->owner->_unit_.name);
            {
                /* Verify that there's no overlap with other variable */
                int a;
                for (a = l->phys_addr; a < l->phys_addr + l->size; ++a) {
                    assert((allocated[(a - ram_base) / 8] & (1 << (a & 7))) == 0);
                    allocated[(a - ram_base) / 8] |= 1 << (a & 7);
                }
            }
        }
        else {
            err("out of 6502 RAM while allocating unit `%s'", l->owner->_unit_.name);
            return;
        }
    }
    free(total_order);
    free(allocated);
}

/*--------------------------------------------------------------------------*/
/* Functions for calculating offsets of code labels in a unit. */

/**
 * Sets the address of this code label to current PC.
 */
static void set_code_address(const unsigned char *b, void *arg)
{
    calc_address_args *args = (calc_address_args *)arg;
    local *l = &args->xu->code_locals.entries[args->index];
    if (!l->resolved) {
        l->phys_addr = pc;
        l->resolved = 1;
        if (program_args.verbose) {
            fprintf(stdout, "  %.4X %s (%s)\n", l->phys_addr,
                    l->name ? l->name : "", l->owner->_unit_.name);
        }
    }
    /* Increase label index */
    args->index++;
}

/**
 * Calculates addresses of code labels in a segment.
 * NOTE: Only the virtual addresses (relative to 0) are calculated.
 * The labels then need to be relocated to obtain the physical address (see below).
 * @param u Unit
 */
static void calc_code_addresses(xunit *u)
{
    calc_address_args args;
    /* Table of callback functions for our purpose. */
    static xasm_bytecodeproc handlers[] =
    {
        NULL,       /* CMD_END */
        inc_pc_count8,  /* CMD_BIN8 */
        inc_pc_count16, /* CMD_BIN16 */
        set_code_address,   /* CMD_LABEL */
        inc_pc_instr,   /* CMD_INSTR */
        inc_pc_1,   /* CMD_DB -- TODO, error if string */
        inc_pc_2,   /* CMD_DW */
        inc_pc_4,   /* CMD_DD */
        inc_pc_count8,  /* CMD_DSI8 */
        inc_pc_count16, /* CMD_DSI16 */
        inc_pc_dsb  /* CMD_DSB */
    };
    /* Fill in args */
    args.xu = u;
    args.index = 0;
    /* Do the walk */
    bytecode_walk(u->_unit_.codeseg.bytes, handlers, (void *)&args);
    /* Store the total size of code */
    u->code_size = pc - u->code_origin;
}

/*--------------------------------------------------------------------------*/

/**
 * Issues a script error.
 */
static void scripterr(xlnk_script *s, xlnk_script_command *c, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    if (!suppress) {
        fprintf(stderr, "error: %s:%d: `%s': ", s->name, c->line, xlnk_script_command_type_to_string(c->type) );
        vfprintf(stderr, fmt, ap);
        fprintf(stderr, "\n");
        err_count++;
    }
    va_end(ap);
}

#define require_arg(s, c, a, d) { \
    d = xlnk_script_get_command_arg(c, a); \
    if (d == NULL) { \
        scripterr(s, c, "missing argument `%s'", a); \
        return; \
    } \
}

#define require_arg_in_range(s, c, a, v, l, h) { \
    if (((v) < (l)) || ((v) > (h))) { \
        scripterr(s, c, "value of argument `%s' is out of range", a); \
        return; \
    } \
}

/*--------------------------------------------------------------------------*/
/* Functions for registering RAM blocks in script. */

/**
 * Registers one RAM block based on 'ram' script command.
 * @param s Linker script
 * @param c Command of type RAM_COMMAND
 * @param arg Not used
 */
static void register_one_ram_block(xlnk_script *s, xlnk_script_command *c, void *arg)
{
    int start;
    int end;
    const char *start_str;
    const char *end_str;
    require_arg(s, c, "start", start_str);
    require_arg(s, c, "end", end_str);
    start = str_to_int(start_str);
    end = str_to_int(end_str);
    require_arg_in_range(s, c, "start", start, 0x0000, 0xFFFF);
    require_arg_in_range(s, c, "end", end, 0x0000, 0xFFFF);
    if (end <= start) {
        scripterr(s, c, "`end' is smaller than `start'");
    }
    add_ram_block(start, end);
}

/**
 * Registers RAM blocks based on 'ram' commands in a script.
 * @param sc Linker script
 */
static void register_ram_blocks(xlnk_script *sc)
{
    /* Table of mappings for our purpose */
    static xlnk_script_commandprocmap map[] = {
        { XLNK_RAM_COMMAND, register_one_ram_block },
        { XLNK_BAD_COMMAND, NULL }
    };
    /* Do the walk */
    xlnk_script_walk(sc, map, NULL);
    /* Calculate total RAM size */
    total_ram = ram_left();
}

/*--------------------------------------------------------------------------*/
/* Functions for loading and initial processing of units in script. */

/**
 * Registers (parses etc.) one unit based on 'link' script command.
 * @param s Linker script
 * @param c Command of type LINK_COMMAND
 * @param arg Pointer to unit index
 */
static void register_one_unit(xlnk_script *s, xlnk_script_command *c, void *arg)
{
    const char *file;
    int *i;
    xunit *xu;
    require_arg(s, c, "file", file);
    /* arg is pointer to unit index */
    i = (int *)arg;
    /* Get pointer to xunit to fill in */
    xu = &units[*i];
    /* Read basic unit from file */
    if (xasm_unit_read(file, &xu->_unit_) == 0) {
        scripterr(s, c, "failed to load unit `%s'", file);
        xu->loaded = 0;
        return;
    }
    xu->loaded = 1;
    verbose(1, "  unit `%s' loaded", file);

    verbose(1, "    registering local symbols...");
    register_locals(xu->_unit_.dataseg.bytes, &xu->data_locals, xu);
    register_locals(xu->_unit_.codeseg.bytes, &xu->code_locals, xu);

    verbose(1, "    registering public symbols...");
    enter_exported_constants(&xu->_unit_);
    enter_exported_locals(&xu->data_locals, &xu->_unit_);
    enter_exported_locals(&xu->code_locals, &xu->_unit_);

    hashtab_put(unit_hash, (void*)file, xu);
    /* Increment unit index */
    (*i)++;
}

/**
 * Registers units based on 'link' commands in script.
 * @param sc Linker script
 */
static void register_units(xlnk_script *sc)
{
    /* Table of mappings for our purpose */
    static xlnk_script_commandprocmap map[] = {
        { XLNK_LINK_COMMAND, register_one_unit },
        { XLNK_BAD_COMMAND, NULL }
    };
    int i = 0;
    /* Do the walk */
    xlnk_script_walk(sc, map, (void *)&i);
}

/*--------------------------------------------------------------------------*/
/* Functions for composing a binary file based on a sequential list of
script commands. */

/**
 * Sets the output file according to 'output' script command.
 * @param s Linker script
 * @param c Command of type OUTPUT_COMMAND
 * @param arg Pointer to file handle
 */
static void set_output(xlnk_script *s, xlnk_script_command *c, void *arg)
{
    const char *file;
    FILE **fpp;
    require_arg(s, c, "file", file);
    /* Arg is pointer to file handle pointer */
    fpp = (FILE **)arg;
    if (*fpp != NULL) {
        fclose(*fpp);
    }
    *fpp = fopen(file, "wb");
    if (*fpp == NULL) {
        scripterr(s, c, "could not open `%s' for writing", file);
    }
    else {
        verbose(1, "  output goes to `%s'", file);
    }
}

/**
 * Copies a file to output according to 'copy' script command.
 * @param s Linker script
 * @param c Command of type COPY_COMMAND
 * @param arg Pointer to file handle
 */
static void copy_to_output(xlnk_script *s, xlnk_script_command *c, void *arg)
{
    const char *file;
    FILE **fpp;
    FILE *cf;
    unsigned char k;
    /* Arg is pointer to file handle pointer */
    fpp = (FILE **)arg;
    if (*fpp == NULL) {
        scripterr(s, c, "no output open");
    }
    else {
        require_arg(s, c, "file", file);
        cf = fopen(file, "rb");
        if (cf == NULL) {
            scripterr(s, c, "could not open `%s' for reading", file);
        }
        else {
            verbose(1, "  copying `%s' to output at position %ld...", file, ftell(*fpp) );
            for (k = fgetc(cf); !feof(cf); k = fgetc(cf) ) {
                fputc(k, *fpp);
            }
            bank_offset += ftell(cf);
            pc += ftell(cf);
            fclose(cf);
            if (bank_offset > bank_size) {
                scripterr(s, c, "bank size (%d) exceeded by %d bytes", bank_size, bank_offset - bank_size);
            }
        }
    }
}

/**
 * Starts a new bank according to 'bank' script command.
 * @param s Linker script
 * @param c Command of type BANK_COMMAND
 * @param arg Pointer to file handle
 */
static void start_bank(xlnk_script *s, xlnk_script_command *c, void *arg)
{
    const char *size_str;
    const char *origin_str;
    size_str = xlnk_script_get_command_arg(c, "size");
    if (size_str != NULL) {
        bank_size = str_to_int(size_str);
        if (bank_size <= 0) {
            scripterr(s, c, "invalid size");
        }
    }
    else {
        /* Use bank size of previous bank if there was one */
        if (bank_size == 0x7FFFFFFF) {
            scripterr(s, c, "no bank size set");
        }
    }
    origin_str = xlnk_script_get_command_arg(c, "origin");
    if (origin_str != NULL) {
        bank_origin = str_to_int(origin_str);
        require_arg_in_range(s, c, "origin", bank_origin, 0x0000, 0xFFFF);
    }
    else {
        /* Use old bank origin */
    }
    bank_id++;
    bank_offset = 0;
    pc = bank_origin;
}

/**
 * Writes unit according to 'link' script command.
 * @param s Linker script
 * @param c Command of type LINK_COMMAND
 * @param arg Pointer to file handle
 */
static void write_unit(xlnk_script *s, xlnk_script_command *c, void *arg)
{
    FILE **fpp;
    xunit *xu;
    const char *file;
    /* Arg is pointer to file handle pointer */
    fpp = (FILE **)arg;
    if (*fpp == NULL) {
        scripterr(s, c, "no output open");
    }
    else {
        require_arg(s, c, "file", file);
        xu = (xunit *)hashtab_get(unit_hash, (void*)file);
        verbose(1, "  appending unit `%s' to output at position %ld...", file, ftell(*fpp));
        write_as_binary(*fpp, xu);
        bank_offset += xu->code_size;
        if (bank_offset > bank_size) {
            scripterr(s, c, "bank size (%d) exceeded by %d bytes", bank_size, bank_offset - bank_size);
        }
    }
}

/**
 * Pads output file according to 'pad' script command.
 * @param s Linker script
 * @param c Command of type PAD_COMMAND
 * @param arg Pointer to file handle
 */
static void write_pad(xlnk_script *s, xlnk_script_command *c, void *arg)
{
    FILE **fpp;
    int i;
    int count;
    int offset;
    int origin;
    const char *offset_str;
    const char *origin_str;
    const char *size_str;
    /* Arg is pointer to file handle pointer */
    fpp = (FILE **)arg;
    if (*fpp == NULL) {
        scripterr(s, c, "no output open");
    }
    else {
        if ((offset_str = xlnk_script_get_command_arg(c, "offset")) != NULL) {
            offset = str_to_int(offset_str);
            count = offset - bank_offset;
        }
        else if ((origin_str = xlnk_script_get_command_arg(c, "origin")) != NULL) {
            origin = str_to_int(origin_str);
            count = origin - pc;
        }
        else if ((size_str = xlnk_script_get_command_arg(c, "size")) != NULL) {
            count = str_to_int(size_str);
        }
        else {
            scripterr(s, c, "missing argument");
            count = 0;
        }
        if (count < 0) {
            scripterr(s, c, "cannot pad backwards");
            count = 0;
        }
        else if (count > 0) {
            verbose(1, "  padding %d bytes...", count);
        }
        for (i=0; i<count; i++) {
            fputc(0, *fpp);
        }
        bank_offset += count;
        pc += count;
        if (bank_offset > bank_size) {
            scripterr(s, c, "bank size (%d) exceeded by %d bytes", bank_size, bank_offset - bank_size);
        }
    }
}

/**
 * Pads to end of bank in file if bank size not reached.
 * @param s Linker script
 * @param c Command of type BANK_COMMAND
 * @param fp File handle
 */
static void maybe_pad_bank(xlnk_script *s, xlnk_script_command *c, FILE *fp)
{
    int i;
    if ( (bank_size != 0x7FFFFFFF) && (bank_offset < bank_size) ) {
        if (fp == NULL) {
            scripterr(s, c, "no output open");
        }
        else {
            for (i=bank_offset; i<bank_size; i++) {
                fputc(0, fp);
            }
        }
    }
}

/**
 * Finishes old bank in output and starts new bank.
 * @param s Linker script
 * @param c Command of type BANK_COMMAND
 * @param arg Pointer to file handle
 */
static void write_bank(xlnk_script *s, xlnk_script_command *c, void *arg)
{
    FILE **fpp;
    /* Arg is pointer to file handle pointer */
    fpp = (FILE **)arg;
    maybe_pad_bank(s, c, *fpp);
    start_bank(s, c, arg);
}

/**
 * Generates the final binary output from the linker.
 * @param sc Linker script
 */
static void generate_binary_output(xlnk_script *sc)
{
    FILE *fp = NULL;
    /* Table of mappings for our purpose */
    static xlnk_script_commandprocmap map[] = {
        { XLNK_OUTPUT_COMMAND, set_output },
        { XLNK_COPY_COMMAND, copy_to_output },
        { XLNK_BANK_COMMAND, write_bank },
        { XLNK_LINK_COMMAND, write_unit },
        { XLNK_PAD_COMMAND, write_pad },
        { XLNK_BAD_COMMAND, NULL }
    };
    /* Reset offsets */
    bank_size = 0x7FFFFFFF;
    bank_offset = 0;
    bank_origin = 0;
    bank_id = -1;
    pc = 0;
    /* Do the walk */
    xlnk_script_walk(sc, map, (void *)&fp);
    /* Pad last bank if necessary */
    maybe_pad_bank(sc, sc->first_command, fp);
}

/*--------------------------------------------------------------------------*/
/* Functions for producing assembly code based on a sequential list of
script commands. */

/**
 * Sets the output file according to 'output' script command.
 * @param s Linker script
 * @param c Command of type OUTPUT_COMMAND
 * @param arg Pointer to file handle
 */
static void asm_set_output(xlnk_script *s, xlnk_script_command *c, void *arg)
{
    /* No-op when generating assembly. */
}

/**
 * Copies a file to output according to 'copy' script command.
 * @param s Linker script
 * @param c Command of type COPY_COMMAND
 * @param arg Pointer to file handle
 */
static void asm_copy_to_output(xlnk_script *s, xlnk_script_command *c, void *arg)
{
    const char *file;
    FILE **fpp;
    FILE *cf;
    /* Arg is pointer to file handle pointer */
    fpp = (FILE **)arg;
    require_arg(s, c, "file", file);
    cf = fopen(file, "rb");
    if (cf == NULL) {
        scripterr(s, c, "could not open `%s' for reading", file);
    } else {
        unsigned char buf[1024];
        int count = fread(buf, 1, 1024, cf);
        fprintf(*fpp, "; begin %s\n", file);
        while (count > 0) {
            print_chunk(*fpp, /*label=*/0, buf, count, /*cols=*/16);
            count = fread(buf, 1, 1024, cf);
        }
        fprintf(*fpp, "; end %s\n", file);
        bank_offset += ftell(cf);
        pc += ftell(cf);
        fclose(cf);
        if (bank_offset > bank_size) {
            scripterr(s, c, "bank size (%d) exceeded by %d bytes", bank_size, bank_offset - bank_size);
        }
    }
}

/**
 * Starts a new bank according to 'bank' script command.
 * @param s Linker script
 * @param c Command of type BANK_COMMAND
 * @param arg Pointer to file handle
 */
static void asm_start_bank(xlnk_script *s, xlnk_script_command *c, void *arg)
{
    FILE *fp = *(FILE**)arg;
    start_bank(s, c, arg);
    fprintf(fp, ".ORG $%.4X\n", pc);
}

/**
 * Writes unit according to 'link' script command.
 * @param s Linker script
 * @param c Command of type LINK_COMMAND
 * @param arg Pointer to file handle
 */
static void asm_write_unit(xlnk_script *s, xlnk_script_command *c, void *arg)
{
    FILE **fpp;
    xunit *xu;
    const char *file;
    /* Arg is pointer to file handle pointer */
    fpp = (FILE **)arg;
    require_arg(s, c, "file", file);
    xu = (xunit *)hashtab_get(unit_hash, (void*)file);
    verbose(1, "  appending unit `%s' to output at position %ld...", file, ftell(*fpp));
    write_as_assembly(*fpp, xu);
    bank_offset += xu->code_size;
    if (bank_offset > bank_size) {
        scripterr(s, c, "bank size (%d) exceeded by %d bytes", bank_size, bank_offset - bank_size);
    }
}

/**
 * Pads output file according to 'pad' script command.
 * @param s Linker script
 * @param c Command of type PAD_COMMAND
 * @param arg Pointer to file handle
 */
static void asm_write_pad(xlnk_script *s, xlnk_script_command *c, void *arg)
{
    FILE **fpp;
    int count;
    int offset;
    int origin;
    const char *offset_str;
    const char *origin_str;
    const char *size_str;
    /* Arg is pointer to file handle pointer */
    fpp = (FILE **)arg;
    if ((offset_str = xlnk_script_get_command_arg(c, "offset")) != NULL) {
        offset = str_to_int(offset_str);
        count = offset - bank_offset;
    } else if ((origin_str = xlnk_script_get_command_arg(c, "origin")) != NULL) {
        origin = str_to_int(origin_str);
        count = origin - pc;
    } else if ((size_str = xlnk_script_get_command_arg(c, "size")) != NULL) {
        count = str_to_int(size_str);
    } else {
        scripterr(s, c, "missing argument");
        count = 0;
    }
    if (count < 0) {
        scripterr(s, c, "cannot pad backwards");
        count = 0;
    } else if (count > 0) {
        verbose(1, "  padding %d bytes...", count);
    }
    fprintf(*fpp, ".DSB $%X\n", count);
    bank_offset += count;
    pc += count;
    if (bank_offset > bank_size) {
        scripterr(s, c, "bank size (%d) exceeded by %d bytes", bank_size, bank_offset - bank_size);
    }
}

/**
 * Pads to end of bank in file if bank size not reached.
 * @param s Linker script
 * @param c Command of type BANK_COMMAND
 * @param fp File handle
 */
static void asm_maybe_pad_bank(xlnk_script *s, xlnk_script_command *c, FILE *fp)
{
    if ( (bank_size != 0x7FFFFFFF) && (bank_offset < bank_size) ) {
        fprintf(fp, ".DSB $%X\n", bank_size - bank_offset);
    }
}

/**
 * Finishes old bank in output and starts new bank.
 * @param s Linker script
 * @param c Command of type BANK_COMMAND
 * @param arg Pointer to file handle
 */
static void asm_write_bank(xlnk_script *s, xlnk_script_command *c, void *arg)
{
    FILE **fpp = (FILE **)arg;
    asm_maybe_pad_bank(s, c, *fpp);
    asm_start_bank(s, c, arg);
}

static void generate_assembly_output(xlnk_script *sc, FILE *fp)
{
    /* Table of mappings for our purpose */
    static xlnk_script_commandprocmap map[] = {
        { XLNK_OUTPUT_COMMAND, asm_set_output },
        { XLNK_COPY_COMMAND, asm_copy_to_output },
        { XLNK_BANK_COMMAND, asm_write_bank },
        { XLNK_LINK_COMMAND, asm_write_unit },
        { XLNK_PAD_COMMAND, asm_write_pad },
        { XLNK_BAD_COMMAND, NULL }
    };
    /* Reset offsets */
    bank_size = 0x7FFFFFFF;
    bank_offset = 0;
    bank_origin = 0;
    bank_id = -1;
    pc = 0;
    fprintf(fp, ".CODESEG\n");
    /* Do the walk */
    xlnk_script_walk(sc, map, (void *)&fp);
    /* Pad last bank if necessary */
    asm_maybe_pad_bank(sc, sc->first_command, fp);
    fprintf(fp, ".END\n");
}

/*--------------------------------------------------------------------------*/

/**
 * Increases bank offset and PC according to size of the file specified by
 * 'copy' script command.
 * @param s Linker script
 * @param c Command of type COPY_COMMAND
 * @param arg Not used
 */
static void inc_offset_copy(xlnk_script *s, xlnk_script_command *c, void *arg)
{
    const char *file;
    FILE *fp;
    require_arg(s, c, "file", file);
    fp = fopen(file, "rb");
    if (fp == NULL) {
        scripterr(s, c, "could not open `%s' for reading", file);
    }
    else {
        fseek(fp, 0, SEEK_END);
        bank_offset += ftell(fp);
        pc += ftell(fp);
        fclose(fp);
        if (bank_offset > bank_size) {
            scripterr(s, c, "bank size (%d) exceeded by %d bytes", bank_size, bank_offset - bank_size);
        }
    }
}

/**
 * Sets the origin of a unit and relocates its code to this location.
 * @param s Linker script
 * @param c Command of type LINK_COMMAND
 * @param arg Not used
 */
static void set_unit_origin(xlnk_script *s, xlnk_script_command *c, void *arg)
{
    xunit *xu;
    const char *file;
    const char *origin_str;
    int origin;
    require_arg(s, c, "file", file);
    xu = (xunit *)hashtab_get(unit_hash, (void*)file);
    origin_str = xlnk_script_get_command_arg(c, "origin");
    if (origin_str != NULL) {
        origin = str_to_int(origin_str);
        require_arg_in_range(s, c, "origin", origin, 0x0000, 0xFFFF);
        xu->code_origin = origin;
        pc = origin;
    }
    else {
        /* No origin specified. Set to PC. */
        xu->code_origin = pc;
    }
    xu->bank_id = bank_id;
    /* Now we can calculate the physical code addresses of the unit. */
    calc_code_addresses(xu);
    verbose(1, "  unit `%s' relocated to %.4X", xu->_unit_.name, xu->code_origin);
    bank_offset += xu->code_size;
}

/**
 * Increases bank offset and PC according to 'pad' script command.
 * @param s Linker script
 * @param c Command of type PAD_COMMAND
 * @param arg Not used
 */
static void inc_offset_pad(xlnk_script *s, xlnk_script_command *c, void *arg)
{
    int count;
    int offset;
    int origin;
    const char *offset_str;
    const char *origin_str;
    const char *size_str;
    if ((offset_str = xlnk_script_get_command_arg(c, "offset")) != NULL) {
        offset = str_to_int(offset_str);
        count = offset - bank_offset;
    }
    else if ((origin_str = xlnk_script_get_command_arg(c, "origin")) != NULL) {
        origin = str_to_int(origin_str);
        count = origin - pc;
    }
    else if ((size_str = xlnk_script_get_command_arg(c, "size")) != NULL) {
        count = str_to_int(size_str);
    }
    else {
        scripterr(s, c, "missing argument");
        count = 0;
    }
    if (count < 0) {
        scripterr(s, c, "cannot pad %d bytes backwards", -count);
        count = 0;
    }
    bank_offset += count;
    pc += count;
    if (bank_offset > bank_size) {
        scripterr(s, c, "bank size (%d) exceeded by %d bytes", bank_size, bank_offset - bank_size);
    }
}

/**
 * Relocates code of all units according to script commands and/or their position
 * in the final binary.
 * @param sc Linker script
 */
static void relocate_units(xlnk_script *sc)
{
    /* Table of mappings for our purpose */
    static xlnk_script_commandprocmap map[] = {
        { XLNK_COPY_COMMAND, inc_offset_copy },
        { XLNK_BANK_COMMAND, start_bank },
        { XLNK_LINK_COMMAND, set_unit_origin },
        { XLNK_PAD_COMMAND, inc_offset_pad },
        { XLNK_BAD_COMMAND, NULL }
    };
    /* Reset offsets */
    bank_size = 0x7FFFFFFF;
    bank_offset = 0;
    bank_origin = 0;
    bank_id = -1;
    pc = 0;
    /* Do the walk */
    xlnk_script_walk(sc, map, NULL);
}

/**
 *
 */
static void maybe_print_ram_statistics()
{
    int used;
    int left;
    if (total_ram > 0) {
        left = ram_left();
        used = total_ram - left;
        verbose(1, "  total RAM: %d bytes", total_ram);
        verbose(1, "  RAM used:  %d bytes (%d%%)", used, (int)(((float)used / (float)total_ram)*100.0f) );
        verbose(1, "  RAM left:  %d bytes (%d%%)", left, (int)(((float)left / (float)total_ram)*100.0f) );
    }
}

/*--------------------------------------------------------------------------*/

/**
 * Program entrypoint.
 */
int main(int argc, char **argv)
{
    int i;
    xlnk_script sc;

    parse_arguments(argc, argv);

    suppress = 0;
    err_count = 0;
    warn_count = 0;

    verbose(1, "parsing linker script...");
    if (xlnk_script_parse(program_args.input_file, &sc) == 0) {
        /* Something bad happened when parsing script, halt */
        return(1);
    }

    verbose(1, "registering RAM blocks...");
    register_ram_blocks(&sc);

    constant_hash = hashtab_create(23, HASHTAB_STRKEYHSH, HASHTAB_STRKEYCMP);
    label_hash = hashtab_create(23, HASHTAB_STRKEYHSH, HASHTAB_STRKEYCMP);
    unit_hash = hashtab_create(11, HASHTAB_STRKEYHSH, HASHTAB_STRKEYCMP);

    unit_count = xlnk_script_count_command_type(&sc, XLNK_LINK_COMMAND);
    if (unit_count > 0) {
        units = (xunit *)malloc( sizeof(xunit) * unit_count );
        memset(units, 0, sizeof(xunit) * unit_count);
    }
    else {
        units = NULL;
    }
    verbose(1, "loading units...");
    register_units(&sc);

    /* Only continue with processing if no unresolved symbols */
    if (err_count == 0) {
        verbose(1, "calculating data addresses...");
        for (i=0; i<unit_count; i++) {
            calc_data_addresses(&units[i]);
        }

        /* TODO: Count references: go through all instructions, find EXTRN and LOCAL operands in expressions */
        /* TODO: Find modes of access for each DATA label (i.e. label MUST be allocated in zero page) */

        verbose(1, "mapping data to RAM...");
        map_data_to_ram();
        maybe_print_ram_statistics();

        if (err_count == 0) {
            verbose(1, "relocating code...");
            suppress = 1;
            relocate_units(&sc);
            suppress = 0;
            relocate_units(&sc);

            if (err_count == 0) {
                verbose(1, "generating output...");
                generate_binary_output(&sc);
                if (generate_assembly)
                    generate_assembly_output(&sc, stdout);
            }
        }
    }

    verbose(1, "cleaning up...");

    for (i=0; i<unit_count; i++) {
        if (units[i].loaded) {
            finalize_local_array( &units[i].data_locals );
            finalize_local_array( &units[i].code_locals );
            xasm_unit_finalize( &units[i]._unit_ );
        }
    }
    hashtab_finalize(label_hash);
    hashtab_finalize(constant_hash);
    hashtab_finalize(unit_hash);
    finalize_ram_blocks();
    xlnk_script_finalize(&sc);

    return (err_count == 0) ? 0 : 1;
}
