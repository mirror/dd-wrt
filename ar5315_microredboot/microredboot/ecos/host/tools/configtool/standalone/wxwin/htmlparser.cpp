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
// htmlparser.cpp :
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   julians
// Contact(s):  julians
// Date:        2001/04/04
// Version:     $Id: htmlparser.cpp,v 1.6 2001/04/12 10:02:22 julians Exp $
// Purpose:
// Description: HTML parser/HTML Help file generator
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
#pragma implementation "htmlparser.h"
#endif

// Includes other headers for precompiled compilation
#include "ecpch.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "wx/textfile.h"
#include "wx/wfstream.h"

#include "ecutils.h"
#include "htmlparser.h"

/*
 * wxSimpleHtmlAttribute
 * Representation of an attribute
 */

wxSimpleHtmlParser::wxSimpleHtmlParser()
{
    m_topLevel = NULL;
    m_pos = 0;
}


wxSimpleHtmlParser::~wxSimpleHtmlParser()
{
    Clear();
}

bool wxSimpleHtmlParser::ParseFile(const wxString& filename)
{
    wxTextFile textFile;

    if (textFile.Open(filename))
    {
        wxString text;
        wxString line;
        int i;
        int count = textFile.GetLineCount();
        for (i = 0; i < count; i++)
        {
            if (i == 0)
                line = textFile.GetFirstLine();
            else
                line = textFile.GetNextLine();

            text += line;
            if (i != (count - 1))
                text += wxT("\n");
        }

#if 0
        for ( line = textFile.GetFirstLine(); !textFile.Eof(); line = textFile.GetNextLine() )
        {
            text += line;
            if (!textFile.Eof())
                text += wxT("\n");
        }
#endif

        return ParseString(text);
    }
    else
        return FALSE;
}

bool wxSimpleHtmlParser::ParseString(const wxString& str)
{
    Clear();

    m_pos = 0;
    m_text = str;
    m_length = str.Length();

    m_topLevel = new wxSimpleHtmlTag(wxT("TOPLEVEL"), wxSimpleHtmlTag_TopLevel);

    return ParseHtml(m_topLevel);
}

// Main recursive parsing function
bool wxSimpleHtmlParser::ParseHtml(wxSimpleHtmlTag* parent)
{
    while (!Eof())
    {
        EatWhitespace();
        if (IsComment())
        {
            ParseComment();
        }
        else if (IsDirective())
        {
            wxSimpleHtmlTag* tag = ParseDirective();
            if (tag)
                parent->AppendTag(tag);
        }
        else if (IsTagClose())
        {
            wxSimpleHtmlTag* tag = ParseTagClose();
            if (tag)
                parent->AppendTag(tag);
        }
        else if (IsTagStartBracket(GetChar(m_pos)))
        {
            wxSimpleHtmlTag* tag = ParseTagHeader();
            if (tag)
                parent->AppendTag(tag);
        }
        else
        {
            // Just a text string
            wxString text;
            ParseText(text);

            wxSimpleHtmlTag* tag = new wxSimpleHtmlTag(wxT("TEXT"), wxSimpleHtmlTag_Text);
            tag->SetText(text);
            parent->AppendTag(tag);
        }
    }
    return TRUE;
}

// Plain text, up until an angled bracket
bool wxSimpleHtmlParser::ParseText(wxString& text)
{
    while (!Eof() && GetChar(m_pos) != wxT('<'))
    {
        text += GetChar(m_pos);
        m_pos ++;
    }
    return TRUE;
}

wxSimpleHtmlTag* wxSimpleHtmlParser::ParseTagHeader()
{
    if (IsTagStartBracket(GetChar(m_pos)))
    {
        m_pos ++;
        EatWhitespace();

        wxString word;
        ReadWord(word, TRUE);

        EatWhitespace();

        wxSimpleHtmlTag* tag = new wxSimpleHtmlTag(word, wxSimpleHtmlTag_Open);

        ParseAttributes(tag);

        EatWhitespace();

        if (IsTagEndBracket(GetChar(m_pos)))
            m_pos ++;

        return tag;
    }
    else
        return NULL;
}

wxSimpleHtmlTag* wxSimpleHtmlParser::ParseTagClose()
{
    Matches(wxT("</"), TRUE);

    EatWhitespace();

    wxString word;
    ReadWord(word, TRUE);

    EatWhitespace();
    m_pos ++;

    wxSimpleHtmlTag* tag = new wxSimpleHtmlTag(word, wxSimpleHtmlTag_Close);
    return tag;
}

bool wxSimpleHtmlParser::ParseAttributes(wxSimpleHtmlTag* tag)
{
    // Parse attributes of a tag header until we reach >
    while (!IsTagEndBracket(GetChar(m_pos)) && !Eof())
    {
        EatWhitespace();

        wxString attrName, attrValue;

        if (IsString())
        {
            ReadString(attrName, TRUE);
            tag->AppendAttribute(attrName, wxEmptyString);
        }
        else if (IsNumeric(GetChar(m_pos)))
        {
            ReadNumber(attrName, TRUE);
            tag->AppendAttribute(attrName, wxEmptyString);
        }
        else
        {
            // Try to read an attribute name/value pair, or at least a name
            // without the value
            ReadLiteral(attrName, TRUE);
            EatWhitespace();

            if (GetChar(m_pos) == wxT('='))
            {
                m_pos ++;
                EatWhitespace();

                if (IsString())
                    ReadString(attrValue, TRUE);
                else if (!Eof() && !IsTagEndBracket(GetChar(m_pos)))
                    ReadLiteral(attrValue, TRUE);
            }
            if (!attrName.IsEmpty())
                tag->AppendAttribute(attrName, attrValue);
        }
    }
    return TRUE;
}

// e.g. <!DOCTYPE ....>
wxSimpleHtmlTag* wxSimpleHtmlParser::ParseDirective()
{
    Matches(wxT("<!"), TRUE);

    EatWhitespace();

    wxString word;
    ReadWord(word, TRUE);

    EatWhitespace();

    wxSimpleHtmlTag* tag = new wxSimpleHtmlTag(word, wxSimpleHtmlTag_Directive);

    ParseAttributes(tag);

    EatWhitespace();

    if (IsTagEndBracket(GetChar(m_pos)))
        m_pos ++;

    return tag;
}

bool wxSimpleHtmlParser::ParseComment()
{
    // Eat the comment tag start
    Matches(wxT("<!--"), TRUE);

    while (!Eof() && !Matches(wxT("-->"), TRUE))
    {
        m_pos ++;
    }

    return TRUE;
}

bool wxSimpleHtmlParser::EatWhitespace()
{
    while (!Eof() && IsWhitespace(GetChar(m_pos)))
        m_pos ++;
    return TRUE;
}

bool wxSimpleHtmlParser::EatWhitespace(int& pos)
{
    while (!Eof(pos) && IsWhitespace(GetChar(pos)))
        pos ++;
    return TRUE;
}

bool wxSimpleHtmlParser::ReadString(wxString& str, bool eatIt)
{
    int pos = m_pos;
    if (GetChar(pos) == (int) '"')
    {
        pos ++;
        while (!Eof(pos) && GetChar(pos) != (int) '"')
        {
            // TODO: how are quotes escaped in HTML?
            str += (wxChar) GetChar(pos);
            pos ++;
        }
        if (GetChar(pos) == (int) '"')
            pos ++;
        if (eatIt)
            m_pos = pos;
        return TRUE;
    }
    else
        return FALSE;
}

bool wxSimpleHtmlParser::ReadWord(wxString& str, bool eatIt)
{
    int pos = m_pos;

    if (!IsAlpha(GetChar(pos)))
        return FALSE;

    str += (wxChar) GetChar(pos) ;
    pos ++;

    while (!Eof(pos) && IsWordChar(GetChar(pos)))
    {
        str += (wxChar) GetChar(pos);
        pos ++;
    }
    if (eatIt)
        m_pos = pos;
    return TRUE;
}

bool wxSimpleHtmlParser::ReadNumber(wxString& str, bool eatIt)
{
    int pos = m_pos;

    if (!IsNumeric(GetChar(pos)))
        return FALSE;

    str += (wxChar) GetChar(pos) ;
    pos ++;

    while (!Eof(pos) && IsNumeric(GetChar(pos)))
    {
        str += (wxChar) GetChar(pos);
        pos ++;
    }
    if (eatIt)
        m_pos = pos;
    return TRUE;
}

// Could be number, string, whatever, but read up until whitespace or end of tag (but not a quoted string)
bool wxSimpleHtmlParser::ReadLiteral(wxString& str, bool eatIt)
{
    int pos = m_pos;

    while (!Eof(pos) && !IsWhitespace(GetChar(pos)) && !IsTagEndBracket(GetChar(pos)) && GetChar(pos) != wxT('='))
    {
        str += GetChar(pos);
        pos ++;
    }
    if (eatIt)
        m_pos = pos;
    return TRUE;
}

bool wxSimpleHtmlParser::IsTagClose()
{
    return Matches(wxT("</"));
}

bool wxSimpleHtmlParser::IsComment()
{
    return Matches(wxT("<!--"));
}

bool wxSimpleHtmlParser::IsDirective()
{
    return Matches(wxT("<!"));
}

bool wxSimpleHtmlParser::IsString()
{
    return (GetChar(m_pos) == (int) '"') ;
}

bool wxSimpleHtmlParser::IsWord()
{
    return (IsAlpha(GetChar(m_pos)));
}

bool wxSimpleHtmlParser::IsTagStartBracket(int ch)
{
    return (ch == wxT('<'));
}

bool wxSimpleHtmlParser::IsTagEndBracket(int ch)
{
    return (ch == wxT('>'));
}

bool wxSimpleHtmlParser::IsWhitespace(int ch)
{
    return ((ch == 13) || (ch == 10) || (ch == 32) || (ch == (int) '\t')) ;
}

bool wxSimpleHtmlParser::IsAlpha(int ch)
{
    return (wxIsalpha((wxChar) ch) != 0);
}

bool wxSimpleHtmlParser::IsWordChar(int ch)
{
    return (wxIsalpha((wxChar) ch) != 0 || ch == wxT('-') || ch == wxT('_') || IsNumeric(ch));
}

bool wxSimpleHtmlParser::IsNumeric(int ch)
{
    return (wxIsdigit((wxChar) ch) != 0 || ch == wxT('-') || ch == wxT('.')) ;
}

// Matches this string (case insensitive)
bool wxSimpleHtmlParser::Matches(const wxString& tok, bool eatIt)
{
    wxString text(m_text.Mid(m_pos, tok.Length()));
    bool success = (text.CmpNoCase(tok) == 0) ;
    if (success && eatIt)
    {
        m_pos += tok.Length();
    }
    return success;
}

// Safe way of getting a character
int wxSimpleHtmlParser::GetChar(size_t i) const
{
    if (i >= m_length)
        return -1;
    return m_text[i];
}

void wxSimpleHtmlParser::Clear()
{
    if (m_topLevel)
        delete m_topLevel;
    m_topLevel = NULL;
    m_text = wxEmptyString;
    m_pos = 0;
    m_length = 0;
}

// Write this file
void wxSimpleHtmlParser::Write(wxOutputStream& stream)
{
    if (m_topLevel)
        m_topLevel->Write(stream);
}

bool wxSimpleHtmlParser::WriteFile(wxString& filename)
{
    wxFileOutputStream fstream(filename);
    if (fstream.Ok())
    {
        Write(fstream);
        return TRUE;
    }
    else
        return FALSE;
}

/*
 * wxSimpleHtmlTag
 * Representation of a tag or chunk of text
 */

wxSimpleHtmlTag::wxSimpleHtmlTag(const wxString& tagName, int tagType)
{
    m_name = tagName;
    m_type = tagType;
    m_attributes = NULL;
    m_children = NULL;
    m_parent = NULL;
    m_next = NULL;
}

wxSimpleHtmlTag::~wxSimpleHtmlTag()
{
    ClearAttributes();
    ClearChildren();
}

//// Operations
void wxSimpleHtmlTag::ClearAttributes()
{
    if (m_attributes)
    {
        wxSimpleHtmlAttribute* attr = m_attributes;
        while (attr)
        {
            wxSimpleHtmlAttribute* next = attr->m_next;

            attr->m_next = NULL;
            delete attr;
            attr = next;
        }
        m_attributes = NULL;
    }
}

wxSimpleHtmlAttribute* wxSimpleHtmlTag::FindAttribute(const wxString& name) const
{
    wxSimpleHtmlAttribute* attr = m_attributes;
    while (attr)
    {
        if (attr->GetName().CmpNoCase(name) == 0)
        {
            return attr;
        }
        attr = attr->m_next;
    }
    return NULL;
}

void wxSimpleHtmlTag::AppendAttribute(const wxString& name, const wxString& value)
{
    wxSimpleHtmlAttribute* attr = new wxSimpleHtmlAttribute(name, value);
    if (m_attributes)
    {
        // Find tail
        wxSimpleHtmlAttribute* last = m_attributes;
        while (last->m_next)
            last = last->m_next;

        last->m_next = attr;
    }
    else
        m_attributes = attr;
}

void wxSimpleHtmlTag::ClearChildren()
{
    if (m_children)
    {
        wxSimpleHtmlTag* child = m_children;
        while (child)
        {
            wxSimpleHtmlTag* next = child->m_next;

            child->m_next = NULL;
            delete child;
            child = next;
        }
        m_children = NULL;
    }
}

void wxSimpleHtmlTag::AppendTag(wxSimpleHtmlTag* tag)
{
    if (m_children)
    {
        // Find tail
        wxSimpleHtmlTag* last = m_children;
        while (last->m_next)
            last = last->m_next;

        last->m_next = tag;
        tag->m_parent = this;
    }
    else
        m_children = tag;
}

// Gets the text from this tag and its descendants
wxString wxSimpleHtmlTag::GetTagText()
{
    wxString text;
    if (m_children)
    {
        wxSimpleHtmlTag* tag = m_children;
        while (tag)
        {
            text += tag->GetTagText();
            tag = tag->m_next;
        }
        return text;
    }
    else if (GetType() == wxSimpleHtmlTag_Text)
        return GetText();
    else
        return wxEmptyString;
}

int wxSimpleHtmlTag::GetAttributeCount() const
{
    int count = 0;
    wxSimpleHtmlAttribute* attr = m_attributes;
    while (attr)
    {
        count ++;
        attr = attr->m_next;
    }
    return count;
}

wxSimpleHtmlAttribute* wxSimpleHtmlTag::GetAttribute(int i) const
{
    int count = 0;
    wxSimpleHtmlAttribute* attr = m_attributes;
    while (attr)
    {
        if (count == i)
            return attr;
        count ++;
        attr = attr->m_next;
    }
    return NULL;
}

int wxSimpleHtmlTag::GetChildCount() const
{
    int count = 0;
    wxSimpleHtmlTag* tag = m_children;
    while (tag)
    {
        count ++;
        tag = tag->m_next;
    }
    return count;
}

bool wxSimpleHtmlTag::HasAttribute(const wxString& name, const wxString& value) const
{
    wxSimpleHtmlAttribute* attr = FindAttribute(name);

    return (attr && (attr->GetValue().CmpNoCase(value) == 0)) ;
}

bool wxSimpleHtmlTag::HasAttribute(const wxString& name) const
{
    return FindAttribute(name) != NULL ;
}

bool wxSimpleHtmlTag::GetAttributeValue(wxString& value, const wxString& attrName)
{
    wxSimpleHtmlAttribute* attr = FindAttribute(attrName);
    if (attr)
    {
        value = attr->GetValue();
        return TRUE;
    }
    else
        return FALSE;
}

// Search forward from this tag until we find a tag with this name & attribute 
wxSimpleHtmlTag* wxSimpleHtmlTag::FindTag(const wxString& tagName, const wxString& attrName)
{
    wxSimpleHtmlTag* tag = m_next;
    while (tag)
    {
        if (tag->NameIs(tagName) && tag->FindAttribute(attrName))
            return tag;

        tag = tag->m_next;
    }
    return NULL;
}

bool wxSimpleHtmlTag::FindTextUntilTagClose(wxString& text, const wxString& tagName)
{
    wxSimpleHtmlTag* tag = this;
    while (tag)
    {
        if (tag->GetType() == wxSimpleHtmlTag_Close && tag->NameIs(tagName))
            return TRUE;

        if (tag->GetType() == wxSimpleHtmlTag_Text)
            text += tag->GetText();

        tag = tag->m_next;
    }
    return TRUE;
}


wxSimpleHtmlTag* wxSimpleHtmlTag::GetChild(int i) const
{
    int count = 0;
    wxSimpleHtmlTag* tag = m_children;
    while (tag)
    {
        if (count == i)
            return tag;

        count ++;
        tag = tag->m_next;
    }
    return NULL;
}

void wxSimpleHtmlTag::Write(wxOutputStream& stream)
{
    switch (GetType())
    {
    case wxSimpleHtmlTag_Text:
        {
            stream << m_text;
            break;
        }
    case wxSimpleHtmlTag_Open:
        {
            stream << "<" << m_name;
            if (GetAttributeCount() > 0)
                stream << " ";
            int i;
            for (i = 0; i < GetAttributeCount(); i++)
            {
                wxSimpleHtmlAttribute* attr = GetAttribute(i);
                attr->Write(stream);
                if (i < GetAttributeCount() - 1)
                    stream << " ";
            }
            stream << ">\n";
            break;
        }
    case wxSimpleHtmlTag_Directive:
        {
            stream << "<!" << m_name << " ";
            int i;
            for (i = 0; i < GetAttributeCount(); i++)
            {
                wxSimpleHtmlAttribute* attr = GetAttribute(i);
                attr->Write(stream);
                if (i < GetAttributeCount() - 1)
                    stream << " ";
            }
            stream << ">\n";
            break;
        }
    case wxSimpleHtmlTag_Close:
        {
            stream << "</" << m_name << ">\n";
            break;
        }
    default:
        {
            break;
        }
    }
    wxSimpleHtmlTag* tag = m_children;
    while (tag)
    {
        tag->Write(stream);
        tag = tag->m_next;
    }

}

void wxSimpleHtmlAttribute::Write(wxOutputStream& stream)
{
    if (m_value.IsEmpty())
        stream << m_name;
    else
    {
        stream << m_name;
        stream << "=\"";
        stream << m_value;
        stream << "\"";
    }
}
