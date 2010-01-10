%{
/*
 * $Id: parser.y,v 1.14 2007/11/11 22:35:51 khansen Exp $
 * $Log: parser.y,v $
 * Revision 1.14  2007/11/11 22:35:51  khansen
 * compile on mac
 *
 * Revision 1.13  2007/08/19 11:19:47  khansen
 * --case-insensitive option
 *
 * Revision 1.12  2007/08/12 18:59:00  khansen
 * ability to generate pure 6502 binary
 *
 * Revision 1.11  2007/08/11 01:25:50  khansen
 * includepaths support (-I option)
 *
 * Revision 1.10  2007/07/22 13:34:38  khansen
 * convert tabs to whitespaces
 *
 * Revision 1.9  2005/01/05 02:28:40  kenth
 * anonymous union parsing
 *
 * Revision 1.8  2004/12/29 21:45:26  kenth
 * xorcyst 1.4.2
 * static indexing
 *
 * Revision 1.7  2004/12/19 19:59:14  kenth
 * xorcyst 1.4.0
 *
 * Revision 1.6  2004/12/16 13:22:27  kenth
 * xorcyst 1.3.5
 *
 * Revision 1.5  2004/12/14 01:50:42  kenth
 * xorcyst 1.3.0
 *
 * Revision 1.4  2004/12/11 02:12:41  kenth
 * xorcyst 1.2.0
 *
 * Revision 1.3  2004/12/09 11:17:15  kenth
 * added: warning_statement, error_statement
 *
 * Revision 1.2  2004/12/06 05:06:21  kenth
 * xorcyst 1.1.0
 *
 * Revision 1.1  2004/06/30 07:57:03  kenth
 * Initial revision
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "symtab.h"
#include "loc.h"
#include "xasm.h"
//#define YYDEBUG 1
int yyparse(void);
void yyerror(const char *);   /* In lexer */
int yylex(void);    /* In lexer */
int yypushandrestart(const char *, int);   /* In lexer */
void __yy_memcpy(char *, char *, int);
extern char *yytext;    /* In lexer */
extern YYLTYPE yylloc;  /* In lexer */
char *scan_include(int); /* In lexer */
extern astnode *root_node;  /* Root of the generated parse tree */
void handle_incsrc(astnode *);  /* See below */
void handle_incbin(astnode *);  /* See below */
%}

%union {
    long integer;
    int mnemonic;
    const char *string;
    const char *label;
    const char *ident;
    astnode *node;
};

%error-verbose

%token <integer> INTEGER_LITERAL
%token <string> STRING_LITERAL
%token <string> FILE_PATH
%token <ident> IDENTIFIER
%token <ident> LOCAL_ID
%token <ident> FORWARD_BRANCH
%token <ident> BACKWARD_BRANCH
%token <label> LABEL
%token <label> LOCAL_LABEL
%token <mnemonic> MNEMONIC

%type <node> identifier identifier_opt local_id assembly_unit statement labelable_statement statement_list statement_list_opt if_statement elif_statement elif_statement_list elif_statement_list_opt ifdef_statement ifndef_statement macro_decl_statement macro_statement instruction_statement data_statement storage_statement null_statement incsrc_statement incbin_statement equ_statement assign_statement public_statement extrn_statement dataseg_statement codeseg_statement charmap_statement struc_decl_statement union_decl_statement enum_decl_statement record_decl_statement instruction expression extended_expression expression_opt arithmetic_expression comparison_expression literal label label_decl identifier_list expression_list file_specifier param_list_opt arg_list_opt else_part_opt scope_access struc_access struc_initializer field_initializer_list field_initializer_list_opt field_initializer datatype storage named_data_statement unnamed_data_statement named_storage_statement unnamed_storage_statement proc_statement rept_statement label_statement message_statement warning_statement error_statement while_statement define_statement align_statement org_statement symbol_type enum_item_list enum_item record_field_list record_field sizeof_arg label_addr_part_opt label_type_part_opt from_part_opt indexed_identifier

%token _LABEL_ BYTE CHAR WORD DWORD DSB DSW DSD DATASEG CODESEG IF IFDEF IFNDEF ELSE ELIF ENDIF INCSRC INCBIN MACRO REPT WHILE ENDM ALIGN EQU DEFINE END PUBLIC EXTRN CHARMAP STRUC UNION ENDS RECORD ENUM ENDE PROC ENDP SIZEOF MASK TAG MESSAGE WARNING ERROR ZEROPAGE ORG

%token '\n'
%token ':'
%token '@'
%token SCOPE_OP
%token 'A' 'X' 'Y'
%token '='
%token '$'
%token '{' '}'
%token '[' ']'
%left ','
%token '.'
%right '#'
%right LO_OP HI_OP
%left '|'
%left '^'
%left '&'
%left EQ_OP NE_OP
%left LE_OP GE_OP '>' '<'
%left SHL_OP SHR_OP
%left '+' '-'
%right UMINUS
%left '*' '/' '%'
%right '!' '~'
%left '('
%right ')'
%start assembly_unit
%%
assembly_unit:
    statement_list end_opt { root_node = astnode_create_list($1); }
    ;

end_opt:
    END line_tail { ; }
    | /* empty */
    ;

statement_list:
    labelable_statement { $$ = $1; }
    | statement_list labelable_statement {
         if ($1 != NULL) { $$ = $1; astnode_add_sibling($$, $2); }
         else { $$ = $2; }
        }
    ;

statement_list_opt:
    statement_list { $$ = $1; }
    | { $$ = NULL; }
    ;

labelable_statement:
    label_decl statement { $$ = $1; astnode_add_sibling($$, $2); }
    | statement { $$ = $1; }
    ;

statement:
    if_statement { $$ = $1; }
    | ifdef_statement { $$ = $1; }
    | ifndef_statement { $$ = $1; }
    | macro_decl_statement { $$ = $1; }
    | macro_statement { $$ = $1; }
    | incsrc_statement { $$ = $1; }
    | incbin_statement { $$ = $1; }
    | equ_statement { $$ = $1; }
    | assign_statement { $$ = $1; }
    | public_statement { $$ = $1; }
    | extrn_statement { $$ = $1; }
    | instruction_statement { $$ = $1; }
    | data_statement { $$ = $1; }
    | storage_statement { $$ = $1; }
    | dataseg_statement { $$ = $1; }
    | codeseg_statement { $$ = $1; }
    | charmap_statement { $$ = $1; }
    | struc_decl_statement { $$ = $1; }
    | union_decl_statement { $$ = $1; }
    | enum_decl_statement { $$ = $1; }
    | record_decl_statement { $$ = $1; }
    | proc_statement { $$ = $1; }
    | rept_statement { $$ = $1; }
    | while_statement { $$ = $1; }
    | label_statement { $$ = $1; }
    | message_statement { $$ = $1; }
    | warning_statement { $$ = $1; }
    | error_statement { $$ = $1; }
    | define_statement { $$ = $1; }
    | align_statement { $$ = $1; }
    | org_statement { $$ = $1; }
    | null_statement { $$ = $1; }
    | error line_tail { $$ = NULL; }
    ;

org_statement:
    ORG expression line_tail { $$ = astnode_create_org($2, @$); }
    ;

align_statement:
    ALIGN identifier_list expression line_tail { $$ = astnode_create_align($2, $3, @$); }
    ;

warning_statement:
    WARNING expression line_tail { $$ = astnode_create_warning($2, @$); }
    ;

error_statement:
    ERROR expression line_tail { $$ = astnode_create_error($2, @$); }
    ;

message_statement:
    MESSAGE expression line_tail { $$ = astnode_create_message($2, @$); }
    ;

label_statement:
    _LABEL_ identifier label_addr_part_opt label_type_part_opt line_tail { $$ = astnode_create_label($2->label, $3, $4, @$); astnode_finalize($2); }
    ;

label_addr_part_opt:
    '=' expression { $$ = $2; }
    | { $$ = NULL; }
    ;

label_type_part_opt:
    ':' symbol_type { $$ = $2; }
    | { $$ = NULL; }
    ;

while_statement:
    WHILE expression line_tail statement_list_opt ENDM line_tail { $$ = astnode_create_while($2, $4, @$); }
    ;

rept_statement:
    REPT expression line_tail statement_list_opt ENDM line_tail { $$ = astnode_create_rept($2, $4, @$); }
    ;

proc_statement:
    PROC identifier line_tail statement_list_opt ENDP line_tail { $$ = astnode_create_proc($2, $4, @$); }
    ;

struc_decl_statement:
    STRUC identifier line_tail statement_list_opt ENDS line_tail { $$ = astnode_create_struc_decl($2, $4, @$); }
    ;

union_decl_statement:
    UNION identifier_opt line_tail statement_list_opt ENDS line_tail { $$ = astnode_create_union_decl($2, $4, @$); }
    ;

enum_decl_statement:
    ENUM identifier line_tail enum_item_list ENDE line_tail { $$ = astnode_create_enum_decl($2, $4, @$); }
    ;

enum_item_list:
    enum_item { $$ = $1; }
    | enum_item_list enum_item { $$ = $1; astnode_add_sibling($$, $2); }
    ;

enum_item:
    assign_statement { $$ = $1; }
    | identifier line_tail { $$ = $1; }
    ;

record_decl_statement:
    RECORD identifier record_field_list line_tail { $$ = astnode_create_record_decl($2, $3, @$); }
    ;

record_field_list:
    record_field { $$ = $1; }
    | record_field_list ',' record_field { $$ = $1; astnode_add_sibling($$, $3); }
    ;

record_field:
    identifier ':' expression { $$ = astnode_create_bitfield_decl($1, $3, @$); }
    ;

charmap_statement:
    CHARMAP file_specifier line_tail { $$ = astnode_create_charmap($2, @$); }
    ;

dataseg_statement:
    DATASEG line_tail { $$ = astnode_create_dataseg(0, @$); }
    | DATASEG ZEROPAGE line_tail { $$ = astnode_create_dataseg(ZEROPAGE_FLAG, @$); }

codeseg_statement:
    CODESEG line_tail { $$ = astnode_create_codeseg(@$); }
    ;

null_statement:
    line_tail { $$ = NULL; }
    ;

label_decl:
    label { $$ = $1; }
    ;

line_tail:
    newline { ; }
    | ':' { ; }
    ;

newline:
    '\n' { ; }
    ;

instruction_statement:
    instruction line_tail { $$ = $1; }
    ;

instruction:
    MNEMONIC { $$ = astnode_create_instruction($1, IMPLIED_MODE, NULL, @$); }
    | MNEMONIC 'A' { $$ = astnode_create_instruction($1, ACCUMULATOR_MODE, NULL, @$); }
    | MNEMONIC '#' expression { $$ = astnode_create_instruction($1, IMMEDIATE_MODE, $3, @$); }
    | MNEMONIC expression { $$ = astnode_create_instruction($1, ABSOLUTE_MODE, $2, @$); }
    | MNEMONIC expression ',' 'X' { $$ = astnode_create_instruction($1, ABSOLUTE_X_MODE, $2, @$); }
    | MNEMONIC expression ',' 'Y' { $$ = astnode_create_instruction($1, ABSOLUTE_Y_MODE, $2, @$); }
    | MNEMONIC '[' expression ',' 'X' ']' { $$ = astnode_create_instruction($1, PREINDEXED_INDIRECT_MODE, $3, @$); }
    | MNEMONIC '[' expression ']' ',' 'Y' { $$ = astnode_create_instruction($1, POSTINDEXED_INDIRECT_MODE, $3, @$); }
    | MNEMONIC '[' expression ']' { $$ = astnode_create_instruction($1, INDIRECT_MODE, $3, @$); }
    ;

expression:
    indexed_identifier { $$ = $1; }
    | SIZEOF sizeof_arg { $$ = astnode_create_sizeof($2, @$); }
    | scope_access { $$ = $1; }
    | struc_access { $$ = $1; }
    | local_id { $$ = $1; }
    | literal { $$ = $1; }
    | '$' { $$ = astnode_create_pc(@$); }
    | arithmetic_expression { $$ = $1; }
    | comparison_expression { $$ = $1; }
    | '(' expression ')' { $$ = $2; }
    | '+' { $$ = astnode_create_forward_branch("+", @$); }
    | '-' { $$ = astnode_create_backward_branch("-", @$); }
    | FORWARD_BRANCH { $$ = astnode_create_forward_branch($1, @$); }
    | BACKWARD_BRANCH { $$ = astnode_create_backward_branch($1, @$); }
    | MASK scope_access { $$ = astnode_create_mask($2, @$); }
    ;

indexed_identifier:
    identifier { $$ = $1; }
    | identifier '[' expression ']' { $$ = astnode_create_index($1, $3, @$); }
    | identifier '(' expression ')' { $$ = astnode_create_index($1, $3, @$); }
    ;

extended_expression:
    expression { $$ = $1; }
    | struc_initializer { $$ = $1; }
    ;

sizeof_arg:
    identifier { $$ = $1; }
    | datatype { $$ = $1; }
    ;

expression_opt:
    expression { $$ = $1; }
    | { $$ = NULL; }
    ;

scope_access:
    identifier SCOPE_OP identifier { $$ = astnode_create_scope($1, $3, @$); }
    ;

struc_access:
    indexed_identifier '.' struc_access { $$ = astnode_create_dot($1, $3, @$); }
    | indexed_identifier '.' indexed_identifier { $$ = astnode_create_dot($1, $3, @$); }
    ;

struc_initializer:
    '{' field_initializer_list_opt '}' { $$ = astnode_create_struc($2, @$); }
    ;

field_initializer_list_opt:
    field_initializer_list { $$ = $1; }
    | { $$ = NULL; }
    ;

field_initializer_list:
    field_initializer { $$ = $1; }
    | field_initializer_list ',' field_initializer { $$ = $1; astnode_add_sibling($$, $3); }
    ;

field_initializer:
    extended_expression { $$ = $1; }
    | { $$ = astnode_create_null(@$); }
    ;

local_id:
    LOCAL_ID { $$ = astnode_create_local_id($1, @$); }
    ;

arithmetic_expression:
    expression '+' expression { $$ = astnode_create_arithmetic(PLUS_OPERATOR, $1, $3, @$); }
    | expression '-' expression { $$ = astnode_create_arithmetic(MINUS_OPERATOR, $1, $3, @$); }
    | expression '*' expression { $$ = astnode_create_arithmetic(MUL_OPERATOR, $1, $3, @$); }
    | expression '/' expression { $$ = astnode_create_arithmetic(DIV_OPERATOR, $1, $3, @$); }
    | expression '%' expression { $$ = astnode_create_arithmetic(MOD_OPERATOR, $1, $3, @$); }
    | expression '&' expression { $$ = astnode_create_arithmetic(AND_OPERATOR, $1, $3, @$); }
    | expression '|' expression { $$ = astnode_create_arithmetic(OR_OPERATOR, $1, $3, @$); }
    | expression '^' expression { $$ = astnode_create_arithmetic(XOR_OPERATOR, $1, $3, @$); }
    | expression SHL_OP expression { $$ = astnode_create_arithmetic(SHL_OPERATOR, $1, $3, @$); }
    | expression SHR_OP expression { $$ = astnode_create_arithmetic(SHR_OPERATOR, $1, $3, @$); }
    | '~' expression { $$ = astnode_create_arithmetic(NEG_OPERATOR, $2, NULL, @$); }
    | '!' expression { $$ = astnode_create_arithmetic(NOT_OPERATOR, $2, NULL, @$); }
    | '^' identifier { $$ = astnode_create_arithmetic(BANK_OPERATOR, $2, NULL, @$); }
    | '<' expression %prec LO_OP { $$ = astnode_create_arithmetic(LO_OPERATOR, $2, NULL, @$); }
    | '>' expression %prec HI_OP { $$ = astnode_create_arithmetic(HI_OPERATOR, $2, NULL, @$); }
    | '-' expression %prec UMINUS { $$ = astnode_create_arithmetic(UMINUS_OPERATOR, $2, NULL, @$); }
    ;

comparison_expression:
    expression EQ_OP expression { $$ = astnode_create_arithmetic(EQ_OPERATOR, $1, $3, @$); }
    | expression NE_OP expression { $$ = astnode_create_arithmetic(NE_OPERATOR, $1, $3, @$); }
    | expression '>' expression { $$ = astnode_create_arithmetic(GT_OPERATOR, $1, $3, @$); }
    | expression '<' expression { $$ = astnode_create_arithmetic(LT_OPERATOR, $1, $3, @$); }
    | expression GE_OP expression { $$ = astnode_create_arithmetic(GE_OPERATOR, $1, $3, @$); }
    | expression LE_OP expression { $$ = astnode_create_arithmetic(LE_OPERATOR, $1, $3, @$); }
    ;

label:
    LABEL { $$ = astnode_create_label($1, NULL, NULL, @$); }
    | LOCAL_LABEL { $$ = astnode_create_local_label($1, @$); }
    | '+' { $$ = astnode_create_forward_branch_decl("+", @$); }
    | '-' { $$ = astnode_create_backward_branch_decl("-", @$); }
    | FORWARD_BRANCH { $$ = astnode_create_forward_branch_decl($1, @$); }
    | BACKWARD_BRANCH { $$ = astnode_create_backward_branch_decl($1, @$); }
    ;

identifier:
    IDENTIFIER { $$ = astnode_create_identifier($1, @$); }
    ;

identifier_opt:
    identifier { $$ = $1; }
    | { $$ = astnode_create_null(@$); }
    ;

literal:
    INTEGER_LITERAL { $$ = astnode_create_integer($1, @$); }
    | STRING_LITERAL { $$ = astnode_create_string($1, @$); }
    ;

if_statement:
    IF expression line_tail statement_list_opt elif_statement_list_opt else_part_opt ENDIF line_tail { $$ = astnode_create_if($2, $4, $5, $6, @$); }
    ;

elif_statement_list_opt:
    elif_statement_list { $$ = $1; }
    | { $$ = NULL; }
    ;

elif_statement_list:
    elif_statement { $$ = $1; }
    | elif_statement_list elif_statement { $$ = $1; astnode_add_sibling($$, $2); }
    ;

elif_statement:
    ELIF expression line_tail statement_list_opt { $$ = astnode_create_case($2, $4, @$); }
    ;

else_part_opt:
    ELSE line_tail statement_list_opt { $$ = $3; }
    | { $$ = NULL; }
    ;

ifdef_statement:
    IFDEF identifier line_tail statement_list_opt else_part_opt ENDIF line_tail { $$ = astnode_create_ifdef($2, $4, $5, @$); }
    ;

ifndef_statement:
    IFNDEF identifier line_tail statement_list_opt else_part_opt ENDIF line_tail { $$ = astnode_create_ifndef($2, $4, $5, @$); }
    ;

data_statement:
    named_data_statement line_tail { $$ = $1; }
    | unnamed_data_statement line_tail { $$ = $1; }
    ;

named_data_statement:
    identifier unnamed_data_statement { $$ = astnode_create_var_decl(0, $1, $2, @$); }
    | ZEROPAGE identifier unnamed_data_statement { $$ = astnode_create_var_decl(ZEROPAGE_FLAG, $2, $3, @$); }
    | PUBLIC identifier unnamed_data_statement { $$ = astnode_create_var_decl(PUBLIC_FLAG, $2, $3, @$); }
    | ZEROPAGE PUBLIC identifier unnamed_data_statement { $$ = astnode_create_var_decl(ZEROPAGE_FLAG | PUBLIC_FLAG, $3, $4, @$); }
    | PUBLIC ZEROPAGE identifier unnamed_data_statement { $$ = astnode_create_var_decl(PUBLIC_FLAG | ZEROPAGE_FLAG, $3, $4, @$); }
    ;

unnamed_data_statement:
    datatype expression_list { $$ = astnode_create_data($1, $2, @$); }
    | datatype { $$ = astnode_create_storage($1, NULL, @$); }
    | datatype '[' expression ']' { $$ = astnode_create_storage($1, $3, @$); }
    ;

datatype:
    BYTE { $$ = astnode_create_datatype(BYTE_DATATYPE, NULL, @$); }
    | CHAR { $$ = astnode_create_datatype(CHAR_DATATYPE, NULL, @$); }
    | WORD { $$ = astnode_create_datatype(WORD_DATATYPE, NULL, @$); }
    | DWORD { $$ = astnode_create_datatype(DWORD_DATATYPE, NULL, @$); }
    | TAG identifier { $$ = astnode_create_datatype(USER_DATATYPE, $2, @$); }
    | '.' identifier { $$ = astnode_create_datatype(USER_DATATYPE, $2, @$); }
    ;

expression_list:
    extended_expression { $$ = $1; }
    | expression_list ',' extended_expression { $$ = $1; astnode_add_sibling($$, $3); }
    ;

incsrc_statement:
    INCSRC file_specifier line_tail { $$ = astnode_create_incsrc($2, @$); handle_incsrc($$); }
    ;

incbin_statement:
    INCBIN file_specifier line_tail { $$ = astnode_create_incbin($2, @$); handle_incbin($$); }
    ;

file_specifier:
    STRING_LITERAL { $$ = astnode_create_string($1, @$); }
    | '<' { $$ = astnode_create_file_path(scan_include('>'), @$); }
    ;

macro_decl_statement:
    MACRO identifier param_list_opt line_tail statement_list_opt ENDM line_tail { $$ = astnode_create_macro_decl($2, $3, $5, @$); }
    ;

param_list_opt:
    identifier_list { $$ = $1; }
    | { $$ = NULL; }
    ;

macro_statement:
    identifier arg_list_opt line_tail { $$ = astnode_create_macro($1, $2, @$); }
    ;

arg_list_opt:
    expression_list { $$ = $1; }
    | { $$ = NULL; }
    ;

identifier_list:
    identifier { $$ = $1; }
    | identifier_list ',' identifier { $$ = $1; astnode_add_sibling($$, $3); }
    ;

equ_statement:
    identifier EQU extended_expression line_tail { $$ = astnode_create_equ($1, $3, @$); }
    ;

assign_statement:
    identifier '=' extended_expression line_tail { $$ = astnode_create_assign($1, $3, @$); }
    ;

define_statement:
    DEFINE identifier line_tail { $$ = astnode_create_equ($2, astnode_create_integer(0, @$), @$); }
    | DEFINE identifier extended_expression line_tail { $$ = astnode_create_equ($2, $3, @$); }
    ;

public_statement:
    PUBLIC identifier_list line_tail { $$ = astnode_create_public($2, @$); }
    ;

extrn_statement:
    EXTRN identifier_list ':' symbol_type from_part_opt line_tail { $$ = astnode_create_extrn($2, $4, $5, @$); }
    ;

from_part_opt:
    '@' identifier { $$ = $2; }
    | { $$ = NULL; }
    ;

symbol_type:
    datatype { $$ = $1; }
    | identifier { $$ = astnode_create_datatype(USER_DATATYPE, $1, @$); }
    | PROC { $$ = astnode_create_integer(PROC_SYMBOL, @$); }
    | _LABEL_ { $$ = astnode_create_integer(LABEL_SYMBOL, @$); }
    ;

storage_statement:
    named_storage_statement { $$ = $1; }
    | unnamed_storage_statement { $$ = $1; }
    ;

named_storage_statement:
    identifier unnamed_storage_statement { $$ = astnode_create_var_decl(0, $1, $2, @$); }
    | ZEROPAGE identifier unnamed_storage_statement { $$ = astnode_create_var_decl(ZEROPAGE_FLAG, $2, $3, @$); }
    | PUBLIC identifier unnamed_storage_statement { $$ = astnode_create_var_decl(PUBLIC_FLAG, $2, $3, @$); }
    | ZEROPAGE PUBLIC identifier unnamed_storage_statement { $$ = astnode_create_var_decl(ZEROPAGE_FLAG | PUBLIC_FLAG, $3, $4, @$); }
    | PUBLIC ZEROPAGE identifier unnamed_storage_statement { $$ = astnode_create_var_decl(PUBLIC_FLAG | ZEROPAGE_FLAG, $3, $4, @$); }
    ;

unnamed_storage_statement:
    storage expression_opt line_tail { $$ = astnode_create_storage($1, $2, @$); }
    ;

storage:
    DSB { $$ = astnode_create_datatype(BYTE_DATATYPE, NULL, @$); }
    | DSW { $$ = astnode_create_datatype(WORD_DATATYPE, NULL, @$); }
    | DSD { $$ = astnode_create_datatype(DWORD_DATATYPE, NULL, @$); }
    ;
%%
/**
 * Takes care of switching to a new scanner input stream when a "incsrc" statement
 * has been encountered.
 * The current stream is pushed on a stack, and will be popped when EOF is reached
 * in the new stream.
 * @param n A node of type INCSRC_NODE
 */
void handle_incsrc(astnode *n)
{
    char errs[512];
    /* Get the node which describes the file to include */
    astnode *file = astnode_get_child(n, 0);
    int quoted_form = (astnode_get_type(file) == STRING_NODE) ? 1 : 0;
    switch (yypushandrestart(file->string, quoted_form)) {
        case 0:
        /* Success */
        break;
        case 1:
        /* Failed to open file */
        sprintf(errs, "could not open '%s' for reading", file->string);
        yyerror(errs);
        break;
        case 2:
        /* Stack overflow */
        yyerror("Maximum include nesting level reached");
        break;
    }
}

// TODO: This shouldn't be done here but rather in astproc module.

FILE *open_included_file(const char *, int, char **);

/**
 * Takes care of including the binary contents of the file specified by a parsed
 * "incbin" statement.
 * Calls yyerror() if the file can't be included for some reason.
 * @param n A node of type INCBIN_NODE
 */
void handle_incbin(astnode *n)
{
    FILE *fp;
    unsigned char *data;
    int size;
    char errs[512];
    /* Get the node which describes the file to include */
    astnode *file = astnode_get_child(n, 0);
    const char *filename = file->string;
    int quoted_form = (astnode_get_type(file) == STRING_NODE) ? 1 : 0;
    /* Try to open it */
    fp = open_included_file(filename, quoted_form, NULL);
    if (fp) {
        /* Get filesize */
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        rewind(fp);
        if (size > 0) {
            /* Allocate buffer to hold file contents */
            data = (unsigned char *)malloc(size);
            /* Read file contents into buffer */
            fread(data, 1, size, fp);
            /* Insert binary node */
            astnode_add_sibling(n, astnode_create_binary(data, size, n->loc) );
        }
        /* Close file */
        fclose(fp);
    } else {
        /* Couldn't open file */
        sprintf(errs, "could not open '%s' for reading", file->string);
        yyerror(errs);
    }
}
