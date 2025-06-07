AUTOMAKE_OPTIONS = dejagnu
export DEJAGNU

# Programs that are expected across the board.
DEJATOOL =

noinst_PROGRAMS = test-schedbatch test-hugetlb test-zombie

test_schedbatch_SOURCES = ps.test/test-schedbatch.c

test_hugetlb_SOURCES = ps.test/test-hugetlb.c

test_zombie_SOURCES = ps.test/test-zombie.c

if LINUX
# These should be in defined in 'across the board' scope, but are
# temporarily disabled on other than linux systems, see commit
# 3d807ae853b8b4264da156065b34f1447658a8ba
DEJATOOL += \
	pmap

if !CYGWIN
if WITH_NCURSES
DEJATOOL += slabtop
endif

DEJATOOL += sysctl
endif

if BUILD_KILL
DEJATOOL += kill
endif

if BUILD_W
DEJATOOL += w
endif

DEJATOOL += \
	free \
	hugetop \
	lib \
	pgrep \
	pkill \
	ps \
	pwdx \
	uptime \
	vmstat
endif

EXTRA_DIST = \
    config/unix.exp \
    free.test/free.exp \
    hugetop.test/hugetop.exp \
    kill.test/kill.exp \
    lib.test/fileutils.exp \
    lib.test/fileutils_badfd.sh \
    lib.test/fileutils_full.sh \
    lib.test/strutils.exp \
    pgrep.test/pgrep.exp \
    pkill.test/pkill.exp \
    pmap.test/pmap.exp \
    ps.test/ps_hugetlb.exp \
    ps.test/ps_output.exp \
    ps.test/ps_personality.exp \
    ps.test/ps_sched_batch.exp \
    pwdx.test/pwdx.exp \
    slabtop.test/slabtop.exp \
    uptime.test/uptime.exp \
    vmstat.test/vmstat.exp \
    w.test/w.exp
if !CYGWIN
EXTRA_DIST += \
    slabtop.test/slabtop.exp \
    sysctl.test/sysctl_read.exp
endif
