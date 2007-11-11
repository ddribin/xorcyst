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

#ifndef SCRIPT_H
#define SCRIPT_H

/**
 * The possible kinds of command.
 */
enum tag_command_type {
    RAM_COMMAND=0,
    OUTPUT_COMMAND,
    COPY_COMMAND,
    BANK_COMMAND,
    LINK_COMMAND,
    OPTIONS_COMMAND,
    PAD_COMMAND,
    BAD_COMMAND
};

typedef enum tag_command_type command_type;

/**
 * A list of command arguments
 */
struct tag_command_arg
{
    char *key;
    char *value;
    struct tag_command_arg *next;
};

typedef struct tag_command_arg command_arg;

/**
 * A command
 */
struct tag_script_command {
    command_type type;
    command_arg *first_arg;
    int line;
    struct tag_script_command *next;
};

typedef struct tag_script_command script_command;

/**
 * A script
 */
struct tag_script {
    char *name;
    script_command *first_command;
};

typedef struct tag_script script;

/** Signature for procedure to process a script command */
typedef void (*script_commandproc)(script *, script_command *, void *);

/**
 * Structure that represents a mapping from script command type to processor function.
 */
struct tag_script_commandprocmap {
    command_type type;
    script_commandproc proc;
};

typedef struct tag_script_commandprocmap script_commandprocmap;

/* Function prototypes */

int script_parse(char *, script *);
void script_finalize(script *);
int script_length(script *);
script_command *script_get_command(script *, int);
void script_walk(script *, script_commandprocmap *, void *);
char *script_get_command_arg(script_command *, char *);
char *script_command_type_to_string(command_type);
int script_count_command_type(script *, command_type);

#endif  /* !SCRIPT_H */
