#include <tizen.h>
#include <sqlite3.h>
#include "bible.h"

void
_popup_del(void *data, Evas_Object *obj, void *event_info)
{
	evas_object_hide(data);
	evas_object_del(data);
	data = NULL;
}

static void
_ctxpopup_dismiss_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = (appdata_s*)data;
	elm_ctxpopup_clear(ad->menu_ctxpopup);
	evas_object_hide(ad->menu_ctxpopup);
	evas_object_del(ad->menu_ctxpopup);
	ad->menu_ctxpopup = NULL;
}

static void
_rate_app_cb(void *data, Evas_Object *obj, void *event_info)
{
	_popup_del(data, obj, event_info);

	char *app_id = NULL;
	char app_url[128];
	app_get_id(&app_id);
	sprintf(app_url, "tizenstore://ProductDetail/%s", app_id);
	app_control_h handler;
	app_control_create(&handler);
	app_control_set_app_id(handler, "org.tizen.tizenstore");
	app_control_set_operation(handler, APP_CONTROL_OPERATION_VIEW);
	app_control_set_uri(handler, app_url);
	app_control_send_launch_request(handler, NULL, NULL);
	app_control_destroy(handler);
	free(app_id);
}

static void
_report_cb(void *data, Evas_Object *obj, void *event_info)
{
	_popup_del(data, obj, event_info);

	char *app_name = NULL;
	char *app_version = NULL;
	char app_details[128];
	app_get_name(&app_name);
	app_get_version(&app_version);
	sprintf(app_details, "Bugs / Suggestions: %s %s", app_name, app_version);
	app_control_h handler;
	app_control_create(&handler);
	app_control_set_launch_mode(handler, APP_CONTROL_LAUNCH_MODE_GROUP);
	app_control_set_operation(handler, APP_CONTROL_OPERATION_COMPOSE);
	app_control_add_extra_data(handler, APP_CONTROL_DATA_SUBJECT, app_details);
	app_control_set_uri(handler, "mailto:godlytalias@yahoo.co.in");
	app_control_send_launch_request(handler, NULL, NULL);
	app_control_destroy(handler);
	free(app_name);
	free(app_version);
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

static void
_install_apps_cb(void *data, Evas_Object *obj, void *event_info)
{
	app_control_h app_control;
	app_control_create(&app_control);
	app_control_set_operation(app_control, APP_CONTROL_OPERATION_VIEW);
	app_control_set_app_id(app_control, "org.tizen.tizenstore");
	app_control_set_uri(app_control, "tizenstore://SellerApps/df2ezocp72");
	app_control_send_launch_request(app_control, NULL, NULL);
	app_control_destroy(app_control);
}

static void
_toggle_parallel_reading(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *pl_genlist = (Evas_Object*)data;
	appdata_s *ad = (appdata_s*)evas_object_data_get(pl_genlist, "appdata");
	if (strcmp(elm_object_text_get(obj), ON) == 0)
	{
		preference_set_boolean("parallel", true);
		elm_object_disabled_set(pl_genlist, EINA_FALSE);
		elm_object_text_set(obj, OFF);
		if (elm_genlist_items_count(pl_genlist) == 1)
			elm_genlist_item_selected_set(elm_genlist_first_item_get(pl_genlist), EINA_TRUE);
	}
	else
	{
		preference_set_boolean("parallel", false);
		preference_remove("parallel_app_id");
		Elm_Object_Item *item = elm_genlist_selected_item_get(pl_genlist);
		if (item) elm_genlist_item_selected_set(item, EINA_FALSE);
		elm_object_disabled_set(pl_genlist, EINA_TRUE);
		ad->parallel_db_path = NULL;
		elm_object_text_set(obj, ON);
		_query_chapter(ad, ad->cur_book, ad->cur_chapter);
	}
}

static Evas_Object*
_parallel_content_get_cb(void *data, Evas_Object *obj, const char *part)
{
	if (strcmp(part, "elm.swallow.end") == 0)
	{
		Evas_Object *check = elm_check_add(obj);
		evas_object_freeze_events_set(check, EINA_TRUE);
		evas_object_repeat_events_set(check, EINA_TRUE);
		evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_show(check);
		return check;
	}
	else return NULL;
}

static void
_parallel_gl_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	package_info_h pkg_info;
	Elm_Object_Item *item = (Elm_Object_Item*)event_info;
	char *db_path = NULL;
	char *temp_path = NULL;
	app_struct *app_det = (app_struct*)elm_object_item_data_get(item);
	appdata_s *ad = (appdata_s*)evas_object_data_get(obj, "appdata");
	Evas_Object *check = elm_object_item_part_content_get(item, "elm.swallow.end");
	elm_check_state_set(check, EINA_TRUE);
	if (ad->parallel_db_path)
	{
		temp_path = (char*)malloc(sizeof(char)*PATH_MAX);
		strcpy(temp_path, ad->parallel_db_path);
	}
	preference_set_boolean("parallel", true);
	preference_set_string("parallel_app_id", app_det->app_id);
	if (ad->parallel_db_path) free(ad->parallel_db_path);
	ad->parallel_db_path = NULL;
	if (package_manager_get_package_info(app_det->app_id, &pkg_info) != PACKAGE_MANAGER_ERROR_NONE)
	{
		if (db_path) free(db_path);
		if (temp_path) free(temp_path);
		Evas_Object *toast = elm_popup_add(ad->naviframe);
		elm_object_style_set(toast, "toast");
		elm_popup_timeout_set(toast, 3.0);
		elm_popup_allow_events_set(toast, EINA_TRUE);
		evas_object_smart_callback_add(toast, "timeout", _popup_del, toast);
		elm_object_text_set(toast, PARALLEL_READING_FAILED);
		evas_object_show(toast);
		return;
	}
	package_info_get_root_path(pkg_info, &db_path);
	ad->parallel_db_path = (char*)malloc(sizeof(char) * (strlen(db_path) + 32));
	strcpy(ad->parallel_db_path, db_path);
	strcat(ad->parallel_db_path, "/shared/res/holybible.db");
	if (db_path) free(db_path);
	package_info_destroy(pkg_info);
	if (temp_path && (strcmp(temp_path, ad->parallel_db_path) == 0))
	{
		free(temp_path);
		return;
	}
	if (temp_path) free(temp_path);
	_query_chapter(ad, ad->cur_book, ad->cur_chapter);
}

static void
_parallel_gl_unselected_cb(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *item = (Elm_Object_Item*)event_info;
	Evas_Object *check = elm_object_item_part_content_get(item, "elm.swallow.end");
	elm_check_state_set(check, EINA_FALSE);
}

static char*
_parallel_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	app_struct *app_det = (app_struct*)data;
	const char *text = app_det->app_name;
	if (strcmp(part, "elm.text") == 0)
		return strdup(text);
	else return NULL;
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

	if (azColName[3])
	{
		verse_item->verse = strdup(argv[3]);
	}

	verse_item->verse_s = NULL;
	verse_item->it = elm_genlist_item_append(ad->bookmarks_notes_genlist, ad->bookmarks_itc, verse_item, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
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
		char verse_ref[128];
		elm_layout_file_set(layout, verse_item->appdata->edj_path, "bookmark_verse_layout");
		elm_object_part_text_set(layout,"elm.text.verse",verse_item->verse);
		sprintf(verse_ref, "%s %d : %d", Books[verse_item->bookcount], verse_item->chaptercount, verse_item->versecount + 1);
		elm_object_part_text_set(layout, "elm.text.reference", verse_ref);
		evas_object_show(layout);
		return layout;
	}
	else return NULL;
}

static Evas_Object*
note_gl_content_get_cb(void *data, Evas_Object *obj, const char *part)
{
	bible_verse_item *verse_item = (bible_verse_item*)data;
	if(strcmp(part, "elm.swallow.content") == 0)
	{
		Evas_Object *layout = elm_layout_add(obj);
		char verse_ref[64];
		elm_layout_file_set(layout, verse_item->appdata->edj_path, "note_item_layout");
		elm_object_part_text_set(layout,"elm.text.verse",verse_item->verse);
		sprintf(verse_ref, "%s %d : %d", Books[verse_item->bookcount], verse_item->chaptercount, verse_item->versecount + 1);
		elm_object_part_text_set(layout, "elm.text.reference", verse_ref);
		evas_object_show(layout);
		return layout;
	}
	else return NULL;
}

static void
_remove_note(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup = (Evas_Object*)data;
	char query[256];
	bible_verse_item *verse_item = (bible_verse_item*)evas_object_data_get(popup, "verse_item");
	sprintf(query, "DELETE FROM notes WHERE bookcount = %d AND chaptercount = %d AND versecount = %d", verse_item->bookcount, verse_item->chaptercount, verse_item->versecount);
	_app_database_query(query, NULL, NULL);
	_check_notes(verse_item->appdata);
	verse_item->note = EINA_FALSE;
	Evas_Object *toast = elm_popup_add(verse_item->appdata->win);
	elm_object_style_set(toast, "toast");
	sprintf(query, NOTE_DELETED);
	elm_object_text_set(toast, query);
	elm_popup_allow_events_set(toast, EINA_TRUE);
	evas_object_show(toast);
	elm_popup_timeout_set(toast, 2.0);
	evas_object_smart_callback_add(toast, "timeout", _popup_del, toast);
	elm_genlist_realized_items_update(verse_item->appdata->genlist);
	if (verse_item->appdata->bookmarks_notes_genlist) {
		if (elm_genlist_items_count(verse_item->appdata->bookmarks_notes_genlist) == 1)
			elm_layout_signal_emit(verse_item->appdata->bookmark_note_layout, "elm,holy_bible,bg,show", "elm");
		elm_object_item_del(verse_item->it);
	}
	_popup_del(popup, NULL, NULL);
}

static void
_remove_bookmark(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup = (Evas_Object*)data;
	char query[256];
	bible_verse_item *verse_item = (bible_verse_item*)evas_object_data_get(popup, "verse_item");
	sprintf(query, "DELETE FROM bookmark WHERE bookcount = %d AND chaptercount = %d AND versecount = %d", verse_item->bookcount, verse_item->chaptercount, verse_item->versecount);
	_app_database_query(query, NULL, NULL);
	_check_bookmarks(verse_item->appdata);
	Evas_Object *toast = elm_popup_add(verse_item->appdata->win);
	elm_object_style_set(toast, "toast");
	sprintf(query, "%s %d : %d %s", Books[verse_item->bookcount], verse_item->chaptercount, verse_item->versecount + 1, REMOVED);
	elm_object_text_set(toast, query);
	elm_popup_allow_events_set(toast, EINA_TRUE);
	evas_object_show(toast);
	elm_popup_timeout_set(toast, 2.0);
	evas_object_smart_callback_add(toast, "timeout", _popup_del, toast);
	elm_genlist_realized_items_update(verse_item->appdata->genlist);
	if (elm_genlist_items_count(verse_item->appdata->bookmarks_notes_genlist) == 1) {
		elm_layout_signal_emit(verse_item->appdata->bookmark_note_layout, "elm,holy_bible,bg,show", "elm");
	}
	elm_object_item_del(verse_item->it);
	_popup_del(popup, NULL, NULL);
}

void
_get_chapter(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup = (Evas_Object*)data;
	bible_verse_item *verse_item = (bible_verse_item*)evas_object_data_get(popup, "verse_item");
	_query_chapter(verse_item->appdata, verse_item->bookcount, verse_item->chaptercount);
	Elm_Object_Item *nf_item = elm_naviframe_bottom_item_get(verse_item->appdata->naviframe);
	elm_naviframe_item_pop_to(nf_item);
	_show_verse(verse_item->appdata, verse_item->versecount);
	_get_chapter_count_query(verse_item->appdata, verse_item->bookcount);
	_popup_del(popup, NULL, NULL);
	elm_object_focus_set(verse_item->appdata->genlist, EINA_TRUE);
}

static void
note_gl_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item*)event_info;
	int fontsize = 25;
	char style[256];
	elm_genlist_item_selected_set(it, EINA_FALSE);
	char popup_text[128];
	bible_verse_item *verse_item = (bible_verse_item*)elm_object_item_data_get(it);
	appdata_s *ad = (appdata_s*)data;
	Evas_Object *popup = elm_popup_add(ad->win);
	elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, 1.0);
	sprintf(popup_text, "%s %d : %d", Books[verse_item->bookcount], verse_item->chaptercount, verse_item->versecount + 1);
	elm_object_part_text_set(popup, "title,text", popup_text);
	Evas_Object *layout = elm_layout_add(popup);
	elm_layout_file_set(layout, ad->edj_path, "standard_layout");
	Evas_Object *entry = elm_entry_add(popup);
	elm_entry_editable_set(entry, EINA_FALSE);
	preference_get_int("fontsize", &fontsize);
	sprintf(style, "DEFAULT='font=Tizen:style=Regular align=left font_size=%d color=#000000 wrap=mixed'hilight=' + font_weight=Bold'", fontsize + 5);
	elm_entry_text_style_user_push(entry, style);
	elm_entry_entry_set(entry, verse_item->verse);
	elm_layout_content_set(layout, "elm.swallow.content", entry);
	elm_object_content_set(popup, layout);
	Evas_Object *button1 = elm_button_add(ad->win);
	elm_object_text_set(button1, CLOSE);
	evas_object_smart_callback_add(button1, "clicked", _popup_del, popup);
	Evas_Object *button2 = elm_button_add(ad->win);
	elm_object_text_set(button2, GET_VERSE);
	evas_object_smart_callback_add(button2, "clicked", _get_chapter, popup);
	evas_object_data_set(popup, "verse_item", verse_item);
	elm_object_part_content_set(popup, "button1", button1);
	elm_object_part_content_set(popup, "button2", button2);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, eext_popup_back_cb, popup);
	evas_object_show(popup);
}

void
note_remove_cb(void *data, Evas_Object *obj, void *event_info)
{
	char popup_text[128];
	bible_verse_item *verse_item;
	if (data)
		verse_item = (bible_verse_item*)data;
	else
	{
		Elm_Object_Item *item = (Elm_Object_Item*)event_info;
		verse_item = (bible_verse_item*)elm_object_item_data_get(item);
		elm_genlist_item_selected_set(item, EINA_FALSE);
	}
	Evas_Object *popup = elm_popup_add(verse_item->appdata->win);
	elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, 0.5);
	sprintf(popup_text, "<align='center'>%s ?</align>", DELETE_NOTE);
	elm_object_text_set(popup, popup_text);
	sprintf(popup_text, "<align='center'>%s %d : %d</align>", Books[verse_item->bookcount], verse_item->chaptercount, verse_item->versecount + 1);
	elm_object_part_text_set(popup, "title,text", popup_text);
	Evas_Object *button1 = elm_button_add(verse_item->appdata->win);
	elm_object_text_set(button1, NO);
	evas_object_smart_callback_add(button1, "clicked", _popup_del, popup);
	Evas_Object *button2 = elm_button_add(verse_item->appdata->win);
	elm_object_text_set(button2, YES);
	evas_object_smart_callback_add(button2, "clicked", _remove_note, popup);
	evas_object_data_set(popup, "verse_item", verse_item);
	elm_object_part_content_set(popup, "button1", button1);
	elm_object_part_content_set(popup, "button2", button2);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, eext_popup_back_cb, popup);
	evas_object_show(popup);
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
	sprintf(popup_text, "<align='center'>%s %d : %d - %s?</align>", Books[verse_item->bookcount], verse_item->chaptercount, verse_item->versecount + 1, GO_TO);
	elm_object_text_set(popup, popup_text);
	Evas_Object *button1 = elm_button_add(ad->win);
	elm_object_text_set(button1, NO);
	evas_object_smart_callback_add(button1, "clicked", _popup_del, popup);
	Evas_Object *button2 = elm_button_add(ad->win);
	elm_object_text_set(button2, YES);
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
	sprintf(popup_text, "<align='center'>%s %d : %d %s ?</align>", Books[verse_item->bookcount], verse_item->chaptercount, verse_item->versecount + 1, REMOVE);
	elm_object_text_set(popup, popup_text);
	elm_object_part_text_set(popup, "title,text", BOOKMARK);
	Evas_Object *button1 = elm_button_add(ad->win);
	elm_object_text_set(button1, NO);
	evas_object_smart_callback_add(button1, "clicked", _popup_del, popup);
	Evas_Object *button2 = elm_button_add(ad->win);
	elm_object_text_set(button2, YES);
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
	int res_count;
	Evas_Object *layout;
	ad->bookmark_note_layout = elm_layout_add(ad->naviframe);
	layout = ad->bookmark_note_layout;
	elm_layout_file_set(layout, ad->edj_path, "bookmarks_layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	ad->bookmarks_notes_genlist = elm_genlist_add(layout);
	elm_object_style_set(ad->bookmarks_notes_genlist, "handler");
	evas_object_size_hint_weight_set(ad->bookmarks_notes_genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(ad->bookmarks_notes_genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_layout_content_set(layout, "elm.swallow.content", ad->bookmarks_notes_genlist);
	evas_object_smart_callback_add(ad->bookmarks_notes_genlist, "selected", gl_selected_cb, ad);
	evas_object_smart_callback_add(ad->bookmarks_notes_genlist, "longpressed", gl_longpressed_cb, ad);

	ad->bookmarks_itc = elm_genlist_item_class_new();
	ad->bookmarks_itc->item_style = "full";
	ad->bookmarks_itc->func.content_get = gl_content_get_cb;
	ad->bookmarks_itc->func.text_get = NULL;
	ad->bookmarks_itc->func.del = gl_del_cb;

	sprintf(query, "SELECT bookcount, chaptercount, versecount, verse FROM bookmark");
	_app_database_query(query, _get_bookmarks_list, ad);

	res_count = elm_genlist_items_count(ad->bookmarks_notes_genlist);
	if (res_count > 0)
		elm_layout_signal_emit(layout, "elm,holy_bible,bg,hide", "elm");
	evas_object_show(layout);
	return layout;
}

static Evas_Object*
_get_notes(appdata_s *ad)
{
	char query[512];
	int res_count;
	Evas_Object *layout;
	ad->bookmark_note_layout = elm_layout_add(ad->naviframe);
	layout = ad->bookmark_note_layout;
	elm_layout_file_set(layout, ad->edj_path, "notes_layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	ad->bookmarks_notes_genlist = elm_genlist_add(layout);
	elm_object_style_set(ad->bookmarks_notes_genlist, "handler");
	evas_object_size_hint_weight_set(ad->bookmarks_notes_genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(ad->bookmarks_notes_genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_layout_content_set(layout, "elm.swallow.content", ad->bookmarks_notes_genlist);
	evas_object_smart_callback_add(ad->bookmarks_notes_genlist, "selected", note_gl_selected_cb, ad);
	evas_object_smart_callback_add(ad->bookmarks_notes_genlist, "longpressed", note_remove_cb, NULL);

	ad->bookmarks_itc = elm_genlist_item_class_new();
	ad->bookmarks_itc->item_style = "full";
	ad->bookmarks_itc->func.content_get = note_gl_content_get_cb;
	ad->bookmarks_itc->func.text_get = NULL;
	ad->bookmarks_itc->func.del = gl_del_cb;

	sprintf(query, "SELECT bookcount, chaptercount, versecount, note FROM notes");
	_app_database_query(query, _get_bookmarks_list, ad);

	res_count = elm_genlist_items_count(ad->bookmarks_notes_genlist);
	if (res_count > 0)
		elm_layout_signal_emit(layout, "elm,holy_bible,bg,hide", "elm");
	evas_object_show(layout);
	return layout;
}

static Eina_Bool
_genlist_free_idler(void *data)
{
	appdata_s *ad = (appdata_s*)data;
	if (ad->bookmarks_notes_genlist)
		elm_genlist_clear(ad->bookmarks_notes_genlist);
	ad->bookmarks_notes_genlist = NULL;
	if (ad->bookmarks_itc)
		elm_genlist_item_class_free(ad->bookmarks_itc);
	ad->bookmarks_itc = NULL;
	return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool
naviframe_pop_cb(void *data, Elm_Object_Item *it)
{
	ecore_idler_add(_genlist_free_idler, data);
	return EINA_TRUE;
}

void
_change_read_mode(appdata_s *ad, Eina_Bool read_mode)
{
	Evas_Object *glayout;
	Elm_Object_Item *item;
	Eina_List *verse_list_temp;
	Eina_List *verse_list = elm_genlist_realized_items_get(ad->genlist);
	EINA_LIST_FOREACH(verse_list, verse_list_temp, item)
	{
		glayout = elm_object_item_part_content_get(item, "elm.swallow.content");
		if (read_mode)
			elm_layout_signal_emit(glayout, "elm,holy_bible,night_mode,on", "elm");
		else
			elm_layout_signal_emit(glayout, "elm,holy_bible,night_mode,off", "elm");
	}
	eina_list_free(verse_list);
	if (read_mode)
		elm_layout_signal_emit(ad->layout, "elm,holy_bible,night_mode,on", "elm");
	else
		elm_layout_signal_emit(ad->layout, "elm,holy_bible,night_mode,off", "elm");
	return;
}

static void
_font_size_change_done(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = (appdata_s*)data;
	Elm_Object_Item *item, *view_item;
	Eina_List *list_items;
	int value = (int)elm_slider_value_get(obj);
	preference_set_int("fontsize", value);

	list_items = elm_genlist_realized_items_get(ad->genlist);
	view_item = (Elm_Object_Item*)eina_list_data_get(list_items);
	item = elm_genlist_first_item_get(ad->genlist);
	while(item)
	{
		elm_genlist_item_update(item);
		item = elm_genlist_item_next_get(item);
	}
	elm_genlist_item_show(view_item, ELM_GENLIST_ITEM_SCROLLTO_IN);
	eina_list_free(list_items);
}

static void
_font_size_changed(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = (appdata_s*)data;
	int value = (int)elm_slider_value_get(obj);

	edje_text_class_set("GTA1", "Tizen:style=Regular", value);
	edje_text_class_set("GTA1B", "Tizen:style=Bold", value);
	edje_text_class_set("GTA1L", "Tizen:style=Light", value);

	elm_genlist_realized_items_update(ad->genlist);
}

static void
_item_selection(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *item = (Elm_Object_Item*)data;
	elm_genlist_item_selected_set(item, EINA_TRUE);
	return;
}

static void
_set_default_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup = (Evas_Object*)data;
	Evas_Object *slider = (Evas_Object*)evas_object_data_get(popup, "slider");
	elm_slider_value_set(slider, 23);
	evas_object_smart_callback_call(slider, "changed", NULL);
	evas_object_smart_callback_call(slider, "slider,drag,stop", NULL);
}

static bool
_check_package(package_info_h pkg_info, void *data)
{
	char *package;
	char *app_id;
	appdata_s *ad = (appdata_s*)data;
	app_struct *app_det;
	app_get_id(&app_id);
	package_info_get_package(pkg_info, &package);
	if ((strncmp(package, "org.tizen.gtaholybible", 22) == 0) ||
			(strcmp(package, "org.tizen.gta_holy_bible") == 0))
	{
		package_manager_compare_result_type_e comp_result;
		package_manager_compare_app_cert_info(app_id, package, &comp_result);
		if ((strcmp(package, app_id) != 0) &&
				(comp_result == PACKAGE_MANAGER_COMPARE_MATCH))
		{
			app_det = (app_struct*)malloc(sizeof(app_struct));
			app_det->app_id = (char*)malloc(sizeof(char) * (strlen(package) + 1));
			strcpy(app_det->app_id, package);
			app_det->app_name = (char*)malloc(sizeof(char) * 32);
			if (strcmp(package, "org.tizen.gta_holy_bible") == 0)
				strcpy(app_det->app_name, "English");
			else
			{
				if (strncmp(package + 22, "mal", 3) == 0)
					strcpy(app_det->app_name, "Malayalam");
				else
				{
					strcpy(app_det->app_name, package + 22);
					app_det->app_name[0] = toupper(app_det->app_name[0]);
				}
			}
			app_det->app_next = NULL;
			if (!ad->app_list_head)
				ad->app_list_head = app_det;
			if (ad->app_list_tail)
			{
				ad->app_list_tail->app_next = app_det;
			}
			ad->app_list_tail = app_det;
		}
	}
	free(package);
	free(app_id);
	return true;
}

static void
_populate_parallel_app_list(appdata_s *ad)
{
	package_manager_h pkg_manager;
	package_manager_create(&pkg_manager);
	package_manager_foreach_package_info(_check_package, ad);
	package_manager_destroy(pkg_manager);
}

static void
ctxpopup_item_select_cb(void *data, Evas_Object *obj, void *event_info)
{
	const char *title_label = elm_object_item_text_get((Elm_Object_Item *) event_info);
	char text_content[2048];
	Elm_Object_Item *nf_it, *sel_item = NULL;
	appdata_s *ad = (appdata_s*)data;

	evas_object_freeze_events_set(obj, EINA_TRUE);
	if (!strcmp(title_label, SEARCH))
	{
		_search_word(ad, NULL, NULL);
		elm_ctxpopup_dismiss(obj);
		return;
	}

	if (!strcmp(title_label, VERSE_DISPLAY_WIDGET))
	{
		_verse_display_widget_list(ad);
		elm_ctxpopup_dismiss(obj);
		return;
	}

	if (!strcmp(title_label, DAY_MODE))
	{
		_change_read_mode(ad, EINA_FALSE);
		preference_set_int("readmode", 1);
		elm_ctxpopup_dismiss(obj);
		return;
	}

	if (!strcmp(title_label, NIGHT_MODE))
	{
		_change_read_mode(ad, EINA_TRUE);
		preference_set_int("readmode", 0);
		elm_ctxpopup_dismiss(obj);
		return;
	}

	if (!strcmp(title_label, BOOKMARKS))
	{
		nf_it = elm_naviframe_item_push(ad->naviframe, BOOKMARKS, NULL, NULL, _get_bookmarks(ad), NULL);
		elm_naviframe_item_pop_cb_set(nf_it, naviframe_pop_cb, ad);
		elm_ctxpopup_dismiss(obj);
		return;
	}

	if (!strcmp(title_label, NOTES))
	{
		nf_it = elm_naviframe_item_push(ad->naviframe, NOTES, NULL, NULL, _get_notes(ad), NULL);
		elm_naviframe_item_pop_cb_set(nf_it, naviframe_pop_cb, ad);
		elm_ctxpopup_dismiss(obj);
		return;
	}

	if (!strcmp(title_label, SHARE))
	{
		elm_ctxpopup_dismiss(obj);
		_change_read_mode(ad, EINA_FALSE);
		_share_verse_cb(ad);
		return;
	}

	if (!strcmp(title_label, COPY))
	{
		elm_ctxpopup_dismiss(obj);
		_change_read_mode(ad, EINA_FALSE);
		_copy_verse_cb(ad);
		return;
	}

	if (!strcmp(title_label, SELECT_CHAPTER))
	{
		_change_book(data, ad->layout, NULL, NULL);
		elm_ctxpopup_dismiss(obj);
		return;
	}

	if (!strcmp(title_label, VERSE_VIEW))
	{
		Eina_List *list = elm_genlist_realized_items_get(ad->genlist);
		_bible_verse_show(ad, ad->genlist, eina_list_data_get(list));
		eina_list_free(list);
		elm_ctxpopup_dismiss(obj);
		return;
	}

	Evas_Object *popup = elm_popup_add(elm_object_top_widget_get(obj));
	elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, 1.0);
	elm_object_part_text_set(popup, "title,text", title_label);

	if (!strcmp(title_label, ABOUT))
	{
		Evas_Object *layout = elm_layout_add(popup);
		elm_layout_file_set(layout, ad->edj_path, "standard_layout");
		Evas_Object *content_box = elm_box_add(popup);
		evas_object_size_hint_weight_set(content_box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(content_box, EVAS_HINT_FILL, EVAS_HINT_FILL);
		Evas_Object *label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		sprintf(text_content, "<color=#000000FF><align=center><b>%s</b></align></color>", TITLE);
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		sprintf(text_content, "<color=#000000FF><align=center><font_size=20>%s</font_size></align></color>", COPYRIGHT);
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		sprintf(text_content, "<color=#000000FF><align=center><em><font_size=20>%s</font_size></em></align></color>", VERSION);
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		sprintf(text_content, " ");
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		sprintf(text_content, "<color=#000000FF><align=center><font_size=20>"
				"This program is free software: you can redistribute it and/or modify "
				"it under the terms of the GNU General Public License as published by "
				"the Free Software Foundation, either version 3 of the License, or "
				"(at your option) any later version.</font_size></align></color>");

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		sprintf(text_content, "<color=#000000FF><align=center><font_size=20>"
				"This program is distributed in the hope that it will be useful, "
				"but WITHOUT ANY WARRANTY; without even the implied warranty of "
				"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the "
				"GNU General Public License for more details.</font_size></align></color>");

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
		sprintf(text_content, " ");
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		sprintf(text_content, "<color=#000000FF><align=center><font_size=16>"
				"Report the bugs or suggestions to "
				"Godly T.Alias (<em>godlytalias@yahoo.co.in</em>).</font_size></align></color>");

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);
		elm_layout_content_set(layout, "elm.swallow.content", content_box);
		evas_object_show(content_box);

		Evas_Object *button_rate = elm_button_add(popup);
		elm_object_text_set(button_rate, RATE_APP);
		evas_object_smart_callback_add(button_rate, "clicked", _rate_app_cb, popup);
		elm_object_part_content_set(popup, "button1", button_rate);
		evas_object_show(button_rate);
		elm_object_content_set(popup, layout);
	}
	else if (!strcmp(title_label, HELP))
	{
		Evas_Object *layout = elm_layout_add(popup);
		elm_layout_file_set(layout, ad->edj_path, "standard_layout");
		Evas_Object *content_box = elm_box_add(popup);
		evas_object_size_hint_weight_set(content_box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(content_box, EVAS_HINT_FILL, EVAS_HINT_FILL);
		Evas_Object *label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		sprintf(text_content, "<color=#000000FF><align=center><b>%s</b></align></color>", TITLE);
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		sprintf(text_content, "<color=#000000FF><align=center><font_size=20>%s</font_size></align></color>", COPYRIGHT);
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		sprintf(text_content, "<color=#000000FF><align=center><em><font_size=20>%s</font_size></em></align></color>", VERSION);
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		sprintf(text_content, " ");
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		sprintf(text_content, "<color=#000000FF><align=left><font_size=25>"
				"<b>Changing chapters</b></font_size></align></color>");

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);


		sprintf(text_content, "<color=#000000FF><align=left><font_size=20>"
				"Users can change the chapters by swiping "
				"left / right in the screen. Users can also click on the arrows in the bottom corners to go to previous or next chapters. "
				"Also if user clicks on the header part of home screen, a new window will be opened "
				"listing out all the Books and the chapters in the selected book. "
				"User can select the Book and Chapter which they want.</font_size></align></color>");

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
		sprintf(text_content, " ");
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		sprintf(text_content, "<color=#000000FF><align=left><font_size=25>"
				"<b>Searching keywords</b></font_size></align></color>");

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);


		sprintf(text_content, "<color=#000000FF><align=left><font_size=20>"
				"User can search a specific keyword and get the verses containing those keywords. "
				"The Search screen can be opened through 'Search' option in application menu. "
				"Keywords can be entered in the text field and press 'Go' to get the list of verses containing the entered keywords. "
				"User can search with more than one keyword seperated by space and can get results which contain all the enetered keywords. "
				"If clicked on a search result item a popup will come with the full verse. "
				"Also if user long press on a search item, user can get option to go to the full chapter of the verse. </font_size></align></color>");

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
		sprintf(text_content, " ");
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);


		sprintf(text_content, "<color=#000000FF><align=left><font_size=25>"
				"<b>Search Preferences</b></font_size></align></color>");

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);


		sprintf(text_content, "<color=#000000FF><align=left><font_size=20>"
				"User can set specific conditions on searching for a keyword. "
				"Search preferences include options to restrict search to new testament, old testament or to some custom books as set by user. "
				"There are certain other options to control the searching,<br/> "
				"<b>Whole Word:</b><br/>User will get only the words exactly matching the keyword entered if this option is enabled. "
				"If it is not enabled search results will include verses with words partially matching the keyword entered also. For eg. on searching Love, if "
				"this option is enabled search results will include verses which contain 'Loves' 'Loved' etc.. else only verses which contain a word "
				"exactly matching 'Love' only will be included in the result.<br/> "
				"<b>Strict Search:</b><br/>This option is useful only when searching with more than one keyword. "
				"If this option is enabled, only verses which contain all the entered keyword "
				"will be included, otherwise even if one keyword entered is present in the verse, it will be included in the result. </font_size></align></color>");

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
		sprintf(text_content, " ");
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		sprintf(text_content, "<color=#000000FF><align=left><font_size=25>"
				"<b>Add notes</b></font_size></align></color>");

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);


		sprintf(text_content, "<color=#000000FF><align=left><font_size=20>"
				"Users can add notes for each verse by selecting the add notes option on "
				"double click / long press the verse. There is a limit of 8kB for the note of each verse. "
				"There are options to view or edit the notes later by double clicking the verse and on opting the view note option. "
				"Users can see all the notes added by them in the Notes option available in the application menu. </font_size></align></color>");

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
		sprintf(text_content, " ");
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		sprintf(text_content, "<color=#000000FF><align=left><font_size=25>"
				"<b>Bookmark verse</b></font_size></align></color>");

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		sprintf(text_content, "<color=#000000FF><align=left><font_size=20>"
				"User can long press / double click on a verse and will get options to Bookmark a verse. "
				"Bookmarked verses will be displayed in red font and with a red strip in left side of verse. "
				"If user want to remove bookmark of a verse, user can go to the bookmarks list in menu option and "
				"can remove bookmark by long pressing on the verse to be removed from bookmark list or by selecting "
				"Remove Bookmark option from the menu on double clicking or long pressing the respective verse on the reading screen.</font_size></align></color>");

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
		sprintf(text_content, " ");
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		sprintf(text_content, "<color=#000000FF><align=left><font_size=25>"
				"<b>Share / Copy verses</b></font_size></align></color>");

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		sprintf(text_content, "<color=#000000FF><align=left><font_size=20>"
				"User can share or copy the desired verses by long pressing / double click on verses. "
				"Verses can also be shared / copied using the Share / Copy options "
				"available in the application menu and then selecting the desired verses. "
				"Reference of verse also will get appended to verse automatically. Verses will be "
				"listed based on the order of selection of verses. "
				"Users can copy more than one verses by selecting the verses needed to copy / share and "
				"then clicking the done button in the header part. Users can get the copied verses from clipboard.</font_size></align></color>");

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
		sprintf(text_content, " ");
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);
		sprintf(text_content, "<color=#000000FF><align=left><font_size=25>"
				"<b>Parallel Reading</b></font_size></align></color>");

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		sprintf(text_content, "<color=#000000FF><align=left><font_size=20>"
				"If user have other GTA Bible applications above v0.5 installed in the device, "
				"users can read both the Bibles together by using this feature. If this feature is enabled "
				"half of the screen will be used for the current application and the other half for the opted "
				"Bible. This feature enables users to read Bible in two languages or versions in parallel and compare them "
				"easily. </font_size></align></color>");

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
		sprintf(text_content, " ");
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);
		sprintf(text_content, "<color=#000000FF><align=left><font_size=25>"
				"<b>Verse Mode</b></font_size></align></color>");

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		sprintf(text_content, "<color=#000000FF><align=left><font_size=20>"
				"This option allows users to read chapters verse vise. "
				"This screen will show one verse at a time in an enlarged manner. "
				"Users can change between verse either by swiping on screen to left / right or by "
				"clicking the arrows in the bottom of the screen. "
				"Users can go to verse mode screen either through application menu "
				"or by double tapping a verse or else by selecting the verse mode option "
				"in the menu shown on long pressing the verse.</font_size></align></color>");

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
		sprintf(text_content, " ");
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);
		sprintf(text_content, "<color=#000000FF><align=left><font_size=25>"
				"<b>Verse Wallpaper Widget</b></font_size></align></color>");

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		sprintf(text_content, "<color=#000000FF><align=left><font_size=20>"
				"This feature enables users to put verses on wallpaper. "
				"Users can add verses to the list by long pressing on the "
				"verse and then clicking on the option 'Add to verse widget'. "
				"Then users have to add the Verse widget in the homescreen "
				"which will display the added verses. Users can change the order "
				"long pressing and dragging the verses to the needed order. "
				"If there are more than one verse in the list, widget will update "
				"verse every 30 minutes and will go to the next verse in the list. "
				"Users can set font size, color and background color from widget settings "
				"window which will be opened on clicking the widget.</font_size></align></color>");

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
		sprintf(text_content, " ");
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);
		sprintf(text_content, "<color=#000000FF><align=left><font_size=25>"
				"<b>Font Size</b></font_size></align></color>");

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		sprintf(text_content, "<color=#000000FF><align=left><font_size=20>"
				"User can set a desired font size between 18 and 36 by dragging the font size slider. "
				"Users can preview the font size changes immediately on changing the slider. There is also "
				"a default button available to reset the font size to default value. Fonts of the application "
				"can be changed by changing the system font. </font_size></align></color>");

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
		sprintf(text_content, " ");
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		sprintf(text_content, "<color=#000000FF><align=left><font_size=25>"
				"<b>Day / Night Reading mode</b></font_size></align></color>");

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		sprintf(text_content, "<color=#000000FF><align=left><font_size=20>"
				"This is an option that enables user to get a better reading experience. If user wants "
				"to read in a darker environment selecting Night Reading Mode option will help user to "
				"reduce the strain in eyes and Day reading mode can be used for reading in bright "
				"environments. </font_size></align></color>");

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
		sprintf(text_content, " ");
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		sprintf(text_content, "<color=#000000FF><align=center><font_size=16>"
				"Report the bugs or suggestions to "
				"Godly T.Alias (<em>godlytalias@yahoo.co.in</em>).</font_size></align></color>");

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);
		elm_layout_content_set(layout, "elm.swallow.content", content_box);
		evas_object_show(content_box);

		Evas_Object *button_bug = elm_button_add(popup);
		elm_object_text_set(button_bug, BUGS_SUGGESTIONS);
		evas_object_smart_callback_add(button_bug, "clicked", _report_cb, popup);
		elm_object_part_content_set(popup, "button1", button_bug);
		evas_object_show(button_bug);
		elm_object_content_set(popup, layout);
	}
	else if (!strcmp(title_label, FONT_SIZE))
	{
		Evas_Object *layout = elm_layout_add(popup);
		elm_layout_file_set(layout, ad->edj_path, "standard_layout");
		int fontsize = 25;
		Evas_Object *slider = elm_slider_add(layout);
		elm_slider_min_max_set(slider, 18, 36);
		elm_slider_step_set(slider, 1.0);
		elm_slider_indicator_show_set(slider, EINA_TRUE);
		elm_slider_indicator_format_set(slider, "%1.0f");
		preference_get_int("fontsize", &fontsize);
		elm_slider_value_set(slider, (double)fontsize);
		evas_object_smart_callback_add(slider, "changed", _font_size_changed, ad);
		evas_object_smart_callback_add(slider, "slider,drag,stop", _font_size_change_done, ad);
		elm_layout_content_set(layout, "elm.swallow.content", slider);

		Evas_Object *button_def = elm_button_add(popup);
		elm_object_text_set(button_def, SET_DEFAULT);
		evas_object_smart_callback_add(button_def, "clicked", _set_default_cb, popup);
		evas_object_data_set(popup, "slider", slider);
		elm_object_part_content_set(popup, "button1", button_def);
		evas_object_show(button_def);
		elm_object_content_set(popup, layout);
	}
	else if (!strcmp(title_label, PARALLEL_READING))
	{
		bool parallel = false;
		app_struct *app_det;
		char *cur_app_id = NULL;
		Elm_Object_Item *item;
		char *cur_para_app_id = NULL;
		preference_get_boolean("parallel", &parallel);
		preference_get_string("parallel_app_id", &cur_para_app_id);
		package_info_h pkg_info;

		Evas_Object *loading_popup = _loading_progress_show(ad->naviframe);
		if (!ad->app_list_head)
			_populate_parallel_app_list(ad);
		Evas_Object *pl_genlist = elm_genlist_add(popup);
		elm_genlist_highlight_mode_set(pl_genlist, EINA_FALSE);
		elm_genlist_select_mode_set(pl_genlist, ELM_OBJECT_SELECT_MODE_ALWAYS);
		evas_object_size_hint_weight_set(pl_genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(pl_genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_smart_callback_add(pl_genlist, "selected", _parallel_gl_selected_cb, NULL);
		evas_object_smart_callback_add(pl_genlist, "unselected", _parallel_gl_unselected_cb, NULL);
		evas_object_data_set(pl_genlist, "appdata", ad);
		Elm_Genlist_Item_Class *pl_itc = elm_genlist_item_class_new();
		pl_itc->item_style = "default";
		pl_itc->func.text_get = _parallel_text_get_cb;
		pl_itc->func.content_get = _parallel_content_get_cb;
		pl_itc->func.del = NULL;
		app_get_id(&cur_app_id);
		if (ad->app_list_head)
		{
			app_det = ad->app_list_head;
			while (app_det)
			{
				if (package_manager_get_package_info(app_det->app_id, &pkg_info) != PACKAGE_MANAGER_ERROR_NONE)
				{
					app_det = app_det->app_next;
					continue;
				}
				char *version = NULL;
				char version_copy[8];
				if (package_info_get_version(pkg_info, &version) == PACKAGE_MANAGER_ERROR_NONE)
				{
					strncpy(version_copy, version, 3);
					if (atof(version_copy) < PARALLEL_READING_SUPPORT_VERSION)
					{
						package_info_destroy(pkg_info);
						app_det = app_det->app_next;
						continue;
					}
				}
				else
				{
					package_info_destroy(pkg_info);
					app_det = app_det->app_next;
					continue;
				}
				package_info_destroy(pkg_info);
				free(version);
				item = elm_genlist_item_append(pl_genlist, pl_itc, app_det, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
				elm_object_item_data_set(item, app_det);
				if (cur_para_app_id && (strcmp(cur_para_app_id, app_det->app_id) == 0))
					sel_item = item;
				app_det = app_det->app_next;
			}
		}
		elm_genlist_item_class_free(pl_itc);
		free(cur_app_id);

		Evas_Object *button_def = elm_button_add(popup);
		if (cur_para_app_id) free(cur_para_app_id);
		elm_object_part_content_set(popup, "button1", button_def);
		evas_object_show(button_def);
		if (elm_genlist_items_count(pl_genlist) > 0)
		{
			if (parallel)
				elm_object_text_set(button_def, OFF);
			else
			{
				elm_object_disabled_set(pl_genlist, EINA_TRUE);
				elm_object_text_set(button_def, ON);
			}
			elm_object_content_set(popup, pl_genlist);
			evas_object_smart_callback_add(button_def, "clicked", _toggle_parallel_reading, pl_genlist);
		}
		else
		{
			elm_object_text_set(button_def, GO_TO_STORE);
			elm_object_text_set(popup, GTA_APP_INSTALL_WARNING);
			evas_object_smart_callback_add(button_def, "clicked", _install_apps_cb, pl_genlist);
		}
		_loading_progress_hide(loading_popup);
	}
	Evas_Object *button = elm_button_add(popup);
	elm_object_text_set(button, CLOSE);
	elm_object_part_content_set(popup, "button2", button);
	evas_object_show(button);
	evas_object_show(popup);
	evas_object_smart_callback_add(button, "clicked", _popup_del, popup);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, eext_popup_back_cb, NULL);
	if (sel_item)
		evas_object_smart_callback_add(popup, "show,finished", _item_selection, sel_item);
	elm_ctxpopup_dismiss(obj);
}

void
show_ctxpopup_more_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = (appdata_s*)data;
	int readmode = 1;

	if (ad->share_copy_mode)
	{
		Evas_Object *win, *ctxpopup;
		ctxpopup = elm_ctxpopup_add(ad->naviframe);
		elm_ctxpopup_auto_hide_disabled_set(ctxpopup, EINA_TRUE);
		elm_object_style_set(ctxpopup, "more/default");
		evas_object_smart_callback_add(ctxpopup, "dismissed", _popup_del, ctxpopup);
		eext_object_event_callback_add(ctxpopup, EEXT_CALLBACK_BACK, eext_ctxpopup_back_cb, NULL);
		eext_object_event_callback_add(ctxpopup, EEXT_CALLBACK_MORE, eext_ctxpopup_back_cb, NULL);

		win = elm_object_top_widget_get(ad->naviframe);
		evas_object_smart_callback_add(win, "rotation,changed", move_more_ctxpopup, ctxpopup);
		move_more_ctxpopup(ctxpopup, ctxpopup, NULL);

		elm_ctxpopup_item_append(ctxpopup, HELP, NULL, ctxpopup_item_select_cb, ad);
		elm_ctxpopup_item_append(ctxpopup, ABOUT, NULL, ctxpopup_item_select_cb, ad);

		elm_ctxpopup_direction_priority_set(ctxpopup, ELM_CTXPOPUP_DIRECTION_UP, ELM_CTXPOPUP_DIRECTION_UNKNOWN, ELM_CTXPOPUP_DIRECTION_UNKNOWN, ELM_CTXPOPUP_DIRECTION_UNKNOWN);
		evas_object_show(ctxpopup);
		return;
	}
	if (ad->menu_ctxpopup)
	{
		move_more_ctxpopup(ad->menu_ctxpopup, ad->menu_ctxpopup, NULL);
		evas_object_freeze_events_set(ad->menu_ctxpopup, EINA_FALSE);
		preference_get_int("readmode", &readmode);
		if (readmode == 0)
			elm_object_item_text_set(ad->readmode_item, DAY_MODE);
		else
			elm_object_item_text_set(ad->readmode_item, NIGHT_MODE);
		evas_object_show(ad->menu_ctxpopup);
	}
	else
	{
		create_ctxpopup_more_menu(ad);
		preference_get_int("readmode", &readmode);
		if (readmode == 0)
			elm_object_item_text_set(ad->readmode_item, DAY_MODE);
		else
			elm_object_item_text_set(ad->readmode_item, NIGHT_MODE);
		move_more_ctxpopup(ad->menu_ctxpopup, ad->menu_ctxpopup, NULL);
		evas_object_show(ad->menu_ctxpopup);
	}
}

void
hide_ctxpopup_more_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = (appdata_s*)data;
	if (ad->menu_ctxpopup)
		elm_ctxpopup_dismiss(ad->menu_ctxpopup);
}

void
create_ctxpopup_more_menu(void *data)
{
	Evas_Object *win;
	appdata_s *ad = (appdata_s*)data;

	if (ad->menu_ctxpopup) return;

	Evas_Object *ctxpopup = elm_ctxpopup_add(ad->naviframe);
	ad->menu_ctxpopup = ctxpopup;
	elm_ctxpopup_auto_hide_disabled_set(ctxpopup, EINA_TRUE);
	elm_object_style_set(ctxpopup, "more/default");
	eext_object_event_callback_add(ctxpopup, EEXT_CALLBACK_BACK, eext_ctxpopup_back_cb, NULL);
	eext_object_event_callback_add(ctxpopup, EEXT_CALLBACK_MORE, eext_ctxpopup_back_cb, NULL);

	win = elm_object_top_widget_get(ad->naviframe);
	evas_object_smart_callback_add(win, "rotation,changed", move_more_ctxpopup, ctxpopup);
	evas_object_smart_callback_add(ctxpopup, "dismissed", _ctxpopup_dismiss_cb, ad);

	elm_ctxpopup_item_append(ctxpopup, SEARCH, NULL, ctxpopup_item_select_cb, ad);
	elm_ctxpopup_item_append(ctxpopup, SHARE, NULL, ctxpopup_item_select_cb, ad);
	elm_ctxpopup_item_append(ctxpopup, COPY, NULL, ctxpopup_item_select_cb, ad);
	elm_ctxpopup_item_append(ctxpopup, NOTES, NULL, ctxpopup_item_select_cb, ad);
	elm_ctxpopup_item_append(ctxpopup, BOOKMARKS, NULL, ctxpopup_item_select_cb, ad);
	elm_ctxpopup_item_append(ctxpopup, SELECT_CHAPTER, NULL, ctxpopup_item_select_cb, ad);
	elm_ctxpopup_item_append(ctxpopup, PARALLEL_READING, NULL, ctxpopup_item_select_cb, ad);
	ad->readmode_item = elm_ctxpopup_item_append(ctxpopup, DAY_MODE, NULL, ctxpopup_item_select_cb, ad);
	elm_ctxpopup_item_append(ctxpopup, VERSE_VIEW, NULL, ctxpopup_item_select_cb, ad);
	elm_ctxpopup_item_append(ctxpopup, VERSE_DISPLAY_WIDGET, NULL, ctxpopup_item_select_cb, ad);
	elm_ctxpopup_item_append(ctxpopup, FONT_SIZE, NULL, ctxpopup_item_select_cb, ad);
	elm_ctxpopup_item_append(ctxpopup, HELP, NULL, ctxpopup_item_select_cb, ad);
	elm_ctxpopup_item_append(ctxpopup, ABOUT, NULL, ctxpopup_item_select_cb, ad);

	elm_ctxpopup_direction_priority_set(ctxpopup, ELM_CTXPOPUP_DIRECTION_UP, ELM_CTXPOPUP_DIRECTION_UNKNOWN, ELM_CTXPOPUP_DIRECTION_UNKNOWN, ELM_CTXPOPUP_DIRECTION_UNKNOWN);
}
