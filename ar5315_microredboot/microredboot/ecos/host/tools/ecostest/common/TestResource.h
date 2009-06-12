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
//
//        TestResource.h
//
//        Test resource class
//
//=================================================================
//=================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     sdf
// Contributors:  sdf
// Date:          1999-04-01
// Description:   This class abstracts test resources for use in the testing infrastructure
// Usage:
//
//####DESCRIPTIONEND####

#ifndef _TESTRESOURCE_H
#define _TESTRESOURCE_H

#include "Collections.h"
#include "eCosStd.h"
#include "eCosTest.h"
#include "eCosSerial.h"
#include "Properties.h"
#include "ResetAttributes.h"

// This class is used to manipulate test resources.  A test resource is the means to execute
// a test (usually an eCosTestServer running on a tcp/ip port)
class CTestResource {
public:

  // ctors/dtors
  CTestResource(
    LPCTSTR pszHostPort,  // Where the resource "lives" - i.e. the host and TCP/IP port for the server
    LPCTSTR target, 
    // Physical port characteristics.  If pszPort is null, simulator run is meant (in which case nBaud is ignored)
    LPCTSTR pszPort=0, int nBaud=0,
    // Associated reset characteristics: pszPort may be a remote server (host:port) or a local port
    LPCTSTR pszResetString=_T("")
    );
  // Not chained or fully initialized
  CTestResource():m_nBaud(0),m_bLocked(false),m_pNextInstance(0),m_pPrevInstance(0){} 
  virtual ~CTestResource();
  
  bool Use(); // Mark this resource as "in use"

  // This causes the reset to be performed (via rsh, on the machine
  CResetAttributes::ResetResult RemoteReset(LogFunc *pfnLog, void *pfnLogparam=0);
  const String Image() const;
  
  // Receive details from the socket
  static bool Load(Duration dTimeout=10*1000) { return LoadSocket(strResourceHostPort,dTimeout); }
  static bool Save(Duration dTimeout=10*1000) { return SaveSocket(strResourceHostPort,dTimeout); }
  
  CeCosTest::ServerStatus Query();

  // [Resource] locking:
  bool Unlock();
  bool Lock ();
  bool IsLocked() const { return m_bLocked; }

  static CTestResource *First() { return pFirstInstance; }
  CTestResource *Next() const { return m_pNextInstance; }
  static CTestResource * Lookup(LPCTSTR  pszHostPort);
  static unsigned int ResourceCount() { return nCount; }
  static void DeleteAllInstances();
  static bool LoadFromDirectory (LPCTSTR psz); // Load information from a set of files in given directory
  static bool SaveToDirectory   (LPCTSTR psz); // Save information, likewise
  
#ifdef _WIN32
  static bool SaveToRegistry(HKEY key,LPCTSTR pszKey);  // Save information to the registry
  static bool LoadFromRegistry(HKEY key,LPCTSTR pszKey);// Load information from the registry
#endif

  void SetInfo (LPCTSTR pszInfo) { m_strInfo=pszInfo; }
  LPCTSTR  Info() const { return m_strInfo; }
  
  // Host and TCP/IP port:
  String HostPort() const { return CeCosSocket::HostPort(m_strHost,m_nPort); }
  // Same information, but separately:
  String Host() const { return m_strHost; }
  int TcpIPPort() const { return m_nPort; }

  // Serial (comms) port:
  LPCTSTR Serial() const { return m_strPort; }
  
  LPCTSTR Target() const { return m_Target; }

  // Is there an associated reset string?
  bool HasReset() const { return !m_strReset.empty(); }
  
  // Baud rate:
  int Baud() const { return m_nBaud; }
  
  void SetHostPort(LPCTSTR pszHostPort) { CeCosSocket::ParseHostPort(pszHostPort,m_strHost,m_nPort); }
  
  void SetTarget	(LPCTSTR target) { m_Target=target; }
  void SetDownload(LPCTSTR pszDownloadPort,int nBaud) { m_strPort=pszDownloadPort; m_nBaud=nBaud; }
  void SetReset  (LPCTSTR pszReset) { m_strReset=pszReset; }
  void SetUser   (LPCTSTR pszUser,LPCTSTR pszEmail) {m_strUser=pszUser; m_strEmail=pszEmail; }
  void SetReason (LPCTSTR pszReason) {m_strReason=pszReason; }
  void SetBoardID(LPCTSTR pszBoardID){m_strBoardID=pszBoardID; }
  void SetDate   (LPCTSTR pszDate)   {m_strDate   =pszDate; }

  LPCTSTR Email  () { return m_strEmail; }
  LPCTSTR User   () { return m_strUser; }
  LPCTSTR Reason () { return m_strReason; }
  LPCTSTR BoardID() { return m_strBoardID; }
  LPCTSTR Date   () { return m_strDate; }
  
  // Scheduling functions
  // Get the array of resources capable of executing "e"
  // nCount gives the number of entries in ar on input
  // result is the number of entries required in ar (may exceed nCount, but this case does no damage)
  static unsigned int GetMatchCount (const CeCosTest::ExecutionParameters &e,bool bIgnoreLocking=false);
  static bool GetMatches (const CeCosTest::ExecutionParameters &e,StringArray &arstr,bool bIgnoreLocking=false); // as before, but callee allocates.  Deallocate using delete [].
  
  static bool SetResourceServer (LPCTSTR pszHostPort) { bool b=CeCosSocket::IsLegalHostPort(pszHostPort);if(b)strResourceHostPort=pszHostPort; return b;}
  static String GetResourceServer (){ return strResourceHostPort; }
  static bool ResourceServerSet() { return CeCosSocket::IsLegalHostPort(GetResourceServer()); }
  
  static CTestResource *GetResource(const CeCosTest::ExecutionParameters &e);
  void Release() {
    //VTRACE(_T("Release %s\n"),Serial1());
    m_bInUse=false;
  }
  
  bool InUse() const { return m_bInUse; }
  static int Count (const CeCosTest::ExecutionParameters &e);
  
  // Reset the hardware attached to this port.  Output goes to pfnLog
  CResetAttributes::ResetResult Reset(LogFunc *pfnLog=0, void *pfnLogparam=0);
  CResetAttributes::ResetResult Reset(String &str); // as above, output to string

  static bool Matches(LPCTSTR pszHostPort, const CeCosTest::ExecutionParameters &e);
  bool FromStr(LPCTSTR pszImage);
  LPCTSTR ResetString() const { return m_strReset; }

  class CTestResourceProperties : public CProperties {
  public:
    CTestResourceProperties(CTestResource *pResource);
    virtual ~CTestResourceProperties(){}
  protected:
  };

protected:
	
  friend class CTestResourceProperties;
  
  String FileName() const;
  String m_strReason,m_strUser,m_strEmail,m_strBoardID, m_strDate;

  String m_strReset;

  static void CALLBACK StringLogFunc (void *pParam,LPCTSTR psz);

  static bool LoadSocket (LPCTSTR pszHostPort,Duration dTimeout=10*1000);
  static bool SaveSocket (LPCTSTR pszHostPort,Duration dTimeout=10*1000);
  static LPCTSTR szFormat;
  
  void Chain();
  
  unsigned static int nCount;
  static String strResourceHostPort;
  static CTestResource *pFirstInstance;
  
  bool Matches  (const CeCosTest::ExecutionParameters &e,bool bIgnoreLocking=false) const;
  

  String m_strInfo;
  bool m_bFlag;
  bool m_bInUse;
  int  m_nBaud;
  String m_strPort;
  
  bool m_bLocked;
  
  String m_Target;
  String m_strHost;
  int m_nPort;
  CTestResource *m_pNextInstance;
  CTestResource *m_pPrevInstance;
};
#endif
