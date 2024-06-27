<?xml version="1.0" encoding="utf-8"?>

<!-- deprecated.xsl
     Copyright (C) 2003 Free Software Foundation, Inc.
     
     This file is part of GNU Classpath.
     
     GNU Classpath is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2, or (at your option)
     any later version.
      
     GNU Classpath is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     General Public License for more details.
     
     You should have received a copy of the GNU General Public License
     along with GNU Classpath; see the file COPYING.  If not, write to the
     Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
     02111-1307 USA.
     -->

<!-- Creates the deprecation information page for HTML documentation.
     -->

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:gjdoc="http://www.gnu.org/software/cp-tools/gjdocxml"
  xmlns:html="http://www.w3.org/TR/REC-html40"
  xmlns="http://www.w3.org/TR/REC-html40">

  <xsl:include href="html_common.xsl"/>

  <xsl:output method="xml"
    encoding="utf-8"
    doctype-public="-//W3C//DTD HTML 4.01 Transitional//EN"
    doctype-system="http://www.w3.org/TR/html4/loose.dtd"
    indent="no"/>

  <xsl:strip-space elements="*"/>


  <xsl:template match="/">

    <xsl:for-each select="/gjdoc:rootdoc/gjdoc:specifiedpackage">
      <xsl:variable name="v_packagename" select="@name"/>
      <xsl:for-each select="/gjdoc:rootdoc/gjdoc:packagedoc[@name=$v_packagename]">
        <xsl:if test="not(gjdoc:tags/gjdoc:tag[gjdoc:kind='@serial' and normalize-space(text())='exclude'])">
          <xsl:if test="../gjdoc:classdoc[gjdoc:containingPackage/@name=$v_packagename and (gjdoc:implements/@qualifiedtypename='java.io.Serializable' or gjdoc:superimplements='java.io.Serializable')]">
            <h1>Package <xsl:value-of select="$v_packagename"/></h1>
            <xsl:for-each select="../gjdoc:classdoc[gjdoc:containingPackage/@name=$v_packagename and (gjdoc:implements/@qualifiedtypename='java.io.Serializable' or gjdoc:superimplements='java.io.Serializable')]">
              <h2>Class <xsl:value-of select="@name"/> extends <xsl:value-of select="gjdoc:superclass/@qualifiedtypename"/></h2>
              <xsl:variable name="v_sub_xml_filename" select="concat(@qualifiedtypename,'.xml')"/>
              <xsl:variable name="v_currentclass" select="@qualifiedtypename"/>
              <xsl:for-each select="document($v_sub_xml_filename,/gjdoc:rootdoc)//gjdoc:classdoc[@qualifiedtypename=$v_currentclass]/@qualifiedtypename/..">
                <xsl:if test="gjdoc:fielddoc[@name='serialVersionUID' and gjdoc:isFinal and gjdoc:isStatic and gjdoc:type/@typename='long']">
                  <p><b>serialVersionUID: </b>?</p>
                </xsl:if>
                <xsl:if test="gjdoc:fielddoc[not(gjdoc:isTransient or gjdoc:isSerialExclude)]">
                  <h3>Serialized Fields</h3>
                  <xsl:for-each select="gjdoc:fielddoc[not(gjdoc:isTransient) and not(gjdoc:isSerialExclude)]">
                    <h4><xsl:value-of select="@name"/></h4>

                    <p><code><xsl:value-of select="gjdoc:type/@typename"/><xsl:text> </xsl:text><b><xsl:value-of select="@name"/></b></code></p>
                  </xsl:for-each>
                </xsl:if>
              </xsl:for-each>
            </xsl:for-each>
          </xsl:if>
        </xsl:if>
      </xsl:for-each>
    </xsl:for-each>
    
  </xsl:template>

</xsl:stylesheet>
