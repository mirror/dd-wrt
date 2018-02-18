/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * (c) by the OLSR project
 *
 * See our Git repository to find out who worked on this file
 * and thus is a copyright holder on it.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 */

#ifdef _WIN32

#if !defined(AFX_FRONTENDDLG_H__7D68FBC0_7448_479B_81F0_3FBBDE291395__INCLUDED_)
#define AFX_FRONTENDDLG_H__7D68FBC0_7448_479B_81F0_3FBBDE291395__INCLUDED_

#if defined _MSC_VER && _MSC_VER > 1000
#pragma once
#endif /* defined _MSC_VER && _MSC_VER > 1000 */

#include "MyTabCtrl.h"
#include "NodeEntry.h"

#define PIPE_MODE_RUN 0
#define PIPE_MODE_INT 1

class CFrontendDlg:public CDialog {
public:
  CFrontendDlg(CWnd * pParent = NULL);

  //{{AFX_DATA(CFrontendDlg)
  enum { IDD = IDD_FRONTEND_DIALOG };
  CButton m_StopButton;
  CButton m_StartButton;
  MyTabCtrl m_TabCtrl;
  //}}AFX_DATA

  unsigned int LogThreadFunc(void);
  unsigned int NetThreadFunc(void);

  CString ConfigFile;
protected:

  //{{AFX_VIRTUAL(CFrontendDlg)
    virtual void DoDataExchange(CDataExchange * pDX);
  //}}AFX_VIRTUAL

public:
  //{{AFX_MSG(CFrontendDlg)
    virtual BOOL OnInitDialog();
  afx_msg void OnOK();
  afx_msg void OnCancel();
  afx_msg void OnStartButton();
  afx_msg void OnStopButton();
  afx_msg void OnExitButton();
  //}}AFX_MSG

protected:
    DECLARE_MESSAGE_MAP() int StartOlsrd(void);
  int StopOlsrd(void);

  int GetInterfaces(void);

  HANDLE Event;

  CString StoredTempFile;

  SOCKET SockHand;

  int PipeMode;
  int ExecutePipe(const char *, HANDLE *, HANDLE *, HANDLE *);

  CWinThread *LogThread;
  CWinThread *NetThread;

  CStringArray Interfaces;
  CStringArray Addresses;
  CStringArray IsWlan;

  HANDLE OutRead, InWrite;
  HANDLE ShimProc;

  void HandleIpcRoute(struct IpcRoute *);
  void HandleIpcConfig(struct IpcConfig *);
  void HandleOlsrHello(struct OlsrHello *, int);
  void HandleOlsrTc(struct OlsrTc *, int);
  void HandleOlsrMid(struct OlsrHeader *);
  void HandleOlsrHna(struct OlsrHeader *);

  void AddNode(unsigned int, unsigned int);
  void AddMpr(unsigned int, unsigned int, unsigned int);
  void AddMid(unsigned int, unsigned int, unsigned int);
  void AddHna(unsigned int, unsigned int, unsigned int, unsigned int);

    CList < class NodeEntry, class NodeEntry & >NodeList;

  void Timeout(void);

  unsigned __int64 Now;

  unsigned int LocalHost;
};

//{{AFX_INSERT_LOCATION}}

#endif /* !defined(AFX_FRONTENDDLG_H__7D68FBC0_7448_479B_81F0_3FBBDE291395__INCLUDED_) */

#endif /* _WIN32 */

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
