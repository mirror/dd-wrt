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

//#####DESCRIPTIONBEGIN####                                             
//*************************************************************************
// BCMenu.h : header file
// Version : 2.63
// Date : December 2, 1999
// Author : Brent Corkum
// Email :  corkum@rocscience.com
// Latest Version : http://www.rocscience.com/~corkum/BCMenu.html
// 
// Bug Fixes and portions of code supplied by:
//
// Ben Ashley,Girish Bharadwaj,Jean-Edouard Lachand-Robert,
// Robert Edward Caldecott,Kenny Goers,Leonardo Zide,
// Stefan Kuhr,Reiner Jung,Martin Vladic,Kim Yoo Chul,
// Oz Solomonovich
//
// You are free to use/modify this code but leave this header intact.
// This class is public domain so you are free to use it any of
// your applications (Freeware,Shareware,Commercial). All I ask is
// that you let me know so that if you have a real winner I can
// brag to my buddies that some of my code is in your app. I also
// wouldn't mind if you sent me a copy of your application since I
// like to play with new stuff.
//*************************************************************************
//####DESCRIPTIONEND####

#ifndef BCMenu_H
#define BCMenu_H

#include <afxtempl.h>

// BCMenuData class. Fill this class structure to define a single menu item:

class BCMenuData
{
wchar_t *m_szMenuText;
public:
BCMenuData () {menuIconNormal=-1;xoffset=-1;bitmap=NULL;pContext=NULL;
               nFlags=0;nID=0;syncflag=0;m_szMenuText=NULL;};
void SetAnsiString(LPCSTR szAnsiString);
void SetWideString(const wchar_t *szWideString);
const wchar_t *GetWideString(void) {return m_szMenuText;};
~BCMenuData ();
CString GetString(void);//returns the menu text in ANSI or UNICODE
int xoffset;
int menuIconNormal;
UINT nFlags,nID,syncflag;
CImageList *bitmap;
void *pContext; // used to attach user data
};

//struct CMenuItemInfo : public MENUITEMINFO {
struct CMenuItemInfo : public 
//MENUITEMINFO 
#ifndef UNICODE   //SK: this fixes warning C4097: typedef-name 'MENUITEMINFO' used as synonym for class-name 'tagMENUITEMINFOA'
tagMENUITEMINFOA
#else
tagMENUITEMINFOW
#endif
    {
	CMenuItemInfo()
	{ memset(this, 0, sizeof(MENUITEMINFO));
	  cbSize = sizeof(MENUITEMINFO);
	}
};




typedef enum {Normal,TextOnly} HIGHLIGHTSTYLE;

#ifndef UNICODE
#define AppendMenu AppendMenuA
#define InsertMenu InsertMenuA
#define InsertODMenu InsertODMenuA
#define AppendODMenu AppendODMenuA
#define AppendODPopupMenu AppendODPopupMenuA
#define ModifyODMenu ModifyODMenuA
#else
#define AppendMenu AppendMenuW
#define InsertMenu InsertMenuW
#define InsertODMenu InsertODMenuW
#define AppendODMenu AppendODMenuW
#define ModifyODMenu ModifyODMenuW
#define AppendODPopupMenu AppendODPopupMenuW
#endif


class BCMenu : public CMenu  // Derived from CMenu
{
// Construction
public:
BCMenu(); 

// Attributes
protected:
CTypedPtrArray<CPtrArray, BCMenuData*> m_MenuList;  // Stores list of menu items 

// When loading an owner-drawn menu using a Resource, BCMenu must keep track of
// the popup menu's that it creates. Warning, this list *MUST* be destroyed
// last item first :)
CTypedPtrArray<CPtrArray, HMENU>  m_SubMenus;  // Stores list of sub-menus 

// Stores a list of all BCMenu's ever created 
static CTypedPtrArray<CPtrArray, HMENU>  m_AllSubMenus;

// Operations
public: 
// Overrides
// ClassWizard generated virtual function overrides
//{{AFX_VIRTUAL(CCustomMenu)
//}}AFX_VIRTUAL 
// Implementation
public:
  static BOOL IsNewShell(void);
  void SetBitmapBackground(COLORREF color);
  void SetDisableOldStyle(void);
  void UnSetDisableOldStyle(void);
  BOOL GetDisableOldStyle(void);
  void UnSetBitmapBackground(void);
  BOOL AddBitmapToImageList(CImageList *list,UINT nResourceID);
  BOOL LoadFromToolBar(UINT nID,UINT nToolBar,int& xoffset);
  void InsertSpaces(void);
  static LRESULT FindKeyboardShortcut(UINT nChar,UINT nFlags,CMenu *pMenu);
  static void UpdateMenu(CMenu *pmenu);
  static BOOL IsMenu(CMenu *submenu);
  static BOOL IsMenu(HMENU submenu);
  void DrawCheckMark(CDC *pDC,int x,int y,COLORREF color);
  void DrawRadioDot(CDC *pDC,int x,int y,COLORREF color);
  BCMenuData *BCMenu::FindMenuItem(UINT nID);
  BCMenu *FindMenuOption(int nId,int& nLoc);
  BCMenuData *FindMenuOption(wchar_t *lpstrText);
  BOOL GetMenuText(UINT id,CString &string,UINT nFlags = MF_BYPOSITION );
  CImageList *checkmaps;
  BOOL checkmapsshare;
  int m_selectcheck;
  int m_unselectcheck;
  BOOL m_bDynIcons;

  void LoadCheckmarkBitmap(int unselect,int select);
  void DitherBlt(HDC hdcDest, int nXDest, int nYDest, int nWidth, 
                 int nHeight, HBITMAP hbm, int nXSrc, int nYSrc);
  void DitherBlt2(CDC *drawdc, int nXDest, int nYDest, int nWidth, 
                  int nHeight, CBitmap &bmp, int nXSrc, int nYSrc);
  static HBITMAP LoadSysColorBitmap(int nResourceId);
  BOOL GetBitmapFromImageList(CDC* pDC,CImageList *imglist,int nIndex,CBitmap &bmp);
  BOOL ImageListDuplicate(CImageList *il,int xoffset,CImageList *newlist);

virtual ~BCMenu();  // Virtual Destructor 
// Drawing: 
virtual void DrawItem( LPDRAWITEMSTRUCT);  // Draw an item
virtual void MeasureItem( LPMEASUREITEMSTRUCT );  // Measure an item

// Customizing:

void SetIconSize (int, int);  // Set icon size

BOOL AppendODMenuA(LPCSTR lpstrText, 
UINT nFlags = MF_OWNERDRAW,
UINT nID = 0,
int nIconNormal = -1);  

BOOL AppendODMenuW(wchar_t *lpstrText, 
UINT nFlags = MF_OWNERDRAW,
UINT nID = 0,
int nIconNormal = -1);  

BOOL AppendODMenuA(LPCSTR lpstrText, 
UINT nFlags,
UINT nID,
CImageList *il,
int xoffset);

BOOL AppendODMenuW(wchar_t *lpstrText, 
UINT nFlags,
UINT nID,
CImageList *il,
int xoffset);

BOOL InsertODMenuA(UINT nPosition,
LPCSTR lpstrText, 
UINT nFlags = MF_OWNERDRAW,
UINT nID = 0,
int nIconNormal = -1); 

BOOL InsertODMenuW(UINT nPosition,
wchar_t *lpstrText, 
UINT nFlags = MF_OWNERDRAW,
UINT nID = 0,
int nIconNormal = -1);  

BOOL InsertODMenuA(UINT nPosition,
LPCSTR lpstrText, 
UINT nFlags,
UINT nID,
CImageList *il,
int xoffset);

BOOL InsertODMenuW(UINT nPosition,
wchar_t *lpstrText, 
UINT nFlags,
UINT nID,
CImageList *il,
int xoffset);


BOOL ModifyODMenuA(const char *lpstrText,UINT nID=0,int nIconNormal=-1);
BOOL ModifyODMenuA(const char *lpstrText,UINT nID,CImageList *il,int xoffset);
BOOL ModifyODMenuA(const char *lpstrText,UINT nID,CBitmap *bmp);
BOOL ModifyODMenuA(const char *lpstrText,const char *OptionText,int nIconNormal);
BOOL ModifyODMenuW(wchar_t *lpstrText,UINT nID=0,int nIconNormal=-1);
BOOL ModifyODMenuW(wchar_t *lpstrText,UINT nID,CImageList *il,int xoffset);
BOOL ModifyODMenuW(wchar_t *lpstrText,UINT nID,CBitmap *bmp);
BOOL ModifyODMenuW(wchar_t *lpstrText,wchar_t *OptionText,int nIconNormal);
BCMenuData *NewODMenu(UINT pos,UINT nFlags,UINT nID,CString string);
void SynchronizeMenu(void);
void BCMenu::InitializeMenuList(int value);
void BCMenu::DeleteMenuList(void);
BCMenuData *BCMenu::FindMenuList(UINT nID);
virtual BOOL LoadMenu(LPCTSTR lpszResourceName);  // Load a menu
virtual BOOL LoadMenu(int nResource);  // ... 
void AddFromToolBar(CToolBar* pToolBar, int nResourceID);
BOOL Draw3DCheckmark(CDC *dc, const CRect& rc,BOOL bSelected,
                     HBITMAP hbmCheck);
BOOL LoadToolbar(UINT nToolBar);
BOOL LoadToolbars(const UINT *arID,int n);

// new overrides for dynamic menu's
BOOL	RemoveMenu(UINT uiId,UINT nFlags);
BOOL	DeleteMenu(UINT uiId,UINT nFlags);
BOOL  AppendMenuA(UINT nFlags,UINT nIDNewItem=0,const char *lpszNewItem=NULL,int nIconNormal=-1);
BOOL  AppendMenuA(UINT nFlags,UINT nIDNewItem,const char *lpszNewItem,CImageList *il,int xoffset);
BOOL  AppendMenuA(UINT nFlags,UINT nIDNewItem,const char *lpszNewItem,CBitmap *bmp);
BOOL  AppendMenuW(UINT nFlags,UINT nIDNewItem=0,wchar_t *lpszNewItem=NULL,int nIconNormal=-1);
BOOL  AppendMenuW(UINT nFlags,UINT nIDNewItem,wchar_t *lpszNewItem,CImageList *il,int xoffset);
BOOL  AppendMenuW(UINT nFlags,UINT nIDNewItem,wchar_t *lpszNewItem,CBitmap *bmp);
BOOL  InsertMenuA(UINT nPosition,UINT nFlags,UINT nIDNewItem=0,const char *lpszNewItem=NULL,int nIconNormal=-1);
BOOL  InsertMenuA(UINT nPosition,UINT nFlags,UINT nIDNewItem,const char *lpszNewItem,CImageList *il,int xoffset);
BOOL  InsertMenuA(UINT nPosition,UINT nFlags,UINT nIDNewItem,const char *lpszNewItem,CBitmap *bmp);
BOOL  InsertMenuW(UINT nPosition,UINT nFlags,UINT nIDNewItem=0,wchar_t *lpszNewItem=NULL,int nIconNormal=-1);
BOOL  InsertMenuW(UINT nPosition,UINT nFlags,UINT nIDNewItem,wchar_t *lpszNewItem,CImageList *il,int xoffset);
BOOL  InsertMenuW(UINT nPosition,UINT nFlags,UINT nIDNewItem,wchar_t *lpszNewItem,CBitmap *bmp);
BCMenu* AppendODPopupMenuA(LPCSTR lpstrText);
BCMenu* AppendODPopupMenuW(wchar_t *lpstrText);

// Destoying:

virtual BOOL DestroyMenu();

// Generated message map functions
protected:
int m_iconX,m_iconY;
COLORREF m_bitmapBackground;
BOOL m_bitmapBackgroundFlag;
BOOL disable_old_style;
}; 

#endif

//*************************************************************************
