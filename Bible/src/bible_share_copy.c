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
    	elm_object_text_set(toast, "No verse selected");
    	evas_object_show(toast);
    	elm_popup_timeout_set(toast, 2.0);
    	evas_object_smart_callback_add(toast, "timeout", _popup_del, toast);
    	return NULL;
	}
	int sel_count = eina_list_count(sel_list);
	if (sel_count == 1)
	{
		char *buf = malloc(sizeof(char)*1024);
		item = eina_list_data_get(sel_list);
		bible_verse_item *verse_item = (bible_verse_item*)elm_object_item_data_get(item);
		sprintf(buf,"%s ~ %s %d : %d", verse_item->verse, Books[verse_item->bookcount], verse_item->chaptercount, verse_item->versecount + 1);
		return buf;
	}
	else
	{
		char *buf = malloc(sizeof(char) * 1024 * sel_count);
		bible_verse_item *verse_item;
		sprintf(buf, "%s %d : ", Books[ad->cur_book], ad->cur_chapter);

		EINA_LIST_FOREACH(sel_list, sel_list_iter, item)
		{
			verse_item = (bible_verse_item*)elm_object_item_data_get(item);
			sprintf(buf, "%s%d", buf, verse_item->versecount + 1);
			if (eina_list_next(sel_list_iter)) strcat(buf, ", ");
		}
		strcat(buf, "\n\n");
		EINA_LIST_FOREACH(sel_list, sel_list_iter, item)
		{
			verse_item = (bible_verse_item*)elm_object_item_data_get(item);
			sprintf(buf,"%s%d. %s\n", buf, verse_item->versecount + 1, verse_item->verse);
		}
		return buf;
	}
}

static void
_share_verse_done_cb(void *data, Evas_Object *obj, void *event_info)
{
	char *buf;
	appdata_s *ad = (appdata_s*)data;
    buf = _prepare_verse_list(ad);
    if (!buf) return;
	app_control_h handler;
	app_control_create(&handler);
	app_control_set_launch_mode(handler, APP_CONTROL_LAUNCH_MODE_GROUP);
	app_control_set_operation(handler, APP_CONTROL_OPERATION_SHARE_TEXT);
	app_control_add_extra_data(handler, APP_CONTROL_DATA_TEXT, buf);
	app_control_send_launch_request(handler, NULL, NULL);
	app_control_destroy(handler);
	free(buf);
	evas_object_smart_callback_del(obj, "clicked", _share_verse_done_cb);
	_cancel_cb(ad, NULL, NULL);
}

static void
_copy_verse_done_cb(void *data, Evas_Object *obj, void *event_info)
{
	char *buf;
	appdata_s *ad = (appdata_s*)data;
    buf = _prepare_verse_list(ad);
    if (!buf) return;
	elm_cnp_selection_set(obj, ELM_SEL_TYPE_CLIPBOARD, ELM_SEL_FORMAT_TEXT, buf, strlen(buf));
	free(buf);
	evas_object_smart_callback_del(obj, "clicked", _copy_verse_done_cb);
	_cancel_cb(ad, NULL, NULL);
}

void
_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = (appdata_s*)data;
	Eina_List *sel_list_iter;
	Elm_Object_Item *item;
	Eina_List *sel_list = elm_genlist_selected_items_get(ad->genlist);
	EINA_LIST_FOREACH(sel_list, sel_list_iter, item)
	{
		elm_genlist_item_selected_set(item, EINA_FALSE);
	}
	Evas_Object *done_btn = elm_layout_content_get(ad->layout, "title_right_button");
	evas_object_smart_callback_del(done_btn, "clicked", _share_verse_done_cb);
	evas_object_smart_callback_del(done_btn, "clicked", _copy_verse_done_cb);
	ad->share_copy_mode = EINA_FALSE;
	elm_layout_signal_emit(ad->layout, "elm,holy_bible,share_copy,off", "elm");
}

void
_share_verse_cb(void *data, Evas_Object *obj, void *event_info)
{
	bible_verse_item *verse_item = (bible_verse_item*)data;
	appdata_s *ad = verse_item->appdata;

	elm_ctxpopup_dismiss(obj);
	Evas_Object *done_btn = elm_layout_content_get(ad->layout, "title_right_button");
	evas_object_smart_callback_add(done_btn, "clicked", _share_verse_done_cb, ad);
	ad->share_copy_mode = EINA_TRUE;
	elm_genlist_item_selected_set(verse_item->it, EINA_TRUE);
	elm_layout_signal_emit(ad->layout, "elm,holy_bible,share_copy,on", "elm");
}

void
_copy_verse_cb(void *data, Evas_Object *obj, void *event_info)
{
	bible_verse_item *verse_item = (bible_verse_item*)data;
	appdata_s *ad = verse_item->appdata;

	elm_ctxpopup_dismiss(obj);
	Evas_Object *done_btn = elm_layout_content_get(ad->layout, "title_right_button");
	evas_object_smart_callback_add(done_btn, "clicked", _copy_verse_done_cb, ad);
	ad->share_copy_mode = EINA_TRUE;
	elm_genlist_item_selected_set(verse_item->it, EINA_TRUE);
	elm_layout_signal_emit(ad->layout, "elm,holy_bible,share_copy,on", "elm");
}
