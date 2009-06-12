//####COPYRIGHTBEGIN####
//                                                                          
// ----------------------------------------------------------------------------
// Copyright (C) 2003 Bart Veer
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// This program is part of the eCos host tools.
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
//==========================================================================
//
//      ecosconfig.cxx
//
//      The implementation of ecosconfig command line processing
//
//==========================================================================
//==========================================================================
//#####DESCRIPTIONBEGIN####                                             
//
// Author(s):           jld
// Date:                1999-11-08
//
//####DESCRIPTIONEND####
//==========================================================================

#ifndef _MSC_VER
#include <sys/param.h>
#include <unistd.h> /* for realpath() */
#endif
#ifdef __CYGWIN__
#include <windows.h>
#include <sys/cygwin.h> /* for cygwin_conv_to_win32_path() */
#endif
#include "cdl_exec.hxx"
#include "ecosconfig.hxx"

#define TOOL_VERSION "2.0"
#define TOOL_COPYRIGHT "Copyright (c) 2002 Red Hat, Inc."
#define DEFAULT_SAVE_FILE "ecos.ecc"
static char* tool = "ecosconfig";

// When running under cygwin there may be confusion between cygwin and
// Windows paths. Some paths will be passed on to the Tcl library,
// which sometimes will accept a cygwin path and sometimes not. This
// does not affect the VC++ build which only accepts Windows paths,
// and obviously it does not affect any Unix platfom.
#ifdef __CYGWIN__
static std::string
translate_path(std::string& path)
{
    std::string result;
    char buffer [MAXPATHLEN + 1];
    if ("" == path) {
        result = path;
    } else {
        cygwin_conv_to_win32_path (path.c_str (), buffer);
        result = std::string(buffer);
    }
    return result;
}
# define TRANSLATE_PATH(a) translate_path(a)
#else
# define TRANSLATE_PATH(a) (a)
#endif

int main (int argc, char * argv []) {

    // process command qualifiers
    std::string repository;     // --srcdir=
    std::string savefile;       // --config=
    std::string install_prefix; // --prefix=
    bool version = false;       // --version
    bool no_resolve = false;    // --no-resolve
    bool quiet = false;         // -q, --quiet
    bool verbose = false;       // -v, --verbose
    bool ignore_errors = false; // -i, --ignore-errors
    bool no_updates = false;    // -n, --no-updates,
    bool help = false;          // --help
    bool enable_debug_set = false;  // --enable-debug or --disable-debug
    int  debug_level = 0;       // --enable-debug=[0|1|2]

    Tcl_FindExecutable(argv[0]);
    
    // getopt() cannot easily be used here since this code has to
    // build with VC++ as well.
    bool args_ok = true;
    int command_index;
    for (command_index = 1; command_index < argc; command_index++) { // for each command line argument
        char* arg = argv[command_index];
        if (0 == strcmp(arg, "--help")) {
            help = true;
        } else if ((0 == strcmp(arg, "-q")) || (0 == strcmp(arg, "--quiet"))) {
            // Allow repeated use of -q and -v to override each other.
            // This is useful in conjunction with shell aliases.
            quiet = true;
            verbose = false;
        } else if ((0 == strcmp(arg, "-v")) || (0 == strcmp(arg, "--verbose"))) {
            verbose = true;
            quiet = false;
        } else if ((0 == strcmp(arg, "-i")) || (0 == strcmp(arg, "--ignore-errors"))) {
            // Duplicate use of -i and the other flags is harmless.
            ignore_errors = true;
        } else if ((0 == strcmp(arg, "-n")) || (0 == strcmp(arg, "--no-updates"))) {
            no_updates = true;
        } else if (0 == strcmp(arg, "--version")) {
            version = true;
        } else if (0 == strcmp(arg, "--no-resolve")) {
            no_resolve = true;
        } else if (0 == strcmp(arg, "--enable-debug")) {
            enable_debug_set = true;
            debug_level = 1;
        } else if (0 == strcmp(arg, "--disable-debug")) {
            enable_debug_set = true;
            debug_level = 0;
        } else if (0 == strncmp(arg, "--srcdir", 8)) {
            // Duplicate use of --srcdir and other data-containing options should
            // be marked as an error.
            if ("" != repository) {
                fprintf(stderr, "%s: the `--srcdir' option should be used only once.\n", tool);
                args_ok = false;
            } else {
                if ('=' == arg[8]) {
                    repository = std::string(arg + 9);
                    if ("" == repository) {
                        fprintf(stderr, "%s: missing component repository after `--srcdir='\n", tool);
                        args_ok = false;
                    }
                } else if ('\0' == arg[8]) {
                    command_index++;
                    if (command_index == argc) {
                        fprintf(stderr, "%s: missing component repository after `--srcdir'\n", tool);
                        args_ok = false;
                    } else {
                        repository = argv[command_index];
                    }
                } else {
                    fprintf(stderr, "%s: invalid option `%s'\n", tool, arg);
                    args_ok = false;
                }
            }
        } else if (0 == strncmp(arg, "--config", 8)) {
            if ("" != savefile) {
                fprintf(stderr, "%s: the `--config' option should be used only once.\n", tool);
                args_ok = false;
            } else {
                if ('=' == arg[8]) {
                    savefile = std::string(arg + 9);
                    if ("" == savefile) {
                        fprintf(stderr, "%s: missing configuration savefile after `--config='\n", tool);
                        args_ok = false;
                    }
                } else if ('\0' == arg[8]) {
                    command_index++;
                    if (command_index == argc) {
                        fprintf(stderr, "%s: missing configuration savefile after `--config'\n", tool);
                        args_ok = false;
                    } else {
                        savefile = argv[command_index];
                    }
                } else {
                    fprintf(stderr, "%s: invalid option `%s'\n", tool, arg);
                    args_ok = false;
                }
            }
        } else if (0 == strncmp(arg, "--prefix", 8)) {
            if ("" != install_prefix) {
                fprintf(stderr, "%s: the `--prefix' option should be used only once.\n", tool);
                args_ok = false;
            } else {
                if ('=' == arg[8]) {
                    install_prefix = std::string(arg + 9);
                    if ("" == install_prefix) {
                        fprintf(stderr, "%s: missing install prefix after `--prefix='\n", tool);
                        args_ok = false;
                    }
                } else if ('\0' == arg[8]) {
                    command_index++;
                    if (command_index == argc) {
                        fprintf(stderr, "%s: missing install prefix after `--prefix'\n", tool);
                        args_ok = false;
                    } else {
                        install_prefix = argv[command_index];
                    }
                } else {
                    fprintf(stderr, "%s: invalid option `%s'\n", tool, arg);
                    args_ok = false;
                }
            }
        } else {
            // The argument is not a qualifier
            // However, none of the sub-commands begin with a -
            if ('-' == arg[0]) {
                fprintf(stderr, "%s: unknown option `%s'\n", tool, arg);
                args_ok = false;
            }
            break; // end of qualifiers
        }
    }

#if 0
    printf("args_ok is %d\n", args_ok);
    printf("help is %d\n", help);
    printf("version is %d\n", version);
    printf("no_resolve is %d\n", no_resolve);
    printf("quiet is %d\n", quiet);
    printf("verbose is %d\n", verbose);
    printf("no-updates is %d\n", no_updates);
    printf("ignore_errors is %d\n", ignore_errors);
    printf("repository is %s\n", repository.c_str());
    printf("savefile is %s\n", savefile.c_str());
    printf("install_prefix is %s\n", install_prefix.c_str());
    exit(EXIT_SUCCESS);
#endif
    
    // Usually argv[command_index] will be a sub-command, unless
    // --help or --version has been used.
    
    // Always output the version number, irrespective of subsequent
    // commands or any problems. This can be useful in batch jobs.
    if (version) {
        printf ("ecosconfig %s (%s %s)\n%s\n", TOOL_VERSION, __DATE__, __TIME__, TOOL_COPYRIGHT);
        if (command_index == argc) {
            return EXIT_SUCCESS;
        }
    }
    // Cope with --help and any user errors. If --help is used then
    // subsequent arguments should be ignored, as should any problems
    // with the arguments. This allows the user to type a partial
    // command, then switch to --help, and use shell history editing
    // to complete/correct the command.
    if (help || !args_ok || (command_index == argc)) {
        usage_message();
        return help ? EXIT_SUCCESS : EXIT_FAILURE;
    }
    
    // set the default save file
    if (savefile.empty ()) { // if the save file was not specified on the command line
        savefile = DEFAULT_SAVE_FILE; // use the default save file
    }

    // find the repository
    if (repository.empty ()) { // if the repository was not specified on the command line
        const char * env_var = getenv ("ECOS_REPOSITORY");
        if (env_var) { // if the ECOS_REPOSITORY environment variable is defined
            repository = env_var;
        } else { // the ECOS_REPOSITORY environment variable is not defined
            // assume that the tool is located in the root of the repository
#ifdef _MSC_VER
            char toolpath [_MAX_PATH + 1];
            _fullpath (toolpath, argv [0], sizeof (toolpath)); // get the absolute path to the tool
#else
            // NOTE: portability problem. realpath() is not a POSIX function.
            // Alternative code may be needed on some platforms.
            char toolpath [MAXPATHLEN + 1];
            realpath (argv [0], toolpath); // get the absolute path to the tool
#endif
            repository = toolpath;
            for (unsigned int n = repository.size () - 1; n > 0; n--) { // for each char starting at the tail
                if (('\\' == repository [n]) || ('/' == repository [n])) { // if the char is a directory separator
                    repository.resize (n); // remove the filename from the filepath
                    break;
                }
            }
        }
    }

    repository          = TRANSLATE_PATH(repository);
    savefile            = TRANSLATE_PATH(savefile);
    install_prefix      = TRANSLATE_PATH(install_prefix);

    // Initialize the cdl_exec code (not quite sure why this needs a
    // separate object rather than just a bunch of statics). 
    cdl_exec exec (trim_path (repository), savefile, trim_path (install_prefix), no_resolve);
    cdl_exec::set_quiet_mode(quiet);
    cdl_exec::set_verbose_mode(verbose);
    cdl_exec::set_ignore_errors_mode(ignore_errors);
    cdl_exec::set_no_updates_mode(no_updates);
    if (enable_debug_set) {
        cdl_exec::set_debug_level(debug_level);
    }
    
    // Now identify and process the sub-command.
    const std::string command = argv [command_index];
    command_index++;
    bool status = false;

    if ("new" == command) {
        // Usage: ecosconfig new <target> [template [version]]
        if ((command_index == argc) || ((command_index + 3) <= argc)) {
            usage_message();
        } else {
            // The default values for template and template_version
            // are part of the cdl_exec class, so cdl_exec::cmd_new() has
            // to be invoked with the right number of arguments.
            if ((command_index + 1) == argc) {
                status = exec.cmd_new(argv[command_index]);
            } else if ((command_index + 2) == argc) {
                status = exec.cmd_new(argv[command_index], argv[command_index + 1]);
            } else {
                status = exec.cmd_new(argv[command_index], argv[command_index + 1], argv[command_index + 2]);
            }
        }
    } else if ("tree" == command) {
        // Usage: ecosconfig tree
        if (command_index == argc) {
            status = exec.cmd_tree ();
        } else {
            usage_message ();
        }
    } else if ("list" == command) {
        // Usage: ecosconfig list
        if (command_index == argc) {
            status = exec.cmd_list ();
        } else {
            usage_message ();
        }
    } else if ("check" == command) {
        // Usage: ecosconfig check
        if (command_index == argc) {
            status = exec.cmd_check ();
        } else {
            usage_message ();
        }
    } else if ("resolve" == command) {
        // Usage: ecosconfig resolve
        if (command_index == argc) {
            status = exec.cmd_resolve ();
        } else {
            usage_message ();
        }
    } else if ("add" == command) {
        // Usage: ecosconfig add <package> [<package2> ...]
        if (command_index < argc) {
            std::vector<std::string> packages;
            for (int n = command_index; n < argc; n++) {
                packages.push_back (argv [n]);
            }
            status = exec.cmd_add (packages);
        } else {
            usage_message ();
        }
    } else if ("remove" == command) {
        // Usage: ecosconfig remove <package> [<package2> ...]
        if (command_index < argc) {
            std::vector<std::string> packages;
            for (int n = command_index; n < argc; n++) {
                packages.push_back (argv [n]);
            }
            status = exec.cmd_remove (packages);
        } else {
            usage_message ();
        }
    } else if ("version" == command) {
        // Usage: ecosconfig version <version> <package> [<package2> ...]
        // Note that it is not possible to change several packages to different versions. 
        if (command_index + 1 < argc) {
            std::vector<std::string> packages;
            for (int n = command_index + 1; n < argc; n++) {
                packages.push_back (argv [n]);
            }
            status = exec.cmd_version (argv [command_index], packages);
        } else {
            usage_message ();
        }

    } else if ("target" == command) {
        // Usage: ecosconfig target <target>
        if (command_index + 1 == argc) {
            status = exec.cmd_target (argv [command_index]);
        } else {
            usage_message ();
        }

    } else if ("template" == command) {
        // Usage: ecosconfig template <template> [<version>]
        if (command_index + 1 == argc) {
            status = exec.cmd_template (argv [command_index]);
        } else if (command_index + 2 == argc) {
            status = exec.cmd_template (argv [command_index], argv [command_index]);
        } else {
            usage_message ();
        }

    } else if ("export" == command) {
        // Usage: ecosconfige export <filename>
        if (command_index + 1 == argc) {
            std::string filename = std::string(argv[command_index]);
            filename = TRANSLATE_PATH(filename);
            status = exec.cmd_export(filename);
        } else {
            usage_message ();
        }

    } else if ("import" == command) {
        // Usage: ecosconfig import <filename>
        if (command_index + 1 == argc) {
            std::string filename = std::string(argv[command_index]);
            filename = TRANSLATE_PATH(filename);
            status = exec.cmd_import(filename);
        } else {
            usage_message ();
        }

    } else {
        usage_message ();
    }

    return status ? EXIT_SUCCESS : EXIT_FAILURE;
}

// remove the trailing directory separator from a file path if present
std::string trim_path (const std::string input) {
    std::string output = input;
    if (! output.empty ()) {
        const char last_char = output [output.size () - 1];
        if (('\\' == last_char) || ('/' == last_char)) { // if the last char is a directory separator
            output.resize (output.size () - 1); // remove the last char
        }
    }
    return output;
}

// print a usage message
void usage_message () {
    printf ("Usage: ecosconfig [ qualifier ... ] [ command ]\n");
    printf ("  commands are:\n");
    printf ("    list                                       : list repository contents\n");
    printf ("    new TARGET [ TEMPLATE [ VERSION ] ]        : create a configuration\n");
    printf ("    target TARGET                              : change the target hardware\n");
    printf ("    template TEMPLATE [ VERSION ]              : change the template\n");
    printf ("    add PACKAGE [ PACKAGE ... ]                : add package(s)\n");
    printf ("    remove PACKAGE [ PACKAGE ... ]             : remove package(s)\n");
    printf ("    version VERSION PACKAGE [ PACKAGE ... ]    : change version of package(s)\n");
    printf ("    export FILE                                : export minimal config info\n");
    printf ("    import FILE                                : import additional config info\n");
    printf ("    check                                      : check the configuration\n");
    printf ("    resolve                                    : resolve conflicts\n");
    printf ("    tree                                       : create a build tree\n");
    printf ("  qualifiers are:\n");
    printf ("    --config=FILE                              : the configuration file\n");
    printf ("    --prefix=DIRECTORY                         : the install prefix\n");
    printf ("    --srcdir=DIRECTORY                         : the source repository\n");
    printf ("    --no-resolve                               : disable conflict resolution\n");
    printf ("    --version                                  : show version and copyright\n");
    printf ("    -q, --quiet                                : reduce verbosity\n");    
    printf ("    -v, --verbose                              : increase verbosity\n");    
    printf ("    -i, --ignore-errors                        : ignore unresolved conflicts\n");
    printf ("    -n, --no-updates                           : read-only mode, do not modify the file system\n");
    printf ("    --enable-debug                             : enable debugging in this configuration\n");
    printf ("    --disable-debug                            : disable debugging in this configuration\n");
    printf ("    --help                                     : display this message\n");
}
