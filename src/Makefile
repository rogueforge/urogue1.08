#
# Makefile for urogue
#

DISTNAME=urogue1.0.7

HDRS		  = dict.h dictutil.h rogue.h
OBJS		  = armor.o \
		artifact.o \
		bag.o \
		chase.o \
		command.o \
		daemon.o \
		daemons.o \
		dict.o \
		dictutil.o \
		encumb.o \
		fight.o \
		getplay.o \
		ident.o \
		init.o \
		io.o \
		list.o \
		magic.o \
		main.o \
		maze.o \
		memory.o \
		misc.o \
		monsdata.o \
		monsters.o \
		move.o \
		newlvl.o \
		options.o \
		pack.o \
		passages.o \
		player.o \
		potions.o \
		random.o \
		rings.o \
		rip.o \
		rogue.o \
		rooms.o \
		save.o \
		scrolls.o \
		status.o \
		sticks.o \
		things.o \
		trader.o \
		verify.o \
		vers.o \
		weapons.o \
		wizard.o

PROGRAM 	  = ur

CFILES		  = armor.c \
		artifact.c \
		bag.c \
		chase.c \
		command.c \
		daemon.c \
		daemons.c \
		dict.c \
		dictutil.c \
		encumb.c \
		fight.c \
		getplay.c \
		ident.c \
		init.c \
		io.c \
		list.c \
		magic.c \
		main.c \
		maze.c \
		memory.c \
		misc.c \
		monsdata.c \
		monsters.c \
		move.c \
		newlvl.c \
		options.c \
		pack.c \
		passages.c \
		player.c \
		potions.c \
		random.c \
		rings.c \
		rip.c \
		rogue.c \
		rooms.c \
		save.c \
		scrolls.c \
		status.c \
		sticks.c \
		things.c \
		trader.c \
		verify.c \
		vers.c \
		weapons.c \
		wizard.c

MISC=           Makefile README.TXT

CC    = gcc
CFLAGS= -O3
CRLIB = -lcurses
RM    = rm -f
TAR   = tar

urogue:	$(OBJS) $(MAKEFILE)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(CRLIB) -o $@

clean:		
	rm -f $(OBJS) urogue a.out core *.map

dist.src:
	make clean
	tar cf $(DISTNAME)-src.tar $(CFILES) $(HDRS) $(MISC)
	gzip -f $(DISTNAME)-src.tar

dist.irix:
	make clean
	make CC=cc CFLAGS="-woff 1116 -O3" urogue
	tar cf $(DISTNAME)-irix.tar urogue README.TXT
	gzip -f $(DISTNAME)-irix.tar

dist.aix:
	make clean
	make CC=xlc CFLAGS="-qmaxmem=16768 -O3 -qstrict" urogue
	tar cf $(DISTNAME)-aix.tar urogue README.TXT
	gzip -f $(DISTNAME)-aix.tar

dist.linux:
	make clean
	make urogue
	tar cf $(DISTNAME)-linux.tar urogue README.TXT
	gzip -f $(DISTNAME)-linux.tar

dist.interix:
	make clean
	make urogue
	tar cf $(DISTNAME)-interix.tar urogue README.TXT
	gzip -f $(DISTNAME)-interix.tar

dist.cygwin:
	make clean
	make urogue
	tar cf $(DISTNAME)-cygwin.tar urogue.exe README.TXT
	gzip -f $(DISTNAME)-cygwin.tar

dist.djgpp:
	make clean
	make LDFLAGS="-L$(DJDIR)/LIB" CRLIB="-lpdcurses" urogue
	rm -f $(DISTNAME)-djgpp.zip
	zip $(DISTNAME)-djgpp.zip urogue.exe README.TXT

