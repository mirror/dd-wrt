.. SPDX-License-Identifier: GPL-2.0-only

Kernel driver iei-wt61p803-puzzle-hwmon
=======================================

Supported chips:
 * IEI WT61P803 PUZZLE for IEI Puzzle M801

   Prefix: 'iei-wt61p803-puzzle-hwmon'

Author: Luka Kovacic <luka.kovacic@sartura.hr>


Description
-----------

This driver adds fan and temperature sensor reading for some IEI Puzzle
series boards.

Sysfs attributes
----------------

The following attributes are supported:

- IEI WT61P803 PUZZLE for IEI Puzzle M801

/sys files in hwmon subsystem
-----------------------------

================= == =====================================================
fan[1-5]_input    RO files for fan speed (in RPM)
pwm[1-2]          RW files for fan[1-2] target duty cycle (0..255)
temp[1-2]_input   RO files for temperature sensors, in millidegree Celsius
================= == =====================================================

/sys files in thermal subsystem
-------------------------------

================= == =====================================================
cur_state         RW file for current cooling state of the cooling device
                     (0..max_state)
max_state         RO file for maximum cooling state of the cooling device
================= == =====================================================
