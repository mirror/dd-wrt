""" binascii_plus

    Enhanced hex packet input and display

    Copyright © (c) 2002 by Paul A. Lambert
    Read LICENSE.txt for license information.
"""
from   binascii import a2b_hex, b2a_hex
import string

viewable = string.letters+string.digits+string.punctuation+' '

def a2b_p(s):
    """ Convert ascii string of hex bytes to a binary string
        this conversion allows any type of white space"""
    s= ''.join(string.split(s))
    return a2b_hex(s)

#---------------

def b2a(bString, separator=' '):
    """ Simple conversion of binary string to
        hex digits with default space separator """
    return string.join(['%.2x'%ord(c) for c in bString], separator)

def b2a_p(binary_string, frnt='        '):
    """ convert a binary string to formatted ascii hex output """
    return b2a_pt(binary_string, frnt=frnt, text_trailer=None)

def b2a_pt(binary_string, frnt='        ', gap='   ', text_trailer=1):
    """ convert a binary string to pretty ascii hex output
        Optional display of ascii text provided at end of hex dump """
    ascii_byte_list =['%.2x'%ord(c) for c in binary_string]
    pretty_list = []
    trailer = []
    for i in range(len(ascii_byte_list)):
        if i != 0:
            if   i % 16 == 0:                           # add trailer + \n every 16 bytes
                if text_trailer != None:
                    pretty_list += [gap] + trailer
                    trailer = []
                pretty_list += '\n'+frnt
            elif i %  8 == 0: pretty_list += '  '       # add an extra space every 8 bytes
            else:             pretty_list += ' '        # space between bytes
        else:
            pretty_list += '\n'+frnt
        pretty_list += ascii_byte_list[i]
        if   string.find(viewable,chr(int(ascii_byte_list[i],16))) >= 0: # if printable and not white space
            trailer += chr(int(ascii_byte_list[i],16))
        else:
            trailer += '.'
    if text_trailer != None:
        nabs = (16-(len(ascii_byte_list)%16))%16
        if nabs>7 : pretty_list += ' '
        pretty_list += nabs*'   '+gap+''.join(trailer)
    return ''.join(pretty_list)

def b2a_pter(s, frnt='        ', gap='   ', text_trailer=None):
    """ convert to pretty display or leave as string if printable and short """
    if len(s)>50: return b2a_pt(s,'        ',gap=gap,text_trailer=text_trailer)
    for char in s:
        if string.find(viewable,char) >= 0: # viewable?
            pass
        else:
            return b2a_pt(s,frnt,gap=gap,text_trailer=text_trailer)
    return '\n'+frnt+'"'+s+'"'







