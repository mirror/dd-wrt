# -*- sh -*-
ELF=
EXECS_TO_BUILD = tm2txt sl2tm
EXECS_TO_INSTALL = tm2txt sl2tm
SYMLINK_DIR=$(HOME)/sys/$(ARCH)/objs/slang/doc/tm/tools
SRCDIR=..
OBJDIR=$(ARCH)objs#

SLANGDIR=../../../../src
EXECLIBS = -L$(SLANGDIR)/$(ARCH)$(ELF)objs -lslang -lm
EXECINC = -I$(SLANGDIR)

COMPILE_CMD=$(CC) -c $(CFLAGS) $(EXECINC)
LINK_CMD=$(CC) $(LDFLAGS)

all: $(OBJDIR) $(EXECS_TO_BUILD)
clean:
	$(RM) $(OBJDIR)/* *~ #*

tm2txt: $(OBJDIR)/tm2txt
	@echo tm2txt created in $(OBJDIR)
$(OBJDIR)/tm2txt: $(OBJDIR)/tm2txt.o $(EXEC_EXTRA_OBJS)
	cd $(OBJDIR); $(LINK_CMD) tm2txt.o -o tm2txt $(EXECLIBS)
$(OBJDIR)/tm2txt.o: tm2txt.c $(tm2txt_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(SRCDIR)/tm2txt.c -o tm2txt.o
sl2tm: $(OBJDIR)/sl2tm
	@echo sl2tm created in $(OBJDIR)
$(OBJDIR)/sl2tm: $(OBJDIR)/sl2tm.o $(EXEC_EXTRA_OBJS)
	cd $(OBJDIR); $(LINK_CMD) sl2tm.o -o sl2tm $(EXECLIBS)
$(OBJDIR)/sl2tm.o: sl2tm.c $(sl2tm_O_DEP)
	cd $(OBJDIR); $(COMPILE_CMD) $(SRCDIR)/sl2tm.c -o sl2tm.o

$(OBJDIR):
	-mkdir $(OBJDIR)

install: $(EXECS_TO_INSTALL)
	@for x in $(EXECS_TO_INSTALL); do \
	  echo Installing $$x in $(BINDIR); \
	  $(INSTALL_CMD) $(OBJDIR)/$$x $(BINDIR); \
	done

symlinks:
	-/bin/rm -f $(ARCH)objs
	-mkdir -p $(SYMLINK_DIR)
	ln -s $(SYMLINK_DIR) $(ARCH)objs

