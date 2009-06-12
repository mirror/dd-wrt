//####COPYRIGHTBEGIN####
//                                                                          
// ----------------------------------------------------------------------------
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
#include "cdl.hxx"

class cdl_exec {
public:
    cdl_exec (const std::string repository_tree, const std::string savefile_name, const std::string install_tree, bool no_resolve);
    bool cmd_new (const std::string cdl_hardware, const std::string cdl_template = "default", const std::string cdl_version = "");
    bool cmd_tree ();
    bool cmd_check ();
    bool cmd_list ();
    bool cmd_add (const std::vector<std::string> cdl_packages);
    bool cmd_remove (const std::vector<std::string> cdl_packages);
    bool cmd_version (const std::string cdl_version, const std::vector<std::string> cdl_packages);
    bool cmd_template (const std::string cdl_template, const std::string cdl_version = "");
    bool cmd_export (const std::string cdl_savefile);
    bool cmd_import (const std::string cdl_savefile);
    bool cmd_target (const std::string cdl_target);
    bool cmd_resolve ();

    static void set_quiet_mode(bool);
    static void set_verbose_mode(bool);
    static void set_ignore_errors_mode(bool);
    static void set_no_updates_mode(bool);
    static void set_debug_level(int);
    
protected:
    static bool quiet;
    static bool verbose;
    static bool ignore_errors;
    static bool no_updates;
    std::string repository;
    std::string savefile;
    std::string install_prefix;
    bool no_resolve;
    static bool debug_level_set;
    static int  debug_level;
    CdlPackagesDatabase pkgdata;
    CdlInterpreter interp;
    CdlConfiguration config;
    void init(bool /* load */);
    void delete_cdl_data ();
    static void diagnostic_handler (std::string message);
    void exception_handler (CdlStringException exception);
    void exception_handler ();
    void report_conflicts();
    static CdlInferenceCallbackResult inference_callback (CdlTransaction transaction);
    static void transaction_callback(const CdlTransactionCallback&);
    std::string resolve_package_alias (const std::string alias);
    std::string resolve_hardware_alias (const std::string alias);
    void update_debug_level(void);
};
