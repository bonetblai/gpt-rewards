LDFLAGS =	#-rdynamic
ARCH	=	-msse -mfpmath=sse -msse2 -mmmx
CFLAGS	=	-Wall -O3 $(ARCH) -DREVISION=\"$(REVISION)\" $(INCDIR)
CC	=	g++ $(CFLAGS)
LD	=	g++ $(LDFLAGS)
RL_INC	=	-I/usr/local/opt/readline/include
RL_LIB	=	-L/usr/local/opt/readline/lib

#
# don't touch beyond this point
#

REVISION=	2.00
OBJS	=	Shell.o Problem.o StandardModel.o SB.o StandardPOMDP.o QBelief.o Quantization.o QMDP.o 
LIB	=	-lmdp -ldl -lm -lreadline -lcurses
LIBDIR	=	-L../lib $(RL_LIB)
INCDIR	=	-I. -I../include $(RL_INC)

.cc.o:
		$(CC) -c $<

compile:
		@echo "*** Compiling `pwd`"
		@make gpt

dep:
		for f in *.cc; do $(CC) -MM $$f; done > dependencies

clean:
		@echo "*** Cleaning `pwd`"
		@rm -f core *~ *.o gpt

gpt:		$(OBJS)
		$(LD) -o gpt $(OBJS) $(LIB) $(LIBDIR)

include dependencies

