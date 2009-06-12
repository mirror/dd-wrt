<!-- {{{ Banner                 -->

<!-- =============================================================== -->
<!--                                                                 -->
<!--     stylesheet.sgml                                             -->
<!--                                                                 -->
<!--     Customize the nwalsh modular stylesheets.                   -->
<!--                                                                 -->
<!-- =============================================================== -->
<!-- ####ECOSGPLCOPYRIGHTBEGIN####                                          -->
<!-- This file is part of eCos, the Embedded Configurable Operating System. -->
<!-- Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.               -->
<!--                                                                        -->
<!-- eCos is free software; you can redistribute it and/or modify it under  -->
<!-- the terms of the GNU General Public License as published by the Free   -->
<!-- Software Foundation; either version 2 or (at your option) any later    -->
<!-- version.                                                               -->
<!--                                                                        -->
<!-- eCos is distributed in the hope that it will be useful, but WITHOUT ANY-->
<!-- WARRANTY; without even the implied warranty of MERCHANTABILITY or      -->
<!-- FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License  -->
<!-- for more details.                                                      -->
<!--                                                                        -->
<!-- You should have received a copy of the GNU General Public License along-->
<!-- with eCos; if not, write to the Free Software Foundation, Inc.,        -->
<!-- 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.                 -->
<!--                                                                        -->
<!-- As a special exception, if other files instantiate templates or use    -->
<!-- macros or inline functions from this file, or you compile this file and-->
<!-- link it with other works to produce a work based on this file, this    -->
<!-- file does not by itself cause the resulting work to be covered by the  -->
<!-- GNU General Public License. However the source code for this file must -->
<!-- still be made available in accordance with section (3) of the GNU      -->
<!-- General Public License.                                                -->
<!--                                                                        -->
<!-- This exception does not invalidate any other reasons why a work based  -->
<!-- on this file might be covered by the GNU General Public License.       -->
<!--                                                                        -->
<!-- Alternative licenses for eCos may be arranged by contacting            -->
<!-- Red Hat, Inc. at http://sources.redhat.com/ecos/ecos-license/          -->
<!-- ####ECOSGPLCOPYRIGHTEND####                                            -->
<!-- =============================================================== -->
<!-- #####DESCRIPTIONBEGIN####                                       -->
<!--                                                                 -->
<!-- Author(s):   bartv                                              -->
<!--              Based on cygnus-both.dsl by Mark Galassi           -->
<!-- Contact(s):  bartv                                              -->
<!-- Date:        2000/03/15                                         -->
<!-- Version:     0.01                                               -->
<!--                                                                 -->
<!-- ####DESCRIPTIONEND####                                          -->
<!-- =============================================================== -->

<!-- }}} -->

<!DOCTYPE style-sheet PUBLIC "-//James Clark//DTD DSSSL Style Sheet//EN" [
<!ENTITY % html "IGNORE">
<![%html;[
<!ENTITY % print "IGNORE">
<!ENTITY docbook.dsl PUBLIC "-//Norman Walsh//DOCUMENT DocBook HTML Stylesheet//EN" CDATA dsssl>
]]>
<!ENTITY % print "INCLUDE">
<![%print;[
<!ENTITY docbook.dsl PUBLIC "-//Norman Walsh//DOCUMENT DocBook Print Stylesheet//EN" CDATA dsssl>
]]>
]>

<style-sheet>
<style-specification id="print" use="docbook">
<style-specification-body> 

;; ====================
;; customize the print (PDF) A4 stylesheet
;; ====================

;; Set the paper parameters as per other eCos documentation
(define %page-width%   210mm)
(define %page-height%  297mm)
(define %left-margin%  0.75in)
(define %right-margin% 0.75in)

;; Assume that we are only producing books, a reasonable assumption
;; given the primary author :-)
(define %two-side% #t)

;; Use 12pt
;;(define %visual-acuity% "presbyopic")

;; Do not use graphics in admonitions, our documentation is supposed
;; to look boring :-(
(define %admon-graphics% #f)

;; Justified text please.
(define %default-quadding% 'justify)

;; A separate page for each man page please.
(define %refentry-new-page% #t)

;; The component writer's guide man pages do not describe functions
(define %refentry-functions% #f)

;; Program listings should use smaller font to fit on page width
(define %verbatim-size-factor% 0.84)


</style-specification-body>
</style-specification>
<external-specification id="docbook" document="docbook.dsl">
</style-sheet>

