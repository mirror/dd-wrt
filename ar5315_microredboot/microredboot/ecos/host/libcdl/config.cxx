//{{{  Banner                                           

//============================================================================
//
//     config.cxx
//
//     Implementation of the CdlConfiguration class
//
//============================================================================
//####COPYRIGHTBEGIN####
//                                                                          
// ----------------------------------------------------------------------------
// Copyright (C) 2002, 2003 Bart Veer
// Copyright (C) 1999, 2000 Red Hat, Inc.
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
// Date:        1999/03/06
// Version:     0.02
//
//####DESCRIPTIONEND####
//============================================================================

//}}}
//{{{  #include's                                       

// ----------------------------------------------------------------------------
#include "cdlconfig.h"

// Get the infrastructure types, assertions, tracing and similar
// facilities.
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/cyg_trac.h>

// <cdl.hxx> defines everything implemented in this module.
// It implicitly supplies <string>, <vector> and <map> because
// the class definitions rely on these headers.
#include <cdl.hxx>

//}}}

//{{{  CdlConfiguration constants and statics           

// ----------------------------------------------------------------------------
CYGDBG_DEFINE_MEMLEAK_COUNTER(CdlConfigurationBody);

//}}}
//{{{  CdlConfiguration:: creation                      

// ----------------------------------------------------------------------------
// The toplevel class will take care of just about everything.

CdlConfigurationBody::CdlConfigurationBody(std::string name, CdlPackagesDatabase db, CdlInterpreter interp)
    : CdlNodeBody(name),
      CdlToplevelBody(interp)
{
    CYG_REPORT_FUNCNAME("CdlConfiguration:: constructor");
    CYG_REPORT_FUNCARG1XV(this);

    current_hardware    = "";
    current_template    = "";
    database            = db;
    save_file           = "";
    description         = "";
    
    cdlconfigurationbody_cookie = CdlConfigurationBody_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();
    
    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------
// The exported interface

CdlConfiguration
CdlConfigurationBody::make(std::string name, CdlPackagesDatabase db, CdlInterpreter interp)
{
    CYG_REPORT_FUNCNAMETYPE("CdlConfiguration::make", "result %p");
    CYG_REPORT_FUNCARG2XV(db, interp);
    CYG_PRECONDITIONC("" != name);
    CYG_PRECONDITION_CLASSC(db);
    CYG_PRECONDITION_CLASSC(interp);

    CdlConfiguration result = new CdlConfigurationBody(name, db, interp);
    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  CdlConfiguration:: destructor                    

// ----------------------------------------------------------------------------
CdlConfigurationBody::~CdlConfigurationBody()
{
    CYG_REPORT_FUNCNAME("CdlConfiguration:: default destructor");
    CYG_REPORT_FUNCARG1("this %p", this);
    CYG_PRECONDITION_THISC();

    // Removing all the packages has to happen here, and in the
    // context of a transaction. The main reason is the extensive
    // use of dynamic casts, after this destructor returns
    // any dynamic casts for this configuration will fail.
    //
    // Arguably some of the unloads should happen by clearing
    // the hardware and template (assuming those are currently
    // set). In practice that would not really gain anything.
    //
    // Unloading the individual packages is a bit more expensive
    // than it should be, since lots of care is taken to keep
    // remaining packages consistent and then those get unloaded
    // as well. However it is the safe approach.
    CdlLocalTransaction transaction(this);
    const std::vector<CdlLoadable>& loadables = this->get_loadables();
    for (int i = loadables.size() - 1; i >= 0; i--) {
        CdlPackage pkg = dynamic_cast<CdlPackage>(loadables[i]);
        if (0 != pkg) {
            this->unload_package(transaction.get(), pkg);
        }
    }
    transaction.propagate();
    transaction.commit();
    
    cdlconfigurationbody_cookie = CdlConfigurationBody_Invalid;
    current_hardware            = "";
    current_template            = "";
    database                    = 0;
    save_file                   = "";
    
    CYGDBG_MEMLEAK_DESTRUCTOR();
    
    CYG_REPORT_RETURN();
}

//}}}
//{{{  CdlConfiguration::check_this()                   

// ----------------------------------------------------------------------------
// There is very little information associated with a configuration.

bool
CdlConfigurationBody::check_this(cyg_assert_class_zeal zeal) const
{
    if (CdlConfigurationBody_Magic != cdlconfigurationbody_cookie) {
        return false;
    }
    CYGDBG_MEMLEAK_CHECKTHIS();

    switch(zeal) {
      case cyg_system_test      :
      case cyg_extreme          :
          if ((0 == database) || !database->check_this(cyg_quick)) {
              return false;
          }
      case cyg_thorough         :
          if (("" != current_hardware) && !database->is_known_target(current_hardware)) {
              return false;
          }
          if (("" != current_template) && !database->is_known_template(current_template)) {
              return false;
          }
      case cyg_quick            :
          if (0 == database) {
              return false;
          }
      case cyg_trivial          :
      case cyg_none             :
      default                   :
          break;
    }

    return CdlNodeBody::check_this(zeal) && CdlContainerBody::check_this(zeal) && CdlToplevelBody::check_this(zeal);
}

//}}}
//{{{  CdlConfiguration:: basic info                    

// ----------------------------------------------------------------------------
// Provide ready access to configuration-specific data.

CdlPackagesDatabase
CdlConfigurationBody::get_database() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlConfiguration::get_database", "result %p");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CYG_REPORT_RETVAL(database);
    return database;
}

std::string
CdlConfigurationBody::get_hardware() const
{
    CYG_REPORT_FUNCNAME("CdlConfiguration::get_hardware");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CYG_REPORT_RETURN();
    return current_hardware;
}

void
CdlConfigurationBody::set_hardware_name(std::string new_name)
{
    CYG_REPORT_FUNCNAME("CdlConfiguration::set_hardware_name");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    current_hardware = new_name;
    
    CYG_REPORT_RETURN();
}

std::string
CdlConfigurationBody::get_template() const
{
    CYG_REPORT_FUNCNAME("CdlConfiguration::get_template");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CYG_REPORT_RETURN();
    return current_template;
}

void
CdlConfigurationBody::set_template_name(std::string new_name)
{
    CYG_REPORT_FUNCNAME("CdlConfiguration::set_template_name");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    current_template = new_name;
    
    CYG_REPORT_RETURN();
}

std::string
CdlConfigurationBody::get_save_file() const
{
    CYG_REPORT_FUNCNAME("CdlConfiguration::get_save_file");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CYG_REPORT_RETURN();
    return save_file;
}

// ----------------------------------------------------------------------------

std::string
CdlConfigurationBody::get_class_name() const
{
    CYG_REPORT_FUNCNAME("CdlConfiguration::get_class_name");
    CYG_PRECONDITION_THISC();
    CYG_REPORT_RETURN();
    return "CdlConfiguration";
}

//}}}
//{{{  Load and unload operations - wrappers            

// ----------------------------------------------------------------------------
// These members are basically wrappers for the functions that do the
// real work. They do things like running the real functions inside
// a newly created transaction.

void
CdlConfigurationBody::load_package(std::string name, std::string version,
                                   CdlDiagnosticFnPtr error_fn, CdlDiagnosticFnPtr warn_fn, bool limbo)
{
    CYG_REPORT_FUNCNAME("CdlConfiguration::load_package");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlLocalTransaction transaction(this);
    this->load_package(transaction.get(), name, version, error_fn, warn_fn, limbo);
    transaction.body();

    CYG_REPORT_RETURN();
}

void
CdlConfigurationBody::unload_package(std::string name, bool limbo)
{
    CYG_REPORT_FUNCNAME("CdlConfiguration::unload_package");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlLocalTransaction transaction(this);
    this->unload_package(transaction.get(), name, limbo);
    transaction.body();

    CYG_REPORT_RETURN();
}

void
CdlConfigurationBody::unload_package(CdlPackage package, bool limbo)
{
    CYG_REPORT_FUNCNAME("CdlConfiguration::unload_package");
    CYG_REPORT_FUNCARG1XV(this);

    CdlLocalTransaction transaction(this);
    this->unload_package(transaction.get(), package, limbo);
    transaction.body();

    CYG_REPORT_RETURN();
}

void
CdlConfigurationBody::unload_package(CdlTransaction transaction, std::string name, bool limbo)
{
    CYG_REPORT_FUNCNAME("CdlConfiguration::unload_package");
    CYG_REPORT_FUNCARG3XV(this, transaction, limbo);
    CYG_INVARIANT_THISC(CdlConfigurationBody);
    CYG_PRECONDITION_CLASSC(transaction);

    CdlNode node = lookup(name);
    CYG_ASSERTC(0 != node);
    CdlPackage package  = dynamic_cast<CdlPackage>(node);
    CYG_ASSERTC(0 != package);

    this->unload_package(transaction, package, limbo);

    CYG_REPORT_RETURN();
}

void
CdlConfigurationBody::change_package_version(std::string name, std::string version,
                                             CdlDiagnosticFnPtr error_fn, CdlDiagnosticFnPtr warn_fn, bool limbo)
{
    CYG_REPORT_FUNCNAME("CdlConfiguration::change_package_version");
    CYG_REPORT_FUNCARG1XV(this);

    CdlLocalTransaction transaction(this);
    this->change_package_version(transaction.get(), name, version, error_fn, warn_fn, limbo);
    transaction.body();

    CYG_REPORT_RETURN();
}

void
CdlConfigurationBody::change_package_version(CdlPackage package, std::string version,
                                             CdlDiagnosticFnPtr error_fn, CdlDiagnosticFnPtr warn_fn, bool limbo)
{
    CYG_REPORT_FUNCNAME("CdlConfiguration::change_package_version");
    CYG_REPORT_FUNCARG1XV(this);

    CdlLocalTransaction transaction(this);
    this->change_package_version(transaction.get(), package, version, error_fn, warn_fn, limbo);
    transaction.body();

    CYG_REPORT_RETURN();
}

void
CdlConfigurationBody::change_package_version(CdlTransaction transaction, std::string name, std::string new_version,
                                             CdlDiagnosticFnPtr error_fn, CdlDiagnosticFnPtr warn_fn, bool limbo)
{
    CYG_REPORT_FUNCNAME("CdlConfiguration::change_package_version");
    CYG_REPORT_FUNCARG2XV(this, transaction);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(transaction);
    CYG_PRECONDITIONC("" != name);

    CdlPackage package = 0;
    CdlNode node = this->lookup(name);
    if (0 != node) {
        package = dynamic_cast<CdlPackage>(node);
    }
    // For now it is illegal to change the version of package that has
    // not been loaded yet
    if (0 == package) {
        throw CdlInputOutputException(std::string("Cannot change version of \"") + name + "\" , this package is not loaded");
    }
    CYG_ASSERT_CLASSC(package);
    
    this->change_package_version(transaction, package, new_version, error_fn, warn_fn, limbo);

    CYG_REPORT_RETURN();
}

void
CdlConfigurationBody::add(std::string filename,
                          CdlDiagnosticFnPtr error_fn, CdlDiagnosticFnPtr warn_fn)
{
    CYG_REPORT_FUNCNAME("CdlConfiguration::add");
    CYG_REPORT_FUNCARG1XV(this);

    CdlLocalTransaction transaction(this);
    this->add(transaction.get(), filename, error_fn, warn_fn);
    transaction.body();

    CYG_REPORT_RETURN();
}

void
CdlConfigurationBody::set_template(std::string name, std::string version,
                                   CdlDiagnosticFnPtr error_fn, CdlDiagnosticFnPtr warn_fn, bool limbo)
{
    CYG_REPORT_FUNCNAME("CdlConfiguration::set_template");
    CYG_REPORT_FUNCARG1XV(this);

    CdlLocalTransaction transaction(this);
    this->set_template(transaction.get(), name, version, error_fn, warn_fn, limbo);
    transaction.body();

    CYG_REPORT_RETURN();
}

void
CdlConfigurationBody::set_template_file(std::string filename,
                                        CdlDiagnosticFnPtr error_fn, CdlDiagnosticFnPtr warn_fn, bool limbo)
{
    CYG_REPORT_FUNCNAME("CdlConfiguration::set_template_file");
    CYG_REPORT_FUNCARG1XV(this);

    CdlLocalTransaction transaction(this);
    this->set_template_file(transaction.get(), filename, error_fn, warn_fn, limbo);
    transaction.body();

    CYG_REPORT_RETURN();
}

void
CdlConfigurationBody::set_template(CdlTransaction transaction, std::string template_name, std::string version,
                                   CdlDiagnosticFnPtr error_fn, CdlDiagnosticFnPtr warn_fn, bool limbo)
{
    CYG_REPORT_FUNCNAME("CdlConfiguration::set_template");
    CYG_REPORT_FUNCARG2XV(this, transaction);

    // Some consistency checks before doing anything damaging
    if (!this->database->is_known_template(template_name)) {
        throw CdlInputOutputException("Unknown template " + template_name);
    }
    std::string template_filename = this->database->get_template_filename(template_name, version);
    if ("" == template_filename) {
        if ("" == version) {
            throw CdlInputOutputException("There is no template file corresponding to " + template_name);
        } else {
            throw CdlInputOutputException("There is no temmplate file corresponding to version "
                                          + version + " of " + template_name);
        }
    }

    // Now use set_template_file() to do the hard work.
    this->set_template_file(transaction, template_filename, error_fn, warn_fn, limbo);
    current_template = template_name;

    CYG_REPORT_RETURN();
}

void
CdlConfigurationBody::unload_template(bool limbo)
{
    CYG_REPORT_FUNCNAME("CdlConfiguration::unload_template");
    CYG_REPORT_FUNCARG1XV(this);

    CdlLocalTransaction transaction(this);
    this->unload_template(transaction.get(), limbo);
    transaction.body();

    CYG_REPORT_RETURN();
}

void
CdlConfigurationBody::set_hardware(std::string name,
                                   CdlDiagnosticFnPtr error_fn, CdlDiagnosticFnPtr warn_fn, bool limbo)
{
    CYG_REPORT_FUNCNAME("CdlConfiguration::set_hardware");
    CYG_REPORT_FUNCARG1XV(this);

    CdlLocalTransaction transaction(this);
    this->set_hardware(transaction.get(), name, error_fn, warn_fn, limbo);
    transaction.body();

    CYG_REPORT_RETURN();
}

void
CdlConfigurationBody::unload_hardware(bool limbo)
{
    CYG_REPORT_FUNCNAME("CdlConfiguration::unload_hardware");
    CYG_REPORT_FUNCARG1XV(this);

    CdlLocalTransaction transaction(this);
    this->unload_hardware(transaction.get(), limbo);
    transaction.body();

    CYG_REPORT_RETURN();
}

//}}}
//{{{  Load and unload - transaction support            

// ----------------------------------------------------------------------------
// A number of commit/cancel auxiliary classes are needed to allow the
// load/unload code to integrate properly with the transaction code.

class CdlConfiguration_CommitCancelLoad :
    public CdlTransactionCommitCancelOp
{
    friend class CdlTest;

  public:

    CdlConfiguration_CommitCancelLoad(CdlPackage package_arg)
        : CdlTransactionCommitCancelOp()
    {
        CYG_ASSERT_CLASSC(package_arg);
        package = package_arg;
    }
    ~CdlConfiguration_CommitCancelLoad()
    {
        package = 0;
    }
    void commit(CdlTransaction transaction)
    {
        CYG_ASSERT_CLASSC(package);
        CdlLoadableBody::transaction_commit_load(transaction, package);
        package = 0;
    }
    void cancel(CdlTransaction transaction)
    {
        CYG_ASSERT_CLASSC(package);
        CdlLoadableBody::transaction_cancel_load(transaction, package);
        package = 0;
    }
    
  protected:
    
  private:
    CdlConfiguration_CommitCancelLoad()
    {
    }
    CdlPackage  package;
};

class CdlConfiguration_CommitCancelUnload :
    public CdlTransactionCommitCancelOp
{
    friend class CdlTest;

  public:
    CdlConfiguration_CommitCancelUnload(CdlPackage package_arg)
        : CdlTransactionCommitCancelOp()
    {
        CYG_ASSERT_CLASSC(package_arg);
        package = package_arg;
    }
    ~CdlConfiguration_CommitCancelUnload()
    {
        package = 0;
    }
    void commit(CdlTransaction transaction)
    {
        CYG_PRECONDITION_CLASSC(package);
        CdlLoadableBody::transaction_commit_unload(transaction, package);
        package = 0;
    }
    void cancel(CdlTransaction transaction)
    {
        CYG_PRECONDITION_CLASSC(package);
        CdlLoadableBody::transaction_cancel_unload(transaction, package);
        package = 0;
    }

  protected:

  private:
    CdlConfiguration_CommitCancelUnload()
    {
    }
    CdlPackage package;
};

// These utility classes can be used to control the hardware and
// template names. If the transaction is cancelled the previous
// name gets re-installed
class CdlConfiguration_CommitCancelHardwareName :
    public CdlTransactionCommitCancelOp
{
    friend class CdlTest;
    
  public:
    CdlConfiguration_CommitCancelHardwareName(std::string old_name_arg)
        : CdlTransactionCommitCancelOp()
    {
        old_name = old_name_arg;
    }
    ~CdlConfiguration_CommitCancelHardwareName()
    {
        old_name = "";
    }
    void commit(CdlTransaction transaction)
    {
        // The new name is already installed, nothing more needs to happen.
        CYG_UNUSED_PARAM(CdlTransaction, transaction);
    }
    void cancel(CdlTransaction transaction)
    {
        // Restore the old name
        CdlToplevel toplevel = transaction->get_toplevel();
        CYG_ASSERTC(0 != toplevel);
        CdlConfiguration configuration = dynamic_cast<CdlConfiguration>(toplevel);
        CYG_ASSERT_CLASSC(configuration);

        configuration->set_hardware_name(old_name);
        CYG_UNUSED_PARAM(CdlTransaction, transaction);
    }
    
  protected:

  private:
    CdlConfiguration_CommitCancelHardwareName()
    {
    }
    std::string old_name;
};

class CdlConfiguration_CommitCancelTemplateName :
    public CdlTransactionCommitCancelOp
{
    friend class CdlTest;
    
  public:
    CdlConfiguration_CommitCancelTemplateName(std::string old_name_arg)
        : CdlTransactionCommitCancelOp()
    {
        old_name = old_name_arg;
    }
    ~CdlConfiguration_CommitCancelTemplateName()
    {
        old_name = "";
    }
    void commit(CdlTransaction transaction)
    {
        // The new name is already installed, nothing more needs to happen.
        CYG_UNUSED_PARAM(CdlTransaction, transaction);
    }
    void cancel(CdlTransaction transaction)
    {
        // Restore the old name
        CdlToplevel toplevel = transaction->get_toplevel();
        CYG_ASSERTC(0 != toplevel);
        CdlConfiguration configuration = dynamic_cast<CdlConfiguration>(toplevel);
        CYG_ASSERT_CLASSC(configuration);

        configuration->set_template_name(old_name);
        CYG_UNUSED_PARAM(CdlTransaction, transaction);
    }
    
  protected:

  private:
    CdlConfiguration_CommitCancelTemplateName()
    {
    }
    std::string old_name;
};

//}}}
//{{{  CdlConfiguration::load_package()                 

// ----------------------------------------------------------------------------
// Loading a package into the current level. This involves the following
// stages.
//
//  1) check that the specified package name and version is valid, by
//     comparing it with the database. When the database was created there
//     will have been checks to make sure that the initial CDL script was
//     present, but more checks can be done here.
//
//  2) before allocating any resources, check that there is no name conflict
//     for the package itself.
//
//  3) create the package object, and add it to the toplevel of the current
//     configuration. It may get reparented later on. Part of the creation
//     process is to allocate a new slave interpreter, which can be updated
//     with various bits of information.
//
//  4) evaluate the toplevel script. Subsidiary component scripts will
//     get evaluated as a side effect. The various nodes will be added
//     to the hierarchy as they are created, but no property binding
//     happens yet.
//
//     Any failure up to this point should result in the entire package
//     being removed from the hierarchy and then destroyed, thus leaving
//     the configuration in its original state.
//
//  5) now property binding needs to take place. This can have lots
//     of side effects, e.g. default values may get calculated, the
//     hierarchy may change because of parent properties, etc.
//     The work is done inside CdlLoadable::bind() which will undo
//     everything on failure - although bad_alloc is the only
//     failure that should occur.
//
//  6) load operations can get cancelled, so a suitable commit/cancel
//     operation needs to allocated and added to the transaction.
//
//  7) if limbo is enabled, previous values should be extracted from
//     limbo if at all possible. In addition the package's value can
//     be set to its version.

void
CdlConfigurationBody::load_package(CdlTransaction transaction, std::string name, std::string version,
                                   CdlDiagnosticFnPtr error_fn, CdlDiagnosticFnPtr warn_fn, bool limbo)
{
    CYG_REPORT_FUNCNAME("CdlConfiguration::load_package");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(transaction);
    CYG_PRECONDITIONC("" != name);

    // Locate the database entry. Also check the version (filling it in if necessary).
    // Get hold of the package directory and the initial script.
    if (!database->is_known_package(name)) {
        throw CdlInputOutputException("Unknown package " + name);
    }
    const std::vector<std::string>& versions = database->get_package_versions(name);
    if ("" == version) {
        version = *(versions.begin());
    } else {
        if (std::find(versions.begin(), versions.end(), version) == versions.end()) {
            throw CdlInputOutputException("Package " + name + " does not have an installed version `" + version + "'.");
        }
    }
    std::string repository      = database->get_package_repository(name, version);
    std::string directory       = database->get_package_directory(name);
    std::string script          = database->get_package_script(name);
    CYG_ASSERTC(("" != directory) && ("" != script));

    // Check that the directory actually exists. For this the configuration's own
    // interpreter can be used.
    CdlInterpreter interp = get_interpreter();
    CYG_ASSERT_CLASSC(interp);

    std::string tcl_cmd = "regsub -all -- {\\\\} [file join " + directory + " " + version + "] / result; return $result";
    std::string tcl_result;
    if (TCL_OK != interp->eval(tcl_cmd, tcl_result)) {
        throw CdlInputOutputException("Cannot load package `" + name + "', internal error constructing pathname.");
    }
    directory = tcl_result;
    
    tcl_cmd   = "file isdirectory [file join \"" + repository + "\" " + directory + "]";
    if ((TCL_OK != interp->eval(tcl_cmd, tcl_result)) || ("1" != tcl_result)) {
        throw CdlInputOutputException("Cannot load package `" + name + "', there is no directory `" + directory + "'.");
    }
    
    // Make sure that there is no name conflict. No resources have been allocated
    // yet, so this is a good time.
    CdlNode node = lookup(name);
    if (0 != node) {
        if (0 != dynamic_cast<CdlPackage>(node)) {
            throw CdlInputOutputException("Package `" + name + "' is already loaded.");
        } else {

            std::string msg = "Name clash for package `" + name + "',there is a `" +
                node->get_class_name() + " " + name + "' already loaded";
            CdlLoadable owner_pkg = node->get_owner();
            if (0 != owner_pkg) {
                msg += " in package " + owner_pkg->get_name();
            }
            throw CdlInputOutputException(msg);
        }
    }

    // Now create the package object itself.
    CdlPackage package  = 0;
    bool       bound    = false;
    CdlConfiguration_CommitCancelLoad* load_op  = 0;
    
    try {
        package = new CdlPackageBody(name, this, repository, directory);

        // The package should be added to the hierarchy immediately.
        // All nodes will get added to the hierarchy as they are
        // created, an operation that has to be undone during
        // failure. 
        this->add_node(package, this, package);
        
        // Load the package data. The various nodes will all end up
        // in a hierarchy below the package, but without any checks
        // for name conflicts etc and ignoring any re-parenting.
        CdlInterpreter interp = package->get_interpreter();
        CYG_ASSERT_CLASSC(interp);

        interp->add_command("unknown",  &CdlParse::unknown_command);
        CdlInterpreterBody::DiagSupport diag_support(interp, error_fn, warn_fn);
        
        // Next figure out the script name, and make sure that it exists.
        std::string actual_script = package->find_absolute_file(script, "cdl");
        if ("" == actual_script) {
            throw CdlInputOutputException("Package " + name + ", unable to find initial script " + script);
        }
        tcl_cmd = "file isfile \"" + actual_script + "\"";
        if ((TCL_OK != interp->eval(tcl_cmd, tcl_result)) || ("1" != tcl_result)) {
            throw CdlInputOutputException("Package " + name + ", " + actual_script + " is not a CDL script");
        }

        // The script is valid. Set up the interpreter appropriately.
        CdlParse::clear_error_count(interp);
        static CdlInterpreterCommandEntry commands[] =
        {
            CdlInterpreterCommandEntry("cdl_package",    &CdlPackageBody::parse_package     ),
            CdlInterpreterCommandEntry("cdl_component",  &CdlComponentBody::parse_component ),
            CdlInterpreterCommandEntry("cdl_option",     &CdlOptionBody::parse_option       ),
            CdlInterpreterCommandEntry("cdl_interface",  &CdlInterfaceBody::parse_interface ),
            CdlInterpreterCommandEntry("cdl_dialog",     &CdlDialogBody::parse_dialog       ),
            CdlInterpreterCommandEntry("cdl_wizard",     &CdlWizardBody::parse_wizard       ),
            CdlInterpreterCommandEntry("",               0                                  )
        };
        CdlInterpreterBody::CommandSupport   interp_cmds(interp, commands);
        CdlInterpreterBody::ContainerSupport interp_container(interp, package);
        CdlInterpreterBody::ContextSupport   interp_context(interp, actual_script);
        
        // The interpreter is now ready.
        (void) interp->eval_file(actual_script);

        // Clean out the commands etc. This interpreter may get used again
        // in future, and it should not be possible to define new options
        // etc. in that invocation.
        interp->remove_command("unknown");
        
        // All the data has been read in without generating an
        // exception. However there may have been errors reported via
        // the parse_error_fn, and any errors at all should result
        // in an exception.
        int error_count = CdlParse::get_error_count(interp);
        if (error_count > 0) {
            std::string tmp;
            Cdl::integer_to_string(error_count, tmp);
            throw CdlParseException("Package " + name + ", " + tmp + " error" +
                                    ((error_count > 1) ? "s" : "") +
                                    " occurred while reading in the CDL data.");
        }

        // All the data has been read in, implying that there are no
        // fatal problems with the data. Now try to bind all
        // references to and from this loadable.
        package->bind(transaction);
        bound = true;

        // Finally, create a suitable transaction commit/cancel object
        // and add it to the transaction.
        load_op = new CdlConfiguration_CommitCancelLoad(package);
        transaction->add_commit_cancel_op(load_op);
        
    } catch (...) {

        // Something went wrong during the create or load. It is necessary
        // to delete the package. Undo all the operations above, in
        // reverse order. The add_commit_cancel_op() was the last step,
        // so need not be undone here.
        if (0 != load_op) {
            delete load_op;
        }
        
        if (0 != package) {
            // Note: no attempt is made to recover from errors here
            if (bound) {
                package->unbind(transaction);
            }
            this->remove_loadable_from_toplevel(package);
            delete package;
        } 
        throw;
    }

    // FIXME: implement limbo support
    
    // We also have a sensible value for the package as a whole.
    // Use this value for both default and user - after all the
    // user has selected the package.
    package->enable_and_set_value(transaction, version, CdlValueSource_Default);
    package->enable_and_set_value(transaction, version, CdlValueSource_User);

    CYG_REPORT_RETURN();
}

//}}}
//{{{  CdlConfiguration::unload_package()               

// ----------------------------------------------------------------------------
// Unloading a package is very simple. If requested, save all current
// values to limbo: there is no point in saving default values, these
// will get recalculated from the default_value property anyway;
// inferred values should be saved, there is no guarantee that the exact
// same value will be calculated again, and if the inferred value is no
// longer correct then the inference engine can freely update it.
//
// Next, unbind the package and remove it from the hierarchy. These
// operations are reversible if the transaction gets cancelled.
// A suitable transaction commit/cancel object is created and
// added to the transaction.
void
CdlConfigurationBody::unload_package(CdlTransaction transaction, CdlPackage package, bool limbo)
{
    CYG_REPORT_FUNCNAME("CdlConfiguration::unload_package");
    CYG_REPORT_FUNCARG4XV(this, transaction, package, limbo);
    CYG_INVARIANT_THISC(CdlConfigurationBody);
    CYG_INVARIANT_CLASSC(CdlTransactionBody, transaction);
    CYG_PRECONDITION_CLASSC(package);

    if (limbo) {
        const std::vector<CdlNode>& pkg_contents = package->get_owned();
        std::vector<CdlNode>::const_iterator node_i;

        for (node_i = pkg_contents.begin(); node_i != pkg_contents.end(); node_i++) {
            CdlValuable valuable = dynamic_cast<CdlValuable>(*node_i);
            if (0 != valuable) {
                if (valuable->has_source(CdlValueSource_Inferred) ||
                    valuable->has_source(CdlValueSource_Wizard)   ||
                    valuable->has_source(CdlValueSource_User)) {
                    
                    set_limbo_value(valuable);
                }
            }
        }
    }

    bool unbound = false;
    bool removed = false;
    CdlConfiguration_CommitCancelUnload* unload_op = 0;
    try {
        
        package->unbind(transaction);
        unbound = true;
        this->remove_loadable_from_toplevel(package);
        removed = true;
        unload_op = new CdlConfiguration_CommitCancelUnload(package);
        transaction->add_commit_cancel_op(unload_op);
        
    } catch(...) {
        if (0 != unload_op) {
            delete unload_op;
        }
        if (removed) {
            this->add_loadable_to_toplevel(package);
        }
        if (unbound) {
            package->bind(transaction);
        }
        throw;
    }
    
    CYG_REPORT_RETURN();
}

//}}}
//{{{  CdlConfiguration::change_package_version()       

// ----------------------------------------------------------------------------
// Changing a package version is just a case of unloading the old version
// and then loading in the new version. Because this all happens in the
// context of a transaction it is possible to undo the unload on
// failure, and the whole transaction can be cancelled at a higher level.

void
CdlConfigurationBody::change_package_version(CdlTransaction transaction, CdlPackage package, std::string new_version,
                                             CdlDiagnosticFnPtr error_fn, CdlDiagnosticFnPtr warn_fn, bool limbo)
{
    CYG_REPORT_FUNCNAME("CdlConfiguration::change_package_version");
    CYG_REPORT_FUNCARG3XV(this, transaction, package);
    CYG_PRECONDITION_THISC();
    CYG_INVARIANT_CLASSC(CdlTransactionBody, transaction);
    CYG_PRECONDITION_CLASSC(package);
    // "" is valid for the version, it indicates the default

    // Since the package is already loaded it must be in the database,
    // but it is possible that the desired version does not exist.
    std::string name = package->get_name();
    const std::vector<std::string>& pkg_versions = database->get_package_versions(name);
    if ("" == new_version) {
        new_version = *(pkg_versions.begin());
    } else if (std::find(pkg_versions.begin(), pkg_versions.end(), new_version) == pkg_versions.end()) {
        throw CdlInputOutputException("Version " + new_version + " of package " + name + " is not installed.");
    }

    bool unloaded = false;
    try {
        this->unload_package(transaction, package, limbo);
        unloaded = true;
        this->load_package(transaction, name, new_version, error_fn, warn_fn, limbo);
    } catch(...) {
        if (unloaded) {
            // There should be a commit/cancel op for the unload package step.
            // This can be undone.
            CdlTransactionCommitCancelOp* unload_op = transaction->get_last_commit_cancel_op();
            CYG_ASSERTC(0 != unload_op);
            CYG_ASSERTC(0 != dynamic_cast<CdlConfiguration_CommitCancelUnload*>(unload_op));
            transaction->cancel_last_commit_cancel_op();
            CYG_UNUSED_PARAM(CdlTransactionCommitCancelOp*, unload_op);
        }
        throw;
    }

    CYG_REPORT_RETURN();
}

//}}}
//{{{  CdlConfiguration::set_hardware() etc.            

// ----------------------------------------------------------------------------
// Setting the hardware involves unloading the old hardware, if any, and
// then loading in the new one. Obviously this should only happen if
// the new hardware name is valid. It would be possible to optimise for
// the case where the old and new hardware are the same, subject
// to dynamic database reload support.

void
CdlConfigurationBody::set_hardware(CdlTransaction transaction, std::string target_name,
                                   CdlDiagnosticFnPtr error_fn, CdlDiagnosticFnPtr warn_fn, bool limbo)
{
    CYG_REPORT_FUNCNAME("CdlConfiguration::set_hardware");
    CYG_REPORT_FUNCARG2XV(this, transaction);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(transaction);

    // Minimal consistency check before attempting anything complicated.
    if (!database->is_known_target(target_name)) {
        throw CdlInputOutputException("Unknown target " + target_name);
    }

    CdlInterpreter interp = this->get_interpreter();
    CdlInterpreterBody::DiagSupport    diag_support(interp, error_fn, warn_fn);
    CdlInterpreterBody::ContextSupport context_support(interp, "Hardware selection");
    
    CdlConfiguration_CommitCancelHardwareName* rename_op = new CdlConfiguration_CommitCancelHardwareName(current_hardware);
    try {
        transaction->add_commit_cancel_op(rename_op);
        const std::vector<CdlLoadable>& loadables = this->get_loadables();
        int i;
        for (i = (int) loadables.size() - 1; i >= 0; i--) {
            CdlPackage package = dynamic_cast<CdlPackage>(loadables[i]);
            if ((0 != package) && package->belongs_to_hardware()) {
                this->unload_package(transaction, package, limbo);
            }
        }
        current_hardware = "";

        if ("" != target_name) {
            
            const std::vector<std::string>& packages = database->get_target_packages(target_name);
            std::vector<std::string>::const_iterator    name_i;
            for (name_i = packages.begin(); name_i != packages.end(); name_i++) {
                // Target specifications may refer to packages that are not
                // installed. This is useful in e.g. an anoncvs environment.
                if (database->is_known_package(*name_i)) {
                    // It is possible for a hardware package to have been
                    // loaded separately, in which case there is no point in
                    // loading it again.
                    if (0 == this->lookup(*name_i)) {
                        this->load_package(transaction, *name_i, "",
                                           error_fn, warn_fn, limbo);
                        CdlPackage package = dynamic_cast<CdlPackage>(this->lookup(*name_i));
                        CYG_LOOP_INVARIANT_CLASSC(package);
                        package->loaded_for_hardware = true;
                    }
                } else {
                    CdlParse::report_warning(interp, "",
                                             std::string("The target specification lists a package `") + *name_i +
                                             "' which is not present in the component repository.");
                }
            }
        }
        current_hardware = target_name;
        
    } catch(...) {
        // Cancel all operations up to and including the rename_op
        CdlTransactionCommitCancelOp* cancel_op = 0;
        do {
            cancel_op = transaction->get_last_commit_cancel_op();
            CYG_LOOP_INVARIANTC(0 != cancel_op);
            transaction->cancel_last_commit_cancel_op();
        } while(cancel_op != rename_op);
        throw;
    }

    // There may have been enables/disables and value data for that target
    // FIXME: any problems get ignored quietly. There should at least
    // be some warnings.
    if ("" != target_name) {
        const std::vector<std::string>& enables  = database->get_target_enables(target_name);
        const std::vector<std::string>& disables = database->get_target_disables(target_name);
        const std::vector<std::pair<std::string, std::string> >& set_values = database->get_target_set_values(target_name);
        
        if ((0 != enables.size()) || (0 != disables.size()) || (0 != set_values.size())) {
            std::vector<std::string>::const_iterator opt_i;
            CdlNode     node;
            CdlValuable valuable;
            CdlValueFlavor flavor;
            
            for (opt_i = enables.begin(); opt_i != enables.end(); opt_i++) {
                valuable = 0;
                node     = this->lookup(*opt_i);
                if (0 != node) {
                    valuable = dynamic_cast<CdlValuable>(node);
                    if (0 != valuable) {
                    }
                }
                if (0 != valuable) {
                    flavor = valuable->get_flavor();
                    if ((CdlValueFlavor_Bool == flavor) || (CdlValueFlavor_BoolData == flavor)) {
                        valuable->enable(transaction, CdlValueSource_User);
                    } else {
                        CdlParse::report_warning(interp, std::string("target `") + target_name + "'",
                                                 std::string("The option `") + *opt_i +
                                                 "' is supposed to be enabled for this target.\n" +
                                                 "However the option does not have a bool or booldata flavors.");
                    }
                } else {
                    CdlParse::report_warning(interp, std::string("target `") + target_name + "'",
                                             std::string("The option `") + *opt_i +
                                             "' is supposed to be enabled for this target.\n" +
                                             "However this option is not in the current configuration.");
                }
            }
            for (opt_i = disables.begin(); opt_i != disables.end(); opt_i++) {
                valuable = 0;
                node = this->lookup(*opt_i);
                if (0 != node) {
                    valuable = dynamic_cast<CdlValuable>(node);
                }
                if (0 != valuable) {
                    flavor = valuable->get_flavor();
                    if ((CdlValueFlavor_Bool == flavor) || (CdlValueFlavor_BoolData == flavor)) {
                        valuable->disable(transaction, CdlValueSource_User);
                    } else {
                        CdlParse::report_warning(interp, std::string("target `") + target_name + "'",
                                                 std::string("The option `") + *opt_i +
                                                 "' is supposed to be disabled for this target.\n" +
                                                 "However the option does not have a bool or booldata flavors.");
                    }
                } else {
                    CdlParse::report_warning(interp, std::string("target `") + target_name + "'",
                                             std::string("The option `") + *opt_i +
                                             "' is supposed to be disabled for this target.\n" +
                                             "However this option is not in the current configuration.");
                }
            }
            std::vector<std::pair<std::string,std::string> >::const_iterator value_i;
            for (value_i = set_values.begin(); value_i != set_values.end(); value_i++) {
                valuable = 0;
                node = this->lookup(value_i->first);
                if (0 != node) {
                    valuable = dynamic_cast<CdlValuable>(node);
                }
                if (0 != valuable) {
                    flavor = valuable->get_flavor();
                    if ((CdlValueFlavor_BoolData == flavor) || (CdlValueFlavor_Data == flavor)) {
                        valuable->set_value(transaction, value_i->second, CdlValueSource_User);
                    } else {
                        CdlParse::report_warning(interp, std::string("target `") + target_name + "'",
                                                 std::string("The option `") + *opt_i +
                                                 "' is supposed to be given the value `" + value_i->second +
                                                 "' for this target.\n" +
                                                 "However the option does not have a data or booldata flavor.");
                    }
                } else {
                    CdlParse::report_warning(interp, std::string("target `") + target_name + "'",
                                             std::string("The option `") + *opt_i +
                                             "' is supposed to be given the value `" + value_i->second +
                                             "' for this target.\n" +
                                             "However this option is not in the current configuration.");
                }
            }
        }
    }

    CYG_REPORT_RETURN();
}

void
CdlConfigurationBody::unload_hardware(CdlTransaction transaction, bool limbo)
{
    CYG_REPORT_FUNCNAME("CdlConfiguration::unload_hardware");
    CYG_REPORT_FUNCARG3XV(this, transaction, limbo);
    CYG_PRECONDITION_THISC();

    CdlConfiguration_CommitCancelHardwareName* rename_op = new CdlConfiguration_CommitCancelHardwareName(current_hardware);
    try {
        transaction->add_commit_cancel_op(rename_op);
    } catch(...) {
        delete rename_op;
        throw;
    }
    current_hardware = "";

    try {
        const std::vector<CdlLoadable>& loadables = this->get_loadables();
        for (int i = (int) loadables.size() - 1; i >= 0; i--) {
            CdlPackage package = dynamic_cast<CdlPackage>(loadables[i]);
            if ((0 != package) && package->belongs_to_hardware()) {
                this->unload_package(transaction, package, limbo);
            }
        }
    } catch(...) {
        CdlTransactionCommitCancelOp* cancel_op = 0;
        do {
            cancel_op = transaction->get_last_commit_cancel_op();
            CYG_LOOP_INVARIANTC(0 != cancel_op);
            transaction->cancel_last_commit_cancel_op();
        } while(cancel_op != rename_op);
        throw;
    }

    CYG_REPORT_RETURN();
}

//}}}
//{{{  CdlConfiguration::set_template() etc             

// ----------------------------------------------------------------------------
void
CdlConfigurationBody::set_template_file(CdlTransaction transaction, std::string filename,
                                        CdlDiagnosticFnPtr error_fn, CdlDiagnosticFnPtr warn_fn, bool limbo)
{
    CYG_REPORT_FUNCNAME("CdlConfiguration::set_template_file");
    CYG_REPORT_FUNCARG3XV(this, transaction, limbo);
    CYG_PRECONDITION_THISC();

    int i;
    CdlConfiguration_CommitCancelTemplateName* rename_op = new CdlConfiguration_CommitCancelTemplateName(current_template);

    // The hard work is done by add(), which loads in a partial savefile.
    // This can have undesirable side effects: changing the name,
    // description, or hardware settings. It must be possible to undo
    // these.
    std::string saved_name        = this->get_name();
    std::string saved_description = this->get_description();
    std::string saved_hardware    = this->get_hardware();

    // New packages will end up at the end of the loadables vector.
    // Each new package needs to be registered as a template one.
    // NOTE: this may break if we start doing more interesting things
    // with savefiles.
    
    try {
        transaction->add_commit_cancel_op(rename_op);
        const std::vector<CdlLoadable>& loadables = this->get_loadables();
        unsigned int load_i;
        for (i = (int) loadables.size() - 1; i >= 0; i--) {
            CdlPackage package = dynamic_cast<CdlPackage>(loadables[i]);
            if ((0 != package) && package->belongs_to_template()) {
                this->unload_package(transaction, package, limbo);
            }
        }
        current_template = "";
        load_i = loadables.size();
        
        this->add(transaction, filename, error_fn, warn_fn);
        this->current_template = filename;
        this->set_name(saved_name);
        this->description = saved_description;
        this->current_hardware = saved_hardware;

        for ( ; load_i < loadables.size(); load_i++) {
            CdlPackage pkg = dynamic_cast<CdlPackage>(loadables[load_i]);
            CYG_ASSERT_CLASSC(pkg);
            pkg->loaded_for_template = true;
        }
        
    } catch(...) {
        
        this->set_name(saved_name);
        this->description = saved_description;
        this->current_hardware = saved_hardware;
        
        // Cancel all operations up to and including the rename_op
        CdlTransactionCommitCancelOp* cancel_op = 0;
        do {
            cancel_op = transaction->get_last_commit_cancel_op();
            CYG_LOOP_INVARIANTC(0 != cancel_op);
            transaction->cancel_last_commit_cancel_op();
        } while(cancel_op != rename_op);
        throw;
    }
    

    CYG_REPORT_RETURN();
}

void
CdlConfigurationBody::unload_template(CdlTransaction transaction, bool limbo)
{
    CYG_REPORT_FUNCNAME("CdlConfiguration::unload_template");
    CYG_REPORT_FUNCARG2XV(this, transaction);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(transaction);

    CdlConfiguration_CommitCancelTemplateName* rename_op = new CdlConfiguration_CommitCancelTemplateName(current_template);
    try {
        transaction->add_commit_cancel_op(rename_op);
    } catch(...) {
        delete rename_op;
        throw;
    }
    current_template = "";

    try {
        const std::vector<CdlLoadable>& loadables = this->get_loadables();
        for (int i = (int) loadables.size() - 1; i >= 0; i--) {
            CdlPackage package = dynamic_cast<CdlPackage>(loadables[i]);
            if ((0 != package) && package->belongs_to_template()) {
                this->unload_package(transaction, package, limbo);
            }
        }
    } catch(...) {
        CdlTransactionCommitCancelOp* cancel_op = 0;
        do {
            cancel_op = transaction->get_last_commit_cancel_op();
            CYG_LOOP_INVARIANTC(0 != cancel_op);
            transaction->cancel_last_commit_cancel_op();
        } while(cancel_op != rename_op);
        throw;
    }
    
    CYG_REPORT_RETURN();
}

//}}}
//{{{  Persistence support                              

//{{{  initialize_savefile_support()                    

// ----------------------------------------------------------------------------
// Initialization. The purpose of this code is to determine all the
// commands that can end up in a particular savefile. This includes
// the cdl_configuration command, commands relevant to packages,
// options, and components, the generic library commands, and
// application-specific commands.
//
// This is a virtual function, it may get invoked indirectly from
// e.g. CdlToplevel::add_savefile_command().

void
CdlConfigurationBody::initialize_savefile_support()
{
    CYG_REPORT_FUNCNAME("CdlConfiguration::initialize_savefile_support");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();
    
    // Start with the generic stuff such as cdl_savefile_version and
    // cdl_command.
    this->CdlToplevelBody::initialize_savefile_support();

    // Now add in the cdl_configuration command and its subcommands.
    this->add_savefile_command("cdl_configuration", 0, &savefile_configuration_command);
    this->add_savefile_subcommand("cdl_configuration", "description", 0, &savefile_description_command);
    this->add_savefile_subcommand("cdl_configuration", "hardware",    0, &savefile_hardware_command);
    this->add_savefile_subcommand("cdl_configuration", "template",    0, &savefile_template_command);
    this->add_savefile_subcommand("cdl_configuration", "package",     0, &savefile_package_command);

    CdlPackageBody::initialize_savefile_support(this);
    CdlComponentBody::initialize_savefile_support(this);
    CdlOptionBody::initialize_savefile_support(this);
    CdlInterfaceBody::initialize_savefile_support(this);
}

//}}}
//{{{  CdlConfiguration::save() - internal              

// ----------------------------------------------------------------------------
// The exported interface is CdlConfiguration::save(). This takes a single
// argument, a filename. It opens the file, and then invokes various
// functions that output the relevants bits of the file.
//
// This member function is responsible for outputting a cdl_configuration
// command.

void
CdlConfigurationBody::save(CdlInterpreter interp, Tcl_Channel chan, int indentation, bool minimal)
{
    CYG_REPORT_FUNCNAME("CdlConfiguration::save");
    CYG_REPORT_FUNCARG5XV(this, interp, chan, indentation, minimal);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(interp);
    CYG_PRECONDITIONC(0 == indentation);

    std::string text = "";
    if (!minimal) {
      text = 
"# This section defines the toplevel configuration object. The only\n\
# values that can be changed are the name of the configuration and\n\
# the description field. It is not possible to modify the target,\n\
# the template or the set of packages simply by editing the lines\n\
# below because these changes have wide-ranging effects. Instead\n\
# the appropriate tools should be used to make such modifications.\n\
\n";
    }

    text += "cdl_configuration " + CdlInterpreterBody::quote(this->get_name()) + " {\n";

    std::string config_data = this->get_description();
    if (!minimal || ("" != text)) {
        text += "    description " + CdlInterpreterBody::quote(config_data) + " ;\n";
    }

    // Repeat the warning.
    if (!minimal) {
        text += "\n    # These fields should not be modified.\n";
    }
    config_data = this->get_hardware();
    if ("" != config_data) {
        text += "    hardware    " + CdlInterpreterBody::quote(config_data) + " ;\n";
    }
    config_data = this->get_template();
    if ("" != config_data) {
        text += "    template    " + CdlInterpreterBody::quote(config_data) + " ;\n";
    }
    std::vector<CdlLoadable>::const_iterator load_i;
    const std::vector<CdlLoadable>& packages = get_loadables();
    for (load_i = packages.begin(); load_i != packages.end(); load_i++) {
        CdlPackage pkg = dynamic_cast<CdlPackage>(*load_i);
        CYG_ASSERT_CLASSC(pkg);
        text += "    package ";
        if (pkg->belongs_to_template()) {
            text += "-template ";
        }
        if (pkg->belongs_to_hardware()) {
            text += "-hardware ";
        }
        text += CdlInterpreterBody::quote(pkg->get_name()) + " " + CdlInterpreterBody::quote(pkg->get_value()) + " ;\n";
    }

    interp->write_data(chan, text);
    
    // If the package was loaded from a file then there may be additional
    // data associated with the configuration that is not currently
    // recognised. This call preserves that data.
    this->CdlNodeBody::save(interp, chan, indentation + 4, minimal);

    interp->write_data(chan, "};\n\n");

    CYG_REPORT_RETURN();
}

//}}}
//{{{  CdlConfiguration::save() - exported interface    

// ----------------------------------------------------------------------------
// This is the exported interface for saving a configuration. The specified
// file is opened via the appropriate Tcl library routines, and then the
// relevant member functions are invoked to output the actual configuration
// date.

void
CdlConfigurationBody::save(std::string filename, bool minimal)
{
    CYG_REPORT_FUNCNAME("CdlConfiguration::save");
    CYG_REPORT_FUNCARG2XV(this, minimal);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITIONC("" != filename);

    // Make sure that the savefile support is properly initialized.
    // This happens during the first save or load operation, or when
    // the application starts to register its own savefile extensions.
    if (!CdlToplevelBody::savefile_support_initialized()) {
        this->initialize_savefile_support();
    }

    // A Tcl interpreter is needed for the call to OpenFileChannel(),
    // and will also be passed to the individual save functions.
    CdlInterpreter interp = this->get_interpreter();
    CYG_ASSERT_CLASSC(interp);

    // Do not worry about forward vs. backward slashes, since the filename
    // is not manipulated in any way. Instead just pass it to Tcl.
    Tcl_Channel chan = Tcl_OpenFileChannel(interp->get_tcl_interpreter(), const_cast<char*>(filename.c_str()), "w", 0666);
    if (0 == chan) {
        throw CdlInputOutputException("Unable to open file " + filename + "\n" + interp->get_result());
    }

    // The channel may end up being registered in various different
    // interpreters, so Tcl_Close() is not the right way to close down
    // the channel. Instead Tcl_RegisterChannel() should be used here
    // to provide reference counting semantics.
    Tcl_RegisterChannel(0, chan);

    // A try/catch body is needed here to make sure that the file gets
    // properly cleaned up.
    std::string tmp;
    try {

        if (!minimal) {
            interp->write_data(chan, "# eCos saved configuration\n\n");
        }
        
        CdlToplevelBody::save_separator(interp, chan, "commands", minimal);
        this->CdlToplevelBody::save_command_details(interp, chan, 0, minimal);
        CdlToplevelBody::save_separator(interp, chan, "toplevel", minimal);
        this->save(interp, chan, 0, minimal);
        CdlToplevelBody::save_separator(interp, chan, "conflicts", minimal);
        this->CdlToplevelBody::save_conflicts(interp, chan, 0, minimal);
        CdlToplevelBody::save_separator(interp, chan, "contents", minimal);
        this->CdlContainerBody::save(interp, chan, 0, minimal);
        this->save_unsupported_commands(interp, chan, 0, minimal);
        
    } catch(...) {
        Tcl_UnregisterChannel(0, chan);
        // NOTE: deleting the file is necessary, it is a bad idea to
        // end up with incomplete save files. It would be even better
        // to write to a temporary file and only overwrite the old
        // savefile on success.
        //
        // Tcl does not provide direct access to the file delete
        // facility, so it is necessary to evaluate a script. This
        // introduces quoting and security problems, since the
        // filename might contain spaces, square brackets, braces...
        // To avoid these problems a variable is used.
        interp->set_variable("__cdlconfig_filename", filename);
        interp->eval("file delete $__cdlconfig_filename", tmp);
        interp->unset_variable("__cdlconfig_filename");
        
        throw;
    }

    // This call will perform the appropriate close.
    Tcl_UnregisterChannel(0, chan);
}

//}}}
//{{{  CdlConfiguration::load() and add()               

// ----------------------------------------------------------------------------
// Most of the work is done in add(). load() simply creates a new configuration
// and then invokes add().
CdlConfiguration
CdlConfigurationBody::load(std::string filename, CdlPackagesDatabase db, CdlInterpreter interp,
                           CdlDiagnosticFnPtr error_fn, CdlDiagnosticFnPtr warn_fn)
{
    CYG_REPORT_FUNCNAMETYPE("CdlConfiguration::load", "result %p");
    CYG_REPORT_FUNCARG4XV(db, interp, error_fn, warn_fn);
    CYG_PRECONDITION_CLASSC(db);
    CYG_PRECONDITION_CLASSC(interp);
    
    CdlConfiguration result = CdlConfigurationBody::make("eCos", db, interp);
    if (0 == result) {
        CYG_REPORT_RETVAL(result);
        return result;
    }
    
    try {
        result->add(filename, error_fn, warn_fn);
        result->save_file = filename;
    } catch(...) {
        delete result;
        throw;
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

// ----------------------------------------------------------------------------
void
CdlConfigurationBody::add(CdlTransaction transaction, std::string filename,
                          CdlDiagnosticFnPtr error_fn, CdlDiagnosticFnPtr warn_fn)
{
    CYG_REPORT_FUNCNAME("CdlConfiguration::add");
    CYG_REPORT_FUNCARG3XV(this, error_fn, warn_fn);
    CYG_PRECONDITION_THISC();

    // Initialize the savefile support, so that it is known what
    // commands can occur in a savefile.
    if (!CdlToplevelBody::savefile_support_initialized()) {
        this->initialize_savefile_support();
    }
    
    // The interpreter should not have any left-over junk.
    CdlInterpreter interp = this->get_interpreter();
    CYG_PRECONDITION_CLASSC(interp);
    CYG_ASSERTC(0 == interp->get_loadable());
    CYG_ASSERTC(0 == interp->get_container());
    CYG_ASSERTC(0 == interp->get_node());
    CYG_ASSERTC(0 == interp->get_transaction());

    // Keep track of enough information to undo all the changes.
    CdlParse::clear_error_count(interp);
    CdlInterpreterBody::DiagSupport    diag_support(interp, error_fn, warn_fn);
    CdlInterpreterBody::ContextSupport context_support(interp, filename);

    try {
        interp->set_transaction(transaction);

        std::vector<CdlInterpreterCommandEntry> commands;
        this->get_savefile_commands(commands);
        CdlInterpreterBody::CommandSupport interp_cmds(interp, commands);

        interp->eval_file(filename);
        
        // All the data has been read in without generating an
        // exception. However there may have been errors reported via
        // the error_fn handling, and any errors at all should result
        // in an exception.
        int error_count = CdlParse::get_error_count(interp);
        if (error_count > 0) {
            std::string tmp;
            Cdl::integer_to_string(error_count, tmp);
            throw CdlInputOutputException("Invalid savefile \"" + filename + "\".\n" +
                                          tmp + " error" + ((error_count > 1) ? "s" : "") +
                                          " occurred while reading in the savefile data.");
        }
        
    } catch(...) {
        interp->set_transaction(0);
        throw;
    }

    interp->set_transaction(0);

    CYG_REPORT_RETURN();
}

//}}}
//{{{  savefile commands                                

// ----------------------------------------------------------------------------
// A cdl_configuration command does not actually do very much. It acts as
// a container for subcommands, and it can be used to change the name.
//
// The command could also check that the current configuration is empty.
// This is not done, to allow multiple savefiles to be loaded into
// a single configuration in future.
int
CdlConfigurationBody::savefile_configuration_command(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("CdlConfiguration::savefile_configuration_command", "result %d");
    CYG_PRECONDITION_CLASSC(interp);

    int result = TCL_OK;
    CdlToplevel toplevel = interp->get_toplevel();
    CYG_ASSERT_CLASSC(toplevel);
    CdlConfiguration config = dynamic_cast<CdlConfiguration>(toplevel);
    CYG_ASSERT_CLASSC(config);

    std::vector<CdlInterpreterCommandEntry> subcommands;
    std::vector<CdlInterpreterCommandEntry>* toplevel_commands = 0;
    
    try {
        std::vector<std::pair<std::string,std::string> > options;
        int data_index = CdlParse::parse_options(interp, "cdl_configuration command", 0, argc, argv, 1, options);

        // A broken cdl_configuration command is pretty fatal, chances are
        // that the entire load is going to fail.
        if (data_index != (argc - 2)) {
            CdlParse::report_error(interp, "", "Invalid cdl_configuration command in savefile, expecting two arguments.");
        } else {
            config->set_name(argv[1]);
            config->get_savefile_subcommands("cdl_configuration", subcommands);
            toplevel_commands = interp->push_commands(subcommands);

            std::string tcl_result;
            result = interp->eval(argv[2], tcl_result);
            
            interp->pop_commands(toplevel_commands);
            toplevel_commands = 0;
        }
        
    } catch(...) {
        if (0 != toplevel_commands) {
            interp->pop_commands(toplevel_commands);
        }
        throw;
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

// ----------------------------------------------------------------------------
int
CdlConfigurationBody::savefile_description_command(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAME("CdlConfiguration::savefile_description_command");
    CYG_PRECONDITION_CLASSC(interp);

    CdlToplevel toplevel = interp->get_toplevel();
    CYG_ASSERT_CLASSC(toplevel);
    CdlConfiguration config = dynamic_cast<CdlConfiguration>(toplevel);
    CYG_ASSERT_CLASSC(config);

    std::vector<std::pair<std::string,std::string> > options;
    int data_index = CdlParse::parse_options(interp, "cdl_configuration/description command", 0, argc, argv, 1, options);
        
    if (data_index != (argc - 1)) {
        CdlParse::report_warning(interp, "",
                                 "Ignoring invalid configuration description command, expecting a single argument.");
    } else {
        config->description = argv[1];
    }    
    return TCL_OK;
}

// ----------------------------------------------------------------------------
int
CdlConfigurationBody::savefile_hardware_command(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAME("CdlConfiguration::savefile_hardware_command");
    CYG_PRECONDITION_CLASSC(interp);

    CdlToplevel toplevel = interp->get_toplevel();
    CYG_ASSERT_CLASSC(toplevel);
    CdlConfiguration config = dynamic_cast<CdlConfiguration>(toplevel);
    CYG_ASSERT_CLASSC(config);

    std::vector<std::pair<std::string,std::string> > options;
    int data_index = CdlParse::parse_options(interp, "cdl_configuration/hardware command", 0, argc, argv, 1, options);
        
    if (data_index != (argc - 1)) {
        CdlParse::report_warning(interp, "", "Ignoring invalid configuration hardware command, expecting a single argument.");
    } else {
        config->current_hardware = argv[1];
    }

    return TCL_OK;
}

// ----------------------------------------------------------------------------
int
CdlConfigurationBody::savefile_template_command(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAME("CdlConfiguration::savefile_template_command");
    CYG_PRECONDITION_CLASSC(interp);

    CdlToplevel toplevel = interp->get_toplevel();
    CYG_ASSERT_CLASSC(toplevel);
    CdlConfiguration config = dynamic_cast<CdlConfiguration>(toplevel);
    CYG_ASSERT_CLASSC(config);

    std::vector<std::pair<std::string,std::string> > options;
    int data_index = CdlParse::parse_options(interp, "cdl_configuration/template command", 0, argc, argv, 1, options);
        
    if (data_index != (argc - 1)) {
        CdlParse::report_warning(interp, "", "Ignoring invalid configuration template command, expecting a single argument.");
    } else {
        config->current_template = argv[1];
    }
    
    return TCL_OK;
}

// ----------------------------------------------------------------------------
int
CdlConfigurationBody::savefile_package_command(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAME("CdlConfiguration::savefile_package_command");
    CYG_PRECONDITION_CLASSC(interp);
    
    CdlToplevel toplevel = interp->get_toplevel();
    CYG_ASSERT_CLASSC(toplevel);
    CdlConfiguration config = dynamic_cast<CdlConfiguration>(toplevel);
    CYG_ASSERT_CLASSC(config);
    CdlPackagesDatabase db = config->get_database();
    CYG_ASSERT_CLASSC(db);

    std::string pkgname;
    std::string pkgversion;
    CdlPackage  pkg = 0;
    
    std::vector<std::pair<std::string,std::string> > options;
    static char* optlist[] = {
        "template:f",
        "hardware:f",
        0
    };
    int data_index = CdlParse::parse_options(interp, "cdl_configuration/package command", optlist, argc, argv, 1, options);

    if (data_index == (argc - 1)) {
        // If no version is specified, silently default to the most recent.
        pkgname = argv[argc - 1];
        pkgversion = "";
    } else if (data_index == (argc - 2)) {
        pkgname = argv[argc - 2];
        pkgversion = argv[argc - 1];
    } else {
        // If we cannot load all the packages then much of the
        // savefile is likely to be problematical.
        CdlParse::report_error(interp, "", "Invalid cdl_configuration/package command, expecting name and version");
        CYG_REPORT_RETURN();
        return TCL_OK;
    }

    if (0 != config->lookup(pkgname)) {
        // If the package was already loaded, check the version string. If the versions
        // are identical then we do not need to worry any further. Otherwise a mismatch
        // warning is appropriate.
        CdlNode node = config->lookup(pkgname);
        CYG_ASSERT_CLASSC(node);
        pkg = dynamic_cast<CdlPackage>(node);
        if (0 == pkg) {
            // The name is in use, but it is not a package
            CdlParse::report_error(interp, "",
                                   std::string("Unable to load package `") + pkgname + "', the name is already in use.");
        } else if (pkgversion != pkg->get_value()) {
            CdlParse::report_warning(interp, "",
                                     std::string("Cannot load version `") + pkgversion + "' of package `" +
                                     pkgname + "', version `" + pkg->get_value() + "' is already loaded.");
        }
    } else if (!db->is_known_package(pkgname)) {
        CdlParse::report_error(interp, "",
                               std::string("Attempt to load an unknown package `") + pkgname + "'.");
    } else {
        if ("" != pkgversion) {
            const std::vector<std::string>& versions = db->get_package_versions(pkgname);
            if (versions.end() == std::find(versions.begin(), versions.end(), pkgversion)) {
                CdlParse::report_warning(interp, "",
                                         std::string("The savefile specifies version `") + pkgversion +
                                         "' for package `" + pkgname + "'\nThis version is not available.\n" +
                                         "Using the most recent version instead.");
                pkgversion = "";
            }
        }
        CdlDiagnosticFnPtr error_fn    = interp->get_error_fn_ptr();
        CdlDiagnosticFnPtr warn_fn     = interp->get_warning_fn_ptr();
        CdlTransaction     transaction = interp->get_transaction();
        config->load_package(transaction, pkgname, pkgversion, error_fn, warn_fn, false);
        CdlNode pkg_node = config->lookup(pkgname);
        CYG_ASSERTC(0 != pkg_node);
        pkg = dynamic_cast<CdlPackage>(pkg_node);
        CYG_ASSERT_CLASSC(pkg);
    }

    if ((0 != pkg) && (0 != options.size())) {
        std::vector<std::pair<std::string,std::string> >::const_iterator opt_i;
        for (opt_i = options.begin(); opt_i != options.end(); opt_i++) {
            if (opt_i->first == "template") {
                pkg->loaded_for_template = true;
            } else if (opt_i->first == "hardware") {
                pkg->loaded_for_hardware = true;
            } 
        }
    }
        
    return TCL_OK;
}

//}}}

//}}}

