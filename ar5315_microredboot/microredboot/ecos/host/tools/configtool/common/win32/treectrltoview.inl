//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s): 	sdf
// Contact(s):	sdf
// Date:		1998/08/11
// Version:		0.01
// Purpose:	
// Description:	Wrapper to avoid use of GetTreeCtrl() in tree view class
// Requires:	
// Provides:	
// See also:    
// Known bugs:	
// Usage:	
//
//####DESCRIPTIONEND####
//
//===========================================================================
	BOOL GetItemRect(HTREEITEM hItem, LPRECT lpRect, BOOL bTextOnly) const { return GetTreeCtrl().GetItemRect(hItem, lpRect, bTextOnly); }
	UINT GetCount() const { return GetTreeCtrl().GetCount(); }
	UINT GetIndent() const { return GetTreeCtrl().GetIndent(); }
	void SetIndent(UINT nIndent) { GetTreeCtrl().SetIndent(nIndent); }
	CImageList* GetImageList(UINT nImageList) const { return GetTreeCtrl().GetImageList(nImageList); }
	CImageList* SetImageList(CImageList* pImageList, int nImageListType) { return GetTreeCtrl().SetImageList(pImageList, nImageListType); }
	HTREEITEM GetNextItem(HTREEITEM hItem, UINT nCode) const { return GetTreeCtrl().GetNextItem(hItem, nCode); }
	HTREEITEM GetChildItem(HTREEITEM hItem) const { return GetTreeCtrl().GetChildItem(hItem); }
	HTREEITEM GetNextSiblingItem(HTREEITEM hItem) const { return GetTreeCtrl().GetNextSiblingItem(hItem); }
	HTREEITEM GetPrevSiblingItem(HTREEITEM hItem) const { return GetTreeCtrl().GetPrevSiblingItem(hItem); }
	HTREEITEM GetParentItem(HTREEITEM hItem) const { return GetTreeCtrl().GetParentItem(hItem); }
	HTREEITEM GetFirstVisibleItem() const { return GetTreeCtrl().GetFirstVisibleItem(); }
	HTREEITEM GetNextVisibleItem(HTREEITEM hItem) const { return GetTreeCtrl().GetNextVisibleItem(hItem); }
	HTREEITEM GetPrevVisibleItem(HTREEITEM hItem) const { return GetTreeCtrl().GetPrevVisibleItem(hItem); }
	HTREEITEM GetSelectedItem() const { return GetTreeCtrl().GetSelectedItem(); }
	HTREEITEM GetDropHilightItem() const { return GetTreeCtrl().GetDropHilightItem(); }
	HTREEITEM GetRootItem() const { return GetTreeCtrl().GetRootItem(); }
	BOOL GetItem(TV_ITEM* pItem) const { return GetTreeCtrl().GetItem(pItem); }
	CString GetItemText(HTREEITEM hItem) const { return GetTreeCtrl().GetItemText(hItem); }
	BOOL GetItemImage(HTREEITEM hItem, int& nImage, int& nSelectedImage) const { return GetTreeCtrl().GetItemImage(hItem, nImage, nSelectedImage); }
	UINT GetItemState(HTREEITEM hItem, UINT nStateMask) const { return GetTreeCtrl().GetItemState(hItem, nStateMask); }
	DWORD GetItemData(HTREEITEM hItem) const { return GetTreeCtrl().GetItemData(hItem); }
	BOOL SetItem(TV_ITEM* pItem) { return GetTreeCtrl().SetItem(pItem); }
	BOOL SetItem(HTREEITEM hItem, UINT nMask, LPCTSTR lpszItem, int nImage,	int nSelectedImage, UINT nState, UINT nStateMask, LPARAM lParam) { return GetTreeCtrl().SetItem(hItem, nMask, lpszItem, nImage,nSelectedImage,nState,nStateMask,lParam); }
	BOOL SetItemText(HTREEITEM hItem, LPCTSTR lpszItem) { return GetTreeCtrl().SetItemText(hItem, lpszItem); }
	BOOL SetItemImage(HTREEITEM hItem, int nImage, int nSelectedImage) { return GetTreeCtrl().SetItemImage(hItem, nImage, nSelectedImage); }
	BOOL SetItemState(HTREEITEM hItem, UINT nState, UINT nStateMask) { return GetTreeCtrl().SetItemState(hItem, nState, nStateMask); }
	BOOL SetItemData(HTREEITEM hItem, DWORD dwData) { return GetTreeCtrl().SetItemData(hItem, dwData); }
	BOOL ItemHasChildren(HTREEITEM hItem) const { return GetTreeCtrl().ItemHasChildren(hItem); }
	CEdit* GetEditControl() const { return GetTreeCtrl(). GetEditControl(); }
	UINT GetVisibleCount() const { return GetTreeCtrl().GetVisibleCount(); }
#ifdef IE4
	CToolTipCtrl* GetToolTips() const {return GetTreeCtrl().GetToolTips() ;}
	CToolTipCtrl* SetToolTips(CToolTipCtrl* pWndTip) const {return GetTreeCtrl().SetToolTips(pWndTip);}
	COLORREF GetBkColor() const {return GetTreeCtrl().GetBkColor();}
	COLORREF SetBkColor(COLORREF clr) {return GetTreeCtrl().SetBkColor(clr);}
	SHORT GetItemHeight() const {return GetTreeCtrl().GetItemHeight();}
	SHORT SetItemHeight(SHORT cyHeight) {return GetTreeCtrl().SetItemHeight(cyHeight);}
	COLORREF GetTextColor() const {return GetTreeCtrl().GetTextColor();}
	COLORREF SetTextColor(COLORREF clr) {return GetTreeCtrl().SetTextColor(clr);}
	BOOL SetInsertMark(HTREEITEM hItem, BOOL fAfter = TRUE) {return GetTreeCtrl().SetInsertMark(hItem, fAfter);}
	BOOL GetCheck(HTREEITEM hItem) const {return GetTreeCtrl().GetCheck(hItem);}
	BOOL SetCheck(HTREEITEM hItem, BOOL fCheck = TRUE) {return GetTreeCtrl().SetCheck(hItem,fCheck);}
	COLORREF GetInsertMarkColor() const {return GetTreeCtrl().GetInsertMarkColor();}
	COLORREF SetInsertMarkColor(COLORREF clrNew) {return GetTreeCtrl().SetInsertMarkColor(clrNew);}
#endif
// Operations

	HTREEITEM InsertItem(LPTV_INSERTSTRUCT lpInsertStruct) { return GetTreeCtrl().InsertItem(lpInsertStruct);  }
	HTREEITEM InsertItem(UINT nMask, LPCTSTR lpszItem, int nImage,int nSelectedImage, UINT nState, UINT nStateMask, LPARAM lParam,HTREEITEM hParent,HTREEITEM hInsertAfter) { return GetTreeCtrl().InsertItem(nMask, lpszItem, nImage,nSelectedImage, nState, nStateMask, lParam,hParent, hInsertAfter); }
	HTREEITEM InsertItem(LPCTSTR lpszItem, HTREEITEM hParent = TVI_ROOT,HTREEITEM hInsertAfter = TVI_LAST) { return GetTreeCtrl().InsertItem(lpszItem, hParent,hInsertAfter); }
	HTREEITEM InsertItem(LPCTSTR lpszItem, int nImage, int nSelectedImage,	HTREEITEM hParent = TVI_ROOT, HTREEITEM hInsertAfter = TVI_LAST) { return GetTreeCtrl().InsertItem(lpszItem,nImage,nSelectedImage,hParent, hInsertAfter); }
	BOOL DeleteItem(HTREEITEM hItem) { return GetTreeCtrl().DeleteItem(hItem); }
	BOOL DeleteAllItems() { return GetTreeCtrl().DeleteAllItems(); }
	BOOL Expand(HTREEITEM hItem, UINT nCode) { return GetTreeCtrl().Expand(hItem,nCode); }
	BOOL Select(HTREEITEM hItem, UINT nCode) { return GetTreeCtrl().Select(hItem,nCode); }
	BOOL SelectItem(HTREEITEM hItem) { return GetTreeCtrl().SelectItem(hItem); }
	BOOL SelectDropTarget(HTREEITEM hItem) { return GetTreeCtrl().SelectDropTarget(hItem); }
	BOOL SelectSetFirstVisible(HTREEITEM hItem) { return GetTreeCtrl().SelectSetFirstVisible(hItem); }
	CEdit* EditLabel(HTREEITEM hItem) { return GetTreeCtrl(). EditLabel(hItem); }
	HTREEITEM HitTest(CPoint pt, UINT* pFlags = NULL) const { return GetTreeCtrl().HitTest(pt,pFlags); }
	HTREEITEM HitTest(TV_HITTESTINFO* pHitTestInfo) const { return GetTreeCtrl().HitTest(pHitTestInfo); }
	CImageList* CreateDragImage(HTREEITEM hItem) { return GetTreeCtrl(). CreateDragImage(hItem); }
	BOOL SortChildren(HTREEITEM hItem) { return GetTreeCtrl().SortChildren(hItem); }
	BOOL EnsureVisible(HTREEITEM hItem) { return GetTreeCtrl().EnsureVisible(hItem); }
	BOOL SortChildrenCB(LPTV_SORTCB pSort) { return GetTreeCtrl().SortChildrenCB(pSort); }
