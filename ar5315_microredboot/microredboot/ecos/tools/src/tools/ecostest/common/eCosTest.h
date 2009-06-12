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
//        eCosTest.h
//
//        run test header
//
//=================================================================
//=================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     sdf
// Contributors:  sdf
// Date:          1999-04-01
// Description:   Run one or more tests
// Usage:
//
//####DESCRIPTIONEND####

#ifndef _ECOSTEST_H
#define _ECOSTEST_H

//=================================================================
// This class represents a single eCos test [executable].
// It includes member functions to run the test and to manage related system resources.
//=================================================================

#include "Collections.h"
#include "eCosStd.h"
#include "eCosTestPlatform.h"
#include "eCosTestUtils.h"
#include "ResetAttributes.h"

class CSubprocess;
class CTestResource;
class CeCosSocket;

class CeCosTest{
public:

  static LPCTSTR pszFormat;
  
  ///////////////////////////////////////////////////////////////////////////
  // Representation of an elapsed time (units of milliseconds)
  enum {NOTIMEOUT=0x7fffffff}; // No timeout specified
  
  ///////////////////////////////////////////////////////////////////////////
  // ctors, dtors and their friends
  class ExecutionParameters;
  CeCosTest(const ExecutionParameters &e, LPCTSTR  const pszExecutable, LPCTSTR  const pszTitle=0);
  virtual ~CeCosTest();
  // Count of number of instances of this class:
  static int InstanceCount;
  // Delete all heap instances of this class (*must* be allocated on heap)
  static void DeleteAllInstances ();
  // Simply wait for instances to die a natural death
  static bool WaitForAllInstances (int nPoll=1000,Duration nTimeout=NOTIMEOUT);
  // Tap them on the shoulder (does not wait)
  static void CancelAllInstances ();
  ///////////////////////////////////////////////////////////////////////////
  
  ///////////////////////////////////////////////////////////////////////////
  // Class used to represent execution parameters (to be passed with request to execute a test)
  ///////////////////////////////////////////////////////////////////////////
  class ExecutionParameters {
  public:
    
    enum RequestType { RUN, QUERY, LOCK, UNLOCK, STOP, RequestTypeMax};
    static RequestType RequestTypeValue(LPCTSTR );
    static const String Image(RequestType r) { return (r>=0 && r<=RequestTypeMax)?String(arRequestImage[r]):String::SFormat(_T("Unknown(%d)"),r); }
    
    Duration    ActiveTimeout()   const { return m_nActiveTimeout; }
    Duration    DownloadTimeout() const { return m_nDownloadTimeout; }

    const CeCosTestPlatform *Platform() const { return CeCosTestPlatform::Get(m_Target); }
    LPCTSTR PlatformName() const { return Platform()?Platform()->Name():_T("UNKNOWN"); }

    RequestType Request() const { return m_Request;}
    void SetActiveTimeout   (Duration t){m_nActiveTimeout=t;}
    void SetDownloadTimeout (Duration t){m_nDownloadTimeout=t;}

    ExecutionParameters (
      RequestType r=CeCosTest::ExecutionParameters::RUN,
      LPCTSTR  Target=_T(""),
      Duration    nActiveTimeout=NOTIMEOUT,
      Duration    nDownloadTimeout=NOTIMEOUT);
    bool FromStr(LPCTSTR psz);
    
    String Image() const;
    virtual ~ExecutionParameters(){}
    bool m_bUseFilter;
  protected:
    static LPCTSTR  arRequestImage [1+RequestTypeMax];
    String m_Target;
    Duration m_nActiveTimeout,m_nDownloadTimeout;
    RequestType m_Request;
    int  m_nUnused1;
    int  m_nUnused2;
    int  m_nUnused3;
    bool m_bUnused2;
    bool m_bUnused3;
  };
  ///////////////////////////////////////////////////////////////////////////
  
  ///////////////////////////////////////////////////////////////////////////
  // Result status stuff.
  // Order is important - SetStatus can only change status in left-to-right direction
  void AnalyzeOutput();
  enum StatusType {NotStarted, NoResult, Inapplicable, Pass, DownloadTimeOut, TimeOut, Cancelled, Fail, 
    AssertFail, StatusTypeMax};
  static StatusType StatusTypeValue (LPCTSTR  const pszStr);
  static const String Image(StatusType s) { return (s>=0 && s<StatusTypeMax)?String(arResultImage[s]):String::SFormat(_T("Unknown(%d)"),s); }
  ///////////////////////////////////////////////////////////////////////////
  
  ///////////////////////////////////////////////////////////////////////////
  // Attributes
  LPCTSTR  const      Executable()             const { return m_strExecutable;}            // Executable name
  
  StatusType          Status()                 const { return m_Status; }                  // Test status
  
  Duration            Download()               const { return m_nDownloadTime; }           // Download time
  Duration            Total()                  const { return m_nTotalTime; }              // Total
  Duration            MaxInactive()            const { return m_nMaxInactiveTime; }        // Max. inactive
  
  LPCTSTR  const  Output()                     const { return m_strOutput; }               // Output generated by a test run [for report purposes]
  //const CTestResource * const Port()           const { return m_pResource; }               // Resource used for a test run [for report purposes]
  
  const String ResultString (bool bIncludeOutput=true)             const;
  ///////////////////////////////////////////////////////////////////////////
  
  ///////////////////////////////////////////////////////////////////////////
  // Running a test:
  
  // Run a test locally:
  bool RunLocal ();
  
  // Run a test remotely: 
  // If pszRemoteHostPort is given, it sends the test for execution on the given host:post.
  // Otherwise, a suitable host:port is determined from the test resource information.
  bool RunRemote (LPCTSTR  const pszRemoteHostPort);
  ///////////////////////////////////////////////////////////////////////////
  
  ///////////////////////////////////////////////////////////////////////////
  // Resource functions
  
  // Run as a server on given TCP/IP port
  static bool RunAgent(int nTcpPort); 
  
  bool InteractiveInferior(LPCTSTR pszHostPort,TCHAR **argv);
  void SetTimeouts (Duration dActive=NOTIMEOUT,Duration dDownload=NOTIMEOUT/*,Duration dElapsed=15*60*1000*/);
  void SetExecutable (LPCTSTR pszExecutable);
  static bool Value (
    LPCTSTR pszStr, 
    struct tm &t,
    StatusType &status,
    String &target,
    String &strExecutionHostPort,
    String &strExecutableTail,
    String &strTitle,
    int &nFileSize,
    Duration &nTotalTime,
    Duration &nMaxInactiveTime,
    Duration &nDownloadTime,
    Duration &nDownloadTimeout,
    Duration &nActiveTimeout,
    int &nDownloadedSize);
  
  enum ServerStatus {SERVER_BUSY, SERVER_READY, SERVER_CANT_RUN, CONNECTION_FAILED, SERVER_LOCKED, ServerStatusMax};
  static LPCTSTR  const Image(ServerStatus s) { return arServerStatusImage[MIN(s,ServerStatusMax)]; }
  static ServerStatus ServerStatusValue(LPCTSTR psz);

  // Force the result to change.  Generally you can only set the result left-to-right in the order of the enumeration literals
  void ForceResult(StatusType s) { m_Status=s; }

  // Get size information (generally by running *gdb-size)
  bool GetSizes();

  // Connect to a test server
  static ServerStatus CeCosTest::Connect (LPCTSTR pszHostPort, CeCosSocket *&pSock, const ExecutionParameters &e,String &strInfo,Duration dTimeout=10*1000);

  // Log some output.  The accumulated output can be retrieved using Output()
  void Log (LPCTSTR  const pszFormat,...);
  void LogString (LPCTSTR psz);

protected:
	int m_nOutputLen;

  void Cancel (); // Stop the run

  // Connect to m_strExecutionHostPort
  void ConnectForExecution ();
  
  // Callback used when EXEC output is used to create host-side processes
  static void CALLBACK AppendFunc(void *pParam,LPCTSTR psz) { ((CeCosTest*)pParam)->Log(_T("%%%% %s"),psz);}
  
	void GetInferiorCommands (StringArray &arstrInferiorCmds);
  CSubprocess *m_pspPipe;
  PtrArray m_arpExecsp;

  // Extract a directive (such as "EXEC:") from the test output.  The index passed keeps track of the last such
  // directive extracted such that successive calls march through the output, returning a different one each time.
	bool GetDirective (LPCTSTR pszDirective, String &str,int &nIndex);
	
  // Keep track of directives in the gdb output
  int m_nLastGdbInst;     // GDB
  int m_nLastExecInst;    // EXEC
  int m_nLastTimeoutInst; // TIMEOUT
  int m_nLastPipeInst;    // PIPE   

  // Commands to be sent to gdb, or other inferior process
  StringArray m_arstrInferiorCmds;
  // Last command sent
	unsigned int m_nCmdIndex;

  // Inferior process output comes through here
  static void CALLBACK SInferiorOutputFunc(void *pParam,LPCTSTR psz) { ((CeCosTest *)pParam)->InferiorOutputFunc(psz); }
  void InferiorOutputFunc(LPCTSTR psz);

  // Has a timeout occurred?  Returns false if so.
  static bool CALLBACK SCheckForTimeout(void *pParam) { return ((CeCosTest *)pParam)->CheckForTimeout(); }

  // Read target ready indicator from server
  bool GetTargetReady(String &strHostPort);
  
  // This may be set to force the socket-to-serial connection to terminate
  bool m_bStopConnectSocketToSerial;
  
  // Convert a path to one understandable by Cygwin
  static String CygPath (LPCTSTR pszPath);

  // Are we at a prompt?
  bool AtPrompt();
  
  // To limit calls to ps, under UNIX.  This prevents the acquisition of cpu time consuming the whole machine :-).
  mutable Time m_tInferiorCpuTime;
  mutable Time m_tPrevSample;
  Time InferiorTime() const;
  
  unsigned int m_nStrippedSize;

  // Path to use to execute subprocesses
  String m_strPath;
  
  // Size of executable
  unsigned int m_nFileSize;                   
  
  // host:port we are connecting to
  String m_strExecutionHostPort;
  
  ///////////////////////////////////////////////////////////////////////////
  // Stuff to manage running gdb (or some other inferior process)
  bool CheckForTimeout();                     // Check for a timeout - set status and return false if it happens
  bool m_bDownloading;                        // Are we currently downloading executable?
  bool InferiorProcessAlive ();                    // Is gdb still alive and kicking?
  Time m_tBase;                               // Base used for measurement of timeouts
  Time m_tBase0;                              // Base used for measurement of timeouts
  Time m_tWallClock0;                         // When the test was actually started

  ///////////////////////////////////////////////////////////////////////////
  // Close the socket used by the current class instance
  void CloseSocket ();
  
  bool send(const void * const pData,unsigned int nLength,LPCTSTR  const pszMsg=_T(""),Duration dTimeout=10*1000);
  bool recv(const void *pData,unsigned int nLength,LPCTSTR  const pszMsg=_T(""),Duration dTimeout=10*1000);
  
  bool sendResult(Duration dTimeout=10*1000);
  bool recvResult(Duration dTimeout=10*1000);
  
  ///////////////////////////////////////////////////////////////////////////
  
  ///////////////////////////////////////////////////////////////////////////
  CeCosSocket *m_pSock;
  
  ExecutionParameters m_ep;
  
  // Chaining to allow *AllInstances functions to work:
  static CeCosTest * pFirstInstance;
  CeCosTest * m_pPrevInstance;
  CeCosTest * m_pNextInstance;
  
  void RunInferior (LPCTSTR pszCmdline);
  
  bool OutputContains(LPCTSTR psz) const { return 0!=_tcsstr(m_strOutput,psz); }
  
  static void CALLBACK SAcceptThreadFunc (void *pParam) {((CeCosTest *)pParam)->AcceptThreadFunc(); }
  void AcceptThreadFunc();

  static void CALLBACK SConnectSocketToSerialThreadFunc(void *pParam) { ((CeCosTest *)pParam)->ConnectSocketToSerialThreadFunc(); }
  void ConnectSocketToSerialThreadFunc();
  
  String m_strExecutable;
  String m_strTitle;
  
  void SetStatus (StatusType status);
  StatusType m_Status;
  
  Duration  m_nDownloadTime;
  Duration  m_nTotalTime;
  Duration  m_nMaxInactiveTime;
  
  CTestResource *m_pResource;
  CSubprocess *m_psp;
  
  String m_strOutput; // the output of the test run goes here

  static LPCTSTR  const arResultImage[1+StatusTypeMax];
  static LPCTSTR  const arServerStatusImage[1+ServerStatusMax];
  bool m_bConnectSocketToSerialThreadDone;
  static void CALLBACK ResetLogFunc(void *pParam, LPCTSTR psz);

  // These are used by RunAgent and its friends for setting up the RDI connection
  int m_nAuxPort;
  int m_nAuxListenSock;


}; // class CeCosTest



#endif
