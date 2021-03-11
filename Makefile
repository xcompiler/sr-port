CC	= gcc
CFLAGS	= -g -O0 -Wall -Icommon
LD	= gcc
LDFLAGS	=
LIBS	= -Lcommon -lcommon -Ldis -ldis -lm -lGL -lSDL2
RANLIB	= ranlib
AR	= ar
POVRAY	= povray
PARTS	= dots/dots tunneli/tun10 glenz/glenz plzpart/plzpart visu/c/u2e \
	  lens/lens techno/koe

all: common-all dis-all $(PARTS)

include common/Makefile
include visu/c/Makefile
include dis/Makefile
include glenz/Makefile
include tunneli/Makefile
include techno/Makefile
include lens/Makefile
include plzpart-gl/Makefile
include dots/Makefile
include water/Makefile

.SUFFIXES: .c .o .a .pov .png

.pov.png:
	$(POVRAY) +W1024 +H768 $<

.c.o:
	$(CC) -c $(CFLAGS) -o $*.o $<

$(PARTS): common/libcommon.a dis/libdis.a

clean:
	rm -f core *~ $(OBJS) depend

depend:
	@echo Generating dependencies...
	@rm -f $@
	@for i in $(OBJS); do \
	  j=`echo $$i | sed 's/\.o$$/.c/'`; \
	  $(CC) -MM -MT $$i $(CFLAGS) $$j >> $@; \
	done
	

include depend
