/*
 * $Id: xasm.c,v 1.22 2007/11/11 22:35:22 khansen Exp $
 * $Log: xasm.c,v $
 * Revision 1.22  2007/11/11 22:35:22  khansen
 * compile on mac
 *
 * Revision 1.21  2007/08/19 11:18:56  khansen
 * --case-insensitive option
 *
 * Revision 1.20  2007/08/12 18:58:12  khansen
 * ability to generate pure 6502 binary (--pure-binary switch)
 *
 * Revision 1.19  2007/08/11 01:24:36  khansen
 * includepaths support (-I option)
 *
 * Revision 1.18  2007/08/10 20:21:02  khansen
 * *** empty log message ***
 *
 * Revision 1.17  2007/08/07 22:42:53  khansen
 * version
 *
 * Revision 1.16  2007/07/22 14:49:40  khansen
 * don't crash in change_extension()
 *
 * Revision 1.15  2007/07/22 13:33:26  khansen
 * convert tabs to whitespaces
 *
 * Revision 1.14  2005/01/09 11:19:23  kenth
 * xorcyst 1.4.5
 *
 * Revision 1.13  2005/01/05 09:37:32  kenth
 * xorcyst 1.4.4
 *
 * Revision 1.12  2005/01/05 01:52:13  kenth
 * xorcyst 1.4.3
 *
 * Revision 1.11  2005/01/04 21:35:10  kenth
 * return error code from main() when error count > 0
 *
 * Revision 1.10  2004/12/29 21:43:50  kenth
 * xorcyst 1.4.2
 *
 * Revision 1.9  2004/12/25 02:23:19  kenth
 * xorcyst 1.4.1
 *
 * Revision 1.8  2004/12/19 19:58:46  kenth
 * xorcyst 1.4.0
 *
 * Revision 1.7  2004/12/18 17:01:21  kenth
 * --debug switch, multiple verbose levels
 *
 * Revision 1.6  2004/12/16 13:20:35  kenth
 * xorcyst 1.3.5
 *
 * Revision 1.5  2004/12/14 01:50:12  kenth
 * xorcyst 1.3.0
 *
 * Revision 1.4  2004/12/11 02:06:27  kenth
 * xorcyst 1.2.0
 *
 * Revision 1.3  2004/12/06 04:53:02  kenth
 * xorcyst 1.1.0
 *
 * Revision 1.2  2004/06/30 23:37:54  kenth
 * replaced argp with something else
 *
 * Revision 1.1  2004/06/30 07:56:02  kenth
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
 * The main program.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "getopt.h"
#include "astnode.h"
#include "astproc.h"
#include "symtab.h"
#include "codegen.h"
#include "xasm.h"

/*---------------------------------------------------------------------------*/

/* Parser stuff we need. */
int yyparse(void);
extern int yydebug;
extern int yynerrs;

/* Scanner stuff we need. */
int yybegin(const char *, int, int);

/* Other. */
astnode *root_node;
static symtab *symbol_table;
char *xasm_path;

/*---------------------------------------------------------------------------*/
/* Argument parsing stuff. */

static char program_version[] = "xasm 1.5.1";

/* Argument variables set by arg parser. */
xasm_arguments xasm_args;

/* Long options for getopt_long(). */
static struct option long_options[] = {
  { "define",   required_argument, 0, 'D' },
  { "include-path", required_argument, 0, 'I' },
  { "output",   required_argument, 0, 'o' },
  { "quiet",    no_argument, 0, 'q' },
  { "silent",   no_argument, 0, 's' },
  { "verbose",  no_argument, 0, 'v' },
  { "debug",    no_argument, 0, 'g' },
  { "help", no_argument, 0, 0 },
  { "usage",    no_argument, 0, 0 },
  { "version",  no_argument, 0, 'V' },
  { "swap-parens", no_argument, 0, 0 },
  { "pure-binary", no_argument, 0, 0 },
  { "case-insensitive", no_argument, 0, 0 },
  { "no-warn",  no_argument, 0, 0 },
  { 0 }
};

/* Prints usage message and exits. */
static void usage()
{
    printf("\
Usage: xasm [-gqsvV] [-D IDENT[=VALUE]] [--define=IDENT]\n\
            [-o FILE] [--output=FILE] [--pure-binary]\n\
            [--include-path=DIR] [-I DIR] [--swap-parens]\n\
            [--case-insensitive]\n\
            [--no-warn] [--verbose] [--quiet] [--silent] \n\
            [--debug] [--help] [--usage] [--version]\n\
            FILE\n\
");
    exit(0);
}

/* Prints help message and exits. */
static void help()
{
    printf("\
Usage: xasm [OPTION...] FILE\n\
The XORcyst Assembler -- it kicks the 6502's ass\n\
\n\
-D, --define=IDENT[=VALUE] Define IDENT\n\
-I, --include-path=DIR     Specify a search path for include files\n\
-o, --output=FILE          Output to FILE instead of standard output\n\
    --pure-binary          Output pure 6502 binary\n\
    --swap-parens          Use ( ) instead of [ ] for indirection\n\
    --case-insensitive     Case-insensitive identifiers\n\
    --no-warn              Suppress warnings\n\
-q, -s, --quiet, --silent  Don't produce any output\n\
-v, --verbose              Produce verbose output\n\
-g, --debug                Retain file locations\n\
    --help                 Give this help list\n\
    --usage                Give a short usage message\n\
-V, --version              Print program version\n\
\n\
Mandatory or optional arguments to long options are also mandatory or optional\n\
for any corresponding short options.\n\
\n\
Report bugs to <dev@null>.\n\
");
    exit(0);
}

/* Prints version and exits. */
static void version()
{
    printf("%s\n", program_version);
    exit(0);
}

/**
 * Checks if a character is alpha (a-z, A-Z).
 */
static int __isalpha(char c)
{
    return ( ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) );
}

/**
 * Checks if a character is alpha (a-z, A-Z) or numeric (0-9).
 */
static int __isalnum(char c)
{
    return ( __isalpha(c) || ((c >= '0') && (c <= '9')) );
}

/**
 * Checks that an identifier matches the regexp [a-zA-Z_][a-zA-Z0-9_]*
 * @param id Identifier to validate
 * @return 1 if OK, 0 otherwise
 */
static int validate_ident(char *id)
{
    int i;
    char c;
    c = id[0];
    if ( !__isalpha(c) && (c != '_') ) {
        return 0;
    }
    for (i=1; i<strlen(id); i++) {
        c = id[i];
        if ( !__isalnum(c) && (c != '_') ) {
            return 0;
        }
    }
    return 1;   /* OK */
}

/* Parses program arguments. */
static void
parse_arguments (int argc, char **argv)
{
    int key;
    char *id;
    char *str;
    astnode *val;
    symtab_entry *e;
    /* Dummy location for --define */
    static location loc = { 0, 0, 0, 0, NULL };
    /* getopt_long stores the option index here. */
    int index = 0;

    /* Set default values. */
    xasm_args.debug = 0;
    xasm_args.silent = 0;
    xasm_args.verbose = 0;
    xasm_args.swap_parens = 0;
    xasm_args.pure_binary = 0;
    xasm_args.case_insensitive = 0;
    xasm_args.input_file = NULL;
    xasm_args.output_file = NULL;
    xasm_args.include_paths = NULL;
    xasm_args.include_path_count = 0;

    /* Parse options. */
    while ((key = getopt_long(argc, argv, "D:I:o:qsvV", long_options, &index)) != -1) {
        switch (key) {
            case 'g':
            xasm_args.debug = 1;
            break;

            case 'q': case 's':
            xasm_args.silent = 1;
            break;

            case 'v':
            xasm_args.verbose++;
            break;

            case 'o':
            xasm_args.output_file = optarg;
            break;

            case 'D':
            if (strchr(optarg, '=') != NULL) {
                /* IDENT=VALUE */
                id = strtok(optarg, "=");
                str = strtok(NULL, "\0");
                /* Parse the value */
                if (str[0] == '\"') {
                    /* Assume string */
                    str = strtok(&str[1], "\"");
                    val = astnode_create_string(str, loc);
                } else {
                    /* Assume integer */
                    val = astnode_create_integer(strtol(str, NULL, 0), loc);
                }
            } else {
                id = optarg;
                val = astnode_create_integer(0, loc);
            }
            if (validate_ident(id)) {
                e = symtab_lookup(id);
                if (e == NULL) {
                    symtab_enter(id, CONSTANT_SYMBOL, val, 0);
                } else {
                    /* Error, redefinition */
                    fprintf(stderr, "--ident: `%s' already defined\n", id);
                }
            } else {
                /* Error, bad identifier */
                fprintf(stderr, "--ident: `%s' is not a valid identifier\n", id);
            }
            break;

            case 'I': {
                char *p;
                int count = xasm_args.include_path_count + 1;
                xasm_args.include_paths = (char **)realloc(
                    xasm_args.include_paths, sizeof(const char *) * count);
                p = (char *)malloc(strlen(optarg) + 1);
                strcpy(p, optarg);
                xasm_args.include_paths[count-1] = p;
                xasm_args.include_path_count = count;
            }
            break;

            case 0:
            /* Use index to differentiate between options */
            if (strcmp(long_options[index].name, "usage") == 0) {
                usage();
            } else if (strcmp(long_options[index].name, "help") == 0) {
                help();
            } else if (strcmp(long_options[index].name, "swap-parens") == 0) {
                xasm_args.swap_parens = 1;
            } else if (strcmp(long_options[index].name, "pure-binary") == 0) {
                xasm_args.pure_binary = 1;
            } else if (strcmp(long_options[index].name, "case-insensitive") == 0) {
                xasm_args.case_insensitive = 1;
            } else if (strcmp(long_options[index].name, "no-warn") == 0) {
                xasm_args.no_warn = 1;
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
        printf("Usage: xasm [OPTION...] FILE\nTry `xasm --help' or `xasm --usage' for more information.\n");
        exit(1);
    }
    else {
        xasm_args.input_file = argv[optind];
    }
}

/*---------------------------------------------------------------------------*/

/**
 * Changes the extension of a filename.
 * @param infile Filename whose extension to change
 * @param ext New extension
 * @param outfile Destination filename
 */
static void change_extension(const char *infile, const char *ext, char *outfile)
{
    char *p;
    /* Find the last dot. */
    p = strrchr(infile, '.');
    if (p == NULL) {
        /* There is no dot, simply concatenate extension. */
        sprintf(outfile, "%s.%s", infile, ext);
    }
    else {
        /* Copy the name up to and including the last dot */
        strncpy(outfile, infile, p - infile + 1);
        outfile[p - infile + 1] = '\0';
        /* Then concatenate the extension. */
        strcat(outfile, ext);
    }
}

/*---------------------------------------------------------------------------*/

/**
 * Prints message only if --verbose option was given to assembler.
 */
static void verbose(const char *s)
{
    if (xasm_args.verbose) {
        printf("%s\n", s);
    }
}

/**
 * Gets total number of errors (parsing + semantics).
 */
static int total_errors()
{
    return yynerrs + astproc_err_count();
}

/**
 * Program entrypoint.
 */
int main(int argc, char *argv[]) {
    char *default_outfile = 0;

    /* Working directory is needed for include statements */
    xasm_path = getcwd(NULL, 0);

    /* Create global symbol table (auto-pushed on stack) */
    symbol_table = symtab_create();

    /* Parse our arguments. */
    parse_arguments (argc, argv);

    /* Open input for scanning */
    if (!yybegin(xasm_args.input_file,
                 xasm_args.swap_parens,
                 xasm_args.case_insensitive)) {
        printf("error: could not open `%s' for reading\n", xasm_args.input_file);
        symtab_finalize(symbol_table);
        return(1);
    }

 /* Parse it into a syntax tree */
    //yydebug = -1;
    verbose("Parsing input...");
    yyparse();

    if (root_node == NULL) {
        symtab_finalize(symbol_table);
        return(0);
    }

    /* First pass does a lot of stuff. */
    verbose("First pass...");
    astproc_first_pass(root_node);

    /* Second pass does more stuff. */
    verbose("Second pass...");
    astproc_second_pass(root_node);

    /* Third pass is fun. */
    verbose("Third pass...");
    astproc_third_pass(root_node);

    if (xasm_args.pure_binary) {
        /* Do another pass to prepare for writing pure 6502 */
        verbose("Fourth pass...");
        astproc_fourth_pass(root_node);
    }

    /* Print the final AST (debugging) */
//    astnode_print(root_node, 0);

    /* If no errors, proceed with code generation. */
    if (total_errors() == 0) {
        if (xasm_args.output_file == NULL) {
            /* Create default name of output */
            const char *default_ext = "o";
            int default_outfile_len = strlen(xasm_args.input_file)
                                    + /*dot*/1 + strlen(default_ext) + 1;
            default_outfile = (char *)malloc(default_outfile_len);
            change_extension(xasm_args.input_file, default_ext, default_outfile);
            xasm_args.output_file = default_outfile;
        }
        /* Write it! */
        verbose("Generating final output...");
        if (xasm_args.pure_binary) {
            astproc_fifth_pass(root_node);
        } else {
            codegen_write(root_node, xasm_args.output_file);
        }
    }

    /* Cleanup */
    verbose("cleaning up...");
    symtab_pop();
    symtab_finalize(symbol_table);
    astnode_finalize(root_node);

    if (default_outfile)
        free(default_outfile);

    if (xasm_args.include_path_count > 0) {
        int i;
        for (i = 0; i < xasm_args.include_path_count; ++i)
            free(xasm_args.include_paths[i]);
        free(xasm_args.include_paths);
    }

    free(xasm_path);

    return (total_errors() == 0) ? 0 : 1;
}
