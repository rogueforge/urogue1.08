# SCCSID %W% %G%

COMPFLAGS	  = 

CFLAGS		  = $(COMPFLAGS)
PFLAGS		  = $(COMPFLAGS)
FFLAGS		  = $(COMPFLAGS)
CCFLAGS 	  = $(COMPFLAGS)
CXXFLAGS	  = $(COMPFLAGS)

DEST		  = /usr/games

# Uncomment this if you want system header files to be expanded
#
SYSHDRS 	  =

EXTHDRS 	  =

HDRS		  = dict.h dictutil.h rogue.h

INSTALL 	  = install

CC		  = cc

LD		  = cc

LDFLAGS 	  = $(COMPFLAGS) -L/usr/lib

LIBS		  = -lcurses

LINTLIBS	  =

LINTFLAGS	  = -Nn10000 $(CFLAGS)

MAKEFILE	  = Makefile

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

SRCS		  = armor.c \
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

all:		$(PROGRAM)

$(PROGRAM):	$(OBJS) $(MAKEFILE)
		$(LD) $(LDFLAGS) $(OBJS) $(LIBS) -o $(PROGRAM)
		@echo "done"

clean:		
		@rm -f $(OBJS) $(PROGRAM) core *.map

lint:		$(LINTLIBS) $(SRCS) $(HDRS) $(EXTHDRS)
		lint $(LINTFLAGS) $(LINTLIBS) $(SRCS)

tags:		$(HDRS) $(SRCS); @ctags $(HDRS) $(SRCS)

sccsget:
		sccs get $(HDRS) $(SRCS) ToDo License.Txt History.Txt

sccsedit:
		sccs edit $(HDRS) $(SRCS) ToDo License.Txt History.Txt

sccsdelta:
		sccs delta $(HDRS) $(SRCS) ToDo License.Txt History.Txt

###
armor.o: rogue.h
artifact.o: rogue.h
bag.o: rogue.h
chase.o: rogue.h
command.o: rogue.h
daemon.o: rogue.h
daemons.o: rogue.h
dict.o: dict.h dictutil.h
dictutil.o: dict.h dictutil.h
encumb.o: rogue.h
fight.o: rogue.h
getplay.o: rogue.h
ident.o: rogue.h
init.o: rogue.h
io.o: rogue.h
list.o: rogue.h 
magic.o: rogue.h
main.o: rogue.h
maze.o: rogue.h
memory.o: dict.h dictutil.h rogue.h
misc.o: rogue.h
monsdata.o: rogue.h
monsters.o: rogue.h
move.o: rogue.h
newlvl.o: rogue.h
options.o: rogue.h
pack.o: rogue.h
passages.o: rogue.h
player.o: rogue.h
potions.o: rogue.h
random.o: rogue.h
rings.o: rogue.h
rip.o: rogue.h
rogue.o: rogue.h
rooms.o: rogue.h
save.o: rogue.h
scrolls.o: rogue.h
status.o: rogue.h
sticks.o: rogue.h
things.o: rogue.h
trader.o: rogue.h
verify.o: rogue.h
weapons.o: rogue.h
wizard.o: rogue.h
