#!/usr/bin/perl
#
# Copyright 1994-2000 Werner Almesberger.
# All rights reserved.
#
# See file COPYING for details.
#
#
$VERSION = "29-APR-2000";
#
#-----------------------------------------------------------------------------
#
# Known bugs:
#
#   Usually doesn't check for prepended backslashes, e.g. things like
#   \\begin{verbatim} would be processed incorrectly.
#
#   Tokenization should be done once at the beginning, not on the fly
#   with cleanup and check procedures at the end of each step.
#
#   Table handling is fairly basic: nested tables don't work and
#   variable borders (e.g. \begin{tabular}{lll} ... \multicolumn{1}{l|} ...)
#   don't format correctly.
#
#-----------------------------------------------------------------------------
#
# Style parameters:
#
#  $WIDTH		line width (characters)
#  $LENGTH		page length, without header and footer (lines)
#  $numbered_sections	number sections (0/1)
#  $indent_sections	indent sections according to nesting depth (0/1)
#  $paged		fill to page size, add header and footer (0/1)
#  $center_figures	center figures (from .asc files)

# default settings (plain)
$WIDTH = 75;

if ($ARGV[0] eq "-id") {
    pop(@ARGV);
    $WIDTH = 72;
    $LENGTH = 58;
    $numbered_sections = 1;
    $indent_sections = 1;
    $paged = 1;
}
elsif ($ARGV[0] eq "-plain") {
    pop(@ARGV);
    # use default
}
elsif ($ARGV[0] eq "-V") {
    print STDERR "version $VERSION\n";
    exit(0);
}
elsif ($ARGV[0] =~ /^-/) {
    die "unrecognized option \"$ARGV[0]\"";
}
#
# default macros
#
$m{"\\\\ldots"} = "...";
#
# read the file
#
print STDERR "[".length($t)."] Reading the file\n";
$/ = "\000";
$t = "\n".<>."\n";
if ($t =~ /^\n%%format\s+([^\n]*)\n/) {
    $t = "\n".$';
    for (split(/\s+/,$1)) {
	if ($_ =~ /^width=(\d+)$/) {
	    $WIDTH = $1;
	}
	elsif ($_ =~ /^length=(\d+)$/) {
	    $LENGTH = $1;
	}
	elsif ($_ =~ /^([-+])numbered_sections$/) {
	    $numbered_sections = $1 eq "+";
	}
	elsif ($_ =~ /^([-+])indent_sections$/) {
	    $indent_sections = $1 eq "+";
	}
	elsif ($_ =~ /^([-+])paged$/) {
	    $paged = $1 eq "+";
	}
	else {
	    die "unrecognized %%format option \"$_\"";
	}
    }
}
#
# universal markers
#
$N = "\000";	# non-character
$X = "\007";	# generic marker
$Y = "\010";	# another generic marker
$Z = "\011";	# yet another generic marker
$B = "\001";	# begin
$E = "\002";	# end
$BS = "\013";	# second begin
$ES = "\014";	# second end
$CO = "\003";	# curly open
$CC = "\004";	# curly close
#
# begin/endfigure marker
#
$BF = "\015";	# %%beginfigure
$EF = "\016";	# %%endfigure
#
# commands to the output formatter
#
$SI = "\020";	# increase indentation by one
$SO = "\021";	# decrease indentation by one
$B1 = "\022";	# one blank line
$B2 = "\023";	# two blank lines
$TB = "\024";	# begin transaction
$TE = "\025";	# end transaction
$VL = "\026";	# verbatim line

sub xlat
{
    local ($l) = @_;

    $l =~ s/\\backslash */$X/g;		# \backslash ->\
    $l =~ s/~/$Y/g;
    $l =~ s/\\$Y/~/g;
    $l =~ s/$Y/ /g;
    $l =~ s/\\([_&%^\$\#\[\]|\-])/\1/g;# unescape special characters
    $l =~ s/\\,//g;			# remove small spaces
    if ($l =~ /\\([A-Za-z]+|.)/) {
	warn "unrecognized command $& ($l)";
	$l = $`."\n!!! UNRECOGNIZED COMMAND: $&\n$'";
    }
    $l =~ s/$X/\\/g;
    $l =~ tr/{}//d;			# delete stray curly braces
    $l =~ s/$CO/{/g;			# put escaped braces back
    $l =~ s/$CC/}/g;
    return $l;
}


sub compose
{
    local ($s,$left,$n,$spc);
    local (@tmp) = @_;

    for (@tmp) {
	s/PAGE/$page/;
    }
    for (@tmp) { $s += length($_); }
    $left = $WIDTH-$s;
    $n = $#_;
    for (@tmp) {
	print $_;
	$spc = $n ? int($left/$n) : 0;
	print " " x $spc;
	$left -= $spc;
	$n--;
    }
    print "\n";
}


sub print
{
    local ($l) = @_;

    $l =~ s/ *$//;
    if (!$paged) {
	print $l;
	return;
    }
    if ($transaction) {
	push(@queue,$l);
	return;
    }
    if (!$line) {
	if ($page) {
	    &compose(@head);
	}
	else {
	    print "\n";
	    $page++;
	}
	print "\n\n";
	$line = 3;
    }
    return if $line == 3 && $l =~ /^\s*\n/;
    if ($line++ < $LENGTH-3) {
	$l =~ s/[ \t]*\n/\n/;
	print $l;
	return;
    }
    print "\n\n";
    &compose(@foot);
    print "\014";
    $line = 0;
    return if $terminating;
    $page++;
    &print($l);
}


sub flush
{
    $transaction = 0;
    if ($line+$#queue+1 > $LENGTH-3) {
	while ($line != 3) { &print("\n"); }
    }
    for (@queue) { &print($_); }
    undef @queue;
}


sub verbatim
{
    local ($b) = @_;

    $b =~ s/~/$X/g;
    $b =~ s/ /~/g;
    $b =~ s/\\/\\backslash /g;
    $b =~ s/$X/\\~/g;
    $b =~ s/[_^%#&{}\$\-]/\\$&/g;
    $b =~ s/[`']/\\$&~/g;
    $b =~ s/\n\n\n/$B2/g;
    $b =~ s/\n\n/$B1/g;
    $b =~ s/\n/\\\\/g;
    return $b;
}

#
# load macros
#
print STDERR "[".length($t)."] Loading macros\n";
while ($t =~ /\n%%(def|cmd)([^\n]*)\n/) {
    $t = $`."\n".$';
    $a = $1;
    $2 =~ /([^\\])=/ || die "= missing in $2";
    if ($a eq "def") {
	$m{$`.$1} = $';
	$c{$`.$1} = "";
    }
    else {
	$m{$`.$1} = "";
	$c{$`.$1} = $';
    }
}
#
#  handle \input
#
print STDERR "[".length($t)."] Loading \\input files (if any)\n";
while ($t =~ /\n\s*\\input\s+(\S+)\s*\n/) {
    $t = $`."\n";
    open(FILE,$1) || die "open $1: $!";
    while ($l = <FILE>) { $t .= $l; }
    close FILE;
    $t .= $';
}
#
# remove %%beginskip ... %%endskip pairs
#
print STDERR "[".length($t)."] Removing %%beginskip ... %%endskip pairs\n";
while ($t =~ /\n%%beginskip\s*\n/) { $t = $`.$B.$'; }
while ($t =~ /\n%%endskip\s*\n/) { $t = $`.$E.$'; }
while ($t =~ /$B[^$B$E]*$E/) { $t = $`."\n".$'; }
$t !~ /[$B$E]/ || die "%%beginskip/%%endskip mismatch";
#
# handle %%head and %%foot
#
while ($t =~ /\n%%head\s+(\S.*\S)\s*\n/) {
    $t = $`."\n".$';
    push(@head,$1);
}
while ($t =~ /\n%%foot\s+(\S.*\S)\s*\n/) {
    $t = $`."\n".$';
    push(@foot,$1);
}
#
# Handle begin/endfigure
#
while ($t =~ /\n%%beginfigure\s*(\S+)\s*\n/) { $t = $`.$BF.$1.$BF."\n".$'; }
while ($t =~ /\n%%endfigure\s*([^\n]*\S)\s*\n/) { $t = $`.$EF.$1.$EF."\n".$'; }
#
#  process macros
#
print STDERR "[".length($t)."] Processing macros (may take a while)\n";
while (1) {
    $none = 1;
    for (keys %m) {
	while ($t =~ /$_/) {
	    $none = 0;
	    if ($c{$_} eq "") {
		eval "\$t = \$`.\"$m{$_}\".\$';";
	    }
	    else {
		eval "\$t = \$`.$c{$_}.\$';";
	    }
	    die "syntax error: $@" if $@;
	}
    }
    last if $none;
    print STDERR "[".length($t)."] "."  next pass\n";
# perfectionist's approach:
#    $l = 0;
#    for (keys %m) {
#	if ($t =~ /$_/) {
#	    if (length($&) > $l) {
#		$i = $_;
#		$l = length($&);
#	    }
#	}
#    }
#    last if !$l;
#    $t =~ /$i/ || die "internal error";
#    eval "\$t = \$`.\"$m{$i}\".\$'";
#    die "syntax error: $@" if $@;
#    print STDERR "[".length($t)."] "."$i\n";
}
#
# handle verbatim sections (we're not trying to be perfect here)
#
print STDERR "[".length($t)."] Handling verbatim sections\n";
while ($t =~ /\\begin{verbatim}([ \t]*\n)?/) { $t = $`."\n\n".$B.$'; }
while ($t =~ /\\end{verbatim}([ \t]*\n)?/) { $t = $`.$E."\n\n".$'; }
while ($t =~ /\\verb([^a-zA-Z \t\n])/ && $t =~ /\\verb$1([^$1]*)$1/) {
    $t = $`.$B.$1.$E.$';
}
while ($t =~ /$B([^$B$E]*)$E/) {
    ($a,$b,$c) = ($`,$1,$');
    die "no support for \\t yet, sorry" if $b =~ /\t/;
    $b = &verbatim($b);
    $t = $a.$b.$c;
}
if ($t =~ /[$B$E]/) {
    if ($t =~ /..........[$B$E]........../) { print STDERR "$&\n"; }
    die "verbatim conflict";
}
#
# hide escaped curly braces
#
print STDERR "[".length($t)."] Hiding escaped curly braces\n";
$t =~ s/\\{/$CO/g;
$t =~ s/\\}/$CC/g;
#
# discard comments and italic corrections
#
print STDERR "[".length($t)."] Discarding comments and italic corrections\n";
while ($t =~ s/([^\\])%[^\n]*\n/$1/g) {};
$t =~ s|\\/||g;
#
# no math mode
#
print STDERR "[".length($t)."] No math mode\n";
while ($t =~ s/([^\\])\$/$1/g) {};
#
# remove tabs and massage blanks
#
print STDERR "[".length($t)."] Removing tabs and massaging blanks\n";
$t =~ s/\\ / /g;	# \cmd\ blah
$t =~ tr/ \t/ /s;
#
# various minor issues
#
print STDERR "[".length($t)."] Dealing with various minor issues\n";
$t =~ s/\\rightarrow\s*/->/g;
$t =~ s/\\quad\s*/~/g;
$t =~ s/\\qquad\s*/~~/g;
$t =~ s/\\vert/|/g;
$t =~ s/\\TeX/TeX/g;
$t =~ s/\\LaTeX/LaTeX/g;
$t =~ s/\\rm\s*//g;
$t =~ s/\\hbox{/{/g;
$t =~ s/\\protect//g;
$t =~ s/\\newpage\s*//g;
$t =~ tr/-/-/s;
$t =~ s/\n\n+/$B1/g;
$t =~ s/\\documentclass(\[[^]]*\])?{[^}]*}//;
$t =~ s/\\begin{document}//;
$t =~ s/\\end{document}(\n|.)*//;
$t =~ s/\\begin{center}//g;
$t =~ s/\\end{center}//g;
while ($t =~ /\\cite{([^}]+)}/) {
    $t = $`."[";
    $after = $';
    for (split(",",$1)) {
	if (defined $cite{$_}) { $t .= "$cite{$_},"; }
	else {
	    $cite{$_} = ++$citation;
	    $bibref[$citation] = $_;
	    $t .= "$citation,";
	    die "unmatched ref $_" unless $after =~ /\\bibitem{$_}/;
	    $after = $`."\\item[\[$citation\]] ".$';
	}
    }
    $t =~ s/,$//;
    $t .= "]$after";
}
$t =~
  s/\\begin{thebibliography}{[^}]*}/\\section{References}\\begin{description}/;
$t =~ s/\\end{thebibliography/\\end{description}/;
#
# Flag unused bibitems
#
while ($t =~ /\\bibitem{([^}]+)}/) {
    print STDERR "Unused: \\bibitem{$1}\n";
    $t = $`."\\item[\[$1\]] ".$';
}
#
# handle footnotes
#
print STDERR "[".length($t)."] Handling footnotes\n";
$t =~ s/\\footnote{/\\footnotemark\\footnotetext{/g;
$t =~ s/\\footnotemark/$X/g;
$t =~ s/\\footnotetext{/$Y/g;
while ($t =~ /$X([^$Y]*)$Y/) {
    ($a,$b,$c) = ($`,$',$1);
    $t =~ /^[^$Y]*$Y$B1/;
    $d = $';
    for ($s = "*"; $d =~ /$Z/; $d = $`.$Y.$') { $s .= "*"; }
    $a = $a.$s.$c;
    while ($b =~ /^([^}]*){([^{}]*)}/) { $b = $`.$1.$B.$2.$E.$'; }
    $b =~ /^([^{}]*)}/ || die "{ } confusion";
    ($b,$t) = ($1,$');
    $b =~ s/$B/{/g;
    $b =~ s/$E/}/g;
    $d = "$B1$Z\\begin{description}\\item[$s] $b\\end{description}$B1";
    if ($t =~ /$B1([^$Z][^$N]*)$/) { $t = $`.$d.$1; }
    else { $t = $t.$d; }
    $t = $a.$t;
}
$t =~ s/$Z//g;
if ($t =~ /[$X$Y$Z$B$E]/) {
    if ($t =~ /..............[$X$Y$Z$B$E]/) { print STDERR "HEY $&\n"; }
    die "footnote confusion";
}
#
# process simple tables ...
#
print STDERR "[".length($t)."] Processing simple tables\n";

sub draw_line # helper function
{
    local ($i,$has_work);

    $has_work = 0;
    for (@l) {
	next unless $_;
	$has_work = 1;
	last;
    }
    return unless $has_work;
    $i = 0;
    for (@d) {
	if ($_ =~ /^\|/) { $a .= $l[$i] ? "--" : $bar[$i] ? "| " : "~ "; }
	$a .= ($l[$i] ? "-" : "~") x ($w[$i]+1);
	if ($_ =~ /\|$/) {
	    $a .= $l[$i+1] ? "--" : $l[$i] ? "- ": $bar[$i+1] ? "| " : "~ ";
	}
	$i++;
    }
    $a .= "\\\\";
    @l = ();
}

while ($t =~ /\\begin{tabular}/) { $t = $`.$B.$'; }
while ($t =~ /\\end{tabular}/) { $t = $`.$E.$'; }
while ($t =~ /$B\{([rlc|]+)\}([^$B$E]*)$E/) {
    ($a,$b,$c,$d) = ($`,$',$2,$1);
print STDERR "doing it\n";
    $c =~ s/\\\\/$X&/g;	# overload - we remove $X& when splitting by &
    $c =~ s/[\s\n]*\\hline[\s\n]*/$X &/g;
    $c =~ s/[\s\n]*\\cline{([^}]*)}[\s\n]*/$X$1 &/g;
    $c =~
      s/[\s\n]*\\multicolumn{(\d+)}{([^}]+)}({[^}]*})[\s\n]*&?/$Y$1 $2 $3&/g;
    ($e = $d) =~ tr/|//cd;
    @d = ();
    while ($d =~ /^(\|*)[a-z](\|*)/) {
	push(@d,$&);
	$d = $';
    }
    @f = ();
    $i = 0;
    while ($c =~ /([^\\])&/) {
	$c = $';
	local ($x,$y) = ($`,$1);
	local ($fill) = 0;
	if ($x =~ /^$Y(\d+) /) {
	    $i += $1;
	    if ($c =~ /^\s*$X&/) {
		# special case: last \multicolumn before \\ doesn't imply &
		$c = $';
		$fill = 1;
 	    }
	}
	else {
	    $i++ unless $x =~ /^$X/;
	}
	if ($y ne $X) {
	    push(@f,$x.$y);
	}
	else {
	    push(@f,$x);
	    $fill = 1;
	}
	if ($fill) {
	    while ($i % @d) {
		push(@f,"");
		$i++;
	    }
	}
    }
    if ($c =~ /(\S)\s*$/) {
	push(@f,$`.$1);
	$c = "$E$'";
	$i++;
    }
    while ($i % @d) {
	push(@f,"");
	$i++;
    }
    @w = ();
    $d =~ tr/|//d;
    $i = 0;
    for (@f) {
	next if $_ =~ /^$X/;
	if ($_ =~ /^$Y(\d+)/) {
	    $i += $1;
	    next;
	}
	$f = $i % @d;
	$_ = &xlat($_);
	$_ =~ s/^[\s\n]*//g;
	$_ =~ s/[\s\n]*$//g;
	if ($w[$f] < length($_)) { $w[$f] = length($_); }
	$i++;
    }
    $i = 0;
    for (@f) {
	next if $_ =~ /^$X/;
	if ($_ !~ /^$Y(\d+) (\S+) {([^}]*)}/) {
	    $i++;
	    next;
	}
	local ($a,$b,$c) = ($1,$2,$3);
	$f = $i % @d;
	$c = &xlat($c);
	$c =~ s/^[\s\n]*//g;
	$c =~ s/[\s\n]*$//g;
	$s = $a-1-($b =~ s/\|/\|/g)*2;
	for ($j = $i; $j < $i+$a; $j++) {
	    $s += $w[$j % @d]+($d[$j % @d] =~ s/\|/\|/g)*2;
	}
	$_ = "$Y$a $s $b {$c}";	# NB: format change to include computed length !
	$s = length($c)-$s;
	for ($j = 0; $j < $a; $j++) {
	    next if $s <= 0;
	    $tmp = int($s/($a-$j));
	    $w[$i++ % @d] += $tmp;
	    $s -= $tmp;
	}
    }
    $l = @d+2*length($e)-1;
    for (@w) { $l += $_; }
    $a .= "$B1";
    $i = 0;
    @l = ();
    @bar = ();
    for (@f) {
	if (/^$X\s/) {
	    $a .= ("-" x $l)."\\\\";
	    next;
        }
	if (/^$X(\d+)-(\d+)/) {
	    for ($1..$2) { $l[$_-1] = 1; }
	    next;
	}
	if (!($i % @d)) {
	    &draw_line;
	    @bar = ();
	}
	if (/^$Y(\d+) (\d+) (\S+) {([^}]*)}/) {
	    ($w,$d,$f) = ($2,$3,$4);
	    $i += $1;
	}
	else {
	    $f = $_;
	    $d = $d[$i % @d];
	    $w = $w[$i % @d];
	    $i++;
	}
	if ($d =~ /^\|/) {
	    $a .= "| ";
	    $bar[0] = 1;
	}
	$g = $w-length($f);
	if ($d =~ /l/) { $a .= $f.("~" x $g); }
	if ($d =~ /c/) {
	    $a .= ("~" x int($g/2)).$f.("~" x ($g-int($g/2)));
	}
	if ($d =~ /r/) { $a .= ("~" x $g).$f; }
	$a .= " ";
	if ($d =~ /\|$/) {
	    $a .= "| ";
	    $bar[$i % @d ? $i % @d : @d] = 1;
	}
	if (!($i % @d)) { $a .= "\\\\"; }
    }
    &draw_line;
    $t = $a.$b.$B1;
}
if ($t =~ /[$B$E$X$Y$Z]/) {
    if ($t =~ /(.|\n)(.|\n)(.|\n)(.|\n)(.|\n)(.|\n)[$B$E$X$Y](.|\n)(.|\n)(.|\n)(.|\n)(.|\n)(.|\n)/) { print STDERR "$&\n"; }
    die "\\begin/end{tabular} mismatch";
}
#
# process lists
#
print STDERR "[".length($t)."] Formatting lists\n";
while ($t =~ /\\begin{itemize}\s*/) { $t = $`.$B.$'; }
while ($t =~ /\\end{itemize}\s*/) { $t = $`.$E.$'; }
while ($t =~ /$B[^$B$E]*$E/) {
    ($a,$b,$c) = ($`,$&,$');
    while ($b =~ /\\item\s*/) { $b = $`.$X.$'; }
    while ($b =~ /$X([^$X]*)([$X$E])/) {
	$b = $`."- ".$SI.$SI.$1.$SO.$SO."\\\\"."$2".$';
    }
    $b =~ /$B([^$B$E]*)$E/;
    $t = $a.$SI.$SI.$B1.$1.$SO.$SO."$B1".$c;
}
$t !~ /[$B$E]/ || die "\\begin/\\end{itemize} mismatch";
while ($t =~ /\\begin{enumerate}\s*/) { $t = $`.$B.$'; }
while ($t =~ /\\end{enumerate}\s*/) { $t = $`.$E.$'; }
$num = 0;
while ($t =~ /$B[^$B$E]*$E/) {
    ($a,$b,$c) = ($`,$&,$');
    while ($b =~ /\\item\s*/) { $b = $`.$X.$'; }
    while ($b =~ /$X([^$X]*)([$X$E])/) {
	$num++;
	$b = $`."$num. ".$SI.$SI.$1.$SO.$SO."\\\\"."$2".$';
    }
    $b =~ /$B([^$B$E]*)$E/;
    $t = $a.$SI.$SI.$B1.$1.$SO.$SO."$B1".$c;
}
$t !~ /[$B$E]/ || die "\\begin/\\end{enumerate} mismatch";
while ($t =~ /\\begin{description}\s*/) { $t = $`.$B.$'; }
while ($t =~ /\\end{description}\s*/) { $t = $`.$E.$'; }
while ($t =~ /$B[^$B$E]*$E/) {
    ($a,$b,$c) = ($`,$&,$');
    while ($b =~ /\\item\[/) { $b = $`.$X."[".$'; }
    while ($b =~ /$X\[/) {
	($d,$e) = ($`,$');
	while ($e =~ s/\[([^\[\]]*)\]/$BS$1$ES/g) {};
	$e =~ /^([^\[\]]*)]\s*([^$X]*)([$X$E])/ || die "\item problem (1)";
	$b = $d.$1."~~".$SI.$SI.$2.$SO.$SO."\\\\".$3.$';
	$b =~ s/$BS/[/g;
	$b =~ s/$ES/]/g;
    }
    $b =~ /$B([^$B$E]*)$E/;
    $t = $a.$SI.$SI.$B1.$1.$SO.$SO.$B1.$c;
}
$t !~ /[$X]/ || die "\item problem (2)";
$t !~ /[$B$E]/ || die "\\begin/\\end{description} mismatch";
#
# process figures
#
print STDERR "[".length($t)."] Removing figures\n";
while ($t =~ /\\begin{figure\*?}\s*/) { $t = $`.$B.$'; }
while ($t =~ /\\end{figure\*?}\s*/) { $t = $`.$E.$'; }
while ($t =~ /$BF([^$BF]*)$BF([^$BF$EF]*)$EF([^$EF]*)$EF|$B[^$B$E]*$E/) {
    ($a,$b,$c) = ($`,$&,$');
    if (substr($b,0,1) eq $BF) {
	$l{$1} = ++$figref;
	$t = $a."\n".$B2.$TB.$2.$B1."Figure $figref: $3".$TE.$B2.$c;
    }
    else {
	if ($b =~ /\\epsfig{file=([^,}]+)\.eps/) {
	    $fig = "";
	    open(FILE,"$1.asc") || die "open $1: $!";
	    while ($l = <FILE>) { $fig .= "$VL$l"; }
	    close FILE;
	    $fig =~ s/\n/\n$VL/g;
	    if ($center_figures) {
		$max = 0;
		for (split("\n",$fig)) {
		    $max = length($_)-1 if length($_) > $max;
		}
		if ($max < $WIDTH) {
		    local ($spc);
		    $spc = " " x (($WIDTH-$max)/2);
		    $fig =~ s/$VL/$VL$spc/g;
		}
	    }
	    $t = $a.&verbatim($fig)."\n$B1\nFigure ";
	}
	else {
	    $t = $a."[ Figure ";
	    $c = " ]".$c;
	}
	if ($b =~ /\\label{([^}]*)}/) {
	    $l{$1} = ++$figref;
	    $t .= " $figref";
	}
	if ($b =~ /\\caption{([^}]*)}/) {
	    $t .= ": $1";
	}
	$t .= $c;
    }
}
$t !~ /[$B$E]/ || die "\\begin/\\end{figure} mismatch";
$t !~ /[$BF$EF]/ || die "%%begin/endfigure problem";

#
# process floating tables
#
print STDERR "[".length($t)."] Removing tables\n";
while ($t =~ /\\begin{table\*?}(\[[^]]*\])?\s*/) { $t = $`.$B.$'; }
while ($t =~ /\\end{table\*?}\s*/) { $t = $`.$E.$'; }
while ($t =~ /$B([^$B$E]*)$E/) {
    ($a,$b,$c) = ($`,$1,$');
    $tag = "Table ";
    if ($b =~ /\\label{([^}]*)}/) {
	$l{$1} = ++$tabref;
	$tag .= " $tabref";
	$b = $`.$';
    }
    if ($b =~ /\\caption{([^}]*)}/) {
	$tag .= ": $1";
	$b = $`.$';
    }
    $t = $a.$b.$tag.$c;
}
$t !~ /[$B$E]/ || die "\\begin/\\end{table} mismatch";
$t !~ /[$BF$EF]/ || die "%%begin/endfigure problem";

#
# process sections and labels
#
print STDERR "[".length($t)."] Processing sections and labels\n";
$t =~ s/\\begin{abstract}/\\section*{Abstract}/g;
$t =~ s/\\end{abstract}//g;
$LB = "\005";	# they don't necessarily have to be unique
$SC = "\006";
while ($t =~ /\\label{/) { $t = $`.$LB."{".$'; }
while ($t =~ /\\((sub)*)section(\*)?{/) { $t = $`.$SC.$3.$1."{".$'; }
$l = "";
while (1) {
    if ($t =~ /^([^$LB$SC]*)$LB\{([^{}]*)\}\s*/) {
	$l{$2} = $l;
	$t = $1.$';
    }
    if ($t =~ /$SC(\*?)((sub)*){/) {
	($a,$b,$c) = ($`,$',$2);
	$numbered = $1 ne "*" && $numbered_sections;
	while ($b =~ /^([^}]*){([^{}]*)}/) { $b = $`.$1.$B.$2.$E.$'; }
	$b =~ /^([^{}]*)}\s*/ || die "{ } confusion";
	($b,$d) = ($1,$');
	$b =~ s/$B/{/g;
	$b =~ s/$E/}/g;
#	$l = $b;
	$so = defined($depth) && $indent_sections ? $SO x (3*($depth+1)) : "";
	$depth = length($c)/3;
	$pfx = "";
	if (!$numbered) {
	    $l = '"'.$b.'"';
	}
	else {
	    $snum[$depth]++;
	    for ($i = 0; $i <= $depth; $i++) {
		$pfx .= sprintf("%d%s",$snum[$i],$i == $depth ? $i ? " " :
		  ". " : ".");
	    }
	    ($l = $pfx) =~ s/[. ]*$//;
	}
	for ($i = $depth+1; $i < 10; $i++) { $snum[$i] = 0; }
	$b = $pfx.$b;
	# fix this
	$b = &xlat($b);
	if ($indent_sections) {
	    $u = $SI x (3*($depth+1));
	}
        else {
	    if (($u = ("=","-","- ","")[$depth]) eq "") {
		$u = "";
	    }
	    else {
	        $u = "\\\\".substr($u x length($b),0,length($b));
	    }
        }
        $t = $a.$so.$B2.$b.$u.$B1.$d;
    }
    else {
	last;
    }
}
#
# handle references
#
print STDERR "[".length($t)."] Handling references\n";
$t =~ s/[Pp]age \\pageref({[^{}]*})/\\ref$1/g;
$t =~ s/\\pageref{[^{}]*}/???/g;
while ($t =~ /\\ref{([^{}]*)}/) {
    $t = $`.(defined($l{$1}) ? $l{$1} : "???").$';
}
#
# collapse whitespace
#
print STDERR "[".length($t)."] Collapsing whitespace\n";
$t =~ s/\\par\s*/\n\n/g;
$t =~ s/ *(\n+) */$1/g;
$t =~ tr/\n/ /;
$t =~ tr/ \t/ /s;	# again
#
# handle line breaks
#
print STDERR "[".length($t)."] Handling line breaks\n";
$t =~ tr/\n//d;
$t =~ s/\\\\/\n/g;
$t =~ s/\\par\s*/$B1/g;
#
# handle accents, umlauts, and double quotes
#
print STDERR "[".length($t)."] Handling accents, umlauts, double quotes ".
  "and hyphens\n";
$t =~ s/\\[`']([AEOUaeou])/$1/g;
$t =~ s/\\([`'])~/$1/g;
$t =~ s/\\"([AOUaou])/$1e/g;
$t =~ s/``/"/g;
$t =~ s/''/"/g;
# no, no hyphens ...
#
# apply ultimate set of fixes to newlines
#
print STDERR "[".length($t)."] Applying ultimate set of fixes to newlines\n";
while ($t =~ s/([\n$B1$B2]+)([$SI$SO])/$2$1/g) {};
$t =~ s/([\n$B1$B2]*)\s+([\n$B1$B2]+)/$1$2/g;
$t =~ s/\n+/\n/g;
$t =~ s/\n?($B1)[\n$B1]*/\n\n/g;
$t =~ s/\n*($B2)[\n$B2]*/\n\n\n/g;
#
# translate what's left
#
print STDERR "[".length($t)."] Final translation\n";
$t = &xlat($t);
$t =~ s/^\s*//;
$t =~ s/\s*$//;
$t .= "\n";
#
# okay, now format and print it
#
print STDERR "[".length($t)."] "."Formatting (may take a while)\n";
$l = "";
$m = 0;
while ($t =~ /([$SI$SO$TB$TE$VL\n]| +)/) {
    if ($` ne "" || substr($1,0,1) eq " ") {
	if (length($l)+length($`) > $WIDTH && $l ne "") {
	    &print($l."\n");
	    $l = "";
	}
	if ($l eq "") { $l = " " x $m; }
	$l = $l.$`.(substr($1,0,1) eq " " ? $1 : "");
    }
    $t = $';
    if ($1 eq $SI) { $m++; }
    if ($1 eq $SO) { $m--; }
    if ($1 eq $TB) { $transaction = 1; }
    if ($1 eq $TE) { &flush; }
    if ($1 eq $VL) {
	$t =~ /\n/;
	&print($l."\n") unless $l eq "";
	$l = "";
	&print($`."\n");
	$t = $';
    }
    if ($1 eq "\n") {
	&print($l."\n");
	$l = "";
#	$t = s/^ *(\S.*)/\1/;
    }
}
&print("$l\n") if $l ne "";
if ($paged) {
    $terminating = 1;
    while ($line) { &print("\n"); }
}
print STDERR "Done\n";
