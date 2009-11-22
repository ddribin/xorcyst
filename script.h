/*
 * $Id: script.h,v 1.2 2007/07/22 13:35:20 khansen Exp $
 * $Log: script.h,v $
 * Revision 1.2  2007/07/22 13:35:20  khansen
 * convert tabs to whitespaces
 *
 * Revision 1.1  2004/06/30 07:56:42  kenth
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

#ifndef XLNK_SCRIPT_H
#define XLNK_SCRIPT_H

/**
 * The possible kinds of command.
 */
enum tag_xlnk_command_type {
    XLNK_RAM_COMMAND=0,
    XLNK_OUTPUT_COMMAND,
    XLNK_COPY_COMMAND,
    XLNK_BANK_COMMAND,
    XLNK_LINK_COMMAND,
    XLNK_OPTIONS_COMMAND,
    XLNK_PAD_COMMAND,
    XLNK_BAD_COMMAND
};

typedef enum tag_xlnk_command_type xlnk_command_type;

/**
 * A list of command arguments
 */
struct tag_xlnk_command_arg
{
    char *key;
    char *value;
    struct tag_xlnk_command_arg *next;
};

typedef struct tag_xlnk_command_arg xlnk_command_arg;

/**
 * A command
 */
struct tag_xlnk_script_command {
    xlnk_command_type type;
    xlnk_command_arg *first_arg;
    int line;
    struct tag_xlnk_script_command *next;
};

typedef struct tag_xlnk_script_command xlnk_script_command;

/**
 * A script
 */
struct tag_xlnk_script {
    const char *name;
    xlnk_script_command *first_command;
};

typedef struct tag_xlnk_script xlnk_script;

/** Signature for procedure to process a script command */
typedef void (*xlnk_script_commandproc)(xlnk_script *, xlnk_script_command *, void *);

/**
 * Structure that represents a mapping from script command type to processor function.
 */
struct tag_xlnk_script_commandprocmap {
    xlnk_command_type type;
    xlnk_script_commandproc proc;
};

typedef struct tag_xlnk_script_commandprocmap xlnk_script_commandprocmap;

/* Function prototypes */

int xlnk_script_parse(const char *, xlnk_script *);
void xlnk_script_finalize(xlnk_script *);
int xlnk_script_length(xlnk_script *);
xlnk_script_command *xlnk_script_get_command(xlnk_script *, int);
void xlnk_script_walk(xlnk_script *, xlnk_script_commandprocmap *, void *);
const char *xlnk_script_get_command_arg(xlnk_script_command *, const char *);
const char *xlnk_script_command_type_to_string(xlnk_command_type);
int xlnk_script_count_command_type(xlnk_script *, xlnk_command_type);

#endif  /* !XLNK_SCRIPT_H */
