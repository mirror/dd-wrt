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
// runtestsdlg.cpp :
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   julians
// Contact(s):  julians
// Date:        2000/09/29
// Version:     $Id: //depot/sw/releases/linuxsrc/src/redboot_cobra/ecos/host/tools/configtool/standalone/wxwin/runtestsdlg.h#3 $
// Purpose:
// Description: Header file for ecRunTestsDialog
// Requires:
// Provides:
// See also:
// Known bugs:
// Usage:
//
//####DESCRIPTIONEND####
//
//===========================================================================

#ifndef _ECOS_RUNTESTSDLG_H_
#define _ECOS_RUNTESTSDLG_H_

#ifdef __GNUG__
    #pragma interface "runtestsdlg.cpp"
#endif

#include "wx/notebook.h"
#include "wx/thread.h"

#include "eCosTest.h"
#include "Properties.h"

/*
 * ecRunTestsTimer
 * Just to force idle processing now and again while
 * the tests are running
 */

class ecRunTestsTimer: public wxTimer
{
public:
    ecRunTestsTimer() {}

    virtual void Notify() ;
};

//----------------------------------------------------------------------------
// ecRunTestsDialog
//----------------------------------------------------------------------------

class ecRunTestsExecutablesDialog;
class ecRunTestsOutputDialog;
class ecRunTestsSummaryDialog;

enum ecRunStatus { ecRunning, ecStopping, ecStopped };

#ifdef __WXGTK__
#define DWORD int
#endif

class ecRunTestsDialog: public wxDialog
{
DECLARE_CLASS(ecRunTestsDialog)
    friend class ecResetThread;
    friend class ecRunTestsTimer; 
public:
    ecRunTestsDialog(wxWindow* parent);
    ~ecRunTestsDialog();

    void OnOK(wxCommandEvent& event);
    void OnRun(wxCommandEvent& event);
    void OnProperties(wxCommandEvent& event);
    //void OnHelp(wxCommandEvent& event);
    void OnPageChange(wxNotebookEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnIdle(wxIdleEvent& event);
    void OnCloseWindow(wxCloseEvent& event);

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

    inline wxNotebook* GetNotebook() const { return m_notebook; }

    // Add the test to the dialog
    void Populate(const wxString& test, bool select = TRUE);
	void SubmitTests();

    // Write to the output buffer for OnIdle to pick up
    void OutputToBuffer(const wxString& str);
    // Write any remaining text
    void FlushBuffer();

    // Thread callbacks
    static void RunCallback(void *pParam);
    static void CALLBACK RunLocalFunc(void *pParam);
    static void CALLBACK RunRemoteFunc(void *pParam);
	static void CALLBACK TestOutputCallback(void *pParam,LPCTSTR psz);

    // Helpers
    // Translate from Windows to Unix serial port nomenclature
    wxString TranslatePort(const wxString& port) const;
	
protected:

    ecRunTestsExecutablesDialog*            m_executables;
    ecRunTestsOutputDialog*                 m_output;
    ecRunTestsSummaryDialog*                m_summary;
    wxNotebook*                             m_notebook;

    ecRunStatus                             m_runStatus;
    CeCosTest::ExecutionParameters          m_ep;
	int                                     m_nNextToSubmit;
    CTestResource*                          m_pResource;
    CProperties                             m_prop;
    wxCriticalSection                       m_CS;
    static ecRunTestsDialog*                m_runTestsDialog;
    bool                                    m_testsAreComplete;

    // Output text by writing to the buffer and letting
    // OnIdle pick it up
    wxString                                m_outputBuffer;
    bool                                    m_outputBufferPresent;
    ecRunTestsTimer                         m_timer;
DECLARE_EVENT_TABLE()
};

/* Executables dialog
 */

class ecRunTestsExecutablesDialog: public wxPanel
{
DECLARE_CLASS(ecRunTestsExecutablesDialog)
DECLARE_EVENT_TABLE()
public:
    ecRunTestsExecutablesDialog(wxWindow* parent);

    void CreateControls( wxPanel *parent);
    void AddFromFolder(const wxString& folder, bool recurse, const wxString& wildcard);
    int SelectedTestCount();
	wxString SelectedTest(int nIndex);

    void OnCheckAll(wxCommandEvent& event);
    void OnUncheckAll(wxCommandEvent& event);
    void OnAdd(wxCommandEvent& event);
    void OnAddFromFolder(wxCommandEvent& event);
    void OnRemove(wxCommandEvent& event);
    void OnUpdateCheckAll(wxUpdateUIEvent& event);
    void OnUpdateUncheckAll(wxUpdateUIEvent& event);
};

#define ecID_RUN_TESTS_CHECK_ALL 10072
#define ecID_RUN_TESTS_UNCHECK_ALL 10073
#define ecID_RUN_TESTS_ADD 10074
#define ecID_RUN_TESTS_ADD_FOLDER 10075
#define ecID_RUN_TESTS_REMOVE 10076
#define ecID_RUN_TESTS_TEST_LIST 2063

/* Output dialog
 */

class ecRunTestsOutputDialog: public wxPanel
{
DECLARE_CLASS(ecRunTestsOutputDialog)
public:
    ecRunTestsOutputDialog(wxWindow* parent);

    void CreateControls( wxPanel *parent);

    void AddText(const wxString& msg);
    void AddLogMsg(const wxString& msg);
};

#define ecID_RUN_TESTS_OUTPUT_TEXT 10088

/* Summary dialog
 */

class WXDLLEXPORT wxListCtrl;

// Windows requires the sort function to be of type
// CALLBACK
#ifndef CALLBACK
#define CALLBACK
#endif

class ecRunTestsSummaryDialog: public wxPanel
{
DECLARE_CLASS(ecRunTestsSummaryDialog)
DECLARE_EVENT_TABLE()
public:
    ecRunTestsSummaryDialog(wxWindow* parent);

    void CreateControls( wxPanel *parent);
	void AddResult (CeCosTest *pTest);
    void OnColClick(wxListEvent& event);
    static int CALLBACK SummarySortFunc(long data1, long data2, long col);

    static wxListCtrl* m_listCtrl;
};

#define ecID_RUN_TESTS_SUMMARY_LIST     2063

// Information used in thread processing
struct ecRunTestsInfo {
  ecRunTestsDialog* pSheet;
  CeCosTest      *  pTest;
};

// Thread class used for resetting a remote target
class ecResetThread : public wxThread
{
public:
    ecResetThread(ecRunTestsInfo* info) { m_info = info; };

    // thread execution starts here
    virtual void *Entry();

    // called when the thread exits - whether it terminates normally or is
    // stopped with Delete() (but not when it is Kill()ed!)
    virtual void OnExit();

public:
    ecRunTestsInfo* m_info;
};


#endif
    // _ECOS_RUNTESTSDLG_H_
