
use strict;
use File::Path;
use File::Copy;

push @INC, "$ENV{SRCDIR}/tst";
require 'vstr_tst_examples.pl';

our $root = "ex_httpd_root";
our $truncate_segv = 0;

sub http_cntl_list
  { # FIXME: see if it looks "OK"
    my $list_pid = tst_proc_fork();
    if (!$list_pid) {
      sleep(2);
      system("./ex_cntl -e list ex_httpd_cntl > /dev/null");
      _exit(0);
    }
    return $list_pid;
  }

sub httpd__munge_ret
  {
    my $output = shift;

    # Remove date, because that changes each time
    $output =~ s/^(Date:).*$/$1/gm;
    # Remove last-modified = start date for error messages
    $output =~
      s!(HTTP/1[.]1 \s (?:30[1237]|40[03456]|41[0234567]|50[0135]) .*)$ (\n)
	^(Date:)$ (\n)
	^(Server:.*)$ (\n)
	^(Last-Modified:) .*$
	!$1$2$3$4$5$6$7!gmx;
    # Remove last modified for trace ops
    $output =~
      s!^(Last-Modified:).*$ (\n)
        ^(Content-Type: \s message/http.*)$
	!$1$2$3!gmx;

    return $output;
  }

sub httpd_file_tst
  {
    my $io_r = shift;
    my $io_w = shift;
    my $xtra = shift || {};
    my $sz   = shift;

    my $data = daemon_get_io_r($io_r);

    $data =~ s/\n/\r\n/g;

    my $output = daemon_io($data,
			   $xtra->{shutdown_w}, $xtra->{slow_write}, 1);

    $output = httpd__munge_ret($output);
    daemon_put_io_w($io_w, $output);
  }

sub httpd_gen_tst
  {
    my $io_r = shift;
    my $io_w = shift;
    my $xtra = shift || {};
    my $sz   = shift;

    my $data = daemon_get_io_r($io_r);

    if (length($data) != 0)
      { failure(sprintf("data(%d) on gen tst", length($data))); }

    if (! exists($xtra->{gen_output}))
      { $xtra->{gen_output} = \&httpd__munge_ret; }

    $data = $xtra->{gen_input}->();

    my $output = daemon_io($data,
			   $xtra->{shutdown_w}, $xtra->{slow_write}, 1);

    $output = $xtra->{gen_output}->($output);

    daemon_put_io_w($io_w, $output);
  }

sub gen_tst_e2big
  {
    my $gen_cb = sub {
      my $data = ("\r\n" x 80_000) . ("x" x 150_000);
      return $data;
    };

    my $gen_out_cb = sub { # Load ex_httpd_null_out_1 ?
      $_ = shift;
      if (m!^HTTP/1.1 400 !)
	{
	  $_ = "";
	}

      return $_;
    };

    sub_tst(\&httpd_gen_tst, "ex_httpd_null",
	    {gen_input => $gen_cb, gen_output => $gen_out_cb,
	     shutdown_w => 0});
  }

use POSIX; # _exit

sub gen_tst_trunc
  {
    return if ($main::truncate_segv);

    my $vhosts = shift;
    my $pid = 0;

    if (!($pid = tst_proc_fork()))
      {
	if (1)
	  {
	    open(STDIN,  "< /dev/null") || failure("open(2): $!");
	    open(STDOUT, "> /dev/null") || failure("open(2): $!");
	    open(STDERR, "> /dev/null") || failure("open(2): $!");
	  }

	my $fname = "$main::root/foo.example.com/4mb_2_2mb_$$";

	if (!$vhosts)
	  {
	    $fname = "$main::root/4mb_2_2mb_$$";
	  }

	if (!($pid = tst_proc_fork()))
	  { # Child goes
	    sleep(4);
	    truncate($fname, 2_000_000);
	    success();
	  }

	open(OUT, ">> $fname") || failure("open($fname): $!");

	truncate($fname, 4_000_000);

	my $gen_cb = sub {
	  sleep(1);
	  my $pad = "x" x 64_000;
	  my $data = <<EOL;
GET http://foo.example.com/4mb_2_2mb_$$ HTTP/1.1\r
Host: $pad\r
\r
EOL
	  $data = $data x 16;
	  return $data;
	};

	my $gen_out_cb = sub { # Load ex_httpd_null_out_1 ?
	  unlink($fname);
	  success();
	};
	# Randomly test as other stuff happens...
	sub_tst(\&httpd_gen_tst, "ex_httpd_null",
		{gen_input => $gen_cb, gen_output => $gen_out_cb,
		 shutdown_w => 0});
	success();
      }
  }

sub gen_tsts
  {
    my $vhosts = shift;

    gen_tst_trunc($vhosts);
    gen_tst_e2big();
  }

sub all_vhost_tsts()
  {
    gen_tsts(1);
    sub_tst(\&httpd_file_tst, "ex_httpd");
    if ($>) { # mode 000 doesn't work if running !uid
    sub_tst(\&httpd_file_tst, "ex_httpd_nonroot"); }

    sub_tst(\&httpd_file_tst, "ex_httpd_errs");

    sub_tst(\&httpd_file_tst, "ex_httpd",
	    {shutdown_w => 0});
    if ($>) {
    sub_tst(\&httpd_file_tst, "ex_httpd_nonroot",
	    {shutdown_w => 0}); }
    sub_tst(\&httpd_file_tst, "ex_httpd_errs",
	    {shutdown_w => 0});
    sub_tst(\&httpd_file_tst, "ex_httpd_shut",
	    {shutdown_w => 0});

    sub_tst(\&httpd_file_tst, "ex_httpd",
	    {                 slow_write => 1});
    if ($>) {
    sub_tst(\&httpd_file_tst, "ex_httpd_nonroot",
	    {                 slow_write => 1}); }
    sub_tst(\&httpd_file_tst, "ex_httpd_errs",
	    {                 slow_write => 1});

    sub_tst(\&httpd_file_tst, "ex_httpd",
	    {shutdown_w => 0, slow_write => 1});
    if ($>) {
    sub_tst(\&httpd_file_tst, "ex_httpd_nonroot",
	    {shutdown_w => 0, slow_write => 1}); }
    sub_tst(\&httpd_file_tst, "ex_httpd_errs",
	    {shutdown_w => 0, slow_write => 1});
    sub_tst(\&httpd_file_tst, "ex_httpd_shut",
	    {shutdown_w => 0, slow_write => 1});
  }

sub all_nonvhost_tsts()
  {
    gen_tsts(0);
    sub_tst(\&httpd_file_tst, "ex_httpd_non-virtual-hosts");
    sub_tst(\&httpd_file_tst, "ex_httpd_non-virtual-hosts",
	    {shutdown_w => 0});
    sub_tst(\&httpd_file_tst, "ex_httpd_non-virtual-hosts",
	    {                 slow_write => 1});
    sub_tst(\&httpd_file_tst, "ex_httpd_non-virtual-hosts",
	    {shutdown_w => 0, slow_write => 1});
  }

sub all_public_only_tsts
  {
    if (!@_) { gen_tsts(1); }
    sub_tst(\&httpd_file_tst, "ex_httpd_public-only");
    sub_tst(\&httpd_file_tst, "ex_httpd_public-only",
	    {shutdown_w => 0});
    sub_tst(\&httpd_file_tst, "ex_httpd_public-only",
	    {                 slow_write => 1});
    sub_tst(\&httpd_file_tst, "ex_httpd_public-only",
	    {shutdown_w => 0, slow_write => 1});
  }

sub all_none_tsts()
  {
    gen_tsts(1);
    sub_tst(\&httpd_file_tst, "ex_httpd_none");
    sub_tst(\&httpd_file_tst, "ex_httpd_none",
	    {shutdown_w => 0});
    sub_tst(\&httpd_file_tst, "ex_httpd_none",
	    {                 slow_write => 1});
    sub_tst(\&httpd_file_tst, "ex_httpd_none",
	    {shutdown_w => 0, slow_write => 1});
  }

sub all_conf_5_tsts()
  {
    sub_tst(\&httpd_file_tst, "ex_httpd_conf_5");
    sub_tst(\&httpd_file_tst, "ex_httpd_conf_5",
	    {shutdown_w => 0});
    sub_tst(\&httpd_file_tst, "ex_httpd_conf_5",
	    {                 slow_write => 1});
    sub_tst(\&httpd_file_tst, "ex_httpd_conf_5",
	    {shutdown_w => 0, slow_write => 1});
  }

sub all_conf_6_tsts()
  {
    sub_tst(\&httpd_file_tst, "ex_httpd_conf_6");
    sub_tst(\&httpd_file_tst, "ex_httpd_conf_6",
	    {shutdown_w => 0});
    sub_tst(\&httpd_file_tst, "ex_httpd_conf_6",
	    {                 slow_write => 1});
    sub_tst(\&httpd_file_tst, "ex_httpd_conf_6",
	    {shutdown_w => 0, slow_write => 1});
  }

sub munge_mtime
  {
    my $num   = shift;
    my $fname = shift;

    my ($a, $b, $c, $d,
	$e, $f, $g, $h,
	$atime, $mtime) = stat("$ENV{SRCDIR}/tst/ex_httpd_tst_1");
    $atime -= ($num * (60 * 60 * 24));
    $mtime -= ($num * (60 * 60 * 24));
    utime $atime, $mtime, $fname;
  }

sub make_data
  {
    my $num   = shift;
    my $data  = shift;
    my $fname = shift;

    open(OUT, "> $fname") || failure("open $fname: $!");
    print OUT $data;
    close(OUT) || failure("close");

    munge_mtime($num, $fname);
  }

sub make_line
  {
    my $num   = shift;
    my $data  = shift;
    my $fname = shift;
    make_data($num, $data . "\n", $fname);
  }

sub make_html
  {
    my $num   = shift;
    my $val   = shift;
    my $fname = shift;

    my $data = <<EOL;
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 3.2 Final//EN">
<html>
  <head>
    <title>Foo $val</title>
  </head>
  <body>
    <h1>Foo $val</h1>
  </body>
</html>
EOL
    make_data($num, $data, $fname);
  }

sub setup
  {
    my $big = "";

    # Needs to be big or the .bz2 file won't stay around due to the 95% rule
    $big .= ("\n" . ("x" x 10) . ("xy" x 10) . ("y" x 10)) x 500;
    $big .= "\n";

    rmtree($root);
    mkpath([$root . "/default",
	    $root . "/default.example.com",
	    $root . "/blah",
	    $root . "/foo.example.com/nxt",
	    $root . "/foo.example.com/corner/index.html",
	    $root . "/foo.example.com/there",
	    $root . "/foo.example.com:1234"]);

    make_html(1, "root",    "$root/index.html");
    make_html(2, "default", "$root/default/index.html");
    make_html(2, "def$big", "$root/default/index-big.html");
    make_html(3, "norm",    "$root/foo.example.com/index.html");
    make_html(4, "port",    "$root/foo.example.com:1234/index.html");
    make_html(5, "corner",
	      "$root/foo.example.com/corner/index.html/index.html");
    make_html(6, "bt",      "$root/foo.example.com:1234/bt.torrent");
    make_html(7, "plain",   "$root/default/README");
    make_html(8, "backup",  "$root/default/index.html~");
    make_html(9, "welcome", "$root/default/welcome.html");
    make_html(9, "welcome", "$root/default/welcome.txt");
    make_html(0, "",        "$root/default/noprivs.html");
    make_html(0, "privs",   "$root/default/noallprivs.html");
    make_line(10, "a none", "$root/foo.example.com/there/5.2-neg-CT");
    make_line(10, "a txt",  "$root/foo.example.com/there/5.2-neg-CT.txt");
    make_line(10, "a html", "$root/foo.example.com/there/5.2-neg-CT.html");
    make_line(10, "b none", "$root/foo.example.com/there/5.2-neg-AL");
    make_line(10, "b def",  "$root/foo.example.com/there/5.2-neg-AL.txt");
    make_line(10, "b jpfb", "$root/foo.example.com/there/5.2-neg-AL.jpfb.txt");
    make_line(10, "b jp",   "$root/foo.example.com/there/5.2-neg-AL.jp.txt");
    make_line(10, "b fr",   "$root/foo.example.com/there/5.2-neg-AL.fr.txt");
    make_line(10, "c none", "$root/foo.example.com/there/5.2-neg");
    make_line(10, "c deft", "$root/foo.example.com/there/5.2-neg.txt");
    make_line(10, "c defh", "$root/foo.example.com/there/5.2-neg.html");
    make_line(10, "c jpbt", "$root/foo.example.com/there/5.2-neg.jpfb.txt");
    make_line(10, "c jpbh", "$root/foo.example.com/there/5.2-neg.jpfb.html");
    make_line(10, "c jpt",  "$root/foo.example.com/there/5.2-neg.jp.txt");
    make_line(10, "c jph",  "$root/foo.example.com/there/5.2-neg.jp.html");
    make_line(10, "c frt",  "$root/foo.example.com/there/5.2-neg.fr.txt");
    make_line(10, "c frh",  "$root/foo.example.com/there/5.2-neg.fr.html");

    open(OUT,     "> $root/foo.example.com/empty") || failure("open empty: $!");
    munge_mtime(44, "$root/foo.example.com/empty");

    system("$ENV{SRCDIR}/gzip-r.pl --force --type=all $root");
    munge_mtime(0, "$root/index.html.gz");
    munge_mtime(0, "$root/index.html.bz2");
    munge_mtime(0, "$root/default/index.html.gz");
    munge_mtime(0, "$root/default/index.html.bz2");
    munge_mtime(0, "$root/foo.example.com/index.html.gz");
    munge_mtime(0, "$root/foo.example.com/index.html.bz2");
    munge_mtime(0, "$root/foo.example.com:1234/index.html.gz");
    munge_mtime(0, "$root/foo.example.com:1234/index.html.bz2");

    chmod(0000, "$root/default/noprivs.html");
    chmod(0600, "$root/default/noallprivs.html");

    system("mkfifo $root/default/fifo");

    my ($a, $b, $c, $d,
	$e, $f, $g, $h,
	$atime, $mtime) = stat("$ENV{SRCDIR}/tst/ex_cat_tst_4");
    copy("$ENV{SRCDIR}/tst/ex_cat_tst_4", "$root/default/bin");
    utime $atime, $mtime, "$root/default/bin";
  }

my $clean_on_exit = 1;
if (@ARGV)
  {
    $clean_on_exit = 0;
    my $cntl_file = shift;
    my $bind_addr = undef;

    daemon_status($cntl_file);

    while (@ARGV)
      {
	my $arg = shift;
	my $y = 0;

	if ($arg eq "setup")
	  { setup(); }
	elsif ($arg eq "trunc")
	  { $truncate_segv = !$truncate_segv; }
	elsif ($arg eq "cntl")
	  { $cntl_file = shift; daemon_status($cntl_file, $bind_addr); }
	elsif ($arg eq "addr")
	  { $bind_addr = shift; daemon_status($cntl_file, $bind_addr); }
	elsif ($arg eq "cleanup")
	  { $clean_on_exit = !$clean_on_exit; }
	elsif (($arg eq "virtual-hosts") || ($arg eq "vhosts"))
	  { all_vhost_tsts(); $y = 1; }
	elsif ($arg eq "public")
	  { all_public_only_tsts(); $y = 1; }
	elsif ($arg eq "none")
	  { all_none_tsts(); $y = 1; }
	elsif ($arg eq "conf_5")
	  { all_conf_5_tsts(); $y = 1; }
	elsif ($arg eq "conf_6")
	  { all_conf_6_tsts(); $y = 1; }
	elsif (($arg eq "non-virtual-hosts") || ($arg eq "non-vhosts"))
	  { all_nonvhost_tsts(); $y = 1; }

	print "-" x 78 . "\n" if ($y);
      }

    success();
  }

our $conf_args_nonstrict = " --configuration-data-jhttpd '(policy <default> (unspecified-hostname-append-port off) (secure-directory-filename no) (HTTP (header-names-strict false)))'";
our $conf_args_strict = " --configuration-data-jhttpd '(policy <default> (secure-directory-filename no) (unspecified-hostname-append-port off))'";

sub httpd_vhost_tst
  {
    daemon_init("ex_httpd", $root, shift);
    system("cat > $root/default/fifo &");
    http_cntl_list();
    all_vhost_tsts();
    daemon_exit();
  }

sub conf_tsts
  {
    my $beg = shift;
    my $end = shift;
    my $args = '';

    for ($beg..$end)
      { $args .= " -C $ENV{SRCDIR}/tst/ex_conf_httpd_tst_$_"; }

    daemon_init("ex_httpd", $root, $args);
    my $list_pid = http_cntl_list();

    for ($beg..$end)
      {
	if (0) {}
	elsif ($_ == 1)
	  {
	    daemon_status("ex_httpd_cntl", "127.0.0.1");
	    all_vhost_tsts();
	    my $old_trunc = $truncate_segv;
	    $truncate_segv = 1;
	    daemon_status("ex_httpd_cntl", "127.0.0.2");
	    all_vhost_tsts();
	    $truncate_segv = $old_trunc;
	    daemon_status("ex_httpd_cntl", "127.0.0.3");
	    all_vhost_tsts();
	  }
	elsif ($_ == 2)
	  {
	    daemon_status("ex_httpd_cntl", "127.0.1.1");
	    all_public_only_tsts("no gen tsts");
	  }
	elsif ($_ == 3)
	  {
	    daemon_status("ex_httpd_cntl", "127.0.2.1");
	    all_nonvhost_tsts();
	  }
	elsif ($_ == 4)
	  {
	    daemon_status("ex_httpd_cntl", "127.0.3.1");
	    all_none_tsts();
	  }
	elsif ($_ == 5)
	  {
	    daemon_status("ex_httpd_cntl", "127.0.4.1");
	    all_conf_5_tsts();
	  }
	elsif ($_ == 6)
	  {
	    daemon_status("ex_httpd_cntl", "127.0.5.1");
	    all_conf_6_tsts();
	  }
      }

    daemon_exit();
  }


END {
  my $save_exit_code = $?;
  if ($clean_on_exit)
    { daemon_cleanup(); }
  $? = $save_exit_code;
}

1;
