/**
 * @file validation_xml.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief performance test - validating YANG data via DSDL tools.
 *
 * Copyright (c) 2016 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <libxml/relaxng.h>
#include <libxslt/transform.h>
#include <libxslt/xsltInternals.h>

int main(int argc, char *argv[])
{
	xmlDocPtr sdoc, ddoc;
	xmlDocPtr sch_result;

	xmlRelaxNGParserCtxtPtr rng_ctxt;
	xmlRelaxNGPtr rng_schema = NULL;
	xmlRelaxNGValidCtxtPtr rng = NULL;
	xsltStylesheetPtr sch = NULL;

	if (argc < 5) {
                fprintf(stderr, "Usage: %s model.yin data.xml model-config.rng model-schematron.xsl\n", argv[0]);
                return 1;
        }

	/* schema */
	sdoc = xmlReadFile(argv[1], NULL, 0);
	if (!sdoc) {
		fprintf(stderr, "Failed to load data model\n");
		return 1;
	}

	/* data */
	ddoc = xmlReadFile(argv[2], NULL, 0);
	if (!sdoc) {
		fprintf(stderr, "Failed to load data\n");
		return 1;
	}

	/* validate */
	rng_ctxt = xmlRelaxNGNewParserCtxt(argv[3]);
	rng_schema = xmlRelaxNGParse(rng_ctxt);
	rng = xmlRelaxNGNewValidCtxt(rng_schema);

	sch = xsltParseStylesheetFile(BAD_CAST argv[4]);

	xmlRelaxNGValidateDoc(rng, ddoc);		
	sch_result = xsltApplyStylesheet(sch, ddoc, NULL);

	xmlRelaxNGFreeParserCtxt(rng_ctxt);
	xmlRelaxNGFree(rng_schema);
	xmlRelaxNGFreeValidCtxt(rng);
	xsltFreeStylesheet(sch);
	xmlFreeDoc(sch_result);

	xmlFreeDoc(ddoc);
	xmlFreeDoc(sdoc);
	
	return 0;
}

