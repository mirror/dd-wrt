//####COPYRIGHTBEGIN####
//
// ----------------------------------------------------------------------------
// Copyright (C) 1998, 1999, 2000 Red Hat, Inc.
// Copyright (C) 2003 John Dallaway
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
// configtooldoc.cpp :
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   julians, jld
// Contact(s):  julians, jld
// Date:        2000/10/05
// Version:     $Id: configtooldoc.cpp,v 1.43 2002/02/13 13:58:18 julians Exp $
// Purpose:
// Description: Implementation file for the ecConfigToolDoc class
// Requires:
// Provides:
// See also:
// Known bugs:
// Usage:
//
//####DESCRIPTIONEND####
//
//===========================================================================

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------
#ifdef __GNUG__
#pragma implementation "configtooldoc.h"
#endif

// Includes other headers for precompiled compilation
#include "ecpch.h"

#include "wx/config.h"
#include "wx/textfile.h"
#include "wx/process.h"
#include "wx/mimetype.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifdef __WXMSW__
#include <windows.h>
#ifndef __CYGWIN__
#include <shlobj.h>
#endif
#include "wx/msw/winundef.h"
#ifdef CreateDirectory
#undef CreateDirectory
#endif
#endif

#include "configtooldoc.h"
#include "configtoolview.h"
#include "configtree.h"
#include "mainwin.h"
#include "ecutils.h"
#include "filename.h"
#include "choosereposdlg.h"
#include "packagesdlg.h"
#include "conflictsdlg.h"
#include "conflictwin.h"
#include "mltwin.h"
#include "build.hxx"
#include "platformeditordlg.h"
#include "runtestsdlg.h"
#include "propertywin.h"
#include "docsystem.h"

IMPLEMENT_DYNAMIC_CLASS(ecConfigToolDoc, wxDocument)

ecConfigToolDoc::ecConfigToolDoc()
{
    m_bRepositoryOpen = FALSE;
    m_CdlPkgData = NULL;
    m_CdlInterp = NULL;
    m_CdlConfig = NULL;
    m_ConflictsOutcome = OK;
    m_strRepository = wxGetApp().GetSettings().m_strRepository;
}

ecConfigToolDoc::~ecConfigToolDoc()
{
    wxGetApp().m_currentDoc = NULL;
    wxGetApp().GetSettings().m_strRepository = m_strRepository;

    CloseRepository();

    // Delete remaining items -- most (if not all) should already
    // have been deleted via the tree item client data
    DeleteItems();
}

void ecConfigToolDoc::DeleteItems()
{
    // Delete any remaining items
    wxNode* node = m_items.First();
    while (node)
    {
        ecConfigItem* item = wxDynamicCast(node->Data(), ecConfigItem);
        wxNode* next = node->Next();

        // Note: automatically removes itself from this list in ~ecConfigItem
        delete item;
        node = next;
    }
}

bool ecConfigToolDoc::OnCloseDocument()
{
    if (wxDocument::OnCloseDocument())
    {
        DeleteItems();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

bool ecConfigToolDoc::Save()
{
    bool ret = FALSE;

    if (!IsModified() && m_savedYet) return TRUE;
    if (m_documentFile == wxT("") || !m_savedYet)
        ret = SaveAs();
    else
        ret = OnSaveDocument(m_documentFile);
    if ( ret )
        SetDocumentSaved(TRUE);
    return ret;
}


bool ecConfigToolDoc::OnCreate(const wxString& path, long flags)
{
    wxGetApp().m_currentDoc = this;

    if (flags & wxDOC_NEW)
    {
        m_bRepositoryOpen = FALSE;

        bool prompt = FALSE;
        if (flags & ecDOC_PROMPT_FOR_REPOSITORY)
            prompt = TRUE;

        if(!OpenRepository(wxEmptyString, prompt))
        {
            wxGetApp().m_currentDoc = NULL;
            return FALSE;
        }

        Modify(FALSE);
        SetDocumentSaved(FALSE);

        wxString rootName(wxT("untitled"));
        wxStripExtension(rootName);
        SetFilename(wxGetApp().GetSettings().GenerateFilename(rootName));
    }

    // Creates the view, so do any view updating after this
    bool success = wxDocument::OnCreate(path, flags);

    if (success)
    {
        if (flags & wxDOC_NEW)
        {
            wxBusyCursor wait;

            ecConfigToolHint hint(NULL, ecSelChanged);
            UpdateAllViews (NULL, & hint);

            SetFilename(GetFilename(), TRUE);

            // load the memory layout for the default target-platform-startup from the current repository

            // TODO
            // m_memoryMap.set_map_size (0xFFFFFFFF); // set the maximum memory map size
            // NewMemoryLayout (CFileName (m_strPackagesDir, m_strMemoryLayoutFolder, _T("include\\pkgconf")));

            // Why should we generate the names at this point, when we only have a temporary filename?
            // Don't do it!
#if 0
            wxGetApp().SetStatusText(wxT("Updating build information..."), FALSE);
            UpdateBuildInfo();
#endif
        }
    }
    return success;
}

bool ecConfigToolDoc::OnSaveDocument(const wxString& filename)
{
    wxBusyCursor cursor;

    const wxString strOldPath(GetFilename());

#if 0
    bool bSaveAs=(filename!=strOldPath);
    if(!IsModified() && wxFileExists(filename))
    {
        return TRUE;
    }
#endif

    bool rc=FALSE;
    if (CheckConflictsBeforeSave())
    { // errors already emitted

        const wxString strPathName(filename);

        wxString str;
        str.Printf(_("Saving configuration %s"), (const wxChar*) filename);

        /* TODO
        CIdleMessage IM(str);
        if(CConfigTool::GetCellView()){
        CConfigTool::GetCellView()->CancelCellEdit();
        }
        */

        // check the configuration

        wxASSERT (m_CdlConfig->check_this (cyg_extreme));

        // save the configuration

        try
        {
            m_CdlConfig->save ((const wxChar*) filename);
            rc=TRUE;
        }

        catch (CdlStringException exception)
        {
            wxString msg;
            msg.Printf(_("Error saving eCos configuration:\n\n%s"), exception.get_message ().c_str ());
            wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK);
        }

        catch (...)
        {
            wxString msg;
            msg.Printf(_("Error saving eCos configuration"));
            wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK);
        }

        if(rc){
            rc=FALSE;
            SetFilename (filename); // called to ensure that MLTDir() will work in this function TODO??

            // save the memory layout files to the build tree and copy to the install tree
            /* TODO
            if (bSaveAs || MemoryMap.map_modified ()) {
            SaveMemoryMap();
            }
            */

            ecConfigToolHint hint(NULL, ecAllSaved);
            UpdateAllViews (NULL, & hint);

            wxASSERT( !m_strBuildTree.IsEmpty() );
            wxASSERT( !m_strInstallTree.IsEmpty() );

            ecFileName buildFilename(m_strBuildTree);
            ecFileName installFilename(m_strInstallTree);

            if (!wxGetApp().GetSettings().m_editSaveFileOnly)
            {
                if (!buildFilename.CreateDirectory(FALSE) || !installFilename.CreateDirectory(FALSE))
                {
                    wxString msg;
                    msg.Printf(_("Failed to save %s"), (const wxChar*) filename);
                    
                    wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION);
                    rc = FALSE;
                }
                else if (GenerateHeaders() && CopyMLTFiles())
                {
                    // in each case errors already emitted
                    // copy new MLT files to the build tree as necessary
                    rc=generate_build_tree (GetCdlConfig(), ecUtils::UnicodeToStdStr(m_strBuildTree), ecUtils::UnicodeToStdStr(m_strInstallTree));
                    rc = TRUE;
                }
            }
            else
            {
                rc = TRUE;
            }
        }
    }
    if(rc)
    {
        Modify(FALSE);
        SetDocumentSaved(TRUE);
        SetFilename(filename);
        wxGetApp().GetSettings().m_lastFilename = filename;
    } else
    {
        SetFilename(strOldPath);
    }
    wxGetApp().GetMainFrame()->UpdateFrameTitle();
    return rc;
}

// Can we generate the build tree yet?
bool ecConfigToolDoc::CanGenerateBuildTree()
{
    if (m_strBuildTree.IsEmpty() || m_strInstallTree.IsEmpty() )
        return FALSE;

    int nCount=0;
    if (GetCdlConfig ())
    {
        // calculate the number of conflicts
        int nCount = GetCdlConfig ()->get_all_conflicts ().size ();

        if (nCount > 0)
            return FALSE;
    }
    else
        return FALSE;

    return TRUE;
}

// A standalone method for generating a build tree without saving first
bool ecConfigToolDoc::GenerateBuildTree()
{
    wxBusyCursor wait;
    if (CanGenerateBuildTree())
    {
        ecFileName buildFilename(m_strBuildTree);
        ecFileName installFilename(m_strInstallTree);
        
        if (!buildFilename.CreateDirectory() || !installFilename.CreateDirectory())
        {
            wxString msg;
            msg.Printf(_("Failed to create build tree"));
            
            wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION);
            return FALSE;
        }
        else if (GenerateHeaders() && CopyMLTFiles())
        {
            // in each case errors already emitted
            // copy new MLT files to the build tree as necessary
            bool rc = generate_build_tree (GetCdlConfig(), ecUtils::UnicodeToStdStr(m_strBuildTree), ecUtils::UnicodeToStdStr(m_strInstallTree));
            rc = TRUE;
        }

    }
    else
        return FALSE;
    return TRUE;
}

bool ecConfigToolDoc::OnOpenDocument(const wxString& filename)
{
    wxGetApp().GetSettings().m_lastFilename = filename;

    wxBusyCursor cursor;

    bool rc=FALSE; // Assume the worst
    CdlInterpreter NewCdlInterp = NULL;
    CdlConfiguration NewCdlConfig = NULL;

    // We have to open the repository or m_CdlPkgData and co. won't be set
    if (!OpenRepository())
        return FALSE;

    wxString str;
    str.Printf(_("Opening save file %s"), (const wxChar*) filename);
    wxGetApp().SetStatusText(str);

    EnableCallbacks(FALSE);

    try
    {
        NewCdlInterp = CdlInterpreterBody::make ();
        NewCdlConfig = CdlConfigurationBody::load ((const wxChar*) filename, m_CdlPkgData, NewCdlInterp, &CdlLoadErrorHandler, &CdlLoadWarningHandler);
        rc = TRUE;
    }
    catch (CdlStringException exception)
    {
        wxString msg;
        msg.Printf(_("Error opening eCos configuration:\n\n%s"), exception.get_message ().c_str ());
        wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK);
    }
    catch (...)
    {
        wxString msg;
        msg.Printf(_("Error opening eCos configuration"));
        wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK);
    }

    if (rc)
    {
        rc=FALSE;
        // check the new configuration

        wxASSERT (NewCdlConfig->check_this (cyg_extreme));

        // switch to the new configuration

        delete m_CdlConfig;
        delete m_CdlInterp;
        m_CdlInterp = NewCdlInterp;
        m_CdlConfig = NewCdlConfig;
        //SetPathName (lpszPathName, TRUE); // called to ensure that MLTDir() will work in this function

        AddAllItems (); // must precede NewMemoryLayout() [CurrentLinkerScript() calls Find()]

        // load the memory layout from the build tree
        // TODO
        NewMemoryLayout (MLTDir ());

        UpdateFailingRuleCount();

        SetFilename(filename);

	// UpdateBuildInfo(); // Don't create directories on opening file

        rc = TRUE;


        ecConfigToolHint hint(NULL, ecFilenameChanged);
        UpdateAllViews (NULL, & hint);
    }

    // re-enable the transaction callback
    EnableCallbacks(TRUE);

    SetDocumentSaved(TRUE); // Necessary or it will pop up the Save As dialog

    wxGetApp().SetStatusText(wxEmptyString, FALSE);

    return rc;
}

void ecConfigToolDoc::AddAllItems()
{
    ecConfigTreeCtrl* treeCtrl = wxGetApp().GetMainFrame()->GetTreeCtrl();
    // Ensure there's no dangling pointer
    wxGetApp().GetMainFrame()->GetPropertyListWindow()->Fill(NULL);

    treeCtrl->DeleteAllItems();

    m_strMemoryLayoutFolder = wxT("");
    m_strLinkerScriptFolder = wxT("");

    // Add the root item
    ecConfigItem* item = NULL;
    wxTreeItemId rootId = treeCtrl->AddRoot(_(""), -1, -1, new ecTreeItemData(item = new ecConfigItem(NULL, _("Configuration"), ecContainer)));
    item->SetTreeItem(rootId);
    item->UpdateTreeItem(* treeCtrl);
    item->SetDescription(_("The root node for all configurable items"));
    m_items.Append(item);

    AddContents(m_CdlConfig, item);
    treeCtrl->Expand(rootId);

    // check that exactly one radio button in each group is enabled
    CheckRadios ();

    // update the rules (conflicts) view
    UpdateFailingRuleCount ();

    if( ! wxGetApp().GetMainFrame() || ! wxGetApp().GetMainFrame()->GetConflictsWindow() ||
        ! wxGetApp().GetMainFrame()->GetConflictsWindow()->IsShown())
    {
        // log all conflicts
        //	LogConflicts (m_CdlConfig->get_structural_conflicts ()); // relating to package availability - ignore for now
        LogConflicts (m_CdlConfig->get_all_conflicts ());
    }

    wxGetApp().GetTreeCtrl()->SelectItem(wxGetApp().GetTreeCtrl()->GetRootItem());
    //ecConfigToolHint hint(item, ecValueChanged);
    ecConfigToolHint hint(NULL, ecSelChanged);

    UpdateAllViews (NULL, & hint);

    if(GetItems().Number()>0){
        wxGetApp().GetTreeCtrl()->Expand(rootId);
    }
    wxGetApp().GetTreeCtrl()->SetFocus();
}

void ecConfigToolDoc::AddContents (const CdlContainer container, ecConfigItem *pParent)
{
    // determine the container contents

    const std::vector<CdlNode>& contents = container->get_contents ();
    std::vector<CdlNode>::const_iterator node_i;
    for (node_i = contents.begin (); node_i != contents.end (); node_i++)
    {
        const CdlNode node = * node_i;
        const CdlPackage pkg = dynamic_cast<CdlPackage> (node);
        const CdlComponent comp = dynamic_cast<CdlComponent> (node);
        const CdlOption opt = dynamic_cast<CdlOption> (node);
        const CdlContainer contnr = dynamic_cast<CdlContainer> (node);

        // if the node in the container is a package, component or option
        // then it is visible and should be added to the tree
        if  (0 != pkg) // the node is a package
        {
            ecConfigItem * pItem = AddItem (pkg, pParent); // add the package
            AddContents (pkg, pItem); // add the package contents
        }
        else if (0 != comp) // the node is a component
        {
            ecConfigItem * pItem = AddItem (comp, pParent); // add the component
            AddContents (comp, pItem); // add the component contents
        }
        else if (0 != opt) // the node is an option
            AddItem (opt, pParent); // add the option

        else if (0 != contnr) // if the node is a container
            AddContents (contnr, pParent); // add the container contents

        // ignore nodes of any other class
    }
}

ecConfigItem * ecConfigToolDoc::AddItem (const CdlUserVisible vitem, ecConfigItem * pParent)
{
    ecConfigItem * pItem = new ecConfigItem (pParent, vitem);

    m_items.Append(pItem);

    if (vitem->get_name () == "CYGHWR_MEMORY_LAYOUT")
    {
        wxASSERT (m_strMemoryLayoutFolder.IsEmpty ());
        m_strMemoryLayoutFolder = vitem->get_owner ()->get_directory().c_str ();
#ifdef __WXMSW__
        m_strMemoryLayoutFolder.Replace(wxT("/"),wxT("\\"));
#endif
        //TRACE (_T("Found memory layout folder: %s\n"), m_strMemoryLayoutFolder);
    }

    if (vitem->get_name () == "CYGBLD_LINKER_SCRIPT")
    {
        wxASSERT (m_strLinkerScriptFolder.IsEmpty ());
        m_strLinkerScriptFolder = vitem->get_owner ()->get_directory().c_str ();
#ifdef __WXMSW__
        m_strLinkerScriptFolder.Replace(wxT("/"),wxT("\\"));
#endif
        //TRACE (_T("Found linker script folder: %s\n"), m_strLinkerScriptFolder);

        // the CDL hardware template name will eventually become the target name,
        // but for now we must deduce the target name from the linker script file name

        const CdlValuable valuable = dynamic_cast<CdlValuable> (vitem);
        ecFileName strLinkerScript (m_strPackagesDir, m_strLinkerScriptFolder, wxString (valuable->get_value ().c_str ()));

#ifdef __WXMSW__
        strLinkerScript.Replace (wxT("/"), wxT("\\"));
#endif

        if(!strLinkerScript.Exists ()){
            wxString msg;
            msg.Printf(wxT("%s does not exist\n"), (const wxChar*) strLinkerScript);
            wxGetApp().Log(msg);
        }
        //TRACE (_T("Target '%s' selected\n"), strLinkerScript.Tail ().Root (), pItem->Macro());
    }

    //TRACE(_T("Created new item from cdl: "));
    //pItem->DumpItem();
    return pItem;
}

void ecConfigToolDoc::CheckRadios()
{
    int nItem;
    for(nItem=0; nItem < GetItems().Number() ; nItem++)
    {
        ecConfigItem *pItem=(ecConfigItem*) GetItems()[nItem];

        if(pItem->HasRadio () && pItem==pItem->FirstRadio())
        {
            wxString strMsg;
            ecConfigItem *pFirstSet=NULL;
            ecConfigItem *pSibItem;

            for ( pSibItem=pItem; pSibItem; pSibItem = pSibItem->NextRadio() )
            {
                if(pSibItem->IsEnabled ())
                {
                    if(pFirstSet)
                    {
                        strMsg += wxT(" ");
                        strMsg += pSibItem->GetMacro ();
                    } else
                    {
                        pFirstSet = pSibItem;
                    }
                }
            }

            if ( !strMsg.IsEmpty() )
            {
                wxString msg2;
                msg2.Printf(_("%s, multiple radio buttons are set: %s%s"),
                    (const wxChar*) pItem->GetMacro(), (const wxChar*) pFirstSet->GetMacro(), (const wxChar*) strMsg);
                wxGetApp().Log(msg2);
            } else if ( !pFirstSet )
            {
                wxString msg2;
                msg2.Printf(_("%s, no radio buttons are set"), (const wxChar*) pItem->GetMacro());
                wxGetApp().Log(msg2);
            }
        }
    }
}

// Find the ecConfigItem referencing the given CdlValuable
ecConfigItem * ecConfigToolDoc::Find (CdlValuable v)
{
    int nItem;
    for (nItem=0 ; nItem < m_items.Number() ; nItem++)
    {
        ecConfigItem *pItem = (ecConfigItem*) m_items[nItem];
        if( v == pItem->GetCdlValuable() )
        {
            return pItem;
        }
    }
    return NULL;
}

ecConfigItem * ecConfigToolDoc::Find(const wxString & strWhat, ecWhereType where)
{
    int nItem;
    for (nItem=0 ; nItem < m_items.Number() ; nItem++)
    {
        ecConfigItem *pItem = (ecConfigItem*) m_items[nItem];
        if (pItem->StringValue(where) == strWhat)
        {
            return pItem;
        }
    }
    return NULL;
}

// a trivial CDL load error handler
void ecConfigToolDoc::CdlLoadErrorHandler (std::string message)
{
    wxGetApp().Log(message.c_str());
};

// a trivial CDL load warning handler
void ecConfigToolDoc::CdlLoadWarningHandler (std::string message)
{
    wxGetApp().Log(message.c_str());
};

// a trivial CDL parse error handler
void ecConfigToolDoc::CdlParseErrorHandler (std::string message)
{
    wxGetApp().Log(message.c_str());
};

// a trivial CDL parse warning handler
void ecConfigToolDoc::CdlParseWarningHandler (std::string message)
{
    wxGetApp().Log(message.c_str());
};


void ecConfigToolDoc::CloseRepository()
{
    if(m_bRepositoryOpen){
        // delete the libCDL objects with the document
        EnableCallbacks(FALSE); // first disable the transaction handler
        delete m_CdlConfig; m_CdlConfig = NULL;
        delete m_CdlInterp; m_CdlInterp = NULL;
        delete m_CdlPkgData; m_CdlPkgData = NULL;
        m_bRepositoryOpen=FALSE;
    }
}

bool ecConfigToolDoc::OpenRepository(const wxString& pszRepository /* = wxEmptyString */, bool bPromptInitially/* =FALSE */)
{
    ecMainFrame* mainFrame = wxGetApp().GetMainFrame();
    if(!m_bRepositoryOpen)
    {
        UpdateFailingRuleCount();

        ecFileName strNewRepository;
        while(!m_bRepositoryOpen)
        {
            if(bPromptInitially)
            {
                ecChooseRepositoryDialog dlg(wxGetApp().GetTopWindow());

                if(wxID_CANCEL==dlg.ShowModal()){
                    wxGetApp().SetStatusText(wxEmptyString);
                    return FALSE;
                }
#ifdef __WXMSW__
                // Ensure display gets updated
                ::UpdateWindow((HWND) mainFrame->GetHWND());
                //wxYield();
#endif
                strNewRepository=dlg.GetFolder();
            } else
            {
                // Use what came in as parameter or what was found in registry
                if (!pszRepository.IsEmpty())
                    strNewRepository = pszRepository;
                else
                    strNewRepository = m_strRepository;

                bPromptInitially=TRUE;
            }
            wxString str;
            if (strNewRepository.IsEmpty())
                str.Printf(_("Opening repository..."));
            else
                str.Printf(_("Opening repository %s..."), (const wxChar*) strNewRepository);
            wxGetApp().SetStatusText(str);

            CdlPackagesDatabase NewCdlPkgData = NULL;
            CdlInterpreter      NewCdlInterp  = NULL;
            CdlConfiguration    NewCdlConfig  = NULL;
            wxString strNewPackagesDir;

            EnableCallbacks(FALSE); // disable transaction callbacks until the config tree is regenerated

            wxBusyCursor wait;
            if(OpenRepository(strNewRepository,NewCdlPkgData,NewCdlInterp,NewCdlConfig,strNewPackagesDir))
            {
                // select the "default" template if it exists
                // otherwise select the first available template

                std::string default_template = "default";
                if (! NewCdlPkgData->is_known_template (default_template))
                {
                    const std::vector<std::string>& templates = NewCdlPkgData->get_templates ();
                    if (templates.size () != 0)
                        default_template = templates [0];
                }

                m_templateVersion = "";
                try
                {
                    const std::vector<std::string>& template_versions = NewCdlPkgData->get_template_versions (default_template);
                    NewCdlConfig->set_template (default_template, template_versions [0], &CdlParseErrorHandler, &CdlParseWarningHandler);
                    m_templateVersion = template_versions [0].c_str();
                }
                catch (CdlStringException exception) {
                    wxString msg;
                    msg.Printf(_("Error loading package template '%s':\n\n%s"), default_template.c_str (), exception.get_message ().c_str ());
                    wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK);
                }
                catch (...) {
                    wxString msg;
                    msg.Printf(_("Error loading package template '%s'."), default_template.c_str ());
                    wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK);
                }

                // check the configuration
                wxASSERT (NewCdlConfig->check_this (cyg_extreme));

                // use the new package database, interpreter and configuration
                delete m_CdlConfig; // delete the previous configuration
                delete m_CdlInterp; // delete the previous interpreter
                delete m_CdlPkgData; // delete the previous package database

                m_CdlPkgData = NewCdlPkgData;
                m_CdlInterp  = NewCdlInterp;
                m_CdlConfig  = NewCdlConfig;

                // save the repository location

                SetRepository(strNewRepository);
                m_strPackagesDir = strNewPackagesDir;

                // clear the previously specified document file name (if any),
                // OnNewDocument() calls DeleteContents() so must be called
                // before AddAllItems()

                wxDocument::OnNewDocument ();

                // generate the CConfigItems from their CDL descriptions
                AddAllItems ();

                m_bRepositoryOpen=TRUE;

                // Rebuild help index if it needs building
                RebuildHelpIndex(FALSE);

            } else {
                // failure
                delete NewCdlConfig; NewCdlConfig = NULL;
                delete NewCdlInterp; NewCdlInterp = NULL;
                delete NewCdlPkgData; NewCdlPkgData = NULL;

            }

            // install a transaction handler callback function now that the tree exists
            EnableCallbacks(TRUE);
        }

    }
    wxGetApp().SetStatusText(wxEmptyString, FALSE);
    return m_bRepositoryOpen;
}

// Find a valid repository given a directory name
bool ecConfigToolDoc::FindRepository (ecFileName& repositoryIn, ecFileName& repositoryOut) const
{
    if (repositoryIn.IsEmpty())
	return FALSE;
    
    if (ecFileName(repositoryIn + wxT("ecos.db")).Exists())
    {
	repositoryOut = repositoryIn;
	repositoryIn = wxPathOnly(repositoryIn);
	return TRUE;
    }
    else if (ecFileName(ecFileName(repositoryIn) + ecFileName(wxT("ecc")) + wxT("ecos.db")).Exists())
    {
	repositoryOut = repositoryIn + wxT("ecc");
	return TRUE;
    }
    else if (ecFileName(ecFileName(repositoryIn) + ecFileName(wxT("packages")) + wxT("ecos.db")).Exists())
    {
	repositoryOut = repositoryIn + wxT("packages");
	return TRUE;
    }
    else
    {
	return FALSE;
    }
}


bool ecConfigToolDoc::OpenRepository (ecFileName& strNewRepository, CdlPackagesDatabase &NewCdlPkgData,CdlInterpreter &NewCdlInterp,CdlConfiguration &NewCdlConfig, wxString &strNewPackagesDir)
{
    bool rc=FALSE;

    if (strNewRepository.IsEmpty())
        return FALSE;

    ecFileName strNewPackagesDir1;
    if (!FindRepository(strNewRepository, strNewPackagesDir1))
    {
        wxString msg;
        msg.Printf(_("%s does not seem to be a source repository."),
                    (const wxChar*) strNewRepository);
        wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK);
    } else
    {
                strNewPackagesDir = strNewPackagesDir1;

                const wxString strDatabase = strNewPackagesDir + wxString(wxFILE_SEP_PATH) + wxT("ecos.db");
                if(!wxFileExists(strDatabase))
                {
                    wxString msg;
                    msg.Printf(_("%s does not seem to be a source repository: %s does not exist"), (const wxChar*) strNewRepository, (const wxChar*) strDatabase);
                    wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK);
                } else
		{

                    // create a CDL repository, interpreter and configuration
                    try {// create a new package database, interpreter and configuration
                        NewCdlPkgData = CdlPackagesDatabaseBody::make ((const wxChar*) ecUtils::NativeToPosixPath(strNewPackagesDir), &CdlParseErrorHandler, &CdlParseWarningHandler);
                        NewCdlInterp = CdlInterpreterBody::make ();
                        NewCdlConfig = CdlConfigurationBody::make ("eCos", NewCdlPkgData, NewCdlInterp);
                    }
                    catch (CdlStringException exception)
                    {
                        wxString msg;
                        msg.Printf(_("Error opening eCos repository:\n\n%s"), exception.get_message ().c_str ());
                        wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK);
                        return FALSE;
                    }
                    catch (...)
                    {
                        wxString msg;
                        msg.Printf(_("Error opening eCos repository."));
                        wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK);
                        return FALSE;
                    }

                    // select the default target if specified in the registry
                    // otherwise select the first available target
                    wxString default_hardware = GetDefaultHardware ();

                    if (! NewCdlPkgData->is_known_target ((const wxChar*) default_hardware)) {
                        const std::vector<std::string>& targets = NewCdlPkgData->get_targets ();
                        if (targets.size () == 0)
                        {
                            wxString msg;
                            msg.Printf(_("Error opening eCos repository:\n\nno hardware templates"));
                            wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK);
                            return FALSE;
                        } else {
                            default_hardware = targets [0].c_str();
                        }
                    }

                    try {
                        m_strCdlErrorMessage = wxT("");
                        NewCdlConfig->set_hardware ((const wxChar*) default_hardware, &CdlParseErrorHandler, &CdlParseWarningHandler);
                    }
                    catch (CdlStringException exception) {
                        if (m_strCdlErrorMessage.IsEmpty ())
                        {
                            wxString msg;
                            msg.Printf(_("Error loading the default hardware template '%s':\n\n%s"), default_hardware.c_str (), exception.get_message ().c_str ());
                            wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK);
                        }
                        else // report the most recent parser message in the message box since there may be no output pane in which to view it
                        {
                            wxString msg;
                            msg.Printf(_("Error loading the default hardware template '%s':\n\n%s\n\nParser message:\n\n%s"), default_hardware.c_str (), exception.get_message ().c_str (), (const wxChar*) m_strCdlErrorMessage);
                            wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK);
                        }
                        return FALSE;
                    }
                    catch (...) {
                        wxString msg;
                        msg.Printf(_("Error loading the default hardware template '%s':\n\n%s"), default_hardware.c_str (), (const wxChar*) m_strCdlErrorMessage);
                        wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK);
                        return FALSE;
                    }
                    rc=TRUE;
                }
    }

    return rc;
}

void ecConfigToolDoc::SelectTemplate (const wxString& newTemplate, const wxString& newTemplateVersion)
{
    if ((newTemplate != m_CdlConfig->get_template().c_str()) || (newTemplateVersion != m_templateVersion)){

        wxBusyCursor wait; // this may take a little while
        DeleteItems();

        m_templateVersion = wxT("");
        try
        {
            m_CdlConfig->set_template (newTemplate.c_str(), newTemplateVersion.c_str(), CdlParseErrorHandler, CdlParseWarningHandler);
            m_templateVersion = newTemplateVersion;
        }
        catch (CdlStringException exception)
        {
            wxString msg;
            msg.Printf(wxT("Error loading package template '%s':\n\n%s"), (const wxChar*) newTemplate.c_str (), (const wxChar*) exception.get_message ().c_str ());
            wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK);
        }
        catch (...)
        {
            wxString msg;
            msg.Printf(wxT("Error loading package template '%s'."), (const wxChar*) newTemplate.c_str ());
            wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK);
        }
        RegenerateData();

        if (!GetFilename().IsEmpty())
        { // not a new document
#if 0 // TODO
            CopyMLTFiles (); // copy new MLT files to the build tree as necessary
#endif
        }
        Modify(TRUE);
        wxGetApp().GetMainFrame()->UpdateFrameTitle();
    }
}

void ecConfigToolDoc::RegenerateData()
{
    wxBusyCursor wait; // This may take a little while
    AddAllItems (); // regenerate all the config items since the topology may have changed
    if (m_strLinkerScriptFolder.IsEmpty ())
    {
        wxMessageBox(_("The eCos linker script macro CYGBLD_LINKER_SCRIPT is not defined."), wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK);
    }
    if (m_strMemoryLayoutFolder.IsEmpty ())
    {
        wxMessageBox(_("The eCos memory layout macro CYGHWR_MEMORY_LAYOUT is not defined."), wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK);
    }
    // TODO
#if 0
    SwitchMemoryLayout (TRUE); // the hardware template may have changed
#endif

    if (GetDocumentSaved() && !wxGetApp().GetSettings().m_editSaveFileOnly)
        UpdateBuildInfo();
    
    // TODO
    // CConfigTool::GetControlView()->SelectItem(Item(0));
}

void ecConfigToolDoc::SelectHardware (const wxString& newTemplate)
{
    const std::string OldTemplate=m_CdlConfig->get_hardware();
    if (newTemplate != OldTemplate.c_str()){
        DeleteItems();

        try
        {
            m_CdlConfig->set_hardware (newTemplate.c_str(), CdlParseErrorHandler, CdlParseWarningHandler);
        }
        catch (CdlStringException exception)
        {
            wxString msg;
            msg.Printf(_("Error loading hardware template '%s':\n\n%s"), (const wxChar*) newTemplate, (const wxChar*) exception.get_message ().c_str ());
            wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK);
            m_CdlConfig->set_hardware (OldTemplate, CdlParseErrorHandler, CdlParseWarningHandler);
        }
        catch (...)
        {
            wxString msg;
            msg.Printf(_("Error loading hardware template '%s'."), (const wxChar*) newTemplate);
            wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK);
            m_CdlConfig->set_hardware (OldTemplate, CdlParseErrorHandler, CdlParseWarningHandler);
        }

        RegenerateData();

        // TODO
#if 0
        if (!GetFilename().IsEmpty())
        {
            CopyMLTFiles (); // copy new MLT files to the build tree as necessary
        }
#endif

        Modify(TRUE);
        wxGetApp().GetMainFrame()->UpdateFrameTitle();
    }
}

void ecConfigToolDoc::SelectPackages ()
{
    // Crashes the Cygwin 1.0 compiler
#ifndef __CYGWIN10__
    ecPackagesDialog dlg(wxGetApp().GetTopWindow());

    // This map holds the ecConfigItem pointers for the packages loaded before the dialog is invoked.
    // We cannot use Find(), which traverses all items - potentially those that have been removed
    wxHashTable arLoadedPackages(wxKEY_STRING);

    wxBeginBusyCursor();

    // generate the contents of the add/remove list boxes
    const std::vector<std::string> & packages = m_CdlPkgData->get_packages ();
    std::vector<std::string>::const_iterator package_i;
    for (package_i = packages.begin (); package_i != packages.end (); package_i++)
    {
        //		if (! m_CdlPkgData->is_hardware_package (* package_i)) // do not list hardware packages
        {
            const std::vector<std::string> & aliases = m_CdlPkgData->get_package_aliases (* package_i);
            wxString strMacroName = package_i->c_str ();

            // use the first alias (if any) as the package identifier
            wxString strPackageName = aliases.size () ? aliases [0].c_str () : strMacroName.c_str();
            ecConfigItem * pItem = Find (strMacroName);
            if (pItem) // if the package is loaded
            {
                arLoadedPackages.Put(strMacroName, pItem);
                // pass the currently selected package version string to the dialog box
                const CdlValuable valuable = pItem->GetCdlValuable();
                dlg.Insert (strPackageName, TRUE, wxEmptyString, valuable ? wxString (valuable->get_value ().c_str ()) : wxString());
            }
            else
            {
                // pass version string of the most latest version to the dialog box
                dlg.Insert (strPackageName, FALSE, wxEmptyString, wxString (m_CdlPkgData->get_package_versions (* package_i) [0].c_str ()));
            }
        }
    }

    wxEndBusyCursor();

    if (wxID_OK == dlg.ShowModal ())
    {
        bool bChanged = FALSE; // until proved otherwise

        // determine whether each package has changed loaded/unloaded state
        for (package_i = packages.begin (); package_i != packages.end (); package_i++)
            //			if (! m_CdlPkgData->is_hardware_package (* package_i)) // do not check hardware packages
        {
            const std::vector<std::string> & aliases = m_CdlPkgData->get_package_aliases (* package_i);
            wxString strMacroName = package_i->c_str ();

            // use the first alias (if any) as the package identifier
            wxString strPackageName = aliases.size () ? aliases [0].c_str () : strMacroName.c_str();

            ecConfigItem *pItem = (ecConfigItem *) arLoadedPackages.Get(strMacroName);
            //bool bPreviouslyLoaded=arLoadedPackages.Lookup(strMacroName,(void *&)pItem);
            bool bPreviouslyLoaded = (pItem != NULL);
            bool bNowLoaded=dlg.IsAdded (strPackageName);

            // unload packages which are no longer required before
            // loading new ones to avoid potential duplicate macro definitions
            if (! bNowLoaded && bPreviouslyLoaded){
                // The package was loaded but should now be unloaded:
                bChanged|=pItem->Unload();
            } else if (bNowLoaded) {// if the package should be loaded
                const wxString strVersion(dlg.GetVersion (strPackageName));
                if (bPreviouslyLoaded) { // if the package is already loaded
                    CdlTransactionCallback::set_callback_fn (NULL); // avoid value refresh attempts during load/unload
                    bChanged|=pItem->ChangeVersion(strVersion);
                    CdlTransactionCallback::set_callback_fn (CdlTransactionHandler); // restore value refresh
                } else {
                    // the package was not loaded but should now be loaded
                    //TRACE (_T("Loading package %s\n"), strMacroName);
                    try
                    {
                        GetCdlConfig()->load_package (ecUtils::UnicodeToStdStr(strMacroName), ecUtils::UnicodeToStdStr (strVersion), ecConfigToolDoc::CdlParseErrorHandler, ecConfigToolDoc::CdlParseWarningHandler);
                        bChanged=true;
                    }
                    catch (CdlStringException exception)
                    {
                        wxString msg;
                        msg.Printf(_("Error loading package %s:\n\n%s"), (const wxChar*) strMacroName, (const wxChar*) exception.get_message ().c_str ());
                        wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK);
                    }
                    catch (...)
                    {
                        wxString msg;
                        msg.Printf(_("Error loading package %s"), (const wxChar*) strMacroName);
                        wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK);
                    }
                }
            }
        }

        if (bChanged) {// at least one package was loaded, unloaded or changed version
            Modify(TRUE);
            wxGetApp().GetMainFrame()->UpdateFrameTitle();
            RegenerateData();
        }
    }
#endif
}


bool ecConfigToolDoc::UpdateBuildInfo(bool WXUNUSED(bFirstTime))
{
    try {
        GetCdlConfig()->get_build_info(m_BuildInfo);
        generate_build_tree (GetCdlConfig(), ecUtils::UnicodeToStdStr(GetBuildTree()), ecUtils::UnicodeToStdStr(GetInstallTree()));
        return TRUE;
    }
    catch(...){
        return FALSE;
    }
}

wxString ecConfigToolDoc::GetDefaultHardware ()
{
#ifdef __WXMSW__
    // get the greatest eCos version subkey
    wxConfig config(wxT("eCos"), wxEmptyString, wxEmptyString, wxEmptyString, wxCONFIG_USE_GLOBAL_FILE);

    wxString versionKey(wxT(""));
    wxString key(wxT(""));
    long index;
    bool bMore = config.GetFirstGroup(key, index);
    while (bMore)
    {
        if (wxIsdigit(key.GetChar(0)) && versionKey.CompareTo(key) < 0)
            versionKey = key;

        bMore = config.GetNextGroup(key, index);
    }

    if (!versionKey.IsEmpty())
    {
        wxString defaultHardware;
        wxString defaultHardwareKey(versionKey + wxString(wxT("/")) + wxString(wxT("Default Hardware")));
        if (config.Read(defaultHardwareKey, & defaultHardware))
        {
            return defaultHardware;
        }
    }
    return wxEmptyString;
#else
    // For other platforms, simply rely on Cdl to get the default hardware.
    return wxEmptyString;
#endif
}

void ecConfigToolDoc::EnableCallbacks (bool bEnable/*=TRUE*/)
{
    CdlTransactionCallback::set_callback_fn(bEnable?&CdlTransactionHandler:0);
    CdlTransactionBody::set_inference_callback_fn(bEnable?&CdlInferenceHandler:0);
    CdlTransactionBody::set_inference_override(CdlValueSource_Invalid);
}

CdlInferenceCallbackResult ecConfigToolDoc::CdlGlobalInferenceHandler(CdlTransaction transaction)
{
    CdlInferenceCallbackResult rc=CdlInferenceCallbackResult_Continue;

    ecConfigToolDoc *pDoc = wxGetApp().GetConfigToolDoc();
    pDoc->m_ConflictsOutcome=NotDone;  // prepare for the case that there are no solutions

    const std::list<CdlConflict>& conflicts=pDoc->GetCdlConfig()->get_all_conflicts();
    ecResolveConflictsDialog dlg(wxGetApp().GetTopWindow(), conflicts, transaction, &pDoc->m_arConflictsOfInterest);

    rc = (wxID_OK == dlg.ShowModal()) ? CdlInferenceCallbackResult_Continue:CdlInferenceCallbackResult_Cancel;
    pDoc->m_ConflictsOutcome=(CdlInferenceCallbackResult_Continue==rc)?OK:Cancel;

    return rc;
}

CdlInferenceCallbackResult ecConfigToolDoc::CdlInferenceHandler (CdlTransaction transaction)
{
    CdlInferenceCallbackResult rc=CdlInferenceCallbackResult_Continue;

    ecConfigToolDoc *pDoc=wxGetApp().GetConfigToolDoc();
    const std::list<CdlConflict>&conflicts=transaction->get_new_conflicts();
    if ((wxGetApp().GetSettings().m_nRuleChecking & ecSettings::Immediate) && conflicts.size()>0)
    {
        if (wxGetApp().GetSettings().m_nRuleChecking & ecSettings::SuggestFixes)
        {
            std::list<CdlConflict> s_conflicts;
            for (std::list<CdlConflict>::const_iterator conf_i= conflicts.begin (); conf_i != conflicts.end (); conf_i++) { // for each conflict
                if((*conf_i)->has_known_solution()){
                    s_conflicts.push_back(*conf_i);
                }
            }
            if(s_conflicts.size()>0)
            {
                wxGetApp().LockValues();

                ecResolveConflictsDialog dlg(wxGetApp().GetTopWindow(), s_conflicts, transaction);
                int ret = dlg.ShowModal() ;

                wxGetApp().UnlockValues();

                return (wxID_OK == ret ? CdlInferenceCallbackResult_Continue:CdlInferenceCallbackResult_Cancel);
            }
        }

        wxGetApp().LockValues();

        wxString strMsg;
        if(1==conflicts.size()){
            strMsg=wxT("There is 1 unresolved conflict. Make the change anyway?");
        } else {
            strMsg.Printf(_("There are %d unresolved conflict%s. Make the change anyway?"), conflicts.size(), (const wxChar*) (1==conflicts.size()? wxT(""):wxT("s")) );
        }
        rc = (wxYES == wxMessageBox(strMsg, wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxYES_NO)) ? CdlInferenceCallbackResult_Continue:CdlInferenceCallbackResult_Cancel;

        wxGetApp().UnlockValues();

    }
    return rc;
}


// a CDL transaction handler to refresh the configuration tree
void ecConfigToolDoc::CdlTransactionHandler (const CdlTransactionCallback & data)
{
    static int nNesting=0;
    //TRACE(_T("Transaction handler: nesting level=%d\n"),nNesting++);
    ecConfigToolDoc *pDoc = wxGetApp().GetConfigToolDoc();

    std::vector<CdlValuable>::const_iterator val_i;
    std::vector<CdlNode>::const_iterator node_i;
    std::list<CdlConflict>::const_iterator conf_i;
    ecConfigToolView *pControlView = (ecConfigToolView*) pDoc->GetFirstView();

    for (val_i = data.value_changes.begin(); val_i != data.value_changes.end(); val_i++)
    {
        const wxString strName((*val_i)->get_name().c_str());
        //TRACE(_T("%s %s : value change\n"), wxString ((*val_i)->get_class_name().c_str()), strName);
        pControlView->Refresh(strName);
        if (strName==wxT("CYGHWR_MEMORY_LAYOUT")){               // the memory layout has changed...
            // TODO
            // pDoc->SwitchMemoryLayout (FALSE); // ...so display a new one
        }
    }
    for (node_i = data.active_changes.begin(); node_i != data.active_changes.end(); node_i++)
    {
        const wxString strName((*node_i)->get_name().c_str());
        //TRACE(_T("%s %s : this has become active or inactive\n"), CString ((*node_i)->get_class_name().c_str()),
        //    CString ((*node_i)->get_name().c_str()));
        if (! dynamic_cast<CdlInterface> (*node_i)){ // if not an interface node
            pControlView->Refresh(strName);
        }
    }
    for (val_i = data.legal_values_changes.begin(); val_i != data.legal_values_changes.end(); val_i++)
    {
        const wxString strName((*node_i)->get_class_name().c_str());
        //TRACE(_T("%s %s : the legal_values list has changed, a new widget may be needed.\n"),
        //    CString ((*val_i)->get_class_name().c_str()), strName);
    }

    for (val_i = data.value_source_changes.begin(); val_i != data.value_source_changes.end(); val_i++)
    {
        const wxString strName((*val_i)->get_name().c_str());
        CdlValueSource source = (*val_i)->get_source();
/*
        TRACE(_T("%s %s : the value source has changed to %s\n"),
            CString ((*val_i)->get_class_name().c_str()), strName,
            CString ((CdlValueSource_Default  == source) ? "default"  :
        (CdlValueSource_Inferred == source) ? "inferred" :
        (CdlValueSource_Wizard   == source) ? "wizard"   : "user"));
*/
        pControlView->Refresh (strName);
    }

    pDoc->UpdateFailingRuleCount();
    nNesting--;
}

bool ecConfigToolDoc::ShowURL(const wxString& strURL1)
{
    bool rc = TRUE;

    wxString strURL(strURL1);

/*
    if(!QualifyDocURL(strURL)){
        return FALSE; // error message already output
    }
*/

    switch (wxGetApp().GetSettings().m_eUseCustomBrowser)
    {
    case ecInternal:
        rc = ShowInternalHtmlHelp(strURL);
        break;
    case ecAssociatedExternal:
        {
            rc = ShowExternalHtmlHelp(strURL);
        }

        break;
    case ecCustomExternal:
        QualifyDocURL(strURL, FALSE);
        wxGetApp().Launch(strURL, wxGetApp().GetSettings().m_strBrowser);
        break;
    default:
        wxASSERT(FALSE);
    }
    return rc;
}

bool ecConfigToolDoc::ShowExternalHtmlHelp (const wxString& strURL)
{
#if defined(__WXMSW__) || defined(__WXGTK__)

    wxString url;

    wxString sep(wxFILE_SEP_PATH);
    wxString docDir(wxString(m_strRepository) + sep + wxString(wxT("doc")));
    if (wxDirExists(docDir + sep + wxT("html")))
        docDir += sep + wxT("html") ;

    if (strURL.Left(7) == wxT("http://") || strURL.Left(7) == wxT("file://"))
        url = strURL;
    else
    {
	if (wxIsAbsolutePath(strURL))
	    url = strURL;
	else
            url = docDir + sep + strURL;
    }

    wxFileType *ft = wxTheMimeTypesManager->GetFileTypeFromExtension(wxT("html"));
    if ( !ft )
    {
        wxLogError(_T("Impossible to determine the file type for extension html"));
        return FALSE;
    }

    wxString cmd;
    bool ok = ft->GetOpenCommand(&cmd,
                                 wxFileType::MessageParameters(url, _T("")));
    delete ft;

    if (!ok)
    {
        // TODO: some kind of configuration dialog here.
        wxMessageBox(_("Could not determine the command for running the browser."),
		   wxGetApp().GetSettings().GetAppName(), wxOK|wxICON_EXCLAMATION);
        return FALSE;
    }

    // Remove spurious file:// if added
#ifdef __WXMSW__
    if (strURL.Left(7) == wxT("http://"))
        cmd.Replace(wxT("file://"), wxT(""));
#endif

    ok = (wxExecute(cmd, FALSE) != 0);

    return ok;

    // Old code using MS HTML Help
#elif 0
    HWND hwndCaller = ::GetDesktopWindow();

    wxString helpFile(wxGetApp().GetHelpFile());
    bool rc = FALSE;
    const ecFileName strFile(HTMLHelpLinkFileName());
    if (wxFileExists(strFile))
        wxRemoveFile(strFile);

    wxTextFile f(strFile);
    if(!ecFileName(helpFile).Exists())
    {
        wxString msg;
        msg.Printf(_("Cannot display help - %s does not exist"), (const wxChar*) helpFile);
        wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK);
    } else if (!f.Create())
    {
        wxString msg;
        msg.Printf(_("Cannot display help - error creating %s"), (const wxChar*) strFile);
        wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK);
    } else
    {
        wxString str;
        str.Printf(_T("<meta HTTP-EQUIV=\"refresh\" CONTENT=\"0;URL=%s\">"), (const wxChar*) strURL);
        f.AddLine(str);
        f.Write();
        f.Close();
        if(0==HtmlHelp(hwndCaller, wxGetApp().GetHelpFile(), HH_DISPLAY_TOPIC, 0))
        {
            wxString msg;
            msg.Printf(_("Cannot display %s"), (const wxChar*) strURL);
            wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK);
        }
#if 0
        else
        {
            // JACS: I don't think we need to do this. Even on the MFC version, the notification
            // callback didn't do anything interesting.
#define ID_HHNOTIFICATION               55017

            // FIXME: Do this the first time only?
            HH_WINTYPE WinType;
            HWND wnd;
            HH_WINTYPE *pWinType=NULL;
            wxString s = wxGetApp().GetHelpFile() + wxT(">mainwin");
            wnd = HtmlHelp(hwndCaller, s, HH_GET_WIN_TYPE, (DWORD) &pWinType);
            WinType=*pWinType;
            WinType.hwndCaller=hwndCaller;
            WinType.fsWinProperties|=HHWIN_PROP_TRACKING;
            WinType.idNotify = ID_HHNOTIFICATION;
            wnd = HtmlHelp(hwndCaller, wxGetApp().GetHelpFile(), HH_SET_WIN_TYPE, (DWORD) &WinType);
            rc = TRUE;
        }
#endif
        //wxRemoveFile(strFile);
    }
    return rc;

#else
    wxMessageBox(_("Sorry, ShowHtmlHelp not yet implemented"), wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK);
    return FALSE;
#endif
}

bool ecConfigToolDoc::ShowInternalHtmlHelp (const wxString& strURL)
{
#if defined(__WXMSW__) || defined(__WXGTK__)

    wxString url(strURL);

    wxString sep(wxFILE_SEP_PATH);
    wxString docDir(wxString(m_strRepository) + sep + wxString(wxT("doc")));
    if (wxDirExists(docDir + sep + wxT("html")))
        docDir += sep + wxT("html") ;

    url = docDir + sep + ecHtmlIndexer::Redirect(docDir, url);

#if 0
    if (strURL.Left(7) == wxT("http://") || strURL.Left(7) == wxT("file://"))
        url = strURL;
    else
        url = docDir + sep + strURL;
#endif

    //url = url.Mid(docDir.Length() + 1);

    if (wxGetApp().HasHelpController())
    {
        return wxGetApp().GetHelpController().Display(url);
    }
    else
        return FALSE;

    // Old code using MS HTML Help
#elif 0
    HWND hwndCaller = ::GetDesktopWindow();

    wxString helpFile(wxGetApp().GetHelpFile());
    bool rc = FALSE;
    const ecFileName strFile(HTMLHelpLinkFileName());
    if (wxFileExists(strFile))
        wxRemoveFile(strFile);

    wxTextFile f(strFile);
    if(!ecFileName(helpFile).Exists())
    {
        wxString msg;
        msg.Printf(_("Cannot display help - %s does not exist"), (const wxChar*) helpFile);
        wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK);
    } else if (!f.Create())
    {
        wxString msg;
        msg.Printf(_("Cannot display help - error creating %s"), (const wxChar*) strFile);
        wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK);
    } else
    {
        wxString str;
        str.Printf(_T("<meta HTTP-EQUIV=\"refresh\" CONTENT=\"0;URL=%s\">"), (const wxChar*) strURL);
        f.AddLine(str);
        f.Write();
        f.Close();
        if(0==HtmlHelp(hwndCaller, wxGetApp().GetHelpFile(), HH_DISPLAY_TOPIC, 0))
        {
            wxString msg;
            msg.Printf(_("Cannot display %s"), (const wxChar*) strURL);
            wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK);
        }
    }
    return rc;

#else
    wxMessageBox(_("Sorry, ShowHtmlHelp not yet implemented"), wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK);
    return FALSE;
#endif
}

const wxString ecConfigToolDoc::HTMLHelpLinkFileName()
{
    return ecFileName(wxGetApp().GetHelpFile()).Head() + wxT("link2.htm");
}

bool ecConfigToolDoc::QualifyDocURL(wxString &strURL, bool prefix)
{
    if(-1==strURL.Find(wxT("://")))
    {
#ifdef __WXMSW__
        strURL.Replace(wxT("/"), wxT("\\"));
#endif
        wxString originalURL(strURL);

        if (! ecFileName (strURL).IsFile ())
        { // if not an absolute filepath
            strURL = GetDocBase () + ecFileName (originalURL); // prepend the doc directory path
        }
        if (!wxFileExists(strURL))
            strURL = GetDocBase() + ecFileName(wxT("html")) + ecFileName(originalURL);

        if (prefix)
            strURL = wxT("file://") + strURL;
    }

    if(0==strURL.Find(wxT("file://")))
    {
        ecFileName strFile(strURL.Right(strURL.Length()-7));
        int nIndex=strFile.Find(wxT('#'), TRUE);

        if ( -1 != nIndex )
        {
            strFile=strFile.Left(nIndex);
        }
#ifdef __WXMSW__
        strFile.Replace(wxT("/"), wxT("\\"));
#endif

        if(!strFile.Exists())
        {
            wxString msg;
            msg.Printf(_("Cannot locate the file %s"), (const wxChar*) strFile);
            wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxID_OK);
            return FALSE;
        }
    }
    return TRUE;
}

wxString ecConfigToolDoc::GetPackageName (const wxString & strAlias)
{
    const std::vector<std::string> & packages = m_CdlPkgData->get_packages ();
    std::vector<std::string>::const_iterator package_i;
    for (package_i = packages.begin (); package_i != packages.end (); package_i++)
    {
        const std::vector<std::string> & aliases = m_CdlPkgData->get_package_aliases (* package_i);
        wxString strPackageAlias = aliases [0].c_str ();
        if (aliases.size () && (strAlias == strPackageAlias))
            return package_i->c_str ();
    }
    return wxEmptyString;
}

const wxString ecConfigToolDoc::GetCurrentTargetPrefix()
{
    ecConfigItem *pItem = Find(wxT("CYGBLD_GLOBAL_COMMAND_PREFIX"));
    if (pItem)
        return pItem->StringValue();
    else
        return wxT("");
}

ecConfigToolDoc::GlobalConflictOutcome ecConfigToolDoc::ResolveGlobalConflicts(wxList *parConflictsOfInterest)
{
    m_ConflictsOutcome=NotDone;
    m_arConflictsOfInterest.Clear();
    if(parConflictsOfInterest)
    {
        wxNode* node = parConflictsOfInterest->First();
        while (node)
        {
            wxObject* obj = (wxObject*) node->Data();
            m_arConflictsOfInterest.Append(obj);
            node = node->Next();
        }
    }
    CdlInferenceCallback fn=CdlTransactionBody::get_inference_callback_fn();
    CdlTransactionBody::set_inference_callback_fn(CdlGlobalInferenceHandler);
    GetCdlInterpreter()->get_toplevel()->resolve_all_conflicts();
    CdlTransactionBody::set_inference_callback_fn(fn);
    if(NotDone==m_ConflictsOutcome){
        // No solutions were available, but we'll run the dialog anyway
        const std::list<CdlConflict>& conflicts=GetCdlConfig()->get_all_conflicts();
        ecResolveConflictsDialog dlg(wxGetApp().GetTopWindow(), conflicts, NULL, &m_arConflictsOfInterest);
        m_ConflictsOutcome = (wxID_OK == dlg.ShowModal())?OK:Cancel;
    }
    return m_ConflictsOutcome;
}

bool ecConfigToolDoc::CheckConflictsBeforeSave()
{
    if (GetCdlInterpreter()->get_toplevel()->get_all_conflicts().size()>0)
    {
        if(ecSettings::Deferred & wxGetApp().GetSettings().m_nRuleChecking)
        {
            if ((ecSettings::SuggestFixes & wxGetApp().GetSettings().m_nRuleChecking) &&
                (Cancel==ResolveGlobalConflicts()))
            {
                return FALSE;
            }
            int nConflicts = GetCdlInterpreter()->get_toplevel()->get_all_conflicts().size();
            switch (nConflicts)
            {
            case 0:
                break;
            case 1:
                {
                    wxString msg;
                    msg.Printf(_("There is 1 unresolved conflict. Save anyway?"));
                    if (wxNO == wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxYES_NO))
                    {
                        return FALSE;
                    }
                }
            default:
                {
                    wxString msg;
                    msg.Printf(_("There are %d unresolved conflicts. Save anyway?"), nConflicts);
                    if (wxNO == wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxYES_NO))
                    {
                        return FALSE;
                    }
                }
            }
        }
    }
    return TRUE;
}

void ecConfigToolDoc::UpdateFailingRuleCount()
{
    int nCount=0;
    if (GetCdlConfig ())
    {
        // if configuration information

        // calculate the number of conflicts
        nCount = GetCdlConfig ()->get_all_conflicts ().size ();
            //	        GetCdlConfig ()->get_structural_conflicts ().size () +    ignore for now

        // update the conflicts view
        if (wxGetApp().GetMainFrame() && wxGetApp().GetMainFrame()->GetConflictsWindow())
        {
            wxGetApp().GetMainFrame()->GetConflictsWindow()->FillRules ();
        }
    }

    if (wxGetApp().GetMainFrame())
    {
        wxGetApp().GetMainFrame()->SetFailRulePane(nCount);
    }
}

void ecConfigToolDoc::LogConflicts (const std::list<CdlConflict> & conflicts)
{
    std::list<CdlConflict>::const_iterator conf_i;
    for (conf_i = conflicts.begin (); conf_i != conflicts.end (); conf_i++) // for each conflict
    {
        wxString strExplain = (* conf_i)->get_explanation ().c_str (); // get the conflict explanation
        wxGetApp().Log (ecUtils::StripExtraWhitespace (strExplain)); // display the message in the output window
    }
}

bool ecConfigToolDoc::SetValue (ecConfigItem &ti, double dValue, CdlTransaction transaction/*=NULL*/)
{
    wxASSERT (ti.GetOptionType () == ecDouble);

    // test if the new double value is in range
    const CdlValuable valuable = ti.GetCdlValuable();
    CdlListValue list_value;
    CdlEvalContext context (NULL, ti.GetCdlItem (), ti.GetCdlItem ()->get_property (CdlPropertyId_LegalValues));
    valuable->get_legal_values ()->eval (context, list_value);
    if (! list_value.is_member (dValue))
    {
        if ( dValue == valuable->get_double_value(CdlValueSource_Current) )
            return FALSE;

        wxString msg;
        msg.Printf(_("%s is not a legal value for %s.\n\nDo you want to use this value anyway?"),
            (const wxChar*) ecUtils::DoubleToStr (dValue), (const wxChar*) ti.GetMacro ());

        if (wxNO == wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_QUESTION|wxYES_NO))
            return FALSE;
    }

    if (! ti.SetValue (dValue,transaction))
        return FALSE;

    Modify(TRUE);
    wxGetApp().GetMainFrame()->UpdateFrameTitle();

    // NB UpdateAllViews isn't in the MFC Configtool for double (though it is for the
    // other types of value). In theory this should lead to a display inconsistency in the MFC tool.
    ecConfigToolHint hint(& ti, ecValueChanged);
    UpdateAllViews (NULL, & hint);

    return TRUE;
}

bool ecConfigToolDoc::SetValue(ecConfigItem &ti, const wxString &strValue, CdlTransaction transaction/*=NULL*/)
{
    // TODO
#if 0
    // warn the user if a modified memory layout is about to be discarded
    if (MemoryMap.map_modified () && (ti.Macro () == _T("CYG_HAL_STARTUP")) &&
        (IDCANCEL == CUtils::MessageBoxFT (MB_OKCANCEL, _T("Changes to the current memory layout will be lost."))))
        return false;
#endif

    bool rc = FALSE;

    switch(ti.GetOptionType())
    {
    case ecOptionTypeNone:
        break;
    case ecEnumerated:
    case ecString:
        rc = ti.SetValue(strValue, transaction);
        break;
    case ecLong:
        {
            long n;
            rc = ecUtils::StrToItemIntegerType(strValue,n) && SetValue(ti,n,transaction);
        }
        break;
    case ecDouble:
        {
            double dValue;
            rc = ecUtils::StrToDouble (strValue, dValue) && SetValue (ti, dValue,transaction);
        }
        break;
    default:
        wxASSERT(FALSE);
        break;

    }
    if(rc){
        Modify(TRUE);
        wxGetApp().GetMainFrame()->UpdateFrameTitle();

        ecConfigToolHint hint(& ti, ecValueChanged);
        UpdateAllViews (NULL, & hint);
    }
    return rc;
}

bool ecConfigToolDoc::SetValue(ecConfigItem &ti, long nValue, CdlTransaction transaction/*=NULL*/)
{
    switch(ti.GetOptionType())
    {
    case ecEnumerated:
    case ecLong:
        break;
    case ecOptionTypeNone:
    case ecString:
    default:
        wxASSERT(FALSE);
        break;
    }

    bool rc = FALSE;

    // TODO
#if 0
    bool bChangingMemmap = MemoryMap.map_modified () && ((ti.Macro ().Compare (_T ("CYG_HAL_STARTUP")) == 0));
#endif

    if(nValue==ti.Value())
    {
        return TRUE;
    }

    // test if the new integer value is in range
    if (ecLong == ti.GetOptionType ())
    {
        const CdlValuable valuable = ti.GetCdlValuable();
        CdlListValue list_value;
        CdlEvalContext context (NULL, ti.GetCdlItem (), ti.GetCdlItem ()->get_property (CdlPropertyId_LegalValues));
        valuable->get_legal_values ()->eval (context, list_value);
        if (! list_value.is_member ((cdl_int) nValue))
        {
            if (nValue == (long) valuable->get_integer_value (CdlValueSource_Current))
                goto Exit;

            wxString msg;
            msg.Printf(_("%s is not a legal value for %s.\n\nDo you want to use this value anyway?"),
                (const wxChar*) ecUtils::IntToStr (nValue, wxGetApp().GetSettings().m_bHex), (const wxChar*) ti.GetMacro ());
            if (wxNO == wxMessageBox (msg, wxGetApp().GetSettings().GetAppName(), wxICON_QUESTION|wxYES_NO))
                goto Exit;
        };
    }

    // TODO
#if 0
    // warn the user if the current memory layout has been changed and will be lost
    // this will happen when the layout has been modified and the target-platform-startup is changed

    if (bChangingMemmap && IDCANCEL==CUtils::MessageBoxFT(MB_OKCANCEL,_T("Changes to the current memory layout will be lost."))){
        goto Exit;
    }
#endif

    // Save state
    if(!ti.SetValue(nValue,transaction)){
        // CanSetValue above should have caught this
        wxString msg;
        msg.Printf(_("Cannot set '%s' to %d"), (const wxChar*) ti.GetItemNameOrMacro(), nValue);
        wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK);
        goto Exit;
    }

    rc = TRUE;
Exit:
    if(rc)
    {
        Modify(TRUE);
        wxGetApp().GetMainFrame()->UpdateFrameTitle();

        UpdateFailingRuleCount ();

        ecConfigToolHint hint(& ti, ecValueChanged);
        UpdateAllViews (NULL, & hint);
    }
    return rc;
}

bool ecConfigToolDoc::SetEnabled(ecConfigItem &ti, bool bEnabled, CdlTransaction transaction/*=NULL*/)
{
    const bool bStatus = ti.SetEnabled (bEnabled, transaction);

    if (bStatus)
    {
        Modify(TRUE);
        ecConfigToolHint hint(& ti, ecValueChanged);
        UpdateAllViews (NULL, & hint);
    }
    return bStatus;
}

const ecFileName ecConfigToolDoc::CurrentLinkerScript()
{
    const ecConfigItem * pItem = Find (wxT("CYGBLD_LINKER_SCRIPT"));
    return pItem ? ecFileName (m_strPackagesDir, m_strLinkerScriptFolder, pItem->StringValue ()) : ecFileName(wxT(""));
}


bool ecConfigToolDoc::GenerateHeaders()
{
    wxString sep(wxFILE_SEP_PATH);

    // Generate headers
    try {
        ecFileName strPkfConfDir(GetInstallTree());
        strPkfConfDir += ecFileName(wxT("include"));
        strPkfConfDir += ecFileName(wxT("pkgconf"));
        if ( !strPkfConfDir.CreateDirectory())
        {
            wxString msg;
            msg.Printf(_("Failed to create %s"), (const wxChar*) strPkfConfDir);
            wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION);
            return FALSE;
        }
        GetCdlConfig()->generate_config_headers(ecUtils::UnicodeToStdStr(strPkfConfDir.ShortName()));
    }
    catch (CdlInputOutputException e) {
        const wxString strMsg(e.get_message().c_str());
        // TRACE(_T("!!! Exception thrown calling generate_config_headers - %s"),strMsg);
        wxString msg;
        msg.Printf(_("Failed to generate header files - %s"), (const wxChar*) strMsg);
        wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION);
        return FALSE;
    }
    return TRUE;
}

const ecFileName ecConfigToolDoc::MLTDir ()
{
    wxString strPathName (GetFilename ());
    wxASSERT (! strPathName.IsEmpty ());
    return strPathName.Left (strPathName.Find (wxT('.'), TRUE)) + wxT("_mlt");
}

int ecConfigToolDoc::GetTestExeNames (wxArrayString& arTestExes, wxArrayString& arMissing)
{
    arTestExes.Clear();
    arMissing.Clear();
    typedef std::vector<CdlBuildInfo_Loadable> EntriesArray;
    const EntriesArray &arEntries=GetBuildInfo().entries;
    for(EntriesArray::size_type j=0;j<arEntries.size();j++)
    {
        const CdlBuildInfo_Loadable &e=arEntries[j];
        wxArrayString ar;
        int n = ecUtils::Chop(wxString(get_tests(GetCdlConfig(),e).c_str()),ar);
        int i;
        for (i=0;i<n;i++)
        {
            wxString strFile;
            strFile += GetInstallTree();
            strFile += wxFILE_SEP_PATH;
            strFile += wxT("tests");
            strFile += wxFILE_SEP_PATH;
            strFile += e.directory.c_str();
            strFile += wxFILE_SEP_PATH;
            strFile += ar[i];

            // Some tests accidentally specify .c
//            wxStripExtension(strFile);

#ifdef __WXMSW__
//            strFile += wxT(".exe");
            strFile.Replace(wxT("/"),wxT("\\"));
#endif

            if (wxFileExists(strFile))
            {
                arTestExes.Add(strFile);
            } else
            {
                arMissing.Add(strFile);
            }
        }
    }
    return arTestExes.Count();
}

bool ecConfigToolDoc::SaveMemoryMap()
{
    // TODO
#if 0
    wxString sep(wxFILE_SEP_PATH);

    const wxString strSuffix(wxT("mlt_") + CurrentMemoryLayout ());
    ecFileName strMLTInstallPkgconfDir(GetInstallTree());
    strMLTInstallPkgconfDir = strMLTInstallPkgconfDir + ecFileName(wxT("include"));
    strMLTInstallPkgconfDir = strMLTInstallPkgconfDir + ecFileName(wxT("pkgconf"));

    bool rc=false;
    if(strMLTInstallPkgconfDir.CreateDirectory(true)){
        const wxString strMLTInstallBase(strMLTInstallPkgconfDir+ecFileName(strSuffix));
        const ecFileName strMLTDir (MLTDir());

        if(strMLTDir.CreateDirectory (TRUE))
        {
            const wxString strMLTBase (strMLTDir + ecFileName (strSuffix));
            // TRACE(_T("Saving memory layout to %s\n"), strMLTBase + _T(".mlt"));
            if(MemoryMap.save_memory_layout (strMLTBase + _T(".mlt"))){
                // TRACE(_T("Exporting memory layout to %s\n"), strMLTInstallPkgconfDir);
                rc=MemoryMap.export_files (strMLTInstallBase + _T(".ldi"), strMLTInstallBase + _T(".h"));
            }
        }
    }
    return rc;
#else
    return FALSE;
#endif
}

bool ecConfigToolDoc::CopyMLTFiles()
{
    wxString sep(wxFILE_SEP_PATH);

    // copy default MLT files for the selected target/platform from the repository if they do not already exist

    // TRACE (_T("Looking for MLT files at %s\n"), PackagesDir() + m_strMemoryLayoutFolder + _T("include\\pkgconf\\mlt_*.*"));
    const ecFileName strInstallDestination(GetInstallTree () + sep + wxString(wxT("include")) + sep + wxT("pkgconf"));
    const ecFileName strMLTDestination (MLTDir ());
    // TRACE (_T("Copying .ldi and .h files to %s\n"), strInstallDestination);
    // TRACE (_T("Copying .mlt files to %s\n"), strMLTDestination);
    bool rc=strInstallDestination.CreateDirectory ( TRUE ) && strMLTDestination.CreateDirectory ( TRUE );
    if (rc)
    {
        wxDir ffFileFind;
        wxString fileName;
        wxString path = GetPackagesDir();
        path += sep;
        path += m_strMemoryLayoutFolder;
        path += sep;
        path += wxString(wxT("include"));
        path += sep;
        path += wxString(wxT("pkgconf"));

        if (!ffFileFind.Open(path))
            return FALSE;

        ecFileName wildcard = wxT("mlt_*.*");

        //bool bLastFile = ffFileFind.FindFile (PackagesDir() + m_strMemoryLayoutFolder + wxT("\\include\\pkgconf\\mlt_*.*"));
        bool bLastFile = ffFileFind.GetFirst (& fileName, wildcard);
        while (bLastFile)
        {
            wxString fullPath = path + sep + fileName;

            if (wxT(".mlt") == fileName.Right (4)) // if a .mlt file
            {
                if (! ecFileName (strMLTDestination, ecFileName (fileName)).Exists ())
                {
                    if (!wxCopyFile (fullPath, strMLTDestination + ecFileName (fileName)))
                    {
                        return FALSE; // message already emitted
                    }
                }
            }
            else // a .h or .ldi file
            {
                if (!ecFileName (strInstallDestination, ecFileName (fileName)).Exists () &&
                    !wxCopyFile (fullPath, strInstallDestination + ecFileName (fileName))){
                    return FALSE; // message already emitted
                }
            }
            bLastFile = ffFileFind.GetNext(& fileName);
        }
    }
    return rc; //FIXME
}

const wxString ecConfigToolDoc::CurrentMemoryLayout ()
{
    const ecConfigItem * pItem = Find (wxT("CYGHWR_MEMORY_LAYOUT"));

    wxString str;
    if (pItem)
        str = pItem->StringValue ();
    return str;
}

bool ecConfigToolDoc::ExportFile()
{
    wxFileDialog dialog(wxGetApp().GetTopWindow(), _("Export eCos Minimal Configuration"),
        wxT(""), wxT(""), wxT("eCos Minimal Configuration (*.ecm)|*.ecm"), wxSAVE|wxOVERWRITE_PROMPT|wxHIDE_READONLY);

    if (dialog.ShowModal() == wxID_OK)
    {
        try {
            m_CdlConfig->save (ecUtils::UnicodeToStdStr (dialog.GetPath()), /* minimal = */ true);
        }
        catch (CdlStringException exception)
        {
            wxString msg;
            wxString err(exception.get_message ().c_str ());
            msg.Printf(_("Error exporting eCos minimal configuration:\n\n%s"), (const wxChar*) err );
            wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK);
            return FALSE;
        }
        catch (...)
        {
            wxString msg;
            msg.Printf(_("Error exporting eCos minimal configuration"));
            wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK);
            return FALSE;
        }
    }
    return TRUE;
}

bool ecConfigToolDoc::ImportFile()
{
    wxFileDialog dialog(wxGetApp().GetTopWindow(), _("Import eCos Minimal Configuration"),
        wxT(""), wxT(""), wxT("eCos Minimal Configuration (*.ecm)|*.ecm"), wxOPEN|wxFILE_MUST_EXIST|wxHIDE_READONLY);

    if (dialog.ShowModal() == wxID_OK)
    {
        try {
            m_CdlConfig->add (ecUtils::UnicodeToStdStr (dialog.GetPath ()), ecConfigToolDoc::CdlParseErrorHandler, ecConfigToolDoc::CdlParseWarningHandler);
        }
        catch (CdlStringException exception)
        {
            wxString msg;
            wxString err(exception.get_message ().c_str ());
            msg.Printf(_("Error importing eCos minimal configuration:\n\n%s"), (const wxChar*) err );
            wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK);
            return FALSE;
        }
        catch (...)
        {
            wxString msg;
            msg.Printf(_("Error importing eCos minimal configuration"));
            wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK);
            return FALSE;
        }

        wxBusyCursor wait;

        AddAllItems (); // regenerate all the config items since the topology may have changed

        if (m_strLinkerScriptFolder.IsEmpty ())
        {
            wxMessageBox(_("The eCos linker script macro CYGBLD_LINKER_SCRIPT is not defined."), wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK);
        }
        if (m_strMemoryLayoutFolder.IsEmpty ())
        {
            wxMessageBox(_("The eCos memory layout macro CYGHWR_MEMORY_LAYOUT is not defined."), wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK);
        }
        SwitchMemoryLayout (TRUE); // the hardware template may have changed
        UpdateBuildInfo ();
        wxGetApp().GetTreeCtrl()->SelectItem (wxGetApp().GetTreeCtrl()->GetRootItem());
        Modify(TRUE);

    }
    return TRUE;
}

bool ecConfigToolDoc::SwitchMemoryLayout (bool bNewTargetPlatform)
{
    bool rc = TRUE;
    if (bNewTargetPlatform && ! m_strBuildTree.IsEmpty ()) // the user has changed target/platform within a build tree
    {
        // copy default MLT save files for the selected target/platform from the repository to the build tree if they do not already exist
        CopyMLTFiles();
    }

    if (m_strBuildTree.IsEmpty ()) // load the memory layout from the repository
    {
        wxString sep(wxFILE_SEP_PATH);
        wxString filename(m_strPackagesDir);
        filename += sep;
        filename += m_strMemoryLayoutFolder;
        filename += sep;
        filename += wxT("include");
        filename += sep;
        filename += wxT("pkgconf");

        rc = NewMemoryLayout (ecFileName (filename));
    }
    else // load the memory layout from the build tree
    {
        rc = NewMemoryLayout (MLTDir ());
    }

    return TRUE; // FIXME
}

bool ecConfigToolDoc::NewMemoryLayout (const wxString &strPrefix)
{
    // TODO
#if ecUSE_MLT
    ecFileName strFileName = CurrentLinkerScript ();
    wxString sep(wxFILE_SEP_PATH);

    m_memoryMap.new_memory_layout (); // delete the old memory layout regardless
    if (! strFileName.IsEmpty ())
        m_memoryMap.import_linker_defined_sections (strFileName); // read the linker-defined section names from the repository (failure is silent)

    wxString strMemoryLayoutFileName = strPrefix + sep + wxString(wxT("mlt_")) + CurrentMemoryLayout () + wxT(".mlt");

    m_memoryMap.load_memory_layout (strMemoryLayoutFileName); // load the new memory layout (failure is silent)
    m_strSelectedSection = wxT("");
    m_strSelectedRegion = wxT("");

    wxGetApp().GetMLTWindow()->RefreshMLT();

    // ecConfigToolHint hint(NULL, ecMemLayoutChanged);
    // UpdateAllViews (NULL, & hint);
#endif

    return TRUE; // FIXME
}

void ecConfigToolDoc::RunTests()
{
    wxString strTarget(GetCdlConfig()->get_hardware ().c_str ());
    wxArrayString ar;
    wxArrayString arTestsMissing;
    int nTests;
    wxGetApp().GetSettings().GetRunTestsSettings().m_strTarget = strTarget;

    {
        wxBusyCursor busy;

        GetCdlConfig()->get_build_info(m_BuildInfo);
        if (NULL==CeCosTestPlatform::Get(strTarget))
        {
            wxString msg;
            msg.Printf(_("%s is not a recognized platform - do you wish to add it?"), (const wxChar*) strTarget);
            if (wxNO == wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_QUESTION|wxYES_NO))
                return;
            
            ecPlatformEditorDialog dlg(wxGetApp().GetTopWindow());
            
            dlg.m_strPlatform = strTarget;
            dlg.m_strPrefix = GetCurrentTargetPrefix();
            dlg.m_strCaption=_("New Platform");
            if(wxID_CANCEL == dlg.ShowModal())
            {
                return;
            }
            CeCosTestPlatform::Add(CeCosTestPlatform(dlg.m_strPlatform,dlg.m_strPrefix,dlg.m_strPrompt,dlg.m_strGDB,dlg.m_bServerSideGdb,dlg.m_strInferior));
            CeCosTestPlatform::Save();
        }
        
        nTests = GetTestExeNames(ar, arTestsMissing);
    }

    const CeCosTestPlatform * etPlatform = CeCosTestPlatform::Get(strTarget);
    wxASSERT (NULL != etPlatform);

    if (-1 != wxString (etPlatform->GdbCmds ()).Find (wxT("cyg_test_is_simulator=1")))
    {   // if a simulator target, disable 'reset hardware' message box
        wxGetApp().GetSettings().GetRunTestsSettings().m_nReset = RESET_NONE ;
    }

    // TODO: I think the remote controls was something that wasn't quite implemented in the MFC tool.
    // It's in the properties dialog, but not used.
    // sheet.HideRemoteControls();
    if (arTestsMissing.Count() > 0)
    {
        wxString msg;
        msg.Printf(_("Not all tests are built. Do you wish to build them now?"));
        if (wxYES == wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_QUESTION|wxYES_NO))
        {
            wxGetApp().Build(wxT("tests"));
            return;
        }
    }
    wxString shellCommands;
    // Don't know why we passed TRUE (no build tools) but we need build tools for gdb
    if (wxGetApp().PrepareEnvironment(TRUE /* FALSE */, & shellCommands))
    {
        ecRunTestsDialog dialog(wxGetApp().GetTopWindow());
        int i;
        for ( i = 0 ; i < nTests; i++)
        {
            dialog.Populate(ar[i], TRUE);
        }
        for ( i = 0 ; i < arTestsMissing.Count(); i++)
        {
            dialog.Populate(arTestsMissing[i], FALSE);
        }

        dialog.ShowModal();
    }
}

// Rebuild the .hhc, .hhp, .hhk files and reinitialize the help controller
bool ecConfigToolDoc::RebuildHelpIndex(bool force)
{
    ecHtmlIndexer indexer(FALSE /* useRelativeURLs */ );

    wxString docDir = GetRepository() ;

    wxString projectFile;
    if (!indexer.IndexDocs(docDir, projectFile, force))
    {
        wxMessageBox("Sorry, could not index documentation.");
        return FALSE;
    }
    else
    {
        // Now we need to reset the help controller.
        wxGetApp().SetHelpFile(projectFile);
        wxGetApp().InitializeHelpController();
        return TRUE;
    }
}

