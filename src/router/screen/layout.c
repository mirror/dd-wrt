/* Copyright (c) 2008, 2009
 *      Juergen Weigert (jnweiger@immd4.informatik.uni-erlangen.de)
 *      Michael Schroeder (mlschroe@immd4.informatik.uni-erlangen.de)
 *      Micah Cowan (micah@cowan.name)
 *      Sadrul Habib Chowdhury (sadrul@users.sourceforge.net)
 * Copyright (c) 1993-2002, 2003, 2005, 2006, 2007
 *      Juergen Weigert (jnweiger@immd4.informatik.uni-erlangen.de)
 *      Michael Schroeder (mlschroe@immd4.informatik.uni-erlangen.de)
 * Copyright (c) 1987 Oliver Laumann
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (see the file COPYING); if not, see
 * https://www.gnu.org/licenses/, or contact Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 *
 ****************************************************************
 */

#include "config.h"

#include "layout.h"

#include <stdbool.h>
#include <stdint.h>

#include "screen.h"

#include "fileio.h"
#include "misc.h"
#include "process.h"
#include "resize.h"

Layout *layouts;
Layout *laytab[MAXLAY];
Layout *layout_last, layout_last_marker;
Layout *layout_attach = &layout_last_marker;

void FreeLayoutCv(Canvas *cv)
{
	Canvas *cnext, *c = cv;
	for (; cv; cv = cnext) {
		if (cv->c_slperp) {
			FreeLayoutCv(cv->c_slperp);
			free(cv->c_slperp);
			cv->c_slperp = NULL;
		}
		cnext = cv->c_slnext;
		cv->c_slnext = NULL;
		if (cv != c)
			free(cv);
	}
}

Layout *CreateLayout(char *title, int startat)
{
	Layout *lay, **pl;
	int i;

	if (startat >= MAXLAY || startat < 0)
		startat = 0;
	for (i = startat;;) {
		if (!laytab[i])
			break;
		if (++i == MAXLAY)
			i = 0;
		if (i == startat) {
			Msg(0, "No more layouts\n");
			return NULL;
		}
	}
	lay = calloc(1, sizeof(Layout));
	lay->lay_title = SaveStr(title);
	lay->lay_autosave = 1;
	lay->lay_number = i;
	laytab[i] = lay;
	lay->lay_next = NULL;

	pl = &layouts;
	while (*pl)
		pl = &(*pl)->lay_next;
	*pl = lay;
	return lay;
}

void SaveLayout(char *name, Canvas *cv)
{
	Layout *lay;
	Canvas *fcv;
	for (lay = layouts; lay; lay = lay->lay_next)
		if (!strcmp(lay->lay_title, name))
			break;
	if (lay)
		FreeLayoutCv(&lay->lay_canvas);
	else
		lay = CreateLayout(name, 0);
	if (!lay)
		return;
	fcv = D_forecv;
	DupLayoutCv(cv, &lay->lay_canvas, true);
	lay->lay_forecv = D_forecv;
	D_forecv = fcv;
	D_layout = lay;
}

void AutosaveLayout(Layout *lay)
{
	Canvas *fcv;
	if (!lay || !lay->lay_autosave)
		return;
	FreeLayoutCv(&lay->lay_canvas);
	fcv = D_forecv;
	DupLayoutCv(&D_canvas, &lay->lay_canvas, true);
	lay->lay_forecv = D_forecv;
	D_forecv = fcv;
}

Layout *FindLayout(char *name)
{
	Layout *lay;
	char *s;
	int i;
	for (i = 0, s = name; *s >= '0' && *s <= '9'; s++)
		i = i * 10 + (*s - '0');
	if (!*s && s != name && i >= 0 && i < MAXLAY)
		return laytab[i];
	for (lay = layouts; lay; lay = lay->lay_next)
		if (!strcmp(lay->lay_title, name))
			break;
	return lay;
}

void LoadLayout(Layout *lay)
{
	if (!display)
		return;

	AutosaveLayout(D_layout);
	if (!lay) {
		while (D_canvas.c_slperp)
			FreeCanvas(D_canvas.c_slperp);
		MakeDefaultCanvas();
		SetCanvasWindow(D_forecv, NULL);
		D_layout = NULL;
		return;
	}
	while (D_canvas.c_slperp)
		FreeCanvas(D_canvas.c_slperp);
	D_cvlist = NULL;
	D_forecv = lay->lay_forecv;
	if (!D_forecv)
		MakeDefaultCanvas();
	DupLayoutCv(&lay->lay_canvas, &D_canvas, false);
	D_canvas.c_ys = (D_has_hstatus == HSTATUS_FIRSTLINE);
	D_canvas.c_ye = D_height - 1 - ((D_canvas.c_slperp && D_canvas.c_slperp->c_slnext)
					|| captionalways) - (D_has_hstatus == HSTATUS_LASTLINE);
	ResizeCanvas(&D_canvas);
	RecreateCanvasChain();
	RethinkDisplayViewports();
	PutWindowCv(&D_canvas);
	ResizeLayersToCanvases();
	D_layout = lay;
}

void NewLayout(char *title, int startat)
{
	Layout *lay;
	Canvas *fcv;

	lay = CreateLayout(title, startat);
	if (!lay)
		return;

	if (display) {
		LoadLayout(NULL);
		fcv = D_forecv;
		DupLayoutCv(&D_canvas, &lay->lay_canvas, true);
		lay->lay_forecv = D_forecv;
		D_forecv = fcv;
		D_layout = lay;
	} else {
		layout_attach = lay;
	}
	lay->lay_autosave = 1;
}

static char *AddLayoutsInfo(char *buf, int len, int where)
{
	char *s, *ss, *t;
	Layout *p, **pp;
	int l;

	s = ss = buf;
	for (pp = laytab; pp < laytab + MAXLAY; pp++) {
		if (pp - laytab == where && ss == buf)
			ss = s;
		if ((p = *pp) == NULL)
			continue;
		t = p->lay_title;
		l = strlen(t);
		if (l > 20)
			l = 20;
		if (s - buf + l > len - 24)
			break;
		if (s > buf) {
			*s++ = ' ';
			*s++ = ' ';
		}
		sprintf(s, "%d", p->lay_number);
		if (p->lay_number == where)
			ss = s;
		s += strlen(s);
		if (display && p == D_layout)
			*s++ = '*';
		*s++ = ' ';
		strncpy(s, t, l);
		s += l;
	}
	*s = 0;
	return ss;
}

void ShowLayouts(int where)
{
	char buf[1024];
	char *s, *ss;

	if (!display)
		return;
	if (!layouts) {
		Msg(0, "No layouts defined\n");
		return;
	}
	if (where == -1 && D_layout)
		where = D_layout->lay_number;
	ss = AddLayoutsInfo(buf, ARRAY_SIZE(buf), where);
	s = buf + strlen(buf);
	if (ss - buf > D_width / 2) {
		ss -= D_width / 2;
		if (s - ss < D_width) {
			ss = s - D_width;
			if (ss < buf)
				ss = buf;
		}
	} else
		ss = buf;
	Msg(0, "%s", ss);
}

void RemoveLayout(Layout *lay)
{
	Layout **layp = &layouts;

	for (; *layp; layp = &(*layp)->lay_next) {
		if (*layp == lay) {
			*layp = lay->lay_next;
			break;
		}
	}
	laytab[lay->lay_number] = NULL;

	if (display && D_layout == lay)
		D_layout = NULL;

	FreeLayoutCv(&lay->lay_canvas);

	if (lay->lay_title)
		free(lay->lay_title);
	free(lay);

	if (layouts)
		LoadLayout((display && D_layout) ? D_layout : *layp ? *layp : layouts);
	Activate(0);
}

void UpdateLayoutCanvas(Canvas *cv, Window *wi)
{
	for (; cv; cv = cv->c_slnext) {
		if (cv->c_layer && Layer2Window(cv->c_layer) == wi) {
			/* A simplistic version of SetCanvasWindow(cv, 0) */
			Layer *l = cv->c_layer;
			cv->c_layer = NULL;
			if (l->l_cvlist == NULL && (wi == NULL || l != wi->w_savelayer))
				KillLayerChain(l);
			l = &cv->c_blank;
			l->l_data = NULL;
			if (l->l_cvlist != cv) {
				cv->c_lnext = l->l_cvlist;
				l->l_cvlist = cv;
			}
			cv->c_layer = l;
			/* Do not end here. Multiple canvases can have the same window */
		}

		if (cv->c_slperp)
			UpdateLayoutCanvas(cv->c_slperp, wi);
	}
}

static void dump_canvas(Canvas *cv, FILE * file)
{
	Canvas *c;
	for (c = cv->c_slperp; c && c->c_slnext; c = c->c_slnext) {
		fprintf(file, "split%s\n", c->c_slorient == SLICE_HORI ? " -v" : "");
	}

	for (c = cv->c_slperp; c; c = c->c_slnext) {
		if (c->c_slperp)
			dump_canvas(c, file);
		else
			fprintf(file, "focus\n");
	}
}

int LayoutDumpCanvas(Canvas *cv, char *filename)
{
	FILE *file = secfopen(filename, "a");
	if (!file)
		return 0;
	dump_canvas(cv, file);
	fclose(file);
	return 1;
}

void RenameLayout(Layout *layout, const char *name)
{
	free(layout->lay_title);
	layout->lay_title = SaveStr(name);
}

int RenumberLayout(Layout *layout, int number)
{
	int old;
	Layout *lay;
	old = layout->lay_number;
	if (number < 0 || number >= MAXLAY)
		return 0;
	lay = laytab[number];
	laytab[number] = layout;
	layout->lay_number = number;
	laytab[old] = lay;
	if (lay)
		lay->lay_number = old;
	return 1;
}
