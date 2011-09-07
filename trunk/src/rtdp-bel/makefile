LDFLAGS =	#-rdynamic
#CFLAGS	=	-Wall -O3 -ffloat-store -pipe -DREVISION=\"$(REVISION)\" $(INCDIR)
CFLAGS	=	-Wall -O3 -m32 -march=pentium4 -msse -mfpmath=sse -msse2 -mmmx -pipe -DREVISION=\"$(REVISION)\" $(INCDIR)
CC	=	g++ $(CFLAGS)
LD	=	g++ $(LDFLAGS)

#
# don't touch beyond this point
#

REVISION=	2.00-cassandra
OBJS	=	Shell.o Problem.o StandardModel.o SB.o StandardPOMDP.o QBelief.o Quantization.o QMDP.o 
LIB	=	-lmdp -ldl -lm -lreadline -lcurses
LIBDIR	=	-L/usr/local/lib -L../lib
INCDIR	=	-I. -I../include -I/usr/local/include -I/opt/sfw/include

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

