/*
 * pagehead.c
 *
 * Copyright (C) 2005 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 */

static void show_snowflakes(webs_t wp)
{
	websWrite(
		wp,
		"<style>"
		".snowflake {"
		"  color: #fff;"
		"  font-size: 1em;"
		"  font-family: Arial, sans-serif;"
		"  text-shadow: 0 0 5px #000;"
		"}"
		".snowflake,.snowflake .inner{animation-iteration-count:infinite;animation-play-state:running}@keyframes snowflakes-fall{0%%{transform:translateY(0)}100%%{transform:translateY(110vh)}}@keyframes snowflakes-shake{0%%,100%%{transform:translateX(0)}50%%{transform:translateX(80px)}}.snowflake{position:fixed;top:-10%%;z-index:9999;-webkit-user-select:none;user-select:none;cursor:default;animation-name:snowflakes-shake;animation-duration:3s;animation-timing-function:ease-in-out}.snowflake .inner{animation-duration:10s;animation-name:snowflakes-fall;animation-timing-function:linear}.snowflake:nth-of-type(0){left:1%%;animation-delay:0s}.snowflake:nth-of-type(0) .inner{animation-delay:0s}.snowflake:first-of-type{left:10%%;animation-delay:1s}.snowflake:first-of-type .inner,.snowflake:nth-of-type(8) .inner{animation-delay:1s}.snowflake:nth-of-type(2){left:20%%;animation-delay:.5s}.snowflake:nth-of-type(2) .inner,.snowflake:nth-of-type(6) .inner{animation-delay:6s}.snowflake:nth-of-type(3){left:30%%;animation-delay:2s}.snowflake:nth-of-type(11) .inner,.snowflake:nth-of-type(3) .inner{animation-delay:4s}.snowflake:nth-of-type(4){left:40%%;animation-delay:2s}.snowflake:nth-of-type(10) .inner,.snowflake:nth-of-type(4) .inner{animation-delay:2s}.snowflake:nth-of-type(5){left:50%%;animation-delay:3s}.snowflake:nth-of-type(5) .inner{animation-delay:8s}.snowflake:nth-of-type(6){left:60%%;animation-delay:2s}.snowflake:nth-of-type(7){left:70%%;animation-delay:1s}.snowflake:nth-of-type(7) .inner{animation-delay:2.5s}.snowflake:nth-of-type(8){left:80%%;animation-delay:0s}.snowflake:nth-of-type(9){left:90%%;animation-delay:1.5s}.snowflake:nth-of-type(9) .inner{animation-delay:3s}.snowflake:nth-of-type(10){left:25%%;animation-delay:0s}.snowflake:nth-of-type(11){left:65%%;animation-delay:2.5s}"
		"</style>"
		"<div class=\"snowflakes\" aria-hidden=\"true\">"
		"  <div class=\"snowflake\">"
		"    <div class=\"inner\">❅</div>"
		"  </div>"
		"  <div class=\"snowflake\">"
		"    <div class=\"inner\">❅</div>"
		"  </div>"
		"  <div class=\"snowflake\">"
		"    <div class=\"inner\">❅</div>"
		"  </div>"
		"  <div class=\"snowflake\">"
		"    <div class=\"inner\">❅</div>"
		"  </div>"
		"  <div class=\"snowflake\">"
		"    <div class=\"inner\">❅</div>"
		"  </div>"
		"  <div class=\"snowflake\">"
		"    <div class=\"inner\">❅</div>"
		"  </div>"
		"  <div class=\"snowflake\">"
		"    <div class=\"inner\">❅</div>"
		"  </div>"
		"  <div class=\"snowflake\">"
		"    <div class=\"inner\">❅</div>"
		"  </div>"
		"  <div class=\"snowflake\">"
		"    <div class=\"inner\">❅</div>"
		"  </div>"
		"  <div class=\"snowflake\">"
		"    <div class=\"inner\">❅</div>"
		"  </div>"
		"  <div class=\"snowflake\">"
		"    <div class=\"inner\">❅</div>"
		"  </div>"
		"  <div class=\"snowflake\">"
		"    <div class=\"inner\">❅</div>"
		"  </div>"
		"</div>");
}
static void show_snowflakes_alt(webs_t wp)
{
	websWrite(wp, "<link type=\"text/css\" rel=\"stylesheet\" href=\"snow.css\" />\n");
websWrite(wp, "<div class=\"wrapper\">\n");
websWrite(wp, "     <div class=\"snow layer1 a\"></div>\n");
websWrite(wp, "     <div class=\"snow layer1\"></div>\n");
websWrite(wp, "     <div class=\"snow layer2 a\"></div>\n");
websWrite(wp, "     <div class=\"snow layer2"></div>\n");
websWrite(wp, "     <div class=\"snow layer3 a\"></div>\n");
websWrite(wp, "     <div class=\"snow layer3\"></div>\n");
websWrite(wp, "</div>\n");
}
static void do_pagehead(webs_t wp, int argc, char_t **argv, int pwc) // Eko
{
	char *charset = live_translate(wp, "lang_charset.set");
	char *translate = "";
	if (!nvram_match("language", "english"))
		translate = " translate=\"no\"";
	websWrite(
		wp,
		"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n<html%s>\n<head>\n<meta http-equiv=\"Content-Type\" content=\"application/xhtml+xml; charset=%s\" />\n",
		translate, charset);
	websWrite(wp, "<meta name=\"robots\" content=\"noindex\" />\n");
	websWrite(wp, "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\" />\n");
#ifndef HAVE_MICRO
	websWrite(
		wp,
		"<link rel=\"icon\" href=\"favicon.ico\" type=\"image/x-icon\" />\n<link rel=\"shortcut icon\" href=\"favicon.ico\" type=\"image/x-icon\" />\n");
#endif
	char *style = nvram_safe_get("router_style");
	char *style_dark = nvram_safe_get("router_style_dark");
	if (!*style)
		style = "elegant";
	if (strcmp(style, "kromo") && strcmp(style, "brainslayer") && strcmp(style, "wikar") && strcmp(style, "xirian")) {
		websWrite(wp, "<link type=\"text/css\" rel=\"stylesheet\" href=\"style/common.css\" />\n");
	}
	websWrite(
		wp,
		"<link type=\"text/css\" rel=\"stylesheet\" href=\"style/%s/style.css\" />\n<!--[if IE]><link type=\"text/css\" rel=\"stylesheet\" href=\"style/common_style_ie.css\" /><![endif]-->\n",
		style);
#ifdef HAVE_MICRO
	websWrite(wp, "<link type=\"text/css\" rel=\"stylesheet\" href=\"style/elegant/fresh.css\" />\n");
#else
	if (!strcmp(style, "blue") || !strcmp(style, "cyan") || !strcmp(style, "elegant") || !strcmp(style, "carlson") ||
	    !strcmp(style, "green") || !strcmp(style, "orange") || !strcmp(style, "red") || !strcmp(style, "yellow")) {
		websWrite(wp, "<link type=\"text/css\" rel=\"stylesheet\" href=\"style/elegant/fresh.css\" />\n");
		if (style_dark != NULL && !strcmp(style_dark, "1")) {
			websWrite(wp, "<link type=\"text/css\" rel=\"stylesheet\" href=\"style/elegant/fresh-dark.css\" />\n");
		}
	}
#endif
	websWrite(
		wp,
		"<script type=\"text/javascript\" src=\"common.js\"></script>\n<script type=\"text/javascript\" src=\"lang_pack/english.js\"></script>\n");
#ifdef HAVE_LANGUAGE
	if (!nvram_match("language", "english"))
		websWrite(wp, "<script type=\"text/javascript\" src=\"lang_pack/language.js\"></script>\n");
#endif
// temp
#ifdef HAVE_FREECWMP
	websWrite(wp, "<script type=\"text/javascript\" src=\"lang_pack/freecwmp-english.js\"></script>\n");
#endif
#ifdef HAVE_PWC
	if (pwc)
		websWrite(
			wp,
			"<link type=\"text/css\" rel=\"stylesheet\" href=\"style/pwc/ddwrt.css\" />\n<script type=\"text/javascript\" src=\"js/prototype.js\"></script>\n<script type=\"text/javascript\" src=\"js/effects.js\"></script>\n<script type=\"text/javascript\" src=\"js/window.js\"></script>\n<script type=\"text/javascript\" src=\"js/window_effects.js\"></script>\n");
#endif
	if ((startswith(wp->request_url, "Wireless") || startswith(wp->request_url, "WL_WPA")) && get_wl_instances() == 3)
		websWrite(wp, "<style type=\"text/css\">#header { height: 11.5em; }</style>\n");
	do_ddwrt_inspired_themes(wp);
#ifdef HAVE_WIKINGS
	websWrite(wp, "<title>:::: Excel Networks ::::");
#elif HAVE_ESPOD
	websWrite(wp, "<title>ESPOD Technologies");
#elif HAVE_SANSFIL
	websWrite(wp, "<title>SANSFIL (build %s)", SVN_REVISION);
#else
	websWrite(wp, "<title>%s (build %s)", nvram_safe_get("router_name"), SVN_REVISION);
#endif
	if (*(argv[0])) {
		websWrite(wp, " - %s", live_translate(wp, argv[0]));
	}
	websWrite(wp, "</title>\n");
	if (wp->path && strcasecmp(wp->path, "Info.htm")) {
		websWrite(wp, "<script type=\"text/javascript\">");
		if (!*(argv[0])) {
			websWrite(wp, "history.pushState({urlPath:'%s'}, \"%s (build %s)\", '%s')\n", wp->path,
				  nvram_safe_get("router_name"), SVN_REVISION, wp->path);
		} else {
			websWrite(wp, "history.pushState({urlPath:'%s'}, \"%s (build %s) - %s\", '%s')\n", wp->path,
				  nvram_safe_get("router_name"), SVN_REVISION, live_translate(wp, argv[0]), wp->path);
		}
		websWrite(wp, "</script>");
	}
	time_t t = time(NULL);
	struct tm *local = localtime(&t);
	if (local->tm_mon == 11 && local->tm_mday >= 24 && local->tm_mday <= 31)
		show_snowflakes(wp);
}

EJ_VISIBLE void ej_do_style(webs_t stream, int argc, char_t **argv)
{
	//      fprintf(stderr, "do style %d\n", argc);
	if (argc) {
		int i;
		for (i = 0; i < argc; i++) {
			//                      fprintf(stderr, "load %s\n", argv[i]);
			do_file(METHOD_GET, NULL, argv[i], stream);
		}
	} else {
		char *style = nvram_safe_get("router_style");
		if (!style)
			style = "elegant";
		char colorscheme[64];
		sprintf(colorscheme, "style/%s/colorscheme.css", style);
		do_file(METHOD_GET, NULL, colorscheme, stream);
		do_file(METHOD_GET, NULL, "style/common_core_rules.css", stream);
	}
	websDone(stream, 200);
}

EJ_VISIBLE void ej_do_hpagehead(webs_t wp, int argc, char_t **argv) // Eko
{
	char *htitle = argv[0];
	char *style = nvram_safe_get("router_style");
	char *style_dark = nvram_safe_get("router_style_dark");
	if (!*style)
		style = "elegant";
	websWrite(wp, "<!DOCTYPE html>\n");
	if (!strcmp(htitle, "doctype_only")) {
		websWrite(wp, "<html lang=\"en\">\n");
		websWrite(wp, "<head>\n");
		websWrite(wp, "<title>About DD-WRT</title>\n");
		websWrite(wp, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\" />\n");
		websWrite(wp, "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\" />\n");
		websWrite(wp, "<link type=\"text/css\" rel=\"stylesheet\" href=\"help/help.css\">\n");
		if (!strcmp(style, "blue") || !strcmp(style, "cyan") || !strcmp(style, "elegant") || !strcmp(style, "green") ||
		    !strcmp(style, "orange") || !strcmp(style, "red") || !strcmp(style, "yellow")) {
			if (style_dark != NULL && !strcmp(style_dark, "1")) {
				websWrite(wp,
					  "<link type=\"text/css\" rel=\"stylesheet\" href=\"../style/help-about-dark.css\" />\n");
			}
		}
		websWrite(wp, "<style type=\"text/css\">");
		websWrite(wp, "* {\n");
		websWrite(wp, "  font-family: Tahoma, Arial, Helvetica, sans-serif;\n");
		websWrite(wp, "  font-size: 1em;\n");
		websWrite(wp, "  text-align: center;\n");
		websWrite(wp, "  line-height: 1.7em;\n");
		websWrite(wp, "}\n");
		websWrite(wp, "body {\n");
		websWrite(wp, "  font-size: 75%%;\n");
		websWrite(wp, "  margin: .906em;\n");
		websWrite(wp, "}\n");
		websWrite(wp, ".t-border {\n");
		websWrite(wp, "  border: 1px solid #ccc;\n");
		websWrite(wp, "  border-radius: 4px;\n");
		websWrite(wp, "}\n");
		websWrite(wp, "th {\n");
		websWrite(wp, "  letter-spacing: .123rem;\n");
		websWrite(wp, "  height: 22px;\n");
		websWrite(wp, "}\n");
		websWrite(wp, ".thc {\n");
		websWrite(wp, "  background-color: #777674;\n");
		websWrite(wp, "  border: 0;\n");
		websWrite(wp, "  color: #fff;\n");
		websWrite(wp, "}\n");
		websWrite(wp, "td {\n");
		websWrite(wp, "  text-align: start;\n");
		websWrite(wp, "  padding: 3px;\n");
		websWrite(wp, "}\n");
		websWrite(wp, "a {\n");
		websWrite(wp, "  text-decoration: none;\n");
		websWrite(wp, "}\n");
		websWrite(wp, "a:hover {\n");
		websWrite(wp, "  text-decoration: underline;\n");
		websWrite(wp, "}\n");
		websWrite(wp, "h1 {\n");
		websWrite(wp, "  font-size: .88rem;\n");
		websWrite(wp, "}\n");
		websWrite(wp, ".table-props {\n");
		websWrite(wp, "  max-width: 80%%;\n");
		websWrite(wp, "  margin: auto;\n");
		websWrite(wp, "}\n");
		websWrite(wp, ".about-bg {\n");
		websWrite(wp, "  background-color: #fff;\n");
		websWrite(wp, "}\n");
		websWrite(wp, "#container {\n");
		websWrite(wp, "  height: 425px;\n");
		websWrite(wp, "  max-width: 80%%;\n");
		websWrite(wp, "  margin: -18em auto .5em auto;\n");
		websWrite(wp, "  overflow: hidden;\n");
		websWrite(wp, "  transform-origin: 50%% 100%%;\n");
		websWrite(wp, "  transform: perspective(205px) rotateX(25deg);\n");
		websWrite(wp, "}\n");
		websWrite(wp, "#credits {\n");
		websWrite(wp, "  height: auto;\n");
		websWrite(wp, "  font-size: 20px;\n");
		websWrite(wp, "  color: #8c6c00;\n");
		websWrite(wp, "  background: linear-gradient(to bottom, #8c6c00, #b98b02);\n");
		websWrite(wp, "  -webkit-text-fill-color: transparent;\n");
		websWrite(wp, "  -webkit-background-clip: text;\n");
		websWrite(wp, "  animation: foolywood 35s linear infinite;\n");
		websWrite(wp, "}\n");
		websWrite(wp, "@keyframes foolywood {\n");
		websWrite(wp, "  from { transform: translateY(100%%); opacity: 1; }\n");
		websWrite(wp, "  to { transform: translateY(-200%%); opacity: .5; }\n");
		websWrite(wp, "}\n");
		websWrite(wp, "</style>\n");
		do_ddwrt_inspired_themes(wp);
		websWrite(wp, "</head>\n");
		return; // stop here, for About.htm
	}
	websWrite(wp, "<html>\n");
	websWrite(wp, "<head>\n");
	websWrite(wp, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=%s\" />\n",
		  live_translate(wp, "lang_charset.set"));
	websWrite(wp, "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\" />\n");
	websWrite(wp, "<link type=\"text/css\" rel=\"stylesheet\" href=\"help.css\">\n");
	if (!strcmp(style, "blue") || !strcmp(style, "cyan") || !strcmp(style, "elegant") || !strcmp(style, "green") ||
	    !strcmp(style, "orange") || !strcmp(style, "red") || !strcmp(style, "yellow")) {
		if (style_dark != NULL && !strcmp(style_dark, "1")) {
			websWrite(wp, "<link type=\"text/css\" rel=\"stylesheet\" href=\"../style/help-about-dark.css\" />\n");
		}
	}
#ifndef HAVE_MICRO
	do_ddwrt_inspired_themes(wp);
#endif
	websWrite(wp, "<script type=\"text/javascript\" src=\"../lang_pack/english.js\"></script>\n");
#ifdef HAVE_LANGUAGE
	if (!nvram_match("language", "english"))
		websWrite(wp, "<script type=\"text/javascript\" src=\"../lang_pack/language.js\"></script>\n");
#endif
	websWrite(wp, "<title>%s (build %s)", live_translate(wp, "share.help"), SVN_REVISION);
	websWrite(wp, " - %s</title>\n", live_translate(wp, htitle));
	websWrite(wp, "<script type=\"text/javascript\" src=\"../common.js\"></script>\n");
	websWrite(wp, "</head>\n");
}

EJ_VISIBLE void ej_do_pagehead_nopwc(webs_t wp, int argc, char_t **argv) // Eko
{
	do_pagehead(wp, argc, argv, 0);
}

EJ_VISIBLE void ej_do_pagehead(webs_t wp, int argc, char_t **argv) // Eko
{
	do_pagehead(wp, argc, argv, 1);
}
