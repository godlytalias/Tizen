#include <tizen.h>
#include <sqlite3.h>
#include "bible.h"

static void
ctxpopup_dismissed_cb(void *data, Evas_Object *obj, void *event_info)
{
	evas_object_del(data);
	data = NULL;
}

void
_popup_del(void *data, Evas_Object *obj, void *event_info)
{
	evas_object_hide(data);
	evas_object_del(data);
	data = NULL;
}

void
move_more_ctxpopup(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
	Evas_Object *win;
	Evas_Coord w, h;
	int pos = -1;
	Evas_Object *ctxpopup = (Evas_Object*)data;

	/* We convince the top widget is a window */
	win = elm_object_top_widget_get(ctxpopup);
	elm_win_screen_size_get(win, NULL, NULL, &w, &h);
	pos = elm_win_rotation_get(win);

	switch (pos) {
		case 0:
		case 180:
			evas_object_move(ctxpopup, (w / 2), h);
			break;
		case 90:
			evas_object_move(ctxpopup,  (h / 2), w);
			break;
		case 270:
			evas_object_move(ctxpopup, (h / 2), w);
			break;
	}
}

static int
_get_verse(void *data, int argc, char **argv, char **azColName)
{
	bible_verse_item *verse_item = (bible_verse_item*)data;
	verse_item->verse = strdup(argv[0]);
	return 0;
}

static int
_get_bookmarks_list(void *data, int argc, char **argv, char **azColName)
{
    bible_verse_item *verse_item = malloc(sizeof(bible_verse_item));
    appdata_s *ad = (appdata_s*)data;
    verse_item->appdata = ad;
    if (azColName[0] && (strcmp(azColName[0], "bookcount") == 0))
    {
    	verse_item->bookcount = atoi(argv[0]);
    }

    if (azColName[1] && (strcmp(azColName[1], "chaptercount") == 0))
    {
    	verse_item->chaptercount = atoi(argv[1]);
    }

    if (azColName[2] && (strcmp(azColName[2], "versecount") == 0))
    {
    	verse_item->versecount = atoi(argv[2]);
    }

    verse_item->it = elm_genlist_item_append(ad->bookmarks_genlist, ad->bookmarks_itc, verse_item, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
    elm_object_item_data_set(verse_item->it, verse_item);

    return 0;
}

static Evas_Object*
gl_content_get_cb(void *data, Evas_Object *obj, const char *part)
{
    bible_verse_item *verse_item = (bible_verse_item*)data;
    if(strcmp(part, "elm.swallow.content") == 0)
    {
    	Evas_Object *layout = elm_layout_add(obj);
    	char verse_ref[64];
    	elm_layout_file_set(layout, verse_item->appdata->edj_path, "bookmark_verse_layout");
     	elm_object_part_text_set(layout,"elm.text.verse",verse_item->verse);
    	sprintf(verse_ref, "%s %d : %d", Books[verse_item->bookcount], verse_item->chaptercount, verse_item->versecount + 1);
    	elm_object_part_text_set(layout, "elm.text.reference", verse_ref);
    	evas_object_show(layout);
    	evas_object_smart_calculate(layout);
    	return layout;
    }
    else return NULL;
}

static void
_remove_bookmark(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup = (Evas_Object*)data;
	char query[256];
	bible_verse_item *verse_item = (bible_verse_item*)evas_object_data_get(popup, "verse_item");
    sprintf(query, "DELETE FROM bookmarks WHERE bookcount = %d AND chaptercount = %d AND versecount = %d", verse_item->bookcount, verse_item->chaptercount, verse_item->versecount);
    _app_database_query(query, NULL, NULL);
    _check_bookmarks(verse_item->appdata);
    Evas_Object *toast = elm_popup_add(verse_item->appdata->win);
    elm_object_style_set(toast, "toast");
    sprintf(query, "Removed %s %d : %d", Books[verse_item->bookcount], verse_item->chaptercount, verse_item->versecount + 1);
    elm_object_text_set(toast, query);
    elm_popup_allow_events_set(toast, EINA_TRUE);
    evas_object_show(toast);
    elm_popup_timeout_set(toast, 2.0);
    evas_object_smart_callback_add(toast, "timeout", _popup_del, toast);
    elm_genlist_realized_items_update(verse_item->appdata->genlist);
    elm_object_item_del(verse_item->it);
    _popup_del(popup, NULL, NULL);
}

void
_get_chapter(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup = (Evas_Object*)data;
	bible_verse_item *verse_item = (bible_verse_item*)evas_object_data_get(popup, "verse_item");
	_query_chapter(verse_item->appdata, verse_item->bookcount, verse_item->chaptercount);
	elm_naviframe_item_pop(verse_item->appdata->naviframe);
	_show_verse(verse_item->appdata, verse_item->versecount);
	_popup_del(popup, NULL, NULL);
}

static void
gl_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item*)event_info;
	elm_genlist_item_selected_set(it, EINA_FALSE);
    char popup_text[128];
    bible_verse_item *verse_item = (bible_verse_item*)elm_object_item_data_get(it);
    appdata_s *ad = (appdata_s*)data;
    Evas_Object *popup = elm_popup_add(ad->win);
    elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, 0.5);
    sprintf(popup_text, "<align='center'>Go to %s %d : %d ?</align>", Books[verse_item->bookcount], verse_item->chaptercount, verse_item->versecount + 1);
    elm_object_text_set(popup, popup_text);
    Evas_Object *button1 = elm_button_add(ad->win);
    elm_object_text_set(button1, "No");
    evas_object_smart_callback_add(button1, "clicked", _popup_del, popup);
    Evas_Object *button2 = elm_button_add(ad->win);
    elm_object_text_set(button2, "Yes");
    evas_object_smart_callback_add(button2, "clicked", _get_chapter, popup);
    evas_object_data_set(popup, "verse_item", verse_item);
    elm_object_part_content_set(popup, "button1", button1);
    elm_object_part_content_set(popup, "button2", button2);
    eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, eext_popup_back_cb, popup);
    evas_object_show(popup);
}

static void
gl_longpressed_cb(void *data, Evas_Object *obj, void *event_info)
{
    Elm_Object_Item *item = (Elm_Object_Item*)event_info;
    char popup_text[128];
    bible_verse_item *verse_item = (bible_verse_item*)elm_object_item_data_get(item);
    appdata_s *ad = (appdata_s*)data;
    Evas_Object *popup = elm_popup_add(ad->win);
	elm_genlist_item_selected_set(item, EINA_FALSE);
    elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, 0.5);
    sprintf(popup_text, "<align='center'>Remove %s %d : %d ?</align>", Books[verse_item->bookcount], verse_item->chaptercount, verse_item->versecount + 1);
    elm_object_text_set(popup, popup_text);
    elm_object_part_text_set(popup, "title,text", "Bookmark!");
    Evas_Object *button1 = elm_button_add(ad->win);
    elm_object_text_set(button1, "No");
    evas_object_smart_callback_add(button1, "clicked", _popup_del, popup);
    Evas_Object *button2 = elm_button_add(ad->win);
    elm_object_text_set(button2, "Yes");
    evas_object_smart_callback_add(button2, "clicked", _remove_bookmark, popup);
    evas_object_data_set(popup, "verse_item", verse_item);
    elm_object_part_content_set(popup, "button1", button1);
    elm_object_part_content_set(popup, "button2", button2);
    eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, eext_popup_back_cb, popup);
    evas_object_show(popup);
}

static Evas_Object*
_get_bookmarks(appdata_s *ad)
{
	char query[512];
	Elm_Object_Item *it;
	bible_verse_item *verse_item;
	Evas_Object *layout = elm_layout_add(ad->naviframe);
	elm_layout_file_set(layout, ad->edj_path, "bookmarks_layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	ad->bookmarks_genlist = elm_genlist_add(layout);
	evas_object_size_hint_weight_set(ad->bookmarks_genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(ad->bookmarks_genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_layout_content_set(layout, "elm.swallow.content", ad->bookmarks_genlist);
	evas_object_smart_callback_add(ad->bookmarks_genlist, "selected", gl_selected_cb, ad);
	evas_object_smart_callback_add(ad->bookmarks_genlist, "longpressed", gl_longpressed_cb, ad);

	ad->bookmarks_itc = elm_genlist_item_class_new();
	ad->bookmarks_itc->item_style = "full";
	ad->bookmarks_itc->func.content_get = gl_content_get_cb;
	ad->bookmarks_itc->func.text_get = NULL;
	ad->bookmarks_itc->func.del = gl_del_cb;

	sprintf(query, "SELECT bookcount, chaptercount, versecount FROM bookmarks");
    _app_database_query(query, _get_bookmarks_list, ad);

    it = elm_genlist_first_item_get(ad->bookmarks_genlist);
    while(it) {
       verse_item = elm_object_item_data_get(it);
       sprintf(query, "SELECT e_verse FROM eng_bible WHERE Book = '%s' AND Chapter = %d AND Versecount = %d", Books[verse_item->bookcount], verse_item->chaptercount, verse_item->versecount + 1);
       _database_query(query, _get_verse, verse_item);
       it = elm_genlist_item_next_get(it);
    }
	evas_object_show(layout);
	return layout;
}

static Eina_Bool
_genlist_free_idler(void *data)
{
	appdata_s *ad = (appdata_s*)data;
	if (ad->bookmarks_genlist)
		elm_genlist_clear(ad->bookmarks_genlist);
	ad->bookmarks_genlist = NULL;
	if (ad->bookmarks_itc)
		elm_genlist_item_class_free(ad->bookmarks_itc);
	ad->bookmarks_itc = NULL;
	return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool
naviframe_pop_cb(void *data, Elm_Object_Item *it)
{
	appdata_s *ad = (appdata_s*)data;
	ecore_idler_add(_genlist_free_idler, data);
	_loading_progress(ad->win);
	return EINA_TRUE;
}

static void
ctxpopup_item_select_cb(void *data, Evas_Object *obj, void *event_info)
{
	const char *title_label = elm_object_item_text_get((Elm_Object_Item *) event_info);
	char text_content[2048];
	Elm_Object_Item *nf_it;
	appdata_s *ad = (appdata_s*)data;

	if (!strcmp(title_label, "Search"))
	{
		_search_word(ad, NULL, NULL);
		_popup_del(obj, NULL, NULL);
		return;
	}

	if (!strcmp(title_label, "Bookmarks"))
	{
	   nf_it = elm_naviframe_item_push(ad->naviframe, "Bookmarks", NULL, NULL, _get_bookmarks(ad), NULL);
	   elm_naviframe_item_pop_cb_set(nf_it, naviframe_pop_cb, ad);
		_popup_del(obj, NULL, NULL);
       return;
	}

	if (!strcmp(title_label, "Select Chapter"))
	{
	   _change_book(data, ad->layout, NULL, NULL);
		_popup_del(obj, NULL, NULL);
       return;
	}

	Evas_Object *popup = elm_popup_add(elm_object_top_widget_get(obj));
	elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, 1.0);
	elm_object_part_text_set(popup, "title,text", title_label);
	Evas_Object *content_box = elm_box_add(popup);
	evas_object_size_hint_weight_set(content_box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(content_box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	if (!strcmp(title_label, "About"))
	{
		Evas_Object *label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		sprintf(text_content,_("<align=center><b>HOLY BIBLE (MKJV)</b></align>"));
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		sprintf(text_content,_("<align=center><font_size=20>Copyright © 2015</font_size></align>"));
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		sprintf(text_content,_("<align=center><em><font_size=20>GTA v0.3</font_size></em></align>"));
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		sprintf(text_content,_(" "));
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		sprintf(text_content, _("<align=center><font_size=20>"
	"This program is free software: you can redistribute it and/or modify "
	"it under the terms of the GNU General Public License as published by "
	"the Free Software Foundation, either version 3 of the License, or "
	"(at your option) any later version.</font_size></align>"));

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		sprintf(text_content, _("<align=center><font_size=20>"
		"This program is distributed in the hope that it will be useful, "
		"but WITHOUT ANY WARRANTY; without even the implied warranty of "
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the "
		"GNU General Public License for more details.</font_size></align>"));

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		sprintf(text_content,_(" "));
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		sprintf(text_content, _("<align=center><font_size=15>"
				"Report the bugs or suggestions to "
				"Godly T.Alias (<em>godlytalias@yahoo.co.in</em>).</font_size></align>"));

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);
	}
	else if (!strcmp(title_label, "Help"))
	{
		Evas_Object *label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		sprintf(text_content,_("<align=center><b>HOLY BIBLE (MKJV)</b></align>"));
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		sprintf(text_content,_("<align=center><font_size=20>Copyright © 2015</font_size></align>"));
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		sprintf(text_content,_("<align=center><em><font_size=20>GTA v0.3</font_size></em></align>"));
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		sprintf(text_content,_(" "));
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		sprintf(text_content, _("<align=left><font_size=25>"
	"<b>Changing chapters</b></font_size></align>"));

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);


		sprintf(text_content, _("<align=left><font_size=20>"
		"Users can change the chapters by swiping "
		"left / right in the screen. Users can also click on the arrows in the bottom corners to go to previous or next chapters. "
		"Also if user clicks on the header part of home screen, a new window will be opened "
		"listing out all the Books and the chapters in the selected book. "
		"User can select the Book and Chapter which they want.</font_size></align>"));

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		sprintf(text_content,_(" "));
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		sprintf(text_content, _("<align=left><font_size=25>"
			"<b>Searching keywords</b></font_size></align>"));

				label = elm_label_add(popup);
				evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
				evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
				elm_label_line_wrap_set(label, ELM_WRAP_WORD);
				elm_object_text_set(label, text_content);
				evas_object_show(label);
				elm_box_pack_end(content_box, label);


				sprintf(text_content, _("<align=left><font_size=20>"
				"User can search a specific keyword and get the verses containing those keywords. "
				"The Search screen can be opened through 'Search' option in application menu. "
				"Keywords can be entered in the text field and press 'Go' to get the list of verses containing the entered keywords. "
				"User can search with more than one keyword seperated by space and can get results which contain all the enetered keywords. "
				"If clicked on a search result item a popup will come with the full verse. "
				"Also if user long press on a search item, user can get option to go to the full chapter of the verse. </font_size></align>"));

				label = elm_label_add(popup);
				evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
				evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
				elm_label_line_wrap_set(label, ELM_WRAP_WORD);
				elm_object_text_set(label, text_content);
				evas_object_show(label);
				elm_box_pack_end(content_box, label);

				label = elm_label_add(popup);
				evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
				evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
				elm_label_line_wrap_set(label, ELM_WRAP_WORD);
				sprintf(text_content,_(" "));
				elm_object_text_set(label, text_content);
				evas_object_show(label);
				elm_box_pack_end(content_box, label);

		sprintf(text_content, _("<align=left><font_size=25>"
	"<b>Bookmark verse</b></font_size></align>"));

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		sprintf(text_content, _("<align=left><font_size=20>"
		"User can long press / double click on a verse and will get options to Bookmark a verse. "
		"Bookmarked verses will be displayed in red font and with a red strip in left side of verse. "
		"If user want to remove bookmark of a verse, user can go to the bookmarks list in menu option and "
		"can remove bookmark by long pressing on the verse to be removed from bookmark list or by selecting "
		"Remove Bookmark option from the menu on double clicking or long pressing the respective verse on the reading screen.</font_size></align>"));

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		sprintf(text_content,_(" "));
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		sprintf(text_content, _("<align=left><font_size=25>"
	"<b>Share / Copy verses</b></font_size></align>"));

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		sprintf(text_content, _("<align=left><font_size=20>"
		"User can share or copy the desired verses by long pressing / double click on verses. "
		"Reference of verse also will get appended to verse automatically. "
		"Users can copy more than one verses one by one and can get the verses from clipboard.</font_size></align>"));

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		sprintf(text_content,_(" "));
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		sprintf(text_content, _("<align=center><font_size=15>"
		"Report the bugs or suggestions to "
		"Godly T.Alias (<em>godlytalias@yahoo.co.in</em>).</font_size></align>"));

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);
	}
	elm_object_content_set(popup, content_box);
	evas_object_show(content_box);
	Evas_Object *button = elm_button_add(popup);
	elm_object_text_set(button, "OK");
	elm_object_part_content_set(popup, "button1", button);
	evas_object_show(button);
	evas_object_show(popup);
	evas_object_smart_callback_add(button, "clicked", _popup_del, popup);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, eext_popup_back_cb, NULL);
	elm_ctxpopup_dismiss(obj);
}

void
create_ctxpopup_more_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *win;
	appdata_s *ad = (appdata_s*)data;

	Evas_Object *ctxpopup = elm_ctxpopup_add(ad->naviframe);
	elm_ctxpopup_auto_hide_disabled_set(ctxpopup, EINA_TRUE);
	elm_object_style_set(ctxpopup, "more/default");
	eext_object_event_callback_add(ctxpopup, EEXT_CALLBACK_BACK, eext_ctxpopup_back_cb, NULL);
	eext_object_event_callback_add(ctxpopup, EEXT_CALLBACK_MORE, eext_ctxpopup_back_cb, NULL);
	evas_object_smart_callback_add(ctxpopup, "dismissed", ctxpopup_dismissed_cb, ctxpopup);

	win = elm_object_top_widget_get(ad->naviframe);
	evas_object_smart_callback_add(win, "rotation,changed", move_more_ctxpopup, ctxpopup);

	elm_ctxpopup_item_append(ctxpopup, "Search", NULL, ctxpopup_item_select_cb, ad);
	elm_ctxpopup_item_append(ctxpopup, "Bookmarks", NULL, ctxpopup_item_select_cb, ad);
	elm_ctxpopup_item_append(ctxpopup, "Select Chapter", NULL, ctxpopup_item_select_cb, ad);
	elm_ctxpopup_item_append(ctxpopup, "Help", NULL, ctxpopup_item_select_cb, ad);
	elm_ctxpopup_item_append(ctxpopup, "About", NULL, ctxpopup_item_select_cb, ad);

	elm_ctxpopup_direction_priority_set(ctxpopup, ELM_CTXPOPUP_DIRECTION_UP, ELM_CTXPOPUP_DIRECTION_UNKNOWN, ELM_CTXPOPUP_DIRECTION_UNKNOWN, ELM_CTXPOPUP_DIRECTION_UNKNOWN);
	move_more_ctxpopup(ctxpopup, ctxpopup, NULL);
	evas_object_show(ctxpopup);
}
