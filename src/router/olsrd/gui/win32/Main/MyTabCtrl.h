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

#if !defined(AFX_MYTABCTRL_H__D443FF52_C52D_4C89_AB4B_19B09687EBAE__INCLUDED_)
#define AFX_MYTABCTRL_H__D443FF52_C52D_4C89_AB4B_19B09687EBAE__INCLUDED_

#if defined _MSC_VER && _MSC_VER > 1000
#pragma once
#endif /* defined _MSC_VER && _MSC_VER > 1000 */

#include "MyDialog1.h"
#include "MyDialog2.h"
#include "MyDialog3.h"
#include "MyDialog4.h"

class MyTabCtrl:public CTabCtrl {
public:
  MyTabCtrl();

  class MyDialog1 m_Dialog1;
  class MyDialog2 m_Dialog2;
  class MyDialog3 m_Dialog3;
  class MyDialog4 m_Dialog4;

  void InitTabDialogs(CStringArray *, CStringArray *, CStringArray *);
  void DisplayTabDialog(void);

  //{{AFX_VIRTUAL(MyTabCtrl)
  //}}AFX_VIRTUAL

    virtual ~ MyTabCtrl();

protected:
    CDialog * Dialogs[4];

  int Sel;

  //{{AFX_MSG(MyTabCtrl)
  afx_msg void OnSelchange(NMHDR * pNMHDR, LRESULT * pResult);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()};

//{{AFX_INSERT_LOCATION}}

#endif /* !defined(AFX_MYTABCTRL_H__D443FF52_C52D_4C89_AB4B_19B09687EBAE__INCLUDED_) */

#endif /* _WIN32 */

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
