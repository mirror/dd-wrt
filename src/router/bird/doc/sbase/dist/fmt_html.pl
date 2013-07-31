#
#  fmt_html.pl
#
#  $Id$
#
#  HTML-specific driver stuff
#
#  © Copyright 1996, Cees de Groot
#
package LinuxDocTools::fmt_html;
use strict;

use LinuxDocTools::CharEnts;
use LinuxDocTools::Vars;

use LinuxDocTools::FixRef;
my $fixref = $LinuxDocTools::FixRef::fixref;

use LinuxDocTools::Html2Html;
my $html2html = $LinuxDocTools::Html2Html::html2html;

my $html = {};
$html->{NAME} = "html";
$html->{HELP} = "";
$html->{OPTIONS} = [
   { option => "split", type => "l", 
     'values' => [ "0", "1", "2" ], short => "s" },
   { option => "toc", type => "l", 
     'values' => [ "0", "1", "2" ], short => "T" },
   { option => "dosnames", type => "f", short => "h" },
   { option => "imagebuttons", type => "f", short => "I"},
   { option => "header", type => "s", short => "H"},
   { option => "footer", type => "s", short => "F"}
];
$html->{'split'}  = 1;
$html->{'toc'}  = -1;
$html->{dosnames}  = 0;
$html->{imagebuttons}  = 0;
$html->{header}  = "";
$html->{footer}  = "";
$html->{preNSGMLS} = sub {
  $global->{NsgmlsOpts} .= " -ifmthtml ";
  $global->{NsgmlsPrePipe} = "cat $global->{file}";
};

$Formats{$html->{NAME}} = $html;

# HTML escape sub.  this is called-back by `parse_data' below in
# `html_preASP' to properly escape `<' and `&' characters coming from
# the SGML source.
my %html_escapes;
$html_escapes{'&'} = '&amp;';
$html_escapes{'<'} = '&lt;';

my $html_escape = sub {
    my ($data) = @_;

    # replace the char with it's HTML equivalent
    $data =~ s|([&<])|$html_escapes{$1}|ge;

    return ($data);
};

#
#  Translate character entities and escape HTML special chars.
#
$html->{preASP} = sub
{
  my ($infile, $outfile) = @_;
  # note the conversion of `sdata_dirs' list to an anonymous array to
  # make a single argument
  my $char_maps = load_char_maps ('.2html', [ Text::EntityMap::sdata_dirs() ]);

  while (<$infile>)
    {
      if (/^-/)
        {
	    my ($str) = $';
	    chop ($str);
	    print $outfile "-" . parse_data ($str, $char_maps, $html_escape) . "\n";
        }
      elsif (/^A/)
        {
	  /^A(\S+) (IMPLIED|CDATA|NOTATION|ENTITY|TOKEN)( (.*))?$/
	      || die "bad attribute data: $_\n";
	  my ($name,$type,$value) = ($1,$2,$4);
	  if ($type eq "CDATA")
	    {
	      # CDATA attributes get translated also
	      $value = parse_data ($value, $char_maps, $html_escape);
	    }
	  print $outfile "A$name $type $value\n";
        }
      else
        {
	  print $outfile $_;
        }
    }
  return 0;
};

#
#  Take the sgmlsasp output, and make something
#  useful from it.
#
$html->{postASP} = sub
{
  my $infile = shift;
  my $filename = $global->{filename};

  #
  #  Set various stuff as a result of option processing.
  #
  my $ext   = "html";
  $ext   = "htm"  if $html->{dosnames};
  my $img   = 0;
  $img   = 1 if $html->{imagebuttons};

  #
  # Bring in file
  #
  my @file = <$infile>;

  #
  #  Find references
  #
  &{$fixref->{init}}($html->{'split'});
  LINE: foreach (@file) {
      foreach my $pat (keys %{$fixref->{rules}}) {
          if (/$pat/) {
              # Call rule function then skip to next line
              &{$fixref->{rules}->{$pat}}; next LINE;
          }
      }
      &{$fixref->{defaultrule}};
  }
  &{$fixref->{finish}};

  #  
  #  Run through html2html, preserving stdout
  #  Also, handle prehtml.sed's tasks
  #
  my $filter = "";
#  $filter = "|$main::progs->{NKF} -e" if ($global->{language} eq "ja");
  open SAVEOUT, ">&STDOUT";
  open STDOUT, "$filter>$filename.$ext" or die qq(Cannot open "$filename.$ext");

  &{$html2html->{init}}($html->{'split'}, $ext, $img, $filename,
                        $fixref->{filenum}, $fixref->{lrec},
			$html->{'header'}, $html->{'footer'}, $html->{'toc'},
                        $global->{tmpbase}, $global->{debug});
  LINE: foreach (@file) {
      s,<P></P>,,g; 			# remove empty <P></P> containers
      foreach my $pat (keys %{$html2html->{rules}}) {
          if (/$pat/) {
              # Call rule function then skip to next line
              &{$html2html->{rules}->{$pat}}; next LINE;
          }
      }
      &{$html2html->{defaultrule}};
  }
  &{$html2html->{finish}};

  close STDOUT;
  open STDOUT, ">&SAVEOUT";

  return 0;
};

1;

