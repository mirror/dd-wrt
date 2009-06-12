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
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s): 	sdf
// Contact(s):	sdf
// Date:		1998/08/11
// Version:		0.01
// Purpose:	
// Description:	The Configuration item class
// Requires:	
// Provides:	
// See also:    
// Known bugs:	
// Usage:	
//
//####DESCRIPTIONEND####
//
//===========================================================================

#ifndef _TREEITEM_H
#define _TREEITEM_H
#include "stdafx.h"
#include "FileName.h"

#define INCLUDEFILE <cdl.hxx>
#include "IncludeSTL.h"

class CCdlInterface;
class CdlGoalExpressionBody;
class CdlTransactionBody;

class CConfigItem {
	friend class CConfigToolDoc;
	friend class CCdlInterface;
public:
	CConfigItem(CConfigItem *pParent, CdlUserVisible CdlItem);
	virtual ~CConfigItem();

  bool ChangeVersion (const CString &strVersion);
	bool Unload();
  CdlPackage GetOwnerPackage() const { return GetCdlItem()?(dynamic_cast<CdlPackage> (GetCdlItem()->get_owner ())):NULL; }
  CdlValuable GetCdlValuable() const { return dynamic_cast<CdlValuable> (GetCdlItem()); }
	bool ViewURL();
	bool ViewHeader();
	bool HasRadio () const;
	bool HasBool () const;
	int  EvalEnumStrings (CStringArray &arEnumStrings) const;
	bool IsDescendantOf (CConfigItem *pAncestor);
	void SetHItem (HTREEITEM hItem) { m_hItem=hItem; };
	CString ItemNameOrMacro() const;
	CConfigItem * NextRadio() const;
	bool CanSetValue (ItemIntegerType nValue) const;
	enum TreeItemType { None, Integer, Enum, String, Double, MaxTreeItemType };
	
	static LPCTSTR TreeItemTypeImage[MaxTreeItemType + 1];

	ItemIntegerType Value()	const;
	bool HasModifiedChildren() const;
	bool IsEnabled() const;
	CConfigItem *FirstRadio() const;
	CString GetURL () const;
	const CFileName FileName () const;
	const CString StringValue (CdlValueSource source = CdlValueSource_Current) const;
  const CString StringValue(WhereType where) const;
	const double DoubleValue (CdlValueSource source = CdlValueSource_Current) const;
	void DumpItem();

	// Access functions
	ItemIntegerType DefaultValue() const;
	const CString StringDefaultValue() const { return StringValue (CdlValueSource_Default); }
	const double DoubleDefaultValue () const { return DoubleValue (CdlValueSource_Default); }
  const CString Name() const { return GetCdlItem()?GetCdlItem()->get_display ().c_str ():CString(_T("Configuration")); }
	TreeItemType Type() const { return m_Type; }
	CConfigItem *Parent() const;
	CConfigItem *FirstChild() const;
	CConfigItem *NextSibling() const;
	bool  Modified() const;
	const CString Desc() const { return m_strDesc; }
  const CString Macro() const { return GetCdlItem()?GetCdlItem()->get_name().c_str():CString();  }
	CdlUserVisible GetCdlItem () const { return m_CdlItem; }
	HTREEITEM HItem() const { return m_hItem; }
	bool IsPackage () const { return NULL!=dynamic_cast<CdlPackage> (GetCdlItem()); }

protected:
	bool SetValue (LPCTSTR pszValue, CdlTransaction transaction=NULL);
	bool SetValue (ItemIntegerType nValue, CdlTransaction transaction=NULL);
	bool SetValue (double dValue, CdlTransaction transaction=NULL);
	bool SetEnabled (bool bEnabled, CdlTransaction transaction=NULL);

	HTREEITEM m_hItem;
	CString m_strDesc; // Short description
	TreeItemType m_Type;
	CdlUserVisible m_CdlItem;
};
#endif
