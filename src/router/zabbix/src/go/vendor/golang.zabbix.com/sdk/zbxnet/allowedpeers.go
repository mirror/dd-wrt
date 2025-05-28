/*
** Copyright (C) 2001-2025 Zabbix SIA
**
** Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
** documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
** rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
** permit persons to whom the Software is furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all copies or substantial portions
** of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
** WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
** COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
** TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
**/

package zbxnet

import (
	"fmt"
	"net"
	"regexp"
	"strings"
)

// AllowedPeers is preparsed content of field Server
type AllowedPeers struct {
	ips   []net.IP
	nets  []*net.IPNet
	names []string
}

// GetAllowedPeers is parses the Server field
func GetAllowedPeers(servers string) (allowedPeers *AllowedPeers, err error) {
	const duplicateErrorMessage = "cannot parse the \"Server\" parameter: address \"%s\" specified more than once"
	const regexDNSString = `^([a-zA-Z0-9_]{1}[a-zA-Z0-9_-]{0,62}){1}(\.[a-zA-Z0-9_]{1}[a-zA-Z0-9_-]{0,62})*[\._]?$`

	ap := &AllowedPeers{}

	if servers != "" {
		opts := strings.Split(servers, ",")
		for _, o := range opts {
			peer := strings.Trim(o, " \t")
			if _, peerNet, err := net.ParseCIDR(peer); nil == err {
				if ap.isPresent(peerNet) {
					return &AllowedPeers{}, fmt.Errorf(duplicateErrorMessage, peer)
				}

				ap.nets = append(ap.nets, peerNet)
				maskLeadSize, maskTotalOnes := peerNet.Mask.Size()
				if 0 == maskLeadSize && 128 == maskTotalOnes {
					_, peerNet, _ = net.ParseCIDR("0.0.0.0/0")
					if !ap.isPresent(peerNet) {
						ap.nets = append(ap.nets, peerNet)
					}
				}
			} else if peerip := net.ParseIP(peer); nil != peerip {
				if ap.isPresent(peerip) {
					return &AllowedPeers{}, fmt.Errorf(duplicateErrorMessage, peer)
				}

				ap.ips = append(ap.ips, peerip)
			} else {
				if ap.isPresent(peer) {
					return &AllowedPeers{}, fmt.Errorf(duplicateErrorMessage, peer)
				}

				regexDNS := regexp.MustCompile(regexDNSString)
				if !regexDNS.MatchString(peer) {
					return &AllowedPeers{}, fmt.Errorf("invalid \"Server\" configuration: incorrect address"+
						" parameter: \"%s\"", peer)
				}

				ap.names = append(ap.names, peer)
			}
		}
	}

	return ap, nil
}

// CheckPeer validate incoming connection peer
func (ap *AllowedPeers) CheckPeer(ip net.IP) bool {
	if ap.checkNetIP(ip) {
		return true
	}

	for _, nameAllowed := range ap.names {
		if ips, err := net.LookupHost(nameAllowed); nil == err {
			for _, ipPeer := range ips {
				ipAllowed := net.ParseIP(ipPeer)
				if ipAllowed.Equal(ip) {
					return true
				}
			}
		}
	}

	return false
}

func (ap *AllowedPeers) isPresent(value interface{}) bool {
	switch v := value.(type) {
	case *net.IPNet:
		for _, va := range ap.nets {
			maskLeadSize, _ := va.Mask.Size()
			maskLeadSizeNew, _ := v.Mask.Size()
			if maskLeadSize <= maskLeadSizeNew && va.Contains(v.IP) {
				return true
			}
		}
	case net.IP:
		if ap.checkNetIP(v) {
			return true
		}
	case string:
		for _, v := range ap.names {
			if v == value {
				return true
			}
		}
	}

	return false
}

func (ap *AllowedPeers) checkNetIP(ip net.IP) bool {
	for _, netAllowed := range ap.nets {
		if netAllowed.Contains(ip) {
			return true
		}
	}
	for _, ipAllowed := range ap.ips {
		if ipAllowed.Equal(ip) {
			return true
		}
	}
	return false
}
