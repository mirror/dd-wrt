# Only load the rhabout plugin if it was installed.
foreach dir $::gdb_plugins {
  if {[file exists [file join $dir rhabout]]} {
    package require RHABOUT 1.0
    $Menu add command Other "About Red Hat" \
      {ManagedWin::open RHAbout} -underline 0

    # To activate the PlugIn sample, uncomment the next line
    set plugins_available 1
  }
}

# Only load the Intel Pentium plugin for x86 targets.
if {[string match "i?86-*" $::GDBStartup(target_name)] && ![TargetSelection::native_debugging]} {
  package require INTELPENTIUM 1.0

  # Add a new cascading-style menu to plugin menu
  $Menu add cascade intel "Intel Pentium" 0

  # Add MSR selection dialog menu item.
  $Menu add command None "MSR Selection..." \
    {ManagedWin::open_dlg MsrSelDlg} -underline 0

  # Add CPU info menu item.
  $Menu add command None "CPU Information..." \
    display_cpu_info -underline 0

  # Activate the PlugIn.
  set plugins_available 1
}
