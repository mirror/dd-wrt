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
//        SectionGeneralPage.cpp
//
//        Memory Layout Tool section general property page class
//
//=================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     John Dallaway
// Contact(s):    jld
// Date:          1998/07/29 $RcsDate$ {or whatever}
// Version:       0.00+  $RcsVersion$ {or whatever}
// Purpose:       Provides a derivation of the MFC CeCosPropertyPage class for
//                general section property selection
// See also:      SectionGeneralPage.h
// Known bugs:    <UPDATE_ME_AT_RELEASE_TIME>
//
//####DESCRIPTIONEND####

#include "stdafx.h"
#include "ConfigtoolDoc.h"
#include "SectionGeneralPage.h"
#include "ConfigTool.h"
#include "CTUtils.h"


#include "memmap.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSectionGeneralPage property page

IMPLEMENT_DYNCREATE(CSectionGeneralPage, CeCosPropertyPage)

CSectionGeneralPage::CSectionGeneralPage() : CeCosPropertyPage(CSectionGeneralPage::IDD)
{
	//{{AFX_DATA_INIT(CSectionGeneralPage)
	m_strFinalRelativeName = _T("");
	m_strNameUser = _T("");
	m_strNameLinker = _T("");
	m_nAlignment = 0;
	m_strFinalAddress = _T("");
	m_strSectionSize = _T("");
	//}}AFX_DATA_INIT
    m_bFinalAbsolute = TRUE;
}

CSectionGeneralPage::~CSectionGeneralPage()
{
}

void CSectionGeneralPage::DoDataExchange(CDataExchange* pDX)
{
	CeCosPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSectionGeneralPage)
	DDX_Control(pDX, IDC_SECTION_GENERAL_ALIGNMENT, m_cboAlignment);
	DDX_Control(pDX, IDC_SECTION_GENERAL_NAME_USER, m_edtNameUser);
	DDX_Control(pDX, IDC_SECTION_GENERAL_NAME_LINKER, m_cboNameLinker);
	DDX_Control(pDX, IDC_SECTION_GENERAL_FINAL_ABSOLUTE_ADDRESS, m_edtFinalAddress);
	DDX_Control(pDX, IDC_SECTION_GENERAL_FINAL_RELATIVE_NAME, m_cboFinalRelativeName);
	DDX_Control(pDX, IDC_SECTION_GENERAL_KNOWN_SIZE, m_btnSectionSizeKnown);
	DDX_Control(pDX, IDC_SECTION_GENERAL_SIZE, m_edtSectionSize);
	DDX_Control(pDX, IDC_SECTION_GENERAL_FINAL_RELATIVE, m_btnFinalRelative);
	DDX_Control(pDX, IDC_SECTION_GENERAL_FINAL_ABSOLUTE, m_btnFinalAbsolute);
	DDX_Control(pDX, IDC_SECTION_GENERAL_NAME_LINKER_DEFINED, m_btnNameLinkerDefined);
	DDX_Control(pDX, IDC_SECTION_GENERAL_NAME_USER_DEFINED, m_btnNameUserDefined);
	DDX_CBString(pDX, IDC_SECTION_GENERAL_FINAL_RELATIVE_NAME, m_strFinalRelativeName);
	DDX_Text(pDX, IDC_SECTION_GENERAL_NAME_USER, m_strNameUser);
	DDX_CBString(pDX, IDC_SECTION_GENERAL_NAME_LINKER, m_strNameLinker);
	DDX_CBIndex(pDX, IDC_SECTION_GENERAL_ALIGNMENT, m_nAlignment);
	DDX_Text(pDX, IDC_SECTION_GENERAL_FINAL_ABSOLUTE_ADDRESS, m_strFinalAddress);
	DDX_Text(pDX, IDC_SECTION_GENERAL_SIZE, m_strSectionSize);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSectionGeneralPage, CeCosPropertyPage)
	//{{AFX_MSG_MAP(CSectionGeneralPage)
	ON_BN_CLICKED(IDC_SECTION_GENERAL_KNOWN_SIZE, OnSectionSizeKnown)
	ON_BN_CLICKED(IDC_SECTION_GENERAL_FINAL_RELATIVE, OnSectionGeneralFinalType)
	ON_BN_CLICKED(IDC_SECTION_GENERAL_NAME_LINKER_DEFINED, OnSectionGeneralNameType)
	ON_BN_CLICKED(IDC_SECTION_GENERAL_FINAL_ABSOLUTE, OnSectionGeneralFinalType)
	ON_BN_CLICKED(IDC_SECTION_GENERAL_NAME_USER_DEFINED, OnSectionGeneralNameType)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSectionGeneralPage message handlers

void CSectionGeneralPage::OnSectionSizeKnown() 
{
    // enable size edit box according to 'size known' check box

    m_edtSectionSize.EnableWindow (m_btnSectionSizeKnown.GetCheck () && (! m_bNameLinkerDefined));
}

void CSectionGeneralPage::OnSectionGeneralFinalType() 
{
    // enable/disable dialog controls according to absolute/relative radio button state

    m_bFinalAbsolute = m_btnFinalAbsolute.GetCheck();

    m_edtFinalAddress.EnableWindow (m_bFinalAbsolute);
    m_cboFinalRelativeName.EnableWindow (! m_bFinalAbsolute);
    // m_cboFinalRelativeVMA.EnableWindow (! m_bFinalAbsolute); FIXME disabled for now (remove ?)
    if (m_cboAlignment.IsWindowEnabled () && m_bFinalAbsolute)
        m_cboAlignment.SetCurSel (0); // reset to 1 byte alignment
    m_cboAlignment.EnableWindow (! m_bFinalAbsolute);
}

BOOL CSectionGeneralPage::OnInitDialog() 
{
  using namespace std;
	CeCosPropertyPage::OnInitDialog();

    mem_map * lpMemoryMap = & CConfigTool::GetConfigToolDoc()->MemoryMap;

    // copy current and unused section names into the linker-defined section names combo box

    for (list <string>::iterator name = lpMemoryMap->linker_defined_section_list.begin (); name != lpMemoryMap->linker_defined_section_list.end (); ++name)
        if (! lpMemoryMap->section_exists (name->c_str ()) || // section name is unused
           (m_bNameLinkerDefined && (m_strNameLinker == name->c_str ()))) // or section name is that of the current section
           m_cboNameLinker.AddString (CString(name->c_str ()));

    // select the initial name in the combo box

    if (m_bNameLinkerDefined && (m_strNameLinker != _T(""))) // the current selection is linker defined
        m_cboNameLinker.SetCurSel (m_cboNameLinker.FindString (-1, m_strNameLinker));
    else if (m_cboNameLinker.GetCount () > 0) // there are names in the combo box
        m_cboNameLinker.SetCurSel (0); // select the first name
    else // there are no names in the combo
        m_btnNameLinkerDefined.EnableWindow (FALSE); // disable the linker-defined section radio button

    // select the initial section name type radio button

    if ((! m_bNameLinkerDefined) || (m_cboNameLinker.GetCount () == 0))
    {
        m_btnNameUserDefined.SetCheck (1); // user-defined section
        if (m_bNameLinkerDefined)
            m_dwSectionSize = 0; // default section size for new sections
        m_btnSectionSizeKnown.SetCheck (m_dwSectionSize > 0); // of known size
    }
    else
        m_btnNameLinkerDefined.SetCheck (1); // linker-defined section

    OnSectionGeneralNameType (); // refresh the state of other controls

    // copy section names into the relative name combo box

    for (list <mem_section>::iterator section = lpMemoryMap->section_list.begin (); section != lpMemoryMap->section_list.end (); ++section)
        if ((((section->size !=0) || (section->linker_defined)) && // eliminate user-defined sections of unknown size
            ((m_bNameLinkerDefined && (m_strNameLinker == _T(""))) || // if not a new section
                ((section->final_location->following_section == NULL) && // eliminate sections with used anchors
                ((section->relocates) || (section->initial_location->following_section == NULL)))) && // eliminate sections already used as final anchors unless a new section
            (section->name.c_str () != m_strNameLinker) && // eliminate the current section
            (section->name.c_str () != m_strNameUser)) || // eliminate the current section
            ((!m_bFinalAbsolute) && (m_strFinalRelativeName == section->name.c_str()))) // or section name is the current final relative name
            m_cboFinalRelativeName.AddString (CString(section->name.c_str ()));

    // select the initial name in the combo box

    if ((! m_bFinalAbsolute) && (m_strFinalRelativeName != _T("")))
        m_cboFinalRelativeName.SetCurSel (m_cboFinalRelativeName.FindString (-1, m_strFinalRelativeName));
    else if (m_cboFinalRelativeName.GetCount () > 0) // there are names in the combo box
        m_cboFinalRelativeName.SetCurSel (0); // select the first name
    else
        m_btnFinalRelative.EnableWindow (FALSE); // disable the relative type radio button

    // select the initial final location type radio button

    if (m_bFinalAbsolute)
        m_btnFinalAbsolute.SetCheck (1);
    else
        m_btnFinalRelative.SetCheck (1);

    OnSectionGeneralFinalType (); // refresh the state of the other controls

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSectionGeneralPage::OnSectionGeneralNameType() 
{
	// enable/disable dialog controls according to linker/user defined radio button state

    m_bNameLinkerDefined = m_btnNameLinkerDefined.GetCheck ();

    m_cboNameLinker.EnableWindow (m_bNameLinkerDefined);
    m_edtNameUser.EnableWindow (! m_bNameLinkerDefined);
    m_btnSectionSizeKnown.EnableWindow (! m_bNameLinkerDefined);
    OnSectionSizeKnown (); // update state of section size edit box
}

BOOL CSectionGeneralPage::OnKillActive() 
{
  using namespace std;
    if (! UpdateData (TRUE))
        return FALSE;

    if (! m_bNameLinkerDefined)
    {
        if ((m_strNameUser == _T("")) || (m_strNameUser.FindOneOf (LD_ILLEGAL_CHARS) != -1))
        {
            AfxMessageBox (IDS_VALIDATE_SECTION_NAME);
            m_edtNameUser.SetFocus ();
            return FALSE;
        }

        mem_map * lpMemoryMap = & CConfigTool::GetConfigToolDoc()->MemoryMap;
        for (list <string>::iterator name = lpMemoryMap->linker_defined_section_list.begin (); name != lpMemoryMap->linker_defined_section_list.end (); ++name)
            if (m_strNameUser == name->c_str ()) // the user-defined name clashes with a linker-defined name
        {
            AfxMessageBox (IDS_VALIDATE_SECTION_NAME_CLASH);
            m_edtNameUser.SetFocus ();
            m_edtNameUser.SetSel (0, -1); // select all text
            return FALSE;
        }
    }

    // convert address and alignment information to a DWORD representation

    CString strAlignment;
    m_cboAlignment.GetLBText (m_nAlignment, strAlignment);
    _stscanf (strAlignment, _T("%lx"), &m_dwAlignment);

    if (m_bFinalAbsolute)
    {
        TCHAR lpszDummy [2];
        if (_stscanf (m_strFinalAddress, _T("%lx%1s"), &m_dwFinalAddress, lpszDummy) != 1)
        {
            AfxMessageBox (IDS_VALIDATE_SECTION_START_ADDRESS);
            m_edtFinalAddress.SetFocus ();
            m_edtFinalAddress.SetSel (0, -1); // select all text
            return FALSE;
        }
    }

    // convert section size to a DWORD representation

    if ((! m_bNameLinkerDefined) && m_btnSectionSizeKnown.GetCheck ())
    {
        TCHAR lpszDummy [2];
        if (_stscanf (m_strSectionSize, _T("%lx%1s"), &m_dwSectionSize, lpszDummy) != 1)
        {
            AfxMessageBox (IDS_VALIDATE_SECTION_SIZE);
            m_edtSectionSize.SetFocus ();
            m_edtSectionSize.SetSel (0, -1); // select all text
            return FALSE;
        }
    }
    else
        m_dwSectionSize = 0;
    
	return CeCosPropertyPage::OnKillActive();
}


BOOL CSectionGeneralPage::OnSetActive() 
{
    // generate hex strings for display

    if ((m_bNameLinkerDefined && (m_strNameLinker == _T(""))) ||
        ((! m_bNameLinkerDefined) && (m_strNameUser == _T("")))) // a new section
    {
        m_strFinalAddress = _T("");
        m_strSectionSize = _T("");
    }
    else if (m_bFinalAbsolute)
        m_strFinalAddress.Format (_T("%08lX"), m_dwFinalAddress);

    if (m_bNameLinkerDefined || (m_dwSectionSize == 0))
        m_strSectionSize == _T("");
    else
        m_strSectionSize.Format (_T("%lX"), m_dwSectionSize);

    if (m_bFinalAbsolute)
        m_nAlignment = 0;
    else
    {
        CString strAlignment;
        strAlignment.Format (_T("%08lX"), m_dwAlignment);
        m_nAlignment = m_cboAlignment.FindString (-1, strAlignment);
    }

    if (! UpdateData (FALSE))
        return FALSE;

	return CeCosPropertyPage::OnSetActive();
}

