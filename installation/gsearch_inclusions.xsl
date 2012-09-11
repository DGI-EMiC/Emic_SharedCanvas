<?xml version="1.0" encoding="UTF-8"?>

<!--
    Document   : gsearch_inclusions.xsl
    Created on : September 7, 2012, 8:52 AM
    Author     : astanley
    Description:
        Purpose of transformation follows.
-->

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:output method="html"/>

    <!-- TODO customize transformation rules 
         syntax recommendation http://www.w3.org/TR/xslt 
    -->
  <xsl:template match="/">
      <!-- This snippet ensures that each datastream for each object is indexed -->

    <xsl:for-each select="//foxml:datastreamVersion[last()]">
      <field>
        <xsl:attribute name="name">hasDatastream</xsl:attribute>
        <xsl:value-of select="substring-before(@ID,'.')"/>
      </field>
    </xsl:for-each>
     <!-- associates OAC annotatins with targeted object  -->

    <xsl:for-each
            select="foxml:datastream[@ID='OAC']/foxml:datastreamVersion[last()]/foxml:xmlContent//rdf:Description/oac:constrains/@rdf:resource">
      <field>
        <xsl:attribute name="name">rdf.constrains</xsl:attribute>
        <xsl:value-of select="."/>
      </field>
    </xsl:for-each>

  </xsl:template>

</xsl:stylesheet>
