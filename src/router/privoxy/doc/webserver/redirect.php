<?php

error_reporting(E_ALL);

// redirect.php
//
// Copyright (C) 2001 The SourceForge ijbswa team.
// May be used under the GNU GPL, version 2 or later.


// Parse the v= and to= paramaters
function parse_parameters()
{
   global $v, $to;
   global $version_major, $version_minor, $version_point;

   $version_major = 0;
   $version_minor = 0;
   $version_point = 0;

   if (isset($v))
   {
      // Version specified

      $v = trim($v);

      // Check if it's valid.
      // Valid versions have the form "n.n.n", where n=digit(s).
      if ( (strspn($v,"0123456789.") == strlen($v)) )
      {
         // Probably valid.  Copy into globals.
         $version_pieces = explode (".", $v, 4);
         if (isset($version_pieces[0]))
         {
            $version_major = 0 + $version_pieces[0];
         }
         if (isset($version_pieces[1]))
         {
            $version_minor = 0 + $version_pieces[1];
         }
         if (isset($version_pieces[2]))
         {
            $version_point = 0 + $version_pieces[2];
         }
      }
   }

   if (isset($to))
   {
      // Trim whitespace and convert to lowercase.
      $to = strtolower(trim($to));

      // Restrict the characters in the string by removing everything
      // from the first disallowed character onwards.
      //
      // Allowed characters are 0-9, a-z, ".", "_", "-".
      //
      $to = substr($to, 0, strspn($to, "0123456789abcdefghijklmnopqrstuvwxyz._-"));
   }
   else
   {
      $to = "";
   }
}

parse_parameters();

// For debugging:
// print "Version {$version_major}.{$version_minor}.{$version_point}<br>";
// print "Target \"{$to}\"<br>";


// Please do NOT delete any of these redirects.  Even if you take them
// out of Privoxy, they may be in use by older releases.

// Note 2: Should *not* include #target part in these URLs.
// (It works with MS IE, but is not valid HTTP.)
//http://ijbswa.sourceforge.net/user-manual/configuration.html
switch($to)
{
   case "faq":
      // Used by 2.9.0+
//      header ("Location: http://www.junkbusters.com/ht/en/ijb2faq.html");
        header ("Location: http://www.privoxy.org/faq/");
    exit;
   case "option":
      // Used by 2.9.0+
      // Config file options
      // called as redirect.php?v=X.X.X&to=option#optionname
//      header ("Location: http://www.junkbusters.com/ht/en/ijb2man.html");
      header ("Location: http://www.privoxy.org/user-manual/configuration.html");
      exit;
   case "win":
      // Used by 2.9.0+ on WIN32
//      header ("Location: http://www.junkbusters.com/ht/en/ijbwin.html");
      header ("Location: http://www.privoxy.org/user-manual/configuration.html");
      exit;
//   case "home":
//      // Currently hard-wired into the code.
//      header ("Location: http://ijbswa.sourceforge.net/");
//      exit;
//   case "gpl":
//      // Currently hard-wired into the code.
//      header ("Location: http://www.fsf.org/copyleft/gpl.html");
//      exit;
   default:
      header ("Location: http://www.privoxy.org/");
      exit;
}

exit;
?>
