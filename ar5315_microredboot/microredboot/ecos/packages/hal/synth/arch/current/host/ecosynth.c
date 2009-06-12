//============================================================================
//
//     ecosynth.c
//
//     The eCos synthetic target I/O auxiliary
//
//============================================================================
//####COPYRIGHTBEGIN####
//                                                                          
// ----------------------------------------------------------------------------
// Copyright (C) 2002 Bart Veer
//
// This file is part of the eCos host tools.
//
// This program is free software; you can redistribute it and/or modify it 
// under the terms of the GNU General Public License as published by the Free 
// Software Foundation; either version 2 of the License, or (at your option) 
// any later version.
// 
// This program is distributed in the hope that it will be useful, but WITHOUT 
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for 
// more details.
// 
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc., 
// 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// ----------------------------------------------------------------------------
//                                                                          
//####COPYRIGHTEND####
//============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   bartv
// Contact(s):  bartv
// Date:        2002-08-05
// Version:     0.01
// Description:
//
// The main module for the eCos synthetic target auxiliary. This
// program is fork'ed and execve'd during the initialization phase of
// any synthetic target application, and is primarily responsible for
// I/O operations. This program should never be run directly by a
// user.
//
//####DESCRIPTIONEND####
//============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <limits.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/param.h>
#include <sys/types.h>
// Avoid compatibility problems with Tcl 8.4 vs. earlier
#define USE_NON_CONST
#include <tcl.h>
#include <tk.h>

// The protocol between host and target is defined by a private
// target-side header.
#include "../src/synth_protocol.h"

// ----------------------------------------------------------------------------
// Statics etc.
#define PROGNAME    "Synthetic target auxiliary"

static int  no_windows = 0; // -nw arg

static pid_t    parent_pid;

// ----------------------------------------------------------------------------
// Overview.
//
// When a synthetic target application is executed, whether directly
// or from inside gdb, it can fork() and execve() the synthetic
// target auxiliary: this program, ecosynth. For now this is disabled
// by default but can be enabled using a --io option on the command line.
// In future it may be enabled by default, but can be suppressed using
// --nio.
//
// stdin, stdout, and stderr will be inherited from the eCos application, so
// that attempts to use printf() or fprintf(stderr, ) from inside the
// auxiliary or its helper applications work as expected. In addition file
// descriptor 3 will be a pipe from the eCos application, and file descriptor
// 4 will be a pipe to the application. The protocol defined in
// ../src/synth_protocol.h runs over these pip
//
// argv[] and environ are also as per the application, with the exception
// of argv[0] which is the full path to this application.
//
// The bulk of the hard work is done inside Tcl. The C++ code is
// responsible only for initialization and for implementing some
// additional commands, for example to send a SIGIO signal to the
// parent when there is a new pending interrupt. The primary script is
// ecosynth.tcl which should be installed alongside the executable.
//
// This code makes use of the standard Tcl initialization facilities:
//
// 1) main() calls Tcl_Main() with the command-line arguments and an
//    application-specific initialization routine ecosynth_appinit().
//
// 2) Tcl_Main() goes through the initialization sequence in generic/tclMain.c.
//    There is one slight complication: Tcl_main() interprets arguments in
//    a way that makes sense for tclsh, but not for ecosynth. Specially if
//    argv[1] exists and does not begin with a - then it will be interpreted
//    as a script to be executed. Hence argv[] is re-allocated and the name
//    of a suitable script is inserted.
//
// 3) ecosynth_appinit() initializes the Tcl interpreter, and optionally
//    the Tk interpreter as well. This is controlled by the presence of
//    a -nw argument on the command line. This initialization routine
//    also takes care of adding some commands to the Tcl interpreter.
//
// 4) Tcl_Main() will now proceed to execute the startup script, which
//    is eccentric.tcl installed in the libexec directory.

static int  ecosynth_appinit(Tcl_Interp*);

int
main(int argc, char** argv)
{
    char    ecosynth_tcl_path[_POSIX_PATH_MAX];
    char**  new_argv;
    int     i;

    parent_pid = getppid();

    // The various core Tcl scripts are installed in the same
    // directory as ecosynth. The Tcl script itself will check whether
    // there is a newer version of itself in the source tree and
    // switch to that instead.
    assert((strlen(LIBEXECDIR) + strlen(PACKAGE_INSTALL) + 20) < _POSIX_PATH_MAX);
    strcpy(ecosynth_tcl_path, LIBEXECDIR);
    strcat(ecosynth_tcl_path, "/ecos/");
    strcat(ecosynth_tcl_path, PACKAGE_INSTALL);
    strcat(ecosynth_tcl_path, "/ecosynth.tcl");

    // Installation sanity checks.
    if (0 != access(ecosynth_tcl_path, F_OK)) {
        fprintf(stderr, PROGNAME ": error, a required Tcl script has not been installed.\n");
        fprintf(stderr, "    The script is \"%s\"\n", ecosynth_tcl_path);
        exit(EXIT_FAILURE);
    }
    if (0 != access(ecosynth_tcl_path, R_OK)) {
        fprintf(stderr, PROGNAME ": error, no read access to a required Tcl script.\n");
        fprintf(stderr, "    The script is \"%s\"\n", ecosynth_tcl_path);
        exit(EXIT_FAILURE);
    }

    // Look for options -nw and -w. This information is needed by the appinit() routine.
    no_windows = 0;
    for (i = 1; i < argc; i++) {
        if ((0 == strcmp("-nw", argv[i])) || (0 == strcmp("--nw", argv[i])) ||
            (0 == strcmp("-no-windows", argv[i])) || (0 == strcmp("--no-windows", argv[i]))) {
            no_windows = 1;
        } else if ((0 == strcmp("-w", argv[i])) || (0 == strcmp("--w", argv[i])) ||
                   (0 == strcmp("-windows", argv[i])) || (0 == strcmp("--windows", argv[i]))) {
            no_windows = 0;
        }
    }
    
    new_argv = malloc((argc+2) * sizeof(char*));
    if (NULL == new_argv) {
        fprintf(stderr, PROGNAME ": internal error, out of memory.\n");
        exit(EXIT_FAILURE);
    }
    new_argv[0] = argv[0];
    new_argv[1] = ecosynth_tcl_path;
    for (i = 1; i < argc; i++) {
        new_argv[i+1] = argv[i];
    }
    new_argv[i+1] = NULL;

    // Ignore SIGINT requests. Those can happen if e.g. the application is
    // ctrl-C'd or if a gdb session is interrupted because this process is
    // a child of the eCos application. Instead the normal code for handling
    // application termination needs to run.
    signal(SIGINT, SIG_IGN);

    // Similarly ignore SIGTSTP if running in graphical mode, it would
    // be inappropriate for the GUI to freeze if the eCos application is
    // suspended. If running in text mode then it is better for both
    // the application and the I/O auxiliary to freeze, halting any further
    // output.
    if (!no_windows) {
        signal(SIGTSTP, SIG_IGN);
    }

    Tcl_Main(argc+1, new_argv, &ecosynth_appinit);
    return EXIT_SUCCESS;
}


// ----------------------------------------------------------------------------
// Commands for the Tcl interpreter. These are few and far between because
// as much work as possible is done in Tcl.

// Send a SIGIO signal to the parent process. This is done whenever a new
// interrupt is pending. There is a possibility of strange behaviour if
// the synthetic target application is exiting at just the wrong moment
// and this process has become a zombie. An alternative approach would
// involve loading the Extended Tcl extension.
static int
ecosynth_send_SIGIO(ClientData  clientData __attribute__ ((unused)),
                    Tcl_Interp* interp,
                    int         argc,
                    char**      argv __attribute__ ((unused)))
{
    if (1 != argc) {
        Tcl_SetResult(interp, "wrong # args: should be \"usbtest::_send_SIGIO\"" , TCL_STATIC);
        return TCL_ERROR;
    }
    
    (void) kill(parent_pid, SIGIO);
    return TCL_OK;
}

// Similarly send a SIGKILL (-9) to the parent process. This allows the GUI
// code to kill of the eCos application
static int
ecosynth_send_SIGKILL(ClientData clientData __attribute__ ((unused)),
                      Tcl_Interp* interp,
                      int         argc,
                      char**      argv __attribute__ ((unused)))
{
    if (1 != argc) {
        Tcl_SetResult(interp, "wrong # args: should be \"usbtest::_send_SIGIO\"" , TCL_STATIC);
        return TCL_ERROR;
    }
    
    (void) kill(parent_pid, SIGKILL);
    return TCL_OK;
}


// ----------------------------------------------------------------------------
// Application-specific initialization.

static int
ecosynth_appinit(Tcl_Interp* interp)
{
    Tcl_Channel from_app;
    Tcl_Channel to_app;

    // Tcl library initialization. This has the effect of executing init.tcl,
    // thus setting up package load paths etc. Not all of that initialization
    // is necessarily appropriate for ecosynth, but some devices may well want
    // to load in additional packages or whatever.
    if (Tcl_Init(interp) == TCL_ERROR) {
        return TCL_ERROR;
    }
    // Optionally initialize Tk as well. This can be suppressed by an
    // argument -nw. Possibly this could be done by the Tcl script
    // instead using dynamic loading.
    //
    // There is a problem with the way that Tk does its argument processing.
    // By default it will accept abbreviations for the standard wish arguments,
    // so if the user specifies e.g. -v then the Tk code will interpret this
    // as an abbreviation for -visual, and will probably complain because
    // -visual takes an argument whereas ecosynth's -v is just a flag.
    //
    // The Tk argument processing can be suppressed simply by temporarily
    // getting rid of the argv variable. The disadvantage is that some
    // standard arguments now have to be processed explicitly:
    //
    //     -colormap map      ignored, of little interest these days
    //     -display  display  currently ignored. This would have to be
    //                        implemented at the C level, not the Tcl level,
    //                        since Tk_Init() will start interacting with the
    //                        X server. Its implementation would involve a
    //                        setenv() call.
    //     -geometry geom     implemented in the Tcl code using "wm geometry"
    //     -name     name     ignored for now.
    //     -sync              ignored, of little interest to typical users
    //     -use      id       ignored, probably of little interest for now
    //     -visual   visual   ignored, of little interest these days
    //
    // so actually the only extra work that is required is for the Tcl code
    // to process -geometry.
    if (!no_windows) {
        Tcl_Obj* argv = Tcl_GetVar2Ex(interp, "argv", NULL, TCL_GLOBAL_ONLY);
        Tcl_IncrRefCount(argv);
        Tcl_UnsetVar(interp, "argv", TCL_GLOBAL_ONLY);
        if (Tk_Init(interp) == TCL_ERROR) {
            return TCL_ERROR;
        }
        Tcl_SetVar2Ex(interp, "argv", NULL, argv, TCL_GLOBAL_ONLY);
        Tcl_DecrRefCount(argv);
    }
    
    // Create the synth:: namespace. Currently this does not seem to be possible from
    // inside C.
    if (TCL_OK != Tcl_Eval(interp, 
                           "namespace eval synth {\n"
                           "    variable channel_from_app 0\n"
                           "    variable channel_to_app 0\n"
                           "}\n")) {
        fprintf(stderr, PROGNAME ": internal error, failed to create Tcl synth:: namespace\n");
        fprintf(stderr, "    Please check Tcl version (8.3 or later required).\n");
        exit(EXIT_FAILURE);
    }

    // The pipe to/from the application is exposed to Tcl, allowing
    // the main communication to be handled at the Tcl level and
    // specifically via the event loop. This requires two additional
    // Tcl channels to be created for file descriptors 3 and 4.
    from_app = Tcl_MakeFileChannel((ClientData) 3, TCL_READABLE);
    if (NULL == from_app) {
        fprintf(stderr, PROGNAME ": internal error, failed to create Tcl channel for pipe from application.\n");
        exit(EXIT_FAILURE);
    }
    Tcl_RegisterChannel(interp, from_app);

    // The channel name will be something like file0. Add a variable to the
    // interpreter to store this name.
    if (NULL == Tcl_SetVar(interp, "synth::_channel_from_app",  Tcl_GetChannelName(from_app), TCL_GLOBAL_ONLY)) {
        fprintf(stderr, PROGNAME ": internal error, failed to create Tcl variable from application channel 0\n");
        exit(EXIT_FAILURE);
    }

    // Repeat for the channel to the application.
    to_app = Tcl_MakeFileChannel((ClientData) 4, TCL_WRITABLE);
    if (NULL == to_app) {
        fprintf(stderr, PROGNAME ": internal error, failed to create Tcl channel for pipe to application.\n");
        exit(EXIT_FAILURE);
    }
    Tcl_RegisterChannel(interp, to_app);
    if (NULL == Tcl_SetVar(interp, "synth::_channel_to_app",  Tcl_GetChannelName(to_app), TCL_GLOBAL_ONLY)) {
        fprintf(stderr, PROGNAME ": internal error, failed to create Tcl variable from application channel 1\n");
        exit(EXIT_FAILURE);
    }

    // The auxiliary may spawn additional programs, via
    // device-specific scripts. Those programs should not have
    // direct access to the pipe - that would just lead to
    // confusion.
    fcntl(3, F_SETFD, FD_CLOEXEC);
    fcntl(4, F_SETFD, FD_CLOEXEC);
    
    // Add synthetic target-specific commands to the Tcl interpreter.
    Tcl_CreateCommand(interp, "synth::_send_SIGIO", &ecosynth_send_SIGIO, (ClientData)NULL, (Tcl_CmdDeleteProc*) NULL);
    Tcl_CreateCommand(interp, "synth::_send_SIGKILL", &ecosynth_send_SIGKILL, (ClientData)NULL, (Tcl_CmdDeleteProc*) NULL);

    // Define additional variables.

    // The version, from the AM_INIT_AUTOMAKE() macro in configure.in
    Tcl_SetVar(interp, "synth::_ecosynth_version", ECOSYNTH_VERSION, TCL_GLOBAL_ONLY);

    // The parent process id, i.e. that for the eCos application itself.
    Tcl_SetVar2Ex(interp, "synth::_ppid", NULL, Tcl_NewIntObj(getppid()), TCL_GLOBAL_ONLY);
               
    // The various directories
    Tcl_SetVar(interp, "synth::_ecosynth_repository", ECOS_REPOSITORY, TCL_GLOBAL_ONLY);
    Tcl_SetVar(interp, "synth::_ecosynth_libexecdir", LIBEXECDIR, TCL_GLOBAL_ONLY);
    Tcl_SetVar(interp, "synth::_ecosynth_package_dir", PACKAGE_DIR, TCL_GLOBAL_ONLY);
    Tcl_SetVar(interp, "synth::_ecosynth_package_version", PACKAGE_VERSION, TCL_GLOBAL_ONLY);
    Tcl_SetVar(interp, "synth::_ecosynth_package_install", PACKAGE_INSTALL, TCL_GLOBAL_ONLY);
    
    return TCL_OK;
}
