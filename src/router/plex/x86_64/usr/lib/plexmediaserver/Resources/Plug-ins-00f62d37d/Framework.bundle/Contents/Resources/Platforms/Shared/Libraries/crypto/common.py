""" crypto.common
    Common utility routines for crypto modules

    Copyright (c) 2002 by Paul A. Lambert
    Read LICENSE.txt for license information.
"""

def xorS(a,b):
    """ XOR two strings """
    assert len(a)==len(b)
    x = []
    for i in range(len(a)):
            x.append( chr(ord(a[i])^ord(b[i])))
    return ''.join(x)
      
def xor(a,b):
    """ XOR two strings """
    x = []
    for i in range(min(len(a),len(b))):
            x.append( chr(ord(a[i])^ord(b[i])))
    return ''.join(x)
