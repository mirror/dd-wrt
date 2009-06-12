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
// MLTView.cpp: implementation of the CMLTView class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#ifndef PLUGIN
#include "BCMenu.h"
#endif
#include "ConfigtoolDoc.h"
#include "MLTView.h"
#include "ConfigTool.h"
#include "CTUtils.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CMLTView, CScrollView)

BEGIN_MESSAGE_MAP(CMLTView, CScrollView)
  //{{AFX_MSG_MAP(CMLTView)
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONDBLCLK()
    ON_WM_SIZE()
    ON_WM_ERASEBKGND()
    ON_WM_MOUSEMOVE()
    ON_COMMAND(ID_EDIT_NEW_REGION, OnMLTNewRegion)
    ON_COMMAND(ID_EDIT_NEW_SECTION, OnMLTNewSection)
    ON_COMMAND(ID_EDIT_DELETE_SECTION, OnMLTDelete)
    ON_COMMAND(ID_EDIT_PROPERTIES, OnMLTProperties)
    ON_UPDATE_COMMAND_UI(ID_EDIT_NEW_SECTION, OnUpdateMLTNewSection)
    ON_UPDATE_COMMAND_UI(ID_EDIT_DELETE_SECTION, OnUpdateMLTDelete)
    ON_UPDATE_COMMAND_UI(ID_EDIT_PROPERTIES, OnUpdateMLTProperties)
    ON_WM_CONTEXTMENU()
    ON_COMMAND(ID_POPUP_PROPERTIES, OnPopupProperties)
    ON_WM_RBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_WM_HELPINFO()
	ON_WM_MENUCHAR()
	//}}AFX_MSG_MAP
  ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipText)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipText)
  // Standard printing commands
  ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
  ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
  ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

CMLTView::CMLTView()
{
  m_uViewWidth = 0;
  m_uUnitCountMax = 0;
  CConfigTool::SetMLTView(this);
}

CMLTView::~CMLTView()
{
  CConfigTool::SetMLTView(0);
  for(POSITION pos = m_arstrTooltipRects.GetStartPosition(); pos != NULL; ){
    CRect *pRect;
    CString strName;
    m_arstrTooltipRects.GetNextAssoc( pos, strName, (void *&)pRect );
    delete pRect;
  }
}

/////////////////////////////////////////////////////////////////////////////
// CMLTView drawing

#define VERT_BORDER 30 /* the vertical border at the top/bottom of the client area */
#define HORIZ_BORDER 30 /* the horizontal border at the left/right of the client area */
#define BAR_HEIGHT 18 /* the height of the memory section caption bar */
#define MAP_HEIGHT 66 /* the height of each memory section rectangle (including caption bar) */
#define REGION_SPACING 115 /* the vertical offset between successive memory regions */
#define ADDRESS_TEXT_SPACE 0.9 /* use 90% of the section width to draw the address */
#define EXTERNAL_TEXT_BORDER 5 /* spacing between external text and border of region */
#define ADDRESS_FORMAT _T("%08X") /* draw memory addresses in hex format */
#define UNITS_PER_SECTION 3 /* memory section width in units, unused sections are 1 unit wide */
#define UNIT_WIDTH_MIN 27 /* minimum width of memory section unit before horizontal scrolling enabled */
#define TICK_HEIGHT 4 /* the height of the tick marks on the memory sections and regions */

#define DISPLAY_MODE 1 /* 1 == const unit width for all regions */
/* 2 == const region width for all regions */

void CMLTView::OnDraw(CDC* pDC)
{
  CConfigToolDoc* pDoc = CConfigTool::GetConfigToolDoc();
  if (pDoc == NULL) // no document so nothing to draw
    return;
  
  // clear the lists of region and section rectangles used for hit testing
  
  listRegionRect.RemoveAll ();
  listSectionRect.RemoveAll ();
  
  // setup background mode
  
  int nOldBkMode = pDC->SetBkMode (TRANSPARENT);
  
  // setup font
  
  CFont fntName;
  if (!fntName.CreatePointFont (80, _T("MS Sans Serif"), pDC))
    return;
  CFont * pOldFont = pDC->SelectObject (&fntName);
  
  // determine max unit count for any region
  
  mem_map * pMemoryMap = &CConfigTool::GetConfigToolDoc()->MemoryMap;
  
  // calculate the unit scaling for DISPLAY_MODE 1
  
  UINT uPixelsPerUnit = UNIT_WIDTH_MIN;
  RECT rectClientRect;
  if (m_uUnitCountMax != 0) // if there is something to draw
  {
    
    GetClientRect (&rectClientRect);
    uPixelsPerUnit = __max ((m_uClientWidth - HORIZ_BORDER * 2) / m_uUnitCountMax, UNIT_WIDTH_MIN);
    m_uViewWidth = uPixelsPerUnit * m_uUnitCountMax + HORIZ_BORDER * 2;
  }
  
  // draw the regions
  
  UINT uRegion = 0;
  UINT uUnitCount;
  list <mem_region>::iterator region;
  for (region = pMemoryMap->region_list.begin (); region != pMemoryMap->region_list.end (); ++region)
  {
    uUnitCount = 0;
    for (list <mem_section_view>::iterator section_view = region->section_view_list.begin (); section_view != region->section_view_list.end (); ++section_view)
      uUnitCount += (section_view->section == NULL ? 1 : UNITS_PER_SECTION);
    
    if (DISPLAY_MODE == 1)
      DrawRegion (pDC, uRegion++, uUnitCount, uPixelsPerUnit, region);
    else // DISPLAY_MODE == 2
      DrawRegion (pDC, uRegion++, uUnitCount, (rectClientRect.right - HORIZ_BORDER * 2) / uUnitCount, region);
  }    
  
  pDC->SelectObject (pOldFont);
  pDC->SetBkMode (nOldBkMode);
}


void CMLTView::DrawRegion (CDC* pDC, UINT uRegion, UINT uUnitCount, UINT uPixelsPerUnit, list <mem_region>::iterator region)
{
  BOOL bDrawFocusRect = FALSE;
  CRect rectAddress;
  CString strAddress;
  
  // setup the drawing objects
  
  CBrush brshUnusedSection;
  if (!brshUnusedSection.CreateHatchBrush (HS_BDIAGONAL, RGB (128, 128, 128)))
    return;
  
  CBrush brshUsedSection;
  if (!brshUsedSection.CreateSolidBrush (GetSysColor (COLOR_WINDOW)))
    return;
  
  CBrush brshInitialSectionBar;
  if (!brshInitialSectionBar.CreateSolidBrush (GetSysColor (COLOR_INACTIVECAPTION)))
    return;
  
  CBrush brshFixedSectionBar;
  if (!brshFixedSectionBar.CreateSolidBrush (GetSysColor (COLOR_ACTIVECAPTION)))
    return;
  
  CBrush brshFinalSectionBar;
  if (!brshFinalSectionBar.CreateSolidBrush (GetSysColor (COLOR_ACTIVECAPTION)))
    return;
  
  CPen penBorder;
  if (!penBorder.CreatePen (PS_SOLID, 1, GetSysColor (COLOR_WINDOWTEXT)))
    return;
  
  CPen penSelected;
  if (!penSelected.CreatePen (PS_SOLID, 2, GetSysColor (COLOR_WINDOWTEXT)))
    return;
  
  // select the border pen object
  
  CPen * pOldPen = pDC->SelectObject (&penBorder);
  
  // calculate the region rectangle
  
  REGIONRECT srRegion;
  srRegion.Rect.SetRect (HORIZ_BORDER, VERT_BORDER + REGION_SPACING * uRegion, HORIZ_BORDER + uUnitCount * uPixelsPerUnit + 1, VERT_BORDER + REGION_SPACING * uRegion + MAP_HEIGHT + 1);
  srRegion.Region = region;
  listRegionRect.AddTail (srRegion);
  
  // draw the region
  
  CPoint pointOrigin (srRegion.Rect.left, srRegion.Rect.top);
  pDC->LPtoDP (&pointOrigin);
  pointOrigin.x %= 8;
  pointOrigin.y %= 8;
  pDC->SetBrushOrg (pointOrigin);
  CBrush * pOldBrush = pDC->SelectObject (&brshUnusedSection);
  pDC->Rectangle (srRegion.Rect);
  /*
  pDC->MoveTo (srRegion.Rect.left, srRegion.Rect.bottom - 1); // draw tick
  pDC->LineTo (srRegion.Rect.left, srRegion.Rect.bottom + TICK_HEIGHT); // draw tick
  */
  if (region == m_riSelectedRegion)
  {
    bDrawFocusRect = TRUE;
    m_rectSelectedItem = srRegion.Rect;
  }
  
  // draw the region label
  
  CRect rectRegionLabel (HORIZ_BORDER, VERT_BORDER + REGION_SPACING * uRegion - EXTERNAL_TEXT_BORDER - 20, m_uViewWidth - HORIZ_BORDER /*HORIZ_BORDER + uUnitCount * uPixelsPerUnit + 1*/, VERT_BORDER + REGION_SPACING * uRegion - EXTERNAL_TEXT_BORDER);
  CString strRegionLabel;
  strRegionLabel.Format (_T("%s (%08X-%08X)%s"), CString(region->name.c_str ()), region->address, region->address + region->size - 1, ((region->type == read_only) ? _T(" read only") : _T("")));
  pDC->DrawText (strRegionLabel, -1, rectRegionLabel, DT_BOTTOM | DT_SINGLELINE);
  
  // draw the start address of the region
  /*
  rectAddress.SetRect (HORIZ_BORDER, VERT_BORDER + REGION_SPACING * uRegion + MAP_HEIGHT + EXTERNAL_TEXT_BORDER, HORIZ_BORDER + ADDRESS_TEXT_SPACE * UNITS_PER_SECTION * uPixelsPerUnit, VERT_BORDER + REGION_SPACING * uRegion + MAP_HEIGHT + EXTERNAL_TEXT_BORDER + 30);
  strAddress.Format (ADDRESS_FORMAT, region->address);
  pDC->DrawText (strAddress, -1, rectAddress, DT_LEFT | DT_SINGLELINE);
  */
  // draw the end address of the region
  /*
  rectAddress.SetRect (HORIZ_BORDER + (uUnitCount - ADDRESS_TEXT_SPACE) * uPixelsPerUnit, VERT_BORDER + REGION_SPACING * uRegion + MAP_HEIGHT + EXTERNAL_TEXT_BORDER, HORIZ_BORDER + (uUnitCount + ADDRESS_TEXT_SPACE) * uPixelsPerUnit, VERT_BORDER + REGION_SPACING * uRegion + MAP_HEIGHT + EXTERNAL_TEXT_BORDER + 30);
  strAddress.Format (ADDRESS_FORMAT, region->address + region->size);
  pDC->DrawText (strAddress, -1, rectAddress, DT_CENTER | DT_SINGLELINE);
  */
  // draw the sections within the region
  
  UINT uSectionUnitCount = 0;
  CRect rectBar;
  SECTIONRECT srSection;
  for (list <mem_section_view>::iterator section_view = region->section_view_list.begin (); section_view != region->section_view_list.end (); ++section_view)
  {
    if (section_view->section != NULL) // the section is used
    {
      // draw the section
      
      pDC->SelectObject (brshUsedSection);
      srSection.Rect.SetRect (HORIZ_BORDER + uSectionUnitCount * uPixelsPerUnit, VERT_BORDER + REGION_SPACING * uRegion, HORIZ_BORDER + (uSectionUnitCount + UNITS_PER_SECTION) * uPixelsPerUnit + 1, VERT_BORDER + REGION_SPACING * uRegion + MAP_HEIGHT + 1);
      srSection.SectionView = section_view;
      listSectionRect.AddTail (srSection);
      pDC->Rectangle (srSection.Rect);
      if (section_view == m_sviSelectedSectionView)
      {
        bDrawFocusRect = TRUE;
        m_rectSelectedItem = srSection.Rect;
      }
      
      // draw text within the section
      
      CString strSection, strSectionLine;
      
      if ((section_view->section_location != initial_location) && (section_view->section->alignment > 1))
      {
        strSectionLine.Format (_T("align %lX\n"), section_view->section->alignment);
        strSection += strSectionLine;
      }
      
      if (section_view->section->size > 0)
      {
        strSectionLine.Format (_T("size %lX\n"), section_view->section->size);
        strSection += strSectionLine;
      }
      
      if (section_view->section_location == final_location)
      {
        strSectionLine.Format (_T("relocated\n"));
        strSection += strSectionLine;
      }
      
      pDC->DrawText (strSection, -1, srSection.Rect - (LPCRECT) CRect (EXTERNAL_TEXT_BORDER, EXTERNAL_TEXT_BORDER + BAR_HEIGHT, EXTERNAL_TEXT_BORDER, EXTERNAL_TEXT_BORDER), DT_LEFT);
      
      // select caption bar colour according to type of section
      
      if (section_view->section_location == initial_location)
      {
        pDC->SetTextColor (GetSysColor (COLOR_INACTIVECAPTIONTEXT));
        pDC->SelectObject (&brshInitialSectionBar);
      }
      else
      {
        pDC->SetTextColor (GetSysColor (COLOR_CAPTIONTEXT));
        pDC->SelectObject (&brshFinalSectionBar);
      }
      
      // draw the caption bar
      
      rectBar.SetRect (HORIZ_BORDER + uSectionUnitCount * uPixelsPerUnit, VERT_BORDER + REGION_SPACING * uRegion, HORIZ_BORDER + (uSectionUnitCount + UNITS_PER_SECTION) * uPixelsPerUnit + 1, VERT_BORDER + REGION_SPACING * uRegion + BAR_HEIGHT + 1);
      pDC->Rectangle (rectBar);
      
      // draw the section name within the caption bar
      
      CString strName(section_view->section->name.c_str ());
      CRect *pRect=NULL;
      m_arstrTooltipRects.Lookup(strName,(void *&)pRect);
      UINT format;
      if(pDC->GetTextExtent(strName).cx>rectBar.Width()-2*EXTERNAL_TEXT_BORDER){
        format=DT_LEFT;
        if(pRect){
          pRect->CopyRect(rectBar);
        } else {
          pRect=new CRect;
          m_arstrTooltipRects.SetAt(strName,pRect);
        }
        // Replace final three characters of name with an elipsis
        int nLength=1+max(1,strName.GetLength()-3);
        do {
          --nLength;
          strName=strName.Left(nLength)+_T("...");
        } while(nLength>1 && pDC->GetTextExtent(strName).cx>rectBar.Width()-2*EXTERNAL_TEXT_BORDER);
       
        rectBar.left+=EXTERNAL_TEXT_BORDER;
        rectBar.right-=EXTERNAL_TEXT_BORDER;
      } else {
        format=DT_CENTER;
        if (pRect) {
          m_arstrTooltipRects.RemoveKey(strName);
        }
      }

      pDC->DrawText (strName, -1, rectBar, format | DT_VCENTER | DT_SINGLELINE);
      pDC->SetTextColor (GetSysColor (COLOR_WINDOWTEXT));
      
      // find the mem_section item describing the current section_view item
      
      list <mem_section>::iterator MemorySection = section_view->section;
      
      // draw the section address if appropriate
      
      if ((section_view->section_location == initial_location))
      {
        if (MemorySection->initial_location->anchor == absolute)
        {
          pDC->MoveTo (srSection.Rect.left, srSection.Rect.bottom - 1); // draw tick
          pDC->LineTo (srSection.Rect.left, srSection.Rect.bottom + TICK_HEIGHT); // draw tick
          rectAddress.SetRect (HORIZ_BORDER + uSectionUnitCount * uPixelsPerUnit, VERT_BORDER + REGION_SPACING * uRegion + MAP_HEIGHT + EXTERNAL_TEXT_BORDER, (int) (HORIZ_BORDER + (uSectionUnitCount + ADDRESS_TEXT_SPACE * UNITS_PER_SECTION) * uPixelsPerUnit), VERT_BORDER + REGION_SPACING * uRegion + MAP_HEIGHT + EXTERNAL_TEXT_BORDER + 30);
          strAddress.Format (ADDRESS_FORMAT, MemorySection->initial_location->address);
          pDC->DrawText (strAddress, -1, rectAddress, DT_LEFT | DT_SINGLELINE);
          
          /*
          if (MemorySection->size > 0) // the end address can be calculated
          {
          rectAddress.SetRect (HORIZ_BORDER + (uSectionUnitCount + UNITS_PER_SECTION - ADDRESS_TEXT_SPACE) * uPixelsPerUnit, VERT_BORDER + REGION_SPACING * uRegion + MAP_HEIGHT + EXTERNAL_TEXT_BORDER, HORIZ_BORDER + (uSectionUnitCount + UNITS_PER_SECTION + ADDRESS_TEXT_SPACE) * uPixelsPerUnit, VERT_BORDER + REGION_SPACING * uRegion + MAP_HEIGHT + EXTERNAL_TEXT_BORDER + 30);
          strAddress.Format (ADDRESS_FORMAT, MemorySection->initial_location->address + MemorySection->size);
          pDC->DrawText (strAddress, -1, rectAddress, DT_CENTER | DT_SINGLELINE);
          }
          */
        }
      }
      
      else if ((section_view->section_location == final_location) || (section_view->section_location == fixed_location))
      {
        if (MemorySection->final_location->anchor == absolute)
        {
          pDC->MoveTo (srSection.Rect.left, srSection.Rect.bottom - 1); // draw tick
          pDC->LineTo (srSection.Rect.left, srSection.Rect.bottom + TICK_HEIGHT); // draw tick
          rectAddress.SetRect (HORIZ_BORDER + uSectionUnitCount * uPixelsPerUnit, VERT_BORDER + REGION_SPACING * uRegion + MAP_HEIGHT + EXTERNAL_TEXT_BORDER, (int) (HORIZ_BORDER + (uSectionUnitCount + ADDRESS_TEXT_SPACE * UNITS_PER_SECTION) * uPixelsPerUnit), VERT_BORDER + REGION_SPACING * uRegion + MAP_HEIGHT + EXTERNAL_TEXT_BORDER + 30);
          strAddress.Format (ADDRESS_FORMAT, MemorySection->final_location->address);
          pDC->DrawText (strAddress, -1, rectAddress, DT_LEFT | DT_SINGLELINE);
          
          /*
          if (MemorySection->size > 0) // the end address can be calculated
          {
          rectAddress.SetRect (HORIZ_BORDER + (uSectionUnitCount + UNITS_PER_SECTION - ADDRESS_TEXT_SPACE) * uPixelsPerUnit, VERT_BORDER + REGION_SPACING * uRegion + MAP_HEIGHT + EXTERNAL_TEXT_BORDER, HORIZ_BORDER + (uSectionUnitCount + UNITS_PER_SECTION + ADDRESS_TEXT_SPACE) * uPixelsPerUnit, VERT_BORDER + REGION_SPACING * uRegion + MAP_HEIGHT + EXTERNAL_TEXT_BORDER + 30);
          strAddress.Format (ADDRESS_FORMAT, MemorySection->final_location->address + MemorySection->size);
          pDC->DrawText (strAddress, -1, rectAddress, DT_CENTER | DT_SINGLELINE);
          }
          */
        }
      }
      
      uSectionUnitCount += UNITS_PER_SECTION;
    }
    else
    {
      uSectionUnitCount++; // unused sections occupy a single unit
    }
  }
  
  // draw the focus rectangle around the selected object (if any)
  
  if (bDrawFocusRect)
    pDC->DrawFocusRect (m_rectSelectedItem + CRect (1, 1, 1, 1));
  
  // restore previous drawing objects
  
  pDC->SelectObject (pOldBrush);
  pDC->SelectObject (pOldPen);
}


SECTIONRECT * CMLTView::SectionHitTest (CPoint pntTest)
{
  for (POSITION posSection = listSectionRect.GetHeadPosition (); posSection != NULL; listSectionRect.GetNext (posSection))
  {
    if (listSectionRect.GetAt (posSection).Rect.PtInRect (pntTest))
      return & listSectionRect.GetAt (posSection);
  }
  
  return NULL;
}


REGIONRECT * CMLTView::RegionHitTest (CPoint pntTest)
{
  for (POSITION posRegion = listRegionRect.GetHeadPosition (); posRegion != NULL; listRegionRect.GetNext (posRegion))
  {
    CRect rectRegion = listRegionRect.GetAt (posRegion).Rect +
      CRect (EXTERNAL_TEXT_BORDER + 20, EXTERNAL_TEXT_BORDER + 20, EXTERNAL_TEXT_BORDER + 20, EXTERNAL_TEXT_BORDER + 20); // extended rectangle to allow clicking on region label
    if (rectRegion.PtInRect (pntTest))
      return & listRegionRect.GetAt (posRegion);
  }
  
  return NULL;
}


/////////////////////////////////////////////////////////////////////////////
// CMLTView printing

BOOL CMLTView::OnPreparePrinting(CPrintInfo* pInfo)
{
  // default preparation
  return DoPreparePrinting(pInfo);
}

void CMLTView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
  // TODO: add extra initialization before printing
}

void CMLTView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
  // TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CMLTView diagnostics

#ifdef _DEBUG
void CMLTView::AssertValid() const
{
  CView::AssertValid();
}

void CMLTView::Dump(CDumpContext& dc) const
{
  CView::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMLTView message handlers


void CMLTView::OnLButtonDown(UINT nFlags, CPoint point) 
{
  if (CConfigTool::GetConfigToolDoc() == NULL) // nothing to do
    return;
  
  list <mem_section_view>::iterator sviOldSelectedSectionView = m_sviSelectedSectionView;
  list <mem_region>::iterator riOldSelectedRegion = m_riSelectedRegion;
  CRect rectOldSelectedItem = m_rectSelectedItem;
  
  CConfigTool::GetConfigToolDoc()->strSelectedRegion = _T("");
  CConfigTool::GetConfigToolDoc()->strSelectedSection = _T("");
  
  SECTIONRECT * psrSection = SectionHitTest (point + GetScrollPosition ());
  if (psrSection != NULL) // a section was clicked
  {
    m_sviSelectedSectionView = psrSection->SectionView;
    m_rectSelectedItem = psrSection->Rect;
    m_riSelectedRegion = NULL;
    CConfigTool::GetConfigToolDoc()->strSelectedSection = m_sviSelectedSectionView->section->name.c_str ();
  }
  
  else // no section was clicked, test for regions
  {
    REGIONRECT * prrRegion = RegionHitTest (point + GetScrollPosition ());
    if (prrRegion != NULL) // a region was clicked, but not a section
    {
      m_riSelectedRegion = prrRegion->Region;
      m_rectSelectedItem = prrRegion->Rect;
      m_sviSelectedSectionView = NULL;
      CConfigTool::GetConfigToolDoc()->strSelectedRegion = m_riSelectedRegion->name.c_str ();
    }
  }
  
  // redraw the focus rectangle if the selection has changed
  
  if ((m_sviSelectedSectionView != sviOldSelectedSectionView) || (m_riSelectedRegion != riOldSelectedRegion))
  {
    //		CDC * pDC = GetDC ();
    if ((sviOldSelectedSectionView != NULL) || (riOldSelectedRegion != NULL))
      //			pDC->DrawFocusRect (rectOldSelectedItem + CRect (1, 1, 1, 1) - GetScrollPosition ());
      InvalidateRect (rectOldSelectedItem + CRect (1, 1, 1, 1) - GetScrollPosition ()); // paint over the old focus rectangle
    if ((m_sviSelectedSectionView != NULL) || (m_riSelectedRegion != NULL))
      //			pDC->DrawFocusRect (m_rectSelectedItem + CRect (1, 1, 1, 1) - GetScrollPosition ());
      InvalidateRect (m_rectSelectedItem + CRect (1, 1, 1, 1) - GetScrollPosition ()); // paint the new focus rectangle
  }
  
  // perform default processing
  
  CView::OnLButtonDown(nFlags, point);
}

void CMLTView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
  if (CConfigTool::GetConfigToolDoc() != NULL)
    CConfigTool::GetConfigToolDoc()->OnMLTProperties ();
  
  CView::OnLButtonDblClk(nFlags, point);
}

void CMLTView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{	
  if ((lHint != 0) && (pHint != NULL) && (lHint != CConfigToolDoc::MemLayoutChanged))
    return; // no need to invalidate the view
  m_arstrTooltipRects.RemoveAll();

  CalcUnitCountMax (); // recalculate the total view width since the section count may have changed
  if (m_uUnitCountMax == 0 || (m_uClientWidth < HORIZ_BORDER * 2)) // there is nothing to draw
    m_uViewWidth = 0;
  else // allow horizontal scrolling when the unit width reduces to UNIT_WIDTH_MIN
    m_uViewWidth = __max ((m_uClientWidth - HORIZ_BORDER * 2) / m_uUnitCountMax, UNIT_WIDTH_MIN) * m_uUnitCountMax + HORIZ_BORDER * 2;
  
  SIZE sizeTotal;
  sizeTotal.cx = __max (m_uViewWidth, m_uClientWidth);
  sizeTotal.cy = CConfigTool::GetConfigToolDoc()->MemoryMap.region_list.size () * REGION_SPACING + EXTERNAL_TEXT_BORDER * 2;
  SetScrollSizes (MM_TEXT, sizeTotal);
  
  CScrollView::OnUpdate (pSender, lHint, pHint);
}

void CMLTView::OnSize(UINT nType, int cx, int cy) 
{
  CScrollView::OnSize(nType, cx, cy);
  
  m_uClientWidth = cx;
  if (m_uUnitCountMax == 0) // there is nothing to draw
    m_uViewWidth = 0;
  else // allow horizontal scrolling when the unit width reduces to UNIT_WIDTH_MIN
    m_uViewWidth = __max ((cx - HORIZ_BORDER * 2) / m_uUnitCountMax, UNIT_WIDTH_MIN) * m_uUnitCountMax + HORIZ_BORDER * 2;
  
  SIZE sizeTotal;
  sizeTotal.cx = __max (m_uViewWidth, m_uClientWidth);
  if (CConfigTool::GetConfigToolDoc() == NULL)
    sizeTotal.cy = 0;
  else
    sizeTotal.cy = CConfigTool::GetConfigToolDoc()->MemoryMap.region_list.size () * REGION_SPACING + EXTERNAL_TEXT_BORDER * 2;
  SetScrollSizes (MM_TEXT, sizeTotal);
}

void CMLTView::CalcUnitCountMax ()
{
  UINT uUnitCountMax = 0;
  UINT uUnitCount;
  list <mem_region>::iterator region;
  mem_map * pMemoryMap = & (CConfigTool::GetConfigToolDoc()->MemoryMap);
  for (region = pMemoryMap->region_list.begin (); region != pMemoryMap->region_list.end (); ++region)
  {
    uUnitCount = 0;
    for (list <mem_section_view>::iterator section_view = region->section_view_list.begin (); section_view != region->section_view_list.end (); ++section_view)
      uUnitCount += (section_view->section == NULL ? 1 : UNITS_PER_SECTION);
    
    if (uUnitCount > uUnitCountMax)
      uUnitCountMax = uUnitCount;
  }
  m_uUnitCountMax = uUnitCountMax;
}

BOOL CMLTView::OnEraseBkgnd(CDC* pDC) 
{
  // Work around bug apparently caused by SlickEdit
  CRect rcClient;
  pDC->GetClipBox(rcClient);
  pDC->FillSolidRect(rcClient,GetSysColor(COLOR_WINDOW));
		
  return TRUE;//return CScrollView::OnEraseBkgnd(pDC);
}

BOOL CMLTView::OnToolTipText(UINT id, NMHDR* pNMHDR, LRESULT*pResult)
{
	// need to handle both ANSI and UNICODE versions of the message
	TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
	TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;
	id=pNMHDR->idFrom;

  if(id){
    for(POSITION pos = m_arstrTooltipRects.GetStartPosition(); pos != NULL; ){
      CRect *pRect;
      CString strTipText;
      m_arstrTooltipRects.GetNextAssoc( pos, strTipText, (void *&)pRect );
      if((UINT)pRect==id){
    #ifndef _UNICODE
	      if (pNMHDR->code == TTN_NEEDTEXTA)
		      lstrcpyn(pTTTA->szText, strTipText, sizeof pTTTA->szText/sizeof TCHAR - 1);
	      else
		      _mbstowcsz(pTTTW->szText, strTipText, sizeof pTTTW->szText/sizeof TCHAR - 1);
    #else
	      if (pNMHDR->code == TTN_NEEDTEXTA)
		      _wcstombsz(pTTTA->szText, strTipText, sizeof pTTTA->szText/sizeof TCHAR - 1);
	      else
		      lstrcpyn(pTTTW->szText, strTipText, sizeof pTTTW->szText/sizeof TCHAR - 1);
    #endif
        *pResult = 0;
        return TRUE;    // message was handled
      }
    }
  }
  return FALSE;
}

int CMLTView::OnToolHitTest( CPoint point, TOOLINFO* pTI ) const
{
  for(POSITION pos = m_arstrTooltipRects.GetStartPosition(); pos != NULL; ){
    CRect *pRect;
    CString strName;
    m_arstrTooltipRects.GetNextAssoc( pos, strName, (void *&)pRect );
    
    if(pRect->PtInRect(point)){
      pTI->hwnd = m_hWnd;
      pTI->uId = (UINT)pRect; 
      pTI->uFlags = 0 ;
      GetClientRect(&pTI->rect);
      pTI->lpszText=LPSTR_TEXTCALLBACK;
	    return pTI->uId;
    }
  }
  return -1;
}

void CMLTView::OnInitialUpdate() 
{
	CScrollView::OnInitialUpdate();
	EnableToolTips(true);
}

void CMLTView::OnMouseMove(UINT nFlags, CPoint point) 
{
/*
  CString strToolTipText;
  for(POSITION pos = m_arstrTooltipRects.GetStartPosition(); pos != NULL; ){
    CRect *pRect;
    CString strName;
    m_arstrTooltipRects.GetNextAssoc( pos, strName, (void *&)pRect );
    
    if(pRect->PtInRect(point)){
      strToolTipText=strName;
      break;
    }
  }
  if(strToolTipText!=m_strTipText){
	  _AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	  CToolTipCtrl* pToolTip = pThreadState->m_pToolTip;
    if(!m_strTipText.IsEmpty()){
      pToolTip->Activate(false);
    }
    pToolTip->Activate(!strToolTipText.IsEmpty());
    m_strTipText=strToolTipText;
  }
*/
	CScrollView::OnMouseMove(nFlags, point);
}

void CMLTView::OnMLTNewRegion ()
{
  CConfigTool::GetConfigToolDoc ()->OnMLTNewRegion ();
}

void CMLTView::OnMLTNewSection ()
{
  CConfigTool::GetConfigToolDoc ()->OnMLTNewSection ();
}

void CMLTView::OnMLTDelete ()
{
  CConfigTool::GetConfigToolDoc ()->OnMLTDelete ();
}

void CMLTView::OnMLTProperties ()
{
  CConfigTool::GetConfigToolDoc ()->OnMLTProperties ();
}

void CMLTView::OnUpdateMLTNewRegion (CCmdUI* pCmdUI)
{
  pCmdUI->Enable (true);
}

void CMLTView::OnUpdateMLTNewSection (CCmdUI* pCmdUI)
{
  pCmdUI->Enable (CConfigTool::GetConfigToolDoc()->MemoryMap.region_list.size() > 0);
}

void CMLTView::OnUpdateMLTDelete (CCmdUI* pCmdUI)
{
  pCmdUI->Enable(
    (!CConfigTool::GetConfigToolDoc ()->strSelectedRegion.IsEmpty() || !CConfigTool::GetConfigToolDoc ()->strSelectedSection.IsEmpty()));
}

void CMLTView::OnUpdateMLTProperties (CCmdUI* pCmdUI)
{
  pCmdUI->Enable(
    (!CConfigTool::GetConfigToolDoc ()->strSelectedRegion.IsEmpty() || !CConfigTool::GetConfigToolDoc ()->strSelectedSection.IsEmpty()));
}


void CMLTView::OnContextMenu(CWnd*, CPoint point) 
{
  CConfigToolDoc* pDoc = CConfigTool::GetConfigToolDoc();
  if (!pDoc->strSelectedRegion.IsEmpty() || !pDoc->strSelectedSection.IsEmpty()){
    if(point.x<0){
      // Keyboard
      point=!pDoc->strSelectedRegion.IsEmpty()?m_rectSelectedItem.TopLeft():m_rectSelectedItem.CenterPoint();
      ClientToScreen(&point);
    }
    Menu menu;
    menu.LoadMenu(IDR_MLTVIEW_POPUP);

    #ifndef PLUGIN
    menu.LoadToolbar(IDR_MISCBAR);
    #endif

    Menu *pPopup=(Menu *)menu.GetSubMenu(0);
    pPopup->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, point.x,point.y,this);  
  }
}   

void CMLTView::OnPopupProperties() 
{
  CConfigTool::GetConfigToolDoc()->OnMLTProperties ();
}

void CMLTView::OnRButtonDown(UINT nFlags, CPoint point) 
{
  OnLButtonDown(nFlags, point);
  ClientToScreen(&point);
  OnContextMenu(this, point) ;
}

void CMLTView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
  POSITION pos;
  switch(nChar){
    case VK_RIGHT:
      pos = listSectionRect.GetHeadPosition ();
      if(!CConfigTool::GetConfigToolDoc()->strSelectedSection.IsEmpty()){
        while(NULL!=pos){
          if (listSectionRect.GetNext (pos).SectionView==m_sviSelectedSectionView){
            if(NULL==pos){
              pos=listSectionRect.GetHeadPosition();
            }
            break;
          }
        }
      }
      OnLButtonDown(0, listSectionRect.GetNext(pos).Rect.CenterPoint()-GetScrollPosition());
      break;
    case VK_LEFT:
      pos = listSectionRect.GetTailPosition ();
      if(!CConfigTool::GetConfigToolDoc()->strSelectedSection.IsEmpty()){
        while(NULL!=pos){
          if (listSectionRect.GetPrev (pos).SectionView==m_sviSelectedSectionView){
            if(NULL==pos){
              pos=listSectionRect.GetTailPosition();
            }
            break;
          }
        }
      }
      OnLButtonDown(0, listSectionRect.GetPrev(pos).Rect.CenterPoint()-GetScrollPosition());
      break;
    case VK_DOWN:
      pos = listRegionRect.GetHeadPosition ();
      if(!CConfigTool::GetConfigToolDoc()->strSelectedRegion.IsEmpty()){
        while(NULL!=pos){
          if (listRegionRect.GetNext (pos).Region==m_riSelectedRegion){
            if(NULL==pos){
              pos=listRegionRect.GetHeadPosition();
            }
            break;
          }
        }
      }
      OnLButtonDown(0, listRegionRect.GetNext(pos).Rect.TopLeft()-CPoint(1,1)-GetScrollPosition());
      break;
    case VK_UP:
      pos = listRegionRect.GetTailPosition ();
      if(!CConfigTool::GetConfigToolDoc()->strSelectedRegion.IsEmpty()){
        while(NULL!=pos){
          if (listRegionRect.GetPrev (pos).Region==m_riSelectedRegion){
            if(NULL==pos){
              pos=listRegionRect.GetTailPosition();
            }
            break;
          }
        }
      }
      OnLButtonDown(0, listRegionRect.GetPrev(pos).Rect.TopLeft()-CPoint(1,1)-GetScrollPosition());
      break;
    default:
      break;
  }
	
	CScrollView::OnKeyDown(nChar, nRepCnt, nFlags);
}


BOOL CMLTView::OnHelpInfo(HELPINFO*) 
{
  return CConfigTool::GetConfigToolDoc()->ShowURL(CUtils::LoadString(IDS_MLT_VIEW_HELP));	
}

LRESULT CMLTView::OnMenuChar(UINT, UINT, CMenu*) 
{
  const MSG *pMsg=GetCurrentMessage();
  // punt to the mainframe to deal with shortcuts in popups
  return AfxGetMainWnd()->SendMessage(pMsg->message,pMsg->wParam,pMsg->lParam);
}
