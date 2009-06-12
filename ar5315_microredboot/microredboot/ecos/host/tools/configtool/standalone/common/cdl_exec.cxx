//####COPYRIGHTBEGIN####
//                                                                          
// ----------------------------------------------------------------------------
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 John Dallaway
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
//      cdl_exec.cxx
//
//      The implementation of each ecosconfig command
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

#ifdef _MSC_VER
#include <direct.h> /* for getcwd() */
#else
#include <unistd.h> /* for getcwd() */
#endif
#ifdef __CYGWIN__
#include <windows.h>
#include <sys/param.h>  /* for MAXPATHLEN */
#include <sys/cygwin.h> /* for cygwin_conv_to_win32_path() */
#endif
#include "build.hxx"
#include "cdl_exec.hxx"

// ----------------------------------------------------------------------------
bool cdl_exec::quiet            = false;
bool cdl_exec::verbose          = false;
bool cdl_exec::ignore_errors    = false;
bool cdl_exec::no_updates       = false;
bool cdl_exec::debug_level_set  = false;
int  cdl_exec::debug_level      = 0;

cdl_exec::cdl_exec (const std::string repository_arg, const std::string savefile_arg,
                    const std::string install_arg, bool no_resolve_arg)
    : repository(repository_arg),
      savefile(savefile_arg),
      install_prefix(install_arg),
      no_resolve(no_resolve_arg),
      pkgdata (NULL),
      interp (NULL),
      config (NULL)
{

    // The inference callback does not actually do anything at present.
    // In future it may be useful for diagnostic purposes.
    CdlTransactionBody::set_inference_callback_fn (&inference_callback);
    
    // Automatic inference is always disabled. The inference engine
    // only gets invoked explicitly, after a suitable transaction callback
    // has been invoked. The problem here is that the transaction callback
    // has to report changes made by the inference engine but there is
    // no way of distinguishing between inferred values that come out of
    // savefiles and inferred values determined by the inference engine.
    CdlTransactionBody::disable_automatic_inference ();
}

void
cdl_exec::set_quiet_mode(bool new_val)
{
    quiet = new_val;
}

void
cdl_exec::set_verbose_mode(bool new_val)
{
    verbose = new_val;
    CdlPackagesDatabaseBody::set_verbose(new_val);
}

void
cdl_exec::set_ignore_errors_mode(bool new_val)
{
    ignore_errors = new_val;
}

void
cdl_exec::set_no_updates_mode(bool new_val)
{
    no_updates = new_val;
}

void
cdl_exec::set_debug_level(int new_level)
{
    debug_level_set = true;
    debug_level = new_level;
}

// ----------------------------------------------------------------------------
void
cdl_exec::init(bool load_config)
{
    pkgdata = CdlPackagesDatabaseBody::make(repository, &diagnostic_handler, &diagnostic_handler);
    interp  = CdlInterpreterBody::make();
    if (load_config) {
        config = CdlConfigurationBody::load (savefile, pkgdata, interp, &diagnostic_handler, &diagnostic_handler);
    }
}

// ----------------------------------------------------------------------------
void
cdl_exec::delete_cdl_data ()
{
    if (0 != config) {
        delete config;
        config = 0;
    }
    if (0 != interp) {
        delete interp;
        interp = 0;
    }
    if (0 != pkgdata) {
        delete pkgdata;
        pkgdata = 0;
    }
}

// ----------------------------------------------------------------------------
bool cdl_exec::cmd_new (const std::string cdl_hardware,
                        const std::string cdl_template /* = "default" */,
                        const std::string cdl_version /* = "" */)
{
    bool status = false;
    try {
        init(false);
        
        config = CdlConfigurationBody::make ("eCos", pkgdata, interp);

        // The hardware and template should be loaded in a single transaction.
        // Validating the target name etc. can be left to libcdl.
        CdlLocalTransaction transact(config);
        config->set_hardware(transact.get(), resolve_hardware_alias(cdl_hardware), &diagnostic_handler, &diagnostic_handler);
        config->set_template(transact.get(), cdl_template, cdl_version, &diagnostic_handler, &diagnostic_handler);
        transact.body();
        transact.destroy();

        if (debug_level_set) {
            this->update_debug_level();
        }
        
        // Unless inference has been suppressed, make sure that the
        // inference engine gets invoked and that its results get
        // reported.
        if (!no_resolve) {
            CdlTransactionBody::set_callback_fn(&transaction_callback);
            config->resolve_all_conflicts();
        }

        // Now report any conflicts which the inference engine could not report. 
        report_conflicts();

        // A savefile should be generated/updated even if there are conflicts.
        // Otherwise the user does not have a chance to edit the savefile
        // and fix things.
        if (!no_updates) {
            config->save (savefile);
        }
        if (ignore_errors || (0 == config->get_all_conflicts().size())) {
            status = true;
        }
    } catch (CdlStringException exception) {
        exception_handler (exception);
    } catch (...) {
        exception_handler ();
    }

    delete_cdl_data ();
    return status;
}

// ----------------------------------------------------------------------------
bool
cdl_exec::cmd_target (const std::string cdl_target)
{
    bool status = false;
    try {
        init(true);
        config->set_hardware (resolve_hardware_alias (cdl_target), &diagnostic_handler, &diagnostic_handler);
        if (debug_level_set) {
            this->update_debug_level();
        }
        if (!no_resolve) {
            CdlTransactionBody::set_callback_fn(&transaction_callback);
            config->resolve_all_conflicts();
        }
        report_conflicts();
        if (!no_updates) {
            config->save (savefile);
        }
        if (ignore_errors || (0 == config->get_all_conflicts().size())) {
            status = true;
        }
    } catch (CdlStringException exception) {
        exception_handler (exception);
    } catch (...) {
        exception_handler ();
    }

    delete_cdl_data ();
    return status;
}

// ----------------------------------------------------------------------------
bool
cdl_exec::cmd_template (const std::string cdl_template, const std::string cdl_version /* = "" */)
{
    bool status = false;
    try {
        init(true);
        config->set_template (cdl_template, cdl_version, &diagnostic_handler, &diagnostic_handler);
        if (debug_level_set) {
            this->update_debug_level();
        }
        if (!no_resolve) {
            CdlTransactionBody::set_callback_fn(&transaction_callback);
            config->resolve_all_conflicts();
        }
        report_conflicts();
        if (!no_updates) {
            config->save (savefile);
        }
        if (ignore_errors || (0 == config->get_all_conflicts().size())) {
            status = true;
        }
    } catch (CdlStringException exception) {
        exception_handler (exception);
    } catch (...) {
        exception_handler ();
    }

    delete_cdl_data ();
    return status;
}

// ----------------------------------------------------------------------------
bool
cdl_exec::cmd_export (const std::string cdl_savefile)
{
    bool status = false;
    try {
        init(true);
        if (debug_level_set) {
            this->update_debug_level();
        }
        if (!no_resolve) {
            CdlTransactionBody::set_callback_fn(&transaction_callback);
            config->resolve_all_conflicts();
        }
        report_conflicts();
        // Exporting to another file should only happen if the
        // configuration is conflict-free. This is different from
        // updating the savefile.
        if (ignore_errors || (0 == config->get_all_conflicts().size())) {
            if (!no_updates) {
                config->save (cdl_savefile, /* minimal = */ true);
            }
            status = true;
        }
    } catch (CdlStringException exception) {
        exception_handler (exception);
    } catch (...) {
        exception_handler ();
    }

    delete_cdl_data ();
    return status;
}

// ----------------------------------------------------------------------------
bool
cdl_exec::cmd_import (const std::string cdl_savefile)
{
    bool status = false;
    try {
        init(true);
        config->add(cdl_savefile, &diagnostic_handler, &diagnostic_handler);
        if (debug_level_set) {
            this->update_debug_level();
        }
        if (!no_resolve) {
            CdlTransactionBody::set_callback_fn(&transaction_callback);
            config->resolve_all_conflicts();
        }
        report_conflicts();
        if (!no_updates) {
            config->save (savefile);
        }
        if (ignore_errors || (0 == config->get_all_conflicts().size())) {
            status = true;
        }
    } catch (CdlStringException exception) {
        exception_handler (exception);
    } catch (...) {
        exception_handler ();
    }

    delete_cdl_data ();
    return status;
}

// ----------------------------------------------------------------------------
bool
cdl_exec::cmd_add (const std::vector<std::string> cdl_packages)
{
    bool status = false;
    try {
        init(true);
        for (unsigned int n = 0; n < cdl_packages.size (); n++) {
            config->load_package (resolve_package_alias (cdl_packages [n]), "", &diagnostic_handler, &diagnostic_handler);
        }
        if (debug_level_set) {
            this->update_debug_level();
        }
        if (!no_resolve) {
            CdlTransactionBody::set_callback_fn(&transaction_callback);
            config->resolve_all_conflicts();
        }
        report_conflicts();
        if (!no_updates) {
            config->save (savefile);
        }
        if (ignore_errors || (0 == config->get_all_conflicts().size())) {
            status = true;
        }
    } catch (CdlStringException exception) {
        exception_handler (exception);
    } catch (...) {
        exception_handler ();
    }

    delete_cdl_data ();
    return status;
}

// ----------------------------------------------------------------------------
bool
cdl_exec::cmd_remove (const std::vector<std::string> cdl_packages)
{
    unsigned int n;
    bool status = false;
    try {
        init(true);
        for (n = 0; n < cdl_packages.size (); n++) {
            if (! config->lookup (resolve_package_alias (cdl_packages [n]))) {
                throw CdlStringException ("Unknown package " + cdl_packages [n]);
            }
        }
        for (n = 0; n < cdl_packages.size (); n++) {
            config->unload_package (resolve_package_alias (cdl_packages [n]));
        }
        if (debug_level_set) {
            this->update_debug_level();
        }
        if (!no_resolve) {
            CdlTransactionBody::set_callback_fn(&transaction_callback);
            config->resolve_all_conflicts();
        }
        report_conflicts();
        if (!no_updates) {
            config->save (savefile);
        }
        if (ignore_errors || (0 == config->get_all_conflicts().size())) {
            status = true;
        }
    } catch (CdlStringException exception) {
        exception_handler (exception);
    } catch (...) {
        exception_handler ();
    }

    delete_cdl_data ();
    return status;
}

// ----------------------------------------------------------------------------
bool
cdl_exec::cmd_version (const std::string cdl_version, const std::vector<std::string> cdl_packages)
{
    bool status = false;
    try {
        init(true);
        for (unsigned int n = 0; n < cdl_packages.size (); n++) {
            config->change_package_version(resolve_package_alias (cdl_packages [n]), cdl_version,
                                           &diagnostic_handler, &diagnostic_handler, true);
        }
        if (debug_level_set) {
            this->update_debug_level();
        }
        if (!no_resolve) {
            CdlTransactionBody::set_callback_fn(&transaction_callback);
            config->resolve_all_conflicts();
        }
        report_conflicts();
        if (!no_updates) {
            config->save (savefile);
        }
        if (ignore_errors || (0 == config->get_all_conflicts().size())) {
            status = true;
        }
    } catch (CdlStringException exception) {
        exception_handler (exception);
    } catch (...) {
        exception_handler ();
    }

    delete_cdl_data ();
    return status;
}

// ----------------------------------------------------------------------------
bool
cdl_exec::cmd_tree ()
{
    bool status = false;
    try {
        init(true);
        if (debug_level_set) {
            this->update_debug_level();
        }
        if (!no_resolve) {
            CdlTransactionBody::set_callback_fn(&transaction_callback);
            config->resolve_all_conflicts();
        }
        report_conflicts();
        if (!no_updates) {
            config->save (savefile);
        }
        // A build tree should only be generated if there are no conflicts,
        // and suppressed if -n is given.
        if (no_updates) {
            // Do nothing
        }
        else if (ignore_errors || (0 == config->get_all_conflicts().size())) {
#ifdef _MSC_VER
            char cwd [_MAX_PATH + 1];
#else
            char cwd [PATH_MAX + 1];
#endif
            getcwd (cwd, sizeof cwd);
#ifdef __CYGWIN__
            char cwd_win32 [MAXPATHLEN + 1];
            cygwin_conv_to_win32_path (cwd, cwd_win32);
            generate_build_tree (config, cwd_win32, install_prefix);
#else
            generate_build_tree (config, cwd, install_prefix);
#endif
            config->generate_config_headers (install_prefix.empty () ? "install/include/pkgconf" : install_prefix + "/include/pkgconf");
            status = true;
#ifdef __CYGWIN__
            char buf[100];
            strcpy(buf, "mount.exe -f -t -u x: /ecos-x");
            //printf("Cwd_win32: %s\n", cwd_win32);

            if ( cwd_win32[1] == ':' )
            {
                buf[19] = tolower(cwd_win32[0]);
                buf[28] = tolower(cwd_win32[0]);
                system(buf);
            }

            //printf("Repository: %s\n", repository.c_str());

            if ( repository[1] == ':' )
            {
                buf[19] = tolower(repository[0]);
                buf[28] = tolower(repository[0]);
                system(buf);
            }
            if ( !install_prefix.empty() )
            {
                //printf("Install prefix: %s\n", install_prefix.c_str());
                if ( install_prefix[1] == ':' )
                {
                    buf[19] = tolower(install_prefix[0]);
                    buf[28] = tolower(install_prefix[0]);
                    system(buf);
                }
            }
#endif
        } else {
            printf("\nUnable to generate build tree, this configuration still contains conflicts.\n");
            printf("Either resolve the conflicts or use --ignore-errors\n");
        }
    } catch (CdlStringException exception) {
        exception_handler (exception);
    } catch (...) {
        exception_handler ();
    }

    delete_cdl_data ();
    return status;
}

// ----------------------------------------------------------------------------
bool
cdl_exec::cmd_list ()
{
    bool status = false;
    try {
        init(false);

        // list the installed packages
        std::vector<std::string> packages = pkgdata->get_packages ();
        std::sort (packages.begin (), packages.end ());
        for (unsigned int package = 0; package < packages.size (); package++) {
            const std::vector<std::string> & aliases = pkgdata->get_package_aliases (packages [package]);
            printf ("Package %s (%s):\n aliases:", packages [package].c_str (), aliases [0].c_str ());
            for (unsigned int alias = 1; alias < aliases.size (); alias++) {
                printf (" %s", aliases [alias].c_str ());
            }
            const std::vector<std::string> & versions = pkgdata->get_package_versions (packages [package]);
            printf ("\n versions:");
            for (unsigned int version = 0; version < versions.size (); version++) {
                printf (" %s", versions [version].c_str ());
            }
            printf ("\n");
        }

        // list the available targets
        std::vector<std::string> targets = pkgdata->get_targets ();
        std::sort (targets.begin (), targets.end ());
        for (unsigned int target = 0; target < targets.size (); target++) {
            const std::vector<std::string> & aliases = pkgdata->get_target_aliases (targets [target]);
            printf ("Target %s (%s):\n aliases:", targets [target].c_str (), aliases [0].c_str ());
            for (unsigned int alias = 1; alias < aliases.size (); alias++) {
                printf (" %s", aliases [alias].c_str ());
            }
            printf ("\n");
        }

        // list the available templates
        std::vector<std::string> templates = pkgdata->get_templates ();
        std::sort (templates.begin (), templates.end ());
        for (unsigned int templ = 0; templ < templates.size (); templ++) {
            const std::vector<std::string> & versions = pkgdata->get_template_versions (templates [templ]);
            printf ("Template %s:\n versions:", templates [templ].c_str ());
            for (unsigned int version = 0; version < versions.size (); version++) {
                printf (" %s", versions [version].c_str ());
            }
            printf ("\n");
        }

        status = true;
    } catch (CdlStringException exception) {
        exception_handler (exception);
    } catch (...) {
        exception_handler ();
    }

    delete_cdl_data ();
    return status;
}

// ----------------------------------------------------------------------------
bool
cdl_exec::cmd_check ()
{
    bool status = false;
    unsigned int n;

    try {
        init(true);
        // check() should never invoke the inference engine. The user
        // wants to determine the current status, which should not
        // change.
        // However, updating the savefile is worthwhile because it
        // will now contain more accurate information about the state.
        // Enabling/disabling debugs is allowed for now because that
        // is unlikely to introduce conflicts.
        if (debug_level_set) {
            this->update_debug_level();
        }
        if (!no_updates) {
            config->save (savefile);
        }

        // report current target and template
        printf ("Target: %s\n", config->get_hardware ().c_str ());
        printf ("Template: %s\n", config->get_template ().c_str ());
        std::vector<std::string> template_packages = pkgdata->get_template_packages (config->get_template ());
        const std::vector<std::string> & hardware_packages = pkgdata->get_target_packages (config->get_hardware ());
        for (n = 0; n < hardware_packages.size (); n++) {
            template_packages.push_back (hardware_packages [n]);
        }

        // report loaded packages not in the templates
        const std::vector<CdlLoadable> & loadables = config->get_loadables ();
        std::vector<std::string> added_packages;
        std::vector<CdlLoadable>::const_iterator loadable_i;
        for (loadable_i = loadables.begin (); loadable_i != loadables.end (); loadable_i++) {
            const CdlNode & node = dynamic_cast<CdlNode> (* loadable_i);
            if (template_packages.end () == std::find (template_packages.begin (), template_packages.end (), node->get_name ())) {
                added_packages.push_back (node->get_name ());
            }
        }
        if (added_packages.size ()) {
            printf ("Added:\n");
        }
        for (n = 0; n < added_packages.size (); n++) {
            printf (" %s\n", added_packages [n].c_str ());
        }

        // report template packages not in the configuration
        std::vector<std::string> removed_packages;
        for (n = 0; n < template_packages.size (); n++) {
            if (! config->lookup (template_packages [n])) {
                removed_packages.push_back (template_packages [n]);
            }
        }
        if (removed_packages.size ()) {
            printf ("Removed:\n");
        }
        for (n = 0; n < removed_packages.size (); n++) {
            printf (" %s\n", removed_packages [n].c_str ());
        }

        // report packages of non-default version
        std::vector<CdlValuable> version_packages;
        for (loadable_i = loadables.begin (); loadable_i != loadables.end (); loadable_i++) {
            const CdlValuable & valuable = dynamic_cast<CdlValuable> (* loadable_i);
            if (pkgdata->get_package_versions (valuable->get_name ()) [0] != valuable->get_value ()) {
                version_packages.push_back (valuable);
            }
        }
        if (version_packages.size ()) {
            printf ("Version(s):\n");
        }
        for (n = 0; n < version_packages.size (); n++) {
            printf (" %s %s\n", version_packages [n]->get_name ().c_str (), version_packages [n]->get_value ().c_str ());
        }

        // report conflicts
        const std::list<CdlConflict> & conflicts = config->get_all_conflicts ();
        if (conflicts.size ()) {
            printf ("%u conflict(s):\n", conflicts.size ());
        } else {
            printf ("No conflicts\n");
        }
        report_conflicts();

        status = true;
    } catch (CdlStringException exception) {
        exception_handler (exception);
    } catch (...) {
        exception_handler ();
    }

    delete_cdl_data ();
    return status;
}

// ----------------------------------------------------------------------------
bool
cdl_exec::cmd_resolve ()
{
    bool status = false;

    try {
        init(true);
        if (debug_level_set) {
            this->update_debug_level();
        }
        CdlTransactionBody::set_callback_fn(&transaction_callback);
        config->resolve_all_conflicts ();
        report_conflicts();
        if (!no_updates) {
            config->save (savefile);
        }
        if (ignore_errors || (0 == config->get_all_conflicts().size())) {
            status = true;
        }
    } catch (CdlStringException exception) {
        exception_handler (exception);
    } catch (...) {
        exception_handler ();
    }

    delete_cdl_data ();
    return status;
}

// ----------------------------------------------------------------------------
// The inference callback. This could give some useful diagnostics, or it
// could do useful things when running in some interactive mode. In batch
// mode it should not do anything.

CdlInferenceCallbackResult
cdl_exec::inference_callback (CdlTransaction transaction)
{
    return CdlInferenceCallbackResult_Continue;
}

// ----------------------------------------------------------------------------
// Output a message with indentation after newlines.
static void
dump_string(unsigned int indent, const std::string& str)
{
    bool newline_pending = false;
    unsigned int i, j;
    for (i = 0; i < str.size(); i++) {
        if (newline_pending) {
            putchar('\n');
            if ('\n' != str[i]) {
                for (j = 0; j < indent; j++) {
                    putchar(' ');
                }
            }
            newline_pending = false;
        }
        if ('\n' == str[i]) {
            newline_pending = true;
        } else {
            putchar(str[i]);
        }
    }
    if (newline_pending) {
        putchar('\n');  // But not the indentation.
    }
}

// ----------------------------------------------------------------------------
// The transaction callback. This should report any changes that have been
// made to the configuration. The amount of output depends on the verbosity
// level selected by the user.
//
// 1) quiet     - no output at all
// 2) default   - list updates done by the inference engine.
// 3) verbose   - this does not currently add anything.
// 
// There is no reporting of new or resolved conflicts. Resolved
// conflicts are probably of no interest in batch mode. New conflicts
// will be handled by report_conflicts(). There is also no information
// given about active state changes, although arguably there should be
// especially in the case of containers.

void
cdl_exec::transaction_callback(const CdlTransactionCallback& callback_data)
{
    if (quiet) {
        return;
    }

    unsigned int i;
    for (i = 0; i < callback_data.value_changes.size(); i++) {
        CdlValuable valuable = callback_data.value_changes[i];
        if (CdlValueSource_Inferred == valuable->get_source()) {
            CdlEvalContext context(0, valuable, 0);
            CdlSimpleValue simple_val;
            CdlSimpleValue::eval_valuable(context, valuable, simple_val);
            std::string msg = std::string("U ") + valuable->get_name() + ", new inferred value ";
            std::string value = simple_val.get_value();
            if ("" == value) {
                msg += "\"\"";
            } else {
                msg += value;
            }
            msg += "\n";
            dump_string(4, msg);
        }
    }
}

// ----------------------------------------------------------------------------
// Report the remaining conflicts in the configuration. These indicate
// problems that the user should fix before going further with the
// configuration, e.g. before generating a build tree.
//
// Quiet verbosity level has no effect on this, but at the verbose level
// it is a good idea to look for a possible solution to the conflict.


void
cdl_exec::report_conflicts()
{
    const std::list<CdlConflict>& all_conflicts = config->get_all_conflicts();
    std::list<CdlConflict>::const_iterator conf_i;
    for (conf_i = all_conflicts.begin(); conf_i != all_conflicts.end(); conf_i++) {
        CdlNode     node = (*conf_i)->get_node();

        std::string msg = std::string("C ") + node->get_name() + ", " + (*conf_i)->get_explanation() + "\n";
        dump_string(2, msg);

        if (verbose && (*conf_i)->resolution_implemented()) {
            // See if there is a possible solution to this conflict.
            // This involves creating a transaction, invoking the
            // inference engine, and cancelling the transaction
            // (thus making sure that nothing actually changes).
            //
            // NOTE: at some stage libcdl may keep track of solutions
            // globally. However, although it will know when a solution
            // becomes invalid it will not necessarily try to resolve
            // all global conflicts after every change, so attempting
            // to do this in a transaction may still be necessary.
            CdlTransaction transact = CdlTransactionBody::make(config);
            transact->resolve(*conf_i);
            if ((*conf_i)->has_known_solution()) {
                std::string soln_msg = "  Possible solution:\n";
                const std::vector<std::pair<CdlValuable, CdlValue> > & soln = (*conf_i)->get_solution();
                unsigned int i;
                for (i = 0; i < soln.size(); i++) {
                    CdlValuable valuable = soln[i].first;
                    soln_msg += valuable->get_name();
                    soln_msg += " -> ";
                    switch(valuable->get_flavor()) {
                      case CdlValueFlavor_Bool :
                        if (!soln[i].second.is_enabled()) {
                            soln_msg += "0 (disabled)";
                        } else {
                            soln_msg += "1 (enabled)";
                        }
                        break;
                      case CdlValueFlavor_Data:
                        soln_msg += soln[i].second.get_value();
                        break;
                      case CdlValueFlavor_BoolData:
                        if (!soln[i].second.is_enabled()) {
                            soln_msg += "0 " + soln[i].second.get_value();
                        } else {
                            soln_msg += "1 " + soln[i].second.get_value();
                        }
                        break;
                        // An option with flavor none cannot be involved
                        // in a solution.
                      default:
                        soln_msg += "<internal error>";
                        break;
                    }
                    soln_msg += "\n";
                }
                
#if 0
                // FIXME: currently this member only works for nested sub-transactions.
                if (transact->user_confirmation_required()) {
                    msg += "This change affects previous user settings.\n";
                }
#endif                
                dump_string(4, soln_msg);
            }
            transact->cancel();
            delete transact;
        }
    }
}

// ----------------------------------------------------------------------------
void
cdl_exec::diagnostic_handler (std::string message)
{
    printf ("%s\n", message.c_str ());
}

void cdl_exec::exception_handler (CdlStringException exception) {
    printf ("%s\n", exception.get_message ().c_str ());
}

void
cdl_exec::exception_handler ()
{
    printf ("Unknown error\n");
}


// ----------------------------------------------------------------------------
std::string
cdl_exec::resolve_package_alias (const std::string alias)
{
    std::string package = alias;

    if (! pkgdata->is_known_package (alias)) { // if the alias is not a package name
        const std::vector<std::string> & packages = pkgdata->get_packages (); // get packages
        for (unsigned int n = 0; n < packages.size (); n++) { // for each package
            const std::vector<std::string> & aliases = pkgdata->get_package_aliases (packages [n]); // get package aliases
            if (aliases.end () != std::find (aliases.begin (), aliases.end (), alias)) { // if alias is found
                package = packages [n]; // note the package
                break;
            }
        }
    }
    return package;
}

std::string
cdl_exec::resolve_hardware_alias (const std::string alias)
{
    std::string target = alias;

    if (! pkgdata->is_known_target (alias)) { // if the alias is not a target name
        const std::vector<std::string> & targets = pkgdata->get_targets (); // get targets
        for (unsigned int n = 0; n < targets.size (); n++) { // for each target
            const std::vector<std::string> & aliases = pkgdata->get_target_aliases (targets [n]); // get target aliases
            if (aliases.end () != std::find (aliases.begin (), aliases.end (), alias)) { // if alias is found
                target = targets [n]; // note the target
                break;
            }
        }
    }
    return target;
}

// ----------------------------------------------------------------------------
// Enable or disable debugging in a configuration.
void
cdl_exec::update_debug_level()
{
    CdlNode node = config->lookup("CYGPKG_INFRA_DEBUG");
    CdlValuable valuable = 0;
    if (0 != node) {
        valuable = dynamic_cast<CdlValuable>(node);
    }
    if (0 == valuable) {
        throw CdlStringException("Cannot enable or disable debugging, the infrastructure package is absent");
    }
    
    if (debug_level > 0) {
        valuable->enable(CdlValueSource_User);
    } else {
        valuable->disable(CdlValueSource_User);
    }
}
