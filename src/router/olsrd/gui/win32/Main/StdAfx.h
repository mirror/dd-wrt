#ifdef _WIN32

// stdafx.h : Include-Datei f�r Standard-System-Include-Dateien,
//  oder projektspezifische Include-Dateien, die h�ufig benutzt, aber
//      in unregelm��igen Abst�nden ge�ndert werden.
//

#if !defined(AFX_STDAFX_H__D43BD17B_AEC0_43A5_9F3B_1DAA99152E38__INCLUDED_)
#define AFX_STDAFX_H__D43BD17B_AEC0_43A5_9F3B_1DAA99152E38__INCLUDED_

#if defined _MSC_VER && _MSC_VER > 1000
#pragma once
#endif /* defined _MSC_VER && _MSC_VER > 1000 */

#define VC_EXTRALEAN            // Selten verwendete Teile der Windows-Header nicht einbinden

#include <afxwin.h>             // MFC-Kern- und -Standardkomponenten
#include <afxext.h>             // MFC-Erweiterungen
#include <afxdtctl.h>           // MFC-Unterst�tzung f�r allgemeine Steuerelemente von Internet Explorer 4
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC-Unterst�tzung f�r g�ngige Windows-Steuerelemente
#endif /* _AFX_NO_AFXCMN_SUPPORT */

#include <afxmt.h>
#include <afxtempl.h>
#include <winsock2.h>
#include <iphlpapi.h>

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ f�gt unmittelbar vor der vorhergehenden Zeile zus�tzliche Deklarationen ein.

#endif /* !defined(AFX_STDAFX_H__D43BD17B_AEC0_43A5_9F3B_1DAA99152E38__INCLUDED_) */

#endif /* _WIN32 */

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
