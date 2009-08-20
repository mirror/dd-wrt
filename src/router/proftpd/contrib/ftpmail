#!/usr/bin/env perl
# ---------------------------------------------------------------------------
# Copyright (C) 2008 TJ Saunders <tj@castaglia.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
#
#  $Id: ftpmail,v 1.4 2008/08/18 20:55:17 castaglia Exp $
#
# ---------------------------------------------------------------------------

use strict;

use File::Basename qw(basename);
use Getopt::Long;
use Mail::Sendmail;
use MIME::Base64 qw(encode_base64);
use Time::HiRes qw(usleep);

my $program = basename($0);

my $opts = {};
GetOptions($opts, 'attach-file', 'fifo=s', 'from=s', 'help', 'ignore-users=s',
  'log=s', 'recipient=s@', 'sleep=s', 'smtp-server=s', 'subject=s',
  'watch-users=s');

if ($opts->{help}) {
  usage();
  exit 0;
}

unless ($opts->{fifo}) {
  print STDERR "$program: missing required --fifo parameter\n";
  exit 1;
}
my $fifo = $opts->{fifo};

unless ($opts->{from}) {
  print STDERR "$program: missing required --from parameter\n";
  exit 1;
}
my $from = $opts->{from};

unless ($opts->{recipient}) {
  print STDERR "$program: missing required --recipient parameter\n";
  exit 1;
}
my $recipients = $opts->{recipient};

unless ($opts->{'smtp-server'}) {
  print STDERR "$program: missing required --smtp-server parameter\n";
  exit 1;
}
my $smtp_server = $opts->{'smtp-server'};

my $delay = 0.5;
if ($opts->{sleep}) {
  $delay = $opts->{sleep};
}

my $fifoh;
if (open($fifoh, "< $fifo")) {

  while (1) {
    my $line = <$fifoh>;
    if ($line) {
      chomp($line);

      if ($line =~ /^(\S+\s+\S+\s+\d+\s+\d+:\d+:\d+\s+\d+)\s+(\d+)\s+(.*?)\s+(\d+)\s+(.*?)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(.*?)\s+.*?(\S+)$/o) {
        my $curr_time = $1;
        my $xfer_nsecs = $2;
        my $client = $3;
        my $nbytes = $4;

        # Note that any spaces or control characters will be replaced in this
        # path with underscores.  This can make finding the actual file, as for
        # attachments, rather difficult; we have to test to find the difference
        # between a real underscore in the name, and a substituted underscore.
        my $path = $5;

        unless (-e $path) {
          # Perform a quick-and-dirty check, on the assumption that all of the
          # underscores in the given path are actually spaces.  If a
          # combination of underscores and spaces appears in the real file,
          # we won't detect that here.

          my $alt_path = $path;
          $alt_path =~ s/_/ /g;

          if (-e $alt_path) {
            $path = $alt_path;
          }
        }

        my $xfer_type = $6;
        my $action_flag = $7;
        my $xfer_direction = $8;
        my $access_mode = $9;
        my $user_name = $10;
        my $completion_status = $11;

        my $send_email = 0;

        if ($xfer_direction eq 'i') {
          $send_email = 1;
        }

        if ($send_email) {

          # First, check for any specific --watch-users filter.  If configured,
          # and if the user name does NOT match the --watch-users filter, then
          # don't send email.  Otherwise, check for an --ignore-users filter,
          # and see if the user matches that ignore filter.

          if ($opts->{'watch-users'}) {
            if ($user_name !~ /$opts->{'watch-users'}/) {
              $send_email = 0;
            }

          } elsif ($opts->{'ignore-users'}) {
            if ($user_name =~ /$opts->{'ignore-users'}/) {
              $send_email = 0;
            }
          }
        }

        if ($send_email) {
          send_email({
            timestamp => $curr_time,
            duration => $xfer_nsecs,
            client => $client,
            size => $nbytes,
            file => $path,
            transfer_type => $xfer_type,
            auth_mode => $access_mode,
            user => $user_name,
            status => $completion_status,
          });
        }
      }

      if ($opts->{log}) {
        # Note: since this opens, writes, then closes the log file for every
        # write, it will interact with log rotation scripts MUCH better than
        # proftpd by itself.  Just one of the small benefits.

        my $log_file = $opts->{log};
        my $logfh;

        if (open($logfh, ">> $log_file")) {
          print $logfh "$line\n";

          unless (close($logfh)) {
            print STDERR "$program: error writing to log file '$log_file': $!\n";
          }

        } else {
          print STDERR "$program: error opening log file '$log_file': $!\n";
        }
      }

    } else {
      # No input at this time.  Sleep for half a second (or less) and check
      # again.
      usleep($delay * 1000000);
    }
  }

  close($fifoh);

} else {
  die "$program: unable to read FIFO '$fifo': $!\n";
}

sub send_email {
  my $upload_info = shift;

  my $file = $upload_info->{file};
  my $file_str = basename($file);

  my $subject = "User '$upload_info->{user}' uploaded file '$file_str' via FTP";
  if ($opts->{subject}) {
    $subject = $opts->{subject};
  }

  my $bytes_str = "bytes";
  if ($upload_info->{size} == 1) {
    $bytes_str = "byte";
  }

  my $status = "Completed";
  if ($upload_info->{status} eq 'i') {
    $status = "Incomplete";
  }

  my $secs_str = "secs";
  if ($upload_info->{duration} == 1) {
    $secs_str = "sec";
  }

  my $type_str = "Binary";
  if ($upload_info->{transfer_type} eq 'a') {
    $type_str = "ASCII";
  }

  my $attached = "";
  if ($opts->{'attach-file'} &&
      -e $file) {
    $attached = "(attached)";
  }

  my $text = <<EOT;
File just uploaded via FTP:

  User: $upload_info->{user}
    Client: $upload_info->{client}

  File: $file $attached
    Size: $upload_info->{size} $bytes_str
    At: $upload_info->{timestamp}
    Duration: $upload_info->{duration} $secs_str
    Status: $status
    Transfer type: $type_str

Cheers,
  --$program

EOT

  my $email_info = {
    smtp => $smtp_server,
    From => $from,
    To => join(', ', @$recipients),
    Subject => $subject,
  };

  if ($opts->{'attach-file'}) {
    if (-e $file) {
      $email_info->{'MIME-Version'} = '1.0';

      my $boundary = '====' . time() . '====';
      $email_info->{'Content-Type'} = "multipart/mixed; boundary=\"$boundary\"";
      $boundary = '--' . $boundary;

      $email_info->{Body} .= "$boundary\n";
      $email_info->{Body} .= "Content-Type: text/plain; charset=\"iso-8859-1\"\n";
      $email_info->{Body} .= "Content-Transfer-Encoding: quoted-printable\n\n";
      $email_info->{Body} .= "$text\n";

      if (open(my $fh, "< $file")) {
        binmode($fh);

        # Note: this reads the entire file into memory, and can fail if
        # the file is too big.

        local $/;
        my $attach;
        while (my $data = <$fh>) {
          $attach .= $data;
        }
        close($fh);

        $email_info->{Body} .= "$boundary\n";

        $email_info->{Body} .= "Content-Disposition: attachment; filename=\"$file\"\n";
        if ($upload_info->{transfer_type} eq 'a') {
          $email_info->{Body} .= "Content-Type: text/plain; charset=\"iso-8859-1\"\n\n";
          $email_info->{Body} .= $attach;

        } else {
          $email_info->{Body} .= "Content-Type: application/octet-stream\n";
          $email_info->{Body} .= "Content-Transfer-Encoding: base64\n\n";
          $email_info->{Body} .= encode_base64($attach);
        }

        $email_info->{Body} .= "\n";

      } else {
        my $timestamp = scalar(localtime());
        print STDERR "$program: $timestamp: error reading file '$file' for attaching: $!\n";
      }

    } else {
      # Couldn't find/access the uploaded file on the filesystem.  This usually
      # indicates either a permissions problem, or a munged filename.
      #
      # XXX Need to handle this better.
    }

  } else {
    $email_info->{Body} = $text;
  }

  my $res = Mail::Sendmail::sendmail(%$email_info);
  unless ($res) {
    my $timestamp = scalar(localtime());

    print STDERR "$program: $timestamp: error sending email: $Mail::Sendmail::error\n";
  }
}

sub usage {
  print <<EOH;

usage: $program [--help] [--fifo \$path] [--from \$addr] [--log \$path]
  [--recipient \$addr] [--subject \$string] [--smtp-server \$addr]
  [--attach-file] [--ignore-users \$regex | --watch-users \$regex]

The purpose of this script is to monitor the TransferLog written by proftpd
for uploaded files.  Whenever a file is uploaded by a user, an email will be
sent to the specified recipients.  In the email there will be the timestamp,
the name of the user who uploaded the file, the path to the uploaded file, the
size of the uploaded file, and the time it took to upload.

Command-line options:

  --attach-file		If used, this will cause a copy of the uploaded file
			to be included, as an attachment, in the generated
			email.

  --fifo \$path		Indicates the path to the FIFO to which proftpd is
			writing its TransferLog.  That is, this is the path
			that you used for the TransferLog directive in your
			proftpd.conf.  This parameter is REQUIRED.

  --from \$addr		Specifies the email address to use in the From header.
			This parameter is REQUIRED.

  --help		Displays this message.

  --ignore-users \$regex
			Specifies a Perl regular expression.  If the uploading
			user name matches this regular expression, then NO
			email notification is sent; otherwise, an email is
			sent.

  --log \$path		Since this script reads the TransferLog using FIFOs,
			the actual TransferLog file is not written by default.
			Use this option to write the normal TransferLog file,
			in addition to watching for uploads.

  --recipient \$addr	Specifies an email address to which to send an email
			notification of the upload.  This option can be
			used multiple times to specify multiple recipients.
			AT LEAST ONE recipient is REQUIRED.

  --smtp-server \$addr	Specifies the SMTP server to which to send the email.
                        This parameter is REQUIRED.

  --subject \$string	Specify a custom Subject header for the email sent.
			The default Subject is:

			  User '\$user' uploaded file '\$file' via FTP

  --watch-users \$regex	Specifies a Perl regular expression.  If the uploading
			user name matches this regular expression, then an
			email notification is sent; otherwise, no email is
			sent.

EOH
}
