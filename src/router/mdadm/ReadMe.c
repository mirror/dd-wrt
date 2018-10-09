/*
 * mdadm - manage Linux "md" devices aka RAID arrays.
 *
 * Copyright (C) 2001-2016 Neil Brown <neilb@suse.com>
 * Copyright (C) 2016-2017 Jes Sorensen <Jes.Sorensen@gmail.com>
 *
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *    Author: Neil Brown
 *    Email: <neilb@suse.de>
 *    Maintainer: Jes Sorensen
 *    Email: <Jes.Sorensen@gmail.com>
 */

#include "mdadm.h"

#ifndef VERSION
#define VERSION "4.1"
#endif
#ifndef VERS_DATE
#define VERS_DATE "2018-10-01"
#endif
char Version[] = "mdadm - v" VERSION " - " VERS_DATE "\n";

/*
 * File: ReadMe.c
 *
 * This file contains general comments about the implementation
 * and the various usage messages that can be displayed by mdadm
 *
 */

/*
 * mdadm has 7 major modes of operation:
 * 1/ Create
 *     This mode is used to create a new array with a superblock
 * 2/ Assemble
 *     This mode is used to assemble the parts of a previously created
 *     array into an active array.  Components can be explicitly given
 *     or can be searched for.  mdadm (optionally) checks that the components
 *     do form a bona-fide array, and can, on request, fiddle superblock
 *     version numbers so as to assemble a faulty array.
 * 3/ Build
 *     This is for building legacy arrays without superblocks
 * 4/ Manage
 *     This is for doing something to one or more devices
 *     in an array, such as add,remove,fail.
 *     run/stop/readonly/readwrite are also available
 * 5/ Misc
 *     This is for doing things to individual devices.
 *     They might be parts of an array so
 *       zero-superblock, examine  might be appropriate
 *     They might be md arrays so
 *       run,stop,rw,ro,detail  might be appropriate
 *     Also query will treat it as either
 * 6/ Monitor
 *     This mode never exits but just monitors arrays and reports changes.
 * 7/ Grow
 *     This mode allows for changing of key attributes of a raid array, such
 *     as size, number of devices, and possibly even layout.
 * 8/ Incremental
 *     Is assembles an array incrementally instead of all at once.
 *     As devices are discovered they can be passed to "mdadm --incremental"
 *     which will collect them.  When enough devices to for an array are
 *     found, it is started.
 */

char short_options[]="-ABCDEFGIQhVXYWZ:vqbc:i:l:p:m:n:x:u:c:d:z:U:N:sarfRSow1tye:k:";
char short_bitmap_options[]=
		"-ABCDEFGIQhVXYWZ:vqb:c:i:l:p:m:n:x:u:c:d:z:U:N:sarfRSow1tye:k:";
char short_bitmap_auto_options[]=
		"-ABCDEFGIQhVXYWZ:vqb:c:i:l:p:m:n:x:u:c:d:z:U:N:sa:rfRSow1tye:k:";

struct option long_options[] = {
    {"manage",    0, 0, ManageOpt},
    {"misc",      0, 0, MiscOpt},
    {"assemble",  0, 0, 'A'},
    {"build",     0, 0, 'B'},
    {"create",    0, 0, 'C'},
    {"detail",    0, 0, 'D'},
    {"examine",   0, 0, 'E'},
    {"follow",    0, 0, 'F'},
    {"grow",      0, 0, 'G'},
    {"incremental",0,0, 'I'},
    {"zero-superblock", 0, 0, KillOpt}, /* deliberately not a short_option */
    {"query",	  0, 0, 'Q'},
    {"examine-bitmap", 0, 0, 'X'},
    {"auto-detect", 0, 0, AutoDetect},
    {"detail-platform", 0, 0, DetailPlatform},
    {"kill-subarray", 1, 0, KillSubarray},
    {"update-subarray", 1, 0, UpdateSubarray},
    {"udev-rules", 2, 0, UdevRules},
    {"offroot", 0, 0, OffRootOpt},
    {"examine-badblocks", 0, 0, ExamineBB},

    {"dump", 1, 0, Dump},
    {"restore", 1, 0, Restore},

    /* synonyms */
    {"monitor",   0, 0, 'F'},

    /* after those will normally come the name of the md device */

    {"help",      0, 0, 'h'},
    {"help-options",0,0, HelpOptions},
    {"version",	  0, 0, 'V'},
    {"verbose",   0, 0, 'v'},
    {"quiet",	  0, 0, 'q'},

    /* For create or build: */
    {"chunk",	  1, 0, ChunkSize},
    {"rounding",  1, 0, ChunkSize}, /* for linear, chunk is really a
				     * rounding number */
    {"level",     1, 0, 'l'}, /* 0,1,4,5,6,linear */
    {"parity",    1, 0, Layout}, /* {left,right}-{a,}symmetric */
    {"layout",    1, 0, Layout},
    {"raid-disks",1, 0, 'n'},
    {"raid-devices",1, 0, 'n'},
    {"spare-disks",1,0, 'x'},
    {"spare-devices",1,0, 'x'},
    {"size",	  1, 0, 'z'},
    {"auto",	  1, 0, Auto}, /* also for --assemble */
    {"assume-clean",0,0, AssumeClean },
    {"metadata",  1, 0, 'e'}, /* superblock format */
    {"bitmap",	  1, 0, Bitmap},
    {"bitmap-chunk", 1, 0, BitmapChunk},
    {"write-behind", 2, 0, WriteBehind},
    {"write-mostly",0, 0, WriteMostly},
    {"failfast",  0, 0,  FailFast},
    {"nofailfast",0, 0,  NoFailFast},
    {"re-add",    0, 0,  ReAdd},
    {"homehost",  1, 0,  HomeHost},
    {"symlinks",  1, 0,  Symlinks},
    {"data-offset",1, 0, DataOffset},
    {"nodes",1, 0, Nodes}, /* also for --assemble */
    {"home-cluster",1, 0, ClusterName},
    {"write-journal",1, 0, WriteJournal},
    {"consistency-policy", 1, 0, 'k'},

    /* For assemble */
    {"uuid",      1, 0, 'u'},
    {"super-minor",1,0, SuperMinor},
    {"name",	  1, 0, 'N'},
    {"config",    1, 0, ConfigFile},
    {"scan",      0, 0, 's'},
    {"force",	  0, 0, Force},
    {"update",	  1, 0, 'U'},
    {"freeze-reshape", 0, 0, FreezeReshape},

    /* Management */
    {"add",       0, 0, Add},
    {"add-spare", 0, 0, AddSpare},
    {"add-journal", 0, 0, AddJournal},
    {"remove",    0, 0, Remove},
    {"fail",      0, 0, Fail},
    {"set-faulty",0, 0, Fail},
    {"replace",   0, 0, Replace},
    {"with",      0, 0, With},
    {"run",       0, 0, 'R'},
    {"stop",      0, 0, 'S'},
    {"readonly",  0, 0, 'o'},
    {"readwrite", 0, 0, 'w'},
    {"no-degraded",0,0,  NoDegraded },
    {"wait",	  0, 0,  WaitOpt},
    {"wait-clean", 0, 0, Waitclean },
    {"action",    1, 0, Action },
    {"cluster-confirm", 0, 0, ClusterConfirm},

    /* For Detail/Examine */
    {"brief",	  0, 0, Brief},
    {"export",	  0, 0, 'Y'},
    {"sparc2.2",  0, 0, Sparc22},
    {"test",      0, 0, 't'},
    {"prefer",    1, 0, Prefer},

    /* For Follow/monitor */
    {"mail",      1, 0, EMail},
    {"program",   1, 0, ProgramOpt},
    {"alert",     1, 0, ProgramOpt},
    {"increment", 1, 0, Increment},
    {"delay",     1, 0, 'd'},
    {"daemonise", 0, 0, Fork},
    {"daemonize", 0, 0, Fork},
    {"oneshot",   0, 0, '1'},
    {"pid-file",  1, 0, 'i'},
    {"syslog",    0, 0, 'y'},
    {"no-sharing", 0, 0, NoSharing},

    /* For Grow */
    {"backup-file", 1,0, BackupFile},
    {"invalid-backup",0,0,InvalidBackup},
    {"array-size", 1, 0, 'Z'},
    {"continue", 0, 0, Continue},

    /* For Incremental */
    {"rebuild-map", 0, 0, RebuildMapOpt},
    {"path", 1, 0, IncrementalPath},

    {0, 0, 0, 0}
};

char Usage[] =
"Usage: mdadm --help\n"
"  for help\n"
;

char Help[] =
"mdadm is used for building, managing, and monitoring\n"
"Linux md devices (aka RAID arrays)\n"
"Usage: mdadm --create device options...\n"
"            Create a new array from unused devices.\n"
"       mdadm --assemble device options...\n"
"            Assemble a previously created array.\n"
"       mdadm --build device options...\n"
"            Create or assemble an array without metadata.\n"
"       mdadm --manage device options...\n"
"            make changes to an existing array.\n"
"       mdadm --misc options... devices\n"
"            report on or modify various md related devices.\n"
"       mdadm --grow options device\n"
"            resize/reshape an active array\n"
"       mdadm --incremental device\n"
"            add/remove a device to/from an array as appropriate\n"
"       mdadm --monitor options...\n"
"            Monitor one or more array for significant changes.\n"
"       mdadm device options...\n"
"            Shorthand for --manage.\n"
"Any parameter that does not start with '-' is treated as a device name\n"
"or, for --examine-bitmap, a file name.\n"
"The first such name is often the name of an md device.  Subsequent\n"
"names are often names of component devices.\n"
"\n"
" For detailed help on the above major modes use --help after the mode\n"
" e.g.\n"
"         mdadm --assemble --help\n"
" For general help on options use\n"
"         mdadm --help-options\n"
;

char OptionHelp[] =
"Any parameter that does not start with '-' is treated as a device name\n"
"or, for --examine-bitmap, a file name.\n"
"The first such name is often the name of an md device.  Subsequent\n"
"names are often names of component devices.\n"
"\n"
"Some common options are:\n"
"  --help        -h   : General help message or, after above option,\n"
"                       mode specific help message\n"
"  --help-options     : This help message\n"
"  --version     -V   : Print version information for mdadm\n"
"  --verbose     -v   : Be more verbose about what is happening\n"
"  --quiet       -q   : Don't print un-necessary messages\n"
"  --brief       -b   : Be less verbose, more brief\n"
"  --export      -Y   : With --detail, --detail-platform or --examine use\n"
"                       key=value format for easy import into environment\n"
"  --force       -f   : Override normal checks and be more forceful\n"
"\n"
"  --assemble    -A   : Assemble an array\n"
"  --build       -B   : Build an array without metadata\n"
"  --create      -C   : Create a new array\n"
"  --detail      -D   : Display details of an array\n"
"  --examine     -E   : Examine superblock on an array component\n"
"  --examine-bitmap -X: Display the detail of a bitmap file\n"
"  --examine-badblocks: Display list of known bad blocks on device\n"
"  --monitor     -F   : monitor (follow) some arrays\n"
"  --grow        -G   : resize/ reshape and array\n"
"  --incremental -I   : add/remove a single device to/from an array as appropriate\n"
"  --query       -Q   : Display general information about how a\n"
"                       device relates to the md driver\n"
"  --auto-detect      : Start arrays auto-detected by the kernel\n"
;
/*
"\n"
" For create or build:\n"
"  --bitmap=     -b   : File to store bitmap in - may pre-exist for --build\n"
"  --chunk=      -c   : chunk size of kibibytes\n"
"  --rounding=        : rounding factor for linear array (==chunk size)\n"
"  --level=      -l   : raid level: 0,1,4,5,6,10,linear, or mp for create.\n"
"                     :    0,1,10,mp,faulty or linear for build.\n"
"  --parity=     -p   : raid5/6 parity algorithm: {left,right}-{,a}symmetric\n"
"  --layout=          : same as --parity, for RAID10: [fno]NN \n"
"  --raid-devices= -n : number of active devices in array\n"
"  --spare-devices= -x: number of spare (eXtra) devices in initial array\n"
"  --size=       -z   : Size (in K) of each drive in RAID1/4/5/6/10 - optional\n"
"  --force       -f   : Honour devices as listed on command line.  Don't\n"
"                     : insert a missing drive for RAID5.\n"
"  --assume-clean     : Assume the array is already in-sync. This is dangerous for RAID5.\n"
"  --bitmap-chunk=    : chunksize of bitmap in bitmap file (Kilobytes)\n"
"  --delay=      -d   : seconds between bitmap updates\n"
"  --write-behind=    : number of simultaneous write-behind requests to allow (requires bitmap)\n"
"  --name=       -N   : Textual name for array - max 32 characters\n"
"\n"
" For assemble:\n"
"  --bitmap=     -b   : File to find bitmap information in\n"
"  --uuid=       -u   : uuid of array to assemble. Devices which don't\n"
"                       have this uuid are excluded\n"
"  --super-minor= -m  : minor number to look for in super-block when\n"
"                       choosing devices to use.\n"
"  --name=       -N   : Array name to look for in super-block.\n"
"  --config=     -c   : config file\n"
"  --scan        -s   : scan config file for missing information\n"
"  --force       -f   : Assemble the array even if some superblocks appear out-of-date\n"
"  --update=     -U   : Update superblock: try '-A --update=?' for list of options.\n"
"  --no-degraded      : Do not start any degraded arrays - default unless --scan.\n"
"\n"
" For detail or examine:\n"
"  --brief       -b   : Just print device name and UUID\n"
"\n"
" For follow/monitor:\n"
"  --mail=       -m   : Address to mail alerts of failure to\n"
"  --program=    -p   : Program to run when an event is detected\n"
"  --alert=           : same as --program\n"
"  --delay=      -d   : seconds of delay between polling state. default=60\n"
"\n"
" General management:\n"
"  --add         -a   : add, or hotadd subsequent devices\n"
"  --re-add           : re-add a recently removed device\n"
"  --remove      -r   : remove subsequent devices\n"
"  --fail        -f   : mark subsequent devices as faulty\n"
"  --set-faulty       : same as --fail\n"
"  --replace          : mark a device for replacement\n"
"  --run         -R   : start a partially built array\n"
"  --stop        -S   : deactivate array, releasing all resources\n"
"  --readonly    -o   : mark array as readonly\n"
"  --readwrite   -w   : mark array as readwrite\n"
"  --zero-superblock  : erase the MD superblock from a device.\n"
"  --wait        -W   : wait for recovery/resync/reshape to finish.\n"
;
*/

char Help_create[] =
"Usage:  mdadm --create device --chunk=X --level=Y --raid-devices=Z devices\n"
"\n"
" This usage will initialise a new md array, associate some\n"
" devices with it, and activate the array.   In order to create an\n"
" array with some devices missing, use the special word 'missing' in\n"
" place of the relevant device name.\n"
"\n"
" Before devices are added, they are checked to see if they already contain\n"
" raid superblocks or filesystems.  They are also checked to see if\n"
" the variance in device size exceeds 1%.\n"
" If any discrepancy is found, the user will be prompted for confirmation\n"
" before the array is created.  The presence of a '--run' can override this\n"
" caution.\n"
"\n"
" If the --size option is given then only that many kilobytes of each\n"
" device is used, no matter how big each device is.\n"
" If no --size is given, the apparent size of the smallest drive given\n"
" is used for raid level 1 and greater, and the full device is used for\n"
" other levels.\n"
"\n"
" Options that are valid with --create (-C) are:\n"
"  --bitmap=          -b : Create a bitmap for the array with the given filename\n"
"                        : or an internal bitmap if 'internal' is given\n"
"  --chunk=           -c : chunk size in kibibytes\n"
"  --rounding=           : rounding factor for linear array (==chunk size)\n"
"  --level=           -l : raid level: 0,1,4,5,6,10,linear,multipath and synonyms\n"
"  --parity=          -p : raid5/6 parity algorithm: {left,right}-{,a}symmetric\n"
"  --layout=             : same as --parity, for RAID10: [fno]NN \n"
"  --raid-devices=    -n : number of active devices in array\n"
"  --spare-devices=   -x : number of spare (eXtra) devices in initial array\n"
"  --size=            -z : Size (in K) of each drive in RAID1/4/5/6/10 - optional\n"
"  --data-offset=        : Space to leave between start of device and start\n"
"                        : of array data.\n"
"  --force            -f : Honour devices as listed on command line.  Don't\n"
"                        : insert a missing drive for RAID5.\n"
"  --run              -R : insist of running the array even if not all\n"
"                        : devices are present or some look odd.\n"
"  --readonly         -o : start the array readonly - not supported yet.\n"
"  --name=            -N : Textual name for array - max 32 characters\n"
"  --bitmap-chunk=       : bitmap chunksize in Kilobytes.\n"
"  --delay=           -d : bitmap update delay in seconds.\n"
"  --write-journal=      : Specify journal device for RAID-4/5/6 array\n"
"  --consistency-policy= : Specify the policy that determines how the array\n"
"                     -k : maintains consistency in case of unexpected shutdown.\n"
"\n"
;

char Help_build[] =
"Usage:  mdadm --build device -chunk=X --level=Y --raid-devices=Z devices\n"
"\n"
" This usage is similar to --create.  The difference is that it creates\n"
" a legacy array without a superblock.  With these arrays there is no\n"
" different between initially creating the array and subsequently\n"
" assembling the array, except that hopefully there is useful data\n"
" there in the second case.\n"
"\n"
" The level may only be 0, 1, 10, linear, multipath, or faulty.\n"
" All devices must be listed and the array will be started once complete.\n"
" Options that are valid with --build (-B) are:\n"
"  --bitmap=          : file to store/find bitmap information in.\n"
"  --chunk=      -c   : chunk size of kibibytes\n"
"  --rounding=        : rounding factor for linear array (==chunk size)\n"
"  --level=      -l   : 0, 1, 10, linear, multipath, faulty\n"
"  --raid-devices= -n : number of active devices in array\n"
"  --bitmap-chunk=    : bitmap chunksize in Kilobytes.\n"
"  --delay=      -d   : bitmap update delay in seconds.\n"
;

char Help_assemble[] =
"Usage: mdadm --assemble device options...\n"
"       mdadm --assemble --scan options...\n"
"\n"
"This usage assembles one or more raid arrays from pre-existing\n"
"components.\n"
"For each array, mdadm needs to know the md device, the identity of\n"
"the array, and a number of sub devices. These can be found in a number\n"
"of ways.\n"
"\n"
"The md device is given on the command line, is found listed in the\n"
"config file, or can be deduced from the array identity.\n"
"The array identity is determined either from the --uuid, --name, or\n"
"--super-minor commandline arguments, from the config file,\n"
"or from the first component device on the command line.\n"
"\n"
"The different combinations of these are as follows:\n"
" If the --scan option is not given, then only devices and identities\n"
" listed on the command line are considered.\n"
" The first device will be the array device, and the remainder will be\n"
" examined when looking for components.\n"
" If an explicit identity is given with --uuid or --super-minor, then\n"
" only devices with a superblock which matches that identity is considered,\n"
" otherwise every device listed is considered.\n"
"\n"
" If the --scan option is given, and no devices are listed, then\n"
" every array listed in the config file is considered for assembly.\n"
" The identity of candidate devices are determined from the config file.\n"
" After these arrays are assembled, mdadm will look for other devices\n"
" that could form further arrays and tries to assemble them.  This can\n"
" be disabled using the 'AUTO' option in the config file.\n"
"\n"
" If the --scan option is given as well as one or more devices, then\n"
" Those devices are md devices that are to be assembled.  Their identity\n"
" and components are determined from the config file.\n"
"\n"
" If mdadm can not find all of the components for an array, it will assemble\n"
" it but not activate it unless --run or --scan is given.  To preserve this\n"
" behaviour even with --scan, add --no-degraded.  Note that \"all of the\n"
" components\" means as many as were present the last time the array was running\n"
" as recorded in the superblock.  If the array was already degraded, and\n"
" the missing device is not a new problem, it will still be assembled.  It\n"
" is only newly missing devices that cause the array not to be started.\n"
"\n"
"Options that are valid with --assemble (-A) are:\n"
"  --bitmap=          : bitmap file to use with the array\n"
"  --uuid=       -u   : uuid of array to assemble. Devices which don't\n"
"                       have this uuid are excluded\n"
"  --super-minor= -m  : minor number to look for in super-block when\n"
"                       choosing devices to use.\n"
"  --name=       -N   : Array name to look for in super-block.\n"
"  --config=     -c   : config file\n"
"  --scan        -s   : scan config file for missing information\n"
"  --run         -R   : Try to start the array even if not enough devices\n"
"                       for a full array are present\n"
"  --force       -f   : Assemble the array even if some superblocks appear\n"
"                     : out-of-date.  This involves modifying the superblocks.\n"
"  --update=     -U   : Update superblock: try '-A --update=?' for option list.\n"
"  --no-degraded      : Assemble but do not start degraded arrays.\n"
"  --readonly    -o   : Mark the array as read-only. No resync will start.\n"
;

char Help_manage[] =
"Usage: mdadm arraydevice options component devices...\n"
"\n"
"This usage is for managing the component devices within an array.\n"
"The --manage option is not needed and is assumed if the first argument\n"
"is a device name or a management option.\n"
"The first device listed will be taken to be an md array device, any\n"
"subsequent devices are (potential) components of that array.\n"
"\n"
"Options that are valid with management mode are:\n"
"  --add         -a   : hotadd subsequent devices to the array\n"
"  --re-add           : subsequent devices are re-added if there were\n"
"                     : recent members of the array\n"
"  --remove      -r   : remove subsequent devices, which must not be active\n"
"  --fail        -f   : mark subsequent devices a faulty\n"
"  --set-faulty       : same as --fail\n"
"  --replace          : mark device(s) to be replaced by spares.  Once\n"
"                     : replacement completes, device will be marked faulty\n"
"  --with             : Indicate which spare a previous '--replace' should\n"
"                     : prefer to use\n"
"  --run         -R   : start a partially built array\n"
"  --stop        -S   : deactivate array, releasing all resources\n"
"  --readonly    -o   : mark array as readonly\n"
"  --readwrite   -w   : mark array as readwrite\n"
;

char Help_misc[] =
"Usage: mdadm misc_option  devices...\n"
"\n"
"This usage is for performing some task on one or more devices, which\n"
"may be arrays or components, depending on the task.\n"
"The --misc option is not needed (though it is allowed) and is assumed\n"
"if the first argument in a misc option.\n"
"\n"
"Options that are valid with the miscellaneous mode are:\n"
"  --query       -Q   : Display general information about how a\n"
"                       device relates to the md driver\n"
"  --detail      -D   : Display details of an array\n"
"  --detail-platform  : Display hardware/firmware details\n"
"  --examine     -E   : Examine superblock on an array component\n"
"  --examine-bitmap -X: Display contents of a bitmap file\n"
"  --examine-badblocks: Display list of known bad blocks on device\n"
"  --zero-superblock  : erase the MD superblock from a device.\n"
"  --run         -R   : start a partially built array\n"
"  --stop        -S   : deactivate array, releasing all resources\n"
"  --readonly    -o   : mark array as readonly\n"
"  --readwrite   -w   : mark array as readwrite\n"
"  --test        -t   : exit status 0 if ok, 1 if degrade, 2 if dead, 4 if missing\n"
"  --wait        -W   : wait for resync/rebuild/recovery to finish\n"
"  --action=          : initiate or abort ('idle' or 'frozen') a 'check' or 'repair'.\n"
;

char Help_monitor[] =
"Usage: mdadm --monitor options devices\n"
"\n"
"This usage causes mdadm to monitor a number of md arrays by periodically\n"
"polling their status and acting on any changes.\n"
"If any devices are listed then those devices are monitored, otherwise\n"
"all devices listed in the config file are monitored.\n"
"The address for mailing advisories to, and the program to handle\n"
"each change can be specified in the config file or on the command line.\n"
"There must be at least one destination for advisories, whether\n"
"an email address, a program, or --syslog\n"
"\n"
"Options that are valid with the monitor (-F --follow) mode are:\n"
"  --mail=       -m   : Address to mail alerts of failure to\n"
"  --program=    -p   : Program to run when an event is detected\n"
"  --alert=           : same as --program\n"
"  --syslog      -y   : Report alerts via syslog\n"
"  --increment=  -r   : Report RebuildNN events in the given increment. default=20\n"
"  --delay=      -d   : seconds of delay between polling state. default=60\n"
"  --config=     -c   : specify a different config file\n"
"  --scan        -s   : find mail-address/program in config file\n"
"  --daemonise   -f   : Fork and continue in child, parent exits\n"
"  --pid-file=   -i   : In daemon mode write pid to specified file instead of stdout\n"
"  --oneshot     -1   : Check for degraded arrays, then exit\n"
"  --test        -t   : Generate a TestMessage event against each array at startup\n"
;

char Help_grow[] =
"Usage: mdadm --grow device options\n"
"\n"
"This usage causes mdadm to attempt to reconfigure a running array.\n"
"This is only possibly if the kernel being used supports a particular\n"
"reconfiguration.\n"
"\n"
"Options that are valid with the grow (-G --grow) mode are:\n"
"  --level=           -l : Tell mdadm what level to convert the array to.\n"
"  --layout=          -p : For a FAULTY array, set/change the error mode.\n"
"                        : for other arrays, update the layout\n"
"  --size=            -z : Change the active size of devices in an array.\n"
"                        : This is useful if all devices have been replaced\n"
"                        : with larger devices.   Value is in Kilobytes, or\n"
"                        : the special word 'max' meaning 'as large as possible'.\n"
"  --assume-clean        : When increasing the --size, this flag will avoid\n"
"                        : a resync of the new space\n"
"  --chunk=           -c : Change the chunksize of the array\n"
"  --raid-devices=    -n : Change the number of active devices in an array.\n"
"  --add=             -a : Add listed devices as part of reshape.  This is\n"
"                        : needed for resizing a RAID0 which cannot have\n"
"                        : spares already present.\n"
"  --bitmap=          -b : Add or remove a write-intent bitmap.\n"
"  --backup-file= file   : A file on a different device to store data for a\n"
"                        : short time while increasing raid-devices on a\n"
"                        : RAID4/5/6 array. Also needed throughout a reshape\n"
"                        : when changing parameters other than raid-devices\n"
"  --array-size=      -Z : Change visible size of array. This does not change any\n"
"                        : data on the device, and is not stable across restarts.\n"
"  --data-offset=        : Location on device to move start of data to.\n"
"  --consistency-policy= : Change the consistency policy of an active array.\n"
"                     -k : Currently works only for PPL with RAID5.\n"
;

char Help_incr[] =
"Usage: mdadm --incremental [-Rqrsf] device\n"
"\n"
"This usage allows for incremental assembly of md arrays.  Devices can be\n"
"added one at a time as they are discovered.  Once an array has all expected\n"
"devices, it will be started.\n"
"\n"
"Optionally, the process can be reversed by using the fail option.\n"
"When fail mode is invoked, mdadm will see if the device belongs to an array\n"
"and then both fail (if needed) and remove the device from that array.\n"
"\n"
"Options that are valid with incremental assembly (-I --incremental) are:\n"
"  --run         -R : Run arrays as soon as a minimal number of devices are\n"
"                   : present rather than waiting for all expected.\n"
"  --quiet       -q : Don't print any information messages, just errors.\n"
"  --rebuild-map -r : Rebuild the 'map' file that mdadm uses for tracking\n"
"                   : partial arrays.\n"
"  --scan        -s : Use with -R to start any arrays that have the minimal\n"
"                   : required number of devices, but are not yet started.\n"
"  --fail        -f : First fail (if needed) and then remove device from\n"
"                   : any array that it is a member of.\n"
;

char Help_config[] =
"The /etc/mdadm.conf config file:\n\n"
" The config file contains, apart from blank lines and comment lines that\n"
" start with a hash(#), array lines, device lines, and various\n"
" configuration lines.\n"
" Each line is constructed of a number of space separated words, and can\n"
" be continued on subsequent physical lines by indenting those lines.\n"
"\n"
" A device line starts with the word 'device' and then has a number of words\n"
" which identify devices.  These words should be names of devices in the\n"
" filesystem, and can contain wildcards. There can be multiple words or each\n"
" device line, and multiple device lines.  All devices so listed are checked\n"
" for relevant super blocks when assembling arrays.\n"
"\n"
" An array line start with the word 'array'.  This is followed by the name of\n"
" the array device in the filesystem, e.g. '/dev/md2'.  Subsequent words\n"
" describe the identity of the array, used to recognise devices to include in the\n"
" array.  The identity can be given as a UUID with a word starting 'uuid=', or\n"
" as a minor-number stored in the superblock using 'super-minor=', or as a list\n"
" of devices.  This is given as a comma separated list of names, possibly\n"
" containing wildcards, preceded by 'devices='. If multiple critea are given,\n"
" than a device must match all of them to be considered.\n"
"\n"
" Other configuration lines include:\n"
"  mailaddr, mailfrom, program     used for --monitor mode\n"
"  create, auto                    used when creating device names in /dev\n"
"  homehost, policy, part-policy   used to guide policy in various\n"
"                                  situations\n"
"\n"
;

char *mode_help[mode_count] = {
	[0]		= Help,
	[ASSEMBLE]	= Help_assemble,
	[BUILD]		= Help_build,
	[CREATE]	= Help_create,
	[MANAGE]	= Help_manage,
	[MISC]		= Help_misc,
	[MONITOR]	= Help_monitor,
	[GROW]		= Help_grow,
	[INCREMENTAL]	= Help_incr,
};
