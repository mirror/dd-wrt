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
// Version:     $Id: runtestsdlg.cpp,v 1.14 2001/06/29 13:48:22 julians Exp $
// Purpose:
// Description: Implementation file for ecRunTestsDialog
// Requires:
// Provides:
// See also:
// Known bugs:
// Usage:
//
//####DESCRIPTIONEND####
//
//===========================================================================

#ifdef __GNUG__
    #pragma implementation "runtestsdlg.cpp"
#endif

#include "ecpch.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/notebook.h"
#include "wx/cshelp.h"
#include "wx/checklst.h"
#include "wx/listctrl.h"
#include "wx/tokenzr.h"

#include "TestResource.h"
#include "runtestsdlg.h"
#include "configtool.h"
#include "settingsdlg.h"
#include "eCosThreadUtils.h"
#include "eCosTrace.h"

void ecRunTestsTimer::Notify()
{
    static bool s_inNotify = FALSE;

    if (s_inNotify)
        return;

    s_inNotify = TRUE;

    // On Windows, simply having the timer going will ping the message queue
    // and cause idle processing to happen.
    // On Unix, this doesn't happen so we have to do the processing explicitly.
#ifdef __WXMSW__
    // Nothing to do
#else
    if ( ecRunTestsDialog::m_runTestsDialog )
    {
	wxIdleEvent event;
	ecRunTestsDialog::m_runTestsDialog->OnIdle(event);
    }
#endif

    s_inNotify = FALSE;
}

/*
 * Run Tests dialog
 */

IMPLEMENT_CLASS(ecRunTestsDialog, wxDialog)

BEGIN_EVENT_TABLE(ecRunTestsDialog, wxDialog)
    EVT_BUTTON(wxID_OK, ecRunTestsDialog::OnOK)
    EVT_BUTTON(ecID_RUN_TESTS_RUN, ecRunTestsDialog::OnRun)
    EVT_BUTTON(ecID_RUN_TESTS_PROPERTIES, ecRunTestsDialog::OnProperties)
    //EVT_BUTTON(wxID_HELP, ecRunTestsDialog::OnHelp)
    EVT_NOTEBOOK_PAGE_CHANGED(-1, ecRunTestsDialog::OnPageChange)
    EVT_SIZE(ecRunTestsDialog::OnSize)
    EVT_IDLE(ecRunTestsDialog::OnIdle)
    EVT_CLOSE(ecRunTestsDialog::OnCloseWindow)
END_EVENT_TABLE()

#define PROPERTY_DIALOG_WIDTH   600
#define PROPERTY_DIALOG_HEIGHT  550

ecRunTestsDialog* ecRunTestsDialog::m_runTestsDialog = NULL;

// For 400x400 settings dialog, size your panels to about 375x325 in dialog editor
// (209 x 162 dialog units)

ecRunTestsDialog::ecRunTestsDialog(wxWindow* parent):
    wxDialog(),
    m_runStatus(ecStopped),
    m_nNextToSubmit(-1),
    m_pResource(NULL),
    m_testsAreComplete(FALSE)
{
    m_runTestsDialog = this;

    SetExtraStyle(wxDIALOG_EX_CONTEXTHELP);

    wxDialog::Create(parent, ecID_RUN_TESTS_DIALOG, _("Run Tests"), wxPoint(0, 0), wxSize(PROPERTY_DIALOG_WIDTH, PROPERTY_DIALOG_HEIGHT),
        wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER);

    // Under MSW, we don't seem to be able to react to a click on the dialog background (no
    // event is generated).
    SetHelpText(_("Run tests from this dialog."));

    wxScreenDC dc;
    wxSize ppi = dc.GetPPI();

    //double scaleFactor = ((double) charH) / 13.0;
    double scaleFactor = ((double) ppi.y) / 96.0;
    // Fudge the scale factor to make the dialog slightly smaller,
    // otherwise it's a bit big. (We're assuming that most displays
    // are 96 or 120 ppi).
    if (ppi.y == 120)
        scaleFactor = 1.14;
    int dialogWidth = (int)(PROPERTY_DIALOG_WIDTH * scaleFactor);
    int dialogHeight = (int)(PROPERTY_DIALOG_HEIGHT * scaleFactor);
    SetSize(dialogWidth, dialogHeight);
        
    m_notebook = new wxNotebook(this, ecID_RUN_TESTS_NOTEBOOK,
         wxPoint(2, 2), wxSize(PROPERTY_DIALOG_WIDTH - 4, PROPERTY_DIALOG_HEIGHT - 4));

    m_executables = new ecRunTestsExecutablesDialog(m_notebook);
    m_notebook->AddPage(m_executables, wxT("Executables"));
    m_executables->TransferDataToWindow();

    m_output = new ecRunTestsOutputDialog(m_notebook);
    m_notebook->AddPage(m_output, wxT("Output"));
    m_output->TransferDataToWindow();

    m_summary = new ecRunTestsSummaryDialog(m_notebook);
    m_notebook->AddPage(m_summary, wxT("Summary"));
    m_summary->TransferDataToWindow();

    wxSizer *item0 = new wxBoxSizer( wxVERTICAL );

    item0->Add( m_notebook, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );

    wxSizer *item1 = new wxBoxSizer( wxHORIZONTAL );

    wxButton *runButton = new wxButton( this, ecID_RUN_TESTS_RUN, "&Run", wxDefaultPosition, wxDefaultSize, 0 );
    item1->Add( runButton, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxButton *okButton = new wxButton( this, wxID_OK, "&Close", wxDefaultPosition, wxDefaultSize, 0 );
    item1->Add( okButton, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxButton *propertiesButton = new wxButton( this, ecID_RUN_TESTS_PROPERTIES, "&Properties...", wxDefaultPosition, wxDefaultSize, 0 );
    item1->Add( propertiesButton, 0, wxALIGN_CENTRE|wxALL, 5 );

/*
    wxButton *helpButton = new wxButton( this, wxID_HELP, "&Help", wxDefaultPosition, wxDefaultSize, 0 );
    item1->Add( helpButton, 0, wxALIGN_CENTRE|wxALL, 5 );
*/

#ifdef __WXGTK__
    // Context-sensitive help button (question mark)
    wxButton *contextButton = new wxContextHelpButton( this );
    item1->Add( contextButton, 0, wxALIGN_CENTRE|wxALL, 5 );
#endif

    // Necessary to add a spacer or the button highlight interferes with the notebook under wxGTK
    item0->Add( 4, 4, 0, wxALIGN_CENTRE|wxALL, 0 );

    item0->Add( item1, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 0 );

    this->SetAutoLayout( TRUE );
    this->SetSizer( item0 );

    okButton->SetDefault();
    okButton->SetFocus();

    Layout();

    m_executables->Layout();
    m_output->Layout();
    m_summary->Layout();

    okButton->SetHelpText(_("Closes the dialog."));
    runButton->SetHelpText(_("Runs one or more tests selected in the Executables window."));
    propertiesButton->SetHelpText(_("Shows timeout and connection properties."));
    //helpButton->SetHelpText(_("Invokes help for the selected dialog."));

    Centre(wxBOTH);

    // TODO: Is this necessary?
#if 0
    m_prop.Add(_T("Active timeout"),        wxGetApp().GetSettings().GetRunTestsSettings().m_nTimeout);
    m_prop.Add(_T("Download timeout"),      wxGetApp().GetSettings().GetRunTestsSettings().m_nDownloadTimeout);
    m_prop.Add(_T("Active timeout type"),   wxGetApp().GetSettings().GetRunTestsSettings().m_nTimeoutType);
    m_prop.Add(_T("Download timeout type"), wxGetApp().GetSettings().GetRunTestsSettings().m_nDownloadTimeoutType);
    m_prop.Add(_T("Remote"),                wxGetApp().GetSettings().GetRunTestsSettings().m_bRemote);
    m_prop.Add(_T("Serial"),                wxGetApp().GetSettings().GetRunTestsSettings().m_bSerial);
    m_prop.Add(_T("Port"),                  wxGetApp().GetSettings().GetRunTestsSettings().m_strPort);
    m_prop.Add(_T("Baud"),                  wxGetApp().GetSettings().GetRunTestsSettings().m_nBaud);
    m_prop.Add(_T("Local TCPIP Host"),      wxGetApp().GetSettings().GetRunTestsSettings().m_strLocalTCPIPHost);
    m_prop.Add(_T("Local TCPIP Port"),      wxGetApp().GetSettings().GetRunTestsSettings().m_nLocalTCPIPPort);
    m_prop.Add(_T("Reset Type"),            wxGetApp().GetSettings().GetRunTestsSettings().m_nReset);
    m_prop.Add(_T("Reset String"),          wxGetApp().GetSettings().GetRunTestsSettings().m_strReset);
    m_prop.Add(_T("Resource Host"),         wxGetApp().GetSettings().GetRunTestsSettings().m_strResourceHost);
    m_prop.Add(_T("Resource Port"),         wxGetApp().GetSettings().GetRunTestsSettings().m_nResourcePort);
    m_prop.Add(_T("Remote Host"),           wxGetApp().GetSettings().GetRunTestsSettings().m_strRemoteHost);
    m_prop.Add(_T("Remote Port"),           wxGetApp().GetSettings().GetRunTestsSettings().m_nRemotePort);
    // TODO
    //m_prop.Add(_T("Recurse"),   executionpage.m_bRecurse);
    m_prop.Add(_T("Farmed"),                wxGetApp().GetSettings().GetRunTestsSettings().m_bFarmed);
    // TODO
    // m_prop.Add(_T("Extension"),executionpage.m_strExtension);
#endif

#ifdef _DEBUG
    CeCosTrace::EnableTracing(CeCosTrace::TRACE_LEVEL_TRACE);
#endif
    CeCosTrace::SetInteractive(TRUE);
    CeCosTrace::SetOutput(TestOutputCallback, this);
    CeCosTrace::SetError (TestOutputCallback, this);

    m_timer.Start(200);
}

ecRunTestsDialog::~ecRunTestsDialog()
{
    m_timer.Stop();
    CeCosTrace::SetInteractive(FALSE);
    m_runTestsDialog = NULL;
    if (m_pResource)
    {
        delete m_pResource;
    }
}

void ecRunTestsDialog::OnOK(wxCommandEvent& event)
{
    if (ecRunning == m_runStatus)
    {
        wxMessageBox(_("Tests are running. Please press Stop before quitting this dialog."),
            wxGetApp().GetSettings().GetAppName(), wxICON_INFORMATION|wxOK, this);
        return;
    }

    wxDialog::OnOK(event);
}

void ecRunTestsDialog::OnCloseWindow(wxCloseEvent& event)
{
    if (ecRunning == m_runStatus)
    {
        wxMessageBox(_("Tests are running. Please press Stop before quitting this dialog."),
            wxGetApp().GetSettings().GetAppName(), wxICON_INFORMATION|wxOK, this);
        event.Veto();
        return;
    }

    this->EndModal(wxID_CANCEL);
    this->Destroy();
}

void ecRunTestsDialog::OnProperties(wxCommandEvent& event)
{
    ecSettingsDialog dialog(this);
    dialog.SetSelection(3);
    dialog.ShowModal();
}

void ecRunTestsDialog::OnRun(wxCommandEvent& event)
{  
    if (ecRunning == m_runStatus)
    {
        m_output->AddLogMsg(_("Run cancelled"));
        m_runStatus = ecStopping;
        m_CS.Enter();
        m_nNextToSubmit=0x7fffffff;
        m_CS.Leave();
        CeCosTest::CancelAllInstances();  
    }
    else
    {
        if ( 0 == m_executables->SelectedTestCount())
        {
            wxMessageBox(_("No tests have been selected for execution."),
                wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK, this);
            return ;
        }
        else
        {
            m_ep = CeCosTest::ExecutionParameters(
                CeCosTest::ExecutionParameters::RUN,
                wxGetApp().GetSettings().GetRunTestsSettings().m_strTarget,
                TIMEOUT_NONE==wxGetApp().GetSettings().GetRunTestsSettings().m_nTimeoutType?0x7fffffff:TIMEOUT_AUTOMATIC==wxGetApp().GetSettings().GetRunTestsSettings().m_nTimeoutType?900000:1000*wxGetApp().GetSettings().GetRunTestsSettings().m_nTimeout,
                TIMEOUT_NONE==wxGetApp().GetSettings().GetRunTestsSettings().m_nDownloadTimeoutType?0x7fffffff:TIMEOUT_AUTOMATIC==wxGetApp().GetSettings().GetRunTestsSettings().m_nDownloadTimeoutType?0:1000*wxGetApp().GetSettings().GetRunTestsSettings().m_nDownloadTimeout);
            
            if ( wxGetApp().GetSettings().GetRunTestsSettings().m_bRemote )
            {
                CTestResource::SetResourceServer(CeCosSocket::HostPort(wxGetApp().GetSettings().GetRunTestsSettings().m_strResourceHost, wxGetApp().GetSettings().GetRunTestsSettings().m_nResourcePort));
                if (!CTestResource::Load())
                {
                    wxMessageBox(_("Could not connect to resource server."),
                        wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK, this);
                    return;
                }
            }
            else
            {
                wxString strPort;
                if (wxGetApp().GetSettings().GetRunTestsSettings().m_bSerial)
                    strPort = wxGetApp().GetSettings().GetRunTestsSettings().m_strPort;
                else
                    strPort = (const wxChar*) CeCosSocket::HostPort(wxGetApp().GetSettings().GetRunTestsSettings().m_strLocalTCPIPHost,wxGetApp().GetSettings().GetRunTestsSettings().m_nLocalTCPIPPort);
                if(0==strPort.Length()){
                    m_pResource=new CTestResource(_T(""),m_ep.PlatformName());
                } else
		{
                    // Translate from e.g. COM2 to /dev/ttyS1 on Unix.
                    // Let's assume the Windows notation is the 'standard'.
		    strPort = TranslatePort(strPort);
                    int nBaud = wxGetApp().GetSettings().GetRunTestsSettings().m_bSerial ? wxGetApp().GetSettings().GetRunTestsSettings().m_nBaud:0;
                    if (RESET_X10 != wxGetApp().GetSettings().GetRunTestsSettings().m_nReset) {
                        m_pResource=new CTestResource(_T(""),m_ep.PlatformName(), strPort, nBaud);
                    } else {
                        m_pResource=new CTestResource(_T(""),m_ep.PlatformName(), strPort, nBaud, wxGetApp().GetSettings().GetRunTestsSettings().m_strReset);
                    }
                }
            }
            m_runStatus = ecRunning;
            m_testsAreComplete = FALSE;
            
            wxButton* runButton = (wxButton*) FindWindow(ecID_RUN_TESTS_RUN);
            runButton->SetLabel(_("&Stop"));
            
            m_nNextToSubmit=0;
            m_output->AddLogMsg(_("Run started"));

            SubmitTests();
        }
    }
}

wxString ecRunTestsDialog::TranslatePort(const wxString& port) const
{
#ifdef __WXGTK__
    wxString name(port.Left(3));
    if (name.CmpNoCase(wxT("COM")) == 0)
    {
	wxString strNum(port.Mid(3));
	if (strNum.IsEmpty())
	    return port;
	int num = atoi(strNum);
	wxString newPort;
	newPort.Printf(wxT("/dev/ttyS%d"), num-1);
	return newPort;
    }
    else
#endif
        return port;
}

void ecRunTestsDialog::SubmitTests()
{
    int iTest;
    int nResources=wxGetApp().GetSettings().GetRunTestsSettings().m_bRemote ? wxMax(1,CTestResource::GetMatchCount (m_ep)):1;
    if(nResources>CeCosTest::InstanceCount){
        if (m_nNextToSubmit >= m_executables->SelectedTestCount()){
            m_runStatus = ecStopped;

            wxButton* runButton = (wxButton*) FindWindow(ecID_RUN_TESTS_RUN);
            runButton->SetLabel(_("&Run"));
            m_output->AddLogMsg(_("Run complete"));

            delete m_pResource;
            m_pResource=0;
            return;
        }
        ecRunTestsInfo *pInfo=new ecRunTestsInfo;
        pInfo->pTest=new CeCosTest(m_ep, m_executables->SelectedTest(m_nNextToSubmit++));
        pInfo->pSheet=this;
        if(wxGetApp().GetSettings().GetRunTestsSettings().m_bRemote){
            CeCosThreadUtils::RunThread(RunRemoteFunc,pInfo, (CeCosThreadUtils::CallbackProc*) RunCallback,_T("RunRemoteFunc"));
        } else {
            bool bRun=false;
            switch((ResetType)wxGetApp().GetSettings().GetRunTestsSettings().m_nReset){
            case RESET_NONE:
                bRun=true;
                break;
            case RESET_X10:
                {
                    // Resetting can take a while, so spawn a thread
                    bRun=false;
                    ecResetThread* thread = new ecResetThread(pInfo);
                    if (thread->Create() != wxTHREAD_NO_ERROR)
                    {
                        // Thread will be deleted automatically when it has finished running
                        thread->Run();
                    }
                    else
                        delete thread;

                    break;
                }
            case RESET_MANUAL:
                {
                    bRun=(wxOK == wxMessageBox(_("Press OK when target is reset - cancel to abort run"),_("Reset board"), wxOK|wxCANCEL));
                    if(!bRun)
                    {
                        m_nNextToSubmit = m_executables->SelectedTestCount();
                        RunCallback(pInfo);
                    }
                    break;
                }
            }
            if(bRun){
                if (1 < m_nNextToSubmit)
                      m_output->AddLogMsg(_("Run continuing"));
                CeCosThreadUtils::RunThread(RunLocalFunc, pInfo, (CeCosThreadUtils::CallbackProc*) RunCallback,_T("RunLocalFunc"));
            }
        }
    }
}

// thread execution starts here
void *ecResetThread::Entry()
{
    wxString str;
    String str1;
    bool bOk=false;
    CResetAttributes::ResetResult n=m_info->pSheet->m_pResource->Reset(str1);
    str = str1;
    if(CResetAttributes::RESET_OK!=n){
        str += wxT(">>> Could not reset target\n");
    }
    str += wxT('\n');

    m_info->pSheet->OutputToBuffer(str);

    if(bOk){
        // we're already in a thread, so we can call the function directly
        m_info->pTest->RunLocal();
    }
    ecRunTestsDialog::RunCallback(m_info);

    return NULL;
}

// called when the thread exits - whether it terminates normally or is
// stopped with Delete() (but not when it is Kill()ed!)
void ecResetThread::OnExit()
{
}

void CALLBACK ecRunTestsDialog::RunLocalFunc(void *pParam)
{
  ((ecRunTestsInfo *)pParam)->pTest->RunLocal();
}

void CALLBACK ecRunTestsDialog::RunRemoteFunc(void *pParam)
{
  ((ecRunTestsInfo *)pParam)->pTest->RunRemote(NULL);
}

void ecRunTestsDialog::RunCallback(void *pParam)
{
    ecRunTestsInfo *pInfo=(ecRunTestsInfo *)pParam;
    ecRunTestsDialog *pSheet=pInfo->pSheet;
    if (m_runTestsDialog) // Will be NULL if dialog has been closed & deleted
    {
        CeCosTest *pTest=pInfo->pTest;

        pInfo->pSheet->m_CS.Enter();

        pSheet->m_summary->AddResult(pTest);
        delete pTest;

        // OnIdle will check this variable and reset the status and button label
        pSheet->m_testsAreComplete = TRUE;

        pInfo->pSheet->m_CS.Leave();
    }
    delete pInfo;
}

void ecRunTestsDialog::OnIdle(wxIdleEvent& event)
{
    FlushBuffer();

    if (m_testsAreComplete)
    {
        m_testsAreComplete = FALSE;
        SubmitTests();
    }

    event.Skip();
}

void CALLBACK ecRunTestsDialog::TestOutputCallback(void *pParam,LPCTSTR psz)
{
    ecRunTestsDialog* pWnd = (ecRunTestsDialog*)pParam;
    if (ecRunTestsDialog::m_runTestsDialog)
    {
        // FIXME: test output should not contain CR characters on non-Windows
        // platforms so need to find the root of this problem
#ifndef __WXMSW__
        wxString output(psz);
        output.Replace(wxT("\r"), wxEmptyString); // remove CR characters
        pWnd->OutputToBuffer(output);
#else
        // FIXME ends
        pWnd->OutputToBuffer(psz);
#endif
    }
}

// Write to the output buffer for OnIdle to pick up
void ecRunTestsDialog::OutputToBuffer(const wxString& str)
{
    wxCriticalSection ct;
    ct.Enter();

    if (m_outputBufferPresent)
        m_outputBuffer += str;
    else
        m_outputBuffer = str;
    m_outputBufferPresent = TRUE;

    ct.Leave();
}

// Write any remaining text
void ecRunTestsDialog::FlushBuffer()
{
    if (m_outputBufferPresent)
    {
        m_output->AddText(m_outputBuffer);
        m_outputBuffer = wxEmptyString;
        m_outputBufferPresent = FALSE;
    }
}

#if 0
void ecRunTestsDialog::OnHelp(wxCommandEvent& event)
{
    int sel = m_notebook->GetSelection();

    wxASSERT_MSG( (sel != -1), wxT("A notebook tab should always be selected."));

    wxWindow* page = (wxWindow*) m_notebook->GetPage(sel);

    wxString helpTopic;
    if (page == m_displayOptions)
    {
        helpTopic = wxT("Display options dialog");
    }

    if (!helpTopic.IsEmpty())
    {
        wxGetApp().GetHelpController().KeywordSearch(helpTopic);
    }
}
#endif

// This sets the text for the selected page, but doesn't help
// when trying to click on a tab: we would expect the appropriate help
// for that tab. We would need to look at the tabs to do this, from within OnContextHelp -
// probably not worth it.
void ecRunTestsDialog::OnPageChange(wxNotebookEvent& event)
{
    event.Skip();
#if 0
    int sel = m_notebook->GetSelection();
    if (sel < 0)
        return;

    wxWindow* page = m_notebook->GetPage(sel);
    if (page)
    {
        wxString helpText;

        if (page == m_displayOptions)
            helpText = _("The display options dialog allows you to change display-related options.");
        else if (page == m_viewerOptions)
            helpText = _("The viewer options dialog allows you to configure viewers.");
        else if (page == m_pathOptions)
            helpText = _("The path options dialog allows you to change tool paths.");
        else if (page == m_conflictResolutionOptions)
            helpText = _("The conflict resolution options dialog allows you to change options related to conflict resolution.");
        m_notebook->SetHelpText(helpText);
    }
#endif
}

bool ecRunTestsDialog::TransferDataToWindow()
{
    // In this case there is no data to be transferred
    m_executables->TransferDataToWindow();
    m_output->TransferDataToWindow();
    m_summary->TransferDataToWindow();
    return TRUE;
}

bool ecRunTestsDialog::TransferDataFromWindow()
{
    // In this case there is no data to be transferred
    m_executables->TransferDataFromWindow();
    m_output->TransferDataFromWindow();
    m_summary->TransferDataFromWindow();
    return TRUE;
}

void ecRunTestsDialog::OnSize(wxSizeEvent& event)
{
    event.Skip();

    wxRefreshControls(this);
}

// Add the test to the dialog
void ecRunTestsDialog::Populate(const wxString& test, bool select)
{
    wxCheckListBox* checkListBox = (wxCheckListBox*) FindWindow(ecID_RUN_TESTS_TEST_LIST);

    wxASSERT( checkListBox );

    checkListBox->Append(test);
    if (select)
        checkListBox->Check(checkListBox->GetCount() - 1, TRUE);
}


/* Executables dialog
 */

IMPLEMENT_CLASS(ecRunTestsExecutablesDialog, wxPanel)

BEGIN_EVENT_TABLE(ecRunTestsExecutablesDialog, wxPanel)
    EVT_BUTTON(ecID_RUN_TESTS_CHECK_ALL, ecRunTestsExecutablesDialog::OnCheckAll)
    EVT_BUTTON(ecID_RUN_TESTS_UNCHECK_ALL, ecRunTestsExecutablesDialog::OnUncheckAll)
    EVT_BUTTON(ecID_RUN_TESTS_ADD, ecRunTestsExecutablesDialog::OnAdd)
    EVT_BUTTON(ecID_RUN_TESTS_ADD_FOLDER, ecRunTestsExecutablesDialog::OnAddFromFolder)
    EVT_BUTTON(ecID_RUN_TESTS_REMOVE, ecRunTestsExecutablesDialog::OnRemove)

    EVT_UPDATE_UI(ecID_RUN_TESTS_CHECK_ALL, ecRunTestsExecutablesDialog::OnUpdateCheckAll)
    EVT_UPDATE_UI(ecID_RUN_TESTS_UNCHECK_ALL, ecRunTestsExecutablesDialog::OnUpdateUncheckAll)

END_EVENT_TABLE()

ecRunTestsExecutablesDialog::ecRunTestsExecutablesDialog(wxWindow* parent):
    wxPanel(parent, ecID_RUN_TESTS_EXECUTABLES)
{
    CreateControls(this);    

    SetHelpText(_("The executables dialog allows you to select tests to be run."));
}

void ecRunTestsExecutablesDialog::CreateControls( wxPanel *parent)
{
    // Create the foreign control
    wxCheckListBox* listBox = new wxCheckListBox(parent, ecID_RUN_TESTS_TEST_LIST, wxDefaultPosition, wxSize(100, 100),
        0, NULL, wxSUNKEN_BORDER|wxLB_EXTENDED);

    wxSizer *item0 = new wxBoxSizer( wxVERTICAL );

    wxSizer *item1 = new wxBoxSizer( wxHORIZONTAL );

    wxButton *item2 = new wxButton( parent, ecID_RUN_TESTS_CHECK_ALL, "C&heck All", wxDefaultPosition, wxDefaultSize, 0 );
    item1->Add( item2, 0, wxALIGN_CENTRE|wxALL, 5 );

    item1->Add( 2, 2, 1, wxALIGN_CENTRE|wxALL, 0 );

    wxButton *item3 = new wxButton( parent, ecID_RUN_TESTS_UNCHECK_ALL, "&Uncheck All", wxDefaultPosition, wxDefaultSize, 0 );
    item1->Add( item3, 0, wxALIGN_CENTRE|wxALL, 5 );

    item1->Add( 2, 2, 1, wxALIGN_CENTRE|wxALL, 0 );

    wxButton *item4 = new wxButton( parent, ecID_RUN_TESTS_ADD, "&Add...", wxDefaultPosition, wxDefaultSize, 0 );
    item1->Add( item4, 0, wxALIGN_CENTRE|wxALL, 5 );

    item1->Add( 2, 2, 1, wxALIGN_CENTRE|wxALL, 0 );

    wxButton *item5 = new wxButton( parent, ecID_RUN_TESTS_ADD_FOLDER, "Add from &Folder...", wxDefaultPosition, wxDefaultSize, 0 );
    item1->Add( item5, 0, wxALIGN_CENTRE|wxALL, 5 );

    item1->Add( 2, 2, 1, wxALIGN_CENTRE|wxALL, 0 );

    wxButton *item6 = new wxButton( parent, ecID_RUN_TESTS_REMOVE, "&Remove", wxDefaultPosition, wxDefaultSize, 0 );
    item1->Add( item6, 0, wxALIGN_CENTRE|wxALL, 5 );

    item0->Add( item1, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    wxWindow *item7 = parent->FindWindow( ecID_RUN_TESTS_TEST_LIST );
    wxASSERT( item7 );
    item0->Add( item7, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    // Add context-sensitive help
    FindWindow(ecID_RUN_TESTS_TEST_LIST)->SetHelpText(_("Displays the set of tests that can be executed. Each test can be selected for execution by checking the adjacent box."));
    FindWindow(ecID_RUN_TESTS_CHECK_ALL)->SetHelpText(_("Selects all tests for execution."));
    FindWindow(ecID_RUN_TESTS_UNCHECK_ALL)->SetHelpText(_("Clears the selection of tests."));
    FindWindow(ecID_RUN_TESTS_ADD)->SetHelpText(_("Adds a test to the set that can be executed."));
    FindWindow(ecID_RUN_TESTS_ADD_FOLDER)->SetHelpText(_("Adds one or more tests to the set that can be executed, from a folder."));
    FindWindow(ecID_RUN_TESTS_REMOVE)->SetHelpText(_("Removes a test from the set that can executed."));

    parent->SetAutoLayout( TRUE );
    parent->SetSizer( item0 );
}

void ecRunTestsExecutablesDialog::OnCheckAll(wxCommandEvent& event)
{
    wxCheckListBox* checkList = (wxCheckListBox*) FindWindow(ecID_RUN_TESTS_TEST_LIST);
    if (!checkList)
        return;

    int i;
    int n = checkList->GetCount();
    for (i = 0; i < n; i++)
        checkList->Check(i, TRUE);
}

void ecRunTestsExecutablesDialog::OnUncheckAll(wxCommandEvent& event)
{
    wxCheckListBox* checkList = (wxCheckListBox*) FindWindow(ecID_RUN_TESTS_TEST_LIST);
    if (!checkList)
        return;

    int i;
    int n = checkList->GetCount();
    for (i = 0; i < n; i++)
        checkList->Check(i, FALSE);
}

void ecRunTestsExecutablesDialog::OnAdd(wxCommandEvent& event)
{
    wxCheckListBox* checkList = (wxCheckListBox*) FindWindow(ecID_RUN_TESTS_TEST_LIST);
    if (!checkList)
        return;

//#ifdef __WXMSW__
//    wxString wildcard(wxT("Executables (*.exe)|*.exe"));
//#else
    wxString wildcard(wxT("Executables (*)|*"));
//#endif

    wxFileDialog dialog(this, _("Choose one or more executables to add"), wxGetCwd(), wxEmptyString,
        wildcard, wxMULTIPLE|wxOPEN);

    if (dialog.ShowModal() == wxID_OK)
    {
        wxArrayString paths;
        dialog.GetPaths(paths);

        bool err = FALSE;

        unsigned int i;
        int n = paths.Count();
        for (i = 0; i < n; i++)
        {
            // TODO: check that it's the right kind of file
            if (-1 == checkList->FindString(paths[i]))
            {
                checkList->Append(paths[i]);
                checkList->Check(checkList->GetCount()-1, TRUE);
            }
            else
                err = TRUE;
        }
        if (err)
            wxMessageBox(_("One or more of the files was already present"),
                wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK, this);
    }
}

void ecRunTestsExecutablesDialog::OnAddFromFolder(wxCommandEvent& event)
{
    wxCheckListBox* checkList = (wxCheckListBox*) FindWindow(ecID_RUN_TESTS_TEST_LIST);
    if (!checkList)
        return;

    // In the MFC tool, a modified version of the folder dialog was used but
    // we can't do that in general in wxWindows; so instead we ask the questions
    // before we show the folder dialog.
    // We won't bother allowing the user to change the extension: on Windows it's .exe,
    // on Unix it's anything.
//#ifdef __WXMSW__
//    wxString filespec(wxT("*.exe"));
//#else
    wxString filespec(wxT("*"));
//#endif

    wxString msg;
    msg.Printf(_("Would you like to add from subfolders, or just the folder you specify?\nChoose Yes to add from subfolders."));
    int ans = wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_QUESTION|wxYES_NO|wxCANCEL, this);
    if (ans == wxCANCEL)
        return;

    bool recurse = (ans == wxYES);

    wxDirDialog dialog(this, _("Choose a folder to add tests from"), wxGetCwd());
    if (dialog.ShowModal() == wxID_OK)
    {
        wxString folder(dialog.GetPath());

        if (!wxDirExists(folder))
        {
            wxMessageBox(_("Sorry, this folder does not exist."), wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK, this);
            return;
        }

        wxBusyCursor busy;
        AddFromFolder(folder, recurse, filespec);
    }
}

void ecRunTestsExecutablesDialog::AddFromFolder(const wxString& folder, bool recurse, const wxString& wildcard)
{
    wxString filename;
    wxCheckListBox* checkList = (wxCheckListBox*) FindWindow(ecID_RUN_TESTS_TEST_LIST);

    {
        wxDir dir;
        if (!dir.Open(folder))
            return;
        
        
        bool success = dir.GetFirst(& filename, wildcard, wxDIR_FILES);
        while (success)
        {
            wxString path = folder + wxString(wxFILE_SEP_PATH) + filename;
            
            if (-1 == checkList->FindString(path))
            {
                checkList->Append(path);
                checkList->Check(checkList->GetCount()-1, TRUE);
            }       
            
            success = dir.GetNext(& filename);
        }
    }
    
    // Recurse down the subfolders
    if (recurse)
    {
        wxArrayString subfolders;

        {
            wxDir dir2;
            if (!dir2.Open(folder))
                return;
            
            bool success = dir2.GetFirst(& filename, wxT("*"), wxDIR_DIRS);
            while (success)
            {
                wxString path = folder + wxString(wxFILE_SEP_PATH) + filename;
                subfolders.Add(path);            
                
                success = dir2.GetNext(& filename);
            }
        }

        unsigned int i;
        for (i = 0; i < subfolders.Count(); i ++)
        {
            AddFromFolder(subfolders[i], recurse, wildcard);
        }
    }
}

void ecRunTestsExecutablesDialog::OnRemove(wxCommandEvent& event)
{
    wxCheckListBox* checkList = (wxCheckListBox*) FindWindow(ecID_RUN_TESTS_TEST_LIST);
    if (!checkList)
        return;

    bool cont = FALSE;
    do
    {
        // Delete the selections one at a time since
        // the indexes change when you delete one

        wxArrayInt ar;
        if (checkList->GetSelections(ar) > 0)
        {
            checkList->Delete(ar[0]);
            cont = TRUE;
        }
        else
            cont = FALSE;

    } while (cont);

}

void ecRunTestsExecutablesDialog::OnUpdateCheckAll(wxUpdateUIEvent& event)
{
    wxCheckListBox* checkList = (wxCheckListBox*) FindWindow(ecID_RUN_TESTS_TEST_LIST);
    if (!checkList)
        return;

    // If there were no unchecked items, we can disable the check all button
    event.Enable( checkList->GetCount() != SelectedTestCount() );
}

void ecRunTestsExecutablesDialog::OnUpdateUncheckAll(wxUpdateUIEvent& event)
{
    event.Enable( SelectedTestCount() > 0 );
}

int ecRunTestsExecutablesDialog::SelectedTestCount()
{
    wxCheckListBox* checkList = (wxCheckListBox*) FindWindow(ecID_RUN_TESTS_TEST_LIST);
    if (!checkList)
        return 0;

    int selCount = 0;
    int i;
    int n = checkList->GetCount();
    for (i = 0; i < n; i++)
    {
        if (checkList->IsChecked(i))
        {
            selCount ++;
        }
    }
    return selCount;
}

wxString ecRunTestsExecutablesDialog::SelectedTest(int nIndex)
{
    wxString str;
    wxCheckListBox* checkList = (wxCheckListBox*) FindWindow(ecID_RUN_TESTS_TEST_LIST);
    if (!checkList)
        return str;

    int i;
    for (i=0; i < checkList->GetCount(); i++)
    {
        if (checkList->IsChecked(i))
        {
            if(0==nIndex--)
            {
                str = checkList->GetString(i);
                break;
            }
        }
    }
    return str;
}

/* Output dialog
 */

IMPLEMENT_CLASS(ecRunTestsOutputDialog, wxPanel)

ecRunTestsOutputDialog::ecRunTestsOutputDialog(wxWindow* parent):
    wxPanel(parent, ecID_RUN_TESTS_OUTPUT)
{
    CreateControls(this);    

    SetHelpText(_("The output dialog displays the run output."));
}

void ecRunTestsOutputDialog::CreateControls( wxPanel *parent)
{
    wxSizer *item0 = new wxBoxSizer( wxVERTICAL );

    wxTextCtrl *item1 = new wxTextCtrl( parent, ecID_RUN_TESTS_OUTPUT_TEXT, "", wxDefaultPosition, wxSize(80,40), wxTE_MULTILINE|wxTE_READONLY|wxTE_RICH|wxCLIP_CHILDREN );
    item0->Add( item1, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    // Add context-sensitive help
    FindWindow(ecID_RUN_TESTS_OUTPUT_TEXT)->SetHelpText(_("Displays the output of test execution."));

    parent->SetAutoLayout( TRUE );
    parent->SetSizer( item0 );
}

void ecRunTestsOutputDialog::AddText(const wxString& msg)
{
    wxTextCtrl* textCtrl = (wxTextCtrl*) FindWindow(ecID_RUN_TESTS_OUTPUT_TEXT);
    wxASSERT (textCtrl != NULL);

    textCtrl->SetInsertionPointEnd();
    textCtrl->AppendText(msg);    
}

void ecRunTestsOutputDialog::AddLogMsg(const wxString& msg)
{
    wxString msg2(msg);

    if ((msg == wxEmptyString) || (msg.Last() != wxT('\n')))
        msg2 += wxT("\n");

    AddText(msg2);
}

/* Summary dialog
 */

IMPLEMENT_CLASS(ecRunTestsSummaryDialog, wxPanel)

BEGIN_EVENT_TABLE(ecRunTestsSummaryDialog, wxPanel)
    EVT_LIST_COL_CLICK(ecID_RUN_TESTS_SUMMARY_LIST, ecRunTestsSummaryDialog::OnColClick)
END_EVENT_TABLE()

wxListCtrl* ecRunTestsSummaryDialog::m_listCtrl = NULL;

ecRunTestsSummaryDialog::ecRunTestsSummaryDialog(wxWindow* parent):
    wxPanel(parent, ecID_RUN_TESTS_SUMMARY)
{
    CreateControls(this);    

    SetHelpText(_("The summary dialog shows a summary of the results of each run."));
}

void ecRunTestsSummaryDialog::CreateControls( wxPanel *parent)
{
    m_listCtrl = new wxListCtrl(parent, ecID_RUN_TESTS_SUMMARY_LIST, wxDefaultPosition, wxSize(100, 100), wxSUNKEN_BORDER|wxLC_REPORT);
    m_listCtrl->InsertColumn(0, "Time", wxLIST_FORMAT_LEFT, 60);
    m_listCtrl->InsertColumn(1, "Host", wxLIST_FORMAT_LEFT, 60);
    m_listCtrl->InsertColumn(2, "Platform", wxLIST_FORMAT_LEFT, 60);
    m_listCtrl->InsertColumn(3, "Executable", wxLIST_FORMAT_LEFT, 60);
    m_listCtrl->InsertColumn(4, "Status", wxLIST_FORMAT_LEFT, 60);
    m_listCtrl->InsertColumn(5, "Size", wxLIST_FORMAT_LEFT, 60);
    m_listCtrl->InsertColumn(6, "Download", wxLIST_FORMAT_LEFT, 60);
    m_listCtrl->InsertColumn(7, "Elapsed", wxLIST_FORMAT_LEFT, 60);
    m_listCtrl->InsertColumn(8, "Execution", wxLIST_FORMAT_LEFT, 60);

    wxSizer *item0 = new wxBoxSizer( wxVERTICAL );

    wxWindow *item1 = parent->FindWindow( ecID_RUN_TESTS_SUMMARY_LIST );
    wxASSERT( item1 );
    item0->Add( item1, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    parent->SetAutoLayout( TRUE );
    parent->SetSizer( item0 );

    // Add context-sensitive help
    FindWindow(ecID_RUN_TESTS_SUMMARY_LIST)->SetHelpText(_("Displays a summary of test execution."));
}

void ecRunTestsSummaryDialog::AddResult (CeCosTest *pTest)
{
    const wxString strResult(pTest->ResultString(FALSE));
    int nLength=strResult.Length();
    wxString arstr[8];
    wxString strTime,strDate;
    int i;

    // 1999-05-28 10:29:28 nan:0 TX39-jmr3904-sim tx39-jmr3904sim-libc10-signal2.exe Fail 0k/1108k D=0.0/0.0 Total=9.3 E=0.6/300.0 
    wxStringTokenizer tok(strResult, wxT(" "));
    strDate = tok.GetNextToken();
    strTime = tok.GetNextToken();

    strDate += wxT(" ");
    strDate += strTime;

    for (i = 0; i < 8; i++)
        arstr[i] = tok.GetNextToken();

    // Remove characters before '=' in time fields
    for ( i = 5 ; i < 8 ; i ++ )
    {
        wxString str = arstr[i].AfterFirst(wxT('-')) ;
        arstr[i] = str.IsEmpty() ? arstr[i] : str;
    }

    int nItem = m_listCtrl->GetItemCount() ;
    int nIndex = m_listCtrl->InsertItem (nItem, strDate);
    m_listCtrl->SetItemData(nItem,nItem);// to support sorting
    for (i = 0; i < 8; i++)
    {
        m_listCtrl->SetItem(nIndex, i+1, arstr[i]);
    }  

#if 0
    // OLD CODE
    int i;
    // TRACE(_T("%s\n"),strResult);
    // 1999-05-28 10:29:28 nan:0 TX39-jmr3904-sim tx39-jmr3904sim-libc10-signal2.exe Fail 0k/1108k D=0.0/0.0 Total=9.3 E=0.6/300.0 
    _stscanf(strResult,_T("%s %s %s %s %s %s %s %s %s %s"),
        strDate.GetBuffer(1+nLength),
        strTime.GetBuffer(1+nLength),
        arstr[0].GetBuffer(1+nLength),
        arstr[1].GetBuffer(1+nLength),
        arstr[2].GetBuffer(1+nLength),
        arstr[3].GetBuffer(1+nLength),
        arstr[4].GetBuffer(1+nLength),
        arstr[5].GetBuffer(1+nLength),
        arstr[6].GetBuffer(1+nLength),
        arstr[7].GetBuffer(1+nLength));

    // Remove before '=' in time fields
    for(i=5;i<8;i++){
        TCHAR *pch=_tcschr(arstr[i],_TCHAR('='));
        if(pch){
            arstr[i]=pch+1;
        }
    }
    
    strDate.ReleaseBuffer();
    strTime.ReleaseBuffer();
    strDate+=_TCHAR(' ');
    strDate+=strTime;
    int nItem=m_List.GetItemCount();
    m_List.InsertItem(nItem,strDate);
    m_List.SetItemData(nItem,nItem);// to support sorting
    for(i=0;i<8;i++){
        m_List.SetItemText(nItem,1+i,arstr[i]);
        arstr[i].ReleaseBuffer();
    }
#endif
}

// Sort function.
// The function is passed the client data of the two items,
// plus another general client data value which in this case
// we use for the column index.
int CALLBACK ecRunTestsSummaryDialog::SummarySortFunc(long data1, long data2, long col)
{
    wxString str1 = wxListCtrlGetItemTextColumn(* m_listCtrl, data1, col);
    wxString str2 = wxListCtrlGetItemTextColumn(* m_listCtrl, data2, col);

    int ret = str1.CmpNoCase(str2);
    return ret;
}

void ecRunTestsSummaryDialog::OnColClick(wxListEvent& event)
{
    m_listCtrl->SortItems((wxListCtrlCompare) SummarySortFunc,(long) event.m_col);

    // The item data contains the index in the list control, so this needs
    // to be reset after sorting.
    int i;
    for (i = m_listCtrl->GetItemCount()-1;i>=0;--i)
    {
        m_listCtrl->SetItemData(i,i);
    }
}
