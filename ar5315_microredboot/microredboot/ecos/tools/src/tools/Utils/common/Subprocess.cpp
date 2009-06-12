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
//===========================================================================
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s): 	sdf
// Contact(s):	sdf
// Date:		1998/08/11
// Version:		0.01
// Purpose:	
// Description:	This is the implementation of the class which allows for spawning subprocesses
//
// Requires:	
// Provides:	
// See also:    
// Known bugs:	
// Usage:	
//
//####DESCRIPTIONEND####
//
//===========================================================================
#include "eCosTrace.h"
#include "Subprocess.h"
#ifdef _WIN32
  #include <tlhelp32.h>

  HINSTANCE CSubprocess::hInstLib1 = VER_PLATFORM_WIN32_NT==CSubprocess::GetPlatform()?LoadLibrary(_T("PSAPI.DLL")):LoadLibrary(_T("Kernel32.DLL")) ;
  HINSTANCE CSubprocess::hInstLib2 = VER_PLATFORM_WIN32_NT==CSubprocess::GetPlatform()?LoadLibrary(_T("NTDLL.DLL")):NULL;

#endif

//#define CloseHandle(x) TRACE(_T("CSubprocess::CloseHandle %x\n"),x);::CloseHandle(x)

const unsigned int CSubprocess::PROCESS_KILL_EXIT_CODE=0xCCFFCCFF;

CSubprocess::CSubprocess(bool bAutoDelete):
  m_pfnContinue(DefaultContinuationFunc),
  m_pContinuationFuncParam(0),
  m_bAutoDelete(bAutoDelete),
  m_bThreadTerminated(true),
  m_bVerbose(false),
  m_nExitCode(-1),
  m_idProcess(0),
  m_pLogparam(0),
  m_pfnLogfunc(0),
  m_bKillThread(false)
{
#ifdef _WIN32
  m_hProcess=0;
#endif
}

CSubprocess::~CSubprocess()
{
  Kill();
  if(!CeCosThreadUtils::WaitFor(m_bThreadTerminated,1000)){
    m_bKillThread=true;
    CeCosThreadUtils::WaitFor(m_bThreadTerminated);  
  }
#ifdef _WIN32
  if(m_hProcess){
    CloseHandle(m_hProcess);
  }
#endif
}

bool CSubprocess::Run(LogFunc *pfnLog,void * pLogparam, LPCTSTR pszCmd,bool bBlock/*=true*/)
{
  bool rc;
  if(!m_bThreadTerminated){
    rc=false;
  } else {
    m_pfnLogfunc=pfnLog;
    m_pLogparam=pLogparam;
#ifdef _WIN32 
    // UNIX does it from the thread func.  WIN32 could too, but it's nice to know at the time 
    // of calling run whether the process is successfully created.
    if(m_hProcess){
      // Normally done in the dtor
      CloseHandle(m_hProcess);
    }
    rc=CreateProcess(pszCmd);
#else 
    m_strCmd=pszCmd;
    rc=true;
#endif
    if(rc){
      m_bKillThread=false;
      if(bBlock){
        // When using RunThread, the manipulation of this Boolean is taken care of.
        // Here we must do it ourselves.
        m_bThreadTerminated=false;
        ThreadFunc();
        m_bThreadTerminated=true;
      } else {
        CeCosThreadUtils::RunThread(SThreadFunc,this,&m_bThreadTerminated,String::SFormat(_T("subprocess %d read"),m_idProcess));
      }
    }
  }
  return rc;
}

#ifdef _WIN32
bool CSubprocess::CreateProcess(LPCTSTR pszCmdline)
{
  
  STARTUPINFO   si;                    // For CreateProcess call
  HANDLE        hrPipe,hwPipe,hwPipe2,m_hrPipeTemp,m_hwPipeTemp;
  // Create the anonymous pipe
  
  SECURITY_ATTRIBUTES saPipe;          // Security for anonymous pipe
  saPipe.nLength = sizeof(SECURITY_ATTRIBUTES);
  saPipe.lpSecurityDescriptor = NULL;
  saPipe.bInheritHandle = true;
  
  ::CreatePipe(&m_hrPipeTemp,&hwPipe,&saPipe,10240);
  
  // In most cases you can get away with using the same anonymous
  // pipe write handle for both the child's standard output and
  // standard error, but this may cause problems if the child app
  // explicitly closes one of its standard output or error handles. If
  // that happens, the anonymous pipe will close, since the child's
  // standard output and error handles are really the same handle. The
  // child won't be able to write to the other write handle since the
  // pipe is now gone, and parent reads from the pipe will return
  // ERROR_BROKEN_PIPE and child output will be lost. To solve this
  // problem, simply duplicate the write end of the pipe to create
  // another distinct, separate handle to the write end of the pipe.
  // One pipe write handle will serve as standard out, the other as
  // standard error. Now *both* write handles must be closed before the
  // write end of the pipe actually closes.
  
  ::DuplicateHandle(::GetCurrentProcess(),			// Source process
    hwPipe,		        // Handle to duplicate
    ::GetCurrentProcess(),   // Destination process
    &hwPipe2,	            // New handle, used as stderr by child 
    0,                     // New access flags - ignored since DUPLICATE_SAME_ACCESS
    true,                  // It's inheritable
    DUPLICATE_SAME_ACCESS);
  
  ::CreatePipe(&hrPipe,&m_hwPipeTemp,&saPipe,10240);
  

  // Create new output read handle and the input write handles, setting
  // the Properties to FALSE. Otherwise, the child inherits the
  // properties and, as a result, non-closeable handles to the pipes
  // are created.
  DuplicateHandle(GetCurrentProcess(),m_hrPipeTemp,
                       GetCurrentProcess(),
                       &m_hrPipe, // Address of new handle.
                       0,FALSE, // Make it uninheritable.
                       DUPLICATE_SAME_ACCESS);

  DuplicateHandle(GetCurrentProcess(),m_hwPipeTemp,
                       GetCurrentProcess(),
                       &m_hwPipe, // Address of new handle.
                       0,FALSE, // Make it uninheritable.
                       DUPLICATE_SAME_ACCESS);

  // Close inheritable copies of the handles we do not want to be inherited:
  CloseHandle(m_hrPipeTemp);
  CloseHandle(m_hwPipeTemp);


  memset(&si, 0, sizeof(si));
  si.cb = sizeof(si);
  
  si.hStdOutput = hwPipe;
  si.hStdError =  hwPipe2;
  si.hStdInput =  hrPipe;
  si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
  si.wShowWindow = SW_SHOW;
  
  LPCTSTR pszDir;
  if(m_strDir.empty()){
    pszDir=NULL; // current directory
  } else {
    pszDir=m_strDir;
  }
  
  PROCESS_INFORMATION pi;
  String strCmd(pszCmdline);

  String strOrigpath;
  if(!m_strPath.empty()){
	  int nSize=GetEnvironmentVariable(_T("PATH"), NULL, 0);
	  if(nSize>0){
      GetEnvironmentVariable(_T("PATH"),strOrigpath.GetBuffer(nSize),nSize);
      strOrigpath.ReleaseBuffer();
      SetEnvironmentVariable(_T("PATH"),m_strPath);
    }
  }

  bool rc=(TRUE==::CreateProcess(NULL,strCmd.GetBuffer(),NULL,NULL,true,DETACHED_PROCESS|CREATE_NEW_PROCESS_GROUP,NULL,pszDir,&si,&pi));

  if(!m_strPath.empty()){
    SetEnvironmentVariable(_T("PATH"),strOrigpath);
  }

  m_nErr=GetLastError();

  strCmd.ReleaseBuffer();
  
  if(rc){
    m_idProcess=pi.dwProcessId;
    m_hProcess=pi.hProcess;
    if(m_bVerbose){
      Output(String::SFormat(_T("*** Process %d created \"%s\"\n"),m_idProcess,pszCmdline));
    }
    TRACE(String::SFormat(_T("Process %d created \"%s\"\n"),m_idProcess,pszCmdline));
    m_nExitCode=STILL_ACTIVE;
    CloseHandle(pi.hThread);
  } else {
    m_idProcess=0;
    if(m_bVerbose){
      Output(String::SFormat(_T("*** Failed to create process \"%s\" %s\n"),pszCmdline,(LPCTSTR)ErrorString()));
    }
    TRACE(String::SFormat(_T("Failed to create process \"%s\" %s\n"),pszCmdline,(LPCTSTR)ErrorString()));
    m_nExitCode=GetLastError();
    CloseHandle(m_hrPipe);m_hrPipe=INVALID_HANDLE_VALUE;
    CloseHandle(m_hwPipe);m_hwPipe=INVALID_HANDLE_VALUE;
  }
  
  CloseHandle(hrPipe);
  CloseHandle(hwPipe);
  CloseHandle(hwPipe2);
  
  return rc;
  
}

void CSubprocess::ThreadFunc()
{

  TRACE(_T("Reading from process %d\n"),m_idProcess);
  
  DWORD dwAvail;
  
  DWORD dwExitCode;

  while (!m_bKillThread && m_pfnContinue(m_pContinuationFuncParam) && ::PeekNamedPipe(m_hrPipe, NULL, 0, 0, &dwAvail, NULL)){
//TRACE(_T("P%d\n"),dwAvail);
    if(dwAvail){
      dwAvail=MIN(dwAvail,80); // Read a maximum of 80 characters at a time
      DWORD dwRead;
      char *buf=new char[dwAvail+1];
//TRACE(_T("R%d\n"),dwAvail);
      if(!::ReadFile(m_hrPipe, buf, dwAvail, &dwRead, NULL)){
        TRACE(_T("ReadFile returns false\n"));
        delete [] buf;
        break;
      }
      buf[dwRead]='\0';
      Output(String::CStrToUnicodeStr(buf));
      delete [] buf;
    }
    else if (!ProcessAlive())
    {
        TRACE(_T("m_bThreadTerminated=%d\n"),m_bThreadTerminated);
        break;
    }
    // Fix for hanging in an endless loop under Windows ME, by Bill Diehls <billabloke@yahoo.com>
    else if (::GetExitCodeProcess(m_hProcess, &dwExitCode) && dwExitCode!=STILL_ACTIVE) 
    {
		break;
    }
    else
    {
      CeCosThreadUtils::Sleep(250);
    }
  }

  ::GetExitCodeProcess(m_hProcess, &dwExitCode);
  m_nExitCode=dwExitCode;
  
#ifdef _DEBUG
  String str;
  switch(dwExitCode){
    case STILL_ACTIVE:
      str=_T("still alive");
      if(m_bKillThread){
        str+=_T(" - requested to stop reading");
      }
      break;
    case PROCESS_KILL_EXIT_CODE:
      str=_T("killed");
      break;
    default:
      str.Format(_T("terminated rc=%d"),dwExitCode);
      break;
  }
  TRACE(_T("Finished reading from process %d (%s)\n"),m_idProcess,(LPCTSTR)str);
#endif

  CloseHandle(m_hrPipe);m_hrPipe=INVALID_HANDLE_VALUE;
  CloseHandle(m_hwPipe);m_hwPipe=INVALID_HANDLE_VALUE;
  
  if(m_bAutoDelete){
    m_bThreadTerminated=true; // or else the dtor will block
    delete this;
  }
}

#else // UNIX

bool CSubprocess::CreateProcess(LPCTSTR pszCmdline)
{
  m_idProcess=0;
  int fdchild=-1; // the file descriptor for the child (slave) half of the pseudo-tty pair

  // Get a free /dev/ptyp0 (master) and /dev/ttyp0 (slave) tty pair
  String strMasterTty,strChildTty;
  for(unsigned int c=0;c<64;c++){
    strMasterTty.Format("/dev/pty%c%x",'p'+c/16,c%16);
    
    m_tty=open(strMasterTty, O_RDWR | O_NOCTTY);
    if (-1!=m_tty) { 
      strChildTty.Format("/dev/tty%c%x",'p'+c/16,c%16);	
      
      fdchild = open(strChildTty, O_RDWR);
      if (-1==fdchild) {
        close(m_tty);
        m_tty=fdchild=-1;
      } else {
        VTRACE("opened %s - fd=%d\n",(LPCTSTR)strMasterTty,m_tty);
        break;
      }
    }
  }

  if(-1==m_tty){
    ERROR(_T("Failed to get a pty\n"));
    return false;
  }  
  
  TRACE(_T("Master pty %s (fd %d) slave pty %s (fd %d)\n"),(LPCTSTR)strMasterTty,m_tty,(LPCTSTR)strChildTty,fdchild);

  m_idProcess=fork();
  
  switch (m_idProcess) {
    // Fork failed
    case -1:
      TRACE(_T("Failed to create process - %s\n"),strerror(errno));
      m_idProcess=0;
      break;
    case 0:
      // Process is created (we're the child)
      {
        // Close all descriptors except the slave side of the pseudo-terminal
        for (int fd = 0; fd < (int) sysconf(_SC_OPEN_MAX); fd++) {
          if(fd!=fdchild){
            close(fd);
          }
        }
        setsid();
        
        dup2(fdchild, 0);
        dup2(fdchild, 1);
        dup2(fdchild, 2);
        
        close(fdchild);

        if(!m_strDir.empty()){
          if(0!=chdir(m_strDir)){
            if(m_bVerbose){
              fprintf(stderr,_T("*** Failed to change directory to %s\n"),(LPCTSTR)m_strDir);
            }
            exit (5);
          }
        }
        if(m_bVerbose){
          fprintf(stderr,_T("*** Process %d created \"%s\"\n"),m_idProcess,pszCmdline);
        }

        StringArray ar;
        int argc=String(pszCmdline).Chop(ar,_TCHAR(' '),true);
        TCHAR **argv=new TCHAR *[1+argc];
        for(int i=0;i<argc;i++){
          argv[i]=new TCHAR[1+strlen(ar[i])];  
          strcpy(argv[i],ar[i]);
        }
        argv[argc]=0;
        if(!m_strPath.empty()){
          _tputenv(String::SFormat(_T("PATH=%s"),(LPCTSTR)m_strPath));
        }
        _texecvp(argv[0], argv);  
      }

      fprintf(stderr,"exec error - %s\n",strerror(errno));
      exit(4);

    default:
      // Process is created (we're the parent)
      TRACE(_T("Closing fd %d\n"),fdchild);
      close(fdchild);
      TRACE(_T("Forked to create process %s - process id <%d>\n"), pszCmdline, m_idProcess);
      break;
  }
  return 0!=m_idProcess;
}

void CSubprocess::ThreadFunc()
{
  if(!CreateProcess(m_strCmd)){
    ERROR(_T("Failed to create process for %s\n"),(LPCTSTR)m_strCmd);
  } else {  
    fcntl(m_tty,F_SETFL,O_NONBLOCK);
    int rc;
    do {
      TCHAR buf[4096];
      rc=read(m_tty, buf, sizeof(buf)-1);
      if(rc>=0){
        buf[rc]='\0';
      }
      switch(rc){
        case -1:
          if(EAGAIN==errno){
            CeCosThreadUtils::Sleep(250);
          } else {
              goto Done;
          }
          break;  
        case 0:
          goto Done;
          continue;
        default:
          buf[rc]=_TCHAR('\0');
          Output(String::CStrToUnicodeStr(buf));
          continue;
      }
    } while(!m_bKillThread && m_pfnContinue(m_pContinuationFuncParam));
Done:
    TRACE(_T("Closing fd %d\n"),m_tty);
    close (m_tty);
  
    switch(waitpid(m_idProcess,&m_nExitCode,WNOHANG));
  }
  
  if(m_bAutoDelete){
    delete this;
  }
}
#endif

void CSubprocess::Output (LPCTSTR psz)
{
  m_pfnLogfunc(m_pLogparam,psz);
}

void CSubprocess::Send(LPCTSTR str)
{
  char *psz=String(str).GetCString();
  int nToWrite=strlen(psz);
  const char *c=psz;
  do {
#ifdef _WIN32
    DWORD dwWritten;
    if(!::WriteFile(m_hwPipe,psz,nToWrite,&dwWritten,0)){
      break;
    }
#else
    int dwWritten = write(m_tty, c, nToWrite);
    if(-1==dwWritten){
      break;
    } 
#endif
    nToWrite-=(int)dwWritten;
    c+=(int)dwWritten;
  } while (nToWrite>0);
  //::FlushFileBuffers(m_hwPipe);
  delete [] psz;
}

bool CSubprocess::Kill(bool bRecurse)
{
TRACE(_T("CSubprocess::Kill pid %d recurse=%d\n"),m_idProcess,bRecurse);
  PInfoArray arPinfo;
  bool rc=false;
  if(m_idProcess && -1!=m_idProcess){
    // Start of with the easy one:
    if(bRecurse) {
      // Need to gather this information before we orphan our grandchildren:
      PSExtract(arPinfo);
    }
    
#ifdef _WIN32

    if(m_hProcess){
      TRACE(_T("Terminate process %s\n"),(LPCTSTR)Name(m_idProcess));
      rc=(TRUE==::TerminateProcess(m_hProcess,PROCESS_KILL_EXIT_CODE));
      // dtor's (or subsequent Run's) responsibility to close the handle
    }

#else
    rc=(0==kill(m_idProcess,SIGTERM));
    int status;
    waitpid(m_idProcess,&status,WNOHANG);
#endif
    
    if(bRecurse) {
      // kill process *and* its children
      // FIXME: needs to be top-down
      for(int i=0;i<(signed)arPinfo.size();i++){
        if(arPinfo[i].IsChildOf(m_idProcess)){
      
#ifdef _WIN32
          // begin hack
          const String strName(Name(arPinfo[i].PID));
          if(_tcsstr(strName,_T("eCosTest")) || _tcsstr(strName,_T("cmd.EXE")) || _tcsstr(strName,_T("CMD.EXE")) || arPinfo[i].PID==(signed)GetCurrentProcessId()){
            continue;
          }
          // end hack
          HANDLE hProcess=::OpenProcess(PROCESS_TERMINATE,false,arPinfo[i].PID);
          if(hProcess){
            TRACE(_T("Terminate process %s\n"),(LPCTSTR)Name(arPinfo[i].PID));
            rc&=(TRUE==::TerminateProcess(hProcess,PROCESS_KILL_EXIT_CODE));
            CloseHandle(hProcess);
          } else {
            rc=false;
          }
#else
          rc&=(0==kill(arPinfo[i].PID,SIGTERM));
          int status;
          waitpid(arPinfo[i].PID,&status,WNOHANG);
#endif
        }
      }
    }
  }
  return rc;
}

Time CSubprocess::CpuTime(bool bRecurse) const
{
  Time t=0;
  // kill process *and* its children
  // FIXME: needs to be top-down
  
#ifdef _WIN32
  __int64 ftCreation,ftExit,ftKernel,ftUser;
  if(m_hProcess && ::GetProcessTimes (m_hProcess,(FILETIME *)&ftCreation,(FILETIME *)&ftExit,(FILETIME *)&ftKernel,(FILETIME *)&ftUser)){
    t+=Time((ftKernel+ftUser)/10000);
  }

  if(bRecurse){
    PInfoArray arPinfo;
    PSExtract(arPinfo);
    if(m_idProcess && -1!=m_idProcess){
      for(int i=0;i<(signed)arPinfo.size();i++){
        if(arPinfo[i].IsChildOf(m_idProcess)){
          t+=arPinfo[i].tCpu;
        }
      }
    }
  }
#else
  PInfoArray arPinfo;
  PSExtract(arPinfo);
  for(int i=0;i<(signed)arPinfo.size();i++){
    if(arPinfo[i].PID==m_idProcess || arPinfo[i].IsChildOf(m_idProcess)){
      t+=arPinfo[i].tCpu;
    }
  }
#endif
  return t;
}

#ifdef _WIN32
bool CSubprocess::PSExtract(CSubprocess::PInfoArray &arPinfo)
{
  bool rc=false;
  arPinfo.clear();
  // If Windows NT:
  switch(GetPlatform()) {
  case VER_PLATFORM_WIN32_NT:
    if(hInstLib1) {
      
      // Get procedure addresses.
      static BOOL (WINAPI *lpfEnumProcesses)( DWORD *, DWORD cb, DWORD * ) = (BOOL(WINAPI *)(DWORD *,DWORD,DWORD*))GetProcAddress( hInstLib1, "EnumProcesses" ) ;
      if( lpfEnumProcesses) {
        
        if(hInstLib2) {
          
          static DWORD (WINAPI *lpfNtQueryInformationProcess)( HANDLE, int, void *, DWORD, LPDWORD ) =
            (DWORD(WINAPI *)(HANDLE, int, void *, DWORD, LPDWORD)) GetProcAddress( hInstLib2,"NtQueryInformationProcess" ) ;
          
          if(lpfNtQueryInformationProcess){
            DWORD dwMaxPids=256;
            DWORD dwPidSize;
            DWORD *arPids = NULL ;
            do {
              delete [] arPids;
              arPids=new DWORD[dwMaxPids];
            } while(lpfEnumProcesses(arPids, dwMaxPids, &dwPidSize) && dwPidSize/sizeof(DWORD)==dwMaxPids) ;
            
            if(dwPidSize/sizeof(DWORD)<dwMaxPids){
              rc=true;
              for( DWORD dwIndex = 0 ; (signed)dwIndex < dwPidSize/sizeof(DWORD); dwIndex++ ) {
                // Regardless of OpenProcess success or failure, we
                // still call the enum func with the ProcID.
                DWORD pid=arPids[dwIndex];
                HANDLE hProcess=::OpenProcess( PROCESS_QUERY_INFORMATION, FALSE, pid ); 
                if (hProcess ) {
                  struct {
                    DWORD ExitStatus; // receives process termination status
                    DWORD PebBaseAddress; // receives process environment block address
                    DWORD AffinityMask; // receives process affinity mask
                    DWORD BasePriority; // receives process priority class
                    ULONG UniqueProcessId; // receives process identifier
                    ULONG InheritedFromUniqueProcessId; // receives parent process identifier
                  } pbi;
                  memset( &pbi, 0, sizeof(pbi)); 
                  DWORD retLen; 
                  __int64 ftCreation,ftExit,ftKernel,ftUser;
                  if(lpfNtQueryInformationProcess(hProcess, 0 /*ProcessBasicInformation*/, &pbi, sizeof(pbi), &retLen)>=0 &&
                    TRUE==::GetProcessTimes (hProcess,(FILETIME *)&ftCreation,(FILETIME *)&ftExit,(FILETIME *)&ftKernel,(FILETIME *)&ftUser)){
                    // The second test is important.  It excludes orphaned processes who appear to have been adopted by virtue of a new
                    // process having been created with the same ID as their original parent.
                    PInfo p;
                    p.PID=pid;
                    p.PPID=pbi.InheritedFromUniqueProcessId;
                    p.tCreation=ftCreation;
                    p.tCpu=Time((ftKernel+ftUser)/10000);
                    arPinfo.push_back(p);
                  }
                  
                  CloseHandle(hProcess); 

                }
              }
            }
            delete [] arPids;
          }          
        }
      }      
    }
    break;
  case VER_PLATFORM_WIN32_WINDOWS:
    
    if( hInstLib1) {
      
      static HANDLE (WINAPI *lpfCreateToolhelp32Snapshot)(DWORD,DWORD)=
        (HANDLE(WINAPI *)(DWORD,DWORD))GetProcAddress( hInstLib1,"CreateToolhelp32Snapshot" ) ;
      static BOOL (WINAPI *lpfProcess32First)(HANDLE,LPPROCESSENTRY32)=
        (BOOL(WINAPI *)(HANDLE,LPPROCESSENTRY32))GetProcAddress( hInstLib1, "Process32First" ) ;
      static BOOL (WINAPI *lpfProcess32Next)(HANDLE,LPPROCESSENTRY32)=
        (BOOL(WINAPI *)(HANDLE,LPPROCESSENTRY32))GetProcAddress( hInstLib1, "Process32Next" ) ;
      if( lpfProcess32Next && lpfProcess32First && lpfCreateToolhelp32Snapshot) {
        
        // Get a handle to a Toolhelp snapshot of the systems
        // processes.
        HANDLE hSnapShot = lpfCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0) ;
        if(INVALID_HANDLE_VALUE != hSnapShot) {
          // Get the first process' information.
          PROCESSENTRY32 procentry;
          procentry.dwSize = sizeof(PROCESSENTRY32) ;
          if(lpfProcess32First( hSnapShot, &procentry )){
            rc=true;
            do {
              PInfo p;
              p.PID=procentry.th32ProcessID;
              p.PPID=procentry.th32ParentProcessID;
              arPinfo.push_back(p);
            } while(lpfProcess32Next( hSnapShot, &procentry ));
          }
          CloseHandle(hSnapShot);
        }
      }
    }
    break;
  default:
    break;
  }    

  SetParents(arPinfo);

  if(!rc){
    ERROR(_T("Couldn't get process information!\n"));
  }
  return rc;
}

#else // UNIX

bool CSubprocess::PSExtract(CSubprocess::PInfoArray &arPinfo)
{
  arPinfo.clear();
  int i;
  FILE *f=popen("ps -l",_T("r") MODE_TEXT);
  if(f){
    char buf[100];
    while(fgets(buf,sizeof(buf)-1,f)){
      TCHAR discard[100];
      PInfo p;
      // Output is in the form
      //  F S   UID   PID  PPID  C PRI  NI ADDR    SZ WCHAN  TTY          TIME CMD
      //100 S   490   877   876  0  70   0    -   368 wait4  pts/0    00:00:00 bash
      int F,UID,C,PRI,NI,SZ,HH,MM,SS; 
      bool rc=(15==_stscanf(buf,_T("%d %s %d %d %d %d %d %d %s %d %s %s %d:%d:%d"),&F,discard,&UID,&p.PID,&p.PPID,&C,&PRI,&NI,discard,&SZ,discard,discard,&HH,&MM,&SS));
      if(rc){
        p.tCpu=1000*(SS+60*(60*HH+MM));
        arPinfo.push_back(p);
      }
    }
    pclose(f);
    for(i=0;i<(signed)arPinfo.size();i++){
      int pid=arPinfo[i].PPID;
      arPinfo[i].pParent=0;
      for(int j=0;j<(signed)arPinfo.size();j++){
        if(i!=j && arPinfo[j].PID==pid){
          arPinfo[i].pParent=&arPinfo[j];
          break;
        }
      }
    }
  } else {
    ERROR(_T("Failed to run ps -l\n"));
  }
  return true; //FIXME
}

#endif

void CSubprocess::SetParents(CSubprocess::PInfoArray &arPinfo)
{
  int i;
  for(i=0;i<(signed)arPinfo.size();i++){
    PInfo &p=arPinfo[i];
    p.pParent=0;
    for(int j=0;j<(signed)arPinfo.size();j++){
      if(arPinfo[j].PID==p.PPID 
#ifdef _WIN32
        && arPinfo[j].tCreation<p.tCreation
#endif
        )
      {
        arPinfo[i].pParent=&arPinfo[j];
        break;
      }
    }
  }

  // Check for circularity
  bool bCircularity=false;
  for(i=0;i<(signed)arPinfo.size();i++){
    PInfo *p=&arPinfo[i];
    for(int j=0;j<(signed)arPinfo.size() && p;j++){
      p=p->pParent;
    }
    // If all is well, p should be NULL here.  Otherwise we have a loop.
    if(p){
      // Make sure it can't foul things up:
      arPinfo[i].pParent=0;
      bCircularity=true;
    }
  }
  
  if(bCircularity){
    ERROR(_T("!!! Circularly linked process list at index %d\n"),i);
    for(int k=0;k<(signed)arPinfo.size();k++){
      const PInfo &p=arPinfo[k];
      ERROR(_T("%d: %s ppid=%4d\n"),k,(LPCTSTR)Name(p.PID),p.PPID);
    }
  }
}

bool CSubprocess::PInfo::IsChildOf(int pid) const
{
  for(PInfo *p=pParent;p && p!=this;p=p->pParent) { // guard against circular linkage
    if(p->PID==pid){
      return true;
    }
  }
  return false;
}

const String CSubprocess::Name(int pid)
{
  String str(String::SFormat(_T("id=%d"),pid));
#ifdef _DEBUG
#ifdef _WIN32
  if(VER_PLATFORM_WIN32_NT==GetPlatform() && hInstLib1){
    static BOOL (WINAPI *lpfEnumProcessModules)( HANDLE, HMODULE *, DWORD, LPDWORD ) =
      (BOOL(WINAPI *)(HANDLE, HMODULE *, DWORD, LPDWORD)) GetProcAddress( hInstLib1,"EnumProcessModules" ) ;
    static DWORD (WINAPI *lpfGetModuleFileNameEx)( HANDLE, HMODULE, LPTSTR, DWORD )=
      (DWORD (WINAPI *)(HANDLE, HMODULE,LPTSTR, DWORD )) GetProcAddress( hInstLib1,"GetModuleFileNameExA" ) ;
    if( lpfEnumProcessModules &&  lpfGetModuleFileNameEx ) {
      HANDLE hProcess=::OpenProcess(PROCESS_ALL_ACCESS,false,pid);
      if(hProcess) {
        HMODULE hMod;
        DWORD dwSize;
        if(lpfEnumProcessModules( hProcess, &hMod, sizeof(HMODULE), &dwSize ) ){
          // Get Full pathname:
          TCHAR buf[1+MAX_PATH];
          lpfGetModuleFileNameEx( hProcess, hMod, buf, MAX_PATH);
          str+=_TCHAR(' ');
          str+=buf;
        }
        CloseHandle(hProcess);
      }
    }
  }
#endif
#endif
  return str;
}

#ifdef _WIN32
DWORD CSubprocess::GetPlatform()
{
  OSVERSIONINFO  osver;
  osver.dwOSVersionInfoSize = sizeof( osver ) ;
  return GetVersionEx( &osver ) ? osver.dwPlatformId : (DWORD)-1;
}
#endif

bool CSubprocess::ProcessAlive()
{
  return !m_bThreadTerminated;
}

void CSubprocess::CloseInput()
{
#ifdef _WIN32
  CloseHandle(m_hwPipe);m_hwPipe=INVALID_HANDLE_VALUE;
#else
  close(m_tty);
#endif
}

bool CSubprocess::Wait(Duration dTimeout)
{
  return CeCosThreadUtils::WaitFor(m_bThreadTerminated,dTimeout);
}

const String CSubprocess::ErrorString() const
{
#ifdef _WIN32
  TCHAR *pszMsg;
  FormatMessage(  
    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
    NULL,
    m_nErr,
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
    (LPTSTR)&pszMsg,
    0,
    NULL 
    );
  return pszMsg;
#else 
  return strerror(errno);
#endif
}

