//####ECOSHOSTGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// This file is part of the eCos host tools.
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
// -------------------------------------------
//####ECOSHOSTGPLCOPYRIGHTEND####
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s): 	sdf
// Contact(s):	sdf
// Date:		1998/08/11
// Version:		0.01
// Purpose:	
// Description:	Wrapper to avoid use of GetListCtrl() in view class
// Requires:	
// Provides:	
// See also:    
// Known bugs:	
// Usage:	
//
//####DESCRIPTIONEND####
//
//===========================================================================
	COLORREF GetBkColor() const { return  GetListCtrl().GetBkColor();}
	BOOL SetBkColor(COLORREF cr) { return  GetListCtrl().SetBkColor(cr);}
	CImageList* GetImageList(int nImageList) const { return  GetListCtrl().GetImageList(nImageList);}
	CImageList* SetImageList(CImageList* pImageList, int nImageListType) { return  GetListCtrl().SetImageList(pImageList, nImageListType);}
	int GetItemCount() const { return  GetListCtrl().GetItemCount();}
	BOOL GetItem(LVITEM* pItem) const { return  GetListCtrl().GetItem(pItem); }
	BOOL SetItem(const LVITEM* pItem) { return  GetListCtrl().SetItem(pItem);}
	BOOL SetItem(int nItem, int nSubItem, UINT nMask, LPCTSTR lpszItem,
		int nImage, UINT nState, UINT nStateMask, LPARAM lParam) { return  GetListCtrl().SetItem(nItem, nSubItem, nMask, lpszItem, nImage, nState, nStateMask, lParam);}
	UINT GetCallbackMask() const  { return  GetListCtrl().GetCallbackMask();}
	BOOL SetCallbackMask(UINT nMask) { return  GetListCtrl().SetCallbackMask(nMask);}
	int GetNextItem(int nItem, int nFlags) const { return  GetListCtrl().GetNextItem(nItem,nFlags);}
	BOOL GetItemRect(int nItem, LPRECT lpRect, UINT nCode) const { return  GetListCtrl().GetItemRect(nItem,lpRect,nCode);}
	BOOL SetItemPosition(int nItem, POINT pt) { return  GetListCtrl().SetItemPosition(nItem,pt);}
	BOOL GetItemPosition(int nItem, LPPOINT lpPoint) const { return  GetListCtrl().GetItemPosition(nItem,lpPoint);}
	int GetStringWidth(LPCTSTR lpsz) const { return  GetListCtrl().GetStringWidth(lpsz);}
	CEdit* GetEditControl() const { return  GetListCtrl().GetEditControl();}
	BOOL GetColumn(int nCol, LVCOLUMN* pColumn) const { return  GetListCtrl().GetColumn(nCol, pColumn);}
	BOOL SetColumn(int nCol, const LVCOLUMN* pColumn) { return  GetListCtrl().SetColumn(nCol, pColumn);}
	int GetColumnWidth(int nCol) const { return  GetListCtrl().GetColumnWidth(nCol);}
	BOOL SetColumnWidth(int nCol, int cx) { return  GetListCtrl().SetColumnWidth(nCol,cx);}
	BOOL GetViewRect(LPRECT lpRect) const { return  GetListCtrl().GetViewRect(lpRect);}
	COLORREF GetTextColor() const { return  GetListCtrl().GetTextColor();}
	BOOL SetTextColor(COLORREF cr) { return  GetListCtrl().SetTextColor(cr);}
	COLORREF GetTextBkColor() const { return  GetListCtrl().GetTextBkColor();}
	BOOL SetTextBkColor(COLORREF cr) { return  GetListCtrl().SetTextBkColor(cr);}
	int GetTopIndex() const { return  GetListCtrl().GetTopIndex();}
	int GetCountPerPage() const { return  GetListCtrl().GetCountPerPage();}
	BOOL GetOrigin(LPPOINT lpPoint) const { return  GetListCtrl().GetOrigin(lpPoint);}
	BOOL SetItemState(int nItem, LVITEM* pItem) { return  GetListCtrl().SetItemState(nItem, pItem);}
	BOOL SetItemState(int nItem, UINT nState, UINT nMask) { return  GetListCtrl().SetItemState(nItem,nState,nMask);}
	UINT GetItemState(int nItem, UINT nMask) const { return  GetListCtrl().GetItemState(nItem,nMask);}
	CString GetItemText(int nItem, int nSubItem) const { return  GetListCtrl().GetItemText(nItem,nSubItem);}
	int GetItemText(int nItem, int nSubItem, LPTSTR lpszText, int nLen) const { return  GetListCtrl().GetItemText(nItem,nSubItem,lpszText,nLen);}
	BOOL SetItemText(int nItem, int nSubItem, LPCTSTR lpszText) { return  GetListCtrl().SetItemText(nItem,nSubItem,lpszText);}
	void SetItemCount(int nItems) { GetListCtrl().SetItemCount(nItems);}
	BOOL SetItemData(int nItem, DWORD dwData) { return  GetListCtrl().SetItemData(nItem,dwData);}
	DWORD GetItemData(int nItem) const { return  GetListCtrl().GetItemData(nItem);}
	UINT GetSelectedCount() const { return  GetListCtrl().GetSelectedCount();}
#ifdef IE4
	int GetNextSelectedItem(POSITION& pos) const { return  GetListCtrl().GetNextSelectedItem(pos);}
	POSITION GetFirstSelectedItemPosition() const { return  GetListCtrl().GetFirstSelectedItemPosition();}
	BOOL SetColumnOrderArray(int iCount, LPINT piArray) { return  GetListCtrl().SetColumnOrderArray(iCount,piArray);}
	BOOL GetColumnOrderArray(LPINT piArray, int iCount = -1) { return  GetListCtrl().GetColumnOrderArray(piArray,iCount);}
	CSize SetIconSpacing(CSize size) { return  GetListCtrl().SetIconSpacing(size);}
	CSize SetIconSpacing(int cx, int cy) { return  GetListCtrl().SetIconSpacing(cx,cy);}
	CHeaderCtrl* GetHeaderCtrl() { return  GetListCtrl().GetHeaderCtrl();}
	HCURSOR GetHotCursor() { return  GetListCtrl().GetHotCursor();}
	HCURSOR SetHotCursor(HCURSOR hc) { return  GetListCtrl().SetHotCursor(hc);}
	BOOL GetSubItemRect(int iItem, int iSubItem, int nArea, CRect& ref) { return  GetListCtrl().GetSubItemRect(iItem,iSubItem,nArea, CRect& ref);}
	int GetHotItem() { return  GetListCtrl().GetHotItem();}
	int SetHotItem(int iIndex) { return  GetListCtrl().SetHotItem(iIndex);}
	int GetSelectionMark() { return  GetListCtrl().GetSelectionMark();}
	int SetSelectionMark(int iIndex) { return  GetListCtrl().SetSelectionMark(iIndex);}
	DWORD GetExtendedStyle() { return  GetListCtrl().GetExtendedStyle();}
	DWORD SetExtendedStyle(DWORD dwNewStyle) { return  GetListCtrl().SetExtendedStyle(dwNewStyle);}
	int SubItemHitTest(LPLVHITTESTINFO pInfo) { return  GetListCtrl().SubItemHitTest(pInfo);}
	void SetWorkAreas(int nWorkAreas, LPRECT lpRect) { return  GetListCtrl().SetWorkAreas(nWorkAreas,lpRect);}
	BOOL SetItemCountEx(int iCount, DWORD dwFlags = LVSICF_NOINVALIDATEALL) { return  GetListCtrl().SetItemCountEx(iCount,dwFlags);}
	CSize ApproximateViewRect(CSize sz = CSize(-1, -1), int iCount = -1) const { return  GetListCtrl().ApproximateViewRect(sz,iCount);}
	BOOL GetBkImage(LVBKIMAGE* plvbkImage) const { return  GetListCtrl().GetBkImage(LVBKIMAGE* plvbkImage);}
	DWORD GetHoverTime() const { return  GetListCtrl().GetHoverTime();}
	void GetWorkAreas(int nWorkAreas, LPRECT prc) const { return  GetListCtrl().GetWorkAreas(nWorkAreas,prc);}
	BOOL SetBkImage(HBITMAP hbm, BOOL fTile = TRUE, int xOffsetPercent = 0, int yOffsetPercent = 0) { return  GetListCtrl().SetBkImage(hbm,fTile,xOffsetPercent,yOffsetPercent);}
	BOOL SetBkImage(LPTSTR pszUrl, BOOL fTile = TRUE, int xOffsetPercent = 0, int yOffsetPercent = 0) { return  GetListCtrl().SetBkImage(pszUrl,fTile,xOffsetPercent,yOffsetPercent);}
	BOOL SetBkImage(LVBKIMAGE* plvbkImage) { return  GetListCtrl().SetBkImage(LVBKIMAGE* plvbkImage);}
	DWORD SetHoverTime(DWORD dwHoverTime = (DWORD)-1) { return  GetListCtrl().SetHoverTime(dwHoverTime);}
	UINT GetNumberOfWorkAreas() const { return  GetListCtrl().GetNumberOfWorkAreas();}
	BOOL GetCheck(int nItem) const { return  GetListCtrl().GetCheck(nItem);}
	BOOL SetCheck(int nItem, BOOL fCheck = TRUE) { return  GetListCtrl().SetCheck(nItem,fCheck);}
#endif

// Operations
	int InsertItem(const LVITEM* pItem) { return  GetListCtrl().InsertItem(pItem);}
	int InsertItem(int nItem, LPCTSTR lpszItem) { return  GetListCtrl().InsertItem(nItem,lpszItem);}
	int InsertItem(int nItem, LPCTSTR lpszItem, int nImage) { return  GetListCtrl().InsertItem(nItem,lpszItem,nImage);}
	BOOL DeleteItem(int nItem) { return  GetListCtrl().DeleteItem(nItem);}
	BOOL DeleteAllItems() { return  GetListCtrl().DeleteAllItems();}
	int FindItem(LVFINDINFO* pFindInfo, int nStart = -1) const { return  GetListCtrl().FindItem(pFindInfo,nStart);}
	int HitTest(LVHITTESTINFO* pHitTestInfo) const { return  GetListCtrl().HitTest(pHitTestInfo);}
	int HitTest(CPoint pt, UINT* pFlags = NULL) const { return  GetListCtrl().HitTest(pt, pFlags);}
	BOOL EnsureVisible(int nItem, BOOL bPartialOK) { return  GetListCtrl().EnsureVisible(nItem,bPartialOK);}
	BOOL Scroll(CSize size) { return  GetListCtrl().Scroll(size);}
	BOOL RedrawItems(int nFirst, int nLast) { return  GetListCtrl().RedrawItems(nFirst,nLast);}
	BOOL Arrange(UINT nCode) { return  GetListCtrl().Arrange(nCode);}
	CEdit* EditLabel(int nItem) { return  GetListCtrl().EditLabel(nItem);}
	int InsertColumn(int nCol, const LVCOLUMN* pColumn) { return  GetListCtrl().InsertColumn(nCol,pColumn);}
	int InsertColumn(int nCol, LPCTSTR lpszColumnHeading, int nFormat = LVCFMT_LEFT, int nWidth = -1, int nSubItem = -1) 
		{ return  GetListCtrl().InsertColumn(nCol,lpszColumnHeading, nFormat, nWidth, nSubItem);}
	BOOL DeleteColumn(int nCol) { return  GetListCtrl().DeleteColumn(nCol);}
	CImageList* CreateDragImage(int nItem, LPPOINT lpPoint) { return  GetListCtrl().CreateDragImage(nItem,lpPoint);}
	BOOL Update(int nItem) { return  GetListCtrl().Update(nItem);}
	BOOL SortItems(PFNLVCOMPARE pfnCompare, DWORD dwData) { return  GetListCtrl().SortItems(pfnCompare,dwData);}

