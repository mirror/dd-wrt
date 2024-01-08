/*
 * styles.c
 *
 * Copyright (C) 2005 - 2022 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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

// SEG DD-WRT addition
EJ_VISIBLE void ej_show_styles(webs_t wp, int argc, char_t **argv)
{
	// <option value="blue" <% nvram_selected("router_style", "blue");
	// %>>Blue</option>
	DIR *directory;
	char buf[256];

	directory = opendir("/www/style");
	struct dirent *entry;

	while ((entry = readdir(directory)) != NULL) {
		sprintf(buf, "style/%s/style.css", entry->d_name);
		FILE *web = getWebsFile(wp, buf);

		if (web == NULL) {
			sprintf(buf, "/www/style/%s/style.css", entry->d_name);
			if (!f_exists(buf))
				continue;
		}
		fclose(web);

		websWrite(wp, "<option value=\"%s\" %s>%s</option>\n",
			  entry->d_name,
			  nvram_match("router_style", entry->d_name) ?
				  "selected=\"selected\"" :
				  "",
			  entry->d_name);
	}
	closedir(directory);
	return;
}

size_t wfwrite(void *buf, size_t size, size_t n, webs_t wp);

void do_error_style(webs_t wp, int status, char *title, char *text)
{
	size_t len = 0;
	FILE *web = _getWebsFile(wp, "style/error_common.css", &len);
	if (!web)
		return;
	if (!len) {
		fclose(web);
		return;
	}
	char *mem = malloc(len + 1 + 16 + strlen(title) + strlen(text) +
			   strlen(text));

	if (!mem) {
		fclose(web);
		return;
	}
	fread(mem, 1, len, web);
	fclose(web);
	int i;
	char stat[32];
	sprintf(stat, "%d", status);
	for (i = 0; i < len; i++) {
		if (mem[i] == '%' && mem[i + 1] == 'd') {
			memmove(&mem[i + strlen(stat)], &mem[i + 2], len - i);
			memcpy(&mem[i], stat, strlen(stat));
			len += strlen(stat);
			break;
		}
	}
	for (i = 0; i < len; i++) {
		if (mem[i] == '%' && mem[i + 1] == 's') {
			memmove(&mem[i + strlen(title)], &mem[i + 2], len - i);
			memcpy(&mem[i], title, strlen(title));
			len += strlen(title);
			break;
		}
	}
	for (i = 0; i < len; i++) {
		if (mem[i] == '%' && mem[i + 1] == 's') {
			memmove(&mem[i + strlen(text)], &mem[i + 2], len - i);
			memcpy(&mem[i], text, strlen(text));
			len += strlen(text);
		}
	}
	websWrite(
		wp,
		"<style id=\"stylus-1\" type=\"text/css\" class=\"stylus\">\n");
	wfwrite(mem, 1, len, wp);
	debug_free(mem);
	websWrite(wp, "</style>\n");
}

#if !defined(HAVE_MICRO) && !defined(HAVE_NO_STYLUS)

EJ_VISIBLE void ej_show_ddwrt_inspired_themes(webs_t wp, int argc,
					      char_t **argv)
{
	/* todo, read dir content and generate this */
	char buf[128];
	websWrite(wp, "<fieldset>\n");
	websWrite(wp, "<legend>%s</legend>\n",
		  tran_string(buf, sizeof(buf), "management.inspired_themes"));
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\">%s</div>\n",
		  tran_string(buf, sizeof(buf), "share.theme"));
	websWrite(wp, "<select name=\"stylus\">\n");
	websWrite(wp, "<option value=\"off\" %s>%s</option>\n",
		  nvram_match("stylus", "off") ? "selected=\"selected\"" : "",
		  tran_string(buf, sizeof(buf), "share.off"));
	websWrite(wp, "<option value=\"aeon\" %s>Aeon</option>\n",
		  nvram_match("stylus", "aeon") ? "selected=\"selected\"" : "");
	websWrite(wp, "<option value=\"antigua\" %s>Antigua</option>\n",
		  nvram_match("stylus", "antigua") ? "selected=\"selected\"" :
						     "");
	websWrite(wp, "<option value=\"dracula\" %s>Dracula</option>\n",
		  nvram_match("stylus", "dracula") ? "selected=\"selected\"" :
						     "");
	websWrite(wp, "<option value=\"duo_cocoa\" %s>Duo Cocoa</option>\n",
		  nvram_match("stylus", "duo_cocoa") ? "selected=\"selected\"" :
						       "");
	websWrite(wp, "<option value=\"material\" %s>Material</option>\n",
		  nvram_match("stylus", "material") ? "selected=\"selected\"" :
						      "");
	websWrite(
		wp,
		"<option value=\"material_darker\" %s>Material Darker</option>\n",
		nvram_match("stylus", "material_darker") ?
			"selected=\"selected\"" :
			"");
	websWrite(
		wp,
		"<option value=\"solarized_dark\" %s>Solarized Dark</option>\n",
		nvram_match("stylus", "solarized_dark") ?
			"selected=\"selected\"" :
			"");
	websWrite(wp, "<option value=\"the_matrix\" %s>The Matrix</option>\n",
		  nvram_match("stylus", "the_matrix") ?
			  "selected=\"selected\"" :
			  "");
	websWrite(wp, "<option value=\"twilight\" %s>Twilight</option>\n",
		  nvram_match("stylus", "twilight") ? "selected=\"selected\"" :
						      "");
	websWrite(wp, "<option value=\"ubuntu\" %s>Ubuntu</option>\n",
		  nvram_match("stylus", "ubuntu") ? "selected=\"selected\"" :
						    "");
	websWrite(wp, "</select>\n");
	websWrite(wp, "</div>\n");
	websWrite(wp, "</fieldset><br />\n");
	return;
}

void do_ddwrt_inspired_themes(webs_t wp)
{
	if (nvram_match("router_style", "kromo") ||
	    nvram_match("router_style", "brainslayer") ||
	    nvram_match("router_style", "wikar") ||
	    nvram_match("router_style", "xirian"))
		return;
	char path[128];
	size_t len = 0;
	sprintf(path, "ddwrt_inspired_themes/%s.stylus",
		nvram_safe_get("stylus"));
	FILE *web = _getWebsFile(wp, path, &len);
	if (!web)
		return;
	if (!len) {
		fclose(web);
		return;
	}
	char *mem = malloc(len + 1);
	if (!mem) {
		fclose(web);
		return;
	}
	fread(mem, 1, len, web);
	fclose(web);
	websWrite(
		wp,
		"<style id=\"stylus-1\" type=\"text/css\" class=\"stylus\">\n");
	wfwrite(mem, 1, len, wp);
	debug_free(mem);
	sprintf(path, "ddwrt_inspired_themes/core.css");
	web = _getWebsFile(wp, path, &len);
	if (!web)
		return;
	if (!len) {
		fclose(web);
		return;
	}
	mem = malloc(len + 1);

	if (!mem) {
		fclose(web);
		return;
	}
	fread(mem, 1, len, web);
	fclose(web);
	wfwrite(mem, 1, len, wp);
	debug_free(mem);
	websWrite(wp, "</style>\n");
}
#else
void do_ddwrt_inspired_themes(webs_t wp)
{
}
#endif
