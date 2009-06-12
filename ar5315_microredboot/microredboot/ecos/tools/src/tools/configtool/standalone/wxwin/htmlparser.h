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
// Author(s): 	julians
// Contact(s):	julians
// Date:		2001/04/04
// Version:		0.01
// Purpose:	
// Description:	HTML parser/HTML Help file generator
// Requires:	
// Provides:	
// See also:    
// Known bugs:	
// Usage:	
//
//####DESCRIPTIONEND####
//  
//===========================================================================

#ifndef _EC_HTMLPARSER_H_
#define _EC_HTMLPARSER_H_

#ifdef __GNUG__
#pragma interface "htmlparser.cpp"
#endif

#include "wx/module.h"
#include "wx/stream.h"

/*

 So how are going to represent it: compare with my Latex parser.
 This generates a hierarchy because it respects the hierarchical nature of the Latex
 commands. However, we don't _have_ to do that, we can make it linear, e.g.

 tag-with-attributes text-chunk end-tag-with-attributes tag-with-attributes text-chunk

 Otherwise, we need knowledge about HTML tags to parse hierarchically. This wouldn't be hard.
 Would need to specify which tags have open/close parts, which don't, and for which it's optional
 (such as <P>).


 */

/*
 * wxSimpleHtmlAttribute
 * Representation of an attribute
 */

class wxSimpleHtmlAttribute
{
    friend class wxSimpleHtmlTag;
public:
    wxSimpleHtmlAttribute(const wxString& name, const wxString& value)
    {
        m_name = name; m_value = value; m_next = NULL;
    }
//// Operations

    // Write this attribute
    void Write(wxOutputStream& stream);

//// Accessors
    const wxString& GetName() const { return m_name; }
    const wxString& GetValue() const { return m_value; }

    wxSimpleHtmlAttribute* GetNextAttribute() { return m_next; }
    void SetNextAttribute(wxSimpleHtmlAttribute* attr) { m_next = attr; }

    bool HasName(const wxString& name) const { return (0 == m_name.CmpNoCase(name)); }
    bool HasValue(const wxString& val) const { return (0 == m_value.CmpNoCase(val)); }

private:
    wxString                m_name;
    wxString                m_value;
    wxSimpleHtmlAttribute*  m_next;
};


/*
 * wxSimpleHtmlTag
 * Representation of a tag or chunk of text
 */

enum { wxSimpleHtmlTag_Text, wxSimpleHtmlTag_TopLevel, wxSimpleHtmlTag_Open, wxSimpleHtmlTag_Close, wxSimpleHtmlTag_Directive  };

class wxSimpleHtmlTag
{
public:
    wxSimpleHtmlTag(const wxString& tagName, int tagType);
    ~wxSimpleHtmlTag();

//// Operations
    void ClearAttributes();
    wxSimpleHtmlAttribute* FindAttribute(const wxString& name) const ;
    void AppendAttribute(const wxString& name, const wxString& value);
    void ClearChildren();
    void AppendTag(wxSimpleHtmlTag* tag);
    // Write this tag
    void Write(wxOutputStream& stream);

    // Gets the text from this tag and its descendants
    wxString GetTagText();

//// Accessors
    const wxString& GetName() const { return m_name; }
    void SetName(const wxString& name) { m_name = name; }

    int GetType() const { return m_type; }
    void SetType(int t) { m_type = t; }

    // If type is wxSimpleHtmlTag_Text, m_text will contain some text.
    const wxString& GetText() const { return m_text; }
    void SetText(const wxString& text) { m_text = text; }

    wxSimpleHtmlAttribute* GetFirstAttribute() { return m_attributes; }
    void SetFirstAttribute(wxSimpleHtmlAttribute* attr) { m_attributes = attr; }

    int GetAttributeCount() const ;
    wxSimpleHtmlAttribute* GetAttribute(int i) const ;

    wxSimpleHtmlTag* GetChildren() const { return m_children; }
    void SetChildren(wxSimpleHtmlTag* children) { m_children = children; }

    wxSimpleHtmlTag* GetParent() const { return m_parent; }
    void SetParent(wxSimpleHtmlTag* parent) { m_parent = parent; }
    int GetChildCount() const;
    wxSimpleHtmlTag*    GetChild(int i) const;
    wxSimpleHtmlTag*    GetNext() const { return m_next; }

//// Convenience accessors & search functions
    bool NameIs(const wxString& name) { return (m_name.CmpNoCase(name) == 0); }
    bool HasAttribute(const wxString& name, const wxString& value) const;
    bool HasAttribute(const wxString& name) const;
    bool GetAttributeValue(wxString& value, const wxString& attrName);

    // Search forward from this tag until we find a tag with this name & attribute 
    wxSimpleHtmlTag* FindTag(const wxString& tagName, const wxString& attrName);

    // Gather the text until we hit the given close tag
    bool FindTextUntilTagClose(wxString& text, const wxString& tagName);

private:
    wxString                m_name;
    int                     m_type;
    wxString                m_text;
    wxSimpleHtmlAttribute*  m_attributes;

    // List of children
    wxSimpleHtmlTag*        m_children;
    wxSimpleHtmlTag*        m_next; // Next sibling
    wxSimpleHtmlTag*        m_parent;
};

/*
 * wxSimpleHtmlParser
 * Simple HTML parser, for such tasks as scanning HTML for keywords, contents, etc.
 */

class wxSimpleHtmlParser : public wxObject
{
    
public:
    wxSimpleHtmlParser();
    ~wxSimpleHtmlParser();

//// Operations
    bool ParseFile(const wxString& filename);
    bool ParseString(const wxString& str);
    void Clear();
    // Write this file
    void Write(wxOutputStream& stream);
    bool WriteFile(wxString& filename);

//// Helpers

    // Main recursive parsing function
    bool ParseHtml(wxSimpleHtmlTag* parent);

    wxSimpleHtmlTag* ParseTagHeader();
    wxSimpleHtmlTag* ParseTagClose();
    bool ParseAttributes(wxSimpleHtmlTag* tag);
    wxSimpleHtmlTag* ParseDirective(); // e.g. <!DOCTYPE ....>
    bool ParseComment(); // Throw away comments
    // Plain text, up until an angled bracket
    bool ParseText(wxString& text);

    bool EatWhitespace(); // Throw away whitespace
    bool EatWhitespace(int& pos); // Throw away whitespace: using 'pos'
    bool ReadString(wxString& str, bool eatIt = FALSE);
    bool ReadWord(wxString& str, bool eatIt = FALSE);
    bool ReadNumber(wxString& str, bool eatIt = FALSE);
    // Could be number, string, whatever, but read up until whitespace.
    bool ReadLiteral(wxString& str, bool eatIt = FALSE);

    bool IsDirective();
    bool IsComment();
    bool IsString();
    bool IsWord();
    bool IsTagClose();
    bool IsTagStartBracket(int ch);
    bool IsTagEndBracket(int ch);
    bool IsWhitespace(int ch);
    bool IsAlpha(int ch);
    bool IsWordChar(int ch);
    bool IsNumeric(int ch);

    // Matches this string (case insensitive)
    bool Matches(const wxString& tok, bool eatIt = FALSE) ;
    bool Eof() const { return (m_pos >= m_length); }
    bool Eof(int pos) const { return (pos >= m_length); }

    void SetPosition(int pos) { m_pos = pos; }


//// Accessors
    wxSimpleHtmlTag* GetTopLevelTag() const { return m_topLevel; }

    // Safe way of getting a character
    int GetChar(size_t i) const;
    
private:

    wxSimpleHtmlTag*    m_topLevel;
    int                 m_pos;    // Position in string
    int                 m_length; // Length of string
    wxString            m_text;   // The actual text

};

/*
 * wxSimpleHtmlTagSpec
 * Describes a tag, and what type it is.
 * wxSimpleHtmlModule will initialise/cleanup a list of these, one per tag type
 */

#if 0
class wxSimpleHtmlTagSpec : public wxObject
{
    
public:
    wxSimpleHtmlTagSpec(const wxString& name, int type);

//// Operations
    static void AddTagSpec(wxSimpleHtmlTagSpec* spec);
    static void Clear();

//// Accessors
    const wxString& GetName() const { return m_name; }
    int GetType() const { return m_type; }

private:

    wxString    m_name;
    int         m_type;

    static wxList* sm_tagSpecs;
};

/*
 * wxSimpleHtmlModule
 * Responsible for init/cleanup of appropriate data structures
 */

class wxSimpleHtmlModule : public wxModule
{
DECLARE_DYNAMIC_CLASS(wxSimpleHtmlModule)

public:
    wxSimpleHtmlModule() {};

    bool OnInit() ;
    void OnExit() ;
};
#endif

#endif
