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
// configitem.h :
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   julians
// Contact(s):  julians
// Date:        2000/09/01
// Version:     $Id: //depot/sw/releases/linuxsrc/src/redboot_cobra/ecos/host/tools/configtool/standalone/wxwin/configitem.h#3 $
// Purpose:
// Description: Header file for ecConfigItem, which describes a config item
//              from this tool's perspective. Note that this duplicates
//              some of the CDL data structures in order to be able to test the
//              UI independently; we may eliminate some of the duplication later.
// Requires:
// Provides:
// See also:
// Known bugs:
// Usage:
//
//####DESCRIPTIONEND####
//
//===========================================================================

#ifndef _ECOS_CONFIGITEM_H_
#define _ECOS_CONFIGITEM_H_

#ifdef __GNUG__
#pragma interface "configitem.h"
#endif

#include "wx/variant.h"
#include "wx/treectrl.h"
#include "wx/spinctrl.h"

#include "filename.h"
#include "cdl.hxx"

/*
 * ecConfigType
 * The type of item
 */

enum ecConfigType
{
    ecConfigTypeNone,
    ecContainer,
    ecPackage,
    ecComponent,
    ecOption
};

/*
 * ecOptionFlavor
 * The flavor of the option
 */

enum ecOptionFlavor
{
    ecFlavorNone,
    ecFlavorBool,
    ecFlavorBoolData,
    ecFlavorData
};

/*
 * ecOptionType
 * The type of option, if this is an option
 */

enum ecOptionType
{
    ecOptionTypeNone,
    ecDouble,
    ecLong,
    ecBool,
    ecString,
    ecEnumerated
};

/*
 * ecUIHint
 * What kind of control to use
 */

enum ecUIHint
{
    ecHintNone,
    ecHintCheck,
    ecHintRadio
};

/*
 * Get string from where?
 */

enum ecWhereType
{
    ecInMacro = 0,
    ecInName,
    ecInDesc,
    ecInCurrentValue,
    ecInDefaultValue
};

/*
 * ecConfigItem
 * Represents a node in the configuration hierarchy.
 * For every ecConfigItem, there is also an ecTreeItemData
 * that points to it.
 */
class ecConfigTreeCtrl;
class ecConfigItem: public wxObject
{
DECLARE_CLASS(ecConfigItem)
    friend class ecConfigToolDoc;

public:
    // active: whether greyed out or not
    // enabled: only if this is a Bool or BoolData: whether enabled/disabled (checked/unchecked)
    ecConfigItem(ecConfigItem* parent, const wxString& name, ecConfigType ctype = ecConfigTypeNone,
        ecOptionFlavor flavor = ecFlavorNone, ecOptionType otype = ecOptionTypeNone,
        bool active = TRUE, bool enabled = TRUE, ecUIHint hint = ecHintNone);
    ecConfigItem(ecConfigItem* parent, CdlUserVisible vitem);
    ~ecConfigItem();

//// Accessors
    void SetParent(ecConfigItem* parent) { m_parent = parent; }
    ecConfigItem* GetParent() const { return m_parent; }

    void SetName(const wxString& name) { m_name = name; }
    const wxString& GetName() const { return m_name; }

    void SetMacro(const wxString& name) { m_macro = name; }
    const wxString& GetMacro() const { return m_macro; }

    void SetDescription(const wxString& descr) { m_strDescr = descr; }
    const wxString& GetDescription() const { return m_strDescr; }

    void SetConfigType(ecConfigType ctype) { m_configType = ctype; }
    ecConfigType GetConfigType() const { return m_configType; }

    void SetOptionType(ecOptionType otype) { m_optionType = otype; }
    ecOptionType GetOptionType() const { return m_optionType; }

    void SetOptionFlavor(ecOptionFlavor flavor) { m_optionFlavor = flavor; }
    ecOptionFlavor GetOptionFlavor() const { return m_optionFlavor; }

    void SetValue(const wxVariant& value) { m_value = value; }
    wxVariant& GetValue() { return m_value; }

    // Only appropriate if Bool or BoolData. Otherwise, assume always enabled.
    void SetEnabled(bool enabled) { m_enabled = enabled; }
    bool GetEnabled() const { return m_enabled; }

    // Whether greyed out or not
    void SetActive(bool active) { m_active = active; }
    bool GetActive() const { return m_active; }

    // UI hint
    void SetUIHint(ecUIHint hint) { m_hint = hint; }
    ecUIHint GetUIHint() const { return m_hint; }

    // Gets the value to display (often an empty string)
    wxString GetDisplayValue() const ;

    wxTreeItemId GetTreeItem() const { return m_treeItem; };
    void SetTreeItem(wxTreeItemId id) { m_treeItem = id; };

    CdlUserVisible GetCdlItem() const { return m_CdlItem; }
    void SetCdlItem(CdlUserVisible cdlItem) { m_CdlItem = cdlItem; }

    wxString GetItemNameOrMacro() const;

	bool IsPackage () const { return NULL!=dynamic_cast<CdlPackage> (GetCdlItem()); }

//// Operations
    // Sets the text and icon for this item
    bool UpdateTreeItem(ecConfigTreeCtrl& treeCtrl);

    // Handle a left click on the icon: e.g. (un)check the option
    void OnIconLeftDown(ecConfigTreeCtrl& treeCtrl);

    // Can we start editing this item?
    bool CanEdit() const;

    // Creates an edit window. It will be positioned by the caller.
    wxWindow* CreateEditWindow(wxWindow* parent);
    
    // Transfers data between item and window
    bool TransferDataToWindow(wxWindow* window);
    bool TransferDataFromWindow(wxWindow* window);

    // Convert from Cdl to internal representation
    bool ConvertFromCdl();

    // Bump by specified amount, or toggle if a boolean value
    bool BumpItem(int nInc);

//// TAKEN FROM MFC VERSION

    const ecFileName GetFilename () const;
    CdlPackage GetOwnerPackage() const { return GetCdlItem()?(dynamic_cast<CdlPackage> (GetCdlItem()->get_owner ())):NULL; }
    bool ChangeVersion (const wxString &strVersion);
    CdlValuable GetCdlValuable() const { return dynamic_cast<CdlValuable> (GetCdlItem()); }
    bool Unload();
    wxString GetURL () const;

    bool HasRadio () const;
    ecConfigItem *FirstRadio() const;

    ecConfigItem *FirstChild() const;
    ecConfigItem *NextSibling() const;

    bool HasModifiedChildren() const;
    bool IsEnabled() const;
    bool IsActive() const; // Added JACS

    bool Modified() const;

    void DumpItem();

    ecConfigItem * NextRadio() const;

    bool IsDescendantOf (ecConfigItem *pAncestor);

    bool ViewURL();
    bool ViewHeader();

    bool HasBool () const;

    long DefaultValue() const;
    long Value() const;
    const wxString StringValue (CdlValueSource source = CdlValueSource_Current) const;
    const wxString StringValue(ecWhereType where) const;
    const double DoubleValue (CdlValueSource source = CdlValueSource_Current) const;
    const wxString StringDefaultValue() const { return StringValue (CdlValueSource_Default); }
    const double DoubleDefaultValue () const { return DoubleValue (CdlValueSource_Default); }
    int  EvalEnumStrings (wxArrayString &arEnumStrings) const;

    // Convert a string representation of 'where' (e.g. "Macro names") to
    // ecWhereType
    static ecWhereType WhereStringToType(const wxString& whereString);
    
    // Convert a type representation of 'where' to a string
    static wxString WhereTypeToString(ecWhereType whereType);
    
protected:

    bool SetValue (const wxString& value, CdlTransaction transaction=NULL);
    bool SetValue (double dValue, CdlTransaction transaction=NULL);
    bool SetValue (long nValue, CdlTransaction transaction=NULL);
    bool SetEnabled (bool bEnabled, CdlTransaction transaction=NULL);
    
protected:
    wxString            m_name;
    wxString            m_macro;
    bool                m_enabled;
    bool                m_active;
    ecConfigType        m_configType;
    ecOptionType        m_optionType;
    ecOptionFlavor      m_optionFlavor;
    ecUIHint            m_hint;
    wxVariant           m_value;
    ecConfigItem*       m_parent;
    wxTreeItemId        m_treeItem;
    CdlUserVisible      m_CdlItem;
    wxString            m_strDescr; // Description
};


//// TODO: put these in separate file

/*
 * ecTextEditorCtrl
 * A specialised wxTextCtrl, for editing string config values
 */

class ecTextEditorCtrl: public wxTextCtrl
{
DECLARE_CLASS(ecTextEditorCtrl)
public:
// Ctor(s)
    ecTextEditorCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
        long style = 0);

//// Event handlers

    void OnEnter(wxCommandEvent& event);
    void OnKillFocus(wxFocusEvent& event);
    void OnLeftDClick(wxMouseEvent& event);

DECLARE_EVENT_TABLE()

private:
};

/*
 * ecDoubleEditorCtrl
 * A specialised wxTextCtrl, for editing double config values
 */

class ecDoubleEditorCtrl: public wxTextCtrl
{
DECLARE_CLASS(ecDoubleEditorCtrl)
public:
// Ctor(s)
    ecDoubleEditorCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
        long style = 0);

//// Event handlers

    void OnEnter(wxCommandEvent& event);
    void OnKillFocus(wxFocusEvent& event);

DECLARE_EVENT_TABLE()
};

/*
 * ecIntegerEditorCtrl
 * A specialised wxSpinCtrl, for editing integer config values
 */

class ecIntegerEditorCtrl: public wxSpinCtrl
{
DECLARE_CLASS(ecIntegerEditorCtrl)
public:
// Ctor(s)
    ecIntegerEditorCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
        long style = 0);

//// Event handlers

    void OnEnter(wxCommandEvent& event);
    void OnKillFocus(wxFocusEvent& event);

DECLARE_EVENT_TABLE()
};

/*
 * ecEnumEditorCtrl
 * A specialised wxChoice, for editing enumerated config values
 */

class ecEnumEditorCtrl: public wxChoice
{
DECLARE_CLASS(ecEnumEditorCtrl)
public:
// Ctor(s)
    ecEnumEditorCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
        long style = 0);

//// Event handlers

    void OnChar(wxKeyEvent& event);
    void OnKillFocus(wxFocusEvent& event);

DECLARE_EVENT_TABLE()
};

/*
 * ecEditStringDialog
 * Pops up to make it easier to edit large string values
 */

class ecEditStringDialog : public ecDialog
{
public:
// Ctor(s)
	ecEditStringDialog(const wxString& initialValue, wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
        long style = 0);
    ~ecEditStringDialog();

//// Event handlers

    void OnOK(wxCommandEvent& event);

//// Operations
    void CreateControls(wxWindow* parent);

//// Accessors
    wxString GetValue() const { return m_value; }

//// Member variables
protected:
    DECLARE_EVENT_TABLE()

    wxString    m_value;
};

#endif
        // _ECOS_CONFIGITEM_H_
