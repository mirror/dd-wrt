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
//        SectionRelocationPage.cpp
//
//        Memory Layout Tool section relocation property page class
//
//=================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     John Dallaway
// Contact(s):    jld
// Date:          1998/07/29 $RcsDate$ {or whatever}
// Version:       0.00+  $RcsVersion$ {or whatever}
// Purpose:       Provides a derivation of the MFC CeCosPropertyPage class for
//                relocation section property selection
// See also:      SectionRelocationPage.h
// Known bugs:    <UPDATE_ME_AT_RELEASE_TIME>
//
//####DESCRIPTIONEND####

#include "stdafx.h"
#include "ConfigtoolDoc.h"
#include "SectionRelocationPage.h"
#include "ConfigTool.h"
#include "CTUtils.h"

#include "memmap.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSectionRelocationPage property page

IMPLEMENT_DYNCREATE(CSectionRelocationPage, CeCosPropertyPage)

CSectionRelocationPage::CSectionRelocationPage() : CeCosPropertyPage(CSectionRelocationPage::IDD)
{
	//{{AFX_DATA_INIT(CSectionRelocationPage)
	m_bRelocates = FALSE;
	m_strInitialRelativeName = _T("");
	m_strInitialAddress = _T("");
	//}}AFX_DATA_INIT
    m_bNewSection = FALSE;
}


CSectionRelocationPage::~CSectionRelocationPage()
{
}


void CSectionRelocationPage::DoDataExchange(CDataExchange* pDX)
{
	CeCosPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSectionRelocationPage)
	DDX_Control(pDX, IDC_SECTION_RELOCATION_RELOCATES, m_btnRelocates);
	DDX_Control(pDX, IDC_SECTION_RELOCATION_INITIAL_RELATIVE, m_btnInitialRelative);
	DDX_Control(pDX, IDC_SECTION_RELOCATION_INITIAL_ABSOLUTE, m_btnInitialAbsolute);
	DDX_Control(pDX, IDC_SECTION_RELOCATION_INITIAL_ABSOLUTE_ADDRESS, m_edtInitialAddress);
	DDX_Control(pDX, IDC_SECTION_RELOCATION_INITIAL_RELATIVE_NAME, m_cboInitialRelativeName);
	DDX_Check(pDX, IDC_SECTION_RELOCATION_RELOCATES, m_bRelocates);
	DDX_CBString(pDX, IDC_SECTION_RELOCATION_INITIAL_RELATIVE_NAME, m_strInitialRelativeName);
	DDX_Text(pDX, IDC_SECTION_RELOCATION_INITIAL_ABSOLUTE_ADDRESS, m_strInitialAddress);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSectionRelocationPage, CeCosPropertyPage)
	//{{AFX_MSG_MAP(CSectionRelocationPage)
	ON_BN_CLICKED(IDC_SECTION_RELOCATION_RELOCATES, OnSectionRelocationRelocates)
	ON_BN_CLICKED(IDC_SECTION_RELOCATION_INITIAL_RELATIVE, OnSectionRelocationInitialType)
	ON_BN_CLICKED(IDC_SECTION_RELOCATION_INITIAL_ABSOLUTE, OnSectionRelocationInitialType)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSectionRelocationPage message handlers

void CSectionRelocationPage::OnSectionRelocationRelocates() 
{
    OnSectionRelocationInitialType ();	
}


void CSectionRelocationPage::OnSectionRelocationInitialType() 
{
    const BOOL bRelocates = m_btnRelocates.GetState() & 0x0003;

    m_bInitialAbsolute = m_btnInitialAbsolute.GetState() & 0x0003;

    m_btnInitialAbsolute.EnableWindow (bRelocates);
    m_btnInitialRelative.EnableWindow (bRelocates && (m_cboInitialRelativeName.GetCount () > 0));
    m_edtInitialAddress.EnableWindow (m_bInitialAbsolute && bRelocates);
    m_cboInitialRelativeName.EnableWindow (bRelocates && ! m_bInitialAbsolute);	
}


BOOL CSectionRelocationPage::OnInitDialog() 
{
  using namespace std;
	CeCosPropertyPage::OnInitDialog();
	
    m_btnInitialAbsolute.SetCheck (1);
	
    mem_map * lpMemoryMap = & CConfigTool::GetConfigToolDoc()->MemoryMap;

    // copy section names into the relative name combo box

    for (list <mem_region>::iterator region = lpMemoryMap->region_list.begin (); region != lpMemoryMap->region_list.end (); ++region)
        if (region->type == read_only) // initial location of relocating section must be in a read-only region
            for (list <mem_section_view>::iterator section_view = region->section_view_list.begin (); section_view != region->section_view_list.end (); ++section_view)
                if ((section_view->section != NULL) && (section_view->section_location != final_location) && // not the final location of a relocating section
                   ((section_view->section->size !=0) || (section_view->section->linker_defined)) && // eliminate user-defined sections of unknown size
                   (m_bNewSection || (section_view->section->initial_location->following_section == NULL)) || // eliminate sections already used as initial anchors unless a new section
                   ((section_view->section != NULL) && (!m_bInitialAbsolute) && (m_strInitialRelativeName == section_view->section->name.c_str()))) // or section name is the current initial relative name
                   m_cboInitialRelativeName.AddString (CString(section_view->section->name.c_str ()));

    if (m_cboInitialRelativeName.GetCount () > 0) // there are anchor sections available
        m_cboInitialRelativeName.SetCurSel (0); // select the first item in the combo box (if any)
    else // there are no anchor sections available
        m_btnInitialRelative.EnableWindow (FALSE); // disable the relative section radio button

    m_btnRelocates.SetCheck (m_bRelocates);
    m_btnInitialAbsolute.SetCheck (m_bInitialAbsolute);
    m_btnInitialRelative.SetCheck (! m_bInitialAbsolute);
    OnSectionRelocationRelocates ();

    if ((! m_bInitialAbsolute) && (m_bRelocates))
        m_cboInitialRelativeName.SetCurSel (m_cboInitialRelativeName.FindString (-1, m_strInitialRelativeName));
    else if (m_cboInitialRelativeName.GetCount () > 0) // there are names in the combo box
        m_cboInitialRelativeName.SetCurSel (0); // select the first name
    else
        m_btnInitialRelative.EnableWindow (FALSE); // disable the relative type radio button

    return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


BOOL CSectionRelocationPage::OnKillActive() 
{
    if (! UpdateData (TRUE))
        return FALSE;

    // convert address to a DWORD representation

    if (m_bRelocates && m_bInitialAbsolute)
    {
        char lpszDummy [2];
        if (_stscanf (m_strInitialAddress, _T("%lx%1s"), &m_dwInitialAddress, lpszDummy) != 1)
        {
            AfxMessageBox (IDS_VALIDATE_SECTION_START_ADDRESS);
            m_edtInitialAddress.SetFocus ();
            m_edtInitialAddress.SetSel (0, -1); // select all text
            return FALSE;
        }
    }
	
	return CeCosPropertyPage::OnKillActive();
}


BOOL CSectionRelocationPage::OnSetActive() 
{
    if (m_bRelocates && m_bInitialAbsolute)
        m_strInitialAddress.Format (_T("%08lX"), m_dwInitialAddress);
    else
        m_strInitialAddress = _T("");
	
    if (! UpdateData (FALSE))
        return FALSE;

    return CeCosPropertyPage::OnSetActive();
}

