# <a name="Introduction"/>Introduction

This workspace is setup to be built with [Gradle](http://www.gradle.org) through
the ["Bndtools Gradle Plugin by Pelagic"]
(https://marketplace.eclipse.org/content/bndtools-plugins-pelagic) Eclipse
plugin.

The build is setup in such a way that bnd (OSGi) projects and projects with
specific gradle files are automatically included in the build; no editing of
Gradle build scripts is needed.

A simple command in the root of the workspace is enough to build it all:

```
gradle build
```

**Note**: The official bnd Gradle plugin is used so that the same build fidelity
is achieved.

However, this build setup adds some extra features to the build...

First off, the build setup is far more flexible and featureful than the build
setup that is delivered by bndtools itself.

Among other things it has the following extra features:

* Support for FindBugs
* Support for JUnit code coverage reports through Jacoco
* An easily customisable setup
* Documentation
* Automatic location of the cnf project
* ...and much more

In order to use this build setup in new workspace, enable the plugin in the
'Prefrences->Bndtools->Generated Resources' pane (and disable the Gradle plugin
that's delivered by bndtools itself, see the [screenshot]
(Bndtools Plugins by Pelagic - Screenshot.png)).

The plugin is compatible with bndtools 2.3.0.REL and later.


# <a name="License"/>License

This document is licensed under the GNU Free Documentation License,
Version 1.3, 3 November 2008


# <a name="OpenSource"/>Open Source

The plugin was originally developed for - and delivered with - bndtools 2.3.0
but has since been replaced with a different implemention in bndtools.

The plugin was thereon forked and now lives on through the support
of [Pelagic](http://www.pelagic.nl).

The project is fully Open Source, licensed under the LGPL, and can be found
at [GitHub](https://github.com/fhuberts/bndtoolsPlugins).

Contributions are welcome!


# <a name="TableOfContents"/>Table Of Contents

* [Introduction](#Introduction)
* [License](#License)
* [Open Source](#OpenSource)
* [Table Of Contents](#TableOfContents)
* [Installing Gradle](#InstallingGradle)
  * [On The System](#InstallingGradleOnTheSystem)
  * [In The Workspace](#InstallingGradleInTheWorkspace)
* [Configuring The Gradle Daemon](#ConfiguringTheGradleDaemon)
* [Projects & Workspaces](#ProjectsAndWorkspaces)
  * [Root Project](#ProjectsAndWorkspacesRootProject)
  * [Sub-Projects](#ProjectsAndWorkspacesSubProjects)
  * [Gradle Workspace](#ProjectsAndWorkspacesGradleWorkspace)
  * [Bnd Workspace](#ProjectsAndWorkspacesBndWorkspace)
  * [Configuration Project](#ProjectsAndWorkspacesCnf)
  * [Bnd Project Layout](#ProjectsAndWorkspacesBndProjectLayout)
* [Build Flow](#BuildFlow)
* [Build Tasks](#BuildTasks)
  * [Bnd Projects](#BuildTasksBndProjects)
    * [jar](#BuildTasksJar)
    * [check](#BuildTasksCheck)
    * [checkNeeded](#BuildTasksCheckNeeded)
    * [release](#BuildTasksRelease)
    * [releaseNeeded](#BuildTasksReleaseNeeded)
    * [runbundles.<name>](#BuildTasksRunBundlesName)
    * [runbundles](#BuildTasksRunbundles)
    * [export.<name>](#BuildTasksExportName)
    * [export](#BuildTasksExport)
    * [echo](#BuildTasksEcho)
    * [bndproperties](#BuildTasksBndProperties)
    * [clean](#BuildTasksClean)
    * [cleanNeeded](#BuildTasksCleanNeeded)
  * [All Projects](#BuildTasksAllProjects)
    * [index](#BuildTasksIndex)
      * [Settings](#BuildTasksIndexSettings)
    * [clean](#BuildTasksAllClean)
    * [cleanNeeded](#BuildTasksAllCleanNeeded)
    * [distClean](#BuildTasksDistClean)
    * [distcleanNeeded](#BuildTasksDistCleanNeeded)
  * [Java Projects](#BuildTasksJavaProjects)
    * [Findbugs](#BuildTasksFindbugs)
      * [findbugsMain](#BuildTasksFindbugsMain)
      * [findbugsTest](#BuildTasksFindbugsTest)
      * [findbugs](#BuildTasksfindbugs)
      * [findbugstest](#BuildTasksfindbugstest)
      * [Settings](#FindbugsSettings)
    * [Jacoco](#BuildTasksJacoco)
      * [Settings](#BuildTasksJacocoSettings)
    * [javadoc](#BuildTasksJavadoc)
      * [Settings](#BuildTasksJavadocSettings)
    * [clean](#BuildTasksJavaClean)
  * [Root Project](#BuildTasksRootProject)
    * [wrapper](#BuildTasksWrapper)
      * [Settings](#BuildTasksRootProjectSettings)
* [Build Options](#BuildOptions)
  * [Bnd Projects](#BuildOptionsBndProjects)
  * [Findbugs](#BuildOptionsFindbugs)
* [Customising The Build](#CustomisingTheBuild)
  * [Gradle](#CustomisingTheBuildGradle)
  * [Bnd](#CustomisingTheBuildBnd)
* [Adding Java Projects To The Build](#AddingJavaProjectsToTheBuild)
* [Jenkins Build Setup](#JenkinsBuildSetup)
* [Replacing The Bndtools Gradle Setup](#ReplacingBndtoolsGradle)


# <a name="InstallingGradle"/>Installing Gradle


## <a name="InstallingGradleOnTheSystem"/>On The System

Obviously, Gradle must be installed on the system before the workspace can be
built with Gradle.

This description assumes a Linux machine. Details may vary on other OSes.

* Download Gradle from [http://www.gradle.org](http://www.gradle.org).

* Unpack the downloaded archive and put it in ```/usr/local/lib```
  as ```/usr/local/lib/gradle-2.0``` (assuming Gradle 2.0 was downloaded).

* Create a symbolic link to the Gradle installation directory to be able to
  easily switch Gradle versions later on:

  ```
  cd /usr/local/lib
  ln -s gradle-2.0 /usr/local/lib/gradle
  ```

* Put the Gradle executable ```/usr/local/lib/gradle/bin/gradle``` on
  the search path by linking to it from ```/usr/local/bin```:

  ```
  ln -s /usr/local/lib/gradle/bin/gradle /usr/local/bin/
  ```


## <a name="InstallingGradleInTheWorkspace"/>In The Workspace

Gradle can be installed in the workspace so that the workspace can be built on
systems that do not have Gradle installed (like build servers).

The procedure is:

* Open a shell and go into the workspace root directory.

* Assuming Gradle is properly installed on the system
  (see [Installing Gradle On The System](#InstallingGradleOnTheSystem)), run:

  ```
  gradle wrapper
  ```

* Commit the files that were created in the workspace to your version control
  system.


# <a name="ConfiguringTheGradleDaemon"/>Configuring The Gradle Daemon

Startup times of a Gradle build can be much improved by using the Gradle daemon.

The Gradle daemon works well when the Gradle build script dependencies are not
changed, which makes it well suited to regular development (where these are not
changed) but **not** for build servers.

The daemon can be easily setup by adding the following line
to ```~/.gradle/gradle.properties```:

```
org.gradle.daemon=true
```

Stopping the Gradle daemon is easily achieved by running:

```
gradle --stop
```


# <a name="ProjectsAndWorkspaces"/>Projects & Workspaces


## <a name="ProjectsAndWorkspacesRootProject"/>Root Project

The Gradle root project is the directory that contains the ```settings.gradle```
file.

Gradle locates the root project by first looking for the ```settings.gradle```
file in the directory from which it was run, and - when not found - then by
searching up in the directory tree.


## <a name="ProjectsAndWorkspacesSubProjects"/>Sub-Projects

The build will include all projects in the build that are:

* **bnd** projects: Directories directly below the root project with
                    a ```bnd.bnd``` file.

* **Gradle** projects: Directories directly below the root project with
                       a ```build.gradle``` file, **or**
                       a ```build-settings.gradle``` file.


## <a name="ProjectsAndWorkspacesGradleWorkspace"/>Gradle Workspace

The Gradle workspace is rooted in the root project and consists of all included
projects - **bnd** *and* **Gradle** projects.


## <a name="ProjectsAndWorkspacesBndWorkspace"/>Bnd Workspace

The bnd workspace is rooted in the root project and contains exactly one
configuration project, and zero or more **bnd** projects.

For it to be a *useful* bnd workspace, it will have to contain at least one bnd
project.


## <a name="ProjectsAndWorkspacesCnf"/>Configuration Project

The configuration project is the directory that contains the ```build.bnd```
file, and is - by default - the ```cnf``` directory.

It contains:

* Placeholder source and classes directories (```src``` and ```bin```
  respectively).

* Bnd workspace configuration.

  * &nbsp;```ext/*.bnd```

    The ```ext``` directory contains bnd settings files that are loaded
    **before** the ```build.bnd``` file.

    The directory typically contains:

    * &nbsp;```junit.bnd```

      This file defines a bnd variable for the libraries that are needed on the
      classpath when running JUnit tests.

    * &nbsp;```pluginpaths.bnd```

      This file instructs bnd to load a number of plugin libraries when it
      runs. Typically it will instruct bnd to load repository plugins. However,
      custom plugins can also be loaded by bnd by adding them to
      the ```-pluginpath``` instruction in this file.

    * &nbsp;```repositories.bnd```

      This file configures the plugins that bnd loads. Typically it will
      configure the repository plugins that are loaded. However, if any built-in
      plugins or custom plugins are loaded then these also will have to be
      configured here. This file also defines which repository is the release
      repository.

  * &nbsp;```build.bnd```

    This file contains workspace-wide settings for bnd and will override
    settings that are defined in any of the ```ext/*.bnd``` files.

* Repositories.

  * &nbsp;```buildrepo```

    This repository contains libraries that are intended **only for build-time**
    usage. None are intended to be deployed as bundles into a running OSGi
    framework, and indeed they may cause unexpected errors if they are used
    in such a fashion.

  * &nbsp;```localrepo```

    This repository contains no libraries by default. It is intended for
    external libraries that are needed by one or more of the projects.

  * &nbsp;```releaserepo```

    This repository contains no libraries by default. By default, bundles end
    up in this repository when they are released.

* Cache.

  The ```cache``` directory contains libraries that are downloaded by the build.
  If the build is self-contained then this cache only contain libraries that are
  retrieved from the workspace itself during the build.

* Build files.

  * &nbsp;```cnf/gradle```

    This directory contains all Gradle build script files that are used by the
    Gradle build, dependencies for the build, and documentation pertaining to
    the build.

    * &nbsp;```template```

      This directory contains build script files that define the build. These
      are **not** meant to be changed.

    * &nbsp;```custom```

      This directory contains build script files that are hooks into the
      build process. They allow specification of overrides for various settings,
      additions to the build, modifications to the build, etc.

      These **are** meant to be changed when build customisations are needed.

    * &nbsp;```dependencies```

      This directory contains libraries that are used by the build in the form
      of a file repository.

    * &nbsp;```doc```

      This directory contains documentation pertaining to the build. The
      document you're now reading is located in this directory.

      <a name="svg"/>Also found in this directory is the
      diagram [template.svg](template.svg) that provides an overview of the
      build setup, much like the Gradle User Guide shows for the Java Plugin.

      The diagram shows all tasks of the build and their dependencies:

      * The light cyan blocks are tasks that are present by default, either
        because Gradle creates them by default of because the project has
        applied the Java plugin, which creates these tasks.

      * The cyan block are tasks that are added or modified by the bnd plugin.
      
      * The orange blocks are tasks that are added or modified by this build
        setup.

      * The arrows depict **execution flow** (so the dependencies are in the
        reverse direction).

      * The **red** arrows depict flows from dependent projects (dependencies
        on projects) .

        For example:

        The ```compileJava``` task of a project is dependent on
        the ```assemble``` task of another project if the latter project is on
        the build path of the former project.

      * The **yellow** arrows depict flows to parent projects (dependencies
        from projects) . These are the reverse of the **red** arrows.

      * The **blue** arrows depict flows/dependencies that are only present
        when the task from which the flows originate is present in the project.

      * The **green** arrows depict flows/dependencies that are disabled by
        default.

      * The **magenta** arrows depict flows/dependencies that are automatically
        scheduled to run when the tasks from which they originate are scheduled
        to run.


## <a name="ProjectsAndWorkspacesBndProjectLayout"/>Bnd Project Layout

A bnd project has a well defined layout with a number of source directories and
one output directory:

* Main sources: located in the ```src``` directory by default. Compiled sources
  will be placed in the ```bin``` directory. Can be overridden with
  the ```src``` and ```bin``` bnd settings respectively.

* Test sources: located in the ```test``` directory by default. Compiled sources
  will be placed in the ```bin_test``` directory. Can be overridden with
  the ```testsrc``` and ```testbin``` bnd settings respectively.

* Output directory ```generated```. Built OSGi bundles will be placed here.
  Can be overridden with the ```target-dir``` bnd setting


# <a name="BuildFlow"/>Build Flow

Understanding the build flow is especially important if the build must be
modified: extra tasks must be added, properties must be overridden, etc.

The build has the following flow:

* <a name="BuildProperties"/>Gradle loads all properties defined in
  the ```gradle.properties``` file in the root of the workspace.

  This file is used to bootstrap the build and (among other things) defines
  the build dependencies:

  * All ```*.uri``` settings are considered to be build dependencies.

    An ```example.uri``` setting will make the build script add the file
    indicated by the URI to the build dependencies when the file exists on the
    local filesystem. If the file doesn't exist on the local filesystem, then
    the build script will download the build dependency from the specified URI
    into the ```cnf/cache``` directory and add it to the build dependencies.

    **Note**: Using a ```*.uri``` setting that points to an external location
    is **not recommended** since the build will then no longer be
    self-contained (because it needs network access).

* Gradle invokes the ```settings.gradle``` file in the root of the workspace.
  This file initialises the build:

  * Detect the location of the configuration project
    (see [Configuration Project](#ProjectsAndWorkspacesCnf)).

  * Initialise the bnd workspace.

  * The build dependencies are determined from the build properties that were
    loaded from the ```gradle.properties``` file
    (see [the explanation of the build properties file](#BuildProperties).

  * Include all **bnd** projects and all **Gradle** projects
    (see [Sub-Projects](#ProjectsAndWorkspacesSubProjects) for an explanation).

* Gradle invokes the ```build.gradle``` file from the root project. This file
  sets up the build:

  * The bnd plugin is loaded.

  * The hook ```cnf/gradle/custom/workspace-pre.gradle``` is invoked.

  * The rest of the build template basically has 3 sections which are applied
    in the order:

    * All projects
    * Sub-Projects
    * Root Project

    **All Projects**

    This section sets up the build for all included projects (including the root
    project) by iterating over all included projects and performing the actions
    described below.

    * The Gradle build directory of the project is set to the bnd
      default (```generated```).

    * The hook ```cnf/gradle/custom/allProject-pre.gradle``` is invoked.

    * If the project has project specific build settings in
      a ```build-settings.gradle``` file in its project root directory then the
      file is invoked.

      Project specific build settings allow - on a project-by-project basis -
      overrides of the build settings, additions to the build, modifications of
      the build, etc.

    * Index tasks are added to the project.

    * Clean tasks are added to the project.

    * The hook ```cnf/gradle/custom/allProject-post.gradle``` is invoked.

    **Sub-Projects**

    This section sets up the build for all included projects, (excluding the
    root project) by iterating over all included sub-projects. In the iteration
    a distinction is made between **bnd** projects and **Gradle** projects.

    * The hook ```cnf/gradle/custom/subProject-pre.gradle``` is invoked.

    * Gradle projects (Non-Bnd Projects)

      * The hook ```cnf/gradle/custom/nonBndProject-pre.gradle``` is invoked.

      * The default tasks are setup (specified by the ```others_defaultTask```
        property). This is a comma separated list of task names.

    * Bnd projects

      * The bnd project instance is stored in the project as
        the ```bndProject``` property.

      * The hook ```cnf/gradle/custom/bndProject-pre.gradle``` is invoked.

      * The bnd plugin is applied.

      * The javadoc bootclasspath is adjusted to include the bootclasspath
        of the bnd project.

      * The hook ```cnf/gradle/custom/bndProject-post.gradle``` is invoked.

    * All projects that have applied the Gradle Java plugin:

      * The hook ```cnf/gradle/custom/javaProject-pre.gradle``` is invoked.

      * The javaDoc task is setup.

      * The findbugs tasks are setup.

      * The jacoco task is setup.

      * The clean task is adjusted to clean all output directories of all
        sourceSets.

      * The hook ```cnf/gradle/custom/javaProject-post.gradle``` is invoked.

    * The hook ```cnf/gradle/custom/subProject-post.gradle``` is invoked.

    **Root Project**

    This section sets up the build (defaults) for the root project.

    * The hook ```cnf/gradle/custom/rootProject-pre.gradle``` is invoked.

    * The default tasks are setup (specified by the ```root_defaultTask```
      property).  This is a comma separated list of task names.

    * The wrapper task is setup.

    * The distclean task is adjusted to also clean the ```cnf/cache```
      and ```.gradle``` directories.

    * The hook ```cnf/gradle/custom/rootProject-post.gradle``` is invoked.

  * The hook ```cnf/gradle/custom/workspace-post.gradle``` is invoked.

* For every included project with a ```build.gradle``` build script, Gradle
  invokes that build script.

* Gradle resolves the build setup.

* Gradle now builds the projects by running the specified (or default) tasks.


# <a name="BuildTasks"/>Build Tasks

The discussion of the build tasks below is split per project type.


## <a name="BuildTasksBndProjects"/>Bnd Projects

This section only discusses tasks that are added or modified compared to the
Gradle Java plugin.


### <a name="BuildTasksJar"/>jar

This task instructs bnd to construct the project's OSGi bundles. The bundles
are placed in the project's ```generated``` directory.

This is comparable to the ```jar``` task that is defined by the Java plugin,
which instructs the Java compiler to construct a standard jar. However, a
bnd project completely replaces that ```jar``` task.

The ```bnd.bnd``` file describes how the OSGi bundle(s) must be constructed and
is therefore taken as input by bnd.

This task is automatically disabled when no bundle(s) must be contructed.


### <a name="BuildTasksCheck"/>check

This task instructs bnd to run bundle (integration) tests.

This is comparable to the ```test``` task that is defined by the Java plugin,
which instructs the Java runtime to run unit tests.

Refer to the bnd manual/website for more details on how to setup bundle tests.

Set ```check.ignoreFailures = true``` on the project to not fail the build
on check failures.

This task is automatically disabled when no bundle tests have been defined.


### <a name="BuildTasksCheckNeeded"/>checkNeeded

This task will invoke the ```check``` task on all projects on which the
project is dependent, after which the ```check``` task is invoked on the
project itself.


### <a name="BuildTasksRelease"/>release

This task instructs bnd to copy the constructed OSGi bundle into the release
repository.

This task is automatically disabled when no release repository is defined or
when no bundle(s) must be constructed.


### <a name="BuildTasksReleaseNeeded"/>releaseNeeded

This task will invoke the ```release``` task on all projects on which the
project is dependent, after which the ```release``` task is invoked on the
project itself.


### <a name="BuildTasksRunBundlesName"/>runbundles.<name>

This task will copy all runbundles metioned in the ``<name>.bndrun``` file in
the project to a distribution directory. The bundles are place in the
directory ```generated/distributions/runbundles/<name>```.

This task is only setup if the project contains a ```<name>.bndrun``` file.


### <a name="BuildTasksRunbundles"/>runbundles

This task will invoke the ```runbundles.<name>``` for all ```*.bndrun``` files
in the project.

This task is automatically disabled when the project contains no ```*.bndrun```
files.


### <a name="BuildTasksExportName"/>export.<name>

This task will export the ```<name>.bndrun``` file in the project to an
executable jar. The executable jar is placed in the
project's ```generated/distributions``` directory as ```<name>.jar```.

This task is only setup if the project contains a ```<name>.bndrun``` file.


### <a name="BuildTasksExport"/>export

This task will export all ```*.bndrun``` files in the project to executable
jars.

This task is automatically disabled when the project contains no ```*.bndrun```
files.


### <a name="BuildTasksEcho"/>echo

This task displays some key bnd properties of the project.


### <a name="BuildTasksBndProperties"/>bndproperties

This task - analogous to the Gradle ```properties``` task - displays the bnd
properties that are defined for the project.

These properties are defined in the ```bnd.bnd``` file in the root of the
project (and optionally other ```*.bnd``` files when using the ```-sub```
instruction for sub-bundles).

Properties that are defined in workspace-wide ```*.bnd``` files that are loaded
from the configuration project (```cnf```) are also displayed as they obviously
also apply to the project (unless overridden by the project, in which case the
overridden values are shown).


### <a name="BuildTasksClean"/>clean

This task instructs bnd to clean up the project, which removes
the output directory and the directories that hold the class files.

This is in addition to the ```clean``` task that is defined by the Java plugin.


### <a name="BuildTasksCleanNeeded"/>cleanNeeded

This task will invoke the ```clean``` task on all projects on which the
project is dependent, after which the ```clean``` task is invoked on the
project itself.


## <a name="BuildTasksAllProjects"/>All Projects

This section discusses tasks that are added to all projects.


### <a name="BuildTasksIndex"/>index

This task can create one or more of the following:
* an uncompressed and/or compressed OBR bundle repository index
* an uncompressed and/or compressed R5 bundle repository index

These indexes are generated from one *or more* configured file trees.

Which files are indexed is controlled by the ```indexDirectories``` property.
Its **syntax** is:

```
<root-directory>;<name>;<name-of-fileTree-property>, ...
```

* &nbsp;```root-directory```: This is the root/base directory
  from where the relative URLs in the index file are calculated, and where
  the index file will be placed. Must be specified but doesn't need to exist.

* &nbsp;```name```: This is the name of the repository. Can be empty, in which
  case the name (*basename*) of the ```root directory``` is used.

* &nbsp;```name-of-fileTree-property```: This is the name of a project property
  that must be an instance of a FileTree. This file tree determines which
  files will be indexed. If not specified then all ```*.jar``` files below
  the ```root directory``` are indexed.

Multiple indexes can be generated by specifying (**syntax** as displayed above):

```
syntax,syntax,...
```

This task is automatically disabled when no index directories have been defined
or when no OBR indexes **and** no R5 indexes are configured to be created
(either uncompressed or compressed).

#### <a name="BuildTasksIndexSettings"/>Settings

* &nbsp;```indexDirectories```: See the explanation just above.

* &nbsp;```indexOBRUncompressed```: **true** to generate an uncompressed OBR
                                    bundle repository index.
                                    Defaults to **false**.

* &nbsp;```indexOBRCompressed```: **true** to generate a compressed OBR
                                  bundle repository index.
                                  Defaults to **false**.

* &nbsp;```indexR5Uncompressed```: **true** to generate an uncompressed R5
                                   bundle repository index.
                                   Defaults to **false**.

* &nbsp;```indexR5Compressed```: **true** to generate a compressed R5
                                 bundle repository index.
                                 Defaults to **true**.

The defaults for the settings can be overridden by defining the settings in the
project's ```build-settings.gradle``` file.


### <a name="BuildTasksAllClean"/>clean

This (empty) task is only added to the project when it doesn't yet have
a ```clean``` task. The reason for this is to be able to easily declare clean
task dependencies.


### <a name="BuildTasksAllCleanNeeded"/>cleanNeeded

This (empty) task is only added to the project when it doesn't yet have
a ```cleanNeeded``` task. The reason for this is to be able to easily declare
clean task dependencies.


### <a name="BuildTasksDistClean"/>distClean

This (empty by default) task is meant to perform additional cleanup compared to
the ```clean``` task.

The build adjusts this task for the root project such that it removes:

* The cache directory in the configuration project.

* The Gradle cache directory.


### <a name="BuildTasksDistCleanNeeded"/>distcleanNeeded

This task will invoke the ```distClean``` task on all projects on which the
project is dependent, after which the ```distClean``` task is invoked on the
project itself.


## <a name="BuildTasksJavaProjects"/>Java Projects

This section discusses tasks that are added to all Java projects (which
includes bnd projects).


### <a name="BuildTasksFindbugs"/>Findbugs

The findbugs plugin is applied to all Java projects. This plugin adds the
tasks ```findbugsMain``` and ```findbugsTest```.

These two tasks are disabled by default since running findbugs is an expensive
operation and is not needed for most builds. Enabling these tasks is discussed
below.

**Note**: The reports that are generated by the findbugs tasks will only have
line numbers when the tasks are run on a build that produces artefacts with
debug information.

#### <a name="BuildTasksFindbugsMain"/>findbugsMain

This task will run findbugs on the main source code.

#### <a name="BuildTasksFindbugsTest"/>findbugsTest

This task will run findbugs on the test source code.

#### <a name="BuildTasksfindbugs"/>findbugs

Specifying this (virtual) task will **enable** the ```findbugsMain``` task.

**Note**: It is still required to specify a task that has a dependency on
the ```findbugsMain``` task to actually run it. The tasks ```check```
and ```build``` are examples of such a task.

#### <a name="BuildTasksfindbugstest"/>findbugstest

Specifying this (virtual) task will **enable** the ```findbugsTest``` task.

**Note**: it is still required to specify a task that has a dependency on
the ```findbugsTest``` task to actually run it. The tasks ```check```
and ```build``` are examples of such a task.

#### <a name="FindbugsSettings"/>Settings

* &nbsp;```findbugsReportXML```: The name of the property that must be defined
                                 in order to generate XML reports instead of
                                 HTML reports (since the findbugs plugin can't
                                 create them both at the same time). Defaults
                                 to **CI**.

* &nbsp;```findbugsIgnoreFailures```: **true** to ignore findbugs warning (to
                                      **not** fail the build). Defaults
                                      to **true**.

* &nbsp;```findbugsIncludesFile```: The file with include rules. Defaults
                                    to ```${rootProject.rootDir}/${rootProject.bnd_cnf}/findbugs/findbugs.include.xml```.

* &nbsp;```findbugsExcludesFile```: The file with exclude rules. Defaults
                                    to ```{rootProject.rootDir}/${rootProject.bnd_cnf}/findbugs/findbugs.exclude.xml```.

The defaults for the settings can be overridden by defining the settings in the
project's ```build-settings.gradle``` file.


### <a name="BuildTasksJacoco"/>Jacoco

The jacoco plugin is applied to all Java projects. This plugin adds the
task ```jacocoTestReport``` which details the test coverage.

The ```jacocoTestReport``` task is automatically run when either or both of
the ```test``` and ```check``` tasks are scheduled to run.

An ```test.exec``` report - for consumption by a build server - is always
created.

#### <a name="BuildTasksJacocoSettings"/>Settings

* &nbsp;```jacocoToolVersion```: The version of the jacoco plugin to use.
                                 Defaults to **0.7.5.201505241946**.

* &nbsp;```jacocoXmlReport```: **true** to generate XML reports. Defaults
                               to **true**.

* &nbsp;```jacocoCsvReport```: **true** to generate CSV reports. Defaults
                               to **false**.

The defaults for the settings can be overridden by defining the settings in the
project's ```build-settings.gradle``` file.


### <a name="BuildTasksJavadoc"/>javadoc

This task generates javadoc for the main source code.

#### <a name="BuildTasksJavadocSettings"/>Settings

* &nbsp;```javadocDir```: The directory (relative to the project's build
                          directory) in which to place the javadoc.
                          Defaults to **javadoc**.

* &nbsp;```javadocTitle```: The same as ```javadocDocTitle```.
                            Defaults to **${project.name}**.

* &nbsp;```javadocFailOnError```: **false** to ignore errors (to **not** fail
                                  the build).
                                  Defaults to **true**.

* &nbsp;```javadocMaxMemory```: The maximum amount of memory that the javadoc
                                task is allowed to consume.
                                Defaults to **256M**.

* &nbsp;```javadocVerbose```: **true** for verbose mode: provide more detailed
                              messages while javadoc is running. Without the
                              verbose option, messages appear for loading the
                              source files, generating the documentation (one
                              message per source file), and sorting. The verbose
                              option causes the printing of additional messages
                              specifying the number of milliseconds to parse
                              each java source file.
                              Defaults to **false**.

* &nbsp;```javadocDocTitle```: The title to be placed near the top of the
                               overview summary file. The title will be placed
                               as a centered, level-one heading directly beneath
                               the upper navigation bar. The title may contain
                               html tags and white space, though if it does, it
                               must be enclosed in quotes. Any internal
                               quotation marks within title may have to be
                               escaped.
                               Defaults to **${project.name}**.

* &nbsp;```javadocWindowTitle```:  The title to be placed in the
                                   HTML ```<title>``` tag. This appears in the
                                   window title and in any browser bookmarks
                                   (favorite places) that someone creates for
                                   this page. This title should not contain any
                                   HTML tags, as the browser will not properly
                                   interpret them. Any internal quotation marks
                                   within title may have to be escaped.
                                   Defaults to **${project.name}**.

* &nbsp;```javadocClassPathBoot```: The bootclasspath.
                                    Defaults to an empty list of files.
                                    Is automatically set for bnd projects.

* &nbsp;```javadocMemberLevel```: The minimum member level to include in the
                                  generated documention. Can be (from lowest
                                  level to highest level): **PRIVATE**
                                  , **PROTECTED**, **PACKAGE**, **PUBLIC**.
                                  Defaults to **PUBLIC**.

* &nbsp;```javadocEncoding```: The encoding name of the source files.
                               Defaults to **UTF-8**.

* &nbsp;```javadocAuthor```: **true** to include the @author text in the
                             generated documentation.
                             Defaults to **true**.

* &nbsp;```javadocBreakIterator```: **true** to use the internationalized
                                    sentence boundary of java.text.BreakIterator
                                    to determine the end of the first sentence
                                    for English (all other locales already use
                                    BreakIterator), rather than an English
                                    language, locale-specific algorithm. By
                                    first sentence, the first sentence in the
                                    main description of a package, class or
                                    member is meant. This sentence is copied to
                                    the package, class or member summary, and to
                                    the alphabetic index.
                                    Defaults to **true**.

* &nbsp;```javadocDocFilesSubDirs```: **true** to enable deep copying
                                      of "doc-files" directories. In other
                                      words, subdirectories and all contents are
                                      recursively copied to the destination. For
                                      example, the
                                      directory ```doc-files/example/images```
                                      and all its contents would now be copied.
                                      Defaults to **true**.

* &nbsp;```javadocNoComment```: **true** to suppress the entire comment body,
                                including the main description and all tags,
                                generating only declarations. This option
                                enables re-using source files originally
                                intended for a different purpose, to produce
                                skeleton HTML documentation at the early stages
                                of a new project.
                                Defaults to **false**.

* &nbsp;```javadocNoDeprecated```: **true** to prevent the generation of any
                                   deprecated API at all in the documentation.
                                   This does what ```javadocNoDeprecatedList```
                                   does, plus it does not generate any
                                   deprecated API throughout the rest of the
                                   documentation. This is useful when writing
                                   code and you don't want to be distracted by
                                   the deprecated code.
                                   Defaults to **false**.

* &nbsp;```javadocNoDeprecatedList```: **true** to prevent the generation of the
                                       file containing the list of deprecated
                                       APIs (deprecated-list.html) and the link
                                       in the navigation bar to that page.
                                       (However, javadoc continues to generate
                                       the deprecated API throughout the rest of
                                       the document.) This is useful if your
                                       source code contains no deprecated API,
                                       and you want to make the navigation bar
                                       cleaner.
                                       Defaults to **false**.

* &nbsp;```javadocNoHelp```: **true** to omit the HELP link in the navigation
                             bars at the top and bottom of each page of output. 
                             Defaults to **false**.

* &nbsp;```javadocNoIndex```: **true** to omit the index from the generated
                              documentation. The index is produced by default.
                              Defaults to **false**.

* &nbsp;```javadocNoNavBar```: **true** to prevent the generation of the
                               navigation bar, header and footer, otherwise
                               found at the top and bottom of the generated
                               pages. Has no effect on the "bottom" option.
                               The option is useful when you are interested only
                               in the content and have no need for navigation,
                               such as converting the files to PostScript or PDF
                               for print only.
                               Defaults to **false**.

* &nbsp;```javadocNoSince```: **true** to omit from the generated documentation
                              the "Since" sections associated with the @since
                              tags.
                              Defaults to **false**.

* &nbsp;```javadocNoTimestamp```: **true** to suppress the timestamp, which is
                                  hidden in an HTML comment in the generated
                                  HTML near the top of each page. Useful when
                                  you want to run javadoc on two source bases
                                  and diff them, as it prevents timestamps from
                                  causing a diff (which would otherwise be a
                                  diff on every page). The timestamp includes
                                  the javadoc version number.
                                  Defaults to **false**.

* &nbsp;```javadocNoTree```: **true** to omit the class/interface hierarchy
                             pages from the generated documentation. These are
                             the pages you reach using the "Tree" button in the
                             navigation bar.
                             Defaults to **false**.

* &nbsp;```javadocSplitIndex```: **true** to split the index file into multiple
                                 files, alphabetically, one file per letter,
                                 plus a file for any index entries that start
                                 with non-alphabetical characters.
                                 Defaults to **true**.

* &nbsp;```javadocUse```: **true** to include one "Use" page for each documented
                          class and package. The page describes what packages,
                          classes, methods, constructors and fields use any API
                          of the given class or package. Given class C, things
                          that use class C would include subclasses of C, fields
                          declared as C, methods that return C, and methods and
                          constructors with parameters of type C. You can
                          access the generated "Use" page by first going to the
                          class or package, then clicking on the "Use" link in
                          the navigation bar.
                          Defaults to **true**.

* &nbsp;```javadocVersion```: **true** to include the @version text in the
                              generated documentation.
                              Defaults to **true**.

The defaults for the settings can be overridden by defining the settings in the
project's ```build-settings.gradle``` file.


### <a name="BuildTasksJavaClean"/>clean

The build adjusts this task for Java projects (which includes bnd projects)
such that it removes:

* The class files output directory of all defined sourcesets.

* The resources output directory of all defined sourcesets.


## <a name="BuildTasksRootProject"/>Root Project

This section discusses tasks that are modified for the root project.

### <a name="BuildTasksWrapper"/>wrapper

This task downloads Gradle and installs it in the workspace,
see [Installing Gradle In The Workspace](#InstallingGradleInTheWorkspace).


#### <a name="BuildTasksRootProjectSettings"/>Settings

* &nbsp;```rootGradleVersion```: The version of the Gradle to use when
                                 generating the Gradle wrapper.
                                 Not setting it will result in the latest
                                 released version of Gradle being used.
                                 No default.

The defaults for the settings can be overridden by defining the settings in the
project's ```build-settings.gradle``` file.


# <a name="BuildOptions"/>Build Options


## <a name="BuildOptionsBndProjects"/>Bnd Projects

* The ```jar``` task can be disabled by:

  * Presence of the ```-nobundles``` instruction in the ```bnd.bnd``` file.

* The ```test``` task can be disabled by:

  * Presence of the ```-nojunit``` instruction in the ```bnd.bnd``` file.

  * Presence of the ```no.junit```  property in the ```bnd.bnd``` file.

* The ```check``` task can be disabled by:

  * Presence of the ```-nojunitosgi``` instruction in the ```bnd.bnd``` file.

  * Absence of the ```Test-Cases``` bnd property in the ```bnd.bnd``` file.

* The ```release``` task can be disabled by:

  * Presence of the ```-nobundles``` instruction in the ```bnd.bnd``` file.

  * Absence of the ```-releaserepo``` instruction in any of the bnd files.


## <a name="BuildOptionsFindbugs"/>Findbugs

The findbugs tasks will - by default - generate HTML reports, but can be
instructed to generate XML reports by setting the ```CI``` Gradle system
property (```-PCI``` on the command line).

&nbsp;```CI``` = **C**ontinous **I**ntegration
                 (since XML reports are typically used on build servers)


# <a name="CustomisingTheBuild"/>Customising The Build


## <a name="CustomisingTheBuildGradle"/>Gradle

The build be can easily customised by putting overrides and additions in any of
the ```cnf/gradle/custom/*.gradle``` build script hook files,
see [Build Flow](#BuildFlow).

Also, any project can - on an individual basis - customise build settings or
specify additions by placing a ```build-settings.gradle``` file in its
root directory.

The ```build-settings.gradle``` file is meant for settings and settings
overrides, the ```build.gradle``` file is meant for tasks.

An example of a ```build-settings.gradle``` file is shown below. This example
shows how a project instructs the build to index its ```bundles``` directory
to generate four indexes named ```example project```.

```
/* Index task overrides */
ext.indexTreeRoot        = 'bundles'
ext.indexTree            = fileTree(indexTreeRoot).include('**/*.jar').exclude('**/*-latest.jar')

ext.indexDirectories     = "${indexTreeRoot};example project;indexTree"
ext.indexOBRUncompressed = true
ext.indexOBRCompressed   = true
ext.indexR5Uncompressed  = true
ext.indexR5Compressed    = true
```


## <a name="CustomisingTheBuildBnd"/>Bnd

The bnd default settings are shown in the ```cnf/build.bnd``` file.
Overrides to workspace-wide bnd settings can be placed in that same file.

If a setting must be overridden or defined for a specific project, then that
setting must be defined in the ```bnd.bnd``` file of that specific project.


# <a name="AddingJavaProjectsToTheBuild"/>Adding Java Projects To The Build

The build automatically includes all bnd projects.

However, regular Java projects are not included automatically:
a ```build.gradle``` file or a ```build-settings.gradle``` file in the root
directory of the project is needed to make that happen.

Such projects only need to apply the Gradle Java plugin, setup their
sourceSets, and setup their build directory. The template will then
automatically apply the buildscript ```cnf/gradle/template/javaProject.gradle```
which adds tasks that are relevant to Java projects,
see [Java Projects](#BuildTasksJavaProjects).

The ```build-settings.gradle``` file shown below can be used as the basis. This
will setup the Java project with the default bnd layout and add tasks that are
relevant to a Java project (```javadoc```, ```findbugs...```, etc.).

```
/*
 * A java project with bnd layout
 */

assert(project != rootProject)

apply plugin: 'java'

/* We use the same directory for java and resources. */
sourceSets {
  main {
    java.srcDirs      = resources.srcDirs   = files('src')
    output.classesDir = output.resourcesDir =       'bin'
  }
  test {
    java.srcDirs        = resources.srcDirs   = files('test'    )
    output.classesDir   = output.resourcesDir =       'test_bin'
  }
}

buildDir = 'generated'
```

The ```build-settings.gradle``` file shown below can be used as the basis for a
project with the Maven layout.

```
/*
 * A java project with Maven layout
 */

assert(project != rootProject)

apply plugin: 'java'

/* We do not use the same directory for java and resources. */
sourceSets {
  main {
    java.srcDirs        = files('src/main/java'     )
    resources.srcDirs   = files('src/main/resources')
    output.classesDir   =       'target/classes'
    output.resourcesDir =       'target/classes'
  }
  test {
    java.srcDirs        = files('src/test/java'     )
    resources.srcDirs   = files('src/test/resources')
    output.classesDir   =       'target/test-classes'
    output.resourcesDir =       'target/test-classes'
  }
}

buildDir = 'target'
```


# <a name="JenkinsBuildSetup"/>Jenkins Build Setup

The screenshot ([Jenkins-Build-Settings.jpg](Jenkins-Build-Settings.jpg)) shows
part of an example job.

The shown settings can be used as an example, but can slightly differ for your
own builds.

The following Jenkins plugins should be installed to take advantage of the
various functionalities of the build (some of which are shown in the
screenshot):

* build timeout plugin
* build-name-setter
* FindBugs Plug-in
* Git Parameter Plug-In
* GIT plugin
* Gradle plugin
* JaCoCo plugin
* Javadoc Plugin
* JUnit Plugin
* Mailer Plugin

Ensure your Jenkins has the most recent versions of these plugins.

This applies especially to the Jacoco plugin since the binary format of Jacoco
was changed a few times in the second half of 2015.


# <a name="ReplacingBndtoolsGradle"/>Replacing The Bndtools Gradle Setup

The bndtools Gradle setup can be easily replaced:
* Create a new workspace with the Gradle setup of this plugin.
* Now copy over the relevant files from the new workspace to the workspace
  in which the setup should be replaced. The files which should be copied over
  can be found here:
  * [root](https://github.com/fhuberts/bndtoolsPlugins/tree/master/nl.pelagic.bndtools.headless.build.plugin.gradle/resources/templates/unprocessed/root): these files should be placed in the
    workspace root.
  * [cnf](https://github.com/fhuberts/bndtoolsPlugins/tree/master/nl.pelagic.bndtools.headless.build.plugin.gradle/resources/templates/unprocessed/cnf): these directories should be placed in the cnf directory
    that is in the workspace root.
