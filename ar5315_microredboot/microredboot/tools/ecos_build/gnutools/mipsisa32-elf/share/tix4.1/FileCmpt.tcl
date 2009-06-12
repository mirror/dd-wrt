# FileCmpt.tcl --
#
#	File access portibility routines.
#
# Copyright (c) 1996, Expert Interface Technologies
#
# See the file "license.terms" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#


# Internal file names
# (1) Idempotent: [tixFileIntName $intName] == $intName
# (2) Does not contain "~", "..", "."
# (3) All DOS type C:foo will be translated to absoulte path such as
#     /\C:\windows\foo
# (4) Does not contail trailing "/" or "\\" characters
#

proc tixFileResolveName {nativeName {defParent ""}} {
    if {$defParent != ""} {
	return [tixNativeName [tixFileIntName $nativeName [tixFileIntName $defParent]]]
    } else {
        return [tixNativeName [tixFileIntName $nativeName]]
    }
}

proc tixNSubFolder {parent sub} {
    return [tixNativeName [tixSubFolder \
	[tixFileIntName $parent] [tixFileIntName $sub]]]
}
