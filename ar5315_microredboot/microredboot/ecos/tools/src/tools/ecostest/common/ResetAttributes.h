//####COPYRIGHTBEGIN####
//                                                                          
// ----------------------------------------------------------------------------
// Copyright (C) 1998, 1999, 2000 Red Hat, Inc.
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
//=================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     sdf
// Contributors:  sdf
// Date:          1999-04-01
// Description:   Holds information related to target reset.
// Usage:
//
//####DESCRIPTIONEND####

#ifndef _RESETATTRIBUTES_H
#define _RESETATTRIBUTES_H

#include "Collections.h"
#include "eCosStd.h"
#include "eCosSerial.h"
#include "eCosSocket.h"
#include "Properties.h"

//=================================================================
// This class deals with resetting a target.
// The ctor accepts the "reset string" this is something like
//    expect($T05,#) 3(off(ginga:5000,a1) delay(2000) on(,..com2,38400))
// and is parsed to generate the commands to reset the target.
// The syntax is:
//    resetstring :== [directive | <integer>(resetstring)] * 
//    directive   :== id(arg[,arg]*)
//    directive   :== port | action | delay | expect
//
// The <integer>(string) syntax repeats argument its until reset is achieved.
// Each directive is given by example below:
// expect: 
//   Expected string(s).            e.g. expect(str1,str2,...).
//   Arguments:
//     The string(s) expected to be output by a target after reset.
//     *all* supplied strings must be present *in order* in the reset string for a reset to be recognized as valid.
// port:
//   Provide port information to apply to all subsequent off|on|on_off|off_on actions until overwritten.
//   e.g. port(com1,38400,1000)
//   Arguments:
//     0. Port
//     1. Baud
//     2. Read timeout
//   These have the meanings described under off|on|on_off|off_on.
// off|on|on_off|off_on:
//   Request a reset host to perform a "power cycle" (or its logical equivalent - 
//   the meanings of "off" and "on" may depend on the reset host)
//   e.g. off_on(ginga:500,A4,com1,38400,10000,1000)
//   Arguments:
//     0. Reset host:port
//     1. Control string (meaning known to the reset host)
//     2. Port (from which to read startup output)
//     3. Baud for the above
//     4. Read timeout
//     5. Delay (between off and on or vice versa)
// delay:
//   Delay for a given time right now.      
//   e.g. delay(1000)
//   Arguments:
//     0. msec to delay
//=================================================================

class CResetAttributes {
public:
  CResetAttributes(LPCTSTR psz=_T(""));

	//bool IsValid();

  const LPCTSTR Image() const { return m_str; }

  bool IsNull() const { return m_str.empty(); }
  static const CResetAttributes NoReset;

  enum ResetResult {INVALID_STRING=-1,RESET_OK=0,NOT_RESET=1,
    RESET_ILLEGAL_DEVICE_CODE=30000,RESET_NO_REPLY, RESET_BAD_CHECKSUM, RESET_BAD_ACK, RESET_UNKNOWN_ERROR};

  // Perform the reset, sending the output to the log function supplied as parameter.
  ResetResult Reset (LogFunc *pfnLog=0, void *pfnLogparam=0,bool bCheckOnly=false);

  enum Action {OFF=0,ON=1,OFF_ON=2,ON_OFF=3};

protected:

  // This function determines whether the board startup has output all that is required
  // All the strings of m_arValidResetStrings [arguments of expect() in the reset string] must be present
  bool IsValidReset();

  Time m_tResetOccurred;

  // The log function.
  LogFunc *m_pfnReset;
  void *m_pfnResetparam;
  // Log some output to the reset log function.
  void ResetLog(LPCTSTR psz);

  CeCosSerial m_Serial;
  CeCosSocket m_Socket;
  
  const TCHAR *GetIdAndArg (LPCTSTR psz,String &strID,String &strArg);
  ResetResult Parse (LPCTSTR psz,bool bCheckOnly=false);
  bool Reset (Action action,bool bCheckOutput);

  void SuckThreadFunc ();
  static void CALLBACK SSuckThreadFunc (void *pParam) { ((CResetAttributes*)pParam)->SuckThreadFunc(); }

  String m_str;           // Here is the reset string.  Could be const, but avoid the assignment operator warnings

  // These members hold the information we extract by parsing the string:

	StringArray m_arValidResetStrings;
  String m_strHostPort;     // host we talk to
  String m_strControl;      // Control string
  int m_nDelay;             // Delay between power off and power on
  String m_strAuxPort;      // Auxiliary port (serial port to listen on if primary port is TCP/IP)
  int m_nReadTimeout;       // mSec to wait for board to say something
  int m_nBaud;              // Baud rate

  String m_strResetOutput;  // The output we get
};

#endif
