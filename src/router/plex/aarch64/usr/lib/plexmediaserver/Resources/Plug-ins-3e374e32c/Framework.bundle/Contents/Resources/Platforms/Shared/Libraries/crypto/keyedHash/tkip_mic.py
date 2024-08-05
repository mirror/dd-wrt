""" crypto.keyedHash.tkip_mic

    A reference implementation of the TKIP Message Integrety Chek (MIC)
    that is defined in IEEE 802.11i

    The MIC is based on Michael, a 64-bit MIC, with a design strength of 20 bits.

    (c) 2002 Paul A. Lambert
"""
from crypto.keyedHash.michael import Michael

class TKIP_MIC(Michael):
    """ The TKIP MIC Calculation for IEEE 802.11 TGi
        This MIC algorithm uses the Michael Message Integrity Check (MIC)
        and incorporates the DA, SA, priority and padding as
        part of the MIC calculation
    """
    def __init__(self, key=None, version='D3'):
        """ """
        self.version = version
        Michael.__init__(self,key)

    def hash(self, sa, da, priority, msduData ):
        """ The TKIP MIC appends sa, da and priority to msduData
            and uses the result in the Michael keyed hash
            to create an 8 octet MIC value
        """
        assert( 0 <= priority <= 15 ), 'Priority must be 4 bit value'
        assert( (len(sa)==6) and (len(da)==6) ), 'Addresses must be 6 octets'

        if self.version == 'D3':
            micData = da + sa + chr(priority) + 3*chr(0) + msduData
        elif self.version == 'D2':
            micData = da + sa + msduData
        else:
            raise 'bad version'

        return Michael.hash(self, micData)


