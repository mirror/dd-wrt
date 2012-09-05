#include "links.h"

/* The location of the box in the bookmark manager */
#define	BM_BOX_IND		5


/* The list of bookmarks */
struct list_head bookmarks = {&bookmarks, &bookmarks};

/* The last used id of a bookmark */
bookmark_id next_bookmark_id = 0;

/* Clears the bookmark list */
void free_bookmarks() {
	struct bookmark *bm;

	foreach(bm, bookmarks) {
		mem_free(bm->title);
		mem_free(bm->url);
	}

	free_list(bookmarks);
}

/* Does final cleanup and saving of bookmarks */
void finalize_bookmarks() {
	write_bookmarks();
	free_bookmarks();
}


/* Loads the bookmarks from file */
void read_bookmarks() {
	unsigned char in_buffer[MAX_STR_LEN];	
	unsigned char *file_name;
	unsigned char *title, *url;	/* Pointers to the start of each field (in buffer)*/
	FILE *f;
	
	file_name = stracpy(links_home);
	if (!file_name) return;
	add_to_strn(&file_name, "bookmarks");
	
	f = fopen(file_name, "r");
	mem_free(file_name);
	if (f == NULL)
		return;
	
	title = in_buffer;
	
	while (fgets(in_buffer, MAX_STR_LEN, f)) {
		url = strchr(in_buffer, '|');
		if (url == NULL) 
			continue;
		*url = '\0';
		url++;
		if (strchr(url, '\n')) *( strchr(url, '\n') ) = '\0';
		add_bookmark(title, url);
	}
	
	fclose(f);
}

/* Saves the bookmarks to file */
void write_bookmarks() {
	struct bookmark *bm;
	FILE *out;
	unsigned char *tmp_file_name, *file_name;

	tmp_file_name = stracpy(links_home);
	if (!tmp_file_name) return;
	add_to_strn(&tmp_file_name, "bookmarks.tmp");
	
	out = fopen(tmp_file_name, "w");
	if (!out) {
		mem_free(tmp_file_name);
		return;
	}

	foreachback(bm, bookmarks) {
		unsigned char *p = stracpy(bm->title);
		int i;
		for (i = strlen(p) - 1; i >= 0; i--) if (p[i] < ' '|| p[i] == '|') p[i] = ' ';
		fputs(p,out);
		fputc('|', out);
		fputs(bm->url,out);
		fputc('\n',out);
		mem_free(p);
	}

	if (ferror(out)) {
		fclose(out);
		goto err;
	}
	if (fclose(out) == EOF) {
		err:
		remove(tmp_file_name);
		mem_free(tmp_file_name);
		return;
	}

	file_name = stracpy(links_home);
	if (!file_name) {
		mem_free(tmp_file_name);
		return;
	}
	add_to_strn(&file_name, "bookmarks");

#ifndef RENAME_OVER_EXISTING_FILES
	unlink(file_name);		/* OS/2 needs this */
#endif
	rename(tmp_file_name, file_name);
	mem_free(tmp_file_name);
	mem_free(file_name);

}

/* Gets a bookmark by id */
struct bookmark *get_bookmark_by_id(bookmark_id id) {
	struct bookmark *bm;

	if (id == BAD_BOOKMARK_ID) 
		return NULL;

	foreach(bm, bookmarks) {
		if (id == bm->id)
			return bm;
	}

	return NULL;
}


/* Adds a bookmark to the bookmark list. Don't play with new_bm after you're
	done. It would be impolite. */
void add_bookmark(const unsigned char *title, const unsigned char *url) {
	struct bookmark *bm;
	int	title_size;	/* How much mem to allocate for the strings */
	int url_size;

	title_size = strlen(title) + 1;
	url_size = strlen(url) + 1;

	bm = mem_alloc(sizeof(struct bookmark));

	bm->title = mem_alloc(title_size);
	bm->url = mem_alloc(url_size);
	
	strcpy(bm->title, title);
	strcpy(bm->url, url);

	bm->id = next_bookmark_id++;

	/* Actually add it */
	add_to_list(bookmarks, bm);
}


/* Updates an existing bookmark. 
 *
 * If the requested bookmark does not exist, return 0. Otherwise, return 1.
 *
 * If any of the fields are NULL, the value is left unchanged.
 */
int bookmark_update(bookmark_id id, unsigned char *title, unsigned char *url) {
	struct bookmark *bm = get_bookmark_by_id(id);

	if (bm == NULL) {
		/* Does not exist. */
		return 0;
	}

	if (title) {
		mem_free(bm->title);
		bm->title = stracpy((unsigned char *)title);
	}

	if (url) {
		mem_free(bm->url);
		bm->url = stracpy((unsigned char *)url);
	}

	return 1;
}

/* Allocates and returns a bookmark */
struct bookmark *create_bookmark(const unsigned char *title, const unsigned char *url) {
	struct bookmark *new_bm = NULL;
	size_t title_size;	/* How much mem to allocate for the strings */
	size_t url_size;

	title_size = strlen(title) + 1;
	url_size = strlen(url) + 1;
	if (title_size > MAXINT) overalloc();
	if (url_size > MAXINT) overalloc();

	new_bm = mem_alloc(sizeof(struct bookmark));

	new_bm->title = mem_alloc(title_size);
	new_bm->url = mem_alloc(url_size);
	
	strcpy(new_bm->title, title);
	strcpy(new_bm->url, url);

	return new_bm;
}


/* Deletes a bookmark, given the id. Returns 0 on failure (no such bm), 1 on 
	success */
int delete_bookmark_by_id(bookmark_id id) {
	struct bookmark *bm;

	bm = get_bookmark_by_id(id);

	if (bm == NULL)
		return 0;

	del_from_list(bm);

	/* Now wipe the bookmark */
	mem_free(bm->title);
	mem_free(bm->url);

	mem_free(bm);
	
	return 1;
}

/****************************************************************************
*
* Bookmark manager stuff.
*
****************************************************************************/

void bookmark_edit_dialog(
		struct terminal *, 
		unsigned char *, 
		const unsigned char *, 
		const unsigned char *, 
		struct session *, 
		struct dialog_data *,
		void when_done(struct dialog *), 
		void *
);

/* Gets the head of the bookmark list kept by the dialog (the one used for 
	display purposes */
/* I really should use this somewhere...
static inline *list_head bookmark_dlg_list_get(struct dialog) {
	return dialog->items[BM_BOX_IND].data;
}
*/

/* Clears the bookmark list from the bookmark_dialog */
static inline void bookmark_dlg_list_clear(struct list_head *bm_list) {
	free_list( *bm_list );
}

/* Updates the bookmark list for a dialog. Returns the number of bookmarks.
	FIXME: Must be changed for hierarchical bookmarks.
*/
int bookmark_dlg_list_update(struct list_head *bm_list) {
	struct bookmark *bm;	/* Iterator over bm list */
	struct box_item	*item;	/* New box item (one per displayed bookmark) */
	unsigned char *text;
	int count = 0;
	bookmark_id id;
	
	/* Empty the list */
	bookmark_dlg_list_clear(bm_list);
	
	/* Copy each bookmark into the display list */
	foreach(bm, bookmarks) {
		/* Deleted in bookmark_dlg_clear_list() */
		item = mem_alloc( sizeof(struct box_item) + strlen(bm->title) + 1);
		item->text = text = ((unsigned char *)item + sizeof(struct box_item));
		item->data = (void *)(id = bm->id);
	
		/* Note that free_i is left at zero */
	
		strcpy(text, bm->title);

		add_to_list( *bm_list, item);
		count++;
	}
	return count;
}


/* Creates the box display (holds everything EXCEPT the actual rendering data) */
struct dlg_data_item_data_box *bookmark_dlg_box_build(struct dlg_data_item_data_box **box) {
	/* Deleted in abort */
	*box = mem_alloc( sizeof(struct dlg_data_item_data_box) );
	memset(*box, 0, sizeof(struct dlg_data_item_data_box));

	init_list((*box)->items);
	
	(*box)->list_len = bookmark_dlg_list_update(&((*box)->items));
	return *box;
}

/* Get the id of the currently selected bookmark */
bookmark_id bookmark_dlg_box_id_get(struct dlg_data_item_data_box *box) {
	struct box_item *citem;
	int sel;
	
	sel = box->sel;
	
	if (sel == -1) 
		return BAD_BOOKMARK_ID;
	
	/* Sel is an index into the list of bookmarks. Therefore, we spin thru
		until sel equals zero, and return the id at that point */
	foreach(citem, box->items) {
		if (sel == 0) 
			return (bookmark_id)(citem->data);
		sel--;
	}

	return BAD_BOOKMARK_ID;
}



/* Cleans up after the bookmark dialog */
void bookmark_dialog_abort_handler(struct dialog_data *dlg) {
	struct dlg_data_item_data_box *box;

	box = (struct dlg_data_item_data_box *)(dlg->dlg->items[BM_BOX_IND].data);

	/* Zap the display list */
	bookmark_dlg_list_clear(&(box->items));

	/* Delete the box structure */
	mem_free(box);
}

/* Handles events for a bookmark dialog */
int bookmark_dialog_event_handler(struct dialog_data *dlg, struct event *ev) {

	switch ((int)ev->ev) {
		case EV_KBD:
			/* Catch change focus requests */
			if (ev->x == KBD_RIGHT || (ev->x == KBD_TAB && !ev->y)) {
				/* MP: dirty crap!!! this should be done in bfu.c */
				/* Move right */
                display_dlg_item(dlg, &dlg->items[dlg->selected], 0);
				if (++dlg->selected >= BM_BOX_IND) 
					dlg->selected = 0; 
                display_dlg_item(dlg, &dlg->items[dlg->selected], 1);
				
				return EVENT_PROCESSED;
			}
			
			if (ev->x == KBD_LEFT || (ev->x == KBD_TAB && ev->y)) {
				/* Move left */
                display_dlg_item(dlg, &dlg->items[dlg->selected], 0);
				if (--dlg->selected < 0) 
					dlg->selected = BM_BOX_IND - 1; 
                display_dlg_item(dlg, &dlg->items[dlg->selected], 1);
				
				return EVENT_PROCESSED;
			}

			/* Moving the box */
			if (ev->x == KBD_DOWN) {
				box_sel_move(&dlg->items[BM_BOX_IND], 1);
				show_dlg_item_box(dlg, &dlg->items[BM_BOX_IND]);
				
				return EVENT_PROCESSED;
			}
			
			if (ev->x == KBD_UP) {
				box_sel_move(&dlg->items[BM_BOX_IND], -1);
				show_dlg_item_box(dlg, &dlg->items[BM_BOX_IND]);
				
				return EVENT_PROCESSED;
			}
			
			if (ev->x == KBD_PAGE_DOWN) {
				box_sel_move(&dlg->items[BM_BOX_IND], dlg->items[BM_BOX_IND].item->gid / 2);
				show_dlg_item_box(dlg, &dlg->items[BM_BOX_IND]);
				
				return EVENT_PROCESSED;
			}
			
			if (ev->x == KBD_PAGE_UP) {
				box_sel_move(&dlg->items[BM_BOX_IND], (-1) * dlg->items[BM_BOX_IND].item->gid / 2);
				show_dlg_item_box(dlg, &dlg->items[BM_BOX_IND]);
				
				return EVENT_PROCESSED;
			}
			
			/* Selecting a button */
		break;
		case EV_INIT:
		case EV_RESIZE:
		case EV_REDRAW:
		case EV_MOUSE:
		case EV_ABORT:
		break;
		default:
			internal("Unknown event received: %d", ev->ev);
	}

	return EVENT_NOT_PROCESSED;
}


/* The titles to appear in the bookmark add dialog */
unsigned char *bookmark_add_msg[] = {
	TEXT_(T_BOOKMARK_TITLE),
	TEXT_(T_URL),
};


/* The titles to appear in the bookmark manager */
unsigned char *bookmark_dialog_msg[] = {
	TEXT_(T_BOOKMARKS),
};


/* Loads the selected bookmark */
void menu_goto_bookmark(struct terminal *term, void *url, struct session *ses) {
	goto_url(ses, (unsigned char*)url);
}


/* Gets the url of the requested bookmark. 
 * 
 * This returns a pointer to the url's data. Be gentle with it. 
 *
 * Returns a NULL if the bookmark has no url, AND if the passed bookmark id is 
 * invalid. 
 */
const unsigned char *bookmark_get_url(bookmark_id id) {
	struct bookmark *bm = get_bookmark_by_id(id);
	
	if (bm == NULL) {
		return NULL;
	}

	return bm->url;
}

/* Gets the name of the requested bookmark.
 *
 * See bookmark_get_url() for further comments.
 */
const unsigned char *bookmark_get_name(bookmark_id id) {
	struct bookmark *bm = get_bookmark_by_id(id);
	
	if (bm == NULL) {
		return NULL;
	}

	return bm->title;
}
		


/* Goes to the called bookmark */
void bookmark_goto(bookmark_id id, struct session *ses) {
	struct bookmark *bm;
	bm = get_bookmark_by_id(id);
	
	if (bm) 
		goto_url(ses, bm->url);
		
}

/* Shows the bookmark list */
void bookmark_menu(struct terminal *term, void *ddd, struct session *ses)
{
	struct bookmark *bm;
	struct menu_item *mi;
	
	if (!(mi = new_menu(3)))
		return;
		
	foreach(bm, bookmarks) {
		add_to_menu(&mi, stracpy(bm->title), "", 0, MENU_FUNC menu_goto_bookmark, (void *)bm->url, 0);
	}
	
	do_menu(term, mi, ses);
}


/* Called to setup the bookmark dialog */
void layout_bookmark_manager(struct dialog_data *dlg)
{
	int max = 0, min = 0;
	int w, rw;
	int y = -1;
	struct terminal *term;

	term = dlg->win->term;
	
	/* Find dimensions of dialog */
	max_text_width(term, bookmark_dialog_msg[0], &max);
	min_text_width(term, bookmark_dialog_msg[0], &min);
	max_buttons_width(term, dlg->items + 2, 2, &max);
	min_buttons_width(term, dlg->items + 2, 2, &min);
	
	w = term->x * 9 / 10 - 2 * DIALOG_LB;
	if (w > max) w = max;
	if (w < min) w = min;
	
	if (w > term->x - 2 * DIALOG_LB) 
		w = term->x - 2 * DIALOG_LB;
		
	if (w < 1) 
		w = 1;
		
	w = rw = 50 ;
	
	y += 1;	/* Blankline between top and top of box */
	dlg_format_box(NULL, term, &dlg->items[BM_BOX_IND], dlg->x + DIALOG_LB, &y, w, NULL, AL_LEFT);
	y += 1;	/* Blankline between box and menu */
	dlg_format_buttons(NULL, term, dlg->items, 5, 0, &y, w, &rw, AL_CENTER);
	w = rw;
	dlg->xw = w + 2 * DIALOG_LB;
	dlg->yw = y + 2 * DIALOG_TB;
	center_dlg(dlg);
	draw_dlg(dlg);
	y = dlg->y + DIALOG_TB;
	
	y++;
	dlg_format_box(term, term, &dlg->items[BM_BOX_IND], dlg->x + DIALOG_LB, &y, w, NULL, AL_LEFT);
	y++;
	dlg_format_buttons(term, term, &dlg->items[0], 5, dlg->x + DIALOG_LB, &y, w, NULL, AL_CENTER);
}


void launch_bm_add_doc_dialog(struct terminal *,struct dialog_data *, struct session *);
/* Callback for the "add" button in the bookmark manager */
int push_add_button(struct dialog_data *dlg, struct dialog_item_data *di) {
	launch_bm_add_doc_dialog(dlg->win->term, dlg, (struct session *)dlg->dlg->udata);
	return 0;
}


/* Called when the goto button is pushed */
int push_goto_button(struct dialog_data *dlg, struct dialog_item_data *goto_btn) {
	bookmark_id id;
	struct dlg_data_item_data_box *box;
	
	box = (struct dlg_data_item_data_box*)(dlg->dlg->items[BM_BOX_IND].data);
	
	/* Follow the bookmark */
	id = bookmark_dlg_box_id_get(box);
	if (id != BAD_BOOKMARK_ID) 
		bookmark_goto(id, (struct session*)goto_btn->item->udata);
	/* FIXME There really should be some feedback to the user here */

	/* Close the bookmark dialog */
	delete_window(dlg->win);
	return 0;
}

/* Called when an edit is complete. */
void bookmark_edit_done(struct dialog *d) {
	bookmark_id id = (bookmark_id)d->udata2;
	struct dialog_data *parent;
	
	bookmark_update(id, d->items[0].data, d->items[1].data);

	parent = d->udata;

	/* Tell the bookmark dialog to redraw */
	if (parent) 
		bookmark_dlg_list_update(&(((struct dlg_data_item_data_box*)parent->dlg->items[BM_BOX_IND].data)->items));
}

/* Called when the edit button is pushed */
int push_edit_button(struct dialog_data *dlg, struct dialog_item_data *edit_btn) {
	bookmark_id id;
	struct dlg_data_item_data_box *box;
	
	box = (struct dlg_data_item_data_box*)(dlg->dlg->items[BM_BOX_IND].data);
	
	/* Follow the bookmark */
	id = bookmark_dlg_box_id_get(box);
	if (id != BAD_BOOKMARK_ID) {
		const unsigned char *name = bookmark_get_name(id);
		const unsigned char *url = bookmark_get_url(id);

		bookmark_edit_dialog(dlg->win->term, TEXT_(T_EDIT_BOOKMARK), name, url, (struct session*)edit_btn->item->udata, dlg, bookmark_edit_done, (void *)id);
	}
	/* FIXME There really should be some feedback to the user here */
	return 0;
}


/* Used to carry extra info between the push_delete_button() and the really_del_bookmark_ */
struct push_del_button_hop_struct {
	struct dialog *dlg;
	struct dlg_data_item_data_box *box;
	bookmark_id id;
};


/* Called to _really_ delete a bookmark (a confirm in the delete dialog) */
void really_del_bookmark(void *vhop) {
	struct push_del_button_hop_struct *hop;
	int last;

	hop = (struct push_del_button_hop_struct *)vhop;
	
	if (!delete_bookmark_by_id(hop->id)) 
		return;

	last = bookmark_dlg_list_update(&(hop->box->items));
	/* In case we deleted the last bookmark */
	if (hop->box->sel >= (last - 1))
		hop->box->sel = last - 1;

	/* Made in push_delete_button() */
	/*mem_free(vhop);*/
}


/* Callback for the "delete" button in the bookmark manager */
int push_delete_button(struct dialog_data *dlg, struct dialog_item_data *some_useless_delete_button) {
	struct bookmark *bm;
	struct push_del_button_hop_struct *hop;
	struct terminal *term; 
	struct dlg_data_item_data_box *box;

	/* FIXME There's probably a nicer way to do this */
	term = dlg->win->term;

	box = (struct dlg_data_item_data_box*)(dlg->dlg->items[BM_BOX_IND].data);

	bm = get_bookmark_by_id(bookmark_dlg_box_id_get(box));
	
	if (bm == NULL) 
		return 0;


	/* Deleted in really_del_bookmark() */
	hop = mem_alloc(sizeof(struct push_del_button_hop_struct));
		
	hop->id = bm->id;
	hop->dlg = dlg->dlg;
	hop->box = box;
    
	msg_box(term, getml(hop, NULL), TEXT_(T_DELETE_BOOKMARK), AL_CENTER | AL_EXTD_TEXT, TEXT_(T_DELETE_BOOKMARK), " \"", bm->title, "\" (", TEXT_(T_url), ": \"", bm->url, "\")?", NULL, hop, 2, TEXT_(T_YES), really_del_bookmark, B_ENTER, TEXT_(T_NO), NULL, B_ESC);
	return 0;
}

/* Builds the "Bookmark manager" dialog */
void menu_bookmark_manager(struct terminal *term, void *fcp, struct session *ses)
{
	struct dialog *d;
	
	/* Create the dialog */
	d = mem_alloc(sizeof(struct dialog) + 7 * sizeof(struct dialog_item) + sizeof(struct bookmark) + 2 * MAX_STR_LEN);
	
	memset(d, 0, sizeof(struct dialog) + 7 * sizeof(struct dialog_item) + sizeof(struct bookmark) + 2 * MAX_STR_LEN);
	
	d->title = TEXT_(T_BOOKMARK_MANAGER);
	d->fn = layout_bookmark_manager;
	d->handle_event = bookmark_dialog_event_handler;
	d->abort = bookmark_dialog_abort_handler;
/*	bookmark_build_dlg_list(d);*/	/* Where the currently displayed list goes */
	d->udata = ses;

	d->items[0].type = D_BUTTON;
	d->items[0].gid = B_ENTER;
	d->items[0].fn = push_goto_button;
	d->items[0].udata = ses;
	d->items[0].text = TEXT_(T_GOTO);
	
	d->items[1].type = D_BUTTON;
	d->items[1].gid = B_ENTER;
	d->items[1].fn = push_edit_button;
	d->items[1].udata = ses;
	d->items[1].text = TEXT_(T_EDIT);
	
	d->items[2].type = D_BUTTON;
	d->items[2].gid = B_ENTER;
	d->items[2].fn = push_delete_button;
	d->items[2].text = TEXT_(T_DELETE);
	
	d->items[3].type = D_BUTTON;
	d->items[3].gid = B_ENTER;
	d->items[3].fn = push_add_button;
	d->items[3].text = TEXT_(T_ADD);
	
	d->items[4].type = D_BUTTON;
	d->items[4].gid = B_ESC;
	d->items[4].fn = cancel_dialog;
	d->items[4].text = TEXT_(T_CLOSE);

	d->items[5].type = D_BOX;	/* MP: D_BOX is nonsence. I tried to remove it, but didn't succeed */
	d->items[5].gid = 12;
	/*d->items[5].data = (void *)bookmark_dlg_box_build();*/	/* Where the currently displayed list goes */
	bookmark_dlg_box_build((struct dlg_data_item_data_box**)(void *)&(d->items[5].data));	/* Where the currently displayed list goes */
	
	d->items[6].type = D_END;
	do_dialog(term, d, getml(d, NULL));
}

/****************************************************************************
*
* Bookmark add dialog
*
****************************************************************************/

/* Adds the bookmark */
void bookmark_add_add(struct dialog *d)
{
	struct dialog_data *parent;
	
	add_bookmark(d->items[0].data, d->items[1].data);

	parent = d->udata;

	/* Tell the bookmark dialog to redraw */
	if (parent) 
		bookmark_dlg_list_update(&(((struct dlg_data_item_data_box*)parent->dlg->items[BM_BOX_IND].data)->items));
}


void launch_bm_add_doc_dialog(struct terminal *term,struct dialog_data *parent,struct session *ses) {
			
	bookmark_edit_dialog(term, TEXT_(T_ADD_BOOKMARK), NULL, NULL, ses, parent, bookmark_add_add, NULL);
}

/* Called to launch an add dialog on the current link */
void launch_bm_add_link_dialog(struct terminal *term,struct dialog_data *parent,struct session *ses) {
	unsigned char url[MAX_STR_LEN];

	/* FIXME: Logic error -- if there is no current link, 
	 * get_current_link_url() will return NULL, which will cause 
	 * bookmark_add_dialog() to try and use the current document's url. 
	 * Instead, it should use "". 
	 */ 
	bookmark_edit_dialog(term, TEXT_(T_ADD_BOOKMARK), NULL, get_current_link_url(ses, url, MAX_STR_LEN), ses, parent, bookmark_add_add, NULL);
}



unsigned char *bm_add_msg[] = {
	TEXT_(T_NNAME),
	TEXT_(T_URL),
};

/* Called to setup the add bookmark dialog */
void layout_add_dialog(struct dialog_data *dlg)
{
	int max = 0, min = 0;
	int w, rw;
	int y = -1;
	struct terminal *term;

	term = dlg->win->term;
	
	max_text_width(term, bm_add_msg[0], &max);
	min_text_width(term, bm_add_msg[0], &min);
	max_text_width(term, bm_add_msg[1], &max);
	min_text_width(term, bm_add_msg[1], &min);
	max_buttons_width(term, dlg->items + 2, 2, &max);
	min_buttons_width(term, dlg->items + 2, 2, &min);
	w = term->x * 9 / 10 - 2 * DIALOG_LB;
	
	if (w > max) w = max;
	if (w < min) w = min;
	if (w > term->x - 2 * DIALOG_LB) w = term->x - 2 * DIALOG_LB;
	if (w < 1) w = 1;

	w = rw = 50;
	
	dlg_format_text(NULL, term, bm_add_msg[0], 0, &y, w, &rw, COLOR_DIALOG_TEXT, AL_LEFT);
	y += 2;
	dlg_format_text(NULL, term, bm_add_msg[1], 0, &y, w, &rw, COLOR_DIALOG_TEXT, AL_LEFT);
	y += 2;
	dlg_format_buttons(NULL, term, dlg->items + 2, 2, 0, &y, w, &rw, AL_CENTER);
	w = rw;
	dlg->xw = w + 2 * DIALOG_LB;
	dlg->yw = y + 2 * DIALOG_TB;
	center_dlg(dlg);
	draw_dlg(dlg);
	y = dlg->y + DIALOG_TB;
	dlg_format_text(term, term, bm_add_msg[0], dlg->x + DIALOG_LB, &y, w, NULL, COLOR_DIALOG_TEXT, AL_LEFT);
	dlg_format_field(NULL, term, &dlg->items[0], dlg->x + DIALOG_LB, &y, w, NULL, AL_LEFT);
	y++;
	dlg_format_text(term, term, bm_add_msg[1], dlg->x + DIALOG_LB, &y, w, NULL, COLOR_DIALOG_TEXT, AL_LEFT);
	dlg_format_field(term, term, &dlg->items[1], dlg->x + DIALOG_LB, &y, w, NULL, AL_LEFT);
	y++;
	dlg_format_buttons(term, term, &dlg->items[2], 2, dlg->x + DIALOG_LB, &y, w, NULL, AL_CENTER);
	
}

/* Edits a bookmark's fields. 
 * If parent is defined, then that points to a dialog that should be sent 
 * an update when the add is done.
 *
 * If either of src_name or src_url are NULL, try to obtain the name and url 
 * of the current document. If you want to create two null fields, pass in a 
 * pointer to a zero length string ("").
*/
void bookmark_edit_dialog(
		struct terminal *term /* Terminal to write on. */, 
		unsigned char *title /* Title of the dialog. */, 
		const unsigned char *src_name /* Pointer to name to use. (can be null)*/, 
		const unsigned char *src_url /* Url to use. (can be null) */, 
		struct session *ses, 
		struct dialog_data *parent /* The parent window launching this one. */,
		void when_done(struct dialog *) /* Function to execute on 'ok'. */, 
		void *done_data /* Spare data to pass to when_done. Stored in udata2 */
) {
	unsigned char *name, *url;

	struct dialog *d;
	
	/* Create the dialog */
	d = mem_alloc(sizeof(struct dialog) + 5 * sizeof(struct dialog_item) + sizeof(struct extension) + 2 * MAX_STR_LEN);
	memset(d, 0, sizeof(struct dialog) + 5 * sizeof(struct dialog_item) + sizeof(struct extension) + 2 * MAX_STR_LEN);

	name = (unsigned char *)&d->items[5];
	url = name + MAX_STR_LEN;

	/* Get the name */
	if (src_name == NULL) {
		/* Unknown name. */
		get_current_title(ses, name, MAX_STR_LEN);
	} else {
		/* Known name. */
		safe_strncpy(name, src_name, MAX_STR_LEN);
	}

	/* Get the url */
	if (src_url == NULL) {
		/* Unknown . */
		get_current_url(ses, url, MAX_STR_LEN);
	} else {
		/* Known url. */
		safe_strncpy(url, src_url, MAX_STR_LEN);
	}

	d->title = title;
	d->fn = layout_add_dialog;
	d->refresh = (void (*)(void *))when_done;
	d->refresh_data = d;
	d->udata = parent;
	d->udata2 = done_data;

	d->items[0].type = D_FIELD;
	d->items[0].dlen = MAX_STR_LEN;
	d->items[0].data = name;
	d->items[0].fn = check_nonempty;

	d->items[1].type = D_FIELD;
	d->items[1].dlen = MAX_STR_LEN;
	d->items[1].data = url;
	d->items[1].fn = check_nonempty;

	d->items[2].type = D_BUTTON;
	d->items[2].gid = B_ENTER;
	d->items[2].fn = ok_dialog;
	d->items[2].text = TEXT_(T_OK);
	
	d->items[3].type = D_BUTTON;
	d->items[3].gid = B_ESC;
	d->items[3].text = TEXT_(T_CANCEL);
	d->items[3].fn = cancel_dialog;
	
	d->items[4].type = D_END;
	
	do_dialog(term, d, getml(d, NULL));
}
