#! /usr/bin/perl -w
 
use strict;
use FileHandle;
 
my $docs = undef;
 
if (0) {}
elsif (-x "../configure") # In docs dir...
  {
    $docs ="../Documentation";
  }
elsif (-x "../../configure") # in build subdir
  {
    $docs ="../../Documentation";
  }
 
if (!defined ($docs))
  {
    STDERR->print("Can't find configure.\n");
    exit (1);
  }

my $title = "Classification of Red Hat security alerts 2003";

my $html_header = "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML//EN\">\n<html>\n<head>\n";
my $html_body = <<EOF;
    <style type="text/css">
      A:visited { color: #ff4040; }
      A:hover { color: #20b2aa; }

      P { text-indent: 1cm; }

      body { background: #FFF; }

  tr.rh { font-weight: bold; font-size: 150%;
          background: #DDD; }

  tr.r10                   { background: #fff; }
  tr.r10:hover             { background: #bbb; }
  tr.r20                   { background: #eee; }
  tr.r20:hover             { background: #bbb; }
  tr.r11                   { background: #fff; text-decoration: underline; }
  tr.r11:hover             { background: #bbb; text-decoration: underline; }
  tr.r21                   { background: #eee; text-decoration: underline; }
  tr.r21:hover             { background: #bbb; text-decoration: underline; }


      strong.sapi  { color: #F00; background: white; }
      strong.msapi { color: #00F; background: white; }

    </style>

  </head>
  <body>
EOF

my $html_footer = "\n</body></html>";


open(OUT, "> security_problems.html") || die "open(security_problems.html): $!";

open(CSV, "> security_problems.csv")  || die "open(security_problems.csv): $!";

print OUT $html_header;
print OUT "<title>", $title, "</title>", "\n";
print OUT $html_body;
print OUT "<h1>", $title, "</h1>", "\n";

print OUT <<EOL;
 <p> This is a list of vulnerabilities release in 2003, with an
attempt to classify them into if they would have been avoided if the code had
used a real dynamic string API and to classify them into a Vulnerability range
(ie. Can you only exploit it from the local machine or can you do it remotely).
 However note that esp. in the later classifications it\'s possible that my
classification is wrong for your environment (for instance if you don\'t pass
untrusted data from the network to unzip, that is a local vulnerability,
if you only allow connections from the local machine to postgresql that becomes
a local vulnerability, if you download arbitrary themes from the network
and load them into WindowMaker that becomes a remote vulnerability or if you
run commands over untrusted NFS mounts then most filesystem vulnerabilities can
become remote vulnerabilities). I\'ve tried to classify each with the most
commmonly expected value.
 <p> A <strong class="sapi">RED</strong> error indicates an error
that <b>could not have occured</b> if the program had been using a
<a href="comparison">real
dynamic string API</a>. </p>
 <p> A <strong class="msapi">BLUE</strong> error indicates an error
that <b>could not have occured</b> it the program had been using
<a href="overview">Vstr</a>. </p>
 <p> You can go straight to the
     <a href="#summary-types">summary of the types</a> of the vulnerabilities or
to the
     <a href="#summary-range">summary of the range</a> of the vulnerabilities.
EOL

print OUT "<table width=\"100%\">";
print OUT "<tr class=\"rh\"><td>" . "Red Hat Package";
print OUT "</td><td>" . "Types of Vulnerability";
print OUT "</td><td>" . "Range of Vulnerability";
print OUT "</td><td>" . "Range, if Vstr was used";
print OUT "</td><td>" . "Range, if any dynamic string API was used";
print OUT "</td></tr>\n";

print CSV <<EOL;
"Advisory count","Red Hat Package","Red Hat Advisory","Type of Vulnerability","Range of Vulnerability"
EOL

open (IN, "< $docs/security_problems.txt") ||
  die "open(security_problems.txt): $!";

my $name = '';
my $ref = {rh => ''};

my @t = ();
my @r = ();

my $t_out =
  {
   OBO => sub { '<strong class="sapi">Off By One</strong><br>' },
   BO  => sub { '<a href="http://www.dwheeler.com/secure-programs/Secure-Programs-HOWTO.html#BUFFER-OVERFLOW"><strong class="sapi">Buffer Overflow</strong></a><br>' },
   IO => sub { '<strong class="sapi">Integer Overflow</strong><br>' },
   XSS => sub { 'Cross Site Scripting<br>' },
   IE => sub { my $t = shift || 'All'; 'Improper Encoding (' . $t . ')<br>' },
   VSTRIE => sub { '<strong class="msapi">Improper Encoding</strong><br>' },
   IV => sub { my $t = shift || ''; 'Input Validation' . $t . '<br>' },
   VSTRIV => sub { '<strong class="msapi">Input Validation</strong><br>' },
   DF => sub { 'Double Free<br>' },
   FIU => sub { 'Free memory that is in use<br>' },
   FUM => sub { 'Free uninitialized memory location<br>' },
   VSTRFIU => sub { '<strong class="msapi">Free memory that is in use</strong><br>' },
   TF => sub { 'Temporary File Creation<br>' },
   IL => sub { 'Information Leak<br>' },
   APE => sub { my $t = shift || "All"; 'Authenticated Privilage Escalation (' . $t . ')<br>' },
   PE => sub { my $t = shift || "All"; 'Privilage Escalation (' . $t . ')<br>' },
   DOS => sub { my $t = shift || "All"; 'Denial of Service (' . $t . ')<br>' },
   MITM => sub { 'Man in the Middle<br>' },
   BP => sub { my $t = shift || "All"; 'Broken Packaging (' . $t . ')<br>' },
  };
my $t_sapi =
  {
   OBO => 1,
   BO => 1,
   IO => 1,
   XSS => 0,
   IE => 0,
   VSTRIE => -1,
   IV => 0,
   VSTRIV => -1,
   DF => 0,
   FIU => 0,
   FUM => 0,
   VSTRFIU => -1,
   TF => 0,
   IL => 0,
   APE       => 0,
   PE        => 0,
   DOS => 0,
   MITM => 0,
   BP => 0,
  };
my $t_cnt =
  {
   OBO       => 0,
   BO        => 0,
   IO        => 0,
   XSS       => 0,
   IE        => 0,
   VSTRIE    => 0,
   IV        => 0,
   VSTRIV    => 0,
   DF        => 0,
   FIU       => 0,
   FUM       => 0,
   VSTRFIU   => 0,
   TF        => 0,
   IL        => 0,
   APE       => 0,
   PE        => 0,
   DOS       => 0,
   MITM      => 0,
   BP => 0,
  };

my $r_out =
  {
   'P' => 'Possibly remote vulnerability',
   'R' => 'Remote vulnerability',
   'L' => 'Local vulnerability',
   'N' => '<strong class="sapi">Not applicable</strong>',
   'VN' => '<strong class="msapi">Not applicable</strong>',
  };
my $r_outs =
  {
   'P' => 'Possible remote vulnerabilities',
   'R' => 'Remote vulnerabilities',
   'L' => 'Local vulnerabilities',
  };

my $row = 0;

my $cnt_stup =
  {
   'P' => 0,
   'R' => 0,
   'L' => 0,
  };
my $cnt_vstr =
  {
   'P' => 0,
   'R' => 0,
   'L' => 0,
  };
my $cnt_dstr =
  {
   'P' => 0,
   'R' => 0,
   'L' => 0,
  };

sub t_main
  {
    $_ = $_[0];
    /^(\w+)(?: \((.+)\))?$/;

    my $t = $1;
    my $x = $2;

    if (!defined($t_cnt->{$t}))
      { die "Not found type (" . $t . ").\n"; }

    return ($t);
  }
sub t_desc
  {
    $_ = $_[0];
    /^(\w+)(?: \((.+)\))?$/;

    my $t = $1;
    my $x = $2;

    return ($t_out->{$t}->($x));
  }

my $csv_count = 0;

while (<IN>)
  {
    if (0) {}
    elsif (/^(Name): (.*)$/)
      {
	@t = ();
	@r = ();
	$name = $2;
      }
    elsif (/^(RH-ref): (.*)$/)
      { $ref->{rh} = $2; }
    elsif (/^(Type): (.*)$/)
      {
	push @t, $2;
	push @r, 'P';
      }
    elsif (/^(Range): (.*)$/)
      {
	$r[-1] = $2;
      }
    elsif (/^$/)
      {
	# Output info...

	$row = ($row % 2) + 1;

	my $stupid_error = 1;

	my $r_val_stup = 'P';
	my $r_val_dstr = 'P';
	my $r_val_vstr = 'P';

	for my $i (0..$#t)
	  {
	    my $t_name = t_main($t[$i]);

	    ++$t_cnt->{$t_name};

	    if (!$t_sapi->{$t_name})
	      {
		if ($r_val_vstr ne 'R')
		  { $r_val_vstr = $r[$i]; }
		if ($r_val_dstr ne 'R')
		  { $r_val_dstr = $r[$i]; }
		if ($r_val_stup ne 'R')
		  { $r_val_stup = $r[$i]; }
		$stupid_error = 0;
	      }
	    elsif ($stupid_error && ($t_sapi->{$t_name} == -1))
	      {
		if ($r_val_dstr ne 'R')
		  { $r_val_dstr = $r[$i]; }
		if ($r_val_stup ne 'R')
		  { $r_val_stup = $r[$i]; }
		$stupid_error = -1;
	      }
	    else
	      {
		if ($r_val_stup ne 'R')
		  { $r_val_stup = $r[$i]; }
	      }
	  }

print OUT "<tr class=\"r${row}${stupid_error}\"><td>";

$_ = $ref->{rh}; s/(\d{4}):(\d{3})-\d{2}/$1-$2/;
my $url = 'http://rhn.redhat.com/errata/' . $_ . '.html';
print OUT '<a href="' . $url . '">';

if (0) {}
elsif ($stupid_error ==  0)
  {
    ++$cnt_vstr->{$r_val_vstr};
    ++$cnt_dstr->{$r_val_dstr};
    ++$cnt_stup->{$r_val_stup};
    print OUT $name;
  }
elsif ($stupid_error == -1)
  {
    $r_val_vstr = 'VN';
    ++$cnt_dstr->{$r_val_dstr};
    ++$cnt_stup->{$r_val_stup};
    print OUT '<strong class="msapi">' . $name . '</strong>';
  }
elsif ($stupid_error ==  1)
  {
    $r_val_vstr = 'N';
    $r_val_dstr = 'N';
    ++$cnt_stup->{$r_val_stup};
    print OUT '<strong class="sapi">' .  $name . '</strong>';
  }

print OUT "</a>";
print OUT "</td><td>";
	for (@t)
	  {
	    OUT->print(t_desc($_));
	  }

	print OUT "</td><td>";
	print OUT $r_out->{$r_val_stup};
	print OUT "</td><td>";
	print OUT $r_out->{$r_val_vstr};
	print OUT "</td><td>";
	print OUT $r_out->{$r_val_dstr};

print OUT "</td></tr>\n";

	++$csv_count;
	for my $i (0..$#t)
	  {
	    print CSV <<EOL;
$csv_count,$name,$url,$t[$i],$r[$i]
EOL
	  }
      }
  }

print OUT <<EOL;
</table>
<br>
<a id="summary-types"><h2>Summary of types of vulnerabilities</h2></a>
<table width="100%">
<tr class="rh"><td> Type of Vulnerability </td><td> Number of Vulnerabilities </td></tr>

EOL


for (sort { $t_cnt->{$b} <=> $t_cnt->{$a} } keys %$t_cnt)
{
  if (!$t_cnt->{$_}) { last; }

  print OUT "<tr><td>";
  print OUT $t_out->{t_main($_)}->();
  print OUT "</td><td>";
  print OUT $t_cnt->{t_main($_)};
  print OUT "</td></tr>\n";
}

print OUT <<EOL;
</table>
<a id="summary-range"><h2>Summary of range of vulnerabilities</h2></a>

<h3>What happened</h3>
<table width="100%">
<tr class="rh"><td> Range of Vulnerability </td><td> Number of Vulnerabilities </td></tr>
EOL

for ('R', 'P', 'L')
{
  print OUT "<tr><td>";
  print OUT $r_outs->{$_};
  print OUT "</td><td>";
  print OUT $cnt_stup->{$_};
  print OUT "</td></tr>\n";
}

my $stup_tot = $cnt_stup->{R} + $cnt_stup->{P} + $cnt_stup->{L};

print OUT "<tr><td>";
print OUT "All Vulnerabilities";
print OUT "</td><td>";
print OUT $stup_tot;
print OUT "</td></tr>\n";

print OUT <<EOL;
</table>
<h3>What would have happened if all the packages had used a dynamic string API</h3>
<table width="100%">
<tr class="rh"><td> Range of Vulnerability </td><td> Number of Vulnerabilities </td></tr>
EOL

my $tot = undef;
my $cnt_p = undef;

for ('R', 'P', 'L')
{
  $cnt_p = sprintf("%d%%", (($cnt_dstr->{$_} * 100) / $cnt_stup->{$_}));

  print OUT "<tr><td>";
  print OUT $r_outs->{$_};
  print OUT "</td><td>";
  print OUT $cnt_dstr->{$_} . " ($cnt_p)";
  print OUT "</td></tr>\n";
}

$tot = $cnt_dstr->{R} + $cnt_dstr->{P} + $cnt_dstr->{L};
$cnt_p = sprintf("%d%%", (($tot * 100) / $stup_tot));

print OUT "<tr><td>";
print OUT "All Vulnerabilities";
print OUT "</td><td>";
print OUT $tot . " ($cnt_p)";
print OUT "</td></tr>\n";

print OUT <<EOL;
</table>
<h3>What would have happened if all the packages had used <a href="overview">Vstr</a></h3>
<table width="100%">
<tr class="rh"><td> Range of Vulnerability </td><td> Number of Vulnerabilities </td></tr>
EOL

for ('R', 'P', 'L')
{
  $cnt_p = sprintf("%d%%", (($cnt_vstr->{$_} * 100) / $cnt_stup->{$_}));

  print OUT "<tr><td>";
  print OUT $r_outs->{$_};
  print OUT "</td><td>";
  print OUT $cnt_vstr->{$_} . " ($cnt_p)";
  print OUT "</td></tr>\n";
}

$tot = $cnt_vstr->{R} + $cnt_vstr->{P} + $cnt_vstr->{L};
$cnt_p = sprintf("%d%%", (($tot * 100) / $stup_tot));

print OUT "<tr><td>";
print OUT "All Vulnerabilities";
print OUT "</td><td>";
print OUT $tot . " ($cnt_p)";
print OUT "</td></tr>\n";


print OUT <<EOL;
</table>
EOL

print OUT $html_footer;
