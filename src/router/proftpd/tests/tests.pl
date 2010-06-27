#!/usr/bin/env perl

use strict;

use Cwd qw(abs_path);
use File::Spec;
use Getopt::Long;
use Test::Harness;

my $opts = {};
GetOptions($opts, 'h|help', 'C|class=s@', 'V|verbose');

if ($opts->{h}) {
  usage();
}

if ($opts->{V}) {
  $ENV{TEST_VERBOSE} = 1;
}

# We use this, rather than use(), since use() is equivalent to a BEGIN
# block, and we want the module to be loaded at run-time.

my $test_dir = (File::Spec->splitpath(abs_path(__FILE__)))[1];
push(@INC, "$test_dir/t/lib");

require ProFTPD::TestSuite::Utils;
import ProFTPD::TestSuite::Utils qw(:testsuite);

# This is to handle the case where this tests.pl script might be
# being used to run test files other than those that ship with proftpd,
# e.g. to run the tests that come with third-party modules.
unless (defined($ENV{PROFTPD_TEST_BIN})) {
  $ENV{PROFTPD_TEST_BIN} = File::Spec->catfile($test_dir, '..', 'proftpd');
}

# Set this environment variable, for other test cases which may want to
# know the directory, and not necessarily just the location of the uninstalled
# `proftpd' binary.  This is useful, for example, for using the utilities.
$ENV{PROFTPD_TEST_PATH} = $test_dir;

$| = 1;

my $test_files;

if (scalar(@ARGV) > 0) {
  $test_files = [@ARGV];

} else {
  $test_files = [qw(
    t/logins.t
    t/commands/user.t
    t/commands/pass.t
    t/commands/pwd.t
    t/commands/cwd.t
    t/commands/cdup.t
    t/commands/syst.t
    t/commands/type.t
    t/commands/mkd.t
    t/commands/rmd.t
    t/commands/dele.t
    t/commands/mdtm.t
    t/commands/size.t 
    t/commands/mode.t
    t/commands/stru.t
    t/commands/allo.t
    t/commands/noop.t
    t/commands/feat.t
    t/commands/help.t
    t/commands/quit.t
    t/commands/rnfr.t
    t/commands/rnto.t
    t/commands/rest.t
    t/commands/pasv.t
    t/commands/epsv.t
    t/commands/port.t
    t/commands/eprt.t
    t/commands/nlst.t
    t/commands/list.t
    t/commands/retr.t
    t/commands/stor.t
    t/commands/stou.t
    t/commands/appe.t
    t/commands/stat.t
    t/commands/abor.t
    t/commands/mlsd.t
    t/commands/mlst.t
    t/commands/mff.t
    t/commands/mfmt.t
    t/config/accessdenymsg.t
    t/config/accessgrantmsg.t
    t/config/allowoverwrite.t
    t/config/authaliasonly.t
    t/config/authgroupfile.t
    t/config/authusingalias.t
    t/config/classes.t
    t/config/createhome.t
    t/config/deleteabortedstores.t
    t/config/dirfakemode.t
    t/config/displayconnect.t
    t/config/displayfiletransfer.t 
    t/config/displaylogin.t
    t/config/displayquit.t
    t/config/groupowner.t
    t/config/hiddenstores.t
    t/config/hidefiles.t
    t/config/hidegroup.t
    t/config/hidenoaccess.t
    t/config/hideuser.t
    t/config/maxinstances.t
    t/config/maxloginattempts.t
    t/config/maxretrievefilesize.t
    t/config/maxstorefilesize.t
    t/config/order.t
    t/config/pathallowfilter.t
    t/config/pathdenyfilter.t
    t/config/requirevalidshell.t
    t/config/rewritehome.t
    t/config/serverident.t
    t/config/storeuniqueprefix.t
    t/config/timeoutidle.t
    t/config/timeoutlogin.t
    t/config/timeoutnotransfer.t
    t/config/timeoutsession.t
    t/config/timeoutstalled.t
    t/config/useftpusers.t
    t/config/useglobbing.t
    t/config/useralias.t
    t/config/userowner.t
    t/config/userpassword.t
    t/config/directory/limits.t
    t/config/directory/umask.t
    t/config/ftpaccess/dele.t
    t/config/ftpaccess/empty.t
    t/config/ftpaccess/merging.t
    t/config/ftpaccess/retr.t
    t/config/limit/anonymous.t
    t/config/limit/rmd.t
    t/config/limit/xmkd.t
    t/config/limit/filters.t
    t/config/limit/subdirs.t
    t/logging/extendedlog.t
    t/logging/transferlog.t
    t/signals/term.t
    t/signals/hup.t
    t/signals/segv.t
    t/signals/abrt.t
    t/utils/ftpcount.t
    t/utils/ftpwho.t
  )];

  # Now interrogate the build to see which module/feature-specific test files
  # should be added to the list.
  my $order = 0;

  my $FEATURE_TESTS = {
    't/modules/mod_ban.t' => {
      order => ++$order,
      test_class => [qw(mod_ban)],
    },

    't/modules/mod_cap.t' => {
      order => ++$order,
      test_class => [qw(mod_cap)],
    },

    't/modules/mod_ctrls.t' => {
      order => ++$order,
      test_class => [qw(mod_ctrls)],
    },

    't/modules/mod_exec.t' => {
      order => ++$order,
      test_class => [qw(mod_exec)],
    },

    't/modules/mod_lang.t' => {
      order => ++$order,
      test_class => [qw(mod_lang)],
    },

    't/modules/mod_quotatab_file.t' => {
      order => ++$order,
      test_class => [qw(mod_quotatab mod_quotatab_file)],
    },

    't/modules/mod_quotatab_sql.t' => {
      order => ++$order,
      test_class => [qw(mod_quotatab mod_quotatab_sql mod_sql_sqlite)],
    },

    't/modules/mod_rewrite.t' => {
      order => ++$order,
      test_class => [qw(mod_rewrite)],
    },

    't/modules/mod_sftp.t' => {
      order => ++$order,
      test_class => [qw(mod_sftp)],
    },

    't/modules/mod_sftp_sql.t' => {
      order => ++$order,
      test_class => [qw(mod_sftp mod_sql_sqlite)],
    },

    't/modules/mod_shaper.t' => {
      order => ++$order,
      test_class => [qw(mod_shaper)],
    },

    't/modules/mod_site_misc.t' => {
      order => ++$order,
      test_class => [qw(mod_site_misc)],
    },

    't/modules/mod_sql.t' => {
      order => ++$order,
      test_class => [qw(mod_sql)],
    },

    't/modules/mod_sql_passwd.t' => {
      order => ++$order,
      test_class => [qw(mod_sql_passwd mod_sql_sqlite)],
    },

    't/modules/mod_sql_sqlite.t' => {
      order => ++$order,
      test_class => [qw(mod_sql_sqlite)],
    },

    't/modules/mod_tls.t' => {
      order => ++$order,
      test_class => [qw(mod_tls)],
    },

    't/modules/mod_unique_id.t' => {
      order => ++$order,
      test_class => [qw(mod_unique_id)],
    },

    't/modules/mod_wrap.t' => {
      order => ++$order,
      test_class => [qw(mod_wrap)],
    },

    't/modules/mod_wrap2_file.t' => {
      order => ++$order,
      test_class => [qw(mod_wrap2_file)],
    },

    't/modules/mod_wrap2_sql.t' => {
      order => ++$order,
      test_class => [qw(mod_sql_sqlite mod_wrap2_sql)],
    },
  };

  my @feature_tests = testsuite_get_runnable_tests($FEATURE_TESTS);
  my $feature_ntests = scalar(@feature_tests);
  if ($feature_ntests > 1 ||
      ($feature_ntests == 1 && $feature_tests[0] ne 'testsuite_empty_test')) {
    push(@$test_files, @feature_tests);
  }
}

$ENV{PROFTPD_TEST} = 1;

if (defined($opts->{C})) {
  $ENV{PROFTPD_TEST_ENABLE_CLASS} = join(':', @{ $opts->{C} });

} else {
  # Disable all 'inprogress' and 'slow' tests by default
  $ENV{PROFTPD_TEST_DISABLE_CLASS} = 'inprogress:slow';
}

runtests(@$test_files) if scalar(@$test_files) > 0;

exit 0;

sub usage {
  print STDOUT <<EOH;

$0: [--help] [--class=\$name] [--verbose]

Examples:

  perl $0
  perl $0 --class foo
  perl $0 --class bar --class baz

EOH
  exit 0;
}
