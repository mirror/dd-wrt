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
// configitem.cpp :
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   julians
// Contact(s):  julians
// Date:        2000/09/01
// Version:     $Id: configitem.cpp,v 1.10 2001/04/30 17:12:32 julians Exp $
// Purpose:
// Description: Implementation file for the ConfigTool application class
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
    #pragma implementation "configitem.h"
#endif

// Includes other headers for precompiled compilation
#include "ecpch.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/settings.h"
#include "wx/valgen.h"

#include "configitem.h"
#include "configtree.h"
#include "configtooldoc.h"
#include "configtoolview.h"
#include "ecutils.h"

IMPLEMENT_CLASS(ecConfigItem, wxObject)

/*
 * ecConfigItem
 * Represents a node in the configuration hierarchy.
 * For every ecConfigItem, there is also an ecTreeItemData
 * that points to it.
 */

ecConfigItem::ecConfigItem(ecConfigItem* parent, const wxString& name, ecConfigType ctype,
                           ecOptionFlavor flavor, ecOptionType otype,
                           bool active, bool enabled, ecUIHint hint)
{
    m_CdlItem = NULL;
    m_name = name;
    m_configType = ctype;
    m_optionType = otype;
    m_optionFlavor = flavor;
    m_enabled = enabled;
    m_active = active;
    m_parent = parent;
    m_hint = hint;
    m_treeItem = wxTreeItemId();

    switch (otype)
    {
    case ecDouble:
        {
            m_value = 0.0;
            break;
        }
    case ecString:
    case ecEnumerated:
        {
            m_value = wxT("");
            break;
        }
    case ecLong:
        {
            m_value = (long) 0;
            break;
        }
    case ecBool:
        {
            m_value = (bool) FALSE;
            break;
        }
    default:
        {
            break;
        }
    }
}

ecConfigItem::ecConfigItem(ecConfigItem* parent, CdlUserVisible vitem)
{
    m_name = wxT("UNNAMED");
    m_configType = ecConfigTypeNone;
    m_optionType = ecOptionTypeNone;
    m_optionFlavor = ecFlavorNone;
    m_enabled = FALSE;
    m_active = FALSE;
    m_parent = parent;
    m_CdlItem = vitem;
    m_hint = ecHintNone;
    m_treeItem = wxTreeItemId();

    ecConfigTreeCtrl* treeCtrl = wxGetApp().GetTreeCtrl();
    m_treeItem = treeCtrl->AppendItem(parent->GetTreeItem(), m_name, -1, -1, new ecTreeItemData(this));

    ConvertFromCdl();
    UpdateTreeItem(* treeCtrl);
}

ecConfigItem::~ecConfigItem()
{
    // Make sure that the tree item no longer references this object
    ecConfigTreeCtrl* treeCtrl = wxGetApp().GetTreeCtrl();
    if (m_treeItem && treeCtrl)
    {
        ecTreeItemData* data = (ecTreeItemData*) treeCtrl->GetItemData(m_treeItem);
        data->SetConfigItem(NULL);
    }

    ecConfigToolDoc* doc = wxGetApp().GetConfigToolDoc();
    if (doc)
    {
        doc->GetItems().DeleteObject(this);
    }
}

// Convert from Cdl to internal representation
bool ecConfigItem::ConvertFromCdl()
{
    if (!GetCdlItem())
        return FALSE;

    m_name = GetCdlItem()->get_display ().c_str ();
    m_macro = GetCdlItem()->get_name().c_str();
    m_strDescr = ecUtils::StripExtraWhitespace (wxString (GetCdlItem()->get_description ().c_str ()));


    // FIXME: re-implement using CdlValuableBody::get_widget_hint()
    // (comment from original MFC configtool)

    if (IsPackage())
    {
        // If a package item, display the package version string
        m_optionType = ecString; 
        m_configType = ecPackage;
        m_optionFlavor = ecFlavorNone;
    }
    else
    {
        const CdlValuable valuable = dynamic_cast<CdlValuable> (GetCdlItem());
        switch (valuable->get_flavor ()){
        case CdlValueFlavor_None:
            m_optionFlavor = ecFlavorNone;
            m_optionType=ecOptionTypeNone; //??? Shouldn't it be ecBool for CdlValueFlavor_Bool?
            m_configType = ecContainer;
            break;
        case CdlValueFlavor_Bool:
            m_optionFlavor = ecFlavorBool;
            m_optionType=ecOptionTypeNone; //??? Shouldn't it be ecBool for CdlValueFlavor_Bool?
            m_configType = ecOption;
            m_hint = (HasRadio() ? ecHintRadio : ecHintCheck);
            break;
        case CdlValueFlavor_Data:
        case CdlValueFlavor_BoolData:

            m_optionFlavor = (valuable->get_flavor() == CdlValueFlavor_Data ? ecFlavorData : ecFlavorBoolData);
            m_configType = ecOption;
            m_hint = (HasRadio() ? ecHintRadio : ecHintCheck);

            if (! valuable->has_legal_values ()) {
                m_optionType=ecString;
            } else if (0 == valuable->get_legal_values ()->ranges.size ()) {
                m_optionType=ecEnumerated;
            } else {
                CdlListValue list_value;
                CdlEvalContext context (NULL, valuable, valuable->get_property (CdlPropertyId_LegalValues));
                valuable->get_legal_values ()->eval (context, list_value);
                m_optionType=list_value.get_double_ranges ().size () ? ecDouble : ecLong;
            }
            break;
        default:
            wxASSERT (0); // specified flavor not supported
            break;
        }
    }  

    m_active = IsActive();
    m_enabled = IsEnabled();

    return TRUE;
}

wxString ecConfigItem::GetItemNameOrMacro() const
{
    return (wxGetApp().GetSettings().m_showMacroNames && !GetMacro().IsEmpty() ? GetMacro() : GetName());
}

// Sets the text and icon for this item
bool ecConfigItem::UpdateTreeItem(ecConfigTreeCtrl& treeCtrl)
{
    treeCtrl.SetItemText(m_treeItem, m_name);

    static wxColour normalColour = wxSystemSettings::GetSystemColour(wxSYS_COLOUR_WINDOWTEXT);
    static wxColour disabledColour = wxSystemSettings::GetSystemColour(wxSYS_COLOUR_GRAYTEXT);

    treeCtrl.SetItemTextColour(m_treeItem, GetActive() ? normalColour : disabledColour);
    

    // Find which icon state we're in so we can get the appropriate icon id
    int iconState = 0;
    wxString iconName;

    switch (GetConfigType())
    {
        case ecContainer:
            {
                iconName = _("Container");
                iconState = 0;
                break;
            }
        case ecPackage:
            {
                iconName = _("Package");
                iconState = 0;
                break;
            }
        case ecComponent:
            {
                iconName = _("??");
                iconState = 0;
                break;
            }
        case ecOption:
            {
                if (GetOptionFlavor() == ecFlavorData)
                {
                    switch (GetOptionType())
                    {
                    case ecDouble:
                    case ecLong:
                        {
                            iconName = _("Integer");
                            iconState = 0;
                            break;
                        }
                    case ecEnumerated:
                        {
                            iconName = _("Enumerated");
                            iconState = 0;
                            break;
                        }
                    case ecString:
                        {
                            iconName = _("Text");
                            iconState = 0;
                            break;
                        }
                    // ??? Actually I don't think there's such a think as ecBool type, only enabled/disabled
                    case ecBool:
                        {
                            if (GetUIHint() == ecHintCheck)
                                iconName = _("Checkbox");
                            else
                                iconName = _("Radiobox");
                            iconState = (m_value.GetBool() ? 0 : 1);
                            break;
                        }
                    default:
                        {
                            break;
                        }
                    }
                }
                if (GetOptionFlavor() == ecFlavorBoolData || GetOptionFlavor() == ecFlavorBool)
                {
                    if (GetUIHint() == ecHintCheck)
                        iconName = _("Checkbox");
                    else
                        iconName = _("Radiobox");
                    iconState = (m_enabled ? 0 : 1);
                }
                break;
            }
        default:
            {
                break;
            }
    }

    if (!iconName.IsEmpty())
    {
        int iconId = treeCtrl.GetIconDB().GetIconId(iconName, iconState, GetActive());
        treeCtrl.SetItemImage(m_treeItem, iconId, wxTreeItemIcon_Normal);
        treeCtrl.SetItemImage(m_treeItem, iconId, wxTreeItemIcon_Selected);
    }

    return TRUE;
}

// Handle a left click on the icon: e.g. (un)check the option
// In the old MFC tool, this was handled by CControlView::Bump
void ecConfigItem::OnIconLeftDown(ecConfigTreeCtrl& treeCtrl)
{
    if (GetConfigType() != ecOption)
        return;

    switch (GetOptionFlavor())
    {
    case ecFlavorBool:
    case ecFlavorBoolData:
        {
            if (GetActive())
            {
                wxGetApp().GetConfigToolDoc()->SetEnabled(*this, !m_enabled);
            }
            break;
        }
    case ecFlavorData:
        {
            if (GetActive())
            {
                switch (GetOptionType())
                {
                case ecLong:
                    {
                        int nInc = 1;

                        long nOldValue = Value();
                        if(nInc==1 && nOldValue == long(-1))
                        {
                            nOldValue=0;
                        } else if(nInc==-1 && nOldValue==0){
                            nOldValue = long(-1);
                        } else {
                            nOldValue+=nInc;
                        }
                        wxGetApp().GetConfigToolDoc()->SetValue(*this, nOldValue);
                        break;
                    }
                case ecEnumerated:
                    {
                        int nInc = 1;

                        wxArrayString arEnum;
                        EvalEnumStrings (arEnum); // calculate legal values just in time
                        if (0 == arEnum.GetCount()) // if no legal values...
                            break;           // ...do nothing
                        int nIndex = -1;
                        const wxString strCurrent = StringValue ();
                        int nEnum;
                        for (nEnum = 0; (nEnum < arEnum.GetCount()) && (nIndex == -1); nEnum++)
                            if (0 == arEnum[nEnum].CompareTo (strCurrent))
                                nIndex = nEnum; // the index of the current value
                            
                            if (nIndex != -1) // if the current value is still legal
                                nIndex += (nInc < 0 ? -1 : 1); // increment/decrement the index
                            else
                                nIndex = 0; // otherwise select the first enum
                            
                            if (nIndex < 0) // if the new index is negative
                                nIndex = arEnum.GetCount()-1; // make it positive

                            wxGetApp().GetConfigToolDoc()->SetValue(*this, arEnum[nIndex % arEnum.GetCount()]);
                            break;
                    }
                default:
                    {
                        break;
                    }
                }
            }
            break;   
        }
    default:
        {
            break;
        }
    }

}

// Gets the value to display (often an empty string)
wxString ecConfigItem::GetDisplayValue() const
{
    wxString str;
    switch(GetOptionType())
    {
    case ecEnumerated:
    case ecLong:
    case ecDouble:
    case ecString:
        {
            if (GetCdlValuable())
                str = StringValue();
        }
        break;
    default:
        break;
    }
    return str;
#if 0
    switch (GetConfigType())
    {
        case ecComponent:
        case ecContainer:
            {
                return wxEmptyString;
                break;
            }
        case ecPackage:
            {
                return m_value.GetString();
                break;
            }
        case ecOption:
            {
                switch (GetOptionType())
                {
                    case ecDouble:
                        {
                            wxString val;
                            val.Printf("%.4lf", (double) m_value.GetDouble());
                            return val;
                        }
                    case ecLong:
                        {
                            wxString val;
                            val.Printf("%.ld", (long) m_value.GetLong());
                            return val;
                            break;
                        }
                    case ecEnumerated:
                    case ecString:
                        {
                            return m_value.GetString();
                            break;
                        }
                    case ecBool:
                        {
                            return wxEmptyString;
                            break;
                        }
                    default:
                        {
                            break;
                        }
                }
                break;
            }
        default:
            {
                break;
            }
    }

    return wxEmptyString;
#endif
}

// Can we start editing this item?
bool ecConfigItem::CanEdit() const
{
    if (!GetActive())
        return FALSE;

    if (GetConfigType() != ecOption)
        return FALSE;

    if (GetOptionFlavor() != ecFlavorData && GetOptionFlavor() != ecFlavorBoolData)
        return FALSE;

    // TODO: other criteria for editability
    return TRUE;
}


// Creates an edit window. It will be positioned by the caller.
wxWindow* ecConfigItem::CreateEditWindow(wxWindow* parent)
{
    wxWindow* window = NULL;

    switch(GetOptionType())
    {
    case ecEnumerated:
        {
            window = new ecEnumEditorCtrl(parent, ecID_ITEM_EDIT_WINDOW, wxDefaultPosition, wxDefaultSize,
                /* wxNO_BORDER */ 0);
            wxArrayString arEnumStrings;
            EvalEnumStrings(arEnumStrings);
            int i;
            for (i = 0; i < arEnumStrings.GetCount(); i++)
            {
                ((ecEnumEditorCtrl*) window)->Append(arEnumStrings[i]);
            }
            break;
        }
    case ecLong:
        {
            window = new ecIntegerEditorCtrl(parent, ecID_ITEM_EDIT_WINDOW, wxDefaultPosition, wxDefaultSize,
                /* wxNO_BORDER | */ wxSP_ARROW_KEYS);
            break;
        }
    case ecDouble:
        {
            window = new ecDoubleEditorCtrl(parent, ecID_ITEM_EDIT_WINDOW, wxDefaultPosition, wxDefaultSize,
                /* wxNO_BORDER|*/ wxTE_PROCESS_ENTER);
            break;
        }
    case ecString:
        {
            window = new ecTextEditorCtrl(parent, ecID_ITEM_EDIT_WINDOW, wxDefaultPosition, wxDefaultSize,
                /* wxNO_BORDER|*/ wxTE_PROCESS_ENTER);
            break;
        }
    default:
        break;
    }

    wxASSERT (window != NULL) ;
    
    return window;
}
    
// Transfers data between item and window
bool ecConfigItem::TransferDataToWindow(wxWindow* window)
{
    if (window->IsKindOf(CLASSINFO(ecTextEditorCtrl)))
    {
        ecTextEditorCtrl* win = (ecTextEditorCtrl*) window;
        win->SetValue(GetDisplayValue());
    }
    else if (window->IsKindOf(CLASSINFO(ecDoubleEditorCtrl)))
    {
        ecDoubleEditorCtrl* win = (ecDoubleEditorCtrl*) window;
        win->SetValue(GetDisplayValue());
    }
    else if (window->IsKindOf(CLASSINFO(ecEnumEditorCtrl)))
    {
        ecEnumEditorCtrl* win = (ecEnumEditorCtrl*) window;
        win->SetStringSelection(GetDisplayValue());
    }
    else if (window->IsKindOf(CLASSINFO(ecIntegerEditorCtrl)))
    {
        ecIntegerEditorCtrl* win = (ecIntegerEditorCtrl*) window;
        long i;
        ecUtils::StrToItemIntegerType(StringValue(), i);

        wxString val;
        val.Printf(wxT("%ld"), i);

        win->SetValue(val);
    }
    return TRUE;
}

bool ecConfigItem::TransferDataFromWindow(wxWindow* window)
{
    ecConfigToolDoc* doc = wxGetApp().GetConfigToolDoc();
    wxASSERT (doc != NULL);

    if (!doc)
        return FALSE;

    if (window->IsKindOf(CLASSINFO(ecTextEditorCtrl)))
    {
        ecTextEditorCtrl* win = (ecTextEditorCtrl*) window;

        wxASSERT ( GetOptionType() == ecString );

        // TODO: do checking
        doc->SetValue(*this, win->GetValue());
    }
    else if (window->IsKindOf(CLASSINFO(ecDoubleEditorCtrl)))
    {
        ecDoubleEditorCtrl* win = (ecDoubleEditorCtrl*) window;

        wxASSERT ( GetOptionType() == ecString );

        // TODO: do checking
        doc->SetValue(*this, atof(win->GetValue()));
    }
    else if (window->IsKindOf(CLASSINFO(ecEnumEditorCtrl)))
    {
        ecEnumEditorCtrl* win = (ecEnumEditorCtrl*) window;

        wxASSERT ( GetOptionType() == ecEnumerated );

        // TODO: do checking
        doc->SetValue(*this, win->GetStringSelection());
    }
    else if (window->IsKindOf(CLASSINFO(ecIntegerEditorCtrl)))
    {
        ecIntegerEditorCtrl* win = (ecIntegerEditorCtrl*) window;

        wxASSERT ( GetOptionType() == ecLong );

        // TODO: do checking
        doc->SetValue(*this, (long) win->GetValue());
    }

    return TRUE;
}

//// Taken from MFC version

const ecFileName ecConfigItem::GetFilename() const
{
    wxString sep(wxFILE_SEP_PATH);

    ecFileName strFile;
    const CdlNode node = dynamic_cast<CdlNode> (m_CdlItem);
    if (node){
        // get the package which owns the configuration item
        const CdlPackage package = GetOwnerPackage();
        if (package){
            
            // return the filename of the config header
            wxString pkg(wxT("include"));
            pkg += sep;
            pkg += wxT("pkgconf");
            strFile=ecFileName(wxGetApp().GetConfigToolDoc()->GetInstallTree()+sep+pkg) + package->get_config_header ().c_str ();
        }
    }
    return strFile;
}

// Change version (of a package)
bool ecConfigItem::ChangeVersion(const wxString &strVersion)
{
    bool rc=FALSE;
    CdlPackage package=dynamic_cast<CdlPackage>(GetCdlItem());
    wxASSERT(package != 0);
    const CdlValuable valuable = GetCdlValuable();
    wxASSERT (valuable != 0);
    const wxString strMacroName(GetMacro());
    if (strVersion != valuable->get_value ().c_str ()) { // if the wrong version is loaded
        // TRACE (wxT("Changing package %s to version '%s'\n"), strMacroName, strVersion);
        try {
            wxGetApp().GetConfigToolDoc()->GetCdlConfig()->change_package_version (package, ecUtils::UnicodeToStdStr (strVersion), ecConfigToolDoc::CdlParseErrorHandler, ecConfigToolDoc::CdlParseWarningHandler);
            rc=TRUE;
        }
        catch (CdlStringException exception) {
            wxString msg;
            msg.Printf(wxT("Error changing package %s to version '%s':\n\n%s"), (const wxChar*) strMacroName, (const wxChar*) strVersion, (const wxChar*) wxString (exception.get_message ().c_str ())) ;
            wxMessageBox(msg);
        }
        catch (...) {
            wxString msg;
            msg.Printf(wxT("Error changing package %s to version '%s'"), (const wxChar*) strMacroName, (const wxChar*) strVersion) ;
            wxMessageBox(msg);
        }
    }
    return rc;
}

// Unload (a package)
bool ecConfigItem::Unload()
{
    bool rc=FALSE;
    CdlPackage package=dynamic_cast<CdlPackage>(GetCdlItem());
    wxASSERT(package);
    ecConfigToolDoc* pDoc=wxGetApp().GetConfigToolDoc();

    // Remove its objects from the view to prevent any painting problems
    ecTreeItemData* data = (ecTreeItemData*) wxGetApp().GetTreeCtrl()->GetItemData(GetTreeItem());
    wxASSERT(data);

    // I _think_ we should do this to stop 'this' from being deleted when we delete the item.
    // But, in that case, where do we delete this item?
    // Perhaps should store them in an array in the document, as per the MFC tool.
    data->SetConfigItem(NULL);

    wxGetApp().GetTreeCtrl()->Delete(GetTreeItem());

    wxNode* node = pDoc->GetItems().First();
    while (node)
    {
        ecConfigItem* item = wxDynamicCast(node->Data(), ecConfigItem);
        if (package == item->GetOwnerPackage())
        {
            item->SetTreeItem(wxTreeItemId()); // Make sure we can't attempt to paint it
            item->SetCdlItem(NULL); // Make sure we can't access stale data
        }
        node = node->Next();
    }

    const wxString strMacroName(GetMacro());
    //TRACE (wxT("Unloading package %s\n"), strMacroName);
    try {
        pDoc->GetCdlConfig()->unload_package (package);
        rc=TRUE;
    }
    catch (CdlStringException exception) {
        wxString msg;
        wxString exceptionMsg(exception.get_message ().c_str ());
        msg.Printf(wxT("Error unloading package %s:\n\n%s"), (const wxChar*) strMacroName, (const wxChar*) exceptionMsg );
        wxMessageBox(msg);
    }
    catch (...) {
        wxString msg;
        msg.Printf(wxT("Error unloading package %s"), (const wxChar*) strMacroName);
        wxMessageBox(msg);
    }
    m_treeItem=wxTreeItemId();   // Make sure we can't attempt to paint it
    m_CdlItem=NULL; // Make sure we can't access stale data
    return rc;
}

wxString ecConfigItem::GetURL() const
{
    for(const ecConfigItem *pItem=this;pItem;pItem=pItem->GetParent()){
        if(pItem->GetCdlItem()){
            wxString strURL;
            strURL=pItem->GetCdlItem()->get_doc_url().c_str();
            if(strURL.Len()){
                return strURL;
            }
            strURL=pItem->GetCdlItem()->get_doc().c_str();
            if(strURL.Len()){
                return strURL;
            }
        }
    }
    return wxT("ref/ecos-ref.html"); // the default URL
}

bool ecConfigItem::SetValue(const wxString& value, CdlTransaction transaction/*=NULL*/)
{
    wxASSERT ((m_optionType == ecString) || (m_optionType == ecEnumerated));
    const CdlValuable valuable = GetCdlValuable();
    wxASSERT (valuable);
    const std::string str = value.c_str();
    if(transaction){
        if (CdlValueFlavor_BoolData == valuable->get_flavor ()){
            // set the user bool to the current bool when changing a booldata
            // value to avoid a possible change in the current bool
            valuable->set_enabled_and_value (transaction, valuable->is_enabled (), str, CdlValueSource_User);
        } else {// CdlValueFlavor_Data
            valuable->set_value (transaction, str, CdlValueSource_User);
        }
    } else {
        if (CdlValueFlavor_BoolData == valuable->get_flavor ()){
            // set the user bool to the current bool when changing a booldata
            // value to avoid a possible change in the current bool
            valuable->set_enabled_and_value (valuable->is_enabled (), str, CdlValueSource_User);
        } else {// CdlValueFlavor_Data
            valuable->set_value (str, CdlValueSource_User);
        }
    }

    // TODO: eliminate m_value, since the value is always taken from the Cdl object.
    m_value = value;
    
    return TRUE;
}

bool ecConfigItem::SetValue (double dValue, CdlTransaction transaction/*=NULL*/)
{
    wxASSERT (m_optionType == ecDouble);

    const CdlValuable valuable = GetCdlValuable();
    wxASSERT (valuable);
    
    if(transaction) {
        if (CdlValueFlavor_BoolData == valuable->get_flavor ()) {
            // set the user bool to the current bool when changing a booldata
            // value to avoid a possible change in the current bool
            valuable->set_enabled_and_value (transaction, valuable->is_enabled (), dValue, CdlValueSource_User);
        } else {// CdlValueFlavor_Data
            valuable->set_double_value (transaction, dValue, CdlValueSource_User);
        }
    } else {
        if (CdlValueFlavor_BoolData == valuable->get_flavor ()) {
            // set the user bool to the current bool when changing a booldata
            // value to avoid a possible change in the current bool
            valuable->set_enabled_and_value (valuable->is_enabled (), dValue, CdlValueSource_User);
        } else {// CdlValueFlavor_Data
            valuable->set_double_value (dValue, CdlValueSource_User);
        }
    }

    // TODO: BoolData?
    m_value = dValue;
    
    return TRUE;
}

bool ecConfigItem::SetValue (long nValue, CdlTransaction transaction/*=NULL*/)
{
    wxASSERT (m_optionType == ecLong);
    const CdlValuable valuable = GetCdlValuable();
    wxASSERT (valuable);
    
    if(transaction) {
        if (CdlValueFlavor_BoolData == valuable->get_flavor ()) {
            // set the user bool to the current bool when changing a booldata
            // value to avoid a possible change in the current bool
            valuable->set_enabled_and_value (transaction, valuable->is_enabled (), (cdl_int) nValue, CdlValueSource_User);
        } else { // CdlValueFlavor_Data
            valuable->set_integer_value (transaction, nValue, CdlValueSource_User);
        }
    } else {
        if (CdlValueFlavor_BoolData == valuable->get_flavor ()) {
            // set the user bool to the current bool when changing a booldata
            // value to avoid a possible change in the current bool
            valuable->set_enabled_and_value (valuable->is_enabled (), (cdl_int) nValue, CdlValueSource_User);
        } else { // CdlValueFlavor_Data
            valuable->set_integer_value (nValue, CdlValueSource_User);
        }
    }

    // TODO: BoolData?
    m_value = nValue;
    
    return TRUE;
}

bool ecConfigItem::HasRadio() const
{
    const CdlValuable valuable = GetCdlValuable();
    if (! valuable)
        return FALSE;
    
    CdlWidgetHint hint;
    valuable->get_widget_hint (hint);
    return (CdlBoolWidget_Radio == hint.bool_widget);
}

ecConfigItem *ecConfigItem::FirstRadio() const
{
    wxASSERT(HasRadio ());
    
    for(ecConfigItem *h=GetParent()->FirstChild();h;h=h->NextSibling()){
        if(h->HasRadio ()){
            return h;
        }
    }
    // No radio buttons found
    wxASSERT(FALSE);
    return FALSE;
}

ecConfigItem *ecConfigItem::FirstChild() const
{ 
    ecConfigTreeCtrl* treeCtrl = wxGetApp().GetTreeCtrl();

    long cookie;
    wxTreeItemId hChild=treeCtrl->GetFirstChild(GetTreeItem(), cookie);
    if (hChild)
    {
        ecTreeItemData* data = (ecTreeItemData*) wxGetApp().GetTreeCtrl()->GetItemData(hChild);
        wxASSERT(data);

        return data->GetConfigItem();
    }
    else
        return NULL;
}

ecConfigItem *ecConfigItem::NextSibling() const
{ 
    ecConfigTreeCtrl* treeCtrl = wxGetApp().GetTreeCtrl();

    wxTreeItemId hChild=treeCtrl->GetNextSibling(GetTreeItem());
    if (hChild)
    {
        ecTreeItemData* data = (ecTreeItemData*) wxGetApp().GetTreeCtrl()->GetItemData(hChild);
        wxASSERT(data);

        return data->GetConfigItem();
    }
    else
        return NULL;
}

bool ecConfigItem::IsEnabled() const
{
    const CdlValuable valuable = GetCdlValuable();
    return NULL==valuable ||valuable->is_enabled();
}

bool ecConfigItem::IsActive() const
{
//    return GetCdlItem()->is_active();
    const CdlValuable valuable = GetCdlValuable();
    if (valuable && ((GetOptionType() != ecOptionTypeNone) || HasBool()))
    {
        return (valuable->is_modifiable () && valuable->is_active ());
    }
    else
        return GetCdlItem()->is_active();
}

bool ecConfigItem::HasModifiedChildren() const
{
    for(ecConfigItem *pItem=FirstChild();pItem;pItem=pItem->NextSibling()){
        if(pItem->Modified()||pItem->HasModifiedChildren()){
            return TRUE;
        }
    }
    return FALSE;
}

bool ecConfigItem::Modified () const
{
    const CdlValuable valuable = GetCdlValuable();
    return 
        valuable        // accommodate the root config item which has no CDL item
        && !IsPackage() // packages are never modified
        && valuable->get_source () != CdlValueSource_Default;
}

void ecConfigItem::DumpItem()
{
    //TRACE(wxT("Item %08x\n\tDisplay Name='%s'\n\tMacro Name='%s'\n\tType=%s"), this,	Name(),           Macro(),    TreeItemTypeImage[m_Type]);
    //TRACE(wxT("\n\tValue=%s\n\tURL=%s\n\tParent=%08x"),StringValue(), GetURL(), Parent());
    
    //TRACE(wxT("\n"));
}

ecConfigItem * ecConfigItem::NextRadio() const
{
    wxASSERT(this->HasRadio ());
    for(ecConfigItem *pItem=NextSibling();pItem;pItem=pItem->NextSibling()){
        if(pItem->HasRadio()){
            return pItem;
        }
    }
    return NULL;
}

bool ecConfigItem::IsDescendantOf(ecConfigItem * pAncestor)
{
    for(ecConfigItem *pItem=GetParent();pItem;pItem=pItem->GetParent()){
        if(pItem==pAncestor){
            return TRUE;
        }
    }
    return FALSE;
}

bool ecConfigItem::ViewHeader()
{
    bool rc=FALSE;
    const ecFileName strFile(GetFilename());
    if(!strFile.IsEmpty())
    {
        ecConfigToolDoc *pDoc=wxGetApp().GetConfigToolDoc();
        if(pDoc->GetBuildTree().IsEmpty()){
            wxString msg;
            msg.Printf(wxT("Cannot display header file until configuration is saved"));
            wxMessageBox(msg);
        } else
        {
            rc=wxGetApp().Launch(strFile, wxGetApp().GetSettings().m_strViewer);
        }
    }
    return rc;
}

bool ecConfigItem::ViewURL()
{
    return wxGetApp().GetConfigToolDoc()->ShowURL(GetURL());
}

bool ecConfigItem::HasBool() const
{
    if (!m_CdlItem) {
        return FALSE;
    } else if (IsPackage()) {
        return FALSE;
    } else {
        const CdlValuable valuable = GetCdlValuable();
        CdlValueFlavor flavor = valuable->get_flavor ();
        return (flavor == CdlValueFlavor_Bool) || (flavor == CdlValueFlavor_BoolData);
    }
}

bool ecConfigItem::SetEnabled(bool bEnabled, CdlTransaction current_transaction/*=NULL*/)
{
    const CdlValuable valuable = GetCdlValuable();
    wxASSERT (valuable);
    
    // use a transaction object to ensure that all config items are changed together
    CdlTransaction transaction = current_transaction ? current_transaction : CdlTransactionBody::make (wxGetApp().GetConfigToolDoc ()->GetCdlConfig ());
    
    if (HasRadio () && bEnabled) { // if a new radio button has been selected
        for (ecConfigItem *pItem = FirstRadio(); pItem; pItem = pItem->NextRadio ()) { // for each radio button in the group
            if (pItem != this) { // if not the newly selected radio button
                pItem->SetEnabled (FALSE, transaction); // disable the radio button
            }
        }
    }
    
    if (CdlValueFlavor_BoolData == valuable->get_flavor ()) {
        // set the user value to the current data value when enabling/disabling
        // a booldata item to avoid a possible change in the current data value
        CdlSimpleValue simple_value = valuable->get_simple_value ();
        valuable->set_enabled_and_value (transaction, bEnabled, simple_value, CdlValueSource_User);
    } else { // CdlValueFlavor_Bool
        valuable->set_enabled (transaction, bEnabled, CdlValueSource_User);
    }
    
    if (! current_transaction) { // if not a recursive call to disable a radio button
        transaction->body (); // commit the transaction
        delete transaction;
        transaction = NULL;
    }
    
    return TRUE;
}

long ecConfigItem::DefaultValue () const
{
    return (long) atoi (StringValue (CdlValueSource_Default)) ;
}

long ecConfigItem::Value () const
{
    wxASSERT (!IsPackage()); // not a package item
    const CdlValuable valuable = GetCdlValuable();
    wxASSERT (valuable);
    long nValue (0);
    
    switch (valuable->get_flavor ())
    {
        //	case CdlValueFlavor_Bool:
        //		nValue = valuable->is_enabled (CdlValueSource_Current) ? 1 : 0;
        //		break;
        
    case CdlValueFlavor_BoolData:
    case CdlValueFlavor_Data:
        nValue = (long) valuable->get_integer_value (CdlValueSource_Current);
        break;
        
    default:
        wxASSERT (0); // specified flavor not supported
    }
    
    return nValue;
}

const double ecConfigItem::DoubleValue (CdlValueSource source /* = CdlValueSource_Current */ ) const
{
    wxASSERT (!IsPackage()); // not a package item
    const CdlValuable valuable = GetCdlValuable();
    wxASSERT (valuable);
    wxASSERT (valuable->has_double_value (source));
    return valuable->get_double_value (source);
}

const wxString ecConfigItem::StringValue (CdlValueSource source /* = CdlValueSource_Current */ ) const
{
    //	wxASSERT (!IsPackage()); // not a package item
    const CdlValuable valuable = GetCdlValuable();
    wxASSERT (valuable);
    wxString strValue (wxT(""));
    
    switch (valuable->get_flavor ())
    {
    case CdlValueFlavor_Data:
    case CdlValueFlavor_BoolData:
    case CdlValueFlavor_None: // a package
        if (m_optionType == ecLong)
            strValue = ecUtils::IntToStr (Value (), wxGetApp().GetSettings().m_bHex);
        else if (m_optionType == ecDouble)
            strValue = ecUtils::DoubleToStr (DoubleValue ());
        else
            strValue = valuable->get_value (source).c_str ();
        break;
        
    default:
        wxASSERT (0); // specified flavor not supported
    }
    
    return strValue;
}

const wxString ecConfigItem::StringValue(ecWhereType where) const
{
    wxString str;
    switch(where){
    case ecInName:
        str=GetName();
        break;
    case ecInMacro:
        str=GetMacro();
        break;
    case ecInDesc:
        str=GetDescription();
        break;
    case ecInCurrentValue:
        if (ecOptionTypeNone==GetOptionType())
	    str = wxEmptyString;
        else
            str = StringValue(CdlValueSource_Current);
        break;
    case ecInDefaultValue:
        if (ecOptionTypeNone==GetOptionType())
	    str = wxEmptyString;
        else
            str = StringValue(CdlValueSource_Default);
        break;
    default:
        wxASSERT(FALSE);
        break;
    }
    return str;
}    

int ecConfigItem::EvalEnumStrings (wxArrayString &arEnumStrings) const
{
    const CdlValuable valuable = GetCdlValuable();
    wxASSERT (valuable);
    /*
    if (m_Type == Boolean)
    {
    arEnumStrings.SetSize (2);
    arEnumStrings.SetAt (0, wxT("True"));
    arEnumStrings.SetAt (1, wxT("False"));
    }
    else
    */
    {
        wxASSERT (m_optionType == ecEnumerated);
        CdlListValue list_value;
        CdlEvalContext context (NULL, m_CdlItem, m_CdlItem->get_property (CdlPropertyId_LegalValues));
        valuable->get_legal_values ()->eval (context, list_value);
        const std::vector<CdlSimpleValue> & table = list_value.get_table ();
        
        // add legal values to the list
        for (unsigned int nValue = 0; nValue < table.size (); nValue++)
        {
            arEnumStrings.Add (table [nValue].get_value ().c_str ());
        }
    }
    return arEnumStrings.GetCount();
}

static const wxChar* gs_whereTypes[] = 
{
        _("Macro names"), 
        _("Item names"), 
        _("Short descriptions"), 
        _("Current Values"), 
        _("Default Values")
};

// Convert a string representation of 'where' (e.g. "Macro names") to
// ecWhereType
ecWhereType ecConfigItem::WhereStringToType(const wxString& whereString)
{
    int sz = 5;
    int i;
    for (i = 0; i < sz; i++)
        if (whereString == gs_whereTypes[i])
            return (ecWhereType) i;

    wxASSERT( FALSE );

    return (ecWhereType) 0;
}

// Convert a type representation of 'where' to a string
wxString ecConfigItem::WhereTypeToString(ecWhereType whereType)
{
    return gs_whereTypes[(size_t) whereType] ;
}

// Bump by specified amount, or toggle if a boolean value
bool ecConfigItem::BumpItem(int nInc)
{
    bool rc = FALSE;
    
    // Take an action for clicking on the icon
    ecConfigToolDoc* pDoc = wxGetApp().GetConfigToolDoc();
    
    // do not modify the option value if it is inactive or not modifiable
    const CdlValuable valuable = GetCdlValuable();
    if (!valuable || (valuable->is_modifiable () && valuable->is_active ()))
    {
        if (0 == nInc) // if a toggle request
        {
            if (HasBool () && ! (HasRadio () && IsEnabled ())) { // only enable (not disable) a radio button
                rc = pDoc->SetEnabled (*this, ! this->IsEnabled ()); // toggle enabled/disabled state
            }
        } else if (IsEnabled ()) { // the item is enabled...
            switch(GetOptionType())
            {
            case ecOptionTypeNone:
            case ecString:
            case ecDouble:
                break;
            case ecEnumerated:
                {
                    wxArrayString arEnum;
                    EvalEnumStrings (arEnum); // calculate legal values just in time
                    if (0==arEnum.Count()) // if no legal values...
                        break;           // ...do nothing
                    int nIndex = -1;
                    const wxString strCurrent = StringValue ();
                    int nEnum;
                    for (nEnum = 0; (nEnum < arEnum.Count()) && (nIndex == -1); nEnum++)
                        if (strCurrent == arEnum[nEnum])
                            nIndex = nEnum; // the index of the current value
                        
                        if (nIndex != -1) // if the current value is still legal
                            nIndex += (nInc < 0 ? -1 : 1); // increment/decrement the index
                        else
                            nIndex = 0; // otherwise select the first enum
                        
                        if (nIndex < 0) // if the new index is negative
                            nIndex = arEnum.Count()-1; // make it positive
                        
                        rc=pDoc->SetValue (*this, arEnum[nIndex % arEnum.Count()]);
                }
                break;
            case ecLong:
                {
                    // TODO: if we're editing, we should get the value in the edit item
                    // and not the ecConfigItem.
                    long nOldValue = Value();
                    if(nInc==1 && nOldValue==-1){
                        nOldValue=0;
                    } else if(nInc==-1 && nOldValue==0){
                        nOldValue=-1;
                    } else {
                        nOldValue+=nInc;
                    }
                    rc=pDoc->SetValue(*this, nOldValue);
                    break;
                }
                
                break;
                /*
                case CConfigItem::Boolean:
                
                  {
                  ItemIntegerType nOldValue=Value(h);
                  pDoc->SetValue(ti,nOldValue^1);
                  }
                  break;
                  case CConfigItem::Radio:
                  
                    if(0==Value(h)){
                    pDoc->SetValue(ti, (ItemIntegerType) 1);
                    }
                    break;
                */
            default:
                break;
            }
        }
    }
    return rc;
}

#if 0

/* Presumably we don't need this since we use the m_parent member instead
ecConfigItem *ecConfigItem::Parent() const 
{ 
    CTreeCtrl &tree=CConfigTool::GetControlView()->GetTreeCtrl();
    HTREEITEM hParent=tree.GetParentItem(HItem());
    return (NULL==hParent||TVI_ROOT==hParent)?NULL:(ecConfigItem *)tree.GetItemData(hParent);
}
*/

#endif

/*
 * ecTextEditorCtrl
 * A specialised wxTextCtrl, for editing config values
 */

BEGIN_EVENT_TABLE(ecTextEditorCtrl, wxTextCtrl)
    EVT_TEXT_ENTER(-1, ecTextEditorCtrl::OnEnter)
    EVT_KILL_FOCUS(ecTextEditorCtrl::OnKillFocus)
    EVT_LEFT_DCLICK(ecTextEditorCtrl::OnLeftDClick)
END_EVENT_TABLE()

IMPLEMENT_CLASS(ecTextEditorCtrl, wxTextCtrl)

ecTextEditorCtrl::ecTextEditorCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size,
                                   long style):
    wxTextCtrl(parent, id, wxEmptyString, pos, size, style)
{
}

void ecTextEditorCtrl::OnEnter(wxCommandEvent& event)
{
    ecValueWindow* parent = (ecValueWindow*) GetParent();
    parent->EndEditing();
}

void ecTextEditorCtrl::OnKillFocus(wxFocusEvent& event)
{
    ecValueWindow* parent = (ecValueWindow*) GetParent();
    parent->EndEditing();
}

// Edit the string in a separate dialog, for convenience
void ecTextEditorCtrl::OnLeftDClick(wxMouseEvent& event)
{
    ecValueWindow* parent = (ecValueWindow*) GetParent();
    ecConfigItem* item = parent->GetCurrentConfigItem();
    ecConfigToolDoc* doc = wxGetApp().GetConfigToolDoc();
    
    wxString initialValue(GetValue());
    
    ecEditStringDialog dialog(initialValue, wxGetApp().GetTopWindow(), ecID_EDIT_STRING_DIALOG);
    if (dialog.ShowModal() == wxID_OK)
    {
        wxString val = dialog.GetValue() ;
        // This control will have been deleted at this point, due to losing the focus.
        // So update the item, not the control.
        // wxTextCtrl::SetValue(val);
        doc->SetValue(*item, val);
    }   
}

/*
 * ecDoubleEditorCtrl
 * A specialised wxTextCtrl, for editing double config values
 */

BEGIN_EVENT_TABLE(ecDoubleEditorCtrl, wxTextCtrl)
    EVT_TEXT_ENTER(-1, ecDoubleEditorCtrl::OnEnter)
    EVT_KILL_FOCUS(ecDoubleEditorCtrl::OnKillFocus)
END_EVENT_TABLE()

IMPLEMENT_CLASS(ecDoubleEditorCtrl, wxTextCtrl)

ecDoubleEditorCtrl::ecDoubleEditorCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size,
                                   long style):
    wxTextCtrl(parent, id, wxEmptyString, pos, size, style)
{
}

void ecDoubleEditorCtrl::OnEnter(wxCommandEvent& event)
{
    ecValueWindow* parent = (ecValueWindow*) GetParent();
    parent->EndEditing();
}

void ecDoubleEditorCtrl::OnKillFocus(wxFocusEvent& event)
{
    ecValueWindow* parent = (ecValueWindow*) GetParent();
    parent->EndEditing();
}

/*
 * ecIntegerEditorCtrl
 * A specialised wxTextCtrl, for editing double config values
 */

BEGIN_EVENT_TABLE(ecIntegerEditorCtrl, wxSpinCtrl)
    EVT_TEXT_ENTER(-1, ecIntegerEditorCtrl::OnEnter)
    EVT_KILL_FOCUS(ecIntegerEditorCtrl::OnKillFocus)
END_EVENT_TABLE()

IMPLEMENT_CLASS(ecIntegerEditorCtrl, wxSpinCtrl)

ecIntegerEditorCtrl::ecIntegerEditorCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size,
                                   long style):
    wxSpinCtrl(parent, id, wxEmptyString, pos, size, style, -32000, 32000, 0)
{
}

void ecIntegerEditorCtrl::OnEnter(wxCommandEvent& event)
{
    ecValueWindow* parent = (ecValueWindow*) GetParent();
    parent->EndEditing();
}

void ecIntegerEditorCtrl::OnKillFocus(wxFocusEvent& event)
{
    ecValueWindow* parent = (ecValueWindow*) GetParent();
    parent->EndEditing();
}

/*
 * ecEnumEditorCtrl
 * A specialised wxChoice, for editing enumerated config values
 */

BEGIN_EVENT_TABLE(ecEnumEditorCtrl, wxChoice)
    EVT_CHAR(ecEnumEditorCtrl::OnChar)
    EVT_KILL_FOCUS(ecEnumEditorCtrl::OnKillFocus)
END_EVENT_TABLE()

IMPLEMENT_CLASS(ecEnumEditorCtrl, wxChoice)

ecEnumEditorCtrl::ecEnumEditorCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size,
                                   long style):
    wxChoice(parent, id, pos, size, 0, 0, style)
{
}

void ecEnumEditorCtrl::OnChar(wxKeyEvent& event)
{
    if (event.GetKeyCode() == WXK_RETURN)
    {
        ecValueWindow* parent = (ecValueWindow*) GetParent();
        parent->EndEditing();
    }
    else
        event.Skip();
}

void ecEnumEditorCtrl::OnKillFocus(wxFocusEvent& event)
{
    ecValueWindow* parent = (ecValueWindow*) GetParent();
    parent->EndEditing();
}

/*
 * ecEditStringDialog
 * Pops up to make it easier to edit large string values
 */

BEGIN_EVENT_TABLE(ecEditStringDialog, ecDialog)
    EVT_BUTTON(wxID_OK, ecEditStringDialog::OnOK)
END_EVENT_TABLE()

ecEditStringDialog::ecEditStringDialog(const wxString& initialValue, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size,
        long style)
{
    m_value = initialValue;
    //SetExtraStyle(wxDIALOG_EX_CONTEXTHELP);

    ecDialog::Create(parent, id, _("String Edit"),
        wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER);

    CreateControls(this);
    
    TransferDataToWindow();

    Centre(wxBOTH);
}

ecEditStringDialog::~ecEditStringDialog()
{
}

//// Event handlers

void ecEditStringDialog::OnOK(wxCommandEvent& event)
{
    wxDialog::OnOK(event);
}

//// Operations
void ecEditStringDialog::CreateControls(wxWindow* parent)
{
    wxSizer *item0 = new wxBoxSizer( wxVERTICAL );

    wxSizer *item1 = new wxBoxSizer( wxHORIZONTAL );

    item1->Add( 20, 20, 10, wxALIGN_CENTRE|wxALL, 5 );

    wxButton *item2 = new wxButton( parent, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    item2->SetDefault();
    item1->Add( item2, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxButton *item3 = new wxButton( parent, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    item1->Add( item3, 0, wxALIGN_CENTRE|wxALL, 5 );

    item0->Add( item1, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxTOP, 5 );

    wxTextCtrl *item4 = new wxTextCtrl( parent, ecID_STRING_EDIT_TEXTCTRL, _(""), wxDefaultPosition, wxSize(420,250), wxTE_MULTILINE );
    item0->Add( item4, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );

    parent->SetAutoLayout( TRUE );
    parent->SetSizer( item0 );
    parent->Layout();
    item0->Fit( parent );
    item0->SetSizeHints( parent );

    FindWindow(ecID_STRING_EDIT_TEXTCTRL)->SetValidator(wxGenericValidator(& m_value));
    FindWindow(ecID_STRING_EDIT_TEXTCTRL)->SetFocus();
}
