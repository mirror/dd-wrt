#
#  Plex Media Framework
#  Copyright (C) 2008-2009 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

"""
  Functions for handling XML and HTML content.
"""
from lxml import etree, html
from lxml.html.soupparser import fromstring
import HTTP, Plugin, Log

####################################################################################################

def Element(name):
  """
    Creates a new Element object with the given name.
    
    The Element class is part of ElementTree. More information can be found at:
    U{http://codespeak.net/lxml/api/elementtree.ElementTree.Element-class.html}

    @param name: The name to use for the new Element
    @type name: string
    @return: Element
  """
  return etree.Element(name)
  
def ElementWithText(name, text):
  """
    Creates a new Element object with the given name containing the given text.
    
    @param name: The name to use for the new Element
    @type name: string
    @param text: The text to the new Element will contain
    @type text: string
    @return: Element
  """
  el = Element(name)
  el.text = text
  return el

####################################################################################################
  
def ElementFromURL(url, use_html_parser=False):
  """
    Creates a new Element object with the response from the given URL.
    
    @param url: The URL used in the request
    @type url: string
    @param use_html_parser: Specifies whether to parse the response as HTML instead of XML
    @type use_html_parser: boolean
    @return: Element
  """
  text = HTTP.Get(url)
  if text is not None:
    Log.Add('(Framework) Request to %s return %d bytes' % (url, len(text)))
    if use_html_parser:
      try:
        root = html.fromstring(text)
        test = html.tostring(root, encoding=unicode)
        return root
      except:
        return fromstring(text)
    else:
      return etree.fromstring(text)
  else:
    return None

####################################################################################################

def ElementFromString(str, use_html_parser=False):
  """
    Creates a new Element object from the given XML string.
    
    @param str: The string to convert
    @type str: string
    @return: Element
  """
  if str is None: return None
    
  if use_html_parser:
    try:
      root = html.fromstring(str)
      test = html.tostring(root, encoding=unicode)
      return root
    except:
      return fromstring(str)
  else:
    return etree.fromstring(str)
  
####################################################################################################
  
def ElementFromFile(path, use_html_parser=False):
  """
    Creates a new Element object from an XML file at the given path.
    
    @param path: The path to the XML file
    @type path: string
    @return: Element
  """
  #text = open(path)
  
  tfile = open(path)
  text = tfile.read()
  tfile.close()
  
  if text is not None:
    if use_html_parser:
      try:
        root = html.fromstring(text)
        test = html.tostring(root, encoding=unicode)
        return root
      except:
        return fromstring(text)
    else:
      return etree.fromstring(text)
  else:
    return None

####################################################################################################

def ElementToString(el):
  """
    Convert a given Element object to an XML string.
    
    @param el: The Element to convert
    @type el: Element
    @return: string
  """
  return etree.tostring(el, pretty_print=Plugin.Debug, encoding='UTF-8')

####################################################################################################

def ElementToFile(el, path):
  """
    Save an Element object to an XML file at the given path.
    
    @param el: The Element to save
    @type el: Element
    @param path: The path to the file
    @type path: string
    @return: string
  """
  etree.ElementTree(el).write(path, pretty_print=True)

####################################################################################################

def TextFromElement(el):
  """
    Return the text from a given Element, or None if the element does not exist.
    
    @param el: The Element to return the text from
    @type el: Element
    @return: string
  """
  if el is None: return None
  return el.text

####################################################################################################

