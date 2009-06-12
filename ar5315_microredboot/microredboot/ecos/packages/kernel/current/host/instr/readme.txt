
README for User Friendly Instrumentation Messages
=================================================

The CDL option CYGDBG_KERNEL_INSTRUMENT_MSGS controls whether the
system is capable of displaying user friendly instrumentation
messages.  To do this it needs to know what event numbers mean
what events - as a text string to print for you.

It gets the information from a table created by the header file
instrument_desc.h in the kernel.  If the instrumentation numbers
change, you must rebuild that header to match.

A further CDL option is provided to help you with that.  The
procedure is as follows:

1) Remove kernel/VERSION/include/instrument_desc.h from your
   repository.  Is is probably a good idea to move it away rather
   than deleting it in case the next stages fail.

2) Make a build configuration, enabling options
   CYGDBG_KERNEL_INSTRUMENT, CYGDBG_KERNEL_INSTRUMENT_MSGS and
   CYGDBG_KERNEL_INSTRUMENT_MSGS_BUILD_HEADERFILE in the kernel.
   Viewed in the GUI configtool, "Kernel instrumentation", "Print
   user friendly instrument messages" and "Rebuild the header
   file" respectively.

3) Make eCos within this build configuration.  It should create
   install/include/cyg/kernel/instrument_desc.h

4) Copy that new file back to your repository, to
   kernel/VERSION/include/instrument_desc.h replacing the file
   you moved aside in step 1.

5) Commit the new file to your version control system or whatever
   you use.

If you wish to rebuild the file "by hand" the command to use, in
a suitable shell, is this:

   $ECOS_REPOSITORY/kernel/$ECOS_VERSION/host/instr/instrument.sh 
        $ECOS_REPOSITORY/kernel/$ECOS_VERSION/include/instrmnt.h
        > $ECOS_BUILD_PREFIX/include/cyg/kernel/instrument_desc.h

(all on one line of course) or to rebuild it directly into the
kernel source repository:

   $ECOS_REPOSITORY/kernel/$ECOS_VERSION/host/instr/instrument.sh 
        $ECOS_REPOSITORY/kernel/$ECOS_VERSION/include/instrmnt.h
        > $ECOS_REPOSITORY/kernel/$ECOS_VERSION/include/instrument_desc.h

It's up to you to sort out file permissions for this to work, and
to set environment variables as required or edit these lines as
you type them.


There is also a host-based program which can print a buffer
nicely for you - if you can get the data into your host.
Enabling CDL option CYGDBG_KERNEL_INSTRUMENT_BUILD_HOST_DUMP or
"Build the host tool to print out a dump" will build it for you
in install/bin.  To build it instead "by hand", use

    mkdir -p tempinc
    cp -r $(ECOS_BUILD_PREFIX)/include/cyg tempinc
    cp -r $(ECOS_BUILD_PREFIX)/include/pkgconf tempinc
    cc -I./tempinc
    $(ECOS_REPOSITORY)/kernel/${ECOS_VERSION}/host/instr/dump_instr.c
    -o $(PREFIX)/bin/dump_instr

again with environment variables as required, or type in whatever
is appropriate.  You still have to somehow get the
instrumentation buffer into a file on the host. 'Exercise for the
reader' as university lecturers tend to say.

One possibility is to set up a tftp *server* in the target which
will serve the instrumentation buffer.  This hint is as far as
support for this goes.



