#
#  fmt_txt.pl
#
#  $Id$
#
#  TXT-specific driver stuff
#
#  © Copyright 1996, Cees de Groot
#
package LinuxDocTools::fmt_txt;
use strict;

use File::Copy;
use Text::EntityMap;
use LinuxDocTools::CharEnts;
use LinuxDocTools::Lang;
use LinuxDocTools::Vars;
use LinuxDocTools::Utils qw(create_temp);

my $txt = {};
$txt->{NAME} = "txt";
$txt->{HELP} = "";
$txt->{OPTIONS} = [
   { option => "manpage", type => "f", short => "m" },
   { option => "filter",  type => "f", short => "f" },
   { option => "blanks",  type => "i", short => "b" }
];
$txt->{manpage} = 0;
$txt->{filter}  = 0;
$txt->{blanks}  = 3;

$Formats{$txt->{NAME}} = $txt;

#
#  Set correct NsgmlsOpts
#
$txt->{preNSGMLS} = sub
{
  if ($txt->{manpage})
    {
      $global->{NsgmlsOpts} .= " -iman ";
      $global->{charset} = "man";
    }
  else
    {
      $global->{NsgmlsOpts} .= " -ifmttxt ";
      $global->{charset} = "latin1" if $global->{charset} eq "latin";
    }


  #
  #  Is there a cleaner solution than this? Can't do it earlier,
  #  would show up in the help messages...
  #
  #  the language support ja.
  #  the charset  support nippon.
  #
  $global->{format} = $global->{charset};
  $global->{charset} = "nippon" if $global->{language} eq "ja";
  $global->{format} = "groff" if $global->{format} eq "ascii";
  $global->{format} = "groff" if $global->{format} eq "nippon";
  $global->{format} = "groff" if $global->{format} eq "euc-kr";
  $ENV{SGML_SEARCH_PATH} =~ s/txt/$global->{format}/;

  $Formats{"groff"} = $txt;
  $Formats{"latin1"} = $txt;
  $Formats{"man"} = $txt;

  $global->{NsgmlsPrePipe} = "cat $global->{file} " ;
};


# Ascii escape sub.  this is called-back by `parse_data' below in
# `txt_preASP' to properly escape `\' characters coming from the SGML
# source.
my $txt_escape = sub {
    my ($data) = @_;

    $data =~ s|"|\\\&\"|g;	# Insert zero-width space in front of "
    $data =~ s|^\.|\\&.|;	# ditto in front of . at start of line
    $data =~ s|\\|\\\\|g;	# Escape backslashes

    return ($data);
};

#
#  Run the file through the genertoc utility before sgmlsasp. Not necessary
#  when producing a manpage. A lot of code from FJM, untested by me.
#
$txt->{preASP} = sub
{
  my ($infile, $outfile) = @_;
  my (@toc, @lines);
  my $char_maps = load_char_maps ('.2tr', [ Text::EntityMap::sdata_dirs() ]);
  if ( $global->{charset} eq "latin1" )
   {
    $char_maps = load_char_maps ('.2l1tr', [ Text::EntityMap::sdata_dirs() ]);
   }

   if ($txt->{manpage})
    {
     while (<$infile>)
      {
       if (/^-/)
        {
         my ($str) = $';
         chop ($str);
         print $outfile "-" . 
                 parse_data ($str, $char_maps, $txt_escape) . "\n";
         next;
        }
       elsif (/^A/)
        {
         /^A(\S+) (IMPLIED|CDATA|NOTATION|ENTITY|TOKEN)( (.*))?$/
             || die "bad attribute data: $_\n";
         my ($name,$type,$value) = ($1,$2,$4);
         if ($type eq "CDATA")
          {
            # CDATA attributes get translated also
            $value = parse_data ($value, $char_maps, $txt_escape);
          }
         print $outfile "A$name $type $value\n";
         next;
        }
        #
        #  Default action if not skipped over with next: copy in to out.
        #
        print $outfile $_;
      }

      return;
    }

  # note the conversion of `sdata_dirs' list to an anonymous array to
  # make a single argument

  #
  #  Build TOC. The file is read into @lines in the meantime, we need to
  #  traverse it twice.
  #
  push (@toc, "(HLINE\n");
  push (@toc, ")HLINE\n");
  push (@toc, "(P\n");
  push (@toc, "-" . Xlat ("Table of Contents") . "\n");
  push (@toc, ")P\n");
  push (@toc, "(VERB\n");
  my (@prevheader, @header);
  my $appendix = 0;
  my $nonprint = 0;
  while (<$infile>)
    {
      push (@lines, $_);

      if (/^\(SECT(.*)/) 
        {
	  @prevheader = @header;
	  @header = @header[0..$1];
	  if ($appendix == 1) 
            {
	      $header[$1] = "A";
	      $appendix = 0;
            } else 
            {
	      $header[$1]++;
	    }
        }
      if (/^\(APPEND(.*)/) 
        {
	  $appendix = 1;
        }
      if (/^\(HEADING/) 
        {
	  $_ = <$infile>;
	  s/\\n/ /g;
	  push (@lines, $_);
	  chop;
	  s/^-//;
	  $_ = join(".",@header) . " " . $_;
	  s/\(\\[0-9][0-9][0-9]\)/\\\1/g;

	  if (!$#header) 
	    {
	      # put a newline before top-level sections unless previous was also
	      # a top level section
	      $_ = "\\n" . $_ unless (!$#prevheader);
	      # put a . and a space after top level sections
	      s/ /. /;
#####	      $_ = "-" . $_ . "\\n";
	      $_ = "-" . $_;
	    } 
	  else 
	    {
	      # subsections get indentation matching hierarchy
	      $_ = "-" . "   " x $#header . $_;
	    }

#	remove tags from a toc
	  s/\)TT//g;
	  s/\(TT//g;
	  s/\)IT//g;
	  s/\(IT//g;
	  s/\)EM//g;
	  s/\(EM//g;
	  s/\)BF//g;
	  s/\(BF//g;
	  s/AID * CDATA.*$//g;
	  s/\)LABEL//g;
	  s/\(LABEL//g;

	  push(@toc, parse_data ($_, $char_maps, $txt_escape));

	  $_ = <$infile>;
	  while (!/^\)HEADING/) {
	    s/\\n/ /g; ####
	    push(@lines, $_);
	    chop;
	    s/^-//;

#	remove tags from a toc
	    s/\)TT//g;
	    s/\(TT//g;
	    s/\)IT//g;
	    s/\(IT//g;
	    s/\)EM//g;
	    s/\(EM//g;
	    s/\)BF//g;
	    s/\(BF//g;
	    s/AID * CDATA.*$//g;
	    s/\)LABEL//g;
	    s/\(LABEL//g;

#	remove NIDX, NCDX from a toc entry
	    if (/^\(NIDX$/ || /^\(NCDX$/) { $nonprint = 1; }
	    if (/^\)NIDX$/ || /^\)NCDX$/) { $nonprint = 1; }

#	  $_ = "-" . $_ . "\\n";
	    push(@toc, parse_data ($_, $char_maps, $txt_escape))
	      if (! $nonprint);
	    $_ = <$infile>;
	  }
	  s/\\n/ /g; ###
	  push(@lines, $_);
	  push(@toc, "\\n\n");
      }
    }
  push (@toc, ")VERB\n");
  push (@toc, "(HLINE\n");
  push (@toc, ")HLINE\n");

  my $inheading = 0;
  my $tipo = '';
  for (@lines)
    {
      if ($inheading)
        {
	  next if (/^\)TT/ || /^\(TT/ || /^\)IT/ || /^\(IT/ ||
                   /^\)EM/ || /^\(EM/ || /^\)BF/ || /^\(BF/);
	  if (/^-/) 
            {
	      $tipo .=  $' ;
	      chop ($tipo);
	      $tipo .= " " unless $tipo =~ / $/;
	    }
	  else 
	    {
	      $tipo =~ s/ $//;
	      if ($tipo)
		{
		  print $outfile "-"
		      . parse_data ($tipo, $char_maps, $txt_escape)
		      . "\n";
		}
	      print $outfile $_;
	      $tipo = '';
	    }
	  if (/^\)HEADING/)
	    {
	      $inheading = 0;
            }
	  next;
	}
      if (/^\(HEADING/) 
        {
	  #
	  #  Go into heading processing mode.
	  #
	  $tipo = '';
	  $inheading = 1;
	}
      if (/^\(TOC/)
        {
	  print $outfile @toc;
	  next;
	}
      if (/^-/)
        {
	  my ($str) = $';
	  chop ($str);
	  print $outfile "-" . parse_data ($str, $char_maps, $txt_escape) . "\n";
	  next;
        }
      elsif (/^A/)
        {
	  /^A(\S+) (IMPLIED|CDATA|NOTATION|ENTITY|TOKEN)( (.*))?$/
	      || die "bad attribute data: $_\n";
	  my ($name,$type,$value) = ($1,$2,$4);
	  if ($type eq "CDATA")
	    {
	      # CDATA attributes get translated also
	      $value = parse_data ($value, $char_maps, $txt_escape);
	    }
	  print $outfile "A$name $type $value\n";
	  next;
        }

      #
      #  Default action if not skipped over with next: copy in to out.
      #
      print $outfile $_;
    }
};


#
#  Take the sgmlsasp output, and make something
#  useful from it.
#
$txt->{postASP} = sub
{
  my $infile = shift;
  my ($outfile, $groffout);

  if ($txt->{manpage})
    {
      $outfile = new FileHandle ">$global->{filename}.man";
    }
  else
    {
      create_temp("$global->{tmpbase}.txt.1");
      $outfile = new FileHandle 
	  "|$main::progs->{GROFF} $global->{pass} -T $global->{charset} -t $main::progs->{GROFFMACRO} >\"$global->{tmpbase}.txt.1\"";
    }

  #
  #  Feed $outfile with roff input.
  #
  while (<$infile>)
    {
      unless (/^\.DS/.../^\.DE/) 
        {
	  s/^[ \t]{1,}(.*)/$1/g;
        }
      s/^\.[ \t].*/\\\&$&/g;
      s/\\fC/\\fR/g;
      s/^.ft C/.ft R/g; 
      print $outfile $_;
    }  
  $outfile->close;

  #
  #  If we were making a manpage, we're done. Otherwise, a little bit
  #  of work is left.
  #
  if ($txt->{manpage})
    {
      return 0;
    }
  else
    {
      $outfile->open (">$global->{filename}.txt");
      $groffout = new FileHandle "<$global->{tmpbase}.txt.1";
      my $count = 0;
      if ($txt->{filter})
        {
	  while (<$groffout>)
	    {
	      s/[^\cH][^\cH]\cH\cH//g;
	      s/.//g;
              if ($txt->{blanks})
                {
                  $count = &{$txt->{cutblank}}($count, $outfile, $_);
                }
              else
                {
                  print $outfile $_;
                }
	    }
	}
      else
        {
          if ($txt->{blanks})
            {
              while (<$groffout>)
                {
                  $count = &{$txt->{cutblank}}($count, $outfile, $_);
                }
            }
          else
            {
	      copy ($groffout, $outfile);
            }
	}
    }
  $groffout->close;
  $outfile->close;

  return 0;
};

$txt->{cutblank} = sub
{
  my ($num, $out, $in) = @_;
  if ( $in =~ /^$/ )
    {
      $num++;
    }
  else
    {
      $num = 0;
    }
  if ( $num <= $txt->{blanks} )
    {
      print $out $in;
    }

  return ($num);
};

1;
