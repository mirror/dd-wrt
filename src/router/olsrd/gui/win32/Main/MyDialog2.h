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

#if !defined(AFX_MYDIALOG2_H__1A381668_A36B_4C51_9B79_643BC2A59D88__INCLUDED_)
#define AFX_MYDIALOG2_H__1A381668_A36B_4C51_9B79_643BC2A59D88__INCLUDED_

#if defined _MSC_VER && _MSC_VER > 1000
#pragma once
#endif /* defined _MSC_VER && _MSC_VER > 1000 */

#include "MyEdit.h"

#define MAKELIB
#define OLSR_PLUGIN
#include <olsr_cfg.h>
#include "afxwin.h"

class MyDialog2:public CDialog {
public:
  MyDialog2(CWnd * pParent = NULL);

  BOOL Create(CWnd * Parent);

  int OpenConfigFile(CString);
  int SaveConfigFile(CString, int);

  CStringArray *Interfaces;
  CStringArray *Addresses;
  CStringArray *IsWlan;

  struct olsrd_config *Conf;

  //{{AFX_DATA(MyDialog2)
  enum { IDD = IDD_DIALOG2 };
  CComboBox m_LqAlgo;
  CComboBox m_TcRed;
  CEdit m_MprCov;
  CButton m_EtxRadio2;
  CButton m_EtxRadio1;
  CButton m_EtxCheck;
  CButton m_Ipv6Check;
  CButton m_InternetCheck;
  CButton m_HystCheck;
  CButton m_FishEyeCheck;
  MyEdit m_HystThresholdHigh;
  MyEdit m_HystThresholdLow;
  MyEdit m_HystScaling;
  MyEdit m_HnaHold;
  MyEdit m_MidHold;
  MyEdit m_PollInt;
  MyEdit m_TcHold;
  MyEdit m_TcInt;
  MyEdit m_HnaInt;
  MyEdit m_MidInt;
  MyEdit m_HelloHold;
  MyEdit m_HelloInt;
  CListCtrl m_InterfaceList;
  CStatic m_DebugLevelText;
  CSliderCtrl m_DebugLevel;
  //}}AFX_DATA

  //{{AFX_VIRTUAL(MyDialog2)
protected:
    virtual void DoDataExchange(CDataExchange * pDX);
  //}}AFX_VIRTUAL

protected:

  //{{AFX_MSG(MyDialog2)
    afx_msg void OnOK();
  afx_msg void OnCancel();
  afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar * pScrollBar);
  virtual BOOL OnInitDialog();
  afx_msg void OnHystCheck();
  afx_msg void OnOpenButton();
  afx_msg void OnSaveButton();
  afx_msg void OnResetButton();
  afx_msg void OnEtxCheck();
  afx_msg void OnEtxRadio1();
  afx_msg void OnEtxRadio2();
  //}}AFX_MSG
    DECLARE_MESSAGE_MAP() CFont EditFont;

  int DebugLevel;
  void SetDebugLevel(int);

  void Reset(void);

  void OnEtxCheckWorker(void);
};

//{{AFX_INSERT_LOCATION}}

#endif /* !defined(AFX_MYDIALOG2_H__1A381668_A36B_4C51_9B79_643BC2A59D88__INCLUDED_) */

#endif /* _WIN32 */

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
