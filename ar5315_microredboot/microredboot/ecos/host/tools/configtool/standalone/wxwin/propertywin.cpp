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
// propertywin.cpp :
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   julians
// Contact(s):  julians
// Date:        2000/09/04
// Version:     $Id: propertywin.cpp,v 1.7 2001/04/24 14:39:13 julians Exp $
// Purpose:
// Description: Implementation file for ecPropertyListCtrl
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
    #pragma implementation "propertywin.h"
#endif

// Includes other headers for precompiled compilation
#include "ecpch.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "propertywin.h"
#include "configtool.h"
#include "configitem.h"
#include "configtooldoc.h"
#include "ecutils.h"

// specify the CDL properties which are to be visible in the properties view
const std::string ecPropertyListCtrl::visible_properties [] =
{
	CdlPropertyId_ActiveIf,
	CdlPropertyId_BuildProc,
	CdlPropertyId_Calculated,
	CdlPropertyId_CancelProc,
	CdlPropertyId_CheckProc,
	CdlPropertyId_Compile,
	CdlPropertyId_ConfirmProc,
	CdlPropertyId_DecorationProc,
	CdlPropertyId_DefaultValue,
	CdlPropertyId_Define,
	CdlPropertyId_DefineHeader,
	CdlPropertyId_DefineProc,
//	CdlPropertyId_Description,
	CdlPropertyId_Dialog,
//	CdlPropertyId_Display,
	CdlPropertyId_DisplayProc,
	CdlPropertyId_Doc,
	CdlPropertyId_EntryProc,
	CdlPropertyId_Flavor,
	CdlPropertyId_DefineFormat,
	CdlPropertyId_Group,
	CdlPropertyId_Hardware,
	CdlPropertyId_IfDefine,
	CdlPropertyId_Implements,
	CdlPropertyId_IncludeDir,
	CdlPropertyId_IncludeFiles,
	CdlPropertyId_InitProc,
	CdlPropertyId_InstallProc,
	CdlPropertyId_LegalValues,
	CdlPropertyId_Library,
	CdlPropertyId_LicenseProc,
	CdlPropertyId_Make,
	CdlPropertyId_Makefile,
	CdlPropertyId_MakeObject,
	CdlPropertyId_NoDefine,
	CdlPropertyId_Object,
	CdlPropertyId_Parent,
	CdlPropertyId_Requires,
	CdlPropertyId_Screen,
	CdlPropertyId_Script,
	CdlPropertyId_UpdateProc,
	CdlPropertyId_Wizard
};

const wxChar* ecPropertyListCtrl::sm_fieldTypeImage[ecMAXFIELDTYPE]=
	{wxT("Type"), wxT("Value"), wxT("Default"), wxT("Macro"), wxT("File"), wxT("URL"), wxT("Enabled")};


/*
 * ecPropertyListCtrl
 */

IMPLEMENT_CLASS(ecPropertyListCtrl, wxListCtrl)

BEGIN_EVENT_TABLE(ecPropertyListCtrl, wxListCtrl)
    EVT_RIGHT_DOWN(ecPropertyListCtrl::OnRightClick)
    EVT_LEFT_DCLICK(ecPropertyListCtrl::OnDoubleClick)
END_EVENT_TABLE()

ecPropertyListCtrl::ecPropertyListCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pt,
        const wxSize& sz, long style):
        wxListCtrl(parent, id, pt, sz, style)
{
    if (!wxGetApp().GetSettings().GetWindowSettings().GetUseDefaults() &&
         wxGetApp().GetSettings().GetWindowSettings().GetFont(wxT("Properties")).Ok())
    {
        SetFont(wxGetApp().GetSettings().GetWindowSettings().GetFont(wxT("Properties")));
    }

    m_f[0]=0.25;
    m_f[1]=0.75;
    m_pti = NULL;
    m_nFirstProperty = 0;
    m_nOnSizeRecursionCount = 0;

    AddColumns();

#if 0
    int j;
    int i = 0;
    for (j = 0; j < 4; j++)
    {
        
        // Insert some dummy items
        
        wxListItem info;
        info.m_text = _("URL");
        info.m_mask = wxLIST_MASK_TEXT ; // | wxLIST_MASK_IMAGE | wxLIST_MASK_DATA;
        info.m_itemId = i;
        info.m_image = -1;
        //info.m_data = (long) doc;
        
        long item = InsertItem(info);
        
        SetItem(i, 1, _("redirects/interrupts.html"));   
        i ++;
        
        info.m_text = _("Enabled");
        info.m_mask = wxLIST_MASK_TEXT ; // | wxLIST_MASK_IMAGE | wxLIST_MASK_DATA;
        info.m_itemId = i;
        info.m_image = -1;
        //info.m_data = (long) doc;
        
        item = InsertItem(info);
        
        SetItem(i, 1, _("True"));
        i ++;
    }
#endif

}

void ecPropertyListCtrl::OnRightClick(wxMouseEvent& event)
{
    PopupMenu(wxGetApp().GetWhatsThisMenu(), event.GetX(), event.GetY());
}

void ecPropertyListCtrl::AddColumns()
{
    InsertColumn(0, "Property", wxLIST_FORMAT_LEFT, 100);
    InsertColumn(1, "Value", wxLIST_FORMAT_LEFT, 300);
}

void ecPropertyListCtrl::Fill(ecConfigItem *pti)
{
    if(NULL==pti){
        ClearAll();
        AddColumns();
        m_nFirstProperty=0;
        m_pti=NULL;
    } else /* if(pti!=m_pti) */ {
        m_pti=pti;
        m_nMaxValueWidth=0;
        ecOptionType type=m_pti->GetOptionType();
        int i;  
        
        // Initially flag all items as unnecessary - calls of SetItem or SetProperty will change this
        for(i=GetItemCount()-1;i>=0;--i){
            SetItemData(i,0);
        }
        if (m_pti->HasBool () || (ecOptionTypeNone!=type)){
            SetItem(ecMacro, m_pti->GetMacro ());
        }
        
        if (m_pti->HasBool ()){
            SetItem(ecEnabled, m_pti->IsEnabled() ? wxT("True") : wxT("False"));
        }
        
        if(!m_pti->GetFilename().IsEmpty()){
            SetItem(ecFile, m_pti->GetFilename());
        }
        SetItem(ecURL, m_pti->GetURL());
        
        if (ecOptionTypeNone!=type){

            switch(type){
            case ecString:
                SetItem(ecValue, m_pti->StringValue());
                SetItem(ecDefaultValue, m_pti->StringDefaultValue());
                break;
            case ecLong:
                SetItem(ecValue, ecUtils::IntToStr(m_pti->Value(), wxGetApp().GetSettings().m_bHex));
                SetItem(ecDefaultValue, ecUtils::IntToStr(m_pti->DefaultValue(), wxGetApp().GetSettings().m_bHex));
                break;
            case ecDouble:
                SetItem(ecValue, ecUtils::DoubleToStr(m_pti->DoubleValue()));
                SetItem(ecDefaultValue, ecUtils::DoubleToStr(m_pti->DoubleDefaultValue()));
                break;
            case ecEnumerated:
                SetItem(ecValue,m_pti->StringValue());
                SetItem(ecDefaultValue,m_pti->StringDefaultValue());
                break;
            default:
                wxASSERT( FALSE );
                break;
            }
            // TODO: set image
            // SetItem(ecType, ecConfigItem::TreeItemTypeImage[type]);
        }

        // List all the properties applicable to me
        const std::string name = ecUtils::UnicodeToStdStr (m_pti->GetMacro ());
        if (name.size () > 0)
        {
            const CdlConfiguration config = wxGetApp().GetConfigToolDoc ()->GetCdlConfig ();
            const CdlNode node = config->find_node (name, true);
            wxASSERT (node);
            const std::vector<CdlProperty> & properties = node->get_properties ();
            std::vector<CdlProperty>::const_iterator property_i;
            // CMapStringToPtr map; // count of each property name
            wxHashTable map(wxKEY_STRING);  // count of each property name

            for (property_i = properties.begin (); property_i != properties.end (); property_i++) {// for each property
                // get the property name
                const CdlProperty &prop=*property_i;
                const wxString strName(prop->get_property_name ().c_str());
                enum {VISIBLE_PROPERTIES_COUNT=sizeof visible_properties/sizeof visible_properties[0]};
                if (std::find (visible_properties, visible_properties + VISIBLE_PROPERTIES_COUNT, ecUtils::UnicodeToStdStr(strName)) != visible_properties + VISIBLE_PROPERTIES_COUNT) {// if the property should be displayed
                    // set the property arguments
                    wxString strPropertyArgs;
                    const std::vector<std::string> & argv = prop->get_argv ();
                    void *p;
                    p = (void*) map.Delete(strName);
                    
                    p=(void *)((int)p+1);
                    map.Put(strName, (wxObject*) p);
                    
                    std::vector<std::string>::const_iterator argv_i;
                    for (argv_i = argv.begin (); argv_i != argv.end (); argv_i++){ // for each property argument...
                        if (argv_i != argv.begin ()){                              // ...except the first (the property name)
                            wxString strArg(ecUtils::StripExtraWhitespace (wxString(argv_i->c_str())));
                            if (strPropertyArgs.Len() + strArg.Len() + 1 > 256) {// if the string is too long for the list control
                                break; // no need to add any further arguments
                            }
                            strPropertyArgs += strArg; // add the argument to the string
                            strPropertyArgs += wxT (" "); // separate arguments by a space character
                        }
                    }
                    // the list control appears to display a maximum of 256 characters
                    int nIndex=SetItem(strName, strPropertyArgs, GetItemCount(), (int)p);
                    SetItemData(nIndex, (long) prop);
                    
                    // display the exclamation icon if the property is in a conflicts list
                    bool bConflictItem =
                        //					PropertyInConflictsList (* property_i, config->get_structural_conflicts ()) || ignore for now
                        PropertyInConflictsList (prop, config->get_all_conflicts ());

                    // TODO: set the image for a conflict item
                    // CListCtrl::SetItem (nIndex, 0, LVIF_IMAGE, NULL, bConflictItem ? 1 : 0, 0, 0, 0 );
                }
            }
        }

        for(i=GetItemCount()-1;i>=0;--i){
            if(0==GetItemData(i)){
                DeleteItem(i);
                if(i<m_nFirstProperty){
                    m_nFirstProperty--;
                }
            }
        }
        // TODO
#if 0
        CRect rect;
        GetClientRect(rect);
        int nAvailWidth=rect.Width()-GetColumnWidth(0);
        int w=max(m_nMaxValueWidth,nAvailWidth);
        m_f[1]=double(w)/double(rect.Width());
        SetColumnWidth(1,w); 
#endif
    }

    Refresh();
}

bool ecPropertyListCtrl::PropertyInConflictsList (CdlProperty property, const std::list<CdlConflict> & conflicts)
{
    std::list<CdlConflict>::const_iterator conf_i;
    for (conf_i = conflicts.begin (); conf_i != conflicts.end (); conf_i++) // for each conflict
        if (property == (* conf_i)->get_property ())
            return true;
        
        return false;
}

void ecPropertyListCtrl::RefreshValue()
{
    if (!m_pti)
        return;

    if (m_pti->HasBool ()){
        SetItem(ecEnabled, m_pti->IsEnabled () ? wxT("True") : wxT("False"));
    }
    if (m_pti->GetOptionType () != ecOptionTypeNone){
        SetItem(ecValue, m_pti->StringValue());
    }
    for (int nItem = m_nFirstProperty; nItem < GetItemCount (); nItem++)
    {
        CdlProperty property = (CdlProperty) GetItemData (nItem);
        wxASSERT (property);
        
        // display the exclamation icon if the property is in a conflicts list
        const CdlConfiguration config = wxGetApp().GetConfigToolDoc ()->GetCdlConfig ();
        bool bConflictItem =
            //						PropertyInConflictsList (property, config->get_structural_conflicts ()) || ignore for now
            PropertyInConflictsList (property, config->get_all_conflicts ());
        
        // TODO
        // CListCtrl::SetItem (nItem, 0, LVIF_IMAGE, NULL, bConflictItem ? 1 : 0, 0, 0, 0 );
    }
    
}

int ecPropertyListCtrl::SetItem(ecFieldType f, const wxString& value)
{
    int nIndex=SetItem(sm_fieldTypeImage[f], value, m_nFirstProperty);
    if(nIndex==m_nFirstProperty){
        m_nFirstProperty++;
    }
    SetItemData(nIndex,1);
    return nIndex;
}

int ecPropertyListCtrl::SetItem(const wxString& item, const wxString& value, int nInsertAs, int nRepeat)
{
    wxASSERT( nInsertAs <= GetItemCount() );

/*
    LVFINDINFO info;
    info.flags =LVFI_STRING;
    info.psz   =pszItem;
    info.vkDirection=VK_DOWN;
    int nIndex=-1;
    do {
        nIndex=FindItem(&info,nIndex);
    } while (--nRepeat>0 && nIndex!=-1);
*/
    // NB: wxListCtrl doesn't support reverse search, so could do it explicitly
    // by iterating through the items.
    // But for now, just ignore the nRepeat flag and find the first one.
    int nIndex = -1;
    nIndex = FindItem(0, /* nIndex */ item);
    
    if(-1==nIndex){
        nIndex = InsertItem(nInsertAs, item);
    }
    
    wxListCtrl::SetItem(nIndex, 1, value);

    // TODO
#if 0
    CDC *pDC=GetDC();
    CFont *pOldFont=pDC->SelectObject(GetFont());
    m_nMaxValueWidth=max(m_nMaxValueWidth,pDC->GetTextExtent(pszValue).cx);
    pDC->SelectObject(pOldFont);
    ReleaseDC(pDC);
#endif

    return nIndex;
}

void ecPropertyListCtrl::OnDoubleClick(wxMouseEvent& event)
{
    // Double-clicked the item

    int flags;
    long item = HitTest(event.GetPosition(), flags);
    if (item > -1)
    {
        const wxString strText = wxListCtrlGetItemTextColumn(*this, item,0);
        if(strText == wxT("File")){
            m_pti->ViewHeader();
        } else if (strText == wxT("URL")) {
            m_pti->ViewURL();
        }
    }

    // TODO
#if 0    
    int pos=GetMessagePos();
    CPoint pt(GET_X_LPARAM(pos),GET_Y_LPARAM(pos));
    ScreenToClient(&pt);
    int nItem=HitTest(pt,NULL);
    
    if(GetItemData(nItem)>1){
        // This is a property row
        const CdlGoalExpression goal = dynamic_cast<CdlGoalExpression> ((CdlProperty) GetItemData (nItem));
        if (goal){
            // This is a rule row
            const CdlExpression expression = goal->get_expression ();
            if (1 == expression->references.size ()) // if the property contains a single reference
            {
                // assume that the reference is to another user visible node and try to find it
                std::string macro_name = expression->references [0].get_destination_name ();
                CConfigItem * pItem = CConfigTool::GetConfigToolDoc ()->Find (CString (macro_name.c_str ()));
                if (pItem) // the referenced node was found so select it
                {
                    CConfigTool::GetControlView()->GetTreeCtrl().SelectItem(pItem->HItem());
                }
            }
        }
    } else {
        const CString strText(GetItemText(nItem,0));
        if(strText==FieldTypeImage[File]){
            m_pti->ViewHeader();
        } else if (strText==FieldTypeImage[URL]) {
            m_pti->ViewURL();
        }
    }
#endif
    event.Skip();
}
