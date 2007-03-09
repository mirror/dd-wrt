#=============================================================================
#
#    rules.mak
#
#    Generic rules for inclusion by all package makefiles.
#
#=============================================================================
#####ECOSGPLCOPYRIGHTBEGIN####
## -------------------------------------------
## This file is part of eCos, the Embedded Configurable Operating System.
## Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
##
## eCos is free software; you can redistribute it and/or modify it under
## the terms of the GNU General Public License as published by the Free
## Software Foundation; either version 2 or (at your option) any later version.
##
## eCos is distributed in the hope that it will be useful, but WITHOUT ANY
## WARRANTY; without even the implied warranty of MERCHANTABILITY or
## FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
## for more details.
##
## You should have received a copy of the GNU General Public License along
## with eCos; if not, write to the Free Software Foundation, Inc.,
## 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
##
## As a special exception, if other files instantiate templates or use macros
## or inline functions from this file, or you compile this file and link it
## with other works to produce a work based on this file, this file does not
## by itself cause the resulting work to be covered by the GNU General Public
## License. However the source code for this file must still be made available
## in accordance with section (3) of the GNU General Public License.
##
## This exception does not invalidate any other reasons why a work based on
## this file might be covered by the GNU General Public License.
##
## Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
## at http://sources.redhat.com/ecos/ecos-license/
## -------------------------------------------
#####ECOSGPLCOPYRIGHTEND####
#=============================================================================
#####DESCRIPTIONBEGIN####
#
# Author(s):     jld
# Contributors:  bartv
# Date:          1999-11-04
# Purpose:       Generic rules for inclusion by all package makefiles
# Description:   
#
#####DESCRIPTIONEND####
#=============================================================================

# FIXME: This definition belongs in the top-level makefile.
export HOST_CC := gcc

.PHONY: default build clean tests headers mlt_headers

# include any dependency rules generated previously
ifneq ($(wildcard *.deps),)
include $(wildcard *.deps)
endif

# GCC since 2.95 does -finit-priority by default so remove it from old HALs
CFLAGS := $(subst -finit-priority,,$(CFLAGS))

# -fvtable-gc is known to be broken in all recent GCC.
CFLAGS := $(subst -fvtable-gc,,$(CFLAGS))

# To support more recent GCC whilst preserving existing behaviour, we need
# to increase the inlining limit globally from the default 600. Note this
# will break GCC 2.95 based tools and earlier. You must use "make OLDGCC=1"
# to avoid this.
ifneq ($(OLDGCC),1)
CFLAGS := -finline-limit=7000 $(CFLAGS)
endif

# Separate C++ flags out from C flags.
ACTUAL_CFLAGS = $(CFLAGS)
ACTUAL_CFLAGS := $(subst -fno-rtti,,$(ACTUAL_CFLAGS))
ACTUAL_CFLAGS := $(subst -frtti,,$(ACTUAL_CFLAGS))
ACTUAL_CFLAGS := $(subst -Woverloaded-virtual,,$(ACTUAL_CFLAGS))
ACTUAL_CFLAGS := $(subst -fvtable-gc,,$(ACTUAL_CFLAGS))

ACTUAL_CXXFLAGS = $(CFLAGS)

# pattern matching rules to generate a library object from source code
# object filenames are prefixed to avoid name clashes
# a single dependency rule is generated (file extension = ".o.d")
%.o.d : %.c
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	$(CC) -c $(INCLUDE_PATH) -I$(dir $<) $(ACTUAL_CFLAGS) -Wp,-MD,$(@:.o.d=.tmp) -o $(dir $@)$(OBJECT_PREFIX)_$(notdir $(@:.o.d=.o)) $<
	@sed -e '/^ *\\/d' -e "s#.*: #$@: #" $(@:.o.d=.tmp) > $@
	@rm $(@:.o.d=.tmp)

%.o.d : %.cxx
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	$(CC) -c $(INCLUDE_PATH) -I$(dir $<) $(ACTUAL_CXXFLAGS) -Wp,-MD,$(@:.o.d=.tmp) -o $(dir $@)$(OBJECT_PREFIX)_$(notdir $(@:.o.d=.o)) $<
	@sed -e '/^ *\\/d' -e "s#.*: #$@: #" $(@:.o.d=.tmp) > $@
	@rm $(@:.o.d=.tmp)

%.o.d : %.cpp
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	$(CC) -c $(INCLUDE_PATH) -I$(dir $<) $(ACTUAL_CXXFLAGS) -Wp,-MD,$(@:.o.d=.tmp) -o $(dir $@)$(OBJECT_PREFIX)_$(notdir $(@:.o.d=.o)) $<
	@sed -e '/^ *\\/d' -e "s#.*: #$@: #" $(@:.o.d=.tmp) > $@
	@rm $(@:.o.d=.tmp)

%.o.d : %.S
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif	
	$(CC) -c $(INCLUDE_PATH) -I$(dir $<) $(ACTUAL_CFLAGS) -Wp,-MD,$(@:.o.d=.tmp) -o $(dir $@)$(OBJECT_PREFIX)_$(notdir $(@:.o.d=.o)) $<
	@sed -e '/^ *\\/d' -e "s#.*: #$@: #" $(@:.o.d=.tmp) > $@
	@rm $(@:.o.d=.tmp)

# pattern matching rules to generate a test object from source code
# object filenames are not prefixed
# a single dependency rule is generated (file extension = ".d")
%.d : %.c
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	$(CC) -c $(INCLUDE_PATH) -I$(dir $<) $(ACTUAL_CFLAGS) -Wp,-MD,$(@:.d=.tmp) -o $(@:.d=.o) $<
	@sed -e '/^ *\\/d' -e "s#.*: #$@: #" $(@:.d=.tmp) > $@
	@rm $(@:.d=.tmp)

%.d : %.cxx
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	$(CC) -c $(INCLUDE_PATH) -I$(dir $<) $(ACTUAL_CXXFLAGS) -Wp,-MD,$(@:.d=.tmp) -o $(@:.d=.o) $<
	@sed -e '/^ *\\/d' -e "s#.*: #$@: #" $(@:.d=.tmp) > $@
	@rm $(@:.d=.tmp)

%.d : %.cpp
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif
	$(CC) -c $(INCLUDE_PATH) -I$(dir $<) $(ACTUAL_CXXFLAGS) -Wp,-MD,$(@:.d=.tmp) -o $(@:.d=.o) $<
	@sed -e '/^ *\\/d' -e "s#.*: #$@: #" $(@:.d=.tmp) > $@
	@rm $(@:.d=.tmp)

%.d : %.S
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif	
	$(CC) -c $(INCLUDE_PATH) -I$(dir $<) $(ACTUAL_CFLAGS) -Wp,-MD,$(@:.d=.tmp) -o $(@:.d=.o) $<
	@sed -e '/^ *\\/d' -e "s#.*: #$@: #" $(@:.d=.tmp) > $@
	@rm $(@:.d=.tmp)

# rule to generate a test executable from object code
$(PREFIX)/tests/$(PACKAGE)/%$(EXEEXT): %.d $(wildcard $(PREFIX)/lib/target.ld) $(wildcard $(PREFIX)/lib/*.[ao])
ifeq ($(HOST),CYGWIN)
	@mkdir -p `cygpath -w "$(dir $@)" | sed "s@\\\\\\\\@/@g"`
else
	@mkdir -p $(dir $@)
endif	
ifneq ($(IGNORE_LINK_ERRORS),)
	-$(CC) -L$(PREFIX)/lib -Ttarget.ld -o $@ $(<:.d=.o) $(LDFLAGS)
else
	$(CC) -L$(PREFIX)/lib -Ttarget.ld -o $@ $(<:.d=.o) $(LDFLAGS) 
endif

# rule to generate all tests and create a dependency file "tests.deps" by
# concatenating the individual dependency rule files (file extension = ".d")
# generated during compilation
tests: tests.stamp

TESTS := $(TESTS:.cpp=)
TESTS := $(TESTS:.cxx=)
TESTS := $(TESTS:.c=)
TESTS := $(TESTS:.S=)
tests.stamp: $(foreach target,$(TESTS),$(target).d $(PREFIX)/tests/$(PACKAGE)/$(target)$(EXEEXT))
ifneq ($(strip $(TESTS)),)
	@cat $(TESTS:%=%.d) > $(@:.stamp=.deps)
endif
	@touch $@

# rule to clean the build tree
clean:
	@find . -type f -print | grep -v makefile | xargs rm -f

# rule to copy MLT files
mlt_headers: $(foreach x,$(MLT),$(PREFIX)/include/pkgconf/$(notdir $x))

$(foreach x,$(MLT),$(PREFIX)/include/pkgconf/$(notdir $x)): $(MLT)
	@cp $(dir $<)/$(notdir $@) $(PREFIX)/include/pkgconf
	@chmod u+w $(PREFIX)/include/pkgconf/$(notdir $@)

# end of file
