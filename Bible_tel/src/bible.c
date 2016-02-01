#include <tizen.h>
#include "bible.h"

static void
win_delete_request_cb(void *data, Evas_Object *obj, void *event_info)
{
	ui_app_exit();
}

static void
app_get_resource(const char *edj_file_in, char *edj_path_out, int edj_path_max)
{
	char *res_path = app_get_resource_path();
	if (res_path) {
		snprintf(edj_path_out, edj_path_max, "%s%s", res_path, edj_file_in);
		free(res_path);
	}
}

void
_app_no_memory(appdata_s *ad)
{
	Evas_Object *toast = elm_popup_add(ad->naviframe);
	elm_object_style_set(toast, "toast");
	elm_popup_allow_events_set(toast, EINA_TRUE);
	elm_popup_timeout_set(toast, 2.0);
	elm_object_text_set(toast, NOT_ENOUGH_MEMORY);
	evas_object_smart_callback_add(toast, "timeout", _popup_del, toast);
	evas_object_show(toast);
	return;
}

static Eina_Bool
_get_prev_chapter(void *data)
{
	appdata_s *ad = (appdata_s*)data;
	Evas_Object *popup = _loading_progress_show(ad->win);

	if (ad->cur_book == 0 && ad->cur_chapter == 1) {
		_get_chapter_count_query(data, 65);
		_query_chapter(data, 65, 22);
	}
	else {
		if (ad->cur_chapter == 1) {
			_get_chapter_count_query(data, ad->cur_book - 1);
			_query_chapter(data, ad->cur_book - 1, ad->chaptercount);
		}
		else
			_query_chapter(data, ad->cur_book, ad->cur_chapter - 1);
	}
	_loading_progress_hide(popup);
	return ECORE_CALLBACK_CANCEL;
}

static void
_prev_chapter(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	ecore_idle_enterer_add(_get_prev_chapter, data);
}

void _show_verse(void *data, int verse)
{
	appdata_s *ad = (appdata_s*)data;
	Elm_Object_Item *item = elm_genlist_nth_item_get(ad->genlist, verse); //verse count start from 0
	if (item) elm_genlist_item_show(item, ELM_GENLIST_ITEM_SCROLLTO_TOP);
	return;
}

static Eina_Bool
_get_nxt_chapter(void *data)
{
	appdata_s *ad = (appdata_s*)data;
	Evas_Object *popup = _loading_progress_show(ad->win);

	if (ad->cur_book == 65 && ad->cur_chapter == 22) {
		_get_chapter_count_query(data, 0);
		_query_chapter(data, 0, 1);
	}
	else {
		if (ad->cur_chapter == ad->chaptercount)
		{
			_get_chapter_count_query(data, ad->cur_book + 1);
			_query_chapter(data, ad->cur_book + 1, 1);
		}
		else
			_query_chapter(data, ad->cur_book, ad->cur_chapter + 1);
	}
	_loading_progress_hide(popup);
	return ECORE_CALLBACK_CANCEL;
}

static void
_nxt_chapter(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	ecore_idle_enterer_add(_get_nxt_chapter, data);
}

static void
_show_warning_popup(appdata_s *ad)
{
	Evas_Object *popup = elm_popup_add(ad->win);
	elm_object_part_text_set(popup, "title,text", BIBLE_NAME_STRING);
	elm_object_text_set(popup, LANGUAGE_SUPPORT_STRING);
	Evas_Object *ok_btn = elm_button_add(popup);
	elm_object_text_set(ok_btn, "GOT IT");
	elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, 1.0);
	elm_object_part_content_set(popup, "button1", ok_btn);
	evas_object_smart_callback_add(ok_btn, "clicked", _popup_del, popup);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, eext_popup_back_cb, popup);
	evas_object_show(popup);
}

static void
_set_reading_mode(appdata_s *ad)
{
	preference_set_int("readmode", 1);
}

static void
_first_launch_check(appdata_s *ad)
{
	bool exist;
	if (preference_is_existing("first_launch", &exist) == 0)
	{
		if (!exist)
		{
			_show_warning_popup(ad);
			_set_reading_mode(ad);
			preference_set_int("first_launch", 0);
		}
	}
	else{
		_show_warning_popup(ad);
		_set_reading_mode(ad);
	}
}

static void
_splash_over(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	appdata_s *ad = (appdata_s*)data;
	_first_launch_check(ad);
	int readmode = 0;
	preference_get_int("readmode", &readmode);
	if (readmode)
		elm_layout_signal_emit(ad->layout, "elm,holy_bible,night_mode,off", "elm");
	else
		elm_layout_signal_emit(ad->layout, "elm,holy_bible,night_mode,on", "elm");
	if (elm_win_wm_rotation_supported_get(ad->win)) {
		int rots[4] = { 0, 90, 180, 270 };
		elm_win_wm_rotation_available_rotations_set(ad->win, (const int *)(&rots), 4);
	}
	eext_object_event_callback_add(ad->naviframe, EEXT_CALLBACK_MORE, eext_naviframe_more_cb, NULL);
}

static void
_content_mouse_down(void *data,
        Evas *evas EINA_UNUSED,
        Evas_Object *obj,
        void *event_info)
{
   appdata_s *ad = (appdata_s*)data;
   if(ad->share_copy_mode) return;

   Evas_Event_Mouse_Down *ev = (Evas_Event_Mouse_Down*)event_info;
   ad->mouse_x = ev->canvas.x;
   ad->mouse_y = ev->canvas.y;
   ad->mouse_down_time = ev->timestamp;
   elm_layout_signal_emit(ad->layout, "elm,holy_bible,arrow,highlight", "elm");
}

static void
_content_mouse_up(void *data,
        Evas *evas EINA_UNUSED,
        Evas_Object *obj,
        void *event_info)
{
   appdata_s *ad = (appdata_s*)data;
   if(ad->share_copy_mode) return;

   int x_del, y_del;
   Evas_Event_Mouse_Up *ev = (Evas_Event_Mouse_Up*)event_info;
   if ((ev->timestamp - ad->mouse_down_time) > 1000) return;
   x_del = ev->canvas.x - ad->mouse_x;
   y_del = ev->canvas.y - ad->mouse_y;
   if (abs(x_del) < (2 * abs(y_del))) return;
   if (abs(x_del) < 100) return;
   if (x_del < 0)
	   _nxt_chapter(data, obj, NULL, NULL);
   else
	   _prev_chapter(data, obj, NULL, NULL);
}

static void
_reset_select_check(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	Evas_Object *check = (Evas_Object*)data;
	elm_check_state_set(check, EINA_FALSE);
}

static void
_select_all_verses(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	Evas_Object *check = (Evas_Object*)data;
	appdata_s *ad = (appdata_s*)evas_object_data_get(check, "appdata");
	Elm_Object_Item *item;
	if (elm_check_state_get(check))
	{
		elm_check_state_set(check, EINA_FALSE);
		item = elm_genlist_first_item_get(ad->genlist);
		while(item)
		{
			elm_genlist_item_selected_set(item, EINA_FALSE);
			elm_genlist_item_update(item);
			item = elm_genlist_item_next_get(item);
		}
	}
	else
	{
		elm_check_state_set(check, EINA_TRUE);
		item = elm_genlist_first_item_get(ad->genlist);
		while(item)
		{
			elm_genlist_item_selected_set(item, EINA_TRUE);
			elm_genlist_item_update(item);
			item = elm_genlist_item_next_get(item);
		}
	}
}

static Evas_Object*
gl_content_get_cb(void *data, Evas_Object *obj, const char *part)
{
    bible_verse_item *verse_item = (bible_verse_item*)data;
    if(strcmp(part, "elm.swallow.content") == 0)
    {
    	Evas_Object *layout = elm_layout_add(obj);
    	char verse_count[5];
    	int readmode = 1;
    	bool parallel = false;
    	preference_get_boolean("parallel", &parallel);
    	if (parallel && verse_item->appdata->parallel_db_path)
    		elm_layout_file_set(layout, verse_item->appdata->edj_path, "parallel_verse_layout");
    	else
    		elm_layout_file_set(layout, verse_item->appdata->edj_path, "verse_layout");
    	preference_get_int("readmode", &readmode);
    	if (readmode == 0)
    	{
    		elm_layout_signal_emit(layout, "elm,holy_bible,night_mode,on", "elm");
    		if (elm_genlist_item_selected_get(verse_item->it))
    			elm_layout_signal_emit(layout, "elm,holy_bible,selection,show", "elm");
    	}
    	else
    		elm_layout_signal_emit(layout, "elm,holy_bible,night_mode,off", "elm");
     	elm_object_part_text_set(layout,"elm.text.verse",verse_item->verse);
     	if (parallel && verse_item->appdata->parallel_db_path)
     		elm_object_part_text_set(layout,"elm.text.verse_s",verse_item->verse_s);
    	sprintf(verse_count, "%d", verse_item->versecount+1);
    	elm_object_part_text_set(layout, "elm.text.verse_count", verse_count);
    	if (verse_item->bookmark) elm_layout_signal_emit(layout, "elm,holy_bible,bookmark,show", "elm");
    	if (verse_item->note) elm_layout_signal_emit(layout, "elm,holy_bible,note,show", "elm");

    	evas_object_show(layout);
    	return layout;
    }
    else return NULL;
}

static void
_remove_bookmark_query(void *data, Evas_Object *obj, void *event_info)
{
	char query[256];
	bible_verse_item *verse_item = (bible_verse_item*)data;
    sprintf(query, "DELETE FROM bookmark WHERE bookcount = %d AND chaptercount = %d AND versecount = %d", verse_item->bookcount, verse_item->chaptercount, verse_item->versecount);
    _app_database_query(query, NULL, NULL);
    verse_item->bookmark = EINA_FALSE;
    elm_genlist_item_update(verse_item->it);
    Evas_Object *toast = elm_popup_add(verse_item->appdata->naviframe);
    elm_object_style_set(toast, "toast");
    elm_popup_allow_events_set(toast, EINA_TRUE);
    elm_popup_timeout_set(toast, 2.0);
    evas_object_show(toast);
    elm_object_text_set(toast, BOOKMARK_REMOVED);
    evas_object_smart_callback_add(toast, "timeout", eext_popup_back_cb, toast);
    elm_ctxpopup_dismiss(obj);
    elm_object_focus_set(verse_item->appdata->genlist, EINA_TRUE);
    return;
}

static void
_share_verse_item_cb(void *data, Evas_Object *obj, void *event_info)
{
	bible_verse_item *verse_item = (bible_verse_item*)data;
	_popup_del(obj, NULL, NULL);
	_share_verse_cb(verse_item->appdata);
	elm_genlist_item_selected_set(verse_item->it, EINA_TRUE);
}

static void
_copy_verse_item_cb(void *data, Evas_Object *obj, void *event_info)
{
	bible_verse_item *verse_item = (bible_verse_item*)data;
	_popup_del(obj, NULL, NULL);
	_copy_verse_cb(verse_item->appdata);
	elm_genlist_item_selected_set(verse_item->it, EINA_TRUE);
}

static void
_bookmark_verse_cb(void *data, Evas_Object *obj, void *event_info)
{
   bible_verse_item *verse_item = (bible_verse_item*)data;
   Evas_Object *toast = elm_popup_add(verse_item->appdata->naviframe);
   elm_object_style_set(toast, "toast");
   elm_popup_allow_events_set(toast, EINA_TRUE);
   if (!verse_item->bookmark)
   {
	   char query[2048];
	   sprintf(query, "INSERT INTO bookmark VALUES(%d, %d, %d, '%s')", verse_item->bookcount, verse_item->chaptercount, verse_item->versecount, verse_item->verse);
	   _app_database_query(query, NULL, NULL);
	   sprintf(query, "%s %s %d : %d", BOOKMARKED, Books[verse_item->bookcount], verse_item->chaptercount, verse_item->versecount + 1);
	   elm_object_text_set(toast, query);
	   verse_item->bookmark = EINA_TRUE;
	   elm_genlist_item_update(verse_item->it);
   }
   else elm_object_text_set(toast, VERSE_ALREADY_BOOKMARKED);
   elm_popup_timeout_set(toast, 2.0);
   evas_object_show(toast);
   evas_object_smart_callback_add(toast, "timeout", eext_popup_back_cb, toast);
   elm_ctxpopup_dismiss(obj);
   elm_object_focus_set(verse_item->appdata->genlist, EINA_TRUE);
}

static void
_save_note_query(void *data, Evas_Object *obj, void *event_info)
{
	char query[8300];
	bible_verse_item *verse_item = (bible_verse_item*)data;
	verse_item->note = EINA_TRUE;
	const char *note = elm_entry_entry_get(verse_item->appdata->note_entry);
	if (strlen(note) == 0 || strlen(note) > 8192)
	{
		Evas_Object *toast = elm_popup_add(verse_item->appdata->naviframe);
		elm_object_style_set(toast, "toast");
		if (strlen(note) == 0)
			elm_object_text_set(toast, NOTHING_TO_SAVE);
		else
			elm_object_text_set(toast, NOTE_IS_TOO_LARGE);
		elm_popup_allow_events_set(toast, EINA_TRUE);
		elm_popup_timeout_set(toast, 2.0);
		evas_object_smart_callback_add(toast, "timeout", _popup_del, toast);
		evas_object_show(toast);
		return;
	}
	sprintf(query, "delete from notes where bookcount=%d and chaptercount=%d and versecount=%d;", verse_item->bookcount, verse_item->chaptercount, verse_item->versecount);
	_app_database_query(query, NULL, NULL);
	sprintf(query, "insert into notes VALUES(%d, %d, %d, '%s');", verse_item->bookcount, verse_item->chaptercount, verse_item->versecount, note);
	_app_database_query(query, NULL, NULL);
	Evas_Object *toast = elm_popup_add(verse_item->appdata->naviframe);
	elm_object_style_set(toast, "toast");
	elm_object_text_set(toast, NOTE_SAVED);
	elm_popup_allow_events_set(toast, EINA_TRUE);
	elm_popup_timeout_set(toast, 2.0);
	evas_object_smart_callback_add(toast, "timeout", _popup_del, toast);
	evas_object_show(toast);
	elm_genlist_item_update(verse_item->it);
}

static void
_edit_note_cb(void *data, Evas_Object *obj, void *event_info)
{
	bible_verse_item *verse_item = (bible_verse_item*)data;
	elm_entry_editable_set(verse_item->appdata->note_entry, EINA_TRUE);
	elm_object_text_set(obj, SAVE);
	evas_object_smart_callback_del_full(obj, "clicked", _edit_note_cb, data);
	evas_object_smart_callback_add(obj, "clicked", _save_note_query, data);
	elm_object_focus_set(verse_item->appdata->note_entry, EINA_TRUE);
	elm_entry_cursor_end_set(verse_item->appdata->note_entry);
}

static int
_get_note(void *data, int argc, char **argv, char **azColName)
{
	bible_verse_item *verse_item = (bible_verse_item*)data;
	elm_entry_entry_set(verse_item->appdata->note_entry, argv[0]);
	elm_entry_editable_set(verse_item->appdata->note_entry, EINA_FALSE);
	return 0;
}

static void
_add_note_cb(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *note_entry;
   bible_verse_item *verse_item = (bible_verse_item*)data;
   elm_ctxpopup_dismiss(obj);
   Evas_Object *note_popup = elm_popup_add(verse_item->appdata->naviframe);
   elm_object_part_text_set(note_popup, "title,text", NOTES);
   Evas_Object *popup_layout = elm_layout_add(note_popup);
   elm_layout_file_set(popup_layout, verse_item->appdata->edj_path, "standard_layout");
   if (!verse_item->note)
   {
	   verse_item->appdata->note_entry = elm_entry_add(note_popup);
	   note_entry = verse_item->appdata->note_entry;
	   elm_entry_single_line_set(note_entry, EINA_FALSE);
	   elm_object_part_text_set(note_entry, "elm.guide", ENTER_THE_NOTES);
	   evas_object_size_hint_weight_set(note_entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	   elm_layout_content_set(popup_layout, "elm.swallow.content", note_entry);
	   Evas_Object *save_btn = elm_button_add(note_popup);
	   elm_object_text_set(save_btn, SAVE);
	   elm_object_part_content_set(note_popup, "button2", save_btn);
	   evas_object_smart_callback_add(save_btn, "clicked", _save_note_query, verse_item);
   }
   else
   {
	   verse_item->appdata->note_entry = elm_entry_add(note_popup);
	   note_entry = verse_item->appdata->note_entry;
	   elm_entry_single_line_set(note_entry, EINA_FALSE);
	   char query[256];
	   sprintf(query, "select note from notes where bookcount=%d and chaptercount=%d and versecount=%d;", verse_item->bookcount, verse_item->chaptercount, verse_item->versecount);
	   _app_database_query(query, _get_note, verse_item);
	   evas_object_size_hint_weight_set(note_entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	   elm_layout_content_set(popup_layout, "elm.swallow.content", note_entry);
	   Evas_Object *edit_btn = elm_button_add(note_popup);
	   elm_object_text_set(edit_btn, EDIT);
	   elm_object_part_content_set(note_popup, "button2", edit_btn);
	   evas_object_smart_callback_add(edit_btn, "clicked", _edit_note_cb, verse_item);
	   Evas_Object *del_btn = elm_button_add(note_popup);
	   elm_object_text_set(del_btn, DELETE);
	   elm_object_part_content_set(note_popup, "button3", del_btn);
	   evas_object_data_set(note_popup, "verse_item", verse_item);
	   evas_object_smart_callback_add(del_btn, "clicked", note_remove_cb, verse_item);
	   evas_object_smart_callback_add(del_btn, "clicked", _popup_del, note_popup);
   }
   elm_object_content_set(note_popup, popup_layout);
   Evas_Object *close = elm_button_add(note_popup);
   elm_object_text_set(close, CLOSE);
   evas_object_smart_callback_add(close, "clicked", _popup_del, note_popup);
   elm_object_part_content_set(note_popup, "button1", close);
   elm_popup_align_set(note_popup, ELM_NOTIFY_ALIGN_FILL, 1.0);
   eext_object_event_callback_add(note_popup, EEXT_CALLBACK_BACK, eext_popup_back_cb, NULL);
   evas_object_show(note_popup);
   if (!verse_item->note)
	   elm_object_focus_set(verse_item->appdata->note_entry, EINA_TRUE);
}

static void
gl_longpressed_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = (appdata_s*)data;
	if (ad->share_copy_mode) return;
	if (!elm_object_focus_get(ad->genlist)) return;

	Elm_Object_Item *it = (Elm_Object_Item*)event_info;
	bible_verse_item *verse_item = (bible_verse_item*)elm_object_item_data_get(it);
	elm_genlist_item_selected_set(it, EINA_FALSE);
	Evas_Object *verse_popup = elm_ctxpopup_add(ad->naviframe);
	elm_ctxpopup_auto_hide_disabled_set(verse_popup, EINA_TRUE);
	elm_ctxpopup_hover_parent_set(verse_popup, ad->genlist);
    evas_object_data_set(ad->genlist, "verse_popup", verse_popup);
    if (!verse_item->bookmark)
    	elm_ctxpopup_item_append(verse_popup, BOOKMARK_VERSE, NULL, _bookmark_verse_cb, verse_item);
    else
    	elm_ctxpopup_item_append(verse_popup, REMOVE_BOOKMARK, NULL, _remove_bookmark_query, verse_item);
    if (!verse_item->note)
    	elm_ctxpopup_item_append(verse_popup, ADD_NOTES, NULL, _add_note_cb, verse_item);
    else
    	elm_ctxpopup_item_append(verse_popup, VIEW_NOTES, NULL, _add_note_cb, verse_item);
	elm_ctxpopup_item_append(verse_popup, SHARE_VERSE, NULL, _share_verse_item_cb, verse_item);
	elm_ctxpopup_item_append(verse_popup, COPY_VERSE, NULL, _copy_verse_item_cb, verse_item);
	evas_object_smart_callback_add(verse_popup, "dismissed", _popup_del, verse_popup);
	eext_object_event_callback_add(verse_popup, EEXT_CALLBACK_BACK, eext_ctxpopup_back_cb, verse_popup);
	eext_object_event_callback_add(verse_popup, EEXT_CALLBACK_MORE, eext_ctxpopup_back_cb, verse_popup);
	evas_object_move(verse_popup, ad->mouse_x, ad->mouse_y);
	evas_object_show(verse_popup);
}

static void
gl_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = (appdata_s*)data;
	Elm_Object_Item *it = (Elm_Object_Item*)event_info;

	if (!ad->share_copy_mode)
		elm_genlist_item_selected_set(it, EINA_FALSE);
}

static void
gl_unselected_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = (appdata_s*)data;
	if (ad->share_copy_mode)
	{
		Evas_Object *layout = elm_layout_content_get(ad->layout, "elm.select.all");
		Evas_Object *check = elm_layout_content_get(layout, "elm.swallow.check");
		elm_check_state_set(check, EINA_FALSE);
	}
}

static void
gl_highlighted_cb(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item*)event_info;

	int readmode = 0;
	preference_get_int("readmode", &readmode);
	if (readmode == 0)
	{
		Evas_Object *layout = elm_object_item_part_content_get(it, "elm.swallow.content");
		elm_layout_signal_emit(layout, "elm,holy_bible,selection,show", "elm");
	}
}

static void
gl_unhighlighted_cb(void *data, Evas_Object *obj, void *event_info)
{
	int readmode = 0;
	Elm_Object_Item *it = (Elm_Object_Item*)event_info;
	preference_get_int("readmode", &readmode);
	if (readmode == 0)
	{
		Evas_Object *layout = elm_object_item_part_content_get(it, "elm.swallow.content");
		elm_layout_signal_emit(layout, "elm,holy_bible,selection,hide", "elm");
	}
}

void
gl_del_cb(void *data, Evas_Object *obj)
{
   bible_verse_item *verse_item = (bible_verse_item*)data;
   free(verse_item->verse);
   if (verse_item->verse_s) free(verse_item->verse_s);
   free(verse_item);
   verse_item = NULL;
}

static Eina_Bool
_progress_show(void *data)
{
	appdata_s *ad = (appdata_s*)data;
	Evas_Object *obj = (Evas_Object*)evas_object_data_get(ad->layout, "progressbar");
	double val = elm_progressbar_value_get(obj);
	if (val == 1.0)
	{
		elm_layout_signal_emit(ad->layout, "elm,holy_bible,loading,done", "elm");
		return ECORE_CALLBACK_CANCEL;
	}
	else
	{
		elm_progressbar_value_set(obj, val + 0.02);
		return ECORE_CALLBACK_RENEW;
	}
}

static Eina_Bool
_load_chapter(void *data)
{
	appdata_s *ad = (appdata_s*)data;
	_query_chapter(data, ad->cur_book, ad->cur_chapter);
	_get_chapter_count_query(data, ad->cur_book);
	ecore_timer_add(0.03, _progress_show, ad);
	return ECORE_CALLBACK_CANCEL;
}

/*static*/ Evas_Object*
_home_screen(appdata_s *ad)
{
	/* Base Layout */
	ad->layout = elm_layout_add(ad->naviframe);
	elm_layout_file_set(ad->layout, ad->edj_path, GRP_MAIN);
	evas_object_size_hint_weight_set(ad->layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(ad->layout);
	elm_win_resize_object_add(ad->win, ad->layout);
	elm_layout_signal_callback_add(ad->layout, "elm,holy_bible,book,change", "elm", _change_book, (void*)ad);
	elm_layout_signal_callback_add(ad->layout, "elm,holy_bible,next,chapter", "elm", _nxt_chapter, (void*)ad);
	elm_layout_signal_callback_add(ad->layout, "elm,holy_bible,prev,chapter", "elm", _prev_chapter, (void*)ad);
	elm_layout_signal_callback_add(ad->layout, "elm,holy_bible,splash,over", "elm", _splash_over, (void*)ad);

	elm_object_part_text_set(ad->layout, "elm.text.copyright", COPYRIGHT);
	elm_object_part_text_set(ad->layout, "elm.text.version", VERSION);
	elm_object_part_text_set(ad->layout, "elm.text.apptitle", TITLE);
	elm_object_part_text_set(ad->layout, "elm.text.loading", LOADING_DATABASE);
	Evas_Object *progressbar = elm_progressbar_add(ad->layout);
	elm_progressbar_value_set(progressbar, 0.0);
	evas_object_data_set(ad->layout, "progressbar", progressbar);
	elm_progressbar_horizontal_set(progressbar, EINA_TRUE);
	evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_content_set(ad->layout, "elm.swallow.progressbar", progressbar);

	Evas_Object *cancel_btn = elm_button_add(ad->naviframe);
	elm_object_style_set(cancel_btn, "naviframe/title_cancel");
	elm_layout_content_set(ad->layout, "title_left_button", cancel_btn);
	evas_object_smart_callback_add(cancel_btn, "clicked", _cancel_cb, ad);

	Evas_Object *done_btn = elm_button_add(ad->layout);
	elm_object_style_set(done_btn, "naviframe/title_done");
	elm_layout_content_set(ad->layout, "title_right_button", done_btn);

	ad->itc = elm_genlist_item_class_new();
	ad->itc->item_style = "full";
	ad->itc->func.content_get = gl_content_get_cb;
	ad->itc->func.text_get = NULL;
	ad->itc->func.del = gl_del_cb;

	ad->genlist = elm_genlist_add(ad->layout);
	evas_object_size_hint_align_set(ad->genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(ad->genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_content_set(ad->layout, "elm.swallow.content", ad->genlist);
	elm_genlist_mode_set(ad->genlist, ELM_LIST_COMPRESS);
	elm_genlist_homogeneous_set(ad->genlist, EINA_FALSE);
	elm_genlist_multi_select_set(ad->genlist, EINA_TRUE);
	evas_object_smart_callback_add(ad->genlist, "selected", gl_selected_cb, ad);
	evas_object_smart_callback_add(ad->genlist, "unselected", gl_unselected_cb, ad);
	evas_object_smart_callback_add(ad->genlist, "highlighted", gl_highlighted_cb, ad);
	evas_object_smart_callback_add(ad->genlist, "unhighlighted", gl_unhighlighted_cb, ad);
	evas_object_smart_callback_add(ad->genlist, "longpressed", gl_longpressed_cb, ad);
	evas_object_smart_callback_add(ad->genlist, "clicked,double", gl_longpressed_cb, ad);
    evas_object_event_callback_add(ad->genlist, EVAS_CALLBACK_MOUSE_DOWN, _content_mouse_down, ad);
    evas_object_event_callback_add(ad->genlist, EVAS_CALLBACK_MOUSE_UP, _content_mouse_up, ad);

    Evas_Object *layout = elm_layout_add(ad->layout);
    elm_layout_file_set(layout, ad->edj_path, "select_all_layout");
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_layout_text_set(layout, "select_all", SELECT_ALL);
	Evas_Object *check = elm_check_add(layout);
	elm_layout_content_set(layout, "elm.swallow.check", check);
	elm_layout_content_set(ad->layout, "elm.select.all", layout);
	evas_object_data_set(check, "appdata", ad);
	evas_object_data_set(ad->genlist, "select_all_check", check);
	elm_layout_signal_callback_add(ad->layout, "elm,holy_bible,share_copy,on", "elm", _reset_select_check, check);
	elm_layout_signal_callback_add(layout, "elm,holy_bible,select_all", "elm", _select_all_verses, check);

	_load_appdata(ad);
	ecore_idler_add(_load_chapter, ad);

	evas_object_show(ad->genlist);
	return ad->layout;
}

static Eina_Bool
naviframe_pop_cb(void *data, Elm_Object_Item *it)
{
	appdata_s *ad = (appdata_s*)data;
	if (ad->share_copy_mode)
	{
		_cancel_cb(data, NULL, NULL);
		return EINA_FALSE;
	}
	Evas_Object *popup = _loading_progress_show(ad->win);
	if (ad->genlist)
		elm_genlist_clear(ad->genlist);
	ad->genlist = NULL;
	if (ad->itc)
		elm_genlist_item_class_free(ad->itc);
	ad->itc = NULL;
	_loading_progress_hide(popup);
	ui_app_exit();
	return EINA_FALSE;
}

void
_loading_progress_hide(Evas_Object *popup)
{
	evas_object_hide(popup);
	evas_object_del(popup);
}

Evas_Object *
_loading_progress_show(Evas_Object *parent)
{
	Evas_Object *toast_popup = elm_popup_add(parent);
	elm_object_style_set(toast_popup, "toast");
	elm_popup_align_set(toast_popup, 0.5, 0.5);

	Evas_Object *loading = elm_progressbar_add(parent);
	elm_object_style_set(loading, "process_medium");
	elm_object_content_set(toast_popup, loading);
	evas_object_size_hint_align_set(loading, EVAS_HINT_FILL, 0.5);
	evas_object_size_hint_weight_set(loading, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_progressbar_pulse(loading, EINA_TRUE);
	evas_object_show(loading);

	evas_object_show(toast_popup);
	return toast_popup;
}

static Eina_Bool
_loading_dismiss(void *data)
{
	Evas_Object *popup = (Evas_Object*)data;
	evas_object_hide(popup);
	evas_object_del(popup);
	return ECORE_CALLBACK_CANCEL;
}

void
_loading_progress(Evas_Object *parent)
{
	Evas_Object *toast_popup = elm_popup_add(parent);
	elm_object_style_set(toast_popup, "toast");
	elm_popup_align_set(toast_popup, 0.5, 0.5);

	Evas_Object *loading = elm_progressbar_add(parent);
	elm_object_style_set(loading, "process_medium");
	elm_object_content_set(toast_popup, loading);
	evas_object_size_hint_align_set(loading, EVAS_HINT_FILL, 0.5);
	evas_object_size_hint_weight_set(loading, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_progressbar_pulse(loading, EINA_TRUE);
	evas_object_show(loading);

	evas_object_show(toast_popup);
	ecore_idler_add(_loading_dismiss, toast_popup);
}

static void
create_base_gui(appdata_s *ad)
{
	Elm_Object_Item *nf_item;
	Evas_Object *menu_btn;
	ad->win = elm_win_util_standard_add(PACKAGE, PACKAGE);
	elm_win_autodel_set(ad->win, EINA_TRUE);
	elm_win_conformant_set(ad->win, EINA_TRUE);
	app_get_resource(EDJ_FILE, ad->edj_path, (int)PATH_MAX);
	elm_app_base_scale_set(1.8);

	Evas_Object *conform = elm_conformant_add(ad->win);
	evas_object_size_hint_weight_set(conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(ad->win, conform);
	evas_object_show(conform);
	elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);
	elm_win_indicator_opacity_set(ad->win, ELM_WIN_INDICATOR_TRANSPARENT);

	evas_object_smart_callback_add(ad->win, "delete,request", win_delete_request_cb, NULL);
	ad->naviframe = elm_naviframe_add(conform);
	evas_object_size_hint_weight_set(ad->naviframe, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(ad->naviframe, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(ad->naviframe);
	nf_item = elm_naviframe_item_push(ad->naviframe, NULL, NULL, NULL, _home_screen(ad), NULL);
	elm_naviframe_item_title_enabled_set(nf_item, EINA_FALSE, EINA_TRUE);
	elm_naviframe_item_pop_cb_set(nf_item, naviframe_pop_cb, ad);
	elm_object_content_set(conform, ad->naviframe);

	menu_btn = elm_button_add(ad->naviframe);
	elm_object_style_set(menu_btn, "naviframe/more/default");
	evas_object_smart_callback_add(menu_btn, "clicked", show_ctxpopup_more_button_cb, ad);
	elm_object_item_part_content_set(nf_item, "toolbar_more_btn", menu_btn);
	evas_object_show(menu_btn);
	eext_object_event_callback_add(ad->naviframe, EEXT_CALLBACK_BACK, eext_naviframe_back_cb, NULL);
	create_ctxpopup_more_menu(ad);

	evas_object_show(ad->win);

}

static bool
app_create(void *data)
{
	/* Hook to take necessary actions before main event loop starts
	   Initialize UI resources and application's data
	   If this function returns true, the main loop of application starts
	   If this function returns false, the application is terminated */
	appdata_s *ad = data;
	ad->cur_book = 0;
	ad->cur_chapter = 1;
	ad->count = 0;
	ad->menu_ctxpopup = NULL;
	ad->app_list_head = NULL;
	ad->app_list_tail = NULL;

	create_base_gui(ad);

	return true;
}

static void
app_control(app_control_h app_control, void *data)
{
	/* Handle the launch request. */
}

static void
app_pause(void *data)
{
	/* Take necessary actions when application becomes invisible. */
}

static void
app_resume(void *data)
{
	/* Take necessary actions when application becomes visible. */
}

static void
app_terminate(void *data)
{
	/* Release all resources. */
	appdata_s *ad = (appdata_s*)data;
	if (ad->parallel_db_path) free(ad->parallel_db_path);
	_save_appdata(data);
	if (ad->app_list_head)
	{
		app_struct *temp;
		temp = ad->app_list_head;
		while(temp)
		{
			free(temp->app_id);
			free(temp->app_name);
			temp = temp->app_next;
			free(ad->app_list_head);
			ad->app_list_head = temp;
		}
	}
	if (ad->menu_ctxpopup)
	{
		elm_ctxpopup_clear(ad->menu_ctxpopup);
		evas_object_del(ad->menu_ctxpopup);
	}
}

static void
ui_app_lang_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LANGUAGE_CHANGED*/
	char *locale = NULL;
	system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale);
	elm_language_set(locale);
	free(locale);
	return;
}

static void
ui_app_orient_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_DEVICE_ORIENTATION_CHANGED*/
	return;
}

static void
ui_app_region_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_REGION_FORMAT_CHANGED*/
}

static void
ui_app_low_battery(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_BATTERY*/
}

static void
ui_app_low_memory(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_MEMORY*/
	appdata_s *ad = (appdata_s*)user_data;
	if (ad) _app_no_memory(ad);
}

int
main(int argc, char *argv[])
{
	appdata_s ad = {0,};
	int ret = 0;

	ui_app_lifecycle_callback_s event_callback = {0,};
	app_event_handler_h handlers[5] = {NULL, };

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.app_control = app_control;

	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, ui_app_low_battery, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, ui_app_low_memory, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED], APP_EVENT_DEVICE_ORIENTATION_CHANGED, ui_app_orient_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, ui_app_lang_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, ui_app_region_changed, &ad);
	ui_app_remove_event_handler(handlers[APP_EVENT_LOW_MEMORY]);

	ret = ui_app_main(argc, argv, &event_callback, &ad);
	if (ret != APP_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "ui_app_main() is failed. err = %d", ret);
	}

	return ret;
}