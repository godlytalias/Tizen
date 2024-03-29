#include "bible.h"

static char*
_prepare_verse_list(appdata_s *ad)
{
	Elm_Object_Item *item;
	Eina_List *sel_list_iter;
	Eina_List *sel_list = elm_genlist_selected_items_get(ad->genlist);
	if (!sel_list)
	{
		Evas_Object *toast = elm_popup_add(ad->naviframe);
		elm_object_style_set(toast, "toast");
		elm_popup_allow_events_set(toast, EINA_TRUE);
		elm_object_text_set(toast, NO_VERSE_SELECTED);
		evas_object_show(toast);
		elm_popup_timeout_set(toast, 2.0);
		evas_object_smart_callback_add(toast, "timeout", _popup_del, toast);
		return NULL;
	}
	int sel_count = eina_list_count(sel_list);
	if (sel_count == 1)
	{
		char *buf = NULL;
		buf = malloc(sizeof(char)*2048);
		if (!buf)
		{
			_app_no_memory(ad);
			return NULL;
		}
		item = eina_list_data_get(sel_list);
		bible_verse_item *verse_item = (bible_verse_item*)elm_object_item_data_get(item);
		if (verse_item->verse_s)
			sprintf(buf,"%s\n%s\n ~ %s %d : %d", verse_item->verse, verse_item->verse_s, Books[verse_item->bookcount], verse_item->chaptercount, verse_item->versecount + 1);
		else
			sprintf(buf,"%s ~ %s %d : %d", verse_item->verse, Books[verse_item->bookcount], verse_item->chaptercount, verse_item->versecount + 1);
		return buf;
	}
	else
	{
		char *buf = NULL;
		buf = malloc(sizeof(char) * 1024 * sel_count);
		if (!buf)
		{
			_app_no_memory(ad);
			return NULL;
		}
		bible_verse_item *verse_item;
		Eina_List *temp_list;
		int last_verse;
		sprintf(buf, "%s %d : ", Books[ad->cur_book], ad->cur_chapter);
		Eina_Bool hyphen = EINA_FALSE;
		Evas_Object *check = ad->select_all_check;

		if ((sel_count == elm_genlist_items_count(ad->genlist)) && elm_check_state_get(check))
			sprintf(buf, "%s1 - %d", buf, sel_count);
		else
			EINA_LIST_FOREACH(sel_list, sel_list_iter, item)
			{
			verse_item = (bible_verse_item*)elm_object_item_data_get(item);
			last_verse = verse_item->versecount + 1;
			temp_list = eina_list_next(sel_list_iter);
			if (temp_list)
			{
				item = eina_list_data_get(temp_list);
				verse_item = (bible_verse_item*)elm_object_item_data_get(item);
				if (!hyphen && ((verse_item->versecount == last_verse)
						|| (verse_item->versecount + 2 == last_verse)))
				{
					sprintf(buf, "%s%d", buf, last_verse);
					strcat(buf, " - ");
					hyphen = EINA_TRUE;
				}
				else if (hyphen && ((verse_item->versecount == last_verse)
						|| (verse_item->versecount + 2 == last_verse)))
					continue;
				else if (hyphen && ((verse_item->versecount != last_verse)
						|| (verse_item->versecount + 2 != last_verse)))
				{
					sprintf(buf, "%s%d", buf, last_verse);
					strcat(buf, ", ");
					hyphen = EINA_FALSE;
				}
				else
				{
					sprintf(buf, "%s%d", buf, last_verse);
					strcat(buf, ", ");
				}
			}
			else
				sprintf(buf, "%s%d", buf, last_verse);
			}
		strcat(buf, "\n");
		if ((sel_count == elm_genlist_items_count(ad->genlist))  && elm_check_state_get(check))
		{
			item = elm_genlist_first_item_get(ad->genlist);
			while (item)
			{
				verse_item = (bible_verse_item*)elm_object_item_data_get(item);
				sprintf(buf,"%s\n%d. %s", buf, verse_item->versecount + 1, verse_item->verse);
				item = elm_genlist_item_next_get(item);
			}
		}
		else
			EINA_LIST_FOREACH(sel_list, sel_list_iter, item)
			{
			verse_item = (bible_verse_item*)elm_object_item_data_get(item);
			sprintf(buf,"%s\n%d. %s", buf, verse_item->versecount + 1, verse_item->verse);
			}
		return buf;
	}
}

void
_share_verse_done_cb(void *data, Evas_Object *obj, void *event_info)
{
	char *buf;
	appdata_s *ad = (appdata_s*)data;
	buf = _prepare_verse_list(ad);
	if (!buf) return;
	_cancel_cb(ad, NULL, NULL);
	app_control_h handler;
	app_control_create(&handler);
	app_control_set_launch_mode(handler, APP_CONTROL_LAUNCH_MODE_GROUP);
	app_control_set_operation(handler, APP_CONTROL_OPERATION_SHARE_TEXT);
	app_control_add_extra_data(handler, APP_CONTROL_DATA_TEXT, buf);
	app_control_send_launch_request(handler, NULL, NULL);
	app_control_destroy(handler);
	free(buf);
	evas_object_smart_callback_del(obj, "clicked", _share_verse_done_cb);
}

void
_copy_verse_done_cb(void *data, Evas_Object *obj, void *event_info)
{
	char *buf;
	appdata_s *ad = (appdata_s*)data;
	buf = _prepare_verse_list(ad);
	if (!buf) return;
	_cancel_cb(ad, NULL, NULL);
	elm_cnp_selection_set(obj, ELM_SEL_TYPE_CLIPBOARD, ELM_SEL_FORMAT_TEXT, buf, strlen(buf));
	free(buf);
	evas_object_smart_callback_del(obj, "clicked", _copy_verse_done_cb);
}

void
_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	int readmode;
	appdata_s *ad = (appdata_s*)data;
	Eina_List *sel_list_iter;
	Elm_Object_Item *item;

	preference_get_int("readmode", &readmode);
	if (readmode == 0)
		_change_read_mode(ad, EINA_TRUE);
	else
		_change_read_mode(ad, EINA_FALSE);
	Eina_List *sel_list = eina_list_clone(elm_genlist_selected_items_get(ad->genlist));
	EINA_LIST_FOREACH(sel_list, sel_list_iter, item)
	{
		elm_genlist_item_selected_set(item, EINA_FALSE);
		elm_genlist_item_update(item);
	}
	eina_list_free(sel_list);
	Evas_Object *done_btn = elm_layout_content_get(ad->layout, "title_right_button");
	evas_object_smart_callback_del(done_btn, "clicked", _share_verse_done_cb);
	evas_object_smart_callback_del(done_btn, "clicked", _copy_verse_done_cb);
	ad->share_copy_mode = EINA_FALSE;
	elm_layout_signal_emit(ad->layout, "elm,holy_bible,share_copy,off", "elm");
}

void
_share_verse_cb(appdata_s *ad)
{
	Evas_Object *done_btn = elm_layout_content_get(ad->layout, "title_right_button");
	evas_object_smart_callback_add(done_btn, "clicked", _share_verse_done_cb, ad);
	ad->share_copy_mode = EINA_TRUE;
	int readmode = 0;
	preference_get_int("readmode", &readmode);
	Evas_Object *select_layout = elm_layout_content_get(ad->layout, "elm.select.all");
	if (readmode == 0)
		elm_layout_signal_emit(select_layout, "elm,holy_bible,night_mode,on", "elm");
	else
		elm_layout_signal_emit(select_layout, "elm,holy_bible,night_mode,off", "elm");
	elm_layout_signal_emit(ad->layout, "elm,holy_bible,share_copy,on", "elm");
	elm_genlist_realized_items_update(ad->genlist);
}

void
_copy_verse_cb(appdata_s *ad)
{
	Evas_Object *done_btn = elm_layout_content_get(ad->layout, "title_right_button");
	evas_object_smart_callback_add(done_btn, "clicked", _copy_verse_done_cb, ad);
	ad->share_copy_mode = EINA_TRUE;
	int readmode = 0;
	preference_get_int("readmode", &readmode);
	Evas_Object *select_layout = elm_layout_content_get(ad->layout, "elm.select.all");
	if (readmode == 0)
		elm_layout_signal_emit(select_layout, "elm,holy_bible,night_mode,on", "elm");
	else
		elm_layout_signal_emit(select_layout, "elm,holy_bible,night_mode,off", "elm");
	elm_layout_signal_emit(ad->layout, "elm,holy_bible,share_copy,on", "elm");
	elm_genlist_realized_items_update(ad->genlist);
}
