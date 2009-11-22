/*
 * $Id: script.c,v 1.3 2007/07/22 13:33:26 khansen Exp $
 * $Log: script.c,v $
 * Revision 1.3  2007/07/22 13:33:26  khansen
 * convert tabs to whitespaces
 *
 * Revision 1.2  2004/12/18 16:59:50  kenth
 * fixed command parser bug
 *
 * Revision 1.1  2004/06/30 07:55:53  kenth
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
 * This file contains functions to parse, manage, process and query XORcyst
 * linker scripts and their commands.
 * Such a script is just a text file with a series of commands of the following
 * form:
 * command_name{arg_name=value, arg_name=value, ...}
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "script.h"

#define SAFE_FREE(a) if (a) { free(a); a = NULL; }

/*---------------------------------------------------------------------------*/

/** Describes a mapping from string to command type */
struct tag_string_to_command_type_mapping {
    const char *string;
    xlnk_command_type type;
};

typedef struct tag_string_to_command_type_mapping string_to_command_type_mapping;

/**
 * Attempts to map a string to a script command type.
 * @param s String representation of command
 * @return The corresponding command
 */
static xlnk_command_type string_to_command_type(const char *s) {
    int i;
    /* Table of mappings */
    static string_to_command_type_mapping map[] = {
        { "ram", XLNK_RAM_COMMAND },
        { "output", XLNK_OUTPUT_COMMAND },
        { "copy", XLNK_COPY_COMMAND },
        { "bank", XLNK_BANK_COMMAND },
        { "link", XLNK_LINK_COMMAND },
        { "options", XLNK_OPTIONS_COMMAND },
        { "pad", XLNK_PAD_COMMAND},
        { NULL, -1}
    };
    /* Try all the mappings */
    for (i=0; map[i].string != NULL; i++) {
        if (strcmp(s, map[i].string) == 0) {
            /* Found it */
            return map[i].type;
        }
    }
    /* Not in map table */
    return XLNK_BAD_COMMAND;
}

/** Describes the arguments that a command should acknowledge */
struct tag_valid_command_args
{
    xlnk_command_type type;
    char **args;
};

typedef struct tag_valid_command_args valid_command_args;

/**
 * Tests if the given command argument name is valid.
 */
static int is_valid_command_arg(xlnk_command_type type, const char *candidate_arg)
{
    int i;
    int j;
    char **args;
    /* Lists of valid args for each command */
    static char *ram_args[] =   { "end", "start", NULL };
    static char *output_args[] =    { "file", NULL };
    static char *copy_args[] =  { "file", NULL };
    static char *bank_args[] =  { "size", "origin", NULL };
    static char *link_args[] =  { "file", "origin", NULL };
    static char *pad_args[] =   { "size", "origin", "offset", NULL };
    /* Table of valid args */
    static valid_command_args ok_args[] = {
        { XLNK_RAM_COMMAND,      ram_args },
        { XLNK_OUTPUT_COMMAND,   output_args },
        { XLNK_COPY_COMMAND,     copy_args },
        { XLNK_BANK_COMMAND,     bank_args },
        { XLNK_LINK_COMMAND,     link_args },
        { XLNK_PAD_COMMAND,      pad_args }
    };
    /* Find arg array for command */
    for (i=0; ok_args[i].type != -1; i++) {
        if (ok_args[i].type == type) {
            /* Now go through array of valid args for command */
            args = ok_args[i].args;
            for (j=0; args[j] != NULL; j++) {
                if (strcmp(args[j], candidate_arg) == 0) {
                    /* Found it. Valid. */
                    return 1;
                }
            }
            /* Didn't find it. Invalid. */
            return 0;
        }
    }
    /* Not valid argument */
    return 0;
}

/*---------------------------------------------------------------------------*/

#define IS_SPACE(c) ( ((c) == '\t') || ((c) == ' ') )

/**
 * Eats whitespace.
 * @param s String with whitespace (possibly)
 * @param i Start index in string, will be incremented beyond whitespace
 */
static void eat_ws(const char *s, int *i)
{
    while (IS_SPACE(s[*i])) (*i)++;
}

/**
 * Tests if a character is in a set of delimiters.
 */
static int is_delim(unsigned char c, const char *delim)
{
    int i;
    /* Compare to all delimiters */
    for (i=0; i<strlen(delim)+1; i++) {
        if (delim[i] == c) return 1;
    }
    /* Not a delimiter */
    return 0;
}

/**
 * Gets a token.
 * @param s String containing token
 * @param i Start index in string, will be incremented beyond token
 * @param delim Set of delimiters which may mark end of token
 * @param dest Where to store the grabbed token
 */
static void get_token(const char *s, int *i, char *delim, char *dest)
{
    unsigned char c;
    int j = 0;
    dest[0] = '\0';
    while (1) {
        /* Get next character */
        c = s[*i];
        /* Test if token delimiter */
        if (is_delim(c, delim)) {
            /* End token */
            dest[j] = '\0';
            return;
        }
        else {
            /* check if escape character */
            if (c == '\\') {
                /* Increase i */
                (*i)++;
                /* Get next character */
                c = s[*i];
                /* Convert to C escape char if applicable */
                switch (c) {
                    case '0':   c = '\0';   break;
                    case 'a':   c = '\a';   break;
                    case 'b':   c = '\b';   break;
                    case 't':   c = '\t';   break;
                    case 'f':   c = '\f';   break;
                    case 'n':   c = '\n';   break;
                    case 'r':   c = '\r';   break;
                }
            }
            /* Copy to dest */
            dest[j++] = c;
            /* Increase i */
            (*i)++;
        }
    }
}

/*---------------------------------------------------------------------------*/

/**
 * Displays an error message generated during script parsing.
 * @param filename Name of file where error was found
 * @param line Line of file
 * @param fmt printf-style format string
 */
static void err(const char *filename, int line, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    /* Print error message w/ location info */
    fprintf(stderr, "%s:%d: error: ", filename, line);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");

    va_end(ap);
}

/*---------------------------------------------------------------------------*/

/**
 * Adds an argument to script command.
 * @param cmd Command
 * @param arg Argument to add
 */
static void add_arg(xlnk_script_command *cmd, xlnk_command_arg *arg)
{
    xlnk_command_arg *a;
    if (cmd->first_arg == NULL) {
        /* Start list */
        cmd->first_arg = arg;
    }
    else {
        /* Add to end of list */
        for (a = cmd->first_arg; a->next != NULL; a = a->next) ;
        a->next = arg;
    }
    arg->next = NULL;
}

/**
 * Adds a command to script.
 * @param sc Script
 * @param cmd Command to add
 */
static void add_command(xlnk_script *sc, xlnk_script_command *cmd)
{
    xlnk_script_command *c;
    if (sc->first_command == NULL) {
        /* Start list */
        sc->first_command = cmd;
    }
    else {
        /* Add to end of list */
        for (c = sc->first_command; c->next != NULL; c = c->next) ;
        c->next = cmd;
    }
    cmd->next = NULL;
}

/**
 * Finalizes a script command.
 * @param cmd Command
 */
static void finalize_command(xlnk_script_command *cmd)
{
    xlnk_command_arg *a;
    xlnk_command_arg *t;
    /* Finalize all arguments */
    for(a = cmd->first_arg; a != NULL; a = t) {
        t = a->next;
        SAFE_FREE(a->key);
        SAFE_FREE(a->value);
        SAFE_FREE(a);
    }
    SAFE_FREE(cmd);
}

/**
 * Gets the processor function for a script command type from a map.
 * @param type The command type
 * @param map A mapping from command types to processor functions
 */
static xlnk_script_commandproc command_type_to_proc(xlnk_command_type type, xlnk_script_commandprocmap *map)
{
    for (; map->proc != NULL; map += 1) {
        if (map->type == type) {
            return map->proc;
        }
    }
    return NULL;
}

/*---------------------------------------------------------------------------*/

/**
 * Parses a script from a file to a script struct.
 * @param filename File to parse
 * @param sc Destination script
 * @return 0 if fail, 1 if OK
 */
int xlnk_script_parse(const char *filename, xlnk_script *sc)
{
    FILE *fp;
    xlnk_command_arg *arg;
    xlnk_script_command *cmd;
    xlnk_command_type type;
    int i;
    char line[1024];
    char cmdname[256];
    char argname[256];
    char argvalue[256];
    static int lineno = 0;
    sc->name = filename;
    sc->first_command = NULL;
    /* Attempt to open script */
    fp = fopen(filename, "rt");
    if (fp == NULL) {
        fprintf(stderr, "error: could not open `%s' for reading\n", filename);
        return 0;
    }
    /* Read commands */
    while (fgets(line, 1023, fp) != NULL) {
        /* Increase line number */
        lineno++;
        /* Skip white space */
        i = 0;
        eat_ws(line, &i);
        /* Skip line if comment or end */
        if ( (line[i] == '#') || (line[i] == '\0') || (line[i] == '\n') ) continue;
        /* Get command name */
        get_token(line, &i, " \t{", cmdname);
        /* Check that it's a valid command */
        if (strlen(cmdname) == 0) {
            err(filename, lineno, "command expected");
            continue;
        }
        else {
            /* Convert from string to command type */
            type = string_to_command_type(cmdname);
            if (type == XLNK_BAD_COMMAND) {
                err(filename, lineno, "unknown command `%s'", cmdname);
                continue;
            }
            else {
                /* Allocate space for command */
                cmd = (xlnk_script_command *)malloc( sizeof(xlnk_script_command) );
                if (cmd != NULL) {
                    /* Set the type */
                    cmd->type = type;
                    /* No arguments (yet) */
                    cmd->first_arg = NULL;
                    /* Store line number */
                    cmd->line = lineno;
                    /* Add command to script */
                    add_command(sc, cmd);
                }
            }
        }
        /* Skip white space */
        eat_ws(line, &i);
        /* Next token should be '{' */
        if (line[i] != '{') {
            err(filename, lineno, "{ expected");
            continue;
        }
        i++;    /* Eat '{' */
        /* Get argument(s) */
        while (line[i] != '}') {
            if (line[i] == '\0') {
                break;
            }
            /* */
            if (cmd->first_arg != NULL) {
                /* Skip white space */
                eat_ws(line, &i);
                /* Next token should be , */
                if (line[i] != ',') {
                    err(filename, lineno, ", expected");
                    continue;
                }
                i++;    /* Eat , */
            }
            /* Skip white space */
            eat_ws(line, &i);
            /* Get argument name */
            get_token(line, &i, " \t=", argname);
            if (strlen(argname) == 0) {
                err(filename, lineno, "argument name expected");
                continue;
            }
            /* Skip white space */
            eat_ws(line, &i);
            /* Next token should be '=' */
            if (line[i] != '=') {
                err(filename, lineno, "= expected");
                continue;
            }
            i++;    /* Eat '=' */
            /* Skip white space */
            eat_ws(line, &i);
            /* Get value */
            get_token(line, &i, " \t},", argvalue);
            if (strlen(argvalue) == 0) {
                err(filename, lineno, "value expected for argument `%s'", argname);
                continue;
            }
            // Check if the argument name is valid for this command */
            if (is_valid_command_arg(cmd->type, argname) ) {
                /* Create argument struct */
                arg = (xlnk_command_arg *)malloc( sizeof(xlnk_command_arg) );
                if (arg != NULL) {
                    arg->key = (char *)malloc( strlen(argname)+1 );
                    arg->value = (char *)malloc( strlen(argvalue)+1 );
                    /* Copy fields */
                    strcpy(arg->key, argname);
                    strcpy(arg->value, argvalue);
                    /* Store argument in list */
                    add_arg(cmd, arg);
                }
            }
            else {
                /* Not valid argument name */
                err(filename, lineno, "invalid argument `%s'", argname);
                continue;
            }
            /* Skip white space */
            eat_ws(line, &i);
        }
    }
    /* Close script */
    fclose(fp);
    /* Success */
    return 1;
}

/**
 * Finalizes a script.
 * @param sc Script
 */
void xlnk_script_finalize(xlnk_script *sc)
{
    xlnk_script_command *c;
    xlnk_script_command *t;
    if (sc == NULL) { return; }
    for(c = sc->first_command; c != NULL; c = t) {
        t = c->next;
        finalize_command(c);
    }
}

/**
 * Gets the length (that is, number of commands) of a script.
 * @param sc Script
 * @return Number of commands in script
 */
int xlnk_script_length(xlnk_script *sc)
{
    xlnk_script_command *c;
    int i;
    if (sc == NULL) { return 0; }
    for (i=0, c=sc->first_command; c != NULL; i++, c = c->next) ;
    return i;
}

/**
 * Gets a command from a script.
 * @param sc Script
 * @param index Command index
 */
xlnk_script_command *xlnk_script_get_command(xlnk_script *sc, int index)
{
    xlnk_script_command *c;
    int i;
    if (sc == NULL) { return NULL; }
    for (i=0, c=sc->first_command; (c != NULL) && (i != index); i++, c = c->next) ;
    return c;
}

/**
 * Processes commands in script.
 * @param sc Script
 * @param map Map from command to processor function
 */
void xlnk_script_walk(xlnk_script *sc, xlnk_script_commandprocmap *map, void *arg)
{
    xlnk_script_command *c;
    xlnk_script_commandproc p;
    if (sc == NULL) { return; }
    /* Walk all the commands */
    for (c=sc->first_command; c != NULL; c = c->next) {
        /* Process this command if it has a processor function */
        p = command_type_to_proc(c->type, map);
        if (p != NULL) {
            p(sc, c, arg);
        }
    }
}

/**
 * Gets value of argument for script command.
 * @param c Command
 * @param key Key (argument name)
 */
const char *xlnk_script_get_command_arg(xlnk_script_command *c, const char *key)
{
    xlnk_command_arg *a;
    if (c == NULL) { return NULL; }
    /* Go through all args */
    for (a = c->first_arg; a != NULL; a = a->next) {
        if (strcmp(key, a->key) == 0) {
            /* Found it, return its value */
            return a->value;
        }
    }
    /* Not found */
    return NULL;
}

/**
 * Gets the string representation of a command type.
 * @param type Command type
 */
const char *xlnk_script_command_type_to_string(xlnk_command_type type)
{
    switch (type) {
        case XLNK_RAM_COMMAND:   return "ram";
        case XLNK_OUTPUT_COMMAND:    return "output";
        case XLNK_COPY_COMMAND:  return "copy";
        case XLNK_BANK_COMMAND:  return "bank";
        case XLNK_LINK_COMMAND:  return "link";
        case XLNK_OPTIONS_COMMAND:   return "options";
        case XLNK_PAD_COMMAND:   return "pad";
        default:
        /* Invalid command */
        break;
    }
    return "Invalid command!";
}

/**
 * Counts the number of commands of the given type in a script.
 * @param sc Script
 * @param type Command type
 */
int xlnk_script_count_command_type(xlnk_script *sc, xlnk_command_type type)
{
    xlnk_script_command *c;
    int count;
    if (sc == NULL) { return 0; }
    for (count=0, c=sc->first_command; c != NULL; c = c->next) {
        if (c->type == type) {
            count++;
        }
    }
    return count;
}
