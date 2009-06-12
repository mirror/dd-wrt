/****************************************************************************
*
*	Name:			iterate.c
*
*	Description:	Runs iterations through a timeing loop for CPU utilization
*
*	Copyright:		(c) 2002 Conexant Systems Inc.
*
*****************************************************************************

  This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your option)
any later version.

  This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
more details.

  You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc., 59
Temple Place, Suite 330, Boston, MA 02111-1307 USA

****************************************************************************
*  $Author: gerg $
*  $Revision: 1.1 $
*  $Modtime: 6/05/02 3:01p $
****************************************************************************/


/****************************************************************************
	Function		: Iterate
	Description		: Performs as many iterations as possible in the given time.
	Abstract:		: This routine is used to measure how busy the CPU is.
					: It runs for a fixed period (measured by looking at the
					: timer register and/or jiffies count) and sees how many
					: iterrations of looping, incrementing a counter and watching
					: for clock edges it can get done in the fixed period of time.
					: Thus the returned count is a function of how much it is
					: preempted and the number of CPU cycles to perform one
					: iteration. This latter factor is a constant and basically
					: arbitrary so only use the returned numbers as ratios since
					: the number is not a MIP or any other standard measure and
					: could change in the future.
					: Use this routine by calling once with DisableInts set to get a
					: maximum. Then call with with DisableInts cleared to see
					: how many iterations are possible when preemption occurs.
					: Then divide the latter number by the former to get a ratio
					: of CPU loading.
					: Call both times using the same setting for DisableCache
					: for the ratio to be meaningful.
					: The ARM940T has 4KB Data and 4KB Instruction cache. Depending
					: on the task size and behavior under consideration for 
					: insertion into the spare cycles, you may wish to take your
					: two measurements with or without cacheing.
	Notes:			: The time period should not exceed approximately 2 seconds
					: or the busyness counter may overflow. This time period can
					: be extended by artificially extending the cycles consumed
					: in each iteration.
					: The time period will be converted to the nearest jiffie.
	Notes:			: Cache enable/disable is not yet supported
****************************************************************************/
unsigned long int Iterate ( unsigned short int MSecs, BOOLEAN DisableInts, BOOLEAN DisableCache ) ;


/***************************************************************************
 $Log: iterate.h,v $
 Revision 1.1  2003/06/29 14:28:18  gerg
 Import of the Conexant 82100 code (from Conexant).
 Compiles, needs to be tested on real hardware.

 * 
 * 1     6/05/02 3:07p Lewisrc
 * New file containing routine Iterate to perform as many iterations as
 * possible in the given time.
****************************************************************************/
/***** end of file $Workfile: iterate.h $ *****/
