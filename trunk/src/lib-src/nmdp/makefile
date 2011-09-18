# Copyright 1996,1997, Brown University, Providence, RI.
# 
#                         All Rights Reserved
# 
# Permission to use, copy, modify, and distribute this software and its
# documentation for any purpose other than its incorporation into a
# commercial product is hereby granted without fee, provided that the
# above copyright notice appear in all copies and that both that
# copyright notice and this permission notice appear in supporting
# documentation, and that the name of Brown University not be used in
# advertising or publicity pertaining to distribution of the software
# without specific, written prior permission.
# 
# BROWN UNIVERSITY DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
# INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR ANY
# PARTICULAR PURPOSE.  IN NO EVENT SHALL BROWN UNIVERSITY BE LIABLE FOR
# ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

######################################################################
# System specific settings - set these to suit your system's program
# name and flags.

# The compiler
CC = g++
#CC = /opt/SUNWspro/bin/cc

# For scanning
LEX = lex

# For parsing
YACC = yacc -d

# Removing files
RM = /bin/rm -f

# Archiver name and flags
AR = ar -crv

MKDIR = ../scripts/mkdir-if-noexist

CP = /bin/cp

# Flags specific to the machine architecture
SYS_FLAGS = -DSYSSOLARIS

######################################################################
# Comment or uncomment these as you need
#DEBUG_FLAGS = -g
#PROFILE_FLAGS = -pg
OPTIMIZE_FLAGS = -O3 -m32 -march=pentium4 -msse -mfpmath=sse -msse2 -mmmx -pipe

# Compile time flags
CFLAGS = $(DEBUG_FLAGS) $(PROFILE_FLAGS) $(OPTIMIZE_FLAGS) \
	$(SYS_FLAGS)

# Load time flags
CLDFLAGS =  $(DEBUG_FLAGS) $(PROFILE_FLAGS) $(OPTIMIZE_FLAGS)

# Extra load and include paths
INCLUDES = 
LIB_DIRS = 

# Extra libraries
GENERAL_LIBS = -lgen -lm -ldl 

CLDLIBS  = $(GENERAL_LIBS)

############################################################
LIB_NAME = mdp
LIB_OBJS =  mdp.o parser.tab.o imm-reward.o lex.yy.o \
		parse_err.o parse_hash.o sparse-matrix.o

all:
	@echo "*** Compiling `pwd`"
	@make compile

compile:	$(LIB_NAME)
$(LIB_NAME):	$(LIB_OBJS)
	@$(CP) *.h ../../include/mdp
	$(AR) ../../lib/lib$(LIB_NAME).a $(LIB_OBJS)

############################################################
DEBUG_BIN = debug
DEBUG_OBJS = debug.o sparse-matrix.o imm-reward.o mdp.o lex.yy.o \
		parse_err.o parse_hash.o parser.tab.o 

debug:	$(DEBUG_OBJS) 
	$(CC) $(LDFLAGS) $(DEBUG_OBJS) $(LDLIBS) -o $(DEBUG_BIN)

######################################################################
# Specifies the rules

.SUFFIXES: .o .c .y .l .h

%.o:	%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

# There are always some exceptions to the rules ;-)

lex.yy.c:	scanner.l parser.tab.c parse_err.h \
		parse_hash.h parse_constant.h
		$(LEX) scanner.l

parser.tab.c:	parser.y  parse_err.h mdp.h \
		parse_hash.h parse_constant.h sparse-matrix.h \
		imm-reward.h
		$(YACC) parser.y
		mv y.tab.c parser.tab.c
		mv y.tab.h parser.tab.h


######################################################################
#  Specifies the dependencies

# Name of the dependency file
#DEPEND = Makefile.depend

#dep $(DEPEND):
#$(CC) -E -M $(INCLUDES) *.[c] > $(DEPEND)
#$(MKDIR) ../../include
#$(MKDIR) ../../include/mdp
#$(CP) *.h ../../include/mdp

# See if a Dependency file exists and if so use it
#ifeq ($(DEPEND),$(wildcard $(DEPEND)))
#include $(DEPEND)
#endif

######################################################################
# Do this to clean up before a fresh make

clean:
	@echo "*** Cleaning `pwd`"
	@$(RM) *.o lex.yy.c y.tab.c parser.tab.c parser.tab.h *~

######################################################################

