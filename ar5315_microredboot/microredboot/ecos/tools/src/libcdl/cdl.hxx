#ifndef __CDL_HXX
# define __CDL_HXX
//{{{  Banner                                   

//============================================================================
//
//      cdl.hxx
//
//      This header file declares the classes related to software
//      configuration.
//
//============================================================================
//####COPYRIGHTBEGIN####
//                                                                          
// ----------------------------------------------------------------------------
// Copyright (C) 2002 Bart Veer
// Copyright (C) 1998, 1999, 2000, 2001 Red Hat, Inc.
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
// Date:        1998/02/09
// Version:     0.02
// Requires:    cdlcore.hxx
// Usage:       #include <cdl.hxx>
//
//####DESCRIPTIONEND####
//============================================================================

//}}}
//{{{  nested #include's                        

// ----------------------------------------------------------------------------
// Software CDL depends on the core but adds no new system requirements.

#ifndef __CDLCORE_HXX
# include <cdlcore.hxx>
#endif

//}}}

//{{{  Forward declarations of the body classes 

// ----------------------------------------------------------------------------
// This section provides forward declarations of the main classes used
// for software configuration.

class CdlConfigurationBody;
class CdlPackageBody;
class CdlComponentBody;
class CdlOptionBody;
class CdlPackagesDatabaseBody;

typedef CdlConfigurationBody*           CdlConfiguration;
typedef CdlPackageBody*                 CdlPackage;
typedef CdlComponentBody*               CdlComponent;
typedef CdlOptionBody*                  CdlOption;
typedef CdlPackagesDatabaseBody*        CdlPackagesDatabase;

typedef const CdlConfigurationBody*     CdlConstConfiguration;
typedef const CdlPackageBody*           CdlConstPackage;
typedef const CdlComponentBody*         CdlConstComponent;
typedef const CdlOptionBody*            CdlConstOption;
typedef const CdlPackagesDatabaseBody*  CdlConstPackagesDatabase;

//}}}
//{{{  CdlPackagesDatabase class                

// ----------------------------------------------------------------------------
// An eCos component repository can get to be quite complicated. There will
// be a number of core packages supplied by Red Hat. There may also be some
// number of third party and unsupported packages. Each package may come in
// several different versions. Keeping track of everything that has been
// installed should involve a separate administration tool, and there should
// be some sort of database of all this information.
//
// At the time of writing there is no such administration tool and there is
// no database with details of the various packages. Instead there is a
// static file "packages" at the top level of the component repository,
// containing some of the desired information.
//
// For now a temporary CdlPackagesDatabase class is provided.
// Essentially this provides C++ access to the packages file. In the
// long term this class will be replaced completely by a full and more
// rational implementation.
//
// The packages database class also, temporarily, provides information about
// targets and templates. This will change in future. A target will be specified
// by a save file, the output from Hardy. A template will also be specified by
// a save file, a partial software configuration.

class CdlPackagesDatabaseBody {

    friend class CdlTest;
    friend class CdlDbParser;
    
  public:

    static CdlPackagesDatabase  make(std::string = "", CdlDiagnosticFnPtr /* error */ = 0,
                                     CdlDiagnosticFnPtr /* warn */ = 0);
    bool                        update(void);
    ~CdlPackagesDatabaseBody();

    std::string                         get_component_repository() const;
    
    const std::vector<std::string>&     get_packages(void) const;
    bool                                is_known_package(std::string) const;
    const std::string&                  get_package_description(std::string) const;
    const std::vector<std::string>&     get_package_aliases(std::string) const;
    const std::vector<std::string>&     get_package_versions(std::string) const;
    const std::string&                  get_package_directory(std::string) const;
    const std::string&                  get_package_script(std::string) const;
    bool                                is_hardware_package(std::string) const;

    const std::vector<std::string>&     get_targets(void) const;
    bool                                is_known_target(std::string) const;
    const std::string&                  get_target_description(std::string) const;
    const std::vector<std::string>&     get_target_aliases(std::string) const;
    const std::vector<std::string>&     get_target_packages(std::string) const;
    const std::vector<std::string>&     get_target_enables(std::string) const;
    const std::vector<std::string>&     get_target_disables(std::string) const;
    const std::vector<std::pair<std::string,std::string> >& get_target_set_values(std::string) const;
    
    const std::vector<std::string>&     get_templates(void) const;
    bool                                is_known_template(std::string) const;
    std::string                         get_template_filename(std::string, std::string = "") const;
    const std::vector<std::string>&     get_template_versions(std::string) const;
    const std::string                   get_template_description(std::string, std::string = "");
    const std::vector<std::string>&     get_template_packages(std::string, std::string = "");
    static void                         extract_template_details(std::string /* filename */, std::string& /* description */,
                                                                 std::vector<std::string>& /* packages */);
    
    // What are the valid compiler flag variables (ARCHFLAGS, ERRFLAGS, ...)?
    // For now the library provides a static vector of these things, but
    // this area is likely to change in future
    static const std::vector<std::string>& get_valid_cflags();

    // Control verbosity when reading in a database
    static void set_verbose(bool);
    
    bool check_this(cyg_assert_class_zeal = cyg_quick) const;
    CYGDBG_DECLARE_MEMLEAK_COUNTER();
    
  private:
    // The only valid constructor gets invoked from the make() member function.
    // The argument should be a pathname for the component repository. The
    // constructor is responsible for reading in the whole packages file.
    CdlPackagesDatabaseBody(std::string, CdlDiagnosticFnPtr, CdlDiagnosticFnPtr);

    std::string                         component_repository;
    std::vector<std::string>            package_names;
    struct package_data {
      public:
        std::string                     description;
        std::vector<std::string>        aliases;
        std::vector<std::string>        versions;
        std::string                     directory;
        std::string                     script;
        bool                            hardware;
    };
    std::map<std::string,struct package_data> packages;
    
    std::vector<std::string>            target_names;
    struct target_data {
      public:
        std::string                     description;
        std::vector<std::string>        aliases;
        std::vector<std::string>        packages;
        std::vector<std::string>        enable;
        std::vector<std::string>        disable;
        std::vector<std::pair<std::string, std::string> > set_values;
    };
    std::map<std::string, struct target_data>    targets;
    
    std::vector<std::string>            template_names;
    struct template_version_data {
      public:
        std::string                     description;
        std::vector<std::string>        packages;
    };
    struct template_data {
      public:
        std::vector<std::string>        versions;
        std::map<std::string, struct template_version_data> version_details;
    };
    std::map<std::string, struct template_data>   templates;
    
    enum {
        CdlPackagesDatabaseBody_Invalid = 0,
        CdlPackagesDatabaseBody_Magic   = 0x50896acb
    } cdlpackagesdatabasebody_cookie;

    // This allows test cases to overwrite the name of the file
    // containing the database information.
    static char* database_name;

    // Control whether or not minor problems with the database should be
    // reported.
    static bool verbose_mode;
    
    // The default constructor, copy constructor and assignment operator are illegal.
    CdlPackagesDatabaseBody();
    CdlPackagesDatabaseBody(const CdlPackagesDatabaseBody&);
    CdlPackagesDatabaseBody& operator=(const CdlPackagesDatabaseBody&);
    
};

//}}}
//{{{  CdlConfiguration class                   

// ----------------------------------------------------------------------------
// The CdlConfiguration class is the toplevel used for mainpulating
// software configurations. It consists of a number of loaded packages,
// each of which consists of some hierarchy of components and options,
// plus dialogs, wizards, and interfaces from the core.
//
// Typically an application will deal with only one configuration at a
// time. There will be exceptions. The most obvious example would be
// some sort of diff utility, but there may well be times when a user
// wants to edit multiple configurations. One example would be a board
// containing two separate processors, e.g. a conventional one coupled
// with a separate DSP, and eCos is supposed to run on both: the two
// chips will need to interact, and hence there may well be
// configurability dependencies between them.
//
// A configuration object does not exist in isolation. It must be tied
// to an eCos component repository via a database objects. It must
// also be supplied with a suitable Tcl interpreter.

class CdlConfigurationBody : public virtual CdlToplevelBody
{
    friend class CdlTest;

  public:
    
    // ----------------------------------------------------------------------------
    // Create a new configuration.
    // Currently this requires a name, a database and a master interpreter.
    // The name is not used very much, but does appear in savefiles.
    // The packages database and the interpreter must be created before
    // any configuration object.
    static CdlConfiguration     make(std::string /* name */, CdlPackagesDatabase, CdlInterpreter);
    CdlPackagesDatabase         get_database() const;

    // Loading and unloading packages. This can happen in a number
    // of different ways:
    //
    // 1) explicitly adding or removing a single package
    // 2) changing the version of a package, which means unloading
    //    the old version and reloading the new one. Generally
    //    user settings should be preserved where possible.
    // 3) loading in a full or minimal savefile. The library
    //    does not actually distinguish between the two when
    //    loading, only when saving.
    // 4) adding a full or minimal savefile.
    // 5) setting a template for the first time. This is much the
    //    same as adding a minimal savefile. However the library
    //    keeps track of the "current" template and only one
    //    can be loaded at a time. The library does not keep
    //    track of which savefile(s) have been added.
    // 6) changing a template. This means unloading the packages
    //    that were loaded for the old template, then loading in
    //    the new one.
    // 7) unsetting the template. This just involves an unload.
    // 8) setting the hardware. Currently the database defines
    //    each supported target, listing the packages that should
    //    be loaded and possibly some option details. This is
    //    subject to change in future.
    // 9) changing the hardware. This is much the same as changing
    //    the template.
    // 10) unsetting the hardware.
    //
    // The unload operations are comparatively safe, although
    // they can result in new conflicts and the user may well want
    // to cancel the operation after discovering how many new
    // problems there would be. The load operations are a bit more
    // dangerous since they depend on external data and hence can
    // fail for a variety of reasons: missing files, corrupt files,
    // clashes with existing data, ... Changing e.g. a template
    // is especially dangerous because of the number of things
    // that can go wrong. All of these operations need to happen
    // in a transaction which the user can cancel. There are two
    // versions of all the relevant routines, one which operates
    // in an existing transaction and one which creates a new
    // one.
    //
    // Any operation that involves loading CDL data takes two
    // CdlDiagnosticFnPtr arguments, one for errors and one for
    // warnings. These should report the message to the user by
    // some suitable means. The error fnptr may throw a
    // CdlParseException to abort the current load immediately,
    // otherwise the load operation will be aborted at the end
    // if any errors were detected. New conflicts are not
    // handled by these diagnostic functions, instead they
    // are handled by the normal transaction methods.

    // A version argument of "" implies the most recent version.
    void load_package(std::string /* name */, std::string /* version */,
                      CdlDiagnosticFnPtr /* error */, CdlDiagnosticFnPtr /* warn */, bool /* limbo */ = true);
    void load_package(CdlTransaction, std::string /* name */, std::string /* version */,
                      CdlDiagnosticFnPtr /* error */, CdlDiagnosticFnPtr /* warn */, bool /* limbo */ = true);

    void unload_package(std::string /* name */, bool /* limbo */ = true);
    void unload_package(CdlPackage, bool /* limbo */ = true);
    void unload_package(CdlTransaction, std::string /* name */, bool /* limbo */ = true);
    void unload_package(CdlTransaction, CdlPackage, bool /* limbo */ = true);
    
    void change_package_version(std::string /*name*/, std::string /*version*/,
                                CdlDiagnosticFnPtr /* error */, CdlDiagnosticFnPtr /* warn */, bool /* limbo */ = true);
    void change_package_version(CdlPackage, std::string /*version*/, CdlDiagnosticFnPtr /* error */,
                                CdlDiagnosticFnPtr /* warn */, bool /* limbo */ = true);

    void change_package_version(CdlTransaction, std::string /*name*/, std::string /*version*/,
                                CdlDiagnosticFnPtr /* error */, CdlDiagnosticFnPtr /* warn */, bool /* limbo */ = true);
    void change_package_version(CdlTransaction, CdlPackage, std::string /*version*/, CdlDiagnosticFnPtr /* error */,
                                CdlDiagnosticFnPtr /* warn */, bool /* limbo */ = true);

    // Loading a savefile is different in that it creates a new
    // toplevel. Since transactions can only be created if the
    // toplevel already exists, it is not possible to have a
    // per-transaction load() operation. It is possible to have
    // a per-transaction add() operation.
    static CdlConfiguration load(std::string /* filename */, CdlPackagesDatabase, CdlInterpreter,
                                 CdlDiagnosticFnPtr /* error */,  CdlDiagnosticFnPtr /* warn */);

    void add(std::string /* filename */,
             CdlDiagnosticFnPtr /* error */, CdlDiagnosticFnPtr /* warn */);
    void add(CdlTransaction, std::string /* filename */,
             CdlDiagnosticFnPtr /* error */, CdlDiagnosticFnPtr /* warn */);

    // As with packages, a version of "" implies the most recent.
    void set_template(std::string, std::string /* version */,
                      CdlDiagnosticFnPtr, CdlDiagnosticFnPtr /* warn */,bool /* limbo */ = true);
    void set_template_file(std::string,
                           CdlDiagnosticFnPtr /* error */, CdlDiagnosticFnPtr /* warn */, bool /* limbo */ = true);
    void set_template(CdlTransaction, std::string, std::string /* version */,
                      CdlDiagnosticFnPtr, CdlDiagnosticFnPtr /* warn */, bool /* limbo */ = true);
    void set_template_file(CdlTransaction, std::string,
                           CdlDiagnosticFnPtr /* error */, CdlDiagnosticFnPtr /* warn */, bool /* limbo */ = true);
    void unload_template(bool /* limbo */ = true);
    void unload_template(CdlTransaction, bool /* limbo */ = true);
    std::string get_template() const;
    void        set_template_name(std::string); // Intended for library use only

    void set_hardware(std::string,
                      CdlDiagnosticFnPtr /* error */, CdlDiagnosticFnPtr /* warn */, bool /* limbo */ = true);
    void set_hardware(CdlTransaction, std::string,
                      CdlDiagnosticFnPtr /* error */, CdlDiagnosticFnPtr /* warn */, bool /* limbo */ = true);
    void unload_hardware(bool /* limbo */ = true);
    void unload_hardware(CdlTransaction, bool /* limbo */ = true);
    std::string get_hardware() const;
    void        set_hardware_name(std::string); // Intended for library use only
    
    // ----------------------------------------------------------------------------
    // Save a configuration to a file
    void        save(std::string, bool /* minimal */ = false);
    void        initialize_savefile_support();
    std::string get_save_file() const;

    // ----------------------------------------------------------------------------
    // Get rid of a configuration.
    ~CdlConfigurationBody();

    virtual std::string         get_class_name() const;
    bool                        check_this(cyg_assert_class_zeal = cyg_quick) const;
    CYGDBG_DECLARE_MEMLEAK_COUNTER();
    
  private:

    // The only legal constructor, invoked from make() and load()
    CdlConfigurationBody(std::string, CdlPackagesDatabase, CdlInterpreter);

    // The internal implementation of the persistence support
    virtual void                save(CdlInterpreter, Tcl_Channel, int, bool);
    static int                  savefile_configuration_command(CdlInterpreter, int, const char*[]);
    static int                  savefile_description_command(CdlInterpreter, int, const char*[]);
    static int                  savefile_hardware_command(CdlInterpreter, int, const char*[]);
    static int                  savefile_template_command(CdlInterpreter, int, const char*[]);
    static int                  savefile_package_command(CdlInterpreter, int, const char*[]);
    
    std::string                 current_hardware;
    std::string                 current_template;
    std::string                 description;
    CdlPackagesDatabase         database;
    std::string                 save_file;
    enum {
        CdlConfigurationBody_Invalid    = 0,
        CdlConfigurationBody_Magic      = 0x5c409a3d
    } cdlconfigurationbody_cookie;

    // The constructor can only be invoked via the make() and load()
    // members. Other constructors and the assignment operator are
    // illegal.
    CdlConfigurationBody();
    CdlConfigurationBody(const CdlConfigurationBody&);
    CdlConfigurationBody& operator=(const CdlConfigurationBody&);
    
};

//}}}
//{{{  CdlPackage class                         

// ----------------------------------------------------------------------------
// Packages inherit from most of the base classes.

class CdlPackageBody : public virtual CdlNodeBody,
                       public virtual CdlContainerBody,
                       public virtual CdlUserVisibleBody,
                       public virtual CdlValuableBody,
                       public virtual CdlParentableBody,
                       public virtual CdlBuildableBody,
                       public virtual CdlDefinableBody,
                       public virtual CdlLoadableBody,
                       public virtual CdlBuildLoadableBody,
                       public virtual CdlDefineLoadableBody
{                       
    friend class CdlTest;

    // Packages should not be created by application code, but
    // the CdlConfiguration class must be able to do so inside
    // load_package();
    friend class CdlConfigurationBody;
    
  public:

    ~CdlPackageBody();
    
    static int          parse_package(CdlInterpreter, int, const char*[]);
    static int          parse_hardware(CdlInterpreter, int, const char*[]);
    static int          parse_install_proc(CdlInterpreter, int, const char*[]);
    static int          parse_license_proc(CdlInterpreter, int, const char*[]);

    // Override the CdlDefineLoadable member. Hardware packages always
    // send their configuration options to hardware.h
    virtual std::string get_config_header() const;
    
    bool                is_hardware_package() const;
    bool                has_install_proc() const;
    const cdl_tcl_code& get_install_proc() const;
    bool                has_license_proc() const;
    const cdl_tcl_code& get_license_proc() const;

    // Propagation support. Because of multiple virtual inheritance
    // it is necessary to invoke the container and valuable
    // update members.
    virtual void update(CdlTransaction, CdlUpdate);
    
    // Persistence support.
    virtual void        save(CdlInterpreter, Tcl_Channel, int, bool);
    static void         initialize_savefile_support(CdlToplevel);
    static int          savefile_package_command(CdlInterpreter, int, const char*[]);

    // Was this package loaded because of a template or hardware setting?
    bool                belongs_to_template() const;
    bool                belongs_to_hardware() const;
    
    virtual std::string get_class_name() const;
    bool                check_this(cyg_assert_class_zeal = cyg_quick) const;
    CYGDBG_DECLARE_MEMLEAK_COUNTER();
    
  private:

    // The only valid constructor requires a number of fields
    CdlPackageBody(std::string /* name */, CdlConfiguration, std::string /* directory */);

    // Other constructors are illegal
    CdlPackageBody();
    CdlPackageBody(const CdlPackageBody&);
    CdlPackageBody& operator=(const CdlPackageBody&);

    bool loaded_for_template;
    bool loaded_for_hardware;
    
    enum {
        CdlPackageBody_Invalid  = 0,
        CdlPackageBody_Magic    = 0x1d7c0d43
    } cdlpackagebody_cookie;
};

//}}}
//{{{  CdlComponent class                       

// ----------------------------------------------------------------------------
// Similarly components just inherit from the appropriate base classes.

class CdlComponentBody : public virtual CdlNodeBody,
                         public virtual CdlContainerBody,
                         public virtual CdlUserVisibleBody,
                         public virtual CdlValuableBody,
                         public virtual CdlParentableBody,
                         public virtual CdlBuildableBody,
                         public virtual CdlDefinableBody
{
    friend class CdlTest;

  public:

    ~CdlComponentBody();
    static int          parse_component(CdlInterpreter, int, const char*[]);
    static int          parse_script(CdlInterpreter, int, const char*[]);

    // Propagation support. Because of multiple virtual inheritance
    // it is necessary to invoke the container and valuable
    // update members.
    virtual void update(CdlTransaction, CdlUpdate);
    
    // Persistence support.
    virtual void        save(CdlInterpreter, Tcl_Channel, int, bool);
    static void         initialize_savefile_support(CdlToplevel);
    static int          savefile_component_command(CdlInterpreter, int, const char*[]);
    
    virtual std::string get_class_name() const;
    bool                check_this(cyg_assert_class_zeal = cyg_quick) const;
    CYGDBG_DECLARE_MEMLEAK_COUNTER();
    
  private:

    // The only valid constructor requires a name.
    CdlComponentBody(std::string);
    
    enum {
        CdlComponentBody_Invalid  = 0,
        CdlComponentBody_Magic    = 0x6359d9a7
    } cdlcomponentbody_cookie;
    
    // Other constructors are illegal
    CdlComponentBody();
    CdlComponentBody(const CdlComponentBody&);
    CdlComponentBody& operator=(const CdlComponentBody&);
};

//}}}
//{{{  CdlOption class                          

// ----------------------------------------------------------------------------
// Again options just inherit their functionality from the base classes.

class CdlOptionBody : public virtual CdlNodeBody,
                      public virtual CdlUserVisibleBody,
                      public virtual CdlValuableBody,
                      public virtual CdlParentableBody,
                      public virtual CdlBuildableBody,
                      public virtual CdlDefinableBody
{
    friend class CdlTest;
    
  public:
    ~CdlOptionBody();
    
    static int          parse_option(CdlInterpreter, int, const char*[]);
    
    // Persistence support.
    virtual void        save(CdlInterpreter, Tcl_Channel, int, bool);
    static void         initialize_savefile_support(CdlToplevel);
    static int          savefile_option_command(CdlInterpreter, int, const char*[]);
    
    virtual std::string get_class_name() const;
    bool                check_this(cyg_assert_class_zeal = cyg_quick) const;
    CYGDBG_DECLARE_MEMLEAK_COUNTER();

  private:
    CdlOptionBody(std::string);

    enum {
        CdlOptionBody_Invalid   = 0,
        CdlOptionBody_Magic     = 0x1c1162d1
    } cdloptionbody_cookie;
    
    CdlOptionBody();
    CdlOptionBody(const CdlOptionBody&);
    CdlOptionBody& operator=(const CdlOptionBody&);
};

//}}}

#endif  /* !__CDL_HXX */
// EOF cdl.hxx
