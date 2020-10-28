# Makefile
# This file is derived from Makefile.SH.  Any changes made here will
# be lost the next time you run Configure.
#  Makefile is used to generate makefile.  The only difference
#  is that makefile has the dependencies filled in at the end.

CC = i486-openwrt-linux-musl-gcc
LD = i486-openwrt-linux-musl-gcc

LDFLAGS = -L/home/xfs/openwrt/lede/openwrt/staging_dir/target-i386_pentium-mmx_musl/usr/lib -L/home/xfs/openwrt/lede/openwrt/staging_dir/target-i386_pentium-mmx_musl/lib -L/home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/usr/lib -L/home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/lib
CLDFLAGS = -L/home/xfs/openwrt/lede/openwrt/staging_dir/target-i386_pentium-mmx_musl/usr/lib -L/home/xfs/openwrt/lede/openwrt/staging_dir/target-i386_pentium-mmx_musl/lib -L/home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/usr/lib -L/home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/lib

mallocsrc = 
mallocobj = 
LNS = /bin/ln -s
# NOTE: some systems don't grok "cp -f". XXX Configure test needed?
CPS = cp
RMS = rm -f
RMS_R = rm -rf
ranlib = :

# The following are mentioned only to make metaconfig include the
# appropriate questions in Configure.  If you want to change these,
# edit config.sh instead, or specify --man1dir=/wherever on
# installman commandline.
bin = /usr/bin
scriptdir = /usr/bin
shrpdir = /usr/lib/perl5/5.28/CORE
privlib = /usr/lib/perl5/5.28
man1dir =  
man1ext = 0
man3dir =  
man3ext = 0

# The following are used to build and install shared libraries for
# dynamic loading.
LDDLFLAGS = -shared -L/home/xfs/openwrt/lede/openwrt/staging_dir/target-i386_pentium-mmx_musl/usr/lib -L/home/xfs/openwrt/lede/openwrt/staging_dir/target-i386_pentium-mmx_musl/lib -L/home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/usr/lib -L/home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/lib
SHRPLDFLAGS = $(LDDLFLAGS)
CCDLFLAGS = -fPIC -rdynamic -Wl,-rpath,/usr/lib/perl5/5.28/CORE
DLSUFFIX = .so
PLDLFLAGS = -fPIC
LIBPERL = libperl.so
LLIBPERL= $(LIBPERL)
LLIBPERL_NONSHR= 
SHRPENV = 

# Static targets are ordinarily built without CCCDLFLAGS.  However,
# if building a shared libperl.so that might later be linked into
# another application, then it might be appropriate to also build static
# extensions (usually just DynaLoader) with relocatable code (e.g. -fPIC
# for GNU cc).
STATIC_LDFLAGS = 

# The following is used to include the current directory in
# the dynamic loader path you are building a shared libperl.
LDLIBPTH = LD_LIBRARY_PATH=/home/xfs/openwrt/lede/openwrt/build_dir/target-i386_pentium-mmx_musl/perl/perl-5.28.1

# Sometimes running an executable is an adventure.
RUN = 

# When cross-compiling we want to use a (mini)perl built for the host, not the target
HOST_PERL = /usr/bin/perl
HOST_GENERATE = /usr/bin/generate_uudmap
SYSTEM_PERL = 

# These variables may need to be manually set for non-Unix systems.
AR = i486-openwrt-linux-musl-ar
HOST_EXE_EXT = 
EXE_EXT = 
LIB_EXT = .a
OBJ_EXT = .o
PATH_SEP = :

# Macros to invoke a copy of miniperl during the build.  Targets which
# are built using these macros should depend on $(MINIPERL_EXE)
MINIPERL_EXE = miniperl$(EXE_EXT)
MINIPERL = $(LDLIBPTH) ./miniperl$(EXE_EXT) -Ilib -I.

# Macros to invoke sort the MANIFEST during build
MANIFEST_SRT = MANIFEST.srt

# Macros to invoke a copy of our fully operational perl during the build.
PERL_EXE = perl$(EXE_EXT)
RUN_PERL = $(LDLIBPTH) $(RUN) ./miniperl$(EXE_EXT) -Ilib -I.
# Macros to run our tests
RUN_TESTS = $(LDLIBPTH) ./runtests

dynamic_ext =   lib/auto/B/B.so lib/auto/Compress/Raw/Bzip2/Bzip2.so lib/auto/Compress/Raw/Zlib/Zlib.so lib/auto/Cwd/Cwd.so lib/auto/DB_File/DB_File.so lib/auto/Data/Dumper/Dumper.so lib/auto/Devel/PPPort/PPPort.so lib/auto/Devel/Peek/Peek.so lib/auto/Digest/MD5/MD5.so lib/auto/Digest/SHA/SHA.so lib/auto/Encode/Encode.so lib/auto/Fcntl/Fcntl.so lib/auto/File/DosGlob/DosGlob.so lib/auto/File/Glob/Glob.so lib/auto/Filter/Util/Call/Call.so lib/auto/GDBM_File/GDBM_File.so lib/auto/Hash/Util/Util.so lib/auto/Hash/Util/FieldHash/FieldHash.so lib/auto/I18N/Langinfo/Langinfo.so lib/auto/IO/IO.so lib/auto/IPC/SysV/SysV.so lib/auto/List/Util/Util.so lib/auto/MIME/Base64/Base64.so lib/auto/Math/BigInt/FastCalc/FastCalc.so lib/auto/Opcode/Opcode.so lib/auto/POSIX/POSIX.so lib/auto/PerlIO/encoding/encoding.so lib/auto/PerlIO/mmap/mmap.so lib/auto/PerlIO/scalar/scalar.so lib/auto/PerlIO/via/via.so lib/auto/SDBM_File/SDBM_File.so lib/auto/Socket/Socket.so lib/auto/Storable/Storable.so lib/auto/Sys/Hostname/Hostname.so lib/auto/Sys/Syslog/Syslog.so lib/auto/Tie/Hash/NamedCapture/NamedCapture.so lib/auto/Time/HiRes/HiRes.so lib/auto/Time/Piece/Piece.so lib/auto/Unicode/Collate/Collate.so lib/auto/XS/APItest/APItest.so lib/auto/XS/Typemap/Typemap.so lib/auto/arybase/arybase.so lib/auto/attributes/attributes.so lib/auto/mro/mro.so lib/auto/re/re.so lib/auto/threads/threads.so lib/auto/threads/shared/shared.so
dynamic_ext_re = lib/auto/re/re.so
static_ext =  
nonxs_ext =   cpan/Archive-Tar/pm_to_blib dist/Attribute-Handlers/pm_to_blib cpan/AutoLoader/pm_to_blib cpan/B-Debug/pm_to_blib cpan/CPAN/pm_to_blib cpan/CPAN-Meta/pm_to_blib cpan/CPAN-Meta-Requirements/pm_to_blib cpan/CPAN-Meta-YAML/pm_to_blib dist/Carp/pm_to_blib cpan/Config-Perl-V/pm_to_blib dist/Devel-SelfStubber/pm_to_blib cpan/Digest/pm_to_blib dist/Dumpvalue/pm_to_blib dist/Env/pm_to_blib ext/Errno/pm_to_blib dist/Exporter/pm_to_blib dist/ExtUtils-CBuilder/pm_to_blib cpan/ExtUtils-Constant/pm_to_blib cpan/ExtUtils-Install/pm_to_blib cpan/ExtUtils-MakeMaker/pm_to_blib cpan/ExtUtils-Manifest/pm_to_blib ext/ExtUtils-Miniperl/pm_to_blib dist/ExtUtils-ParseXS/pm_to_blib cpan/File-Fetch/pm_to_blib ext/File-Find/pm_to_blib cpan/File-Path/pm_to_blib cpan/File-Temp/pm_to_blib ext/FileCache/pm_to_blib dist/Filter-Simple/pm_to_blib cpan/Getopt-Long/pm_to_blib cpan/HTTP-Tiny/pm_to_blib dist/I18N-Collate/pm_to_blib dist/I18N-LangTags/pm_to_blib cpan/IO-Compress/pm_to_blib cpan/IO-Socket-IP/pm_to_blib cpan/IO-Zlib/pm_to_blib cpan/IPC-Cmd/pm_to_blib ext/IPC-Open3/pm_to_blib cpan/JSON-PP/pm_to_blib cpan/Locale-Codes/pm_to_blib dist/Locale-Maketext/pm_to_blib cpan/Locale-Maketext-Simple/pm_to_blib cpan/Math-BigInt/pm_to_blib cpan/Math-BigRat/pm_to_blib cpan/Math-Complex/pm_to_blib cpan/Memoize/pm_to_blib dist/Module-CoreList/pm_to_blib cpan/Module-Load/pm_to_blib cpan/Module-Load-Conditional/pm_to_blib cpan/Module-Loaded/pm_to_blib cpan/Module-Metadata/pm_to_blib cpan/NEXT/pm_to_blib dist/Net-Ping/pm_to_blib cpan/Params-Check/pm_to_blib cpan/Perl-OSType/pm_to_blib cpan/PerlIO-via-QuotedPrint/pm_to_blib cpan/Pod-Checker/pm_to_blib cpan/Pod-Escapes/pm_to_blib ext/Pod-Functions/pm_to_blib ext/Pod-Html/pm_to_blib cpan/Pod-Parser/pm_to_blib cpan/Pod-Perldoc/pm_to_blib cpan/Pod-Simple/pm_to_blib cpan/Pod-Usage/pm_to_blib dist/Safe/pm_to_blib dist/Search-Dict/pm_to_blib dist/SelfLoader/pm_to_blib cpan/Term-ANSIColor/pm_to_blib cpan/Term-Cap/pm_to_blib dist/Term-Complete/pm_to_blib dist/Term-ReadLine/pm_to_blib dist/Test/pm_to_blib cpan/Test-Harness/pm_to_blib cpan/Test-Simple/pm_to_blib dist/Text-Abbrev/pm_to_blib cpan/Text-Balanced/pm_to_blib cpan/Text-ParseWords/pm_to_blib cpan/Text-Tabs/pm_to_blib dist/Thread-Queue/pm_to_blib dist/Thread-Semaphore/pm_to_blib dist/Tie-File/pm_to_blib ext/Tie-Memoize/pm_to_blib cpan/Tie-RefHash/pm_to_blib cpan/Time-Local/pm_to_blib dist/XSLoader/pm_to_blib cpan/autodie/pm_to_blib dist/autouse/pm_to_blib dist/base/pm_to_blib cpan/bignum/pm_to_blib dist/constant/pm_to_blib dist/encoding-warnings/pm_to_blib cpan/experimental/pm_to_blib dist/if/pm_to_blib dist/lib/pm_to_blib cpan/libnet/pm_to_blib cpan/parent/pm_to_blib cpan/perlfaq/pm_to_blib cpan/podlators/pm_to_blib dist/Unicode-Normalize/pm_to_blib cpan/version/pm_to_blib
ext = $(dynamic_ext) $(static_ext) $(nonxs_ext)
DYNALOADER = DynaLoader$(OBJ_EXT)

libs = -lpthread -ldl -lm -lcrypt -lutil -lc 

public = $(PERL_EXE) utilities

shellflags = 

# The /dev/tty or the moral equivalent.
devtty = /dev/tty

# This is set to  MAKE=make if your make command doesn't
# do it for you.
#

# Mention gmake here so it gets probed for by Configure.

# If you're going to use valgrind and it can't be invoked as plain valgrind
# then you'll need to change this, or override it on the make command line.
VALGRIND ?= valgrind
VG_TEST  ?= ./perl -e 1 2>/dev/null

DTRACE =  
DTRACE_H = 

DTRACE_PERLLIB_O =  # "dtrace -G" output for perllib_objs
DTRACE_MINI_O    =     # "dtrace -G" output for common and mini
DTRACE_MAIN_O    =     # "dtrace -G" output for perlmain.o

FIRSTMAKEFILE = makefile

# Any special object files needed by this architecture, e.g. os2/os2.obj
ARCHOBJS = 

.SUFFIXES: .c $(OBJ_EXT) .i .s

# grrr
SHELL = /bin/sh

# how to tr(anslate) newlines
TRNL = '\n'

OPTIMIZE = -O2

EXTRAS = 

INSTALLPREFIXEXP = /usr


CCCMD    = sh $(shellflags) cflags "optimize='$(OPTIMIZE)'" $@

CCCMDSRC = sh $(shellflags) cflags "optimize='$(OPTIMIZE)'" $<

CONFIGPM_FROM_CONFIG_SH = lib/Config.pm lib/Config_heavy.pl
CONFIGPM = $(CONFIGPM_FROM_CONFIG_SH) lib/Config_git.pl

CONFIGPOD = lib/Config.pod

CONFIGH = config.h

private = preplibrary $(CONFIGPM) $(CONFIGPOD) git_version.h lib/buildcustomize.pl

# Files to be built with variable substitution before miniperl
# is available.
sh = Makefile.SH cflags.SH config_h.SH makedepend.SH myconfig.SH runtests.SH pod/Makefile.SH

shextract = Makefile cflags config.h makedepend myconfig runtests pod/Makefile

addedbyconf = UU $(shextract)

# Unicode data files generated by mktables
unidatafiles = lib/unicore/Decomposition.pl lib/unicore/TestProp.pl \
	lib/unicore/CombiningClass.pl lib/unicore/Name.pl \
	lib/unicore/UCD.pl lib/unicore/Name.pm \
	lib/unicore/Heavy.pl lib/unicore/mktables.lst

# Directories of Unicode data files generated by mktables
unidatadirs = lib/unicore/To lib/unicore/lib

h1 = EXTERN.h INTERN.h XSUB.h av.h $(CONFIGH) cop.h cv.h dosish.h
h2 = embed.h form.h gv.h handy.h hv.h hv_func.h keywords.h mg.h op.h opcode.h
h3 = pad.h patchlevel.h perl.h perlapi.h perly.h pp.h proto.h regcomp.h
h4 = regexp.h scope.h sv.h unixish.h util.h iperlsys.h thread.h
h5 = utf8.h warnings.h mydtrace.h op_reg_common.h l1_char_class_tab.h
h6 = charclass_invlists.h
h = $(h1) $(h2) $(h3) $(h4) $(h5) $(h6)

c1 = av.c scope.c op.c doop.c doio.c dump.c gv.c hv.c mg.c reentr.c mro_core.c perl.c
c2 = perly.c pp.c pp_hot.c pp_ctl.c pp_sys.c regcomp.c regexec.c utf8.c sv.c
c3 = taint.c toke.c util.c deb.c run.c universal.c pad.c globals.c keywords.c
c4 = perlio.c perlapi.c numeric.c mathoms.c locale.c pp_pack.c pp_sort.c caretx.c dquote.c time64.c
c5 = $(mallocsrc)

c = $(c1) $(c2) $(c3) $(c4) $(c5) miniperlmain.c opmini.c perlmini.c

obj1 = $(mallocobj) gv$(OBJ_EXT) toke$(OBJ_EXT) perly$(OBJ_EXT) pad$(OBJ_EXT) regcomp$(OBJ_EXT) dump$(OBJ_EXT) util$(OBJ_EXT) mg$(OBJ_EXT) reentr$(OBJ_EXT) mro_core$(OBJ_EXT) keywords$(OBJ_EXT)
obj2 = hv$(OBJ_EXT) av$(OBJ_EXT) run$(OBJ_EXT) pp_hot$(OBJ_EXT) sv$(OBJ_EXT) pp$(OBJ_EXT) scope$(OBJ_EXT) pp_ctl$(OBJ_EXT) pp_sys$(OBJ_EXT)
obj3 = doop$(OBJ_EXT) doio$(OBJ_EXT) regexec$(OBJ_EXT) utf8$(OBJ_EXT) taint$(OBJ_EXT) deb$(OBJ_EXT) universal$(OBJ_EXT) globals$(OBJ_EXT) perlio$(OBJ_EXT) perlapi$(OBJ_EXT) numeric$(OBJ_EXT) mathoms$(OBJ_EXT) locale$(OBJ_EXT) pp_pack$(OBJ_EXT) pp_sort$(OBJ_EXT) caretx$(OBJ_EXT) dquote$(OBJ_EXT) time64$(OBJ_EXT)

# split the objects into 3 exclusive sets: those used by both miniperl and
# perl, and those used by just one or the other. Doesn't include the
# actual perl(mini)main.o, nor any dtrace objects.

common_objs    = $(obj1) $(obj2) $(obj3) $(ARCHOBJS)
mini_only_objs = opmini$(OBJ_EXT) perlmini$(OBJ_EXT)
main_only_objs = op$(OBJ_EXT)     perl$(OBJ_EXT)

miniperl_objs_nodt = $(mini_only_objs) $(common_objs) miniperlmain$(OBJ_EXT)
perllib_objs_nodt  = $(main_only_objs) $(common_objs)


miniperl_objs = $(miniperl_objs_nodt) $(DTRACE_MINI_O)
perllib_objs  = $(perllib_objs_nodt) $(DTRACE_PERLLIB_O)
perlmain_objs = perlmain$(OBJ_EXT) $(DTRACE_MAIN_O)

miniperl_dep = $(miniperl_objs)
perllib_dep = $(perllib_objs)
perlmain_dep = $(perlmain_objs)


perltoc_pod_prereqs = extra.pods pod/perl5281delta.pod pod/perlapi.pod pod/perlintern.pod pod/perlmodlib.pod pod/perluniprops.pod
generated_pods = pod/perltoc.pod $(perltoc_pod_prereqs)
generated_headers = uudmap.h bitcount.h mg_data.h

lintflags = \
    -b \
    -n \
    -p \
    -Ncheck=%all \
    -Nlevel=4 \
    -errchk=parentheses \
    -errhdr=%all \
    -errfmt=src \
    -errtags \
    -erroff=E_ASSIGN_NARROW_CONV \
    -erroff=E_BAD_PTR_CAST \
    -erroff=E_BAD_PTR_CAST_ALIGN \
    -erroff=E_BAD_PTR_INT_COMBINATION \
    -erroff=E_BAD_SIGN_EXTEND \
    -erroff=E_BLOCK_DECL_UNUSED \
    -erroff=E_CASE_FALLTHRU \
    -erroff=E_CONST_EXPR \
    -erroff=E_CONSTANT_CONDITION \
    -erroff=E_END_OF_LOOP_CODE_NOT_REACHED \
    -erroff=E_EQUALITY_NOT_ASSIGNMENT \
    -erroff=E_EXPR_NULL_EFFECT \
    -erroff=E_FALSE_LOGICAL_EXPR \
    -erroff=E_INCL_NUSD \
    -erroff=E_LOOP_EMPTY \
    -erroff=E_MAIN_PARAM \
    -erroff=E_POINTER_TO_OBJECT \
    -erroff=E_PTRDIFF_OVERFLOW \
    -erroff=E_SHIFT_CNT_NEG_TOO_BIG_L \
    -erroff=E_STATIC_UNUSED \
    -erroff=E_TRUE_LOGICAL_EXPR

.c$(OBJ_EXT): 
	@echo `$(CCCMD)` $(PLDLFLAGS) $*.c
	@`$(CCCMD)` $(PLDLFLAGS) $*.c

.c.i:	perl.h config.h
	@echo `$(CCCMDSRC)` -E $*.c \> $*.i
	@`$(CCCMDSRC)` -E $*.c > $*.i

.c.s:	perl.h config.h
	@echo `$(CCCMDSRC)` -S $*.c
	@`$(CCCMDSRC)` -S $*.c

all: $(FIRSTMAKEFILE) $(MINIPERL_EXE) $(generated_pods) $(private) $(unidatafiles) $(public) $(dynamic_ext) $(nonxs_ext) extras.make $(MANIFEST_SRT) common_build
	@echo " ";
	@echo "	Everything is up to date. Type '$(MAKE) test' to run test suite."

$(MANIFEST_SRT):	MANIFEST $(PERL_EXE)
	@$(RUN_PERL) Porting/manisort -q || (echo "WARNING: re-sorting MANIFEST"; \
	    $(RUN_PERL) Porting/manisort -q -o MANIFEST; sh -c true)
	@touch $(MANIFEST_SRT)

.PHONY: all utilities

# Both git_version.h and lib/Config_git.pl are built
# by make_patchnum.pl.
git_version.h: lib/Config_git.pl

lib/Config_git.pl: $(MINIPERL_EXE) make_patchnum.pl
	$(MINIPERL) make_patchnum.pl

# make sure that we recompile perl.c if the git version changes
perl$(OBJ_EXT): git_version.h

utils/Makefile: $(MINIPERL_EXE) $(CONFIGPM) utils/Makefile.PL
	$(MINIPERL) utils/Makefile.PL

utilities:	utils/Makefile $(MINIPERL_EXE) $(CONFIGPM) lib/auto/Cwd/Cwd$(DLSUFFIX) FORCE
	@echo " "; echo "	Making utilities"; cd utils; $(LDLIBPTH) $(MAKE) all


# This is now done by installman only if you actually want the man pages.
#	@echo " "; echo "	Making docs"; cd pod; $(MAKE) all;

# Phony target to force checking subdirectories.
# Apparently some makes require an action for the FORCE target.
.PHONY: FORCE
FORCE:
	@sh -c true

opmini.c: op.c
	$(RMS) opmini.c
	$(LNS) op.c opmini.c

opmini$(OBJ_EXT): opmini.c
	echo @`$(CCCMD)` $(PLDLFLAGS) -DPERL_IS_MINIPERL -DPERL_EXTERNAL_GLOB opmini.c
	@`$(CCCMD)` $(PLDLFLAGS) -DPERL_IS_MINIPERL -DPERL_EXTERNAL_GLOB opmini.c

perlmini.c: perl.c
	$(RMS) perlmini.c
	$(LNS) perl.c perlmini.c

perlmini$(OBJ_EXT): perlmini.c
	echo @`$(CCCMD)` $(PLDLFLAGS) -DPERL_IS_MINIPERL -DPERL_EXTERNAL_GLOB perlmini.c
	@`$(CCCMD)` $(PLDLFLAGS) -DPERL_IS_MINIPERL -DPERL_EXTERNAL_GLOB perlmini.c

globals$(OBJ_EXT): $(generated_headers)

uudmap.h mg_data.h: bitcount.h

generate_uudmap$(OBJ_EXT): mg_raw.h

bitcount.h: generate_uudmap$(HOST_EXE_EXT)
	./generate_uudmap$(HOST_EXE_EXT) $(generated_headers)

generate_uudmap$(HOST_EXE_EXT): generate_uudmap$(OBJ_EXT)
	@$(RMS) generate_uudmap$(HOST_EXE_EXT)
	$(LNS) $(HOST_GENERATE) generate_uudmap$(HOST_EXE_EXT)

perlmain.c: $(MINIPERL_EXE) ext/ExtUtils-Miniperl/pm_to_blib
	$(MINIPERL) -MExtUtils::Miniperl -e 'writemain(\"perlmain.c", @ARGV)' DynaLoader $(static_ext)

# The file ext.libs is a list of libraries that must be linked in
# for static extensions, e.g. -lm -lgdbm, etc.  The individual
# static extension Makefile's add to it.
ext.libs: $(static_ext)
	-@test -f ext.libs || touch ext.libs

$(LIBPERL): $& $(perllib_dep) $(DYNALOADER) $(LIBPERLEXPORT)
	$(RMS) $@
	$(LD) -o $@ $(SHRPLDFLAGS) $(perllib_objs) $(DYNALOADER) $(libs)

# How to build executables.

# The $& notation tells Sequent machines that it can do a parallel make,
# and is harmless otherwise.
# The miniperl -w -MExporter line is a basic cheap test to catch errors
# before make goes on to run preplibrary and then MakeMaker on extensions.
# This is very handy because later errors are often caused by miniperl
# build problems but that's not obvious to the novice.
# The Module used here must not depend on Config or any extensions.

# The seemingly cranky ordering of having $(MINIPERL_EXE) depend on
# lib/buildcustomize.pl despite the reality that lib/buildcustomize.pl needs
# miniperl to exist first permits us to
# a) have one rule generate both miniperl and lib/buildcustomize.pl
#    (so that lib/buildcustomize.pl is always available. This simplifies things)
# b) have the rest of the Makefile depend on the more obvious $(MINIPERL_EXE)

$(MINIPERL_EXE): lib/buildcustomize.pl

lib/buildcustomize.pl: $& $(miniperl_dep) write_buildcustomize.pl
	@$(RMS) miniperl.xok
	@$(RMS) $(MINIPERL_EXE)
	$(LNS) $(HOST_PERL) $(MINIPERL_EXE)
	$(LDLIBPTH) ./miniperl$(HOST_EXE_EXT) -w -Ilib -I. -Idist/Exporter/lib -MExporter -e '<?>' || sh -c 'echo >&2 Failed to build miniperl.  Please run make minitest; exit 1'
	$(MINIPERL) -f write_buildcustomize.pl 'osname' "linux"

$(PERL_EXE): $& $(perlmain_dep) $(LIBPERL) $(static_ext) ext.libs $(PERLEXPORT) write_buildcustomize.pl
	@$(RMS) miniperl.xok
	$(SHRPENV) $(CC) -o perl $(CLDFLAGS) $(CCDLFLAGS) $(perlmain_objs) $(static_ext) $(LLIBPERL) `cat ext.libs` $(libs)

# Microperl.  This is just a convenience thing if one happens to
# build also the full Perl and therefore the real big Makefile:
# usually one should manually explicitly issue the below command.

.PHONY: microperl
microperl:
	$(MAKE) -f Makefile.micro

.PHONY: makeppport
makeppport: $(MINIPERL_EXE) $(CONFIGPM) $(nonxs_ext)
	$(MINIPERL) mkppport


.PHONY: preplibrary common_build
preplibrary: $(MINIPERL_EXE) $(CONFIGPM) $(PREPLIBRARY_LIBPERL)

$(CONFIGPM_FROM_CONFIG_SH): $(CONFIGPOD)

$(CONFIGPOD): config.sh $(MINIPERL_EXE) configpm Porting/Glossary lib/Config_git.pl
	$(MINIPERL) configpm

unidatafiles $(unidatafiles) pod/perluniprops.pod: uni.data

uni.data: $(MINIPERL_EXE) $(CONFIGPM) lib/unicore/mktables $(nonxs_ext)
	$(MINIPERL) lib/unicore/mktables -C lib/unicore -P pod -maketest -makelist -p
#	Commented out so always runs, mktables looks at far more files than we
#	can in this makefile to decide if needs to run or not
#	touch uni.data

# $(PERL_EXE) and ext because pod_lib.pl needs Digest::MD5
# But also this ensures that all extensions are built before we try to scan
# them, which picks up Devel::PPPort's documentation.
pod/perltoc.pod: $(perltoc_pod_prereqs) $(PERL_EXE) $(ext) pod/buildtoc
	$(RUN_PERL) -f pod/buildtoc -q

pod/perlapi.pod: pod/perlintern.pod

pod/perlintern.pod: $(MINIPERL_EXE) autodoc.pl embed.fnc
	$(MINIPERL) autodoc.pl

pod/perlmodlib.pod: $(MINIPERL_EXE) pod/perlmodlib.PL MANIFEST
	$(MINIPERL) pod/perlmodlib.PL -q

pod/perl5281delta.pod: pod/perldelta.pod
	$(RMS) pod/perl5281delta.pod
	$(LNS) perldelta.pod pod/perl5281delta.pod

extra.pods: $(MINIPERL_EXE)
	-@test ! -f extra.pods || $(RMS) `cat extra.pods`
	@$(RMS) extra.pods
	-@for x in `grep -l '^=[a-z]' README.* | grep -v README.vms` ; do \
	    nx=`echo $$x | sed -e "s/README\.//"`; \
	    $(LNS) ../$$x "pod/perl"$$nx".pod" ; \
	    echo "pod/perl"$$nx".pod" >> extra.pods ; \
	done

extras.make: $(PERL_EXE)
	-@test ! -s extras.lst || PATH="`pwd`:${PATH}" PERL5LIB="`pwd`/lib" $(RUN_PERL) -MCPAN -e '@ARGV&&make(@ARGV)' `cat extras.lst`

extras.test: $(PERL_EXE)
	-@test ! -s extras.lst || PATH="`pwd`:${PATH}" PERL5LIB="`pwd`/lib" $(RUN_PERL) -MCPAN -e '@ARGV&&test(@ARGV)' `cat extras.lst`

extras.install: $(PERL_EXE)
	-@test ! -s extras.lst || PATH="`pwd`:${PATH}" PERL5LIB="`pwd`/lib" $(RUN_PERL) -MCPAN -e '@ARGV&&install(@ARGV)' `cat extras.lst`

.PHONY: install install-strip install-all install-verbose install-silent \
	no-install install.perl install.man install.html

# Set this to an empty string to avoid an attempt of rebuild before install
INSTALL_DEPENDENCE = all

no_install no-install: install-notify

install: install-all

dist/Storable/lib/Storable/Limit.pm : $(PERL_EXE) dist/Storable/stacksize lib/auto/Storable/Storable.so lib/auto/Cwd/Cwd.so
	cd dist/Storable ; $(LDLIBPTH) $(MAKE) lib/Storable/Limit.pm

lib/Storable/Limit.pm : dist/Storable/lib/Storable/Limit.pm
	test -d lib/Storable || mkdir lib/Storable
	cp dist/Storable/lib/Storable/Limit.pm lib/Storable/Limit.pm

install_all install-all: $(INSTALL_DEPENDENCE) installperl all installman
	$(HOST_PERL) installperl --destdir=$(DESTDIR)  $(INSTALLFLAGS) $(STRIPFLAGS)
	$(HOST_PERL) installman --destdir=$(DESTDIR) 
	-@test ! -s extras.lst || PATH="`pwd`:${PATH}" PERL5LIB="`pwd`/lib" $(RUN_PERL) -Ilib -MCPAN -e '@ARGV&&install(@ARGV)' `cat extras.lst`

install_notify install-notify: $(INSTALL_DEPENDENCE) installperl all installman
	$(HOST_PERL) installperl --destdir=$(DESTDIR) --notify $(INSTALLFLAGS) $(STRIPFLAGS)
	$(HOST_PERL) installman --destdir=$(DESTDIR) --notify

install_silent install-silent: $(INSTALL_DEPENDENCE) installperl all installman
	$(HOST_PERL) installperl --destdir=$(DESTDIR) --silent $(INSTALLFLAGS) $(STRIPFLAGS)
	$(HOST_PERL) installman --destdir=$(DESTDIR) --silent
	-@test ! -s extras.lst || PATH="`pwd`:${PATH}" PERL5LIB="`pwd`/lib" $(RUN_PERL) -Ilib -MCPAN -e '@ARGV&&install(@ARGV)' `cat extras.lst`

install_strip install-strip: $(INSTALL_DEPENDENCE) installperl all installman
	$(HOST_PERL) installperl --destdir=$(DESTDIR) --strip $(INSTALLFLAGS) $(STRIPFLAGS)
	$(HOST_PERL) installman --destdir=$(DESTDIR) --strip
	-@test ! -s extras.lst || PATH="`pwd`:${PATH}" PERL5LIB="`pwd`/lib" $(RUN_PERL) -Ilib -MCPAN -e '@ARGV&&install(@ARGV)' `cat extras.lst`

install_verbose install-verbose: $(INSTALL_DEPENDENCE) installperl all installman
	$(HOST_PERL) installperl --destdir=$(DESTDIR) --verbose $(INSTALLFLAGS) $(STRIPFLAGS)
	$(HOST_PERL) installman --destdir=$(DESTDIR) --verbose
	-@test ! -s extras.lst || PATH="`pwd`:${PATH}" PERL5LIB="`pwd`/lib" $(RUN_PERL) -Ilib -MCPAN -e '@ARGV&&install(@ARGV)' `cat extras.lst`
install.perl:	$(INSTALL_DEPENDENCE) installperl
	$(HOST_PERL) installperl --destdir=$(DESTDIR) $(INSTALLFLAGS) $(STRIPFLAGS)
	-@test ! -s extras.lst || $(MAKE) extras.install
install.man:	all installman
	$(HOST_PERL) installman --destdir=$(DESTDIR) $(INSTALLFLAGS)

# XXX Experimental. Hardwired values, but useful for testing.
# Eventually Configure could ask for some of these values.
install.html: all installhtml
	-@test -f README.vms && cd vms && $(LNS) ../README.vms README_vms.pod && cd ..
	$(HOST_PERL) installhtml   \
      --podroot=. --podpath=. --recurse  \
      --htmldir=$(privlib)/html   \
      --splithead=pod/perlipc     \
      --splititem=pod/perlfunc    \
      --ignore=Porting/Maintainers.pm,Porting/pumpkin.pod,Porting/repository.pod \
      --verbose

# I now supply perly.c with the kits, so the following section is
# used only if you force bison to run by saying
# 	make regen_perly
# You normally shouldn't remake perly.[ch].

.PHONY: regen_perly

run_byacc run-byacc:
	@echo "run_byacc is obsolete; try 'make regen_perly' instead"

# this outputs perly.h, perly.act and perly.tab
regen_perly regen-perly:
	perl regen_perly.pl

# We don't want to regenerate perly.c and perly.h, but they might
# appear out-of-date after a patch is applied or a new distribution is
# made.
perly.c: perly.y
	-@sh -c true

perly.h: perly.y
	-@sh -c true

SYM  = globvar.sym perlio.sym

SYMH = perlvars.h intrpvar.h

CHMOD_W = chmod +w

# The following files are generated automatically
#	embed.pl:	proto.h embed.h embedvar.h perlapi.h perlapi.c
#	opcode.pl:	opcode.h opnames.h pp_proto.h
#	regcomp.pl:	regnodes.h
#	warnings.pl:	warnings.h lib/warnings.pm
#	feature.pl:	feature.h lib/feature.pm
# The correct versions should be already supplied with the perl kit,
# in case you don't have perl available.
# To force them to be regenerated, run
#       perl regen.pl
# with your existing copy of perl
# (make regen_headers is kept for backwards compatibility)

.PHONY: regen_headers regen_all

regen:	FORCE
	-perl regen.pl
	-perl regen/uconfig_h.pl

regen_headers regen-headers:	FORCE
	-perl regen.pl -v
	-perl regen/uconfig_h.pl -v

regen_meta regen-meta:  META.yml META.json

META.yml:   FORCE
	PATH="`pwd`:${PATH}" PERL5LIB="`pwd`/lib" $(RUN_PERL) Porting/makemeta -y

META.json:   FORCE
	PATH="`pwd`:${PATH}" PERL5LIB="`pwd`/lib" $(RUN_PERL) Porting/makemeta -j


regen_all regen-all: regen regen_meta

.PHONY:	manisort manicheck

manisort:	FORCE
	@perl Porting/manisort -q || (echo "WARNING: re-sorting MANIFEST"; \
		perl Porting/manisort -q -o MANIFEST; sh -c true)

manicheck:	FORCE
	perl Porting/manicheck

# Extensions:
# Names added to $(dynamic_ext) or $(static_ext) or $(nonxs_ext) will
# automatically get built.  There should ordinarily be no need to change
# any of this part of makefile.
#
# The dummy dependency is a place holder in case $(dynamic_ext) or
# $(static_ext) is empty.
#
# DynaLoader may be needed for extensions that use Makefile.PL.

$(DYNALOADER):	$(MINIPERL_EXE) lib/buildcustomize.pl preplibrary FORCE $(nonxs_ext)
	$(MINIPERL) make_ext.pl $@ $(MAKE_EXT_ARGS) MAKE="$(MAKE)" LIBPERL_A=$(LIBPERL) LINKTYPE=static $(STATIC_LDFLAGS)

d_dummy $(dynamic_ext):	$(MINIPERL_EXE) lib/buildcustomize.pl preplibrary makeppport $(DYNALOADER) FORCE $(PERLEXPORT) $(LIBPERL)
	$(MINIPERL) make_ext.pl $@ $(MAKE_EXT_ARGS) MAKE="$(MAKE)" LIBPERL_A=$(LIBPERL) LINKTYPE=dynamic

s_dummy $(static_ext):	$(MINIPERL_EXE) lib/buildcustomize.pl preplibrary makeppport $(DYNALOADER) FORCE
	$(MINIPERL) make_ext.pl $@ $(MAKE_EXT_ARGS) MAKE="$(MAKE)" LIBPERL_A=$(LIBPERL) LINKTYPE=static $(STATIC_LDFLAGS)

n_dummy $(nonxs_ext):	$(MINIPERL_EXE) lib/buildcustomize.pl preplibrary FORCE
	$(MINIPERL) make_ext.pl $@ $(MAKE_EXT_ARGS) MAKE="$(MAKE)" LIBPERL_A=$(LIBPERL)

ext/Pod-Functions/pm_to_blib: cpan/Pod-Simple/pm_to_blib cpan/Pod-Escapes/pm_to_blib pod/perlfunc.pod
cpan/IO-Compress/pm_to_blib: dist/lib/pm_to_blib

lib/auto/Math/BigInt/FastCalc/FastCalc.so: lib/auto/List/Util/Util.so

.PHONY: printconfig
printconfig:
	@eval `$(RUN_PERL) -V:$(CONFIGVAR)`; echo $$$(CONFIGVAR)

.PHONY: clean _tidy _mopup _cleaner1 _cleaner2 \
	realclean _realcleaner clobber _clobber \
	distclean veryclean _verycleaner

clean:		realclean

realclean:	_realcleaner _mopup
	@echo "Note that '$(MAKE) realclean' does not delete config.sh or Policy.sh"

_clobber:
	@$(RMS) Cross/run-* Cross/to-* Cross/from-* Cross/mkdir
	$(RMS_R) host
	$(RMS) t/test_state
	$(RMS) config.sh cppstdin Policy.sh extras.lst
	$(RMS) $(MANIFEST_SRT)

clobber:	_realcleaner _mopup _clobber

distclean:	clobber

# Like distclean but also removes emacs backups and *.orig.
veryclean:	_verycleaner _mopup _clobber
	$(RMS) Obsolete Wanted

# Do not 'make _mopup' directly.
_mopup:
	$(RMS) *$(OBJ_EXT) *$(LIB_EXT) all perlmain.c opmini.c perlmini.c generate_uudmap$(EXE_EXT) $(generated_headers)
	-rmdir .depending
	-@test -f extra.pods && $(RMS) `cat extra.pods`
	-@test -f vms/README_vms.pod && $(RMS) vms/README_vms.pod
	$(RMS) perl.exp ext.libs $(generated_pods) uni.data opmini.o perlmini.o pod/roffitall
	$(RMS) perl.export perl.dll perl.libexp perl.map perl.def
	$(RMS) *perl.xok
	$(RMS) cygwin.c libperl*.def libperl*.dll cygperl*.dll *.exe.stackdump
	$(RMS) $(PERL_EXE) $(MINIPERL_EXE) $(LIBPERL) libperl.* microperl
	$(RMS) config.arch config.over $(DTRACE_H)

_cleaner1:
	-cd os2; $(RMS) Makefile
	-cd pod; $(LDLIBPTH) $(MAKE) $(CLEAN)
	-cd utils; $(LDLIBPTH) $(MAKE) $(CLEAN)
	-@if test -f $(MINIPERL_EXE) ; then \
	for x in $(DYNALOADER) $(dynamic_ext) $(static_ext) $(nonxs_ext) ; do \
	$(MINIPERL) make_ext.pl --target=$(CLEAN) $$x MAKE="$(MAKE)" ; \
	done ; \
	else \
	sh $(CLEAN).sh ; \
	fi
	$(RMS) realclean.sh veryclean.sh
	$(RMS) `grep -v ^# mkppport.lst | grep . | sed -e 's/$$/\/ppport.h/'`

# Dear POSIX, thanks for making the default to xargs to be
# run once if nothhing is passed in. It is such a great help.

# Some systems do not support "?", so keep these files separate.
_cleaner2:
	$(RMS) core.*perl.*.? t/core.perl.*.? .?*.c
	$(RMS) core *perl.core t/core t/*perl.core core.* t/core.*
	$(RMS) t/$(PERL_EXE) t/rantests
	$(RMS_R) t/tmp*
	$(RMS_R) $(addedbyconf)
	$(RMS) $(FIRSTMAKEFILE) $(FIRSTMAKEFILE).old makefile.old utils/Makefile
	$(RMS) $(private)
	$(RMS_R) $(unidatafiles) $(unidatadirs)
	$(RMS_R) lib/auto
	$(RMS) lib/.exists lib/*/.exists lib/*/*/.exists
	$(RMS) h2ph.man
	$(RMS_R) .config
	$(RMS) preload
	$(RMS) pod2htmd.tmp
	$(RMS_R) pod/perlfunc pod/perlipc
	-rmdir ext/B/lib
	$(RMS) so_locations $(LIBPERL_NONSHR) $(MINIPERL_NONSHR)
	-rmdir lib/version lib/threads lib/inc/ExtUtils lib/inc lib/encoding
	-rmdir lib/autodie/exception lib/autodie/Scope lib/autodie lib/XS
	-rmdir lib/Win32API lib/VMS lib/Unicode/Collate/Locale
	-rmdir lib/Unicode/Collate/CJK lib/Unicode/Collate lib/Tie/Hash
	-rmdir lib/Thread lib/Text lib/Test2/Util lib/Test2/Tools
	-rmdir lib/Test2/IPC/Driver lib/Test2/IPC lib/Test2/Hub/Interceptor
	-rmdir lib/Test2/Hub lib/Test2/Formatter lib/Test2/EventFacet
	-rmdir lib/Test2/Event/TAP lib/Test2/Event lib/Test2/API lib/Test2
	-rmdir lib/Test/use lib/Test/Tester lib/Test/Builder/Tester
	-rmdir lib/Test/Builder/IO lib/Test/Builder lib/Test lib/Term
	-rmdir lib/TAP/Parser/YAMLish lib/TAP/Parser/SourceHandler
	-rmdir lib/TAP/Parser/Scheduler lib/TAP/Parser/Result
	-rmdir lib/TAP/Parser/Iterator lib/TAP/Parser lib/TAP/Harness
	-rmdir lib/TAP/Formatter/File lib/TAP/Formatter/Console
	-rmdir lib/TAP/Formatter lib/TAP lib/Sys/Syslog lib/Sys lib/Sub
	-rmdir lib/Search lib/Scalar lib/Pod/Text lib/Pod/Simple
	-rmdir lib/Pod/Perldoc lib/PerlIO/via lib/PerlIO lib/Perl
	-rmdir lib/Parse/CPAN lib/Parse lib/Params lib/Net/FTP lib/Module/Load
	-rmdir lib/Module/CoreList lib/Module lib/Memoize lib/Math/BigInt
	-rmdir lib/Math/BigFloat lib/Math lib/MIME lib/Locale/Maketext
	-rmdir lib/Locale/Codes lib/Locale lib/List/Util lib/List lib/JSON/PP
	-rmdir lib/JSON lib/IPC lib/IO/Uncompress/Adapter lib/IO/Uncompress
	-rmdir lib/IO/Socket lib/IO/Compress/Zlib lib/IO/Compress/Zip
	-rmdir lib/IO/Compress/Gzip lib/IO/Compress/Base
	-rmdir lib/IO/Compress/Adapter lib/IO/Compress lib/IO
	-rmdir lib/I18N/LangTags lib/I18N lib/Hash/Util lib/Hash lib/HTTP
	-rmdir lib/Filter/Util lib/Filter lib/File/Spec lib/ExtUtils/Typemaps
	-rmdir lib/ExtUtils/ParseXS lib/ExtUtils/MakeMaker/version
	-rmdir lib/ExtUtils/MakeMaker lib/ExtUtils/Liblist
	-rmdir lib/ExtUtils/Constant lib/ExtUtils/Command
	-rmdir lib/ExtUtils/CBuilder/Platform/Windows
	-rmdir lib/ExtUtils/CBuilder/Platform lib/ExtUtils/CBuilder
	-rmdir lib/Exporter lib/Encode/Unicode lib/Encode/MIME/Header
	-rmdir lib/Encode/MIME lib/Encode/KR lib/Encode/JP lib/Encode/CN
	-rmdir lib/Encode lib/Digest lib/Devel lib/Data lib/Config/Perl
	-rmdir lib/Compress/Raw lib/Compress lib/Carp lib/CPAN/Plugin
	-rmdir lib/CPAN/Meta/History lib/CPAN/Meta lib/CPAN/LWP
	-rmdir lib/CPAN/Kwalify lib/CPAN/HTTP lib/CPAN/FTP lib/CPAN/Exception
	-rmdir lib/CPAN/API lib/CPAN lib/Attribute lib/Archive/Tar lib/Archive
	-rmdir lib/App/Prove/State/Result lib/App/Prove/State lib/App/Prove
	-rmdir lib/App lib/Amiga

_realcleaner:
	@$(LDLIBPTH) $(MAKE) _cleaner1 CLEAN=distclean
	@$(LDLIBPTH) $(MAKE) _cleaner2

_verycleaner:
	@$(LDLIBPTH) $(MAKE) _cleaner1 CLEAN=veryclean
	@$(LDLIBPTH) $(MAKE) _cleaner2
	$(RMS) *~ *.orig */*~ */*.orig */*/*~ */*/*.orig

.PHONY: lint
lint: $(c)
	$(RMS) *.ln
	lint $(lintflags) -DPERL_CORE -D_REENTRANT -DDEBUGGING -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 $(c)

cscopeflags = -Rb  # Recursive, build-only.

.PHONY:	cscope
# To query the cscope.out "cscope -dLnsymbol" where n = 0 means uses,
# 1 = definitions, 2 = callees, 3 = callers, for example
# "cscope -dL1Perl_mg_set" or run cscope interactively (no arguments).
cscope.out cscope: $(c) $(h)
	cscope $(cscopeflags)

# Need to unset during recursion to go out of loop.
# The README below ensures that the dependency list is never empty and
# that when MAKEDEPEND is empty $(FIRSTMAKEFILE) doesn't need rebuilding.

MAKEDEPEND = Makefile makedepend

$(FIRSTMAKEFILE):	README $(MAKEDEPEND)
	$(MAKE) depend MAKEDEPEND=

Makefile: Makefile.SH config.sh
	$(SHELL) Makefile.SH

cflags: cflags.SH config.sh
	$(SHELL) cflags.SH

config.h: config_h.SH config.sh
	$(SHELL) config_h.SH

makedepend: makedepend.SH config.sh
	$(SHELL) makedepend.SH

myconfig: myconfig.SH config.sh
	$(SHELL) myconfig.SH

runtests: runtests.SH config.sh
	$(SHELL) runtests.SH

pod/Makefile: pod/Makefile.SH config.sh
	$(SHELL) pod/Makefile.SH

.PHONY: depend
depend: makedepend $(DTRACE_H) $(generated_headers)
	sh ./makedepend MAKE="$(MAKE)" cflags

.PHONY: test check test_prep test_prep_nodll test_prep_pre \
	test_prep_reonly test_tty test-tty test_notty test-notty \
	test_harness test_harness_notty minitest test-reonly _test

# The _test target is there just for Test::Smoke, which does a make test_prep
# always before invoking this target, thus preventing checking again
_test:
	$(RUN_TESTS) choose

# Cannot delegate rebuilding of t/perl to make
# to allow interlaced test and minitest

# Architecture-neutral stuff:

test_prep_pre: preplibrary utilities $(nonxs_ext)
test_prep test-prep: test_prep_pre $(MINIPERL_EXE) $(unidatafiles) $(PERL_EXE) \
	$(dynamic_ext) $(TEST_PERL_DLL) runtests $(generated_pods) common_build
	cd t && ($(RMS) $(PERL_EXE); $(LNS) ../$(PERL_EXE) $(PERL_EXE))

test_prep_reonly: $(MINIPERL_EXE) $(PERL_EXE) $(dynamic_ext_re) $(TEST_PERL_DLL)
	$(MINIPERL) make_ext.pl $(dynamic_ext_re) MAKE="$(MAKE)" LIBPERL_A=$(LIBPERL) LINKTYPE=dynamic
	cd t && ($(RMS) $(PERL_EXE); $(LNS) ../$(PERL_EXE) $(PERL_EXE))

test check: test_prep
	$(RUN_TESTS) choose

test_tty test-tty: test_prep
	$(RUN_TESTS) tty

test_notty test-notty: test_prep
	$(RUN_TESTS) no-tty

# Targets for valgrind testing:

test.valgrind check.valgrind:	test_prep
	@echo "Checking usemymalloc='n' in config.sh..."
	@grep "^usemymalloc="    config.sh
	@grep "^usemymalloc='n'" config.sh >/dev/null || exit 1
	@echo "And of course you have to have valgrind..."
	$(VALGRIND) $(VG_TEST) || exit 1
	PERL_VALGRIND=1 VALGRIND='$(VALGRIND)' TESTFILE=harness $(RUN_TESTS) choose

# minitest can't depend on lib/Config.pm because that might be where miniperl
# is crashing.
minitest_prep:
	-@test -f lib/Config.pm || $(MAKE) lib/Config.pm $(unidatafiles)
	@echo " "
	@echo "You may see some irrelevant test failures if you have been unable"
	@echo "to build lib/Config.pm, or the Unicode data files."
	@echo " "
	- cd t && ($(RMS) $(PERL_EXE); $(LNS) ../$(MINIPERL_EXE) $(PERL_EXE))

MINITEST_TESTS = base/*.t comp/*.t cmd/*.t run/*.t io/*.t re/*.t opbasic/*.t op/*.t uni/*.t perf/*.t

minitest: $(MINIPERL_EXE) minitest_prep
	- cd t && $(RUN_PERL) TEST $(MINITEST_TESTS) <$(devtty)

minitest-notty minitest_notty: $(MINIPERL_EXE) minitest_prep
	- cd t && PERL_SKIP_TTY_TEST=1 $(RUN_PERL) TEST $(MINITEST_TESTS)

# Test via harness

test_harness test-harness: test_prep
	TESTFILE=harness $(RUN_TESTS) choose

test_harness_notty: test_prep
	HARNESS_NOTTY=1 TESTFILE=harness $(RUN_TESTS) choose

test_reonly test-reonly: test_prep_reonly
	TEST_ARGS='-re \bre\/' TESTFILE=harness $(RUN_TESTS) choose


# Porting tests (well-formedness of pod, manifest, etc)

test_porting test-porting: test_prep
	cd t && $(RUN_PERL) harness porting/*.t ../lib/diagnostics.t


# intended as a common way to add dependencies to test-prep and all
common_build:  lib/Storable/Limit.pm


# Handy way to run perlbug -ok without having to install and run the
# installed perlbug. We don't re-run the tests here - we trust the user.
# Please *don't* use this unless all tests pass.
# If you want to report test failures, use "make nok" instead.

.PHONY: ok okfile oknack okfilenack nok nokfile noknack nokfilenack

ok:	utilities
	$(RUN_PERL) utils/perlbug -ok -s '(UNINSTALLED)'

okfile:	utilities
	$(RUN_PERL) utils/perlbug -ok -s '(UNINSTALLED)' -F perl.ok

oknack:	utilities
	$(RUN_PERL) utils/perlbug -ok -s '(UNINSTALLED)' -A

okfilenack:	utilities
	$(RUN_PERL) utils/perlbug -ok -s '(UNINSTALLED)' -F perl.ok -A

nok:	utilities
	$(RUN_PERL) utils/perlbug -nok -s '(UNINSTALLED)'

nokfile:	utilities
	$(RUN_PERL) utils/perlbug -nok -s '(UNINSTALLED)' -F perl.nok

noknack:	utilities
	$(RUN_PERL) utils/perlbug -nok -s '(UNINSTALLED)' -A

nokfilenack:	utilities
	$(RUN_PERL) utils/perlbug -nok -s '(UNINSTALLED)' -F perl.nok -A

.PHONY: clist hlist

clist:	$(c)
	echo $(c) | tr ' ' $(TRNL) >.clist

hlist:  $(h)
	echo $(h) | tr ' ' $(TRNL) >.hlist

.PHONY: distcheck
distcheck: FORCE
	perl '-MExtUtils::Manifest=&fullcheck' -e 'fullcheck()'

.PHONY: ctags

TAGS: $(c1) $(c2) $(c3) $(c4) $(c5) $(h)
	etags $(c1) $(c2) $(c3) $(c4) $(c5) $(h)

ctags:
	ctags -f Tags -N --totals --languages=c --langmap=c:+.h --exclude=opmini.c --exclude=perlmini.c *.c *.h

# AUTOMATICALLY GENERATED MAKE DEPENDENCIES--PUT NOTHING BELOW THIS LINE
# If this runs make out of memory, delete /usr/include lines.
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/alloca.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/arpa/inet.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/assert.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/alltypes.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/errno.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/fcntl.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/float.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl_fix.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipc.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipcstat.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/limits.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/posix.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/resource.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/sem.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/setjmp.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/signal.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/socket.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stat.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stdint.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/crypt.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/ctype.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/dirent.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/endian.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/errno.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fcntl.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/features.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/float.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdio.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdlib.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/string.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/strings.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/select.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/socket.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/unistd.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/grp.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/inttypes.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/limits.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/locale.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/math.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netdb.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netinet/in.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pthread.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pwd.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sched.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/setjmp.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/shadow.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/signal.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdarg.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdbool.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdc-predef.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stddef.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdint.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdio.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdlib.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/string.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/strings.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/cdefs.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/file.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/glibc-types.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ioctl.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ipc.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/param.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/resource.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/select.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/sem.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/socket.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/stat.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/time.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/times.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/types.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/wait.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/time.h
av$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/unistd.h
av$(OBJ_EXT): EXTERN.h
av$(OBJ_EXT): av.c
av$(OBJ_EXT): av.h
av$(OBJ_EXT): charclass_invlists.h
av$(OBJ_EXT): config.h
av$(OBJ_EXT): cop.h
av$(OBJ_EXT): cv.h
av$(OBJ_EXT): embed.h
av$(OBJ_EXT): embedvar.h
av$(OBJ_EXT): form.h
av$(OBJ_EXT): gv.h
av$(OBJ_EXT): handy.h
av$(OBJ_EXT): hv.h
av$(OBJ_EXT): hv_func.h
av$(OBJ_EXT): hv_macro.h
av$(OBJ_EXT): inline.h
av$(OBJ_EXT): intrpvar.h
av$(OBJ_EXT): iperlsys.h
av$(OBJ_EXT): mg.h
av$(OBJ_EXT): mg_vtable.h
av$(OBJ_EXT): mydtrace.h
av$(OBJ_EXT): op.h
av$(OBJ_EXT): op_reg_common.h
av$(OBJ_EXT): opcode.h
av$(OBJ_EXT): opnames.h
av$(OBJ_EXT): overload.h
av$(OBJ_EXT): pad.h
av$(OBJ_EXT): parser.h
av$(OBJ_EXT): patchlevel.h
av$(OBJ_EXT): perl.h
av$(OBJ_EXT): perlio.h
av$(OBJ_EXT): perlvars.h
av$(OBJ_EXT): perly.h
av$(OBJ_EXT): pp.h
av$(OBJ_EXT): proto.h
av$(OBJ_EXT): reentr.h
av$(OBJ_EXT): regcharclass.h
av$(OBJ_EXT): regexp.h
av$(OBJ_EXT): sbox32_hash.h
av$(OBJ_EXT): scope.h
av$(OBJ_EXT): sv.h
av$(OBJ_EXT): thread.h
av$(OBJ_EXT): unicode_constants.h
av$(OBJ_EXT): unixish.h
av$(OBJ_EXT): utf8.h
av$(OBJ_EXT): util.h
av$(OBJ_EXT): warnings.h
av$(OBJ_EXT): zaphod32_hash.h
av$(OBJ_EXT): cflags
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/alloca.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/arpa/inet.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/assert.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/alltypes.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/errno.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/fcntl.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/float.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl_fix.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipc.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipcstat.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/limits.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/posix.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/resource.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/sem.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/setjmp.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/signal.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/socket.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stat.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stdint.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/crypt.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/ctype.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/dirent.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/endian.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/errno.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fcntl.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/features.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/float.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdio.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdlib.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/string.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/strings.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/select.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/socket.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/unistd.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/grp.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/inttypes.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/limits.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/locale.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/math.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netdb.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netinet/in.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pthread.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pwd.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sched.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/setjmp.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/shadow.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/signal.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdarg.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdbool.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdc-predef.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stddef.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdint.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdio.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdlib.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/string.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/strings.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/cdefs.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/file.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/glibc-types.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ioctl.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ipc.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/param.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/resource.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/select.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/sem.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/socket.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/stat.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/time.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/times.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/types.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/wait.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/time.h
scope$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/unistd.h
scope$(OBJ_EXT): EXTERN.h
scope$(OBJ_EXT): av.h
scope$(OBJ_EXT): charclass_invlists.h
scope$(OBJ_EXT): config.h
scope$(OBJ_EXT): cop.h
scope$(OBJ_EXT): cv.h
scope$(OBJ_EXT): embed.h
scope$(OBJ_EXT): embedvar.h
scope$(OBJ_EXT): form.h
scope$(OBJ_EXT): gv.h
scope$(OBJ_EXT): handy.h
scope$(OBJ_EXT): hv.h
scope$(OBJ_EXT): hv_func.h
scope$(OBJ_EXT): hv_macro.h
scope$(OBJ_EXT): inline.h
scope$(OBJ_EXT): intrpvar.h
scope$(OBJ_EXT): iperlsys.h
scope$(OBJ_EXT): mg.h
scope$(OBJ_EXT): mg_vtable.h
scope$(OBJ_EXT): mydtrace.h
scope$(OBJ_EXT): op.h
scope$(OBJ_EXT): op_reg_common.h
scope$(OBJ_EXT): opcode.h
scope$(OBJ_EXT): opnames.h
scope$(OBJ_EXT): overload.h
scope$(OBJ_EXT): pad.h
scope$(OBJ_EXT): parser.h
scope$(OBJ_EXT): patchlevel.h
scope$(OBJ_EXT): perl.h
scope$(OBJ_EXT): perlio.h
scope$(OBJ_EXT): perlvars.h
scope$(OBJ_EXT): perly.h
scope$(OBJ_EXT): pp.h
scope$(OBJ_EXT): proto.h
scope$(OBJ_EXT): reentr.h
scope$(OBJ_EXT): regcharclass.h
scope$(OBJ_EXT): regexp.h
scope$(OBJ_EXT): sbox32_hash.h
scope$(OBJ_EXT): scope.c
scope$(OBJ_EXT): scope.h
scope$(OBJ_EXT): sv.h
scope$(OBJ_EXT): thread.h
scope$(OBJ_EXT): unicode_constants.h
scope$(OBJ_EXT): unixish.h
scope$(OBJ_EXT): utf8.h
scope$(OBJ_EXT): util.h
scope$(OBJ_EXT): warnings.h
scope$(OBJ_EXT): zaphod32_hash.h
scope$(OBJ_EXT): cflags
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/alloca.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/arpa/inet.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/assert.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/alltypes.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/errno.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/fcntl.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/float.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl_fix.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipc.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipcstat.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/limits.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/posix.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/resource.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/sem.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/setjmp.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/signal.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/socket.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stat.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stdint.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/crypt.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/ctype.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/dirent.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/endian.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/errno.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fcntl.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/features.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/float.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdio.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdlib.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/string.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/strings.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/select.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/socket.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/unistd.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/grp.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/inttypes.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/limits.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/locale.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/math.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netdb.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netinet/in.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pthread.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pwd.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sched.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/setjmp.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/shadow.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/signal.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdarg.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdbool.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdc-predef.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stddef.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdint.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdio.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdlib.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/string.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/strings.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/cdefs.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/file.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/glibc-types.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ioctl.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ipc.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/param.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/resource.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/select.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/sem.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/socket.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/stat.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/time.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/times.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/types.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/wait.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/time.h
op$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/unistd.h
op$(OBJ_EXT): EXTERN.h
op$(OBJ_EXT): XSUB.h
op$(OBJ_EXT): av.h
op$(OBJ_EXT): charclass_invlists.h
op$(OBJ_EXT): config.h
op$(OBJ_EXT): cop.h
op$(OBJ_EXT): cv.h
op$(OBJ_EXT): embed.h
op$(OBJ_EXT): embedvar.h
op$(OBJ_EXT): feature.h
op$(OBJ_EXT): form.h
op$(OBJ_EXT): gv.h
op$(OBJ_EXT): handy.h
op$(OBJ_EXT): hv.h
op$(OBJ_EXT): hv_func.h
op$(OBJ_EXT): hv_macro.h
op$(OBJ_EXT): inline.h
op$(OBJ_EXT): intrpvar.h
op$(OBJ_EXT): iperlsys.h
op$(OBJ_EXT): keywords.h
op$(OBJ_EXT): mg.h
op$(OBJ_EXT): mg_vtable.h
op$(OBJ_EXT): mydtrace.h
op$(OBJ_EXT): op.c
op$(OBJ_EXT): op.h
op$(OBJ_EXT): op_reg_common.h
op$(OBJ_EXT): opcode.h
op$(OBJ_EXT): opnames.h
op$(OBJ_EXT): overload.h
op$(OBJ_EXT): pad.h
op$(OBJ_EXT): parser.h
op$(OBJ_EXT): patchlevel.h
op$(OBJ_EXT): perl.h
op$(OBJ_EXT): perlapi.h
op$(OBJ_EXT): perlio.h
op$(OBJ_EXT): perlvars.h
op$(OBJ_EXT): perly.h
op$(OBJ_EXT): pp.h
op$(OBJ_EXT): proto.h
op$(OBJ_EXT): reentr.h
op$(OBJ_EXT): regcharclass.h
op$(OBJ_EXT): regcomp.h
op$(OBJ_EXT): regexp.h
op$(OBJ_EXT): regnodes.h
op$(OBJ_EXT): sbox32_hash.h
op$(OBJ_EXT): scope.h
op$(OBJ_EXT): sv.h
op$(OBJ_EXT): thread.h
op$(OBJ_EXT): unicode_constants.h
op$(OBJ_EXT): unixish.h
op$(OBJ_EXT): utf8.h
op$(OBJ_EXT): util.h
op$(OBJ_EXT): warnings.h
op$(OBJ_EXT): zaphod32_hash.h
op$(OBJ_EXT): cflags
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/alloca.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/arpa/inet.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/assert.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/alltypes.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/errno.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/fcntl.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/float.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl_fix.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipc.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipcstat.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/limits.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/posix.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/resource.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/sem.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/setjmp.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/signal.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/socket.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stat.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stdint.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/crypt.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/ctype.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/dirent.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/endian.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/errno.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fcntl.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/features.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/float.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdio.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdlib.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/string.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/strings.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/select.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/socket.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/unistd.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/grp.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/inttypes.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/limits.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/locale.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/math.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netdb.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netinet/in.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pthread.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pwd.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sched.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/setjmp.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/shadow.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/signal.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdarg.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdbool.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdc-predef.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stddef.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdint.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdio.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdlib.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/string.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/strings.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/cdefs.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/file.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/glibc-types.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ioctl.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ipc.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/param.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/resource.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/select.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/sem.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/socket.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/stat.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/time.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/times.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/types.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/wait.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/time.h
doop$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/unistd.h
doop$(OBJ_EXT): EXTERN.h
doop$(OBJ_EXT): av.h
doop$(OBJ_EXT): charclass_invlists.h
doop$(OBJ_EXT): config.h
doop$(OBJ_EXT): cop.h
doop$(OBJ_EXT): cv.h
doop$(OBJ_EXT): doop.c
doop$(OBJ_EXT): embed.h
doop$(OBJ_EXT): embedvar.h
doop$(OBJ_EXT): form.h
doop$(OBJ_EXT): gv.h
doop$(OBJ_EXT): handy.h
doop$(OBJ_EXT): hv.h
doop$(OBJ_EXT): hv_func.h
doop$(OBJ_EXT): hv_macro.h
doop$(OBJ_EXT): inline.h
doop$(OBJ_EXT): intrpvar.h
doop$(OBJ_EXT): iperlsys.h
doop$(OBJ_EXT): mg.h
doop$(OBJ_EXT): mg_vtable.h
doop$(OBJ_EXT): mydtrace.h
doop$(OBJ_EXT): op.h
doop$(OBJ_EXT): op_reg_common.h
doop$(OBJ_EXT): opcode.h
doop$(OBJ_EXT): opnames.h
doop$(OBJ_EXT): overload.h
doop$(OBJ_EXT): pad.h
doop$(OBJ_EXT): parser.h
doop$(OBJ_EXT): patchlevel.h
doop$(OBJ_EXT): perl.h
doop$(OBJ_EXT): perlio.h
doop$(OBJ_EXT): perlvars.h
doop$(OBJ_EXT): perly.h
doop$(OBJ_EXT): pp.h
doop$(OBJ_EXT): proto.h
doop$(OBJ_EXT): reentr.h
doop$(OBJ_EXT): regcharclass.h
doop$(OBJ_EXT): regexp.h
doop$(OBJ_EXT): sbox32_hash.h
doop$(OBJ_EXT): scope.h
doop$(OBJ_EXT): sv.h
doop$(OBJ_EXT): thread.h
doop$(OBJ_EXT): unicode_constants.h
doop$(OBJ_EXT): unixish.h
doop$(OBJ_EXT): utf8.h
doop$(OBJ_EXT): util.h
doop$(OBJ_EXT): warnings.h
doop$(OBJ_EXT): zaphod32_hash.h
doop$(OBJ_EXT): cflags
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/alloca.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/arpa/inet.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/assert.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/alltypes.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/errno.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/fcntl.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/float.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl_fix.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipc.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipcstat.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/limits.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/msg.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/posix.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/resource.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/sem.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/setjmp.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/shm.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/signal.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/socket.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stat.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stdint.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/crypt.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/ctype.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/dirent.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/endian.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/errno.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fcntl.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/features.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/float.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdio.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdlib.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/string.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/strings.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/select.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/socket.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/unistd.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/grp.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/inttypes.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/limits.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/locale.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/math.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netdb.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netinet/in.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pthread.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pwd.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sched.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/setjmp.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/shadow.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/signal.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdarg.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdbool.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdc-predef.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stddef.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdint.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdio.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdlib.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/string.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/strings.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/cdefs.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/file.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/glibc-types.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ioctl.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ipc.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/msg.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/param.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/resource.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/select.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/sem.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/shm.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/socket.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/stat.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/time.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/times.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/types.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/wait.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/time.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/unistd.h
doio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/utime.h
doio$(OBJ_EXT): EXTERN.h
doio$(OBJ_EXT): av.h
doio$(OBJ_EXT): charclass_invlists.h
doio$(OBJ_EXT): config.h
doio$(OBJ_EXT): cop.h
doio$(OBJ_EXT): cv.h
doio$(OBJ_EXT): doio.c
doio$(OBJ_EXT): embed.h
doio$(OBJ_EXT): embedvar.h
doio$(OBJ_EXT): form.h
doio$(OBJ_EXT): gv.h
doio$(OBJ_EXT): handy.h
doio$(OBJ_EXT): hv.h
doio$(OBJ_EXT): hv_func.h
doio$(OBJ_EXT): hv_macro.h
doio$(OBJ_EXT): inline.h
doio$(OBJ_EXT): intrpvar.h
doio$(OBJ_EXT): iperlsys.h
doio$(OBJ_EXT): mg.h
doio$(OBJ_EXT): mg_vtable.h
doio$(OBJ_EXT): mydtrace.h
doio$(OBJ_EXT): op.h
doio$(OBJ_EXT): op_reg_common.h
doio$(OBJ_EXT): opcode.h
doio$(OBJ_EXT): opnames.h
doio$(OBJ_EXT): overload.h
doio$(OBJ_EXT): pad.h
doio$(OBJ_EXT): parser.h
doio$(OBJ_EXT): patchlevel.h
doio$(OBJ_EXT): perl.h
doio$(OBJ_EXT): perlio.h
doio$(OBJ_EXT): perlvars.h
doio$(OBJ_EXT): perly.h
doio$(OBJ_EXT): pp.h
doio$(OBJ_EXT): proto.h
doio$(OBJ_EXT): reentr.h
doio$(OBJ_EXT): regcharclass.h
doio$(OBJ_EXT): regexp.h
doio$(OBJ_EXT): sbox32_hash.h
doio$(OBJ_EXT): scope.h
doio$(OBJ_EXT): sv.h
doio$(OBJ_EXT): thread.h
doio$(OBJ_EXT): unicode_constants.h
doio$(OBJ_EXT): unixish.h
doio$(OBJ_EXT): utf8.h
doio$(OBJ_EXT): util.h
doio$(OBJ_EXT): warnings.h
doio$(OBJ_EXT): zaphod32_hash.h
doio$(OBJ_EXT): cflags
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/alloca.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/arpa/inet.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/assert.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/alltypes.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/errno.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/fcntl.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/float.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl_fix.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipc.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipcstat.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/limits.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/posix.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/resource.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/sem.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/setjmp.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/signal.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/socket.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stat.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stdint.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/crypt.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/ctype.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/dirent.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/endian.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/errno.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fcntl.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/features.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/float.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdio.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdlib.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/string.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/strings.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/select.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/socket.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/unistd.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/grp.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/inttypes.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/limits.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/locale.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/math.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netdb.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netinet/in.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pthread.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pwd.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sched.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/setjmp.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/shadow.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/signal.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdarg.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdbool.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdc-predef.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stddef.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdint.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdio.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdlib.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/string.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/strings.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/cdefs.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/file.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/glibc-types.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ioctl.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ipc.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/param.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/resource.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/select.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/sem.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/socket.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/stat.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/time.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/times.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/types.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/wait.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/time.h
dump$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/unistd.h
dump$(OBJ_EXT): EXTERN.h
dump$(OBJ_EXT): av.h
dump$(OBJ_EXT): charclass_invlists.h
dump$(OBJ_EXT): config.h
dump$(OBJ_EXT): cop.h
dump$(OBJ_EXT): cv.h
dump$(OBJ_EXT): dump.c
dump$(OBJ_EXT): embed.h
dump$(OBJ_EXT): embedvar.h
dump$(OBJ_EXT): form.h
dump$(OBJ_EXT): gv.h
dump$(OBJ_EXT): handy.h
dump$(OBJ_EXT): hv.h
dump$(OBJ_EXT): hv_func.h
dump$(OBJ_EXT): hv_macro.h
dump$(OBJ_EXT): inline.h
dump$(OBJ_EXT): intrpvar.h
dump$(OBJ_EXT): iperlsys.h
dump$(OBJ_EXT): mg.h
dump$(OBJ_EXT): mg_names.inc
dump$(OBJ_EXT): mg_vtable.h
dump$(OBJ_EXT): mydtrace.h
dump$(OBJ_EXT): op.h
dump$(OBJ_EXT): op_reg_common.h
dump$(OBJ_EXT): opcode.h
dump$(OBJ_EXT): opnames.h
dump$(OBJ_EXT): overload.h
dump$(OBJ_EXT): pad.h
dump$(OBJ_EXT): parser.h
dump$(OBJ_EXT): patchlevel.h
dump$(OBJ_EXT): perl.h
dump$(OBJ_EXT): perlio.h
dump$(OBJ_EXT): perlvars.h
dump$(OBJ_EXT): perly.h
dump$(OBJ_EXT): pp.h
dump$(OBJ_EXT): proto.h
dump$(OBJ_EXT): reentr.h
dump$(OBJ_EXT): regcharclass.h
dump$(OBJ_EXT): regcomp.h
dump$(OBJ_EXT): regexp.h
dump$(OBJ_EXT): regnodes.h
dump$(OBJ_EXT): sbox32_hash.h
dump$(OBJ_EXT): scope.h
dump$(OBJ_EXT): sv.h
dump$(OBJ_EXT): thread.h
dump$(OBJ_EXT): unicode_constants.h
dump$(OBJ_EXT): unixish.h
dump$(OBJ_EXT): utf8.h
dump$(OBJ_EXT): util.h
dump$(OBJ_EXT): warnings.h
dump$(OBJ_EXT): zaphod32_hash.h
dump$(OBJ_EXT): cflags
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/alloca.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/arpa/inet.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/assert.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/alltypes.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/errno.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/fcntl.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/float.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl_fix.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipc.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipcstat.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/limits.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/posix.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/resource.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/sem.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/setjmp.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/signal.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/socket.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stat.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stdint.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/crypt.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/ctype.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/dirent.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/endian.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/errno.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fcntl.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/features.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/float.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdio.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdlib.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/string.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/strings.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/select.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/socket.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/unistd.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/grp.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/inttypes.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/limits.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/locale.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/math.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netdb.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netinet/in.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pthread.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pwd.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sched.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/setjmp.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/shadow.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/signal.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdarg.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdbool.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdc-predef.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stddef.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdint.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdio.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdlib.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/string.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/strings.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/cdefs.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/file.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/glibc-types.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ioctl.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ipc.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/param.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/resource.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/select.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/sem.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/socket.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/stat.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/time.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/times.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/types.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/wait.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/time.h
gv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/unistd.h
gv$(OBJ_EXT): EXTERN.h
gv$(OBJ_EXT): XSUB.h
gv$(OBJ_EXT): av.h
gv$(OBJ_EXT): charclass_invlists.h
gv$(OBJ_EXT): config.h
gv$(OBJ_EXT): cop.h
gv$(OBJ_EXT): cv.h
gv$(OBJ_EXT): embed.h
gv$(OBJ_EXT): embedvar.h
gv$(OBJ_EXT): feature.h
gv$(OBJ_EXT): form.h
gv$(OBJ_EXT): gv.c
gv$(OBJ_EXT): gv.h
gv$(OBJ_EXT): handy.h
gv$(OBJ_EXT): hv.h
gv$(OBJ_EXT): hv_func.h
gv$(OBJ_EXT): hv_macro.h
gv$(OBJ_EXT): inline.h
gv$(OBJ_EXT): intrpvar.h
gv$(OBJ_EXT): iperlsys.h
gv$(OBJ_EXT): keywords.h
gv$(OBJ_EXT): mg.h
gv$(OBJ_EXT): mg_vtable.h
gv$(OBJ_EXT): mydtrace.h
gv$(OBJ_EXT): op.h
gv$(OBJ_EXT): op_reg_common.h
gv$(OBJ_EXT): opcode.h
gv$(OBJ_EXT): opnames.h
gv$(OBJ_EXT): overload.h
gv$(OBJ_EXT): overload.inc
gv$(OBJ_EXT): pad.h
gv$(OBJ_EXT): parser.h
gv$(OBJ_EXT): patchlevel.h
gv$(OBJ_EXT): perl.h
gv$(OBJ_EXT): perlapi.h
gv$(OBJ_EXT): perlio.h
gv$(OBJ_EXT): perlvars.h
gv$(OBJ_EXT): perly.h
gv$(OBJ_EXT): pp.h
gv$(OBJ_EXT): proto.h
gv$(OBJ_EXT): reentr.h
gv$(OBJ_EXT): regcharclass.h
gv$(OBJ_EXT): regexp.h
gv$(OBJ_EXT): sbox32_hash.h
gv$(OBJ_EXT): scope.h
gv$(OBJ_EXT): sv.h
gv$(OBJ_EXT): thread.h
gv$(OBJ_EXT): unicode_constants.h
gv$(OBJ_EXT): unixish.h
gv$(OBJ_EXT): utf8.h
gv$(OBJ_EXT): util.h
gv$(OBJ_EXT): warnings.h
gv$(OBJ_EXT): zaphod32_hash.h
gv$(OBJ_EXT): cflags
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/alloca.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/arpa/inet.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/assert.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/alltypes.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/errno.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/fcntl.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/float.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl_fix.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipc.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipcstat.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/limits.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/posix.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/resource.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/sem.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/setjmp.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/signal.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/socket.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stat.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stdint.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/crypt.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/ctype.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/dirent.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/endian.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/errno.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fcntl.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/features.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/float.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdio.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdlib.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/string.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/strings.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/select.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/socket.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/unistd.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/grp.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/inttypes.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/limits.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/locale.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/math.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netdb.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netinet/in.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pthread.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pwd.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sched.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/setjmp.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/shadow.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/signal.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdarg.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdbool.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdc-predef.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stddef.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdint.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdio.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdlib.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/string.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/strings.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/cdefs.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/file.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/glibc-types.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ioctl.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ipc.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/param.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/resource.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/select.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/sem.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/socket.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/stat.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/time.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/times.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/types.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/wait.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/time.h
hv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/unistd.h
hv$(OBJ_EXT): EXTERN.h
hv$(OBJ_EXT): av.h
hv$(OBJ_EXT): charclass_invlists.h
hv$(OBJ_EXT): config.h
hv$(OBJ_EXT): cop.h
hv$(OBJ_EXT): cv.h
hv$(OBJ_EXT): embed.h
hv$(OBJ_EXT): embedvar.h
hv$(OBJ_EXT): form.h
hv$(OBJ_EXT): gv.h
hv$(OBJ_EXT): handy.h
hv$(OBJ_EXT): hv.c
hv$(OBJ_EXT): hv.h
hv$(OBJ_EXT): hv_func.h
hv$(OBJ_EXT): hv_macro.h
hv$(OBJ_EXT): inline.h
hv$(OBJ_EXT): intrpvar.h
hv$(OBJ_EXT): iperlsys.h
hv$(OBJ_EXT): mg.h
hv$(OBJ_EXT): mg_vtable.h
hv$(OBJ_EXT): mydtrace.h
hv$(OBJ_EXT): op.h
hv$(OBJ_EXT): op_reg_common.h
hv$(OBJ_EXT): opcode.h
hv$(OBJ_EXT): opnames.h
hv$(OBJ_EXT): overload.h
hv$(OBJ_EXT): pad.h
hv$(OBJ_EXT): parser.h
hv$(OBJ_EXT): patchlevel.h
hv$(OBJ_EXT): perl.h
hv$(OBJ_EXT): perlio.h
hv$(OBJ_EXT): perlvars.h
hv$(OBJ_EXT): perly.h
hv$(OBJ_EXT): pp.h
hv$(OBJ_EXT): proto.h
hv$(OBJ_EXT): reentr.h
hv$(OBJ_EXT): regcharclass.h
hv$(OBJ_EXT): regexp.h
hv$(OBJ_EXT): sbox32_hash.h
hv$(OBJ_EXT): scope.h
hv$(OBJ_EXT): sv.h
hv$(OBJ_EXT): thread.h
hv$(OBJ_EXT): unicode_constants.h
hv$(OBJ_EXT): unixish.h
hv$(OBJ_EXT): utf8.h
hv$(OBJ_EXT): util.h
hv$(OBJ_EXT): warnings.h
hv$(OBJ_EXT): zaphod32_hash.h
hv$(OBJ_EXT): cflags
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/alloca.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/arpa/inet.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/assert.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/alltypes.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/errno.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/fcntl.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/float.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl_fix.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipc.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipcstat.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/limits.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/posix.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/resource.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/sem.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/setjmp.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/signal.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/socket.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stat.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stdint.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/crypt.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/ctype.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/dirent.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/endian.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/errno.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fcntl.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/features.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/float.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdio.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdlib.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/string.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/strings.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/select.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/socket.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/unistd.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/grp.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/inttypes.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/limits.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/locale.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/math.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netdb.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netinet/in.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pthread.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pwd.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sched.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/setjmp.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/shadow.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/signal.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdarg.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdbool.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdc-predef.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stddef.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdint.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdio.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdlib.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/string.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/strings.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/cdefs.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/file.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/glibc-types.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ioctl.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ipc.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/param.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/prctl.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/resource.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/select.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/sem.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/socket.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/stat.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/time.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/times.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/types.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/wait.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/time.h
mg$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/unistd.h
mg$(OBJ_EXT): EXTERN.h
mg$(OBJ_EXT): av.h
mg$(OBJ_EXT): charclass_invlists.h
mg$(OBJ_EXT): config.h
mg$(OBJ_EXT): cop.h
mg$(OBJ_EXT): cv.h
mg$(OBJ_EXT): embed.h
mg$(OBJ_EXT): embedvar.h
mg$(OBJ_EXT): form.h
mg$(OBJ_EXT): gv.h
mg$(OBJ_EXT): handy.h
mg$(OBJ_EXT): hv.h
mg$(OBJ_EXT): hv_func.h
mg$(OBJ_EXT): hv_macro.h
mg$(OBJ_EXT): inline.h
mg$(OBJ_EXT): intrpvar.h
mg$(OBJ_EXT): iperlsys.h
mg$(OBJ_EXT): mg.c
mg$(OBJ_EXT): mg.h
mg$(OBJ_EXT): mg_vtable.h
mg$(OBJ_EXT): mydtrace.h
mg$(OBJ_EXT): op.h
mg$(OBJ_EXT): op_reg_common.h
mg$(OBJ_EXT): opcode.h
mg$(OBJ_EXT): opnames.h
mg$(OBJ_EXT): overload.h
mg$(OBJ_EXT): pad.h
mg$(OBJ_EXT): parser.h
mg$(OBJ_EXT): patchlevel.h
mg$(OBJ_EXT): perl.h
mg$(OBJ_EXT): perlio.h
mg$(OBJ_EXT): perlvars.h
mg$(OBJ_EXT): perly.h
mg$(OBJ_EXT): pp.h
mg$(OBJ_EXT): proto.h
mg$(OBJ_EXT): reentr.h
mg$(OBJ_EXT): regcharclass.h
mg$(OBJ_EXT): regexp.h
mg$(OBJ_EXT): sbox32_hash.h
mg$(OBJ_EXT): scope.h
mg$(OBJ_EXT): sv.h
mg$(OBJ_EXT): thread.h
mg$(OBJ_EXT): unicode_constants.h
mg$(OBJ_EXT): unixish.h
mg$(OBJ_EXT): utf8.h
mg$(OBJ_EXT): util.h
mg$(OBJ_EXT): warnings.h
mg$(OBJ_EXT): zaphod32_hash.h
mg$(OBJ_EXT): cflags
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/alloca.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/arpa/inet.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/assert.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/alltypes.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/errno.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/fcntl.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/float.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl_fix.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipc.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipcstat.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/limits.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/posix.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/resource.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/sem.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/setjmp.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/signal.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/socket.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stat.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stdint.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/crypt.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/ctype.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/dirent.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/endian.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/errno.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fcntl.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/features.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/float.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdio.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdlib.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/string.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/strings.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/select.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/socket.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/unistd.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/grp.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/inttypes.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/limits.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/locale.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/math.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netdb.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netinet/in.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pthread.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pwd.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sched.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/setjmp.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/shadow.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/signal.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdarg.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdbool.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdc-predef.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stddef.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdint.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdio.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdlib.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/string.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/strings.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/cdefs.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/file.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/glibc-types.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ioctl.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ipc.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/param.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/resource.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/select.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/sem.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/socket.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/stat.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/time.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/times.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/types.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/wait.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/time.h
reentr$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/unistd.h
reentr$(OBJ_EXT): EXTERN.h
reentr$(OBJ_EXT): av.h
reentr$(OBJ_EXT): charclass_invlists.h
reentr$(OBJ_EXT): config.h
reentr$(OBJ_EXT): cop.h
reentr$(OBJ_EXT): cv.h
reentr$(OBJ_EXT): embed.h
reentr$(OBJ_EXT): embedvar.h
reentr$(OBJ_EXT): form.h
reentr$(OBJ_EXT): gv.h
reentr$(OBJ_EXT): handy.h
reentr$(OBJ_EXT): hv.h
reentr$(OBJ_EXT): hv_func.h
reentr$(OBJ_EXT): hv_macro.h
reentr$(OBJ_EXT): inline.h
reentr$(OBJ_EXT): intrpvar.h
reentr$(OBJ_EXT): iperlsys.h
reentr$(OBJ_EXT): mg.h
reentr$(OBJ_EXT): mg_vtable.h
reentr$(OBJ_EXT): mydtrace.h
reentr$(OBJ_EXT): op.h
reentr$(OBJ_EXT): op_reg_common.h
reentr$(OBJ_EXT): opcode.h
reentr$(OBJ_EXT): opnames.h
reentr$(OBJ_EXT): overload.h
reentr$(OBJ_EXT): pad.h
reentr$(OBJ_EXT): parser.h
reentr$(OBJ_EXT): patchlevel.h
reentr$(OBJ_EXT): perl.h
reentr$(OBJ_EXT): perlio.h
reentr$(OBJ_EXT): perlvars.h
reentr$(OBJ_EXT): perly.h
reentr$(OBJ_EXT): pp.h
reentr$(OBJ_EXT): proto.h
reentr$(OBJ_EXT): reentr.c
reentr$(OBJ_EXT): reentr.h
reentr$(OBJ_EXT): regcharclass.h
reentr$(OBJ_EXT): regexp.h
reentr$(OBJ_EXT): sbox32_hash.h
reentr$(OBJ_EXT): scope.h
reentr$(OBJ_EXT): sv.h
reentr$(OBJ_EXT): thread.h
reentr$(OBJ_EXT): unicode_constants.h
reentr$(OBJ_EXT): unixish.h
reentr$(OBJ_EXT): utf8.h
reentr$(OBJ_EXT): util.h
reentr$(OBJ_EXT): warnings.h
reentr$(OBJ_EXT): zaphod32_hash.h
reentr$(OBJ_EXT): cflags
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/alloca.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/arpa/inet.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/assert.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/alltypes.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/errno.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/fcntl.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/float.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl_fix.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipc.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipcstat.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/limits.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/posix.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/resource.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/sem.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/setjmp.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/signal.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/socket.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stat.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stdint.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/crypt.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/ctype.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/dirent.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/endian.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/errno.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fcntl.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/features.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/float.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdio.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdlib.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/string.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/strings.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/select.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/socket.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/unistd.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/grp.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/inttypes.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/limits.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/locale.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/math.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netdb.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netinet/in.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pthread.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pwd.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sched.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/setjmp.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/shadow.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/signal.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdarg.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdbool.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdc-predef.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stddef.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdint.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdio.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdlib.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/string.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/strings.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/cdefs.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/file.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/glibc-types.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ioctl.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ipc.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/param.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/resource.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/select.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/sem.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/socket.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/stat.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/time.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/times.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/types.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/wait.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/time.h
mro_core$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/unistd.h
mro_core$(OBJ_EXT): EXTERN.h
mro_core$(OBJ_EXT): XSUB.h
mro_core$(OBJ_EXT): av.h
mro_core$(OBJ_EXT): charclass_invlists.h
mro_core$(OBJ_EXT): config.h
mro_core$(OBJ_EXT): cop.h
mro_core$(OBJ_EXT): cv.h
mro_core$(OBJ_EXT): embed.h
mro_core$(OBJ_EXT): embedvar.h
mro_core$(OBJ_EXT): form.h
mro_core$(OBJ_EXT): gv.h
mro_core$(OBJ_EXT): handy.h
mro_core$(OBJ_EXT): hv.h
mro_core$(OBJ_EXT): hv_func.h
mro_core$(OBJ_EXT): hv_macro.h
mro_core$(OBJ_EXT): inline.h
mro_core$(OBJ_EXT): intrpvar.h
mro_core$(OBJ_EXT): iperlsys.h
mro_core$(OBJ_EXT): mg.h
mro_core$(OBJ_EXT): mg_vtable.h
mro_core$(OBJ_EXT): mro_core.c
mro_core$(OBJ_EXT): mydtrace.h
mro_core$(OBJ_EXT): op.h
mro_core$(OBJ_EXT): op_reg_common.h
mro_core$(OBJ_EXT): opcode.h
mro_core$(OBJ_EXT): opnames.h
mro_core$(OBJ_EXT): overload.h
mro_core$(OBJ_EXT): pad.h
mro_core$(OBJ_EXT): parser.h
mro_core$(OBJ_EXT): patchlevel.h
mro_core$(OBJ_EXT): perl.h
mro_core$(OBJ_EXT): perlapi.h
mro_core$(OBJ_EXT): perlio.h
mro_core$(OBJ_EXT): perlvars.h
mro_core$(OBJ_EXT): perly.h
mro_core$(OBJ_EXT): pp.h
mro_core$(OBJ_EXT): proto.h
mro_core$(OBJ_EXT): reentr.h
mro_core$(OBJ_EXT): regcharclass.h
mro_core$(OBJ_EXT): regexp.h
mro_core$(OBJ_EXT): sbox32_hash.h
mro_core$(OBJ_EXT): scope.h
mro_core$(OBJ_EXT): sv.h
mro_core$(OBJ_EXT): thread.h
mro_core$(OBJ_EXT): unicode_constants.h
mro_core$(OBJ_EXT): unixish.h
mro_core$(OBJ_EXT): utf8.h
mro_core$(OBJ_EXT): util.h
mro_core$(OBJ_EXT): warnings.h
mro_core$(OBJ_EXT): zaphod32_hash.h
mro_core$(OBJ_EXT): cflags
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/alloca.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/arpa/inet.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/assert.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/alltypes.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/errno.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/fcntl.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/float.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl_fix.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipc.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipcstat.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/limits.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/posix.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/resource.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/sem.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/setjmp.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/signal.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/socket.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stat.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stdint.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/crypt.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/ctype.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/dirent.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/endian.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/errno.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fcntl.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/features.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/float.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdio.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdlib.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/string.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/strings.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/select.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/socket.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/unistd.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/grp.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/inttypes.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/limits.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/locale.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/math.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netdb.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netinet/in.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pthread.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pwd.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sched.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/setjmp.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/shadow.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/signal.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdarg.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdbool.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdc-predef.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stddef.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdint.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdio.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdlib.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/string.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/strings.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/cdefs.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/file.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/glibc-types.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ioctl.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ipc.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/param.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/resource.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/select.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/sem.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/socket.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/stat.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/time.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/times.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/types.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/wait.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/time.h
perl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/unistd.h
perl$(OBJ_EXT): EXTERN.h
perl$(OBJ_EXT): XSUB.h
perl$(OBJ_EXT): av.h
perl$(OBJ_EXT): charclass_invlists.h
perl$(OBJ_EXT): config.h
perl$(OBJ_EXT): cop.h
perl$(OBJ_EXT): cv.h
perl$(OBJ_EXT): embed.h
perl$(OBJ_EXT): embedvar.h
perl$(OBJ_EXT): form.h
perl$(OBJ_EXT): gv.h
perl$(OBJ_EXT): handy.h
perl$(OBJ_EXT): hv.h
perl$(OBJ_EXT): hv_func.h
perl$(OBJ_EXT): hv_macro.h
perl$(OBJ_EXT): inline.h
perl$(OBJ_EXT): intrpvar.h
perl$(OBJ_EXT): iperlsys.h
perl$(OBJ_EXT): mg.h
perl$(OBJ_EXT): mg_vtable.h
perl$(OBJ_EXT): mydtrace.h
perl$(OBJ_EXT): op.h
perl$(OBJ_EXT): op_reg_common.h
perl$(OBJ_EXT): opcode.h
perl$(OBJ_EXT): opnames.h
perl$(OBJ_EXT): overload.h
perl$(OBJ_EXT): pad.h
perl$(OBJ_EXT): parser.h
perl$(OBJ_EXT): patchlevel.h
perl$(OBJ_EXT): perl.c
perl$(OBJ_EXT): perl.h
perl$(OBJ_EXT): perl_inc_macro.h
perl$(OBJ_EXT): perlapi.h
perl$(OBJ_EXT): perlio.h
perl$(OBJ_EXT): perlvars.h
perl$(OBJ_EXT): perly.h
perl$(OBJ_EXT): pp.h
perl$(OBJ_EXT): proto.h
perl$(OBJ_EXT): reentr.h
perl$(OBJ_EXT): regcharclass.h
perl$(OBJ_EXT): regexp.h
perl$(OBJ_EXT): sbox32_hash.h
perl$(OBJ_EXT): scope.h
perl$(OBJ_EXT): sv.h
perl$(OBJ_EXT): thread.h
perl$(OBJ_EXT): unicode_constants.h
perl$(OBJ_EXT): unixish.h
perl$(OBJ_EXT): utf8.h
perl$(OBJ_EXT): util.h
perl$(OBJ_EXT): warnings.h
perl$(OBJ_EXT): zaphod32_hash.h
perl$(OBJ_EXT): cflags
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/alloca.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/arpa/inet.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/assert.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/alltypes.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/errno.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/fcntl.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/float.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl_fix.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipc.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipcstat.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/limits.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/posix.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/resource.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/sem.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/setjmp.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/signal.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/socket.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stat.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stdint.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/crypt.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/ctype.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/dirent.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/endian.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/errno.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fcntl.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/features.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/float.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdio.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdlib.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/string.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/strings.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/select.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/socket.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/unistd.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/grp.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/inttypes.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/limits.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/locale.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/math.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netdb.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netinet/in.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pthread.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pwd.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sched.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/setjmp.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/shadow.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/signal.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdarg.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdbool.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdc-predef.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stddef.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdint.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdio.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdlib.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/string.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/strings.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/cdefs.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/file.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/glibc-types.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ioctl.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ipc.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/param.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/resource.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/select.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/sem.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/socket.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/stat.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/time.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/times.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/types.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/wait.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/time.h
perly$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/unistd.h
perly$(OBJ_EXT): EXTERN.h
perly$(OBJ_EXT): av.h
perly$(OBJ_EXT): charclass_invlists.h
perly$(OBJ_EXT): config.h
perly$(OBJ_EXT): cop.h
perly$(OBJ_EXT): cv.h
perly$(OBJ_EXT): embed.h
perly$(OBJ_EXT): embedvar.h
perly$(OBJ_EXT): feature.h
perly$(OBJ_EXT): form.h
perly$(OBJ_EXT): gv.h
perly$(OBJ_EXT): handy.h
perly$(OBJ_EXT): hv.h
perly$(OBJ_EXT): hv_func.h
perly$(OBJ_EXT): hv_macro.h
perly$(OBJ_EXT): inline.h
perly$(OBJ_EXT): intrpvar.h
perly$(OBJ_EXT): iperlsys.h
perly$(OBJ_EXT): keywords.h
perly$(OBJ_EXT): mg.h
perly$(OBJ_EXT): mg_vtable.h
perly$(OBJ_EXT): mydtrace.h
perly$(OBJ_EXT): op.h
perly$(OBJ_EXT): op_reg_common.h
perly$(OBJ_EXT): opcode.h
perly$(OBJ_EXT): opnames.h
perly$(OBJ_EXT): overload.h
perly$(OBJ_EXT): pad.h
perly$(OBJ_EXT): parser.h
perly$(OBJ_EXT): patchlevel.h
perly$(OBJ_EXT): perl.h
perly$(OBJ_EXT): perlio.h
perly$(OBJ_EXT): perlvars.h
perly$(OBJ_EXT): perly.act
perly$(OBJ_EXT): perly.c
perly$(OBJ_EXT): perly.h
perly$(OBJ_EXT): perly.tab
perly$(OBJ_EXT): perly.y
perly$(OBJ_EXT): pp.h
perly$(OBJ_EXT): proto.h
perly$(OBJ_EXT): reentr.h
perly$(OBJ_EXT): regcharclass.h
perly$(OBJ_EXT): regexp.h
perly$(OBJ_EXT): sbox32_hash.h
perly$(OBJ_EXT): scope.h
perly$(OBJ_EXT): sv.h
perly$(OBJ_EXT): thread.h
perly$(OBJ_EXT): unicode_constants.h
perly$(OBJ_EXT): unixish.h
perly$(OBJ_EXT): utf8.h
perly$(OBJ_EXT): util.h
perly$(OBJ_EXT): warnings.h
perly$(OBJ_EXT): zaphod32_hash.h
perly$(OBJ_EXT): cflags
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/alloca.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/arpa/inet.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/assert.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/alltypes.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/errno.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/fcntl.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/float.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl_fix.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipc.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipcstat.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/limits.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/posix.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/resource.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/sem.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/setjmp.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/signal.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/socket.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stat.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stdint.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/crypt.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/ctype.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/dirent.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/endian.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/errno.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fcntl.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/features.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/float.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdio.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdlib.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/string.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/strings.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/select.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/socket.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/unistd.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/grp.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/inttypes.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/limits.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/locale.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/math.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netdb.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netinet/in.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pthread.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pwd.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sched.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/setjmp.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/shadow.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/signal.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdarg.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdbool.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdc-predef.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stddef.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdint.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdio.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdlib.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/string.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/strings.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/cdefs.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/file.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/glibc-types.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ioctl.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ipc.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/param.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/resource.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/select.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/sem.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/socket.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/stat.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/time.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/times.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/types.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/wait.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/time.h
pp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/unistd.h
pp$(OBJ_EXT): EXTERN.h
pp$(OBJ_EXT): av.h
pp$(OBJ_EXT): charclass_invlists.h
pp$(OBJ_EXT): config.h
pp$(OBJ_EXT): cop.h
pp$(OBJ_EXT): cv.h
pp$(OBJ_EXT): embed.h
pp$(OBJ_EXT): embedvar.h
pp$(OBJ_EXT): form.h
pp$(OBJ_EXT): gv.h
pp$(OBJ_EXT): handy.h
pp$(OBJ_EXT): hv.h
pp$(OBJ_EXT): hv_func.h
pp$(OBJ_EXT): hv_macro.h
pp$(OBJ_EXT): inline.h
pp$(OBJ_EXT): intrpvar.h
pp$(OBJ_EXT): iperlsys.h
pp$(OBJ_EXT): keywords.h
pp$(OBJ_EXT): mg.h
pp$(OBJ_EXT): mg_vtable.h
pp$(OBJ_EXT): mydtrace.h
pp$(OBJ_EXT): op.h
pp$(OBJ_EXT): op_reg_common.h
pp$(OBJ_EXT): opcode.h
pp$(OBJ_EXT): opnames.h
pp$(OBJ_EXT): overload.h
pp$(OBJ_EXT): pad.h
pp$(OBJ_EXT): parser.h
pp$(OBJ_EXT): patchlevel.h
pp$(OBJ_EXT): perl.h
pp$(OBJ_EXT): perlio.h
pp$(OBJ_EXT): perlvars.h
pp$(OBJ_EXT): perly.h
pp$(OBJ_EXT): pp.c
pp$(OBJ_EXT): pp.h
pp$(OBJ_EXT): proto.h
pp$(OBJ_EXT): reentr.h
pp$(OBJ_EXT): regcharclass.h
pp$(OBJ_EXT): regexp.h
pp$(OBJ_EXT): sbox32_hash.h
pp$(OBJ_EXT): scope.h
pp$(OBJ_EXT): sv.h
pp$(OBJ_EXT): thread.h
pp$(OBJ_EXT): unicode_constants.h
pp$(OBJ_EXT): unixish.h
pp$(OBJ_EXT): utf8.h
pp$(OBJ_EXT): util.h
pp$(OBJ_EXT): warnings.h
pp$(OBJ_EXT): zaphod32_hash.h
pp$(OBJ_EXT): cflags
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/alloca.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/arpa/inet.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/assert.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/alltypes.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/errno.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/fcntl.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/float.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl_fix.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipc.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipcstat.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/limits.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/posix.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/resource.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/sem.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/setjmp.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/signal.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/socket.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stat.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stdint.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/crypt.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/ctype.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/dirent.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/endian.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/errno.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fcntl.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/features.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/float.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdio.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdlib.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/string.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/strings.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/select.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/socket.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/unistd.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/grp.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/inttypes.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/limits.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/locale.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/math.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netdb.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netinet/in.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pthread.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pwd.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sched.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/setjmp.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/shadow.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/signal.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdarg.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdbool.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdc-predef.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stddef.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdint.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdio.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdlib.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/string.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/strings.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/cdefs.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/file.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/glibc-types.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ioctl.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ipc.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/param.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/resource.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/select.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/sem.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/socket.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/stat.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/time.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/times.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/types.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/wait.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/time.h
pp_hot$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/unistd.h
pp_hot$(OBJ_EXT): EXTERN.h
pp_hot$(OBJ_EXT): av.h
pp_hot$(OBJ_EXT): charclass_invlists.h
pp_hot$(OBJ_EXT): config.h
pp_hot$(OBJ_EXT): cop.h
pp_hot$(OBJ_EXT): cv.h
pp_hot$(OBJ_EXT): embed.h
pp_hot$(OBJ_EXT): embedvar.h
pp_hot$(OBJ_EXT): form.h
pp_hot$(OBJ_EXT): gv.h
pp_hot$(OBJ_EXT): handy.h
pp_hot$(OBJ_EXT): hv.h
pp_hot$(OBJ_EXT): hv_func.h
pp_hot$(OBJ_EXT): hv_macro.h
pp_hot$(OBJ_EXT): inline.h
pp_hot$(OBJ_EXT): intrpvar.h
pp_hot$(OBJ_EXT): iperlsys.h
pp_hot$(OBJ_EXT): mg.h
pp_hot$(OBJ_EXT): mg_vtable.h
pp_hot$(OBJ_EXT): mydtrace.h
pp_hot$(OBJ_EXT): op.h
pp_hot$(OBJ_EXT): op_reg_common.h
pp_hot$(OBJ_EXT): opcode.h
pp_hot$(OBJ_EXT): opnames.h
pp_hot$(OBJ_EXT): overload.h
pp_hot$(OBJ_EXT): pad.h
pp_hot$(OBJ_EXT): parser.h
pp_hot$(OBJ_EXT): patchlevel.h
pp_hot$(OBJ_EXT): perl.h
pp_hot$(OBJ_EXT): perlio.h
pp_hot$(OBJ_EXT): perlvars.h
pp_hot$(OBJ_EXT): perly.h
pp_hot$(OBJ_EXT): pp.h
pp_hot$(OBJ_EXT): pp_hot.c
pp_hot$(OBJ_EXT): proto.h
pp_hot$(OBJ_EXT): reentr.h
pp_hot$(OBJ_EXT): regcharclass.h
pp_hot$(OBJ_EXT): regexp.h
pp_hot$(OBJ_EXT): sbox32_hash.h
pp_hot$(OBJ_EXT): scope.h
pp_hot$(OBJ_EXT): sv.h
pp_hot$(OBJ_EXT): thread.h
pp_hot$(OBJ_EXT): unicode_constants.h
pp_hot$(OBJ_EXT): unixish.h
pp_hot$(OBJ_EXT): utf8.h
pp_hot$(OBJ_EXT): util.h
pp_hot$(OBJ_EXT): warnings.h
pp_hot$(OBJ_EXT): zaphod32_hash.h
pp_hot$(OBJ_EXT): cflags
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/alloca.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/arpa/inet.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/assert.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/alltypes.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/errno.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/fcntl.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/float.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl_fix.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipc.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipcstat.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/limits.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/posix.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/resource.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/sem.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/setjmp.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/signal.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/socket.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stat.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stdint.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/crypt.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/ctype.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/dirent.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/endian.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/errno.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fcntl.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/features.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/float.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdio.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdlib.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/string.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/strings.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/select.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/socket.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/unistd.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/grp.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/inttypes.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/limits.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/locale.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/math.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netdb.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netinet/in.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pthread.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pwd.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sched.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/setjmp.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/shadow.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/signal.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdarg.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdbool.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdc-predef.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stddef.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdint.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdio.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdlib.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/string.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/strings.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/cdefs.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/file.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/glibc-types.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ioctl.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ipc.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/param.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/resource.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/select.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/sem.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/socket.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/stat.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/time.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/times.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/types.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/wait.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/time.h
pp_ctl$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/unistd.h
pp_ctl$(OBJ_EXT): EXTERN.h
pp_ctl$(OBJ_EXT): av.h
pp_ctl$(OBJ_EXT): charclass_invlists.h
pp_ctl$(OBJ_EXT): config.h
pp_ctl$(OBJ_EXT): cop.h
pp_ctl$(OBJ_EXT): cv.h
pp_ctl$(OBJ_EXT): embed.h
pp_ctl$(OBJ_EXT): embedvar.h
pp_ctl$(OBJ_EXT): form.h
pp_ctl$(OBJ_EXT): gv.h
pp_ctl$(OBJ_EXT): handy.h
pp_ctl$(OBJ_EXT): hv.h
pp_ctl$(OBJ_EXT): hv_func.h
pp_ctl$(OBJ_EXT): hv_macro.h
pp_ctl$(OBJ_EXT): inline.h
pp_ctl$(OBJ_EXT): intrpvar.h
pp_ctl$(OBJ_EXT): iperlsys.h
pp_ctl$(OBJ_EXT): mg.h
pp_ctl$(OBJ_EXT): mg_vtable.h
pp_ctl$(OBJ_EXT): mydtrace.h
pp_ctl$(OBJ_EXT): op.h
pp_ctl$(OBJ_EXT): op_reg_common.h
pp_ctl$(OBJ_EXT): opcode.h
pp_ctl$(OBJ_EXT): opnames.h
pp_ctl$(OBJ_EXT): overload.h
pp_ctl$(OBJ_EXT): pad.h
pp_ctl$(OBJ_EXT): parser.h
pp_ctl$(OBJ_EXT): patchlevel.h
pp_ctl$(OBJ_EXT): perl.h
pp_ctl$(OBJ_EXT): perlio.h
pp_ctl$(OBJ_EXT): perlvars.h
pp_ctl$(OBJ_EXT): perly.h
pp_ctl$(OBJ_EXT): pp.h
pp_ctl$(OBJ_EXT): pp_ctl.c
pp_ctl$(OBJ_EXT): proto.h
pp_ctl$(OBJ_EXT): reentr.h
pp_ctl$(OBJ_EXT): regcharclass.h
pp_ctl$(OBJ_EXT): regexp.h
pp_ctl$(OBJ_EXT): sbox32_hash.h
pp_ctl$(OBJ_EXT): scope.h
pp_ctl$(OBJ_EXT): sv.h
pp_ctl$(OBJ_EXT): thread.h
pp_ctl$(OBJ_EXT): unicode_constants.h
pp_ctl$(OBJ_EXT): unixish.h
pp_ctl$(OBJ_EXT): utf8.h
pp_ctl$(OBJ_EXT): util.h
pp_ctl$(OBJ_EXT): warnings.h
pp_ctl$(OBJ_EXT): zaphod32_hash.h
pp_ctl$(OBJ_EXT): cflags
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/alloca.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/arpa/inet.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/assert.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/alltypes.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/errno.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/fcntl.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/float.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl_fix.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipc.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipcstat.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/limits.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/posix.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/resource.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/sem.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/setjmp.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/signal.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/socket.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stat.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stdint.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/crypt.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/ctype.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/dirent.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/endian.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/errno.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fcntl.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/features.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/float.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdio.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdlib.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/string.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/strings.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/select.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/socket.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/unistd.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/grp.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/inttypes.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/limits.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/locale.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/math.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netdb.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netinet/in.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pthread.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pwd.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sched.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/setjmp.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/shadow.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/signal.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdarg.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdbool.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdc-predef.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stddef.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdint.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdio.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdlib.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/string.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/strings.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/cdefs.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/file.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/glibc-types.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ioctl.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ipc.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/param.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/resource.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/select.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/sem.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/socket.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/stat.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/time.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/times.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/types.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/wait.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/time.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/unistd.h
pp_sys$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/utime.h
pp_sys$(OBJ_EXT): EXTERN.h
pp_sys$(OBJ_EXT): av.h
pp_sys$(OBJ_EXT): charclass_invlists.h
pp_sys$(OBJ_EXT): config.h
pp_sys$(OBJ_EXT): cop.h
pp_sys$(OBJ_EXT): cv.h
pp_sys$(OBJ_EXT): embed.h
pp_sys$(OBJ_EXT): embedvar.h
pp_sys$(OBJ_EXT): form.h
pp_sys$(OBJ_EXT): gv.h
pp_sys$(OBJ_EXT): handy.h
pp_sys$(OBJ_EXT): hv.h
pp_sys$(OBJ_EXT): hv_func.h
pp_sys$(OBJ_EXT): hv_macro.h
pp_sys$(OBJ_EXT): inline.h
pp_sys$(OBJ_EXT): intrpvar.h
pp_sys$(OBJ_EXT): iperlsys.h
pp_sys$(OBJ_EXT): mg.h
pp_sys$(OBJ_EXT): mg_vtable.h
pp_sys$(OBJ_EXT): mydtrace.h
pp_sys$(OBJ_EXT): op.h
pp_sys$(OBJ_EXT): op_reg_common.h
pp_sys$(OBJ_EXT): opcode.h
pp_sys$(OBJ_EXT): opnames.h
pp_sys$(OBJ_EXT): overload.h
pp_sys$(OBJ_EXT): pad.h
pp_sys$(OBJ_EXT): parser.h
pp_sys$(OBJ_EXT): patchlevel.h
pp_sys$(OBJ_EXT): perl.h
pp_sys$(OBJ_EXT): perlio.h
pp_sys$(OBJ_EXT): perlvars.h
pp_sys$(OBJ_EXT): perly.h
pp_sys$(OBJ_EXT): pp.h
pp_sys$(OBJ_EXT): pp_sys.c
pp_sys$(OBJ_EXT): proto.h
pp_sys$(OBJ_EXT): reentr.h
pp_sys$(OBJ_EXT): regcharclass.h
pp_sys$(OBJ_EXT): regexp.h
pp_sys$(OBJ_EXT): sbox32_hash.h
pp_sys$(OBJ_EXT): scope.h
pp_sys$(OBJ_EXT): sv.h
pp_sys$(OBJ_EXT): thread.h
pp_sys$(OBJ_EXT): time64.h
pp_sys$(OBJ_EXT): time64_config.h
pp_sys$(OBJ_EXT): unicode_constants.h
pp_sys$(OBJ_EXT): unixish.h
pp_sys$(OBJ_EXT): utf8.h
pp_sys$(OBJ_EXT): util.h
pp_sys$(OBJ_EXT): warnings.h
pp_sys$(OBJ_EXT): zaphod32_hash.h
pp_sys$(OBJ_EXT): cflags
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/alloca.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/arpa/inet.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/assert.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/alltypes.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/errno.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/fcntl.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/float.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl_fix.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipc.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipcstat.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/limits.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/posix.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/resource.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/sem.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/setjmp.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/signal.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/socket.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stat.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stdint.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/crypt.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/ctype.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/dirent.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/endian.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/errno.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fcntl.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/features.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/float.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdio.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdlib.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/string.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/strings.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/select.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/socket.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/unistd.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/grp.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/inttypes.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/limits.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/locale.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/math.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netdb.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netinet/in.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pthread.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pwd.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sched.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/setjmp.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/shadow.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/signal.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdarg.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdbool.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdc-predef.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stddef.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdint.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdio.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdlib.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/string.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/strings.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/cdefs.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/file.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/glibc-types.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ioctl.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ipc.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/param.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/resource.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/select.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/sem.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/socket.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/stat.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/time.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/times.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/types.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/wait.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/time.h
regcomp$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/unistd.h
regcomp$(OBJ_EXT): EXTERN.h
regcomp$(OBJ_EXT): INTERN.h
regcomp$(OBJ_EXT): av.h
regcomp$(OBJ_EXT): charclass_invlists.h
regcomp$(OBJ_EXT): config.h
regcomp$(OBJ_EXT): cop.h
regcomp$(OBJ_EXT): cv.h
regcomp$(OBJ_EXT): dquote_inline.h
regcomp$(OBJ_EXT): embed.h
regcomp$(OBJ_EXT): embedvar.h
regcomp$(OBJ_EXT): form.h
regcomp$(OBJ_EXT): gv.h
regcomp$(OBJ_EXT): handy.h
regcomp$(OBJ_EXT): hv.h
regcomp$(OBJ_EXT): hv_func.h
regcomp$(OBJ_EXT): hv_macro.h
regcomp$(OBJ_EXT): inline.h
regcomp$(OBJ_EXT): intrpvar.h
regcomp$(OBJ_EXT): invlist_inline.h
regcomp$(OBJ_EXT): iperlsys.h
regcomp$(OBJ_EXT): mg.h
regcomp$(OBJ_EXT): mg_vtable.h
regcomp$(OBJ_EXT): mydtrace.h
regcomp$(OBJ_EXT): op.h
regcomp$(OBJ_EXT): op_reg_common.h
regcomp$(OBJ_EXT): opcode.h
regcomp$(OBJ_EXT): opnames.h
regcomp$(OBJ_EXT): overload.h
regcomp$(OBJ_EXT): pad.h
regcomp$(OBJ_EXT): parser.h
regcomp$(OBJ_EXT): patchlevel.h
regcomp$(OBJ_EXT): perl.h
regcomp$(OBJ_EXT): perlio.h
regcomp$(OBJ_EXT): perlvars.h
regcomp$(OBJ_EXT): perly.h
regcomp$(OBJ_EXT): pp.h
regcomp$(OBJ_EXT): proto.h
regcomp$(OBJ_EXT): reentr.h
regcomp$(OBJ_EXT): regcharclass.h
regcomp$(OBJ_EXT): regcomp.c
regcomp$(OBJ_EXT): regcomp.h
regcomp$(OBJ_EXT): regexp.h
regcomp$(OBJ_EXT): regnodes.h
regcomp$(OBJ_EXT): sbox32_hash.h
regcomp$(OBJ_EXT): scope.h
regcomp$(OBJ_EXT): sv.h
regcomp$(OBJ_EXT): thread.h
regcomp$(OBJ_EXT): unicode_constants.h
regcomp$(OBJ_EXT): unixish.h
regcomp$(OBJ_EXT): utf8.h
regcomp$(OBJ_EXT): util.h
regcomp$(OBJ_EXT): warnings.h
regcomp$(OBJ_EXT): zaphod32_hash.h
regcomp$(OBJ_EXT): cflags
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/alloca.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/arpa/inet.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/assert.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/alltypes.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/errno.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/fcntl.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/float.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl_fix.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipc.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipcstat.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/limits.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/posix.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/resource.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/sem.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/setjmp.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/signal.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/socket.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stat.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stdint.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/crypt.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/ctype.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/dirent.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/endian.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/errno.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fcntl.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/features.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/float.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdio.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdlib.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/string.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/strings.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/select.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/socket.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/unistd.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/grp.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/inttypes.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/limits.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/locale.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/math.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netdb.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netinet/in.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pthread.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pwd.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sched.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/setjmp.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/shadow.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/signal.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdarg.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdbool.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdc-predef.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stddef.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdint.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdio.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdlib.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/string.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/strings.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/cdefs.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/file.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/glibc-types.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ioctl.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ipc.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/param.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/resource.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/select.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/sem.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/socket.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/stat.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/time.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/times.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/types.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/wait.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/time.h
regexec$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/unistd.h
regexec$(OBJ_EXT): EXTERN.h
regexec$(OBJ_EXT): av.h
regexec$(OBJ_EXT): charclass_invlists.h
regexec$(OBJ_EXT): config.h
regexec$(OBJ_EXT): cop.h
regexec$(OBJ_EXT): cv.h
regexec$(OBJ_EXT): embed.h
regexec$(OBJ_EXT): embedvar.h
regexec$(OBJ_EXT): form.h
regexec$(OBJ_EXT): gv.h
regexec$(OBJ_EXT): handy.h
regexec$(OBJ_EXT): hv.h
regexec$(OBJ_EXT): hv_func.h
regexec$(OBJ_EXT): hv_macro.h
regexec$(OBJ_EXT): inline.h
regexec$(OBJ_EXT): intrpvar.h
regexec$(OBJ_EXT): invlist_inline.h
regexec$(OBJ_EXT): iperlsys.h
regexec$(OBJ_EXT): mg.h
regexec$(OBJ_EXT): mg_vtable.h
regexec$(OBJ_EXT): mydtrace.h
regexec$(OBJ_EXT): op.h
regexec$(OBJ_EXT): op_reg_common.h
regexec$(OBJ_EXT): opcode.h
regexec$(OBJ_EXT): opnames.h
regexec$(OBJ_EXT): overload.h
regexec$(OBJ_EXT): pad.h
regexec$(OBJ_EXT): parser.h
regexec$(OBJ_EXT): patchlevel.h
regexec$(OBJ_EXT): perl.h
regexec$(OBJ_EXT): perlio.h
regexec$(OBJ_EXT): perlvars.h
regexec$(OBJ_EXT): perly.h
regexec$(OBJ_EXT): pp.h
regexec$(OBJ_EXT): proto.h
regexec$(OBJ_EXT): reentr.h
regexec$(OBJ_EXT): regcharclass.h
regexec$(OBJ_EXT): regcomp.h
regexec$(OBJ_EXT): regexec.c
regexec$(OBJ_EXT): regexp.h
regexec$(OBJ_EXT): regnodes.h
regexec$(OBJ_EXT): sbox32_hash.h
regexec$(OBJ_EXT): scope.h
regexec$(OBJ_EXT): sv.h
regexec$(OBJ_EXT): thread.h
regexec$(OBJ_EXT): unicode_constants.h
regexec$(OBJ_EXT): unixish.h
regexec$(OBJ_EXT): utf8.h
regexec$(OBJ_EXT): util.h
regexec$(OBJ_EXT): warnings.h
regexec$(OBJ_EXT): zaphod32_hash.h
regexec$(OBJ_EXT): cflags
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/alloca.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/arpa/inet.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/assert.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/alltypes.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/errno.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/fcntl.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/float.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl_fix.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipc.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipcstat.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/limits.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/posix.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/resource.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/sem.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/setjmp.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/signal.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/socket.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stat.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stdint.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/crypt.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/ctype.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/dirent.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/endian.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/errno.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fcntl.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/features.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/float.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdio.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdlib.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/string.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/strings.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/select.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/socket.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/unistd.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/grp.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/inttypes.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/limits.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/locale.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/math.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netdb.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netinet/in.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pthread.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pwd.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sched.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/setjmp.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/shadow.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/signal.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdarg.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdbool.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdc-predef.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stddef.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdint.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdio.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdlib.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/string.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/strings.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/cdefs.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/file.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/glibc-types.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ioctl.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ipc.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/param.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/resource.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/select.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/sem.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/socket.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/stat.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/time.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/times.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/types.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/wait.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/time.h
utf8$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/unistd.h
utf8$(OBJ_EXT): EXTERN.h
utf8$(OBJ_EXT): av.h
utf8$(OBJ_EXT): charclass_invlists.h
utf8$(OBJ_EXT): config.h
utf8$(OBJ_EXT): cop.h
utf8$(OBJ_EXT): cv.h
utf8$(OBJ_EXT): embed.h
utf8$(OBJ_EXT): embedvar.h
utf8$(OBJ_EXT): form.h
utf8$(OBJ_EXT): gv.h
utf8$(OBJ_EXT): handy.h
utf8$(OBJ_EXT): hv.h
utf8$(OBJ_EXT): hv_func.h
utf8$(OBJ_EXT): hv_macro.h
utf8$(OBJ_EXT): inline.h
utf8$(OBJ_EXT): intrpvar.h
utf8$(OBJ_EXT): invlist_inline.h
utf8$(OBJ_EXT): iperlsys.h
utf8$(OBJ_EXT): mg.h
utf8$(OBJ_EXT): mg_vtable.h
utf8$(OBJ_EXT): mydtrace.h
utf8$(OBJ_EXT): op.h
utf8$(OBJ_EXT): op_reg_common.h
utf8$(OBJ_EXT): opcode.h
utf8$(OBJ_EXT): opnames.h
utf8$(OBJ_EXT): overload.h
utf8$(OBJ_EXT): pad.h
utf8$(OBJ_EXT): parser.h
utf8$(OBJ_EXT): patchlevel.h
utf8$(OBJ_EXT): perl.h
utf8$(OBJ_EXT): perlio.h
utf8$(OBJ_EXT): perlvars.h
utf8$(OBJ_EXT): perly.h
utf8$(OBJ_EXT): pp.h
utf8$(OBJ_EXT): proto.h
utf8$(OBJ_EXT): reentr.h
utf8$(OBJ_EXT): regcharclass.h
utf8$(OBJ_EXT): regexp.h
utf8$(OBJ_EXT): sbox32_hash.h
utf8$(OBJ_EXT): scope.h
utf8$(OBJ_EXT): sv.h
utf8$(OBJ_EXT): thread.h
utf8$(OBJ_EXT): uni_keywords.h
utf8$(OBJ_EXT): unicode_constants.h
utf8$(OBJ_EXT): unixish.h
utf8$(OBJ_EXT): utf8.c
utf8$(OBJ_EXT): utf8.h
utf8$(OBJ_EXT): util.h
utf8$(OBJ_EXT): warnings.h
utf8$(OBJ_EXT): zaphod32_hash.h
utf8$(OBJ_EXT): cflags
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/alloca.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/arpa/inet.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/assert.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/alltypes.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/errno.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/fcntl.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/float.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl_fix.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipc.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipcstat.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/limits.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/posix.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/resource.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/sem.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/setjmp.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/signal.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/socket.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stat.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stdint.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/crypt.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/ctype.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/dirent.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/endian.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/errno.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fcntl.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/features.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/float.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdio.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdlib.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/string.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/strings.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/select.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/socket.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/unistd.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/grp.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/inttypes.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/limits.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/locale.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/math.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netdb.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netinet/in.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pthread.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pwd.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sched.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/setjmp.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/shadow.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/signal.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdarg.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdbool.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdc-predef.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stddef.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdint.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdio.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdlib.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/string.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/strings.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/cdefs.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/file.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/glibc-types.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ioctl.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ipc.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/param.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/resource.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/select.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/sem.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/socket.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/stat.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/time.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/times.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/types.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/wait.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/time.h
sv$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/unistd.h
sv$(OBJ_EXT): EXTERN.h
sv$(OBJ_EXT): av.h
sv$(OBJ_EXT): charclass_invlists.h
sv$(OBJ_EXT): config.h
sv$(OBJ_EXT): cop.h
sv$(OBJ_EXT): cv.h
sv$(OBJ_EXT): embed.h
sv$(OBJ_EXT): embedvar.h
sv$(OBJ_EXT): form.h
sv$(OBJ_EXT): gv.h
sv$(OBJ_EXT): handy.h
sv$(OBJ_EXT): hv.h
sv$(OBJ_EXT): hv_func.h
sv$(OBJ_EXT): hv_macro.h
sv$(OBJ_EXT): inline.h
sv$(OBJ_EXT): intrpvar.h
sv$(OBJ_EXT): iperlsys.h
sv$(OBJ_EXT): mg.h
sv$(OBJ_EXT): mg_vtable.h
sv$(OBJ_EXT): mydtrace.h
sv$(OBJ_EXT): op.h
sv$(OBJ_EXT): op_reg_common.h
sv$(OBJ_EXT): opcode.h
sv$(OBJ_EXT): opnames.h
sv$(OBJ_EXT): overload.h
sv$(OBJ_EXT): pad.h
sv$(OBJ_EXT): parser.h
sv$(OBJ_EXT): patchlevel.h
sv$(OBJ_EXT): perl.h
sv$(OBJ_EXT): perlio.h
sv$(OBJ_EXT): perlvars.h
sv$(OBJ_EXT): perly.h
sv$(OBJ_EXT): pp.h
sv$(OBJ_EXT): proto.h
sv$(OBJ_EXT): reentr.h
sv$(OBJ_EXT): regcharclass.h
sv$(OBJ_EXT): regcomp.h
sv$(OBJ_EXT): regexp.h
sv$(OBJ_EXT): regnodes.h
sv$(OBJ_EXT): sbox32_hash.h
sv$(OBJ_EXT): scope.h
sv$(OBJ_EXT): sv.c
sv$(OBJ_EXT): sv.h
sv$(OBJ_EXT): thread.h
sv$(OBJ_EXT): unicode_constants.h
sv$(OBJ_EXT): unixish.h
sv$(OBJ_EXT): utf8.h
sv$(OBJ_EXT): util.h
sv$(OBJ_EXT): warnings.h
sv$(OBJ_EXT): zaphod32_hash.h
sv$(OBJ_EXT): cflags
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/alloca.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/arpa/inet.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/assert.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/alltypes.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/errno.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/fcntl.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/float.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl_fix.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipc.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipcstat.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/limits.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/posix.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/resource.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/sem.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/setjmp.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/signal.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/socket.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stat.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stdint.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/crypt.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/ctype.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/dirent.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/endian.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/errno.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fcntl.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/features.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/float.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdio.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdlib.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/string.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/strings.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/select.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/socket.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/unistd.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/grp.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/inttypes.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/limits.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/locale.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/math.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netdb.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netinet/in.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pthread.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pwd.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sched.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/setjmp.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/shadow.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/signal.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdarg.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdbool.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdc-predef.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stddef.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdint.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdio.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdlib.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/string.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/strings.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/cdefs.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/file.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/glibc-types.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ioctl.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ipc.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/param.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/resource.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/select.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/sem.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/socket.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/stat.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/time.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/times.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/types.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/wait.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/time.h
taint$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/unistd.h
taint$(OBJ_EXT): EXTERN.h
taint$(OBJ_EXT): av.h
taint$(OBJ_EXT): charclass_invlists.h
taint$(OBJ_EXT): config.h
taint$(OBJ_EXT): cop.h
taint$(OBJ_EXT): cv.h
taint$(OBJ_EXT): embed.h
taint$(OBJ_EXT): embedvar.h
taint$(OBJ_EXT): form.h
taint$(OBJ_EXT): gv.h
taint$(OBJ_EXT): handy.h
taint$(OBJ_EXT): hv.h
taint$(OBJ_EXT): hv_func.h
taint$(OBJ_EXT): hv_macro.h
taint$(OBJ_EXT): inline.h
taint$(OBJ_EXT): intrpvar.h
taint$(OBJ_EXT): iperlsys.h
taint$(OBJ_EXT): mg.h
taint$(OBJ_EXT): mg_vtable.h
taint$(OBJ_EXT): mydtrace.h
taint$(OBJ_EXT): op.h
taint$(OBJ_EXT): op_reg_common.h
taint$(OBJ_EXT): opcode.h
taint$(OBJ_EXT): opnames.h
taint$(OBJ_EXT): overload.h
taint$(OBJ_EXT): pad.h
taint$(OBJ_EXT): parser.h
taint$(OBJ_EXT): patchlevel.h
taint$(OBJ_EXT): perl.h
taint$(OBJ_EXT): perlio.h
taint$(OBJ_EXT): perlvars.h
taint$(OBJ_EXT): perly.h
taint$(OBJ_EXT): pp.h
taint$(OBJ_EXT): proto.h
taint$(OBJ_EXT): reentr.h
taint$(OBJ_EXT): regcharclass.h
taint$(OBJ_EXT): regexp.h
taint$(OBJ_EXT): sbox32_hash.h
taint$(OBJ_EXT): scope.h
taint$(OBJ_EXT): sv.h
taint$(OBJ_EXT): taint.c
taint$(OBJ_EXT): thread.h
taint$(OBJ_EXT): unicode_constants.h
taint$(OBJ_EXT): unixish.h
taint$(OBJ_EXT): utf8.h
taint$(OBJ_EXT): util.h
taint$(OBJ_EXT): warnings.h
taint$(OBJ_EXT): zaphod32_hash.h
taint$(OBJ_EXT): cflags
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/alloca.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/arpa/inet.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/assert.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/alltypes.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/errno.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/fcntl.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/float.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl_fix.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipc.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipcstat.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/limits.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/posix.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/resource.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/sem.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/setjmp.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/signal.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/socket.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stat.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stdint.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/crypt.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/ctype.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/dirent.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/endian.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/errno.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fcntl.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/features.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/float.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdio.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdlib.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/string.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/strings.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/select.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/socket.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/unistd.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/grp.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/inttypes.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/limits.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/locale.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/math.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netdb.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netinet/in.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pthread.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pwd.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sched.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/setjmp.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/shadow.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/signal.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdarg.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdbool.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdc-predef.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stddef.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdint.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdio.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdlib.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/string.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/strings.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/cdefs.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/file.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/glibc-types.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ioctl.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ipc.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/param.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/resource.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/select.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/sem.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/socket.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/stat.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/time.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/times.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/types.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/wait.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/time.h
toke$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/unistd.h
toke$(OBJ_EXT): EXTERN.h
toke$(OBJ_EXT): av.h
toke$(OBJ_EXT): charclass_invlists.h
toke$(OBJ_EXT): config.h
toke$(OBJ_EXT): cop.h
toke$(OBJ_EXT): cv.h
toke$(OBJ_EXT): dquote_inline.h
toke$(OBJ_EXT): embed.h
toke$(OBJ_EXT): embedvar.h
toke$(OBJ_EXT): feature.h
toke$(OBJ_EXT): form.h
toke$(OBJ_EXT): gv.h
toke$(OBJ_EXT): handy.h
toke$(OBJ_EXT): hv.h
toke$(OBJ_EXT): hv_func.h
toke$(OBJ_EXT): hv_macro.h
toke$(OBJ_EXT): inline.h
toke$(OBJ_EXT): intrpvar.h
toke$(OBJ_EXT): invlist_inline.h
toke$(OBJ_EXT): iperlsys.h
toke$(OBJ_EXT): keywords.h
toke$(OBJ_EXT): mg.h
toke$(OBJ_EXT): mg_vtable.h
toke$(OBJ_EXT): mydtrace.h
toke$(OBJ_EXT): op.h
toke$(OBJ_EXT): op_reg_common.h
toke$(OBJ_EXT): opcode.h
toke$(OBJ_EXT): opnames.h
toke$(OBJ_EXT): overload.h
toke$(OBJ_EXT): pad.h
toke$(OBJ_EXT): parser.h
toke$(OBJ_EXT): patchlevel.h
toke$(OBJ_EXT): perl.h
toke$(OBJ_EXT): perlio.h
toke$(OBJ_EXT): perlvars.h
toke$(OBJ_EXT): perly.h
toke$(OBJ_EXT): pp.h
toke$(OBJ_EXT): proto.h
toke$(OBJ_EXT): reentr.h
toke$(OBJ_EXT): regcharclass.h
toke$(OBJ_EXT): regexp.h
toke$(OBJ_EXT): sbox32_hash.h
toke$(OBJ_EXT): scope.h
toke$(OBJ_EXT): sv.h
toke$(OBJ_EXT): thread.h
toke$(OBJ_EXT): toke.c
toke$(OBJ_EXT): unicode_constants.h
toke$(OBJ_EXT): unixish.h
toke$(OBJ_EXT): utf8.h
toke$(OBJ_EXT): util.h
toke$(OBJ_EXT): warnings.h
toke$(OBJ_EXT): zaphod32_hash.h
toke$(OBJ_EXT): cflags
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/alloca.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/arpa/inet.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/assert.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/alltypes.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/errno.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/fcntl.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/float.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl_fix.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipc.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipcstat.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/limits.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/posix.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/resource.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/sem.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/setjmp.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/signal.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/socket.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stat.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stdint.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/crypt.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/ctype.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/dirent.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/endian.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/errno.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fcntl.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/features.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/float.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdio.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdlib.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/string.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/strings.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/select.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/socket.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/unistd.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/grp.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/inttypes.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/limits.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/locale.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/math.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netdb.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netinet/in.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pthread.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pwd.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sched.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/setjmp.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/shadow.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/signal.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdarg.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdbool.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdc-predef.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stddef.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdint.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdio.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdlib.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/string.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/strings.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/cdefs.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/file.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/glibc-types.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ioctl.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ipc.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/param.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/resource.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/select.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/sem.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/socket.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/stat.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/time.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/times.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/types.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/wait.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/time.h
util$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/unistd.h
util$(OBJ_EXT): EXTERN.h
util$(OBJ_EXT): av.h
util$(OBJ_EXT): charclass_invlists.h
util$(OBJ_EXT): config.h
util$(OBJ_EXT): cop.h
util$(OBJ_EXT): cv.h
util$(OBJ_EXT): embed.h
util$(OBJ_EXT): embedvar.h
util$(OBJ_EXT): form.h
util$(OBJ_EXT): gv.h
util$(OBJ_EXT): handy.h
util$(OBJ_EXT): hv.h
util$(OBJ_EXT): hv_func.h
util$(OBJ_EXT): hv_macro.h
util$(OBJ_EXT): inline.h
util$(OBJ_EXT): intrpvar.h
util$(OBJ_EXT): iperlsys.h
util$(OBJ_EXT): mg.h
util$(OBJ_EXT): mg_vtable.h
util$(OBJ_EXT): mydtrace.h
util$(OBJ_EXT): op.h
util$(OBJ_EXT): op_reg_common.h
util$(OBJ_EXT): opcode.h
util$(OBJ_EXT): opnames.h
util$(OBJ_EXT): overload.h
util$(OBJ_EXT): pad.h
util$(OBJ_EXT): parser.h
util$(OBJ_EXT): patchlevel.h
util$(OBJ_EXT): perl.h
util$(OBJ_EXT): perlio.h
util$(OBJ_EXT): perliol.h
util$(OBJ_EXT): perlvars.h
util$(OBJ_EXT): perly.h
util$(OBJ_EXT): pp.h
util$(OBJ_EXT): proto.h
util$(OBJ_EXT): reentr.h
util$(OBJ_EXT): regcharclass.h
util$(OBJ_EXT): regexp.h
util$(OBJ_EXT): sbox32_hash.h
util$(OBJ_EXT): scope.h
util$(OBJ_EXT): sv.h
util$(OBJ_EXT): thread.h
util$(OBJ_EXT): unicode_constants.h
util$(OBJ_EXT): unixish.h
util$(OBJ_EXT): utf8.h
util$(OBJ_EXT): util.c
util$(OBJ_EXT): util.h
util$(OBJ_EXT): vutil.c
util$(OBJ_EXT): warnings.h
util$(OBJ_EXT): zaphod32_hash.h
util$(OBJ_EXT): cflags
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/alloca.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/arpa/inet.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/assert.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/alltypes.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/errno.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/fcntl.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/float.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl_fix.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipc.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipcstat.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/limits.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/posix.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/resource.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/sem.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/setjmp.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/signal.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/socket.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stat.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stdint.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/crypt.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/ctype.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/dirent.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/endian.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/errno.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fcntl.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/features.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/float.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdio.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdlib.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/string.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/strings.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/select.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/socket.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/unistd.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/grp.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/inttypes.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/limits.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/locale.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/math.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netdb.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netinet/in.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pthread.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pwd.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sched.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/setjmp.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/shadow.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/signal.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdarg.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdbool.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdc-predef.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stddef.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdint.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdio.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdlib.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/string.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/strings.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/cdefs.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/file.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/glibc-types.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ioctl.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ipc.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/param.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/resource.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/select.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/sem.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/socket.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/stat.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/time.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/times.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/types.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/wait.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/time.h
deb$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/unistd.h
deb$(OBJ_EXT): EXTERN.h
deb$(OBJ_EXT): av.h
deb$(OBJ_EXT): charclass_invlists.h
deb$(OBJ_EXT): config.h
deb$(OBJ_EXT): cop.h
deb$(OBJ_EXT): cv.h
deb$(OBJ_EXT): deb.c
deb$(OBJ_EXT): embed.h
deb$(OBJ_EXT): embedvar.h
deb$(OBJ_EXT): form.h
deb$(OBJ_EXT): gv.h
deb$(OBJ_EXT): handy.h
deb$(OBJ_EXT): hv.h
deb$(OBJ_EXT): hv_func.h
deb$(OBJ_EXT): hv_macro.h
deb$(OBJ_EXT): inline.h
deb$(OBJ_EXT): intrpvar.h
deb$(OBJ_EXT): iperlsys.h
deb$(OBJ_EXT): mg.h
deb$(OBJ_EXT): mg_vtable.h
deb$(OBJ_EXT): mydtrace.h
deb$(OBJ_EXT): op.h
deb$(OBJ_EXT): op_reg_common.h
deb$(OBJ_EXT): opcode.h
deb$(OBJ_EXT): opnames.h
deb$(OBJ_EXT): overload.h
deb$(OBJ_EXT): pad.h
deb$(OBJ_EXT): parser.h
deb$(OBJ_EXT): patchlevel.h
deb$(OBJ_EXT): perl.h
deb$(OBJ_EXT): perlio.h
deb$(OBJ_EXT): perlvars.h
deb$(OBJ_EXT): perly.h
deb$(OBJ_EXT): pp.h
deb$(OBJ_EXT): proto.h
deb$(OBJ_EXT): reentr.h
deb$(OBJ_EXT): regcharclass.h
deb$(OBJ_EXT): regexp.h
deb$(OBJ_EXT): sbox32_hash.h
deb$(OBJ_EXT): scope.h
deb$(OBJ_EXT): sv.h
deb$(OBJ_EXT): thread.h
deb$(OBJ_EXT): unicode_constants.h
deb$(OBJ_EXT): unixish.h
deb$(OBJ_EXT): utf8.h
deb$(OBJ_EXT): util.h
deb$(OBJ_EXT): warnings.h
deb$(OBJ_EXT): zaphod32_hash.h
deb$(OBJ_EXT): cflags
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/alloca.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/arpa/inet.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/assert.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/alltypes.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/errno.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/fcntl.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/float.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl_fix.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipc.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipcstat.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/limits.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/posix.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/resource.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/sem.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/setjmp.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/signal.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/socket.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stat.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stdint.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/crypt.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/ctype.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/dirent.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/endian.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/errno.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fcntl.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/features.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/float.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdio.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdlib.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/string.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/strings.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/select.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/socket.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/unistd.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/grp.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/inttypes.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/limits.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/locale.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/math.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netdb.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netinet/in.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pthread.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pwd.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sched.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/setjmp.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/shadow.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/signal.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdarg.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdbool.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdc-predef.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stddef.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdint.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdio.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdlib.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/string.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/strings.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/cdefs.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/file.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/glibc-types.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ioctl.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ipc.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/param.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/resource.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/select.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/sem.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/socket.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/stat.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/time.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/times.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/types.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/wait.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/time.h
run$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/unistd.h
run$(OBJ_EXT): EXTERN.h
run$(OBJ_EXT): av.h
run$(OBJ_EXT): charclass_invlists.h
run$(OBJ_EXT): config.h
run$(OBJ_EXT): cop.h
run$(OBJ_EXT): cv.h
run$(OBJ_EXT): embed.h
run$(OBJ_EXT): embedvar.h
run$(OBJ_EXT): form.h
run$(OBJ_EXT): gv.h
run$(OBJ_EXT): handy.h
run$(OBJ_EXT): hv.h
run$(OBJ_EXT): hv_func.h
run$(OBJ_EXT): hv_macro.h
run$(OBJ_EXT): inline.h
run$(OBJ_EXT): intrpvar.h
run$(OBJ_EXT): iperlsys.h
run$(OBJ_EXT): mg.h
run$(OBJ_EXT): mg_vtable.h
run$(OBJ_EXT): mydtrace.h
run$(OBJ_EXT): op.h
run$(OBJ_EXT): op_reg_common.h
run$(OBJ_EXT): opcode.h
run$(OBJ_EXT): opnames.h
run$(OBJ_EXT): overload.h
run$(OBJ_EXT): pad.h
run$(OBJ_EXT): parser.h
run$(OBJ_EXT): patchlevel.h
run$(OBJ_EXT): perl.h
run$(OBJ_EXT): perlio.h
run$(OBJ_EXT): perlvars.h
run$(OBJ_EXT): perly.h
run$(OBJ_EXT): pp.h
run$(OBJ_EXT): proto.h
run$(OBJ_EXT): reentr.h
run$(OBJ_EXT): regcharclass.h
run$(OBJ_EXT): regexp.h
run$(OBJ_EXT): run.c
run$(OBJ_EXT): sbox32_hash.h
run$(OBJ_EXT): scope.h
run$(OBJ_EXT): sv.h
run$(OBJ_EXT): thread.h
run$(OBJ_EXT): unicode_constants.h
run$(OBJ_EXT): unixish.h
run$(OBJ_EXT): utf8.h
run$(OBJ_EXT): util.h
run$(OBJ_EXT): warnings.h
run$(OBJ_EXT): zaphod32_hash.h
run$(OBJ_EXT): cflags
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/alloca.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/arpa/inet.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/assert.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/alltypes.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/errno.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/fcntl.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/float.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl_fix.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipc.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipcstat.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/limits.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/posix.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/resource.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/sem.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/setjmp.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/signal.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/socket.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stat.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stdint.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/crypt.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/ctype.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/dirent.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/endian.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/errno.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fcntl.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/features.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/float.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdio.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdlib.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/string.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/strings.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/select.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/socket.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/unistd.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/grp.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/inttypes.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/limits.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/locale.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/math.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netdb.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netinet/in.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pthread.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pwd.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sched.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/setjmp.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/shadow.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/signal.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdarg.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdbool.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdc-predef.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stddef.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdint.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdio.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdlib.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/string.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/strings.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/cdefs.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/file.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/glibc-types.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ioctl.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ipc.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/param.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/resource.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/select.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/sem.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/socket.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/stat.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/time.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/times.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/types.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/wait.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/time.h
universal$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/unistd.h
universal$(OBJ_EXT): EXTERN.h
universal$(OBJ_EXT): XSUB.h
universal$(OBJ_EXT): av.h
universal$(OBJ_EXT): charclass_invlists.h
universal$(OBJ_EXT): config.h
universal$(OBJ_EXT): cop.h
universal$(OBJ_EXT): cv.h
universal$(OBJ_EXT): embed.h
universal$(OBJ_EXT): embedvar.h
universal$(OBJ_EXT): form.h
universal$(OBJ_EXT): gv.h
universal$(OBJ_EXT): handy.h
universal$(OBJ_EXT): hv.h
universal$(OBJ_EXT): hv_func.h
universal$(OBJ_EXT): hv_macro.h
universal$(OBJ_EXT): inline.h
universal$(OBJ_EXT): intrpvar.h
universal$(OBJ_EXT): iperlsys.h
universal$(OBJ_EXT): mg.h
universal$(OBJ_EXT): mg_vtable.h
universal$(OBJ_EXT): mydtrace.h
universal$(OBJ_EXT): op.h
universal$(OBJ_EXT): op_reg_common.h
universal$(OBJ_EXT): opcode.h
universal$(OBJ_EXT): opnames.h
universal$(OBJ_EXT): overload.h
universal$(OBJ_EXT): pad.h
universal$(OBJ_EXT): parser.h
universal$(OBJ_EXT): patchlevel.h
universal$(OBJ_EXT): perl.h
universal$(OBJ_EXT): perlapi.h
universal$(OBJ_EXT): perlio.h
universal$(OBJ_EXT): perliol.h
universal$(OBJ_EXT): perlvars.h
universal$(OBJ_EXT): perly.h
universal$(OBJ_EXT): pp.h
universal$(OBJ_EXT): proto.h
universal$(OBJ_EXT): reentr.h
universal$(OBJ_EXT): regcharclass.h
universal$(OBJ_EXT): regexp.h
universal$(OBJ_EXT): sbox32_hash.h
universal$(OBJ_EXT): scope.h
universal$(OBJ_EXT): sv.h
universal$(OBJ_EXT): thread.h
universal$(OBJ_EXT): unicode_constants.h
universal$(OBJ_EXT): universal.c
universal$(OBJ_EXT): unixish.h
universal$(OBJ_EXT): utf8.h
universal$(OBJ_EXT): util.h
universal$(OBJ_EXT): vutil.h
universal$(OBJ_EXT): warnings.h
universal$(OBJ_EXT): zaphod32_hash.h
universal$(OBJ_EXT): cflags
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/alloca.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/arpa/inet.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/assert.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/alltypes.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/errno.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/fcntl.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/float.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl_fix.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipc.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipcstat.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/limits.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/posix.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/resource.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/sem.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/setjmp.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/signal.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/socket.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stat.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stdint.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/crypt.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/ctype.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/dirent.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/endian.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/errno.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fcntl.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/features.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/float.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdio.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdlib.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/string.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/strings.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/select.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/socket.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/unistd.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/grp.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/inttypes.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/limits.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/locale.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/math.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netdb.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netinet/in.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pthread.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pwd.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sched.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/setjmp.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/shadow.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/signal.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdarg.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdbool.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdc-predef.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stddef.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdint.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdio.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdlib.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/string.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/strings.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/cdefs.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/file.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/glibc-types.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ioctl.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ipc.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/param.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/resource.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/select.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/sem.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/socket.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/stat.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/time.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/times.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/types.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/wait.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/time.h
pad$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/unistd.h
pad$(OBJ_EXT): EXTERN.h
pad$(OBJ_EXT): av.h
pad$(OBJ_EXT): charclass_invlists.h
pad$(OBJ_EXT): config.h
pad$(OBJ_EXT): cop.h
pad$(OBJ_EXT): cv.h
pad$(OBJ_EXT): embed.h
pad$(OBJ_EXT): embedvar.h
pad$(OBJ_EXT): form.h
pad$(OBJ_EXT): gv.h
pad$(OBJ_EXT): handy.h
pad$(OBJ_EXT): hv.h
pad$(OBJ_EXT): hv_func.h
pad$(OBJ_EXT): hv_macro.h
pad$(OBJ_EXT): inline.h
pad$(OBJ_EXT): intrpvar.h
pad$(OBJ_EXT): iperlsys.h
pad$(OBJ_EXT): keywords.h
pad$(OBJ_EXT): mg.h
pad$(OBJ_EXT): mg_vtable.h
pad$(OBJ_EXT): mydtrace.h
pad$(OBJ_EXT): op.h
pad$(OBJ_EXT): op_reg_common.h
pad$(OBJ_EXT): opcode.h
pad$(OBJ_EXT): opnames.h
pad$(OBJ_EXT): overload.h
pad$(OBJ_EXT): pad.c
pad$(OBJ_EXT): pad.h
pad$(OBJ_EXT): parser.h
pad$(OBJ_EXT): patchlevel.h
pad$(OBJ_EXT): perl.h
pad$(OBJ_EXT): perlio.h
pad$(OBJ_EXT): perlvars.h
pad$(OBJ_EXT): perly.h
pad$(OBJ_EXT): pp.h
pad$(OBJ_EXT): proto.h
pad$(OBJ_EXT): reentr.h
pad$(OBJ_EXT): regcharclass.h
pad$(OBJ_EXT): regexp.h
pad$(OBJ_EXT): sbox32_hash.h
pad$(OBJ_EXT): scope.h
pad$(OBJ_EXT): sv.h
pad$(OBJ_EXT): thread.h
pad$(OBJ_EXT): unicode_constants.h
pad$(OBJ_EXT): unixish.h
pad$(OBJ_EXT): utf8.h
pad$(OBJ_EXT): util.h
pad$(OBJ_EXT): warnings.h
pad$(OBJ_EXT): zaphod32_hash.h
pad$(OBJ_EXT): cflags
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/alloca.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/arpa/inet.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/assert.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/alltypes.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/errno.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/fcntl.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/float.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl_fix.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipc.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipcstat.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/limits.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/posix.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/resource.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/sem.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/setjmp.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/signal.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/socket.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stat.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stdint.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/crypt.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/ctype.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/dirent.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/endian.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/errno.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fcntl.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/features.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/float.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdio.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdlib.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/string.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/strings.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/select.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/socket.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/unistd.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/grp.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/inttypes.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/limits.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/locale.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/math.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netdb.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netinet/in.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pthread.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pwd.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sched.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/setjmp.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/shadow.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/signal.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdarg.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdbool.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdc-predef.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stddef.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdint.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdio.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdlib.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/string.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/strings.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/cdefs.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/file.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/glibc-types.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ioctl.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ipc.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/param.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/resource.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/select.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/sem.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/socket.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/stat.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/time.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/times.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/types.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/wait.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/time.h
globals$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/unistd.h
globals$(OBJ_EXT): INTERN.h
globals$(OBJ_EXT): av.h
globals$(OBJ_EXT): bitcount.h
globals$(OBJ_EXT): charclass_invlists.h
globals$(OBJ_EXT): config.h
globals$(OBJ_EXT): cop.h
globals$(OBJ_EXT): cv.h
globals$(OBJ_EXT): embed.h
globals$(OBJ_EXT): embedvar.h
globals$(OBJ_EXT): form.h
globals$(OBJ_EXT): globals.c
globals$(OBJ_EXT): gv.h
globals$(OBJ_EXT): handy.h
globals$(OBJ_EXT): hv.h
globals$(OBJ_EXT): hv_func.h
globals$(OBJ_EXT): hv_macro.h
globals$(OBJ_EXT): inline.h
globals$(OBJ_EXT): intrpvar.h
globals$(OBJ_EXT): iperlsys.h
globals$(OBJ_EXT): l1_char_class_tab.h
globals$(OBJ_EXT): mg.h
globals$(OBJ_EXT): mg_data.h
globals$(OBJ_EXT): mg_vtable.h
globals$(OBJ_EXT): mydtrace.h
globals$(OBJ_EXT): op.h
globals$(OBJ_EXT): op_reg_common.h
globals$(OBJ_EXT): opcode.h
globals$(OBJ_EXT): opnames.h
globals$(OBJ_EXT): overload.h
globals$(OBJ_EXT): pad.h
globals$(OBJ_EXT): parser.h
globals$(OBJ_EXT): patchlevel.h
globals$(OBJ_EXT): perl.h
globals$(OBJ_EXT): perlapi.h
globals$(OBJ_EXT): perlio.h
globals$(OBJ_EXT): perlvars.h
globals$(OBJ_EXT): perly.h
globals$(OBJ_EXT): pp.h
globals$(OBJ_EXT): proto.h
globals$(OBJ_EXT): reentr.h
globals$(OBJ_EXT): regcharclass.h
globals$(OBJ_EXT): regexp.h
globals$(OBJ_EXT): sbox32_hash.h
globals$(OBJ_EXT): scope.h
globals$(OBJ_EXT): sv.h
globals$(OBJ_EXT): thread.h
globals$(OBJ_EXT): unicode_constants.h
globals$(OBJ_EXT): unixish.h
globals$(OBJ_EXT): utf8.h
globals$(OBJ_EXT): util.h
globals$(OBJ_EXT): uudmap.h
globals$(OBJ_EXT): warnings.h
globals$(OBJ_EXT): zaphod32_hash.h
globals$(OBJ_EXT): cflags
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/alloca.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/arpa/inet.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/assert.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/alltypes.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/errno.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/fcntl.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/float.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl_fix.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipc.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipcstat.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/limits.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/posix.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/resource.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/sem.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/setjmp.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/signal.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/socket.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stat.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stdint.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/crypt.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/ctype.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/dirent.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/endian.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/errno.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fcntl.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/features.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/float.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdio.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdlib.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/string.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/strings.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/select.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/socket.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/unistd.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/grp.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/inttypes.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/limits.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/locale.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/math.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netdb.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netinet/in.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pthread.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pwd.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sched.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/setjmp.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/shadow.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/signal.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdarg.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdbool.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdc-predef.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stddef.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdint.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdio.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdlib.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/string.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/strings.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/cdefs.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/file.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/glibc-types.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ioctl.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ipc.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/param.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/resource.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/select.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/sem.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/socket.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/stat.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/time.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/times.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/types.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/wait.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/time.h
keywords$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/unistd.h
keywords$(OBJ_EXT): EXTERN.h
keywords$(OBJ_EXT): av.h
keywords$(OBJ_EXT): charclass_invlists.h
keywords$(OBJ_EXT): config.h
keywords$(OBJ_EXT): cop.h
keywords$(OBJ_EXT): cv.h
keywords$(OBJ_EXT): embed.h
keywords$(OBJ_EXT): embedvar.h
keywords$(OBJ_EXT): feature.h
keywords$(OBJ_EXT): form.h
keywords$(OBJ_EXT): gv.h
keywords$(OBJ_EXT): handy.h
keywords$(OBJ_EXT): hv.h
keywords$(OBJ_EXT): hv_func.h
keywords$(OBJ_EXT): hv_macro.h
keywords$(OBJ_EXT): inline.h
keywords$(OBJ_EXT): intrpvar.h
keywords$(OBJ_EXT): iperlsys.h
keywords$(OBJ_EXT): keywords.c
keywords$(OBJ_EXT): keywords.h
keywords$(OBJ_EXT): mg.h
keywords$(OBJ_EXT): mg_vtable.h
keywords$(OBJ_EXT): mydtrace.h
keywords$(OBJ_EXT): op.h
keywords$(OBJ_EXT): op_reg_common.h
keywords$(OBJ_EXT): opcode.h
keywords$(OBJ_EXT): opnames.h
keywords$(OBJ_EXT): overload.h
keywords$(OBJ_EXT): pad.h
keywords$(OBJ_EXT): parser.h
keywords$(OBJ_EXT): patchlevel.h
keywords$(OBJ_EXT): perl.h
keywords$(OBJ_EXT): perlio.h
keywords$(OBJ_EXT): perlvars.h
keywords$(OBJ_EXT): perly.h
keywords$(OBJ_EXT): pp.h
keywords$(OBJ_EXT): proto.h
keywords$(OBJ_EXT): reentr.h
keywords$(OBJ_EXT): regcharclass.h
keywords$(OBJ_EXT): regexp.h
keywords$(OBJ_EXT): sbox32_hash.h
keywords$(OBJ_EXT): scope.h
keywords$(OBJ_EXT): sv.h
keywords$(OBJ_EXT): thread.h
keywords$(OBJ_EXT): unicode_constants.h
keywords$(OBJ_EXT): unixish.h
keywords$(OBJ_EXT): utf8.h
keywords$(OBJ_EXT): util.h
keywords$(OBJ_EXT): warnings.h
keywords$(OBJ_EXT): zaphod32_hash.h
keywords$(OBJ_EXT): cflags
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/alloca.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/arpa/inet.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/assert.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/alltypes.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/errno.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/fcntl.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/float.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl_fix.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipc.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipcstat.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/limits.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/posix.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/resource.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/sem.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/setjmp.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/signal.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/socket.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stat.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stdint.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/crypt.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/ctype.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/dirent.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/endian.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/errno.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fcntl.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/features.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/float.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdio.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdlib.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/string.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/strings.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/select.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/socket.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/unistd.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/grp.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/inttypes.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/limits.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/locale.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/math.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netdb.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netinet/in.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pthread.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pwd.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sched.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/setjmp.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/shadow.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/signal.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdarg.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdbool.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdc-predef.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stddef.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdint.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdio.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdlib.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/string.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/strings.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/cdefs.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/file.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/glibc-types.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ioctl.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ipc.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/param.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/resource.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/select.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/sem.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/socket.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/stat.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/time.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/times.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/types.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/wait.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/time.h
perlio$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/unistd.h
perlio$(OBJ_EXT): EXTERN.h
perlio$(OBJ_EXT): XSUB.h
perlio$(OBJ_EXT): av.h
perlio$(OBJ_EXT): charclass_invlists.h
perlio$(OBJ_EXT): config.h
perlio$(OBJ_EXT): cop.h
perlio$(OBJ_EXT): cv.h
perlio$(OBJ_EXT): embed.h
perlio$(OBJ_EXT): embedvar.h
perlio$(OBJ_EXT): form.h
perlio$(OBJ_EXT): gv.h
perlio$(OBJ_EXT): handy.h
perlio$(OBJ_EXT): hv.h
perlio$(OBJ_EXT): hv_func.h
perlio$(OBJ_EXT): hv_macro.h
perlio$(OBJ_EXT): inline.h
perlio$(OBJ_EXT): intrpvar.h
perlio$(OBJ_EXT): iperlsys.h
perlio$(OBJ_EXT): mg.h
perlio$(OBJ_EXT): mg_vtable.h
perlio$(OBJ_EXT): mydtrace.h
perlio$(OBJ_EXT): op.h
perlio$(OBJ_EXT): op_reg_common.h
perlio$(OBJ_EXT): opcode.h
perlio$(OBJ_EXT): opnames.h
perlio$(OBJ_EXT): overload.h
perlio$(OBJ_EXT): pad.h
perlio$(OBJ_EXT): parser.h
perlio$(OBJ_EXT): patchlevel.h
perlio$(OBJ_EXT): perl.h
perlio$(OBJ_EXT): perlapi.h
perlio$(OBJ_EXT): perlio.c
perlio$(OBJ_EXT): perlio.h
perlio$(OBJ_EXT): perliol.h
perlio$(OBJ_EXT): perlvars.h
perlio$(OBJ_EXT): perly.h
perlio$(OBJ_EXT): pp.h
perlio$(OBJ_EXT): proto.h
perlio$(OBJ_EXT): reentr.h
perlio$(OBJ_EXT): regcharclass.h
perlio$(OBJ_EXT): regexp.h
perlio$(OBJ_EXT): sbox32_hash.h
perlio$(OBJ_EXT): scope.h
perlio$(OBJ_EXT): sv.h
perlio$(OBJ_EXT): thread.h
perlio$(OBJ_EXT): unicode_constants.h
perlio$(OBJ_EXT): unixish.h
perlio$(OBJ_EXT): utf8.h
perlio$(OBJ_EXT): util.h
perlio$(OBJ_EXT): warnings.h
perlio$(OBJ_EXT): zaphod32_hash.h
perlio$(OBJ_EXT): cflags
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/alloca.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/arpa/inet.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/assert.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/alltypes.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/errno.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/fcntl.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/float.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl_fix.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipc.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipcstat.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/limits.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/posix.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/resource.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/sem.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/setjmp.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/signal.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/socket.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stat.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stdint.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/crypt.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/ctype.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/dirent.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/endian.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/errno.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fcntl.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/features.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/float.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdio.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdlib.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/string.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/strings.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/select.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/socket.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/unistd.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/grp.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/inttypes.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/limits.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/locale.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/math.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netdb.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netinet/in.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pthread.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pwd.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sched.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/setjmp.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/shadow.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/signal.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdarg.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdbool.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdc-predef.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stddef.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdint.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdio.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdlib.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/string.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/strings.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/cdefs.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/file.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/glibc-types.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ioctl.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ipc.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/param.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/resource.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/select.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/sem.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/socket.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/stat.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/time.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/times.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/types.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/wait.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/time.h
perlapi$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/unistd.h
perlapi$(OBJ_EXT): EXTERN.h
perlapi$(OBJ_EXT): av.h
perlapi$(OBJ_EXT): charclass_invlists.h
perlapi$(OBJ_EXT): config.h
perlapi$(OBJ_EXT): cop.h
perlapi$(OBJ_EXT): cv.h
perlapi$(OBJ_EXT): embed.h
perlapi$(OBJ_EXT): embedvar.h
perlapi$(OBJ_EXT): form.h
perlapi$(OBJ_EXT): gv.h
perlapi$(OBJ_EXT): handy.h
perlapi$(OBJ_EXT): hv.h
perlapi$(OBJ_EXT): hv_func.h
perlapi$(OBJ_EXT): hv_macro.h
perlapi$(OBJ_EXT): inline.h
perlapi$(OBJ_EXT): intrpvar.h
perlapi$(OBJ_EXT): iperlsys.h
perlapi$(OBJ_EXT): mg.h
perlapi$(OBJ_EXT): mg_vtable.h
perlapi$(OBJ_EXT): mydtrace.h
perlapi$(OBJ_EXT): op.h
perlapi$(OBJ_EXT): op_reg_common.h
perlapi$(OBJ_EXT): opcode.h
perlapi$(OBJ_EXT): opnames.h
perlapi$(OBJ_EXT): overload.h
perlapi$(OBJ_EXT): pad.h
perlapi$(OBJ_EXT): parser.h
perlapi$(OBJ_EXT): patchlevel.h
perlapi$(OBJ_EXT): perl.h
perlapi$(OBJ_EXT): perlapi.c
perlapi$(OBJ_EXT): perlapi.h
perlapi$(OBJ_EXT): perlio.h
perlapi$(OBJ_EXT): perlvars.h
perlapi$(OBJ_EXT): perly.h
perlapi$(OBJ_EXT): pp.h
perlapi$(OBJ_EXT): proto.h
perlapi$(OBJ_EXT): reentr.h
perlapi$(OBJ_EXT): regcharclass.h
perlapi$(OBJ_EXT): regexp.h
perlapi$(OBJ_EXT): sbox32_hash.h
perlapi$(OBJ_EXT): scope.h
perlapi$(OBJ_EXT): sv.h
perlapi$(OBJ_EXT): thread.h
perlapi$(OBJ_EXT): unicode_constants.h
perlapi$(OBJ_EXT): unixish.h
perlapi$(OBJ_EXT): utf8.h
perlapi$(OBJ_EXT): util.h
perlapi$(OBJ_EXT): warnings.h
perlapi$(OBJ_EXT): zaphod32_hash.h
perlapi$(OBJ_EXT): cflags
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/alloca.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/arpa/inet.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/assert.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/alltypes.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/errno.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/fcntl.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/float.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl_fix.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipc.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipcstat.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/limits.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/posix.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/resource.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/sem.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/setjmp.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/signal.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/socket.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stat.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stdint.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/crypt.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/ctype.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/dirent.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/endian.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/errno.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fcntl.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/features.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/float.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdio.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdlib.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/string.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/strings.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/select.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/socket.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/unistd.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/grp.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/inttypes.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/limits.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/locale.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/math.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netdb.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netinet/in.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pthread.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pwd.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sched.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/setjmp.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/shadow.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/signal.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdarg.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdbool.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdc-predef.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stddef.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdint.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdio.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdlib.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/string.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/strings.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/cdefs.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/file.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/glibc-types.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ioctl.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ipc.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/param.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/resource.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/select.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/sem.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/socket.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/stat.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/time.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/times.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/types.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/wait.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/time.h
numeric$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/unistd.h
numeric$(OBJ_EXT): EXTERN.h
numeric$(OBJ_EXT): av.h
numeric$(OBJ_EXT): charclass_invlists.h
numeric$(OBJ_EXT): config.h
numeric$(OBJ_EXT): cop.h
numeric$(OBJ_EXT): cv.h
numeric$(OBJ_EXT): embed.h
numeric$(OBJ_EXT): embedvar.h
numeric$(OBJ_EXT): form.h
numeric$(OBJ_EXT): gv.h
numeric$(OBJ_EXT): handy.h
numeric$(OBJ_EXT): hv.h
numeric$(OBJ_EXT): hv_func.h
numeric$(OBJ_EXT): hv_macro.h
numeric$(OBJ_EXT): inline.h
numeric$(OBJ_EXT): intrpvar.h
numeric$(OBJ_EXT): iperlsys.h
numeric$(OBJ_EXT): mg.h
numeric$(OBJ_EXT): mg_vtable.h
numeric$(OBJ_EXT): mydtrace.h
numeric$(OBJ_EXT): numeric.c
numeric$(OBJ_EXT): op.h
numeric$(OBJ_EXT): op_reg_common.h
numeric$(OBJ_EXT): opcode.h
numeric$(OBJ_EXT): opnames.h
numeric$(OBJ_EXT): overload.h
numeric$(OBJ_EXT): pad.h
numeric$(OBJ_EXT): parser.h
numeric$(OBJ_EXT): patchlevel.h
numeric$(OBJ_EXT): perl.h
numeric$(OBJ_EXT): perlio.h
numeric$(OBJ_EXT): perlvars.h
numeric$(OBJ_EXT): perly.h
numeric$(OBJ_EXT): pp.h
numeric$(OBJ_EXT): proto.h
numeric$(OBJ_EXT): reentr.h
numeric$(OBJ_EXT): regcharclass.h
numeric$(OBJ_EXT): regexp.h
numeric$(OBJ_EXT): sbox32_hash.h
numeric$(OBJ_EXT): scope.h
numeric$(OBJ_EXT): sv.h
numeric$(OBJ_EXT): thread.h
numeric$(OBJ_EXT): unicode_constants.h
numeric$(OBJ_EXT): unixish.h
numeric$(OBJ_EXT): utf8.h
numeric$(OBJ_EXT): util.h
numeric$(OBJ_EXT): warnings.h
numeric$(OBJ_EXT): zaphod32_hash.h
numeric$(OBJ_EXT): cflags
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/alloca.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/arpa/inet.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/assert.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/alltypes.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/errno.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/fcntl.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/float.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl_fix.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipc.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipcstat.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/limits.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/posix.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/resource.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/sem.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/setjmp.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/signal.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/socket.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stat.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stdint.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/crypt.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/ctype.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/dirent.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/endian.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/errno.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fcntl.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/features.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/float.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdio.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdlib.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/string.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/strings.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/select.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/socket.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/unistd.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/grp.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/inttypes.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/limits.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/locale.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/math.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netdb.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netinet/in.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pthread.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pwd.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sched.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/setjmp.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/shadow.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/signal.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdarg.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdbool.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdc-predef.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stddef.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdint.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdio.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdlib.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/string.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/strings.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/cdefs.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/file.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/glibc-types.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ioctl.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ipc.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/param.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/resource.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/select.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/sem.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/socket.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/stat.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/time.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/times.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/types.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/wait.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/time.h
mathoms$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/unistd.h
mathoms$(OBJ_EXT): EXTERN.h
mathoms$(OBJ_EXT): av.h
mathoms$(OBJ_EXT): charclass_invlists.h
mathoms$(OBJ_EXT): config.h
mathoms$(OBJ_EXT): cop.h
mathoms$(OBJ_EXT): cv.h
mathoms$(OBJ_EXT): embed.h
mathoms$(OBJ_EXT): embedvar.h
mathoms$(OBJ_EXT): form.h
mathoms$(OBJ_EXT): gv.h
mathoms$(OBJ_EXT): handy.h
mathoms$(OBJ_EXT): hv.h
mathoms$(OBJ_EXT): hv_func.h
mathoms$(OBJ_EXT): hv_macro.h
mathoms$(OBJ_EXT): inline.h
mathoms$(OBJ_EXT): intrpvar.h
mathoms$(OBJ_EXT): iperlsys.h
mathoms$(OBJ_EXT): mathoms.c
mathoms$(OBJ_EXT): mg.h
mathoms$(OBJ_EXT): mg_vtable.h
mathoms$(OBJ_EXT): mydtrace.h
mathoms$(OBJ_EXT): op.h
mathoms$(OBJ_EXT): op_reg_common.h
mathoms$(OBJ_EXT): opcode.h
mathoms$(OBJ_EXT): opnames.h
mathoms$(OBJ_EXT): overload.h
mathoms$(OBJ_EXT): pad.h
mathoms$(OBJ_EXT): parser.h
mathoms$(OBJ_EXT): patchlevel.h
mathoms$(OBJ_EXT): perl.h
mathoms$(OBJ_EXT): perlio.h
mathoms$(OBJ_EXT): perlvars.h
mathoms$(OBJ_EXT): perly.h
mathoms$(OBJ_EXT): pp.h
mathoms$(OBJ_EXT): proto.h
mathoms$(OBJ_EXT): reentr.h
mathoms$(OBJ_EXT): regcharclass.h
mathoms$(OBJ_EXT): regexp.h
mathoms$(OBJ_EXT): sbox32_hash.h
mathoms$(OBJ_EXT): scope.h
mathoms$(OBJ_EXT): sv.h
mathoms$(OBJ_EXT): thread.h
mathoms$(OBJ_EXT): unicode_constants.h
mathoms$(OBJ_EXT): unixish.h
mathoms$(OBJ_EXT): utf8.h
mathoms$(OBJ_EXT): util.h
mathoms$(OBJ_EXT): warnings.h
mathoms$(OBJ_EXT): zaphod32_hash.h
mathoms$(OBJ_EXT): cflags
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/alloca.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/arpa/inet.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/assert.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/alltypes.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/errno.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/fcntl.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/float.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl_fix.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipc.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipcstat.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/limits.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/posix.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/resource.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/sem.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/setjmp.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/signal.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/socket.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stat.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stdint.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/crypt.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/ctype.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/dirent.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/endian.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/errno.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fcntl.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/features.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/float.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdio.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdlib.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/string.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/strings.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/select.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/socket.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/unistd.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/wchar.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/grp.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/inttypes.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/langinfo.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/limits.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/locale.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/math.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netdb.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netinet/in.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/nl_types.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pthread.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pwd.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sched.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/setjmp.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/shadow.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/signal.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdarg.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdbool.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdc-predef.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stddef.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdint.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdio.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdlib.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/string.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/strings.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/cdefs.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/file.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/glibc-types.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ioctl.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ipc.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/param.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/resource.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/select.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/sem.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/socket.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/stat.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/time.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/times.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/types.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/wait.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/time.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/unistd.h
locale$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/wchar.h
locale$(OBJ_EXT): EXTERN.h
locale$(OBJ_EXT): av.h
locale$(OBJ_EXT): charclass_invlists.h
locale$(OBJ_EXT): config.h
locale$(OBJ_EXT): cop.h
locale$(OBJ_EXT): cv.h
locale$(OBJ_EXT): embed.h
locale$(OBJ_EXT): embedvar.h
locale$(OBJ_EXT): form.h
locale$(OBJ_EXT): gv.h
locale$(OBJ_EXT): handy.h
locale$(OBJ_EXT): hv.h
locale$(OBJ_EXT): hv_func.h
locale$(OBJ_EXT): hv_macro.h
locale$(OBJ_EXT): inline.h
locale$(OBJ_EXT): intrpvar.h
locale$(OBJ_EXT): iperlsys.h
locale$(OBJ_EXT): locale.c
locale$(OBJ_EXT): mg.h
locale$(OBJ_EXT): mg_vtable.h
locale$(OBJ_EXT): mydtrace.h
locale$(OBJ_EXT): op.h
locale$(OBJ_EXT): op_reg_common.h
locale$(OBJ_EXT): opcode.h
locale$(OBJ_EXT): opnames.h
locale$(OBJ_EXT): overload.h
locale$(OBJ_EXT): pad.h
locale$(OBJ_EXT): parser.h
locale$(OBJ_EXT): patchlevel.h
locale$(OBJ_EXT): perl.h
locale$(OBJ_EXT): perl_langinfo.h
locale$(OBJ_EXT): perlio.h
locale$(OBJ_EXT): perlvars.h
locale$(OBJ_EXT): perly.h
locale$(OBJ_EXT): pp.h
locale$(OBJ_EXT): proto.h
locale$(OBJ_EXT): reentr.h
locale$(OBJ_EXT): regcharclass.h
locale$(OBJ_EXT): regexp.h
locale$(OBJ_EXT): sbox32_hash.h
locale$(OBJ_EXT): scope.h
locale$(OBJ_EXT): sv.h
locale$(OBJ_EXT): thread.h
locale$(OBJ_EXT): unicode_constants.h
locale$(OBJ_EXT): unixish.h
locale$(OBJ_EXT): utf8.h
locale$(OBJ_EXT): util.h
locale$(OBJ_EXT): warnings.h
locale$(OBJ_EXT): zaphod32_hash.h
locale$(OBJ_EXT): cflags
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/alloca.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/arpa/inet.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/assert.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/alltypes.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/errno.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/fcntl.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/float.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl_fix.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipc.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipcstat.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/limits.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/posix.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/resource.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/sem.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/setjmp.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/signal.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/socket.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stat.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stdint.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/crypt.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/ctype.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/dirent.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/endian.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/errno.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fcntl.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/features.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/float.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdio.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdlib.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/string.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/strings.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/select.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/socket.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/unistd.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/grp.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/inttypes.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/limits.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/locale.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/math.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netdb.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netinet/in.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pthread.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pwd.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sched.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/setjmp.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/shadow.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/signal.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdarg.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdbool.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdc-predef.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stddef.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdint.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdio.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdlib.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/string.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/strings.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/cdefs.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/file.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/glibc-types.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ioctl.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ipc.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/param.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/resource.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/select.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/sem.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/socket.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/stat.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/time.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/times.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/types.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/wait.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/time.h
pp_pack$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/unistd.h
pp_pack$(OBJ_EXT): EXTERN.h
pp_pack$(OBJ_EXT): av.h
pp_pack$(OBJ_EXT): charclass_invlists.h
pp_pack$(OBJ_EXT): config.h
pp_pack$(OBJ_EXT): cop.h
pp_pack$(OBJ_EXT): cv.h
pp_pack$(OBJ_EXT): embed.h
pp_pack$(OBJ_EXT): embedvar.h
pp_pack$(OBJ_EXT): form.h
pp_pack$(OBJ_EXT): gv.h
pp_pack$(OBJ_EXT): handy.h
pp_pack$(OBJ_EXT): hv.h
pp_pack$(OBJ_EXT): hv_func.h
pp_pack$(OBJ_EXT): hv_macro.h
pp_pack$(OBJ_EXT): inline.h
pp_pack$(OBJ_EXT): intrpvar.h
pp_pack$(OBJ_EXT): iperlsys.h
pp_pack$(OBJ_EXT): mg.h
pp_pack$(OBJ_EXT): mg_vtable.h
pp_pack$(OBJ_EXT): mydtrace.h
pp_pack$(OBJ_EXT): op.h
pp_pack$(OBJ_EXT): op_reg_common.h
pp_pack$(OBJ_EXT): opcode.h
pp_pack$(OBJ_EXT): opnames.h
pp_pack$(OBJ_EXT): overload.h
pp_pack$(OBJ_EXT): packsizetables.inc
pp_pack$(OBJ_EXT): pad.h
pp_pack$(OBJ_EXT): parser.h
pp_pack$(OBJ_EXT): patchlevel.h
pp_pack$(OBJ_EXT): perl.h
pp_pack$(OBJ_EXT): perlio.h
pp_pack$(OBJ_EXT): perlvars.h
pp_pack$(OBJ_EXT): perly.h
pp_pack$(OBJ_EXT): pp.h
pp_pack$(OBJ_EXT): pp_pack.c
pp_pack$(OBJ_EXT): proto.h
pp_pack$(OBJ_EXT): reentr.h
pp_pack$(OBJ_EXT): regcharclass.h
pp_pack$(OBJ_EXT): regexp.h
pp_pack$(OBJ_EXT): sbox32_hash.h
pp_pack$(OBJ_EXT): scope.h
pp_pack$(OBJ_EXT): sv.h
pp_pack$(OBJ_EXT): thread.h
pp_pack$(OBJ_EXT): unicode_constants.h
pp_pack$(OBJ_EXT): unixish.h
pp_pack$(OBJ_EXT): utf8.h
pp_pack$(OBJ_EXT): util.h
pp_pack$(OBJ_EXT): warnings.h
pp_pack$(OBJ_EXT): zaphod32_hash.h
pp_pack$(OBJ_EXT): cflags
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/alloca.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/arpa/inet.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/assert.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/alltypes.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/errno.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/fcntl.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/float.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl_fix.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipc.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipcstat.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/limits.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/posix.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/resource.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/sem.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/setjmp.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/signal.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/socket.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stat.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stdint.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/crypt.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/ctype.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/dirent.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/endian.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/errno.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fcntl.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/features.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/float.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdio.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdlib.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/string.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/strings.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/select.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/socket.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/unistd.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/grp.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/inttypes.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/limits.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/locale.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/math.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netdb.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netinet/in.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pthread.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pwd.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sched.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/setjmp.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/shadow.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/signal.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdarg.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdbool.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdc-predef.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stddef.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdint.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdio.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdlib.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/string.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/strings.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/cdefs.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/file.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/glibc-types.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ioctl.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ipc.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/param.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/resource.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/select.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/sem.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/socket.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/stat.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/time.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/times.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/types.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/wait.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/time.h
pp_sort$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/unistd.h
pp_sort$(OBJ_EXT): EXTERN.h
pp_sort$(OBJ_EXT): av.h
pp_sort$(OBJ_EXT): charclass_invlists.h
pp_sort$(OBJ_EXT): config.h
pp_sort$(OBJ_EXT): cop.h
pp_sort$(OBJ_EXT): cv.h
pp_sort$(OBJ_EXT): embed.h
pp_sort$(OBJ_EXT): embedvar.h
pp_sort$(OBJ_EXT): form.h
pp_sort$(OBJ_EXT): gv.h
pp_sort$(OBJ_EXT): handy.h
pp_sort$(OBJ_EXT): hv.h
pp_sort$(OBJ_EXT): hv_func.h
pp_sort$(OBJ_EXT): hv_macro.h
pp_sort$(OBJ_EXT): inline.h
pp_sort$(OBJ_EXT): intrpvar.h
pp_sort$(OBJ_EXT): iperlsys.h
pp_sort$(OBJ_EXT): mg.h
pp_sort$(OBJ_EXT): mg_vtable.h
pp_sort$(OBJ_EXT): mydtrace.h
pp_sort$(OBJ_EXT): op.h
pp_sort$(OBJ_EXT): op_reg_common.h
pp_sort$(OBJ_EXT): opcode.h
pp_sort$(OBJ_EXT): opnames.h
pp_sort$(OBJ_EXT): overload.h
pp_sort$(OBJ_EXT): pad.h
pp_sort$(OBJ_EXT): parser.h
pp_sort$(OBJ_EXT): patchlevel.h
pp_sort$(OBJ_EXT): perl.h
pp_sort$(OBJ_EXT): perlio.h
pp_sort$(OBJ_EXT): perlvars.h
pp_sort$(OBJ_EXT): perly.h
pp_sort$(OBJ_EXT): pp.h
pp_sort$(OBJ_EXT): pp_sort.c
pp_sort$(OBJ_EXT): proto.h
pp_sort$(OBJ_EXT): reentr.h
pp_sort$(OBJ_EXT): regcharclass.h
pp_sort$(OBJ_EXT): regexp.h
pp_sort$(OBJ_EXT): sbox32_hash.h
pp_sort$(OBJ_EXT): scope.h
pp_sort$(OBJ_EXT): sv.h
pp_sort$(OBJ_EXT): thread.h
pp_sort$(OBJ_EXT): unicode_constants.h
pp_sort$(OBJ_EXT): unixish.h
pp_sort$(OBJ_EXT): utf8.h
pp_sort$(OBJ_EXT): util.h
pp_sort$(OBJ_EXT): warnings.h
pp_sort$(OBJ_EXT): zaphod32_hash.h
pp_sort$(OBJ_EXT): cflags
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/alloca.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/arpa/inet.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/assert.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/alltypes.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/errno.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/fcntl.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/float.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl_fix.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipc.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipcstat.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/limits.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/posix.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/resource.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/sem.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/setjmp.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/signal.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/socket.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stat.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stdint.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/crypt.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/ctype.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/dirent.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/endian.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/errno.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fcntl.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/features.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/float.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdio.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdlib.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/string.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/strings.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/select.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/socket.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/unistd.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/grp.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/inttypes.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/limits.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/locale.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/math.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netdb.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netinet/in.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pthread.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pwd.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sched.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/setjmp.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/shadow.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/signal.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdarg.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdbool.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdc-predef.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stddef.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdint.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdio.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdlib.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/string.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/strings.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/cdefs.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/file.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/glibc-types.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ioctl.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ipc.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/param.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/resource.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/select.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/sem.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/socket.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/stat.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/time.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/times.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/types.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/wait.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/time.h
caretx$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/unistd.h
caretx$(OBJ_EXT): EXTERN.h
caretx$(OBJ_EXT): XSUB.h
caretx$(OBJ_EXT): av.h
caretx$(OBJ_EXT): caretx.c
caretx$(OBJ_EXT): charclass_invlists.h
caretx$(OBJ_EXT): config.h
caretx$(OBJ_EXT): cop.h
caretx$(OBJ_EXT): cv.h
caretx$(OBJ_EXT): embed.h
caretx$(OBJ_EXT): embedvar.h
caretx$(OBJ_EXT): form.h
caretx$(OBJ_EXT): gv.h
caretx$(OBJ_EXT): handy.h
caretx$(OBJ_EXT): hv.h
caretx$(OBJ_EXT): hv_func.h
caretx$(OBJ_EXT): hv_macro.h
caretx$(OBJ_EXT): inline.h
caretx$(OBJ_EXT): intrpvar.h
caretx$(OBJ_EXT): iperlsys.h
caretx$(OBJ_EXT): mg.h
caretx$(OBJ_EXT): mg_vtable.h
caretx$(OBJ_EXT): mydtrace.h
caretx$(OBJ_EXT): op.h
caretx$(OBJ_EXT): op_reg_common.h
caretx$(OBJ_EXT): opcode.h
caretx$(OBJ_EXT): opnames.h
caretx$(OBJ_EXT): overload.h
caretx$(OBJ_EXT): pad.h
caretx$(OBJ_EXT): parser.h
caretx$(OBJ_EXT): patchlevel.h
caretx$(OBJ_EXT): perl.h
caretx$(OBJ_EXT): perlapi.h
caretx$(OBJ_EXT): perlio.h
caretx$(OBJ_EXT): perlvars.h
caretx$(OBJ_EXT): perly.h
caretx$(OBJ_EXT): pp.h
caretx$(OBJ_EXT): proto.h
caretx$(OBJ_EXT): reentr.h
caretx$(OBJ_EXT): regcharclass.h
caretx$(OBJ_EXT): regexp.h
caretx$(OBJ_EXT): sbox32_hash.h
caretx$(OBJ_EXT): scope.h
caretx$(OBJ_EXT): sv.h
caretx$(OBJ_EXT): thread.h
caretx$(OBJ_EXT): unicode_constants.h
caretx$(OBJ_EXT): unixish.h
caretx$(OBJ_EXT): utf8.h
caretx$(OBJ_EXT): util.h
caretx$(OBJ_EXT): warnings.h
caretx$(OBJ_EXT): zaphod32_hash.h
caretx$(OBJ_EXT): cflags
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/alloca.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/arpa/inet.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/assert.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/alltypes.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/errno.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/fcntl.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/float.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl_fix.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipc.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipcstat.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/limits.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/posix.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/resource.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/sem.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/setjmp.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/signal.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/socket.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stat.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stdint.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/crypt.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/ctype.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/dirent.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/endian.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/errno.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fcntl.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/features.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/float.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdio.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdlib.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/string.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/strings.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/select.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/socket.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/unistd.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/grp.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/inttypes.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/limits.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/locale.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/math.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netdb.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netinet/in.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pthread.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pwd.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sched.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/setjmp.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/shadow.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/signal.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdarg.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdbool.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdc-predef.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stddef.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdint.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdio.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdlib.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/string.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/strings.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/cdefs.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/file.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/glibc-types.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ioctl.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ipc.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/param.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/resource.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/select.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/sem.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/socket.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/stat.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/time.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/times.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/types.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/wait.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/time.h
dquote$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/unistd.h
dquote$(OBJ_EXT): EXTERN.h
dquote$(OBJ_EXT): av.h
dquote$(OBJ_EXT): charclass_invlists.h
dquote$(OBJ_EXT): config.h
dquote$(OBJ_EXT): cop.h
dquote$(OBJ_EXT): cv.h
dquote$(OBJ_EXT): dquote.c
dquote$(OBJ_EXT): dquote_inline.h
dquote$(OBJ_EXT): embed.h
dquote$(OBJ_EXT): embedvar.h
dquote$(OBJ_EXT): form.h
dquote$(OBJ_EXT): gv.h
dquote$(OBJ_EXT): handy.h
dquote$(OBJ_EXT): hv.h
dquote$(OBJ_EXT): hv_func.h
dquote$(OBJ_EXT): hv_macro.h
dquote$(OBJ_EXT): inline.h
dquote$(OBJ_EXT): intrpvar.h
dquote$(OBJ_EXT): iperlsys.h
dquote$(OBJ_EXT): mg.h
dquote$(OBJ_EXT): mg_vtable.h
dquote$(OBJ_EXT): mydtrace.h
dquote$(OBJ_EXT): op.h
dquote$(OBJ_EXT): op_reg_common.h
dquote$(OBJ_EXT): opcode.h
dquote$(OBJ_EXT): opnames.h
dquote$(OBJ_EXT): overload.h
dquote$(OBJ_EXT): pad.h
dquote$(OBJ_EXT): parser.h
dquote$(OBJ_EXT): patchlevel.h
dquote$(OBJ_EXT): perl.h
dquote$(OBJ_EXT): perlio.h
dquote$(OBJ_EXT): perlvars.h
dquote$(OBJ_EXT): perly.h
dquote$(OBJ_EXT): pp.h
dquote$(OBJ_EXT): proto.h
dquote$(OBJ_EXT): reentr.h
dquote$(OBJ_EXT): regcharclass.h
dquote$(OBJ_EXT): regexp.h
dquote$(OBJ_EXT): sbox32_hash.h
dquote$(OBJ_EXT): scope.h
dquote$(OBJ_EXT): sv.h
dquote$(OBJ_EXT): thread.h
dquote$(OBJ_EXT): unicode_constants.h
dquote$(OBJ_EXT): unixish.h
dquote$(OBJ_EXT): utf8.h
dquote$(OBJ_EXT): util.h
dquote$(OBJ_EXT): warnings.h
dquote$(OBJ_EXT): zaphod32_hash.h
dquote$(OBJ_EXT): cflags
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/alloca.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/arpa/inet.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/assert.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/alltypes.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/errno.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/fcntl.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/float.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl_fix.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipc.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipcstat.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/limits.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/posix.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/resource.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/sem.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/setjmp.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/signal.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/socket.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stat.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stdint.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/crypt.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/ctype.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/dirent.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/endian.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/errno.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fcntl.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/features.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/float.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdio.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdlib.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/string.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/strings.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/select.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/socket.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/unistd.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/grp.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/inttypes.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/limits.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/locale.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/math.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netdb.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netinet/in.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pthread.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pwd.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sched.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/setjmp.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/shadow.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/signal.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdarg.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdbool.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdc-predef.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stddef.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdint.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdio.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdlib.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/string.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/strings.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/cdefs.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/file.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/glibc-types.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ioctl.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ipc.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/param.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/resource.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/select.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/sem.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/socket.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/stat.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/time.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/times.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/types.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/wait.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/time.h
time64$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/unistd.h
time64$(OBJ_EXT): EXTERN.h
time64$(OBJ_EXT): av.h
time64$(OBJ_EXT): charclass_invlists.h
time64$(OBJ_EXT): config.h
time64$(OBJ_EXT): cop.h
time64$(OBJ_EXT): cv.h
time64$(OBJ_EXT): embed.h
time64$(OBJ_EXT): embedvar.h
time64$(OBJ_EXT): form.h
time64$(OBJ_EXT): gv.h
time64$(OBJ_EXT): handy.h
time64$(OBJ_EXT): hv.h
time64$(OBJ_EXT): hv_func.h
time64$(OBJ_EXT): hv_macro.h
time64$(OBJ_EXT): inline.h
time64$(OBJ_EXT): intrpvar.h
time64$(OBJ_EXT): iperlsys.h
time64$(OBJ_EXT): mg.h
time64$(OBJ_EXT): mg_vtable.h
time64$(OBJ_EXT): mydtrace.h
time64$(OBJ_EXT): op.h
time64$(OBJ_EXT): op_reg_common.h
time64$(OBJ_EXT): opcode.h
time64$(OBJ_EXT): opnames.h
time64$(OBJ_EXT): overload.h
time64$(OBJ_EXT): pad.h
time64$(OBJ_EXT): parser.h
time64$(OBJ_EXT): patchlevel.h
time64$(OBJ_EXT): perl.h
time64$(OBJ_EXT): perlio.h
time64$(OBJ_EXT): perlvars.h
time64$(OBJ_EXT): perly.h
time64$(OBJ_EXT): pp.h
time64$(OBJ_EXT): proto.h
time64$(OBJ_EXT): reentr.h
time64$(OBJ_EXT): regcharclass.h
time64$(OBJ_EXT): regexp.h
time64$(OBJ_EXT): sbox32_hash.h
time64$(OBJ_EXT): scope.h
time64$(OBJ_EXT): sv.h
time64$(OBJ_EXT): thread.h
time64$(OBJ_EXT): time64.c
time64$(OBJ_EXT): time64.h
time64$(OBJ_EXT): time64_config.h
time64$(OBJ_EXT): unicode_constants.h
time64$(OBJ_EXT): unixish.h
time64$(OBJ_EXT): utf8.h
time64$(OBJ_EXT): util.h
time64$(OBJ_EXT): warnings.h
time64$(OBJ_EXT): zaphod32_hash.h
time64$(OBJ_EXT): cflags
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/alloca.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/alloca.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/arpa/inet.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/arpa/inet.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/assert.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/assert.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/alltypes.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/alltypes.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/errno.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/errno.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/fcntl.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/fcntl.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/float.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/float.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl_fix.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl_fix.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipc.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipc.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipcstat.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipcstat.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/limits.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/limits.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/posix.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/posix.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/resource.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/resource.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/sem.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/sem.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/setjmp.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/setjmp.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/signal.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/signal.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/socket.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/socket.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stat.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stat.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stdint.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stdint.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/crypt.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/crypt.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/ctype.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/ctype.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/dirent.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/dirent.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/endian.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/endian.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/errno.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/errno.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fcntl.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fcntl.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/features.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/features.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/float.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/float.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdio.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdio.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdlib.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdlib.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/string.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/string.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/strings.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/strings.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/select.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/select.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/socket.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/socket.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/unistd.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/unistd.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/grp.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/grp.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/inttypes.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/inttypes.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/limits.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/limits.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/locale.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/locale.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/math.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/math.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netdb.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netdb.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netinet/in.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netinet/in.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pthread.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pthread.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pwd.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pwd.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sched.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sched.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/setjmp.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/setjmp.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/shadow.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/shadow.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/signal.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/signal.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdarg.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdarg.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdbool.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdbool.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdc-predef.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdc-predef.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stddef.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stddef.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdint.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdint.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdio.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdio.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdlib.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdlib.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/string.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/string.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/strings.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/strings.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/cdefs.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/cdefs.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/file.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/file.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/glibc-types.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/glibc-types.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ioctl.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ioctl.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ipc.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ipc.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/param.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/param.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/resource.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/resource.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/select.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/select.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/sem.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/sem.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/socket.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/socket.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/stat.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/stat.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/time.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/time.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/times.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/times.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/types.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/types.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/wait.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/wait.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/time.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/time.h
perlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/unistd.h
miniperlmain$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/unistd.h
perlmain$(OBJ_EXT): EXTERN.h
miniperlmain$(OBJ_EXT): EXTERN.h
perlmain$(OBJ_EXT): XSUB.h
miniperlmain$(OBJ_EXT): XSUB.h
perlmain$(OBJ_EXT): av.h
miniperlmain$(OBJ_EXT): av.h
perlmain$(OBJ_EXT): charclass_invlists.h
miniperlmain$(OBJ_EXT): charclass_invlists.h
perlmain$(OBJ_EXT): config.h
miniperlmain$(OBJ_EXT): config.h
perlmain$(OBJ_EXT): cop.h
miniperlmain$(OBJ_EXT): cop.h
perlmain$(OBJ_EXT): cv.h
miniperlmain$(OBJ_EXT): cv.h
perlmain$(OBJ_EXT): embed.h
miniperlmain$(OBJ_EXT): embed.h
perlmain$(OBJ_EXT): embedvar.h
miniperlmain$(OBJ_EXT): embedvar.h
perlmain$(OBJ_EXT): form.h
miniperlmain$(OBJ_EXT): form.h
perlmain$(OBJ_EXT): gv.h
miniperlmain$(OBJ_EXT): gv.h
perlmain$(OBJ_EXT): handy.h
miniperlmain$(OBJ_EXT): handy.h
perlmain$(OBJ_EXT): hv.h
miniperlmain$(OBJ_EXT): hv.h
perlmain$(OBJ_EXT): hv_func.h
miniperlmain$(OBJ_EXT): hv_func.h
perlmain$(OBJ_EXT): hv_macro.h
miniperlmain$(OBJ_EXT): hv_macro.h
perlmain$(OBJ_EXT): inline.h
miniperlmain$(OBJ_EXT): inline.h
perlmain$(OBJ_EXT): intrpvar.h
miniperlmain$(OBJ_EXT): intrpvar.h
perlmain$(OBJ_EXT): iperlsys.h
miniperlmain$(OBJ_EXT): iperlsys.h
perlmain$(OBJ_EXT): mg.h
miniperlmain$(OBJ_EXT): mg.h
perlmain$(OBJ_EXT): mg_vtable.h
miniperlmain$(OBJ_EXT): mg_vtable.h
perlmain$(OBJ_EXT): miniperlmain.c
miniperlmain$(OBJ_EXT): miniperlmain.c
perlmain$(OBJ_EXT): mydtrace.h
miniperlmain$(OBJ_EXT): mydtrace.h
perlmain$(OBJ_EXT): op.h
miniperlmain$(OBJ_EXT): op.h
perlmain$(OBJ_EXT): op_reg_common.h
miniperlmain$(OBJ_EXT): op_reg_common.h
perlmain$(OBJ_EXT): opcode.h
miniperlmain$(OBJ_EXT): opcode.h
perlmain$(OBJ_EXT): opnames.h
miniperlmain$(OBJ_EXT): opnames.h
perlmain$(OBJ_EXT): overload.h
miniperlmain$(OBJ_EXT): overload.h
perlmain$(OBJ_EXT): pad.h
miniperlmain$(OBJ_EXT): pad.h
perlmain$(OBJ_EXT): parser.h
miniperlmain$(OBJ_EXT): parser.h
perlmain$(OBJ_EXT): patchlevel.h
miniperlmain$(OBJ_EXT): patchlevel.h
perlmain$(OBJ_EXT): perl.h
miniperlmain$(OBJ_EXT): perl.h
perlmain$(OBJ_EXT): perlapi.h
miniperlmain$(OBJ_EXT): perlapi.h
perlmain$(OBJ_EXT): perlio.h
miniperlmain$(OBJ_EXT): perlio.h
perlmain$(OBJ_EXT): perlvars.h
miniperlmain$(OBJ_EXT): perlvars.h
perlmain$(OBJ_EXT): perly.h
miniperlmain$(OBJ_EXT): perly.h
perlmain$(OBJ_EXT): pp.h
miniperlmain$(OBJ_EXT): pp.h
perlmain$(OBJ_EXT): proto.h
miniperlmain$(OBJ_EXT): proto.h
perlmain$(OBJ_EXT): reentr.h
miniperlmain$(OBJ_EXT): reentr.h
perlmain$(OBJ_EXT): regcharclass.h
miniperlmain$(OBJ_EXT): regcharclass.h
perlmain$(OBJ_EXT): regexp.h
miniperlmain$(OBJ_EXT): regexp.h
perlmain$(OBJ_EXT): sbox32_hash.h
miniperlmain$(OBJ_EXT): sbox32_hash.h
perlmain$(OBJ_EXT): scope.h
miniperlmain$(OBJ_EXT): scope.h
perlmain$(OBJ_EXT): sv.h
miniperlmain$(OBJ_EXT): sv.h
perlmain$(OBJ_EXT): thread.h
miniperlmain$(OBJ_EXT): thread.h
perlmain$(OBJ_EXT): unicode_constants.h
miniperlmain$(OBJ_EXT): unicode_constants.h
perlmain$(OBJ_EXT): unixish.h
miniperlmain$(OBJ_EXT): unixish.h
perlmain$(OBJ_EXT): utf8.h
miniperlmain$(OBJ_EXT): utf8.h
perlmain$(OBJ_EXT): util.h
miniperlmain$(OBJ_EXT): util.h
perlmain$(OBJ_EXT): warnings.h
miniperlmain$(OBJ_EXT): warnings.h
perlmain$(OBJ_EXT): zaphod32_hash.h
miniperlmain$(OBJ_EXT): zaphod32_hash.h
perlmain$(OBJ_EXT): cflags
miniperlmain$(OBJ_EXT): cflags
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/alloca.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/arpa/inet.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/assert.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/alltypes.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/errno.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/fcntl.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/float.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl_fix.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipc.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipcstat.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/limits.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/posix.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/resource.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/sem.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/setjmp.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/signal.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/socket.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stat.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stdint.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/crypt.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/ctype.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/dirent.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/endian.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/errno.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fcntl.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/features.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/float.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdio.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdlib.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/string.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/strings.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/select.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/socket.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/unistd.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/grp.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/inttypes.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/limits.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/locale.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/math.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netdb.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netinet/in.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pthread.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pwd.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sched.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/setjmp.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/shadow.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/signal.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdarg.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdbool.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdc-predef.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stddef.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdint.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdio.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdlib.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/string.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/strings.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/cdefs.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/file.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/glibc-types.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ioctl.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ipc.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/param.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/resource.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/select.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/sem.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/socket.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/stat.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/time.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/times.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/types.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/wait.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/time.h
opmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/unistd.h
opmini$(OBJ_EXT): EXTERN.h
opmini$(OBJ_EXT): XSUB.h
opmini$(OBJ_EXT): av.h
opmini$(OBJ_EXT): charclass_invlists.h
opmini$(OBJ_EXT): config.h
opmini$(OBJ_EXT): cop.h
opmini$(OBJ_EXT): cv.h
opmini$(OBJ_EXT): embed.h
opmini$(OBJ_EXT): embedvar.h
opmini$(OBJ_EXT): feature.h
opmini$(OBJ_EXT): form.h
opmini$(OBJ_EXT): gv.h
opmini$(OBJ_EXT): handy.h
opmini$(OBJ_EXT): hv.h
opmini$(OBJ_EXT): hv_func.h
opmini$(OBJ_EXT): hv_macro.h
opmini$(OBJ_EXT): inline.h
opmini$(OBJ_EXT): intrpvar.h
opmini$(OBJ_EXT): iperlsys.h
opmini$(OBJ_EXT): keywords.h
opmini$(OBJ_EXT): mg.h
opmini$(OBJ_EXT): mg_vtable.h
opmini$(OBJ_EXT): mydtrace.h
opmini$(OBJ_EXT): op.h
opmini$(OBJ_EXT): op_reg_common.h
opmini$(OBJ_EXT): opcode.h
opmini$(OBJ_EXT): opmini.c
opmini$(OBJ_EXT): opnames.h
opmini$(OBJ_EXT): overload.h
opmini$(OBJ_EXT): pad.h
opmini$(OBJ_EXT): parser.h
opmini$(OBJ_EXT): patchlevel.h
opmini$(OBJ_EXT): perl.h
opmini$(OBJ_EXT): perlapi.h
opmini$(OBJ_EXT): perlio.h
opmini$(OBJ_EXT): perlvars.h
opmini$(OBJ_EXT): perly.h
opmini$(OBJ_EXT): pp.h
opmini$(OBJ_EXT): proto.h
opmini$(OBJ_EXT): reentr.h
opmini$(OBJ_EXT): regcharclass.h
opmini$(OBJ_EXT): regcomp.h
opmini$(OBJ_EXT): regexp.h
opmini$(OBJ_EXT): regnodes.h
opmini$(OBJ_EXT): sbox32_hash.h
opmini$(OBJ_EXT): scope.h
opmini$(OBJ_EXT): sv.h
opmini$(OBJ_EXT): thread.h
opmini$(OBJ_EXT): unicode_constants.h
opmini$(OBJ_EXT): unixish.h
opmini$(OBJ_EXT): utf8.h
opmini$(OBJ_EXT): util.h
opmini$(OBJ_EXT): warnings.h
opmini$(OBJ_EXT): zaphod32_hash.h
opmini$(OBJ_EXT): cflags
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/alloca.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/arpa/inet.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/assert.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/alltypes.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/errno.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/fcntl.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/float.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ioctl_fix.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipc.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/ipcstat.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/limits.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/posix.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/resource.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/sem.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/setjmp.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/signal.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/socket.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stat.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/bits/stdint.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/crypt.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/ctype.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/dirent.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/endian.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/errno.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fcntl.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/features.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/float.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdio.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/stdlib.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/string.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/strings.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/select.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/sys/socket.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/fortify/unistd.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/grp.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/inttypes.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/limits.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/locale.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/math.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netdb.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/netinet/in.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pthread.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/pwd.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sched.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/setjmp.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/shadow.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/signal.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdarg.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdbool.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdc-predef.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stddef.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdint.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdio.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/stdlib.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/string.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/strings.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/cdefs.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/file.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/glibc-types.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ioctl.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/ipc.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/param.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/resource.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/select.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/sem.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/socket.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/stat.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/time.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/times.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/types.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/sys/wait.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/time.h
perlmini$(OBJ_EXT): /home/xfs/openwrt/lede/openwrt/staging_dir/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/include/unistd.h
perlmini$(OBJ_EXT): EXTERN.h
perlmini$(OBJ_EXT): XSUB.h
perlmini$(OBJ_EXT): av.h
perlmini$(OBJ_EXT): charclass_invlists.h
perlmini$(OBJ_EXT): config.h
perlmini$(OBJ_EXT): cop.h
perlmini$(OBJ_EXT): cv.h
perlmini$(OBJ_EXT): embed.h
perlmini$(OBJ_EXT): embedvar.h
perlmini$(OBJ_EXT): form.h
perlmini$(OBJ_EXT): gv.h
perlmini$(OBJ_EXT): handy.h
perlmini$(OBJ_EXT): hv.h
perlmini$(OBJ_EXT): hv_func.h
perlmini$(OBJ_EXT): hv_macro.h
perlmini$(OBJ_EXT): inline.h
perlmini$(OBJ_EXT): intrpvar.h
perlmini$(OBJ_EXT): iperlsys.h
perlmini$(OBJ_EXT): mg.h
perlmini$(OBJ_EXT): mg_vtable.h
perlmini$(OBJ_EXT): mydtrace.h
perlmini$(OBJ_EXT): op.h
perlmini$(OBJ_EXT): op_reg_common.h
perlmini$(OBJ_EXT): opcode.h
perlmini$(OBJ_EXT): opnames.h
perlmini$(OBJ_EXT): overload.h
perlmini$(OBJ_EXT): pad.h
perlmini$(OBJ_EXT): parser.h
perlmini$(OBJ_EXT): patchlevel.h
perlmini$(OBJ_EXT): perl.h
perlmini$(OBJ_EXT): perl_inc_macro.h
perlmini$(OBJ_EXT): perlapi.h
perlmini$(OBJ_EXT): perlio.h
perlmini$(OBJ_EXT): perlmini.c
perlmini$(OBJ_EXT): perlvars.h
perlmini$(OBJ_EXT): perly.h
perlmini$(OBJ_EXT): pp.h
perlmini$(OBJ_EXT): proto.h
perlmini$(OBJ_EXT): reentr.h
perlmini$(OBJ_EXT): regcharclass.h
perlmini$(OBJ_EXT): regexp.h
perlmini$(OBJ_EXT): sbox32_hash.h
perlmini$(OBJ_EXT): scope.h
perlmini$(OBJ_EXT): sv.h
perlmini$(OBJ_EXT): thread.h
perlmini$(OBJ_EXT): unicode_constants.h
perlmini$(OBJ_EXT): unixish.h
perlmini$(OBJ_EXT): utf8.h
perlmini$(OBJ_EXT): util.h
perlmini$(OBJ_EXT): warnings.h
perlmini$(OBJ_EXT): zaphod32_hash.h
perlmini$(OBJ_EXT): cflags
# WARNING: Put nothing here or make depend will gobble it up!
