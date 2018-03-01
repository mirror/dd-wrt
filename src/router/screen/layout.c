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
 * http://www.gnu.org/licenses/, or contact Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 *
 ****************************************************************
 */

#include "config.h"
#include "screen.h"
#include "extern.h"
#include "layout.h"

extern struct display *display;
extern int captionalways;

struct layout *layouts;
struct layout *laytab[MAXLAY];
struct layout *layout_last, layout_last_marker;
struct layout *layout_attach = &layout_last_marker;

void FreeLayoutCv(struct canvas *cv)
{
  struct canvas *cnext, *c = cv;
  for (; cv; cv = cnext) {
    if (cv->c_slperp) {
      FreeLayoutCv(cv->c_slperp);
      free(cv->c_slperp);
      cv->c_slperp = 0;
    }
    cnext = cv->c_slnext;
    cv->c_slnext = 0;
    if (cv != c)
      free(cv);
  }
}

struct layout *CreateLayout(char *title, int startat)
{
  struct layout *lay, **pl;
  int i;

  if (startat >= MAXLAY || startat < 0)
    startat = 0;
  for (i = startat; ;) {
    if (!laytab[i])
      break;
    if (++i == MAXLAY)
      i = 0;
    if (i == startat) {
      Msg(0, "No more layouts\n");
      return 0;
    }
  }
  lay = (struct layout *)calloc(1, sizeof(*lay));
  lay->lay_title = SaveStr(title);
  lay->lay_autosave = 1;
  lay->lay_number = i;
  laytab[i] = lay;
  lay->lay_next = 0;

  pl = &layouts;
  while (*pl)
    pl = &(*pl)->lay_next;
  *pl = lay;
  return lay;
}

void SaveLayout(char *name, struct canvas *cv)
{
  struct layout *lay;
  struct canvas *fcv;
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
  DupLayoutCv(cv, &lay->lay_canvas, 1);
  lay->lay_forecv = D_forecv;
  D_forecv = fcv;
  D_layout = lay;
}

void AutosaveLayout(struct layout *lay)
{
  struct canvas *fcv;
  if (!lay || !lay->lay_autosave)
    return;
  FreeLayoutCv(&lay->lay_canvas);
  fcv = D_forecv;
  DupLayoutCv(&D_canvas, &lay->lay_canvas, 1);
  lay->lay_forecv = D_forecv;
  D_forecv = fcv;
}

struct layout *FindLayout(char *name)
{
  struct layout *lay;
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

void LoadLayout(struct layout *lay, struct canvas *cv)
{
  AutosaveLayout(D_layout);
  if (!lay) {
    while (D_canvas.c_slperp)
      FreeCanvas(D_canvas.c_slperp);
    MakeDefaultCanvas();
    SetCanvasWindow(D_forecv, 0);
    D_layout = 0;
    return;
  }
  while (D_canvas.c_slperp)
    FreeCanvas(D_canvas.c_slperp);
  D_cvlist = 0;
  D_forecv = lay->lay_forecv;
  if (!D_forecv)
    MakeDefaultCanvas();
  DupLayoutCv(&lay->lay_canvas, &D_canvas, 0);
  D_canvas.c_ys = (D_has_hstatus == HSTATUS_FIRSTLINE);
  D_canvas.c_ye = D_height - 1 - ((D_canvas.c_slperp && D_canvas.c_slperp->c_slnext) ||
                  captionalways) - (D_has_hstatus == HSTATUS_LASTLINE);
  ResizeCanvas(&D_canvas);
  RecreateCanvasChain();
  RethinkDisplayViewports();
  PutWindowCv(&D_canvas);
  ResizeLayersToCanvases();
  D_layout = lay;
}

void NewLayout(char *title, int startat)
{
  struct layout *lay;
  struct canvas *fcv;

  lay = CreateLayout(title, startat);
  if (!lay)
    return;

  if (display) {
    LoadLayout(0, &D_canvas);
    fcv = D_forecv;
    DupLayoutCv(&D_canvas, &lay->lay_canvas, 1);
    lay->lay_forecv = D_forecv;
    D_forecv = fcv;
    D_layout = lay;
  }
  else
    layout_attach = lay;

  lay->lay_autosave = 1;
}


static char *AddLayoutsInfo(char *buf, int len, int where)
{
  char *s, *ss, *t;
  struct layout *p, **pp;
  int l;

  s = ss = buf;
  for (pp = laytab; pp < laytab + MAXLAY; pp++) {
    if (pp - laytab == where && ss == buf)
      ss = s;
    if ((p = *pp) == 0)
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
  ss = AddLayoutsInfo(buf, sizeof(buf), where);
  s = buf + strlen(buf);
  if (ss - buf > D_width / 2) {
    ss -= D_width / 2;
    if (s - ss < D_width) {
      ss = s - D_width;
      if (ss < buf)
        ss = buf;
    }
  }
  else
    ss = buf;
  Msg(0, "%s", ss);
}

void RemoveLayout(struct layout *lay)
{
  struct layout **layp = &layouts;

  for (; *layp; layp = &(*layp)->lay_next) {
    if (*layp == lay) {
      *layp = lay->lay_next;
      break;
    }
  }
  laytab[lay->lay_number] = (struct layout *)0;

  if (display && D_layout == lay)
    D_layout = (struct layout *)0;

  FreeLayoutCv(&lay->lay_canvas);

  if (lay->lay_title)
    free(lay->lay_title);
  free(lay);

  if (layouts)
    LoadLayout((display && D_layout) ? D_layout : *layp ? *layp : layouts,
	display ? &D_canvas : (struct canvas *)0);
  Activate(0);
}

void UpdateLayoutCanvas(struct canvas *cv, struct win * wi)
{
  for (; cv; cv = cv->c_slnext) {
    if (cv->c_layer && Layer2Window(cv->c_layer) == wi) {
      /* A simplistic version of SetCanvasWindow(cv, 0) */
      struct layer *l = cv->c_layer;
      cv->c_layer = 0;
      if (l->l_cvlist == 0 && (wi == 0 || l != wi->w_savelayer))
        KillLayerChain(l);

      l = &cv->c_blank;
      l->l_data = 0;
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


static void dump_canvas(struct canvas *cv, FILE *file)
{
  struct canvas *c;
  for (c = cv->c_slperp; c && c->c_slnext; c = c->c_slnext)
    fprintf(file, "split%s\n", c->c_slorient == SLICE_HORI ? " -v" : "");
    
  for (c = cv->c_slperp; c; c = c->c_slnext) {
    if (c->c_slperp)
      dump_canvas(c, file);
    else
      fprintf(file, "focus\n");
  }
}

int LayoutDumpCanvas(struct canvas *cv, char *filename)
{
  FILE *file = secfopen(filename, "a");
  if (!file)
    return 0;
  dump_canvas(cv, file);
  fclose(file);
  return 1;
}

void RenameLayout(struct layout *layout, const char *name)
{
  free(layout->lay_title);
  layout->lay_title = SaveStr(name);
}

int RenumberLayout(struct layout *layout, int number)
{
  int old;
  struct layout *lay;
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
