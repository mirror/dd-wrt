#!/usr/bin/perl
# A little script for finding the closest patch version
# to the given kernel verison
# David Bryson <mutex@mac.com>

$kdir = $ARGV[0];
$looptype = $ARGV[1];
$reverse = $ARGV[2];

unless( open( VERSION, "../version" ) ) {
  die "No version file present";
}

$version = <VERSION>;
close( VERSION );
chomp( $version );
@kversion = split /[\.-]/, $version;

$kmajor = $kversion[0];
$kminor = $kversion[1];
$kpatch = $kversion[2];

$version = join( ".", $kmajor, $kminor, $kpatch );

opendir( LINUX, "linux-$kmajor.$kminor" ) ||
  die "cannot cd to linux-$kmajor.$kminor dying\n";

@dirlisting = readdir( LINUX );

foreach $file (@dirlisting) {
  $patch = $file;

    @patch_version = split /[\.-]/, $patch;

  if( $patch_version[1] == $looptype ) {

    $pmajor = $patch_version[2];
    $pminor = $patch_version[3];
    $ppatch = $patch_version[4];

    if( $ppatch == $kpatch ) {
      break;
    }
    if( $ppatch < $kpatch ) {
      for(; $kpatch != $ppatch; $kpatch--) {}
    }
    else {
      for(; $kpatch != $ppatch; $kpatch++) {}
    }
  }
}

$final = join( ".", $kmajor, $kminor, $kpatch );

symlink("$kdir", "linux-$final");
system "patch $reverse -p0 <linux-$kmajor.$kminor/loop-$looptype-$final";
unlink("linux-$final");
