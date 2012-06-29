#!/usr/bin/perl

# a hack to display OLSR topology information
#
# copyleft 2004 Bruno Randolf <br1@subnet.at>
# licensed under the GPL

use IO::Socket;
use Getopt::Long;

$TOPPATH = "/tmp";
$NAME = "topology";
$FILENAME = "$TOPPATH/$NAME.dot";
$EXT = "png";
$SERVER = "localhost";
$PORT = "2004";
$FULLSCREEN = 0;
$HELP = 0;
$KEEP = 0;
$BGCOLOR = "black";
$STYLE = 1;
$SIZE = "8,8";
$ROOTWIN = 0;
$ONCE = 0;
$GRAPH = 1;
$SHOW = 1;
$FONTNAME = "Courier";
$FONTSIZE = 12;
$LINEWIDTH = 0;
$LINECOLOR = 0;
$RESOLV = 0;

GetOptions ("server=s" => \$SERVER,
	    "port=s" => \$PORT,
	    "fullscreen!" => \$FULLSCREEN,
	    "help!" => \$HELP,
	    "keep!" => \$KEEP,
	    "bgcolor=s" => \$BGCOLOR,
	    "fontname=s" => \$FONTNAME,
	    "fontsize=s" => \$FONTSIZE,
	    "style=i" => \$STYLE,
	    "size=s" => \$SIZE,
	    "rootwin!" => \$ROOTWIN,
	    "once!" => \$ONCE,
	    "graph!" => \$GRAPH,
	    "show!" => \$SHOW,
	    "linewidth!" => \$LINEWIDTH,
	    "linecolor!" => \$LINECOLOR,
	    "resolv" => \$RESOLV,
	     );

if ($HELP) {
	print << "EOF";
usage: $0 [ --server SERVER ] [--port PORT] [--fullscreen] [--keep]

a hack to display OLSR topology information

options:
        --server SERVER    connect to OLSR node (default: localhost)
        --port PORT        connect to port (default: 2004)
        --bgcolor          background color (default: black)
        --fontname         font name (default: Courier)
        --fontsize         font size (default: 12)
        --style            drawing style 1, 2 or 3 (default:1)
        --size X,Y         image size in inches for graphviz (default: 8,8)
        --[no]fullscreen   display fullscreen (default: off)
        --[no]rootwin      display in root window (default: off)
        --[no]graph        genereate graphics (default: on)
        --[no]show         display the graphics (default: on)
        --[no]once         run only 1 time, then exit (default: run forever)
        --[no]linewith     change line width according to metric (default: off)
        --[no]linecolor    change line color according to metric (default: off)
        --[no]resolv       resolv hostnames (default: off)
        --[no]keep         keep the .dot files with timestamp in /tmp (default: off)

requires the "graphviz" and "imagemagick" packages installed
and the "olsrd_dot_draw" plugin configured on the olsr node

EOF
	exit;
}


`touch $TOPPATH/$NAME.$EXT`;

$remote = IO::Socket::INET->new(
                        Proto    => "tcp",
                        PeerAddr => $SERVER,
                        PeerPort => $PORT,
                    )
                  or die "cannot connect to port $PORT at $SERVER!\nis the olsrd_dot_draw plugin loaded and configured to allow connections from this host?\n";

$f;
$start = 1;

$FULLOPT = "-backdrop -background black" if $FULLSCREEN;

if ($STYLE == 1) {
	$DOT_CMD = "neato -Tpng -Gsize=${SIZE} -Gbgcolor=${BGCOLOR} -Gsplines=true -Nstyle=filled -Nfontname=${FONTNAME} -Nfontsize=${FONTSIZE} -Efontname=${FONTNAME} -Efontsize=${FONTSIZE} -Ncolor=white -Nfillcolor=white -Ecolor=grey -Elen=4 -Earrowsize=2 -Efontcolor=white $FILENAME -o $TOPPATH/$NAME.new";
}
elsif ($STYLE == 2) {
	$BGCOLOR = "grey";
	$DOT_CMD = "dot -Tpng -Gsize=${SIZE} -Elen=2 -Ncolor=grey -Nstyle=filled -Nfillcolor=white -Nfontname=${FONTNAME} -Nfontsize=${FONTSIZE} -Efontname=${FONTNAME} -Efontsize=${FONTSIZE} -Nfontcolor=red -Nwidth=1 -Gbgcolor=$BGCOLOR $FILENAME -o $TOPPATH/$NAME.new";
}
elsif ($STYLE == 3) {
	$BGCOLOR = "\#ff6600";
	$DOT_CMD = "neato -Tpng -Gsize=10,9 -Gbgcolor=${BGCOLOR} -Gsplines=true -Nstyle=filled -Nfontname=${FONTNAME} -Nfontsize=${FONTSIZE} -Efontname=${FONTNAME} -Efontsize=${FONTSIZE} -Nheight=1.3 -Nwidth=1.3 -Gsplines=true -Ncolor=darkslategrey -Nfillcolor=darkslategrey -Ecolor=black -Elen=4 -Earrowsize=3 $FILENAME -o $TOPPATH/$NAME.new";
}


while ( <$remote> ) 
{
	$line = $_;
	if ($RESOLV) {
		$line = resolv( $line );
	}
	if ($LINEWIDTH || $LINECOLOR) {
		$line = draw_thicker_metric_lines( $line );
	}
	$f = $f . $line;

	# end of one graph
	if ( $line =~ /}/i ) { 
		#print "* ";
		open DOTFILE, ">$FILENAME";
		print DOTFILE $f;
		close DOTFILE;
		$f = "";

		if ($GRAPH) {
			`$DOT_CMD`;
			`mv $TOPPATH/$NAME.new $TOPPATH/$NAME.$EXT`;
		}
		if ($KEEP) {
			`cp $TOPPATH/$NAME.dot $TOPPATH/$NAME-\$(date +'%Y-%m-%d-%H-%M-%S').dot`;
		}
		if ($SHOW) {
			if ($ROOTWIN) {
				system "display -window root -backdrop $TOPPATH/$NAME.$EXT &";
			}
			elsif ($start) {
				system "display $FULLOPT -update 5 $TOPPATH/$NAME.$EXT &";
				$start = 0;
			}
		}
		exit if $ONCE;
	}
}

print "connection closed\n";


sub resolv {
	my $l = shift;
	if ( $l =~ /\"(.*)\" -> "([^[]*)"(.*)/) {
		my $from = $1;
		my $to = $2;
		my $rest = $3;
		$from =~ s/\"//g;
		$to =~ s/\"//g;
		my $iaddrf = inet_aton($from);
		my $fromn  = gethostbyaddr($iaddrf, AF_INET);
		my $iaddrt = inet_aton($to);
		my $ton  = gethostbyaddr($iaddrt, AF_INET);
		$fromn = $from if ! $fromn;
		$ton = $to if ! $ton;
		$l = "\"$fromn\" -> \"$ton\"$rest\n";
	}
	return $l;
}


sub draw_thicker_metric_lines { 
	# sla 04.04.2005
	# a hack to draws thicker lines for better metrics (the better the thicker).
	# colorize the metric lines.
	#
	my $l = shift;
	if ($l =~ /.*label="[0-9].*/){     # recognizion pattern
		@n=split (/"/,$l);         # split the string to get the metric value
		if ($LINECOLOR) {
			if ( $n[5]>2 ) {        # colorize metrics greater than 2 gray            
				$c="888888";
			}
			else {                     # colorize metrics less than 2 black
				$c="000000";            
			}
			$setcol = "color=\"#$c\",";
		}
		if ($LINEWIDTH) {
			if ($n[5]>0 && $n[5]<10) {  # thicker lines if 10 > metric > 0
				$lt=6/$n[5];
				$at=$lt/2;
			}
			else {                     # at least draw a thin line
				$lt=1;
				$at=$lt;
			}
			$setwidth = "style=\"setlinewidth($lt)\", arrowsize=$at";
		}
		$l =~ s/\]/, $setcol $setwidth]/g; # replace pattern
	}
	return $l;
}

__END__
