# {{{  Banner                                                   

# ============================================================================
# 
#      watchdog.tcl
# 
#      Watchdog support for the eCos synthetic target I/O auxiliary
# 
# ============================================================================
# ####COPYRIGHTBEGIN####
#                                                                           
#  ----------------------------------------------------------------------------
#  Copyright (C) 2002 Bart Veer
# 
#  This file is part of the eCos host tools.
# 
#  This program is free software; you can redistribute it and/or modify it 
#  under the terms of the GNU General Public License as published by the Free 
#  Software Foundation; either version 2 of the License, or (at your option) 
#  any later version.
#  
#  This program is distributed in the hope that it will be useful, but WITHOUT 
#  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
#  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for 
#  more details.
#  
#  You should have received a copy of the GNU General Public License along with
#  this program; if not, write to the Free Software Foundation, Inc., 
#  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
# 
#  ----------------------------------------------------------------------------
#                                                                           
# ####COPYRIGHTEND####
# ============================================================================
# #####DESCRIPTIONBEGIN####
# 
#  Author(s):   bartv
#  Contact(s):  bartv
#  Date:        2002/09/04
#  Version:     0.01
#  Description:
#      Implementation of the watchdog device. This script should only ever
#      be run from inside the ecosynth auxiliary.
# 
# ####DESCRIPTIONEND####
# ============================================================================

# }}}

namespace eval watchdog {

    # Was initialization successful?
    variable init_ok 1

    # Has the alarm triggered?
    variable alarm_triggered 0
    
    # The eCos application process id. This is needed to send a SIGPWR signal
    # if the watchdog triggers, and to access /proc/<pid>/stat to obtain
    # timing information. Strictly speaking _ppid is not exported by
    # the I/O auxiliary.
    if { ! [info exists synth::_ppid] } {
        synth::report_error "Watchdog initialization failed, _ppid variable required"
        return ""
    }
    variable ecos_pid $synth::_ppid

    # Resolution, i.e. how long to go between checks. Currently this is hard-wired
    # to one second, or 1000 ms. This may become configurable, either on the
    # target-side via CDL or on the host-side via the target definition file.
    # Note that currently the watchdog device and the GUI get updated via the
    # same timer. If the resolution is changed to e.g. 10 seconds then it might
    # be a good idea to update the GUI more frequently, although there are
    # arguments for keeping the animation in step with the real work.
    variable resolution 1000
    
    # Options from the target definition file
    variable use_wallclock 0
    variable window_pack   "-in .main.n -side right"
    variable sound_file    ""
    variable sound_player  "play"

    if { [synth::tdf_has_device "watchdog"] } {
        if { [synth::tdf_has_option "watchdog" "use"] } {
            set _use [synth::tdf_get_option "watchdog" "use"]
            if { "wallclock_time" == $_use } {
                set watchdog::use_wallclock 1
            } elseif { "consumed_cpu_time" == $_use } {
                set watchdog::use_wallclock 0
            } else {
                synth::report_error "Invalid entry in target definition file $synth::target_definition\n\
                                    \    Device watchdog, option \"use\" should be \"wallclock_time\" or \"consumed_cpu_time\"\n"
            }
            unset _use
        }
        if { [synth::tdf_has_option "watchdog" "watchdog_pack"] } {
            set watchdog::window_pack [synth::tdf_get_option "watchdog" "watchdog_pack"]
            # Too complicated to validate here, instead leave it to a catch statement
            # when the window actually gets packed
        }
        if { [synth::tdf_has_option "watchdog" "sound"] } {
            set _sound_file [synth::tdf_get_option "watchdog" "sound"]
            # Look for this sound file in the install tree first, then absolute or relative
            if { [file exists [file join $synth::device_install_dir $_sound_file] ] } {
                set _sound_file [file join $synth::device_install_dir $_sound_file]
            }
            if { ![file exists $_sound_file] } {
                synth::report_error "Invalid entry in target definition file $synth::target_definition\n\
                                    \    Device watchdog, option \"sound\", failed to find $_sound_file\n"
            } elseif { ! [file readable $_sound_file] } {
                synth::report_error "Invalid entry in target definition file $synth::target_definition\n\
                                    \    Device watchdog, option \"sound\", no read access to file $_sound_file\n"
            } else {
                set watchdog::sound_file $_sound_file
            }
            unset _sound_file
        }
        if { [synth::tdf_has_option "watchdog" "sound_player"] } {
            set watchdog::sound_player [synth::tdf_get_option "watchdog" "sound_player"]
        }
    }

    # There is no point in creating the watchdog window if any of the image files are missing
    if { $synth::flag_gui } {
        foreach _image [list "doghouse.gif" "alarm.gif" "eye.gif" "asleep.gif"] {
            variable image_[file rootname $_image]
            if { ! [synth::load_image "watchdog::image_[file rootname $_image]" [file join $synth::device_install_dir $_image]] } {
                synth::report_error "Watchdog device, unable to load image $_image\n\
                                    \    This file should have been installed in $synth::device_install_dir\n"
                set watchdog::init_ok 0
            }
        }
    }
    if { $synth::flag_gui && $watchdog::init_ok } {
        canvas .watchdog -width [image width $image_doghouse] -height [image height $image_doghouse] \
            -borderwidth 0
        variable background [.watchdog create image 0 0 -anchor nw -image $image_doghouse]
    
        # Eye positions inside the doghouse. The eye is an 8x10 gif,
        # mostly white but transparent around the corners
        variable left_eye_x         48
        variable left_eye_y         70
        variable right_eye_x        58
        variable right_eye_y        70
        
        # Pupil positions relative to the eye. The pupils are 3x3 rectangles.
        # The dog can look in one of nine different directions, with both eyes
        # looking in the same direction (if visible)
        variable pupil_positions { { 1 6 } { 1 5 } { 1 3 } { 3 1 } { 3 4 } { 3 6 } { 4 3 } { 4 5 } { 4 6 } }
    
        # Which eyes are currently visible: none, left, right or both
        variable eyes "none"
        # What is the current pupil position?
        variable pupils 4
        
        variable left_eye  [.watchdog create image $left_eye_x $left_eye_y -anchor nw -image $image_eye]
        variable right_eye [.watchdog create image $right_eye_x $right_eye_y -anchor nw -image $image_eye]
    
        variable left_pupil \
            [.watchdog create rectangle \
                 [expr $left_eye_x + [lindex [lindex $pupil_positions $pupils] 0]]     \
                 [expr $left_eye_y + [lindex [lindex $pupil_positions $pupils] 1]]     \
                 [expr $left_eye_x + [lindex [lindex $pupil_positions $pupils] 0] + 2] \
                 [expr $left_eye_y + [lindex [lindex $pupil_positions $pupils] 1] + 2] \
                 -fill black]
        variable right_pupil \
            [.watchdog create rectangle \
                 [expr $right_eye_x + [lindex [lindex $pupil_positions $pupils] 0]]     \
                 [expr $right_eye_y + [lindex [lindex $pupil_positions $pupils] 1]]     \
                 [expr $right_eye_x + [lindex [lindex $pupil_positions $pupils] 0] + 2] \
                 [expr $right_eye_y + [lindex [lindex $pupil_positions $pupils] 1] + 2] \
                 -fill black]

    
        # The dog is asleep until the eCos application activates the watchdog device
        .watchdog lower $left_eye $background
        .watchdog lower $right_eye $background
        .watchdog lower $left_pupil $background
        .watchdog lower $right_pupil $background

        # Prepare for an alarm, but obviously the alarm picture should be hidden for now.
        variable alarm [.watchdog create image 30 56 -anchor nw -image $image_alarm]
        .watchdog lower $alarm $background

        # Start asleep
        variable asleep [.watchdog create image 48 70 -anchor nw -image $image_asleep]
        
        # Now try to pack the watchdog window using the option provided by the
        # user. If that fails, report the error and pack in a default window.
        if { [catch { eval pack .watchdog $watchdog::window_pack } message] } {
            synth::report_error "Watchdog device, failed to pack window in $watchdog::window_pack\n    $message"
            pack .watchdog -in .main.n -side right
        }
                   
        # Updating the display. This happens once a second.
        # If neither eye is visible, choose randomly between
        # left-only, right-only or both. Otherwise there is
        # a one in eight chance of blinking, probably switching
        # to one of the other eye modes
        #
        # Also, the visible pupil(s) will move every second, to one
        # of nine positions
        proc gui_update { } {
        
            if { "none" == $watchdog::eyes} {
                set rand [expr int(3 * rand())]
                if { 0 == $rand } {
                    set watchdog::eyes "left"
                    .watchdog raise $watchdog::left_eye   $watchdog::background
                    .watchdog raise $watchdog::left_pupil $watchdog::left_eye
                } elseif { 1 == $rand } {
                    set watchdog::eyes "right"
                    .watchdog raise $watchdog::right_eye   $watchdog::background
                    .watchdog raise $watchdog::right_pupil $watchdog::right_eye
                } else {
                    set watchdog::eyes "both"
                    .watchdog raise $watchdog::left_eye    $watchdog::background
                    .watchdog raise $watchdog::left_pupil  $watchdog::left_eye
                    .watchdog raise $watchdog::right_eye   $watchdog::background
                    .watchdog raise $watchdog::right_pupil $watchdog::right_eye
                }
            } else {
                if { 0 == [expr int(8 * rand())] } {
                    set watchdog::eyes "none"
                    .watchdog lower $watchdog::left_eye    $watchdog::background
                    .watchdog lower $watchdog::right_eye   $watchdog::background
                    .watchdog lower $watchdog::left_pupil  $watchdog::background
                    .watchdog lower $watchdog::right_pupil $watchdog::background

                    # There is no point in moving the pupils if both eyes are shut
                    return
                }
            }

            set watchdog::pupils [expr int(9 * rand())]
            set new_pupil_x [lindex [lindex $watchdog::pupil_positions $watchdog::pupils] 0]
            set new_pupil_y [lindex [lindex $watchdog::pupil_positions $watchdog::pupils] 1]

            if { ("left" == $watchdog::eyes) || ("both" == $watchdog::eyes) } {
                .watchdog coords $watchdog::left_pupil              \
                    [expr $watchdog::left_eye_x + $new_pupil_x]     \
                    [expr $watchdog::left_eye_y + $new_pupil_y]     \
                    [expr $watchdog::left_eye_x + $new_pupil_x + 2] \
                    [expr $watchdog::left_eye_y + $new_pupil_y + 2]
            }
            if { ("right" == $watchdog::eyes) || ("both" == $watchdog::eyes) } {
                .watchdog coords $watchdog::right_pupil              \
                    [expr $watchdog::right_eye_x + $new_pupil_x]     \
                    [expr $watchdog::right_eye_y + $new_pupil_y]     \
                    [expr $watchdog::right_eye_x + $new_pupil_x + 2] \
                    [expr $watchdog::right_eye_y + $new_pupil_y + 2]
            }
        }

        # Cancel the gui display when the eCos application has exited.
        # The watchdog is allowed to go back to sleep. If the application
        # exited because of the watchdog then of course the alarm picture
        # should remain visible, otherwise it would be just a flash.
        proc gui_cancel { } {
            .watchdog lower $watchdog::left_eye    $watchdog::background
            .watchdog lower $watchdog::right_eye   $watchdog::background
            .watchdog lower $watchdog::left_pupil  $watchdog::background
            .watchdog lower $watchdog::right_pupil $watchdog::background
            if { ! $watchdog::alarm_triggered } {
                .watchdog raise $watchdog::asleep      $watchdog::background
            }
        }
        
        # Raise the alarm. This involves hiding the eyes and raising
        # the alarm picture. If sound is enabled, the sound player
        # should be invoked
        proc gui_alarm { } {
            .watchdog lower $watchdog::asleep      $watchdog::background
            .watchdog lower $watchdog::left_eye    $watchdog::background
            .watchdog lower $watchdog::right_eye   $watchdog::background
            .watchdog lower $watchdog::left_pupil  $watchdog::background
            .watchdog lower $watchdog::right_pupil $watchdog::background
            .watchdog raise $watchdog::alarm       $watchdog::background

            if { "" != $watchdog::sound_file } {
                # Catch errors on the actual exec, e.g. if the sound player is
                # invalid, but play the sound in the background. If there are
                # problems actually playing the sound then the user should
                # still see a message on stderr. Blocking the entire auxiliary
                # for a few seconds is not acceptable.
                if { [catch { eval exec -- $watchdog::sound_player $watchdog::sound_file & } message] } {
                    synth::report_warning "Watchdog device, failed to play alarm sound file\n    $message\n"
                }
            }
        }

	set _watchdog_help [file join $synth::device_src_dir "doc" "devs-watchdog-synth.html"]
	if { ![file readable $_watchdog_help] } {
	    synth::report_warning "Failed to locate synthetic watchdog documentation $_watchdog_help\n\
			          \    Help->Watchdog target menu option disabled.\n"
	    set _watchdog_help ""
	}
	if { "" == $_watchdog_help } {
	    .menubar.help add command -label "Watchdog" -state disabled
	} else {
	    .menubar.help add command -label "Watchdog" -command [list synth::handle_help "file://$_watchdog_help"]
	}
    }

    # Now for the real work. By default the watchdog is asleep. The eCos
    # application can activate it with a start message, which results
    # in an "after" timer. That runs once a second to check whether or not
    # the watchdog should trigger, and also updates the GUI.
    #
    # The target-side code should perform a watchdog reset at least once
    # a second, which involves another message to this script and the
    # setting of the reset_received flag.
    #
    # The update handler gets information about the eCos application using
    # /proc/<pid>/stat (see man 5 proc). The "state" field is important:
    # a state of T indicates that the application is stopped, probably
    # inside gdb, so cannot reset the watchdog. The other important field
    # is utime, the total number of jiffies (0.01 seconds) executed in
    # user space. The code maintains an open file handle to the /proc file.

    variable reset_received     0
    variable after_id           ""
    variable proc_stat          ""
    variable last_jiffies       0
    
    set _filename "/proc/[set watchdog::ecos_pid]/stat"
    if { [catch { open $_filename "r" } proc_stat ] } {
        synth::report_error "Watchdog device, failed to open $_filename\n    $proc_stat\n"
        set watchdog::init_ok 0
    }
    unset _filename

    proc update { } {
        set watchdog::after_id [after $watchdog::resolution watchdog::update]
        if { $synth::flag_gui } {
            watchdog::gui_update
        }
        seek $watchdog::proc_stat 0 "start"
        set line [gets $watchdog::proc_stat]
        scan $line "%*d %*s %s %*d %*d %*d %*d %*d %*lu %*lu %*lu %*lu %*lu %lu" state jiffies

	# In theory it is possible to examine the state field (the third argument).
	# If set to T then that indicates the eCos application is traced or
	# stopped, probably inside gdb, and it would make sense to act as if
	# the application had sent a reset. Unfortunately the state also appears
	# to be set to T if the application is blocked in a system call while
	# being debugged - including the idle select(), making the test useless.
	# FIXME: figure out how to distinguish between being blocked inside gdb
	# and being in a system call.
	#if { "T" == $state } {
	#    set watchdog::reset_received 1
	#    return
	#}
	
        # If there has been a recent reset the eCos application can continue to run for a bit longer.
        if { $watchdog::reset_received } {
            set watchdog::last_jiffies $jiffies
            set watchdog::reset_received 0
            return
        }

        # We have not received a reset. If the watchdog is using wallclock time then
        # that is serious. If the watchdog is using elapsed cpu time then the eCos
        # application may not actually have consumed a whole second of cpu time yet.
        if { $watchdog::use_wallclock || (($jiffies - $watchdog::last_jiffies) > ($watchdog::resolution / 10)) } {
            set watchdog::alarm_triggered 1
            # Report the situation via the main text window
            synth::report "Watchdog device: the eCos application has not sent a recent reset\n    Raising SIGPWR signal.\n"
            # Then kill off the eCos application
            exec kill -PWR $watchdog::ecos_pid
            # There is no point in another run of the timer
            after cancel $watchdog::after_id
            # And if the GUI is running, raise the alarm visually
            if { $synth::flag_gui } {
                watchdog::gui_alarm
            }
        }
    }

    # When the eCos application has exited, cancel the timer and
    # clean-up the GUI. Also get rid of the open file since the
    # /proc/<pid>/stat file is no longer meaningful
    proc exit_hook { arg_list } {
        if { "" != $watchdog::after_id } {
            after cancel $watchdog::after_id
        }
        if { $synth::flag_gui } {
            watchdog::gui_cancel
        }
        close $watchdog::proc_stat
    }
    synth::hook_add "ecos_exit" watchdog::exit_hook
    
    proc handle_request { id reqcode arg1 arg2 reqdata reqlen reply_len } {
        if { 0x01 == $reqcode } {
            # A "start" request. If the watchdog has already started,
            # this request is a no-op. Otherwise a timer is enabled.
            # This is made to run almost immediately, so that the
            # GUI gets a quick update. Setting the reset_received flag
            # ensures that the watchdog will not trigger immediately
            set watchdog::reset_received 1
            if { "" == $watchdog::after_id } {
                set watchdog::after_id [after 1 watchdog::update]
            }
            if { $synth::flag_gui } {
                .watchdog lower $watchdog::asleep $watchdog::background
            }
        } elseif { 0x02 == $reqcode } {
            # A "reset" request. Just set a flag, the update handler
            # will detect this next time it runs.
            set watchdog::reset_received 1
        }
    }
    
    proc instantiate { id name data } {
        return watchdog::handle_request
    }
}

if { $watchdog::init_ok } {
    return watchdog::instantiate
} else {
    synth::report "Watchdog cannot be instantiated, initialization failed.\n"
    return ""
}
