## Makefile.am -- Process this file with automake to produce Makefile.in

bin_PROGRAMS = xasm xlnk
xasm_SOURCES = astnode.c astnode.h astproc.c astproc.h charmap.c charmap.h \
codegen.c codegen.h loc.h objdef.h opcode.c opcode.h parser.y scanner.l symtab.c \
symtab.h xasm.c my_getopt.c my_getopt.h getopt.h
xlnk_SOURCES = xlnk.c script.c script.h unit.c unit.h hashtab.c hashtab.h \
opcode.c opcode.h my_getopt.c my_getopt.h getopt.h

AM_CFLAGS = -Wall
AM_LFLAGS = -i
AM_YFLAGS = -d
