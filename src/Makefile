# Copyright (C) 2014-2017 Hideaki Narita

PROJNAME=ikura
TARGETEXE=$(PROJNAME)
TARGETMO=$(PROJNAME).mo
PACKAGENAME=$(PROJNAME)
DEFAULTDOMAIN=$(PROJNAME)
LOCALES=ja_JP

PROJECTROOT=../

BINROOT=$(PROJECTROOT)bin/
BINDIR=$(BINROOT)$(PLATFORM)/$(CONFIGURATION)/
BINENGDIR=$(BINDIR)en_US/LC_MESSAGES/
BINJPNDIR=$(BINDIR)ja_JP/LC_MESSAGES/
OBJROOT=obj/
OBJDIR=$(OBJROOT)$(PLATFORM)/$(CONFIGURATION)/
LOCROOT=loc/
LOCENGDIR=$(LOCROOT)en_US/
LOCJPNDIR=$(LOCROOT)ja_JP/

PLATFORM=linux
CONFIGURATION=debug

DESTDIR=/usr/local/
DESTBINDIR=$(DESTDIR)bin/
DESTLOCDIR=$(DESTDIR)share/locale/
DESTENGDIR=$(DESTLOCDIR)en_US/LC_MESSAGES/
DESTJPNDIR=$(DESTLOCDIR)ja_JP/LC_MESSAGES/

######################################################################

COMPILE=$(CC) $(CFLAGS) $(CPPFLAGS) -c
LINK=$(LD) $(LDFLAGS)

CC=g++
CFLAGS=$(STDCFLAGS) $(USRCFLAGS) $(EXTCFLAGS)
CPPFLAGS=$(STDCPPFLAGS) $(USRCPPFLAGS) $(EXTCPPFLAGS)
LD=g++
LDFLAGS=$(STDLDFLAGS) $(USRLDFLAGS) $(EXTLDFLAGS)

STDCFLAGS=-Wall -Werror $(GTKMMCFLAGS)
STDCPPFLAGS=-DLINUX -D_GNU_SOURCE
STDLDFLAGS=
STDLIBS=$(GTKMMLIBS)

ifeq ($(CONFIGURATION), release)
USRCFLAGS=-O3
USRCPPFLAGS=
else
USRCFLAGS=-g
USRCPPFLAGS=-D_DEBUG
endif
USRLDFLAGS=

GTKMMCFLAGS=`pkg-config --cflags gtkmm-2.4`
GTKMMLIBS=`pkg-config --libs gtkmm-2.4`

######################################################################

POTMERGE=potmerge
POUPDATE=poupdate

######################################################################

RM=rm -f
RMALL=rm -fr
MKDIR=mkdir
MKDIRS=mkdir -p
INSTALL=install

######################################################################

$(OBJDIR)%.o: %.cc
	@test -d $(@D) || $(MKDIRS) $(@D)
	$(COMPILE) -o $@ $<

$(OBJDIR)%.pot: %.cc
	@test -d $(@D) || $(MKDIRS) $(@D)
	xgettext --package-name $(PACKAGENAME) --default-domain $(DEFAULTDOMAIN) --output $@ $<

######################################################################

all::

clean distclean::
	$(RM) *~

clean::
	$(RMALL) $(BINDIR)

clean::
	$(RMALL) $(OBJDIR)

distclean::
	$(RMALL) $(BINROOT)

distclean::
	$(RMALL) $(OBJROOT)

both:: release debug

release debug::
	$(MAKE) all CONFIGURATION=$@

release-clean::
	$(MAKE) clean CONFIGURATION=release

debug-clean::
	$(MAKE) clean CONFIGURATION=debug

######################################################################

PROJ1=$(BINDIR)$(TARGETEXE)
OBJS1=$(OBJDIR)Main.o \
$(OBJDIR)MainWindow.o \
$(OBJDIR)MainWindowClipboard.o \
$(OBJDIR)InputBuffer.o \
$(OBJDIR)HistoryBuffer.o \
$(OBJDIR)VariableStore.o \
$(OBJDIR)VariableDialog.o \
$(OBJDIR)Expression.o \
$(OBJDIR)Parser.o \
$(OBJDIR)Lexer.o \
$(OBJDIR)OperatorInfo.o \
$(OBJDIR)LocaleInfo.o \
$(OBJDIR)UTF8.o \
$(OBJDIR)Exception.o \
$(OBJDIR)SigfpeHandler.o
LIBS1=

$(PROJ1): $(OBJS1)
	@test -d $(BINDIR) || $(MKDIRS) $(BINDIR)
	$(LINK) -o $@ $(OBJS1) $(LIBS1) $(STDLIBS)
ifeq ($(CONFIGURATION), release)
	strip $(PROJ1)
endif

all:: $(PROJ1)

install::
	$(MAKE) bin-install CONFIGURATION=release

bin-install:: $(PROJ1)
	$(INSTALL) -m 755 $(PROJ1) $(DESTBINDIR)$(TARGETEXE)

######################################################################

POTFILE=$(OBJDIR)Ikura.pot

po::
	xgettext --package-name $(PACKAGENAME) --default-domain $(DEFAULTDOMAIN) --output $(POTFILE) `find . -name \*.cc | sort`

######################################################################

PROJ2=$(BINENGDIR)$(TARGETMO)
SRC2=$(LOCENGDIR)Ikura.po

$(PROJ2): $(SRC2)
	@test -d $(@D) || $(MKDIRS) $(@D)
	msgfmt --verbose --output-file $(PROJ2) $(SRC2)

all:: $(PROJ2)

po::
	$(POUPDATE) $(POTFILE) $(SRC2)

po-clean::
	$(RM) $(SRC2).backup

install::
	$(MAKE) eng-install CONFIGURATION=release

eng-install:: $(PROJ2)
	$(INSTALL) -m 644 $(PROJ2) $(DESTENGDIR)$(TARGETMO)

######################################################################

PROJ3=$(BINJPNDIR)$(TARGETMO)
SRC3=$(LOCJPNDIR)Ikura.po

$(PROJ3): $(SRC3)
	@test -d $(@D) || $(MKDIRS) $(@D)
	msgfmt --verbose --output-file $(PROJ3) $(SRC3)

all:: $(PROJ3)

po::
	$(POUPDATE) $(POTFILE) $(SRC3)

po-clean::
	$(RM) $(SRC3).backup

install::
	$(MAKE) jpn-install CONFIGURATION=release

jpn-install:: $(PROJ3)
	$(INSTALL) -m 644 $(PROJ3) $(DESTJPNDIR)$(TARGETMO)

######################################################################
