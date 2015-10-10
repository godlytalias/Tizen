#include <tizen.h>
#include <sqlite3.h>
#include "bible.h"

static void
win_delete_request_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = (appdata_s*)data;
	if (ad->db) sqlite3_close(ad->db);
	elm_genlist_item_class_free(ad->itc);
	sqlite3_shutdown();
	ui_app_exit();
}

static void
layout_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = data;
	/* Let window go to hide state. */
	elm_win_lower(ad->win);
}

void
app_get_resource(const char *edj_file_in, char *edj_path_out, int edj_path_max)
{
	char *res_path = app_get_resource_path();
	if (res_path) {
		snprintf(edj_path_out, edj_path_max, "%s%s", res_path, edj_file_in);
		free(res_path);
	}
}

static void
_prev_chapter(void *data,
              Evas_Object *obj ,
              void *event_info EINA_UNUSED)
{
	appdata_s *ad = (appdata_s*)data;
	if (ad->cur_book == 0 && ad->cur_chapter == 1) return;
	if (ad->cur_chapter == 1) {
		_get_chapter_count_query(data, ad->cur_book - 1);
		_query_chapter(data, ad->cur_book - 1, ad->chaptercount);
	}
	else
		_query_chapter(data, ad->cur_book, ad->cur_chapter - 1);

}

static void
_nxt_chapter(void *data,
              Evas_Object *obj ,
              void *event_info EINA_UNUSED)
{
	appdata_s *ad = (appdata_s*)data;
	if (ad->cur_book == 65 && ad->cur_chapter == 22) return;
	_get_chapter_count_query(data, ad->cur_book);
	if (ad->cur_chapter == ad->chaptercount)
	{
		_query_chapter(data, ad->cur_book + 1, 1);
	}
	else
		_query_chapter(data, ad->cur_book, ad->cur_chapter + 1);
}



static Evas_Object*
gl_content_get_cb(void *data, Evas_Object *obj, const char *part)
{
    bible_verse_item *verse_item = (bible_verse_item*)data;
    if(strcmp(part, "elm.swallow.content") == 0)
    {
    	Evas_Object *layout = elm_layout_add(obj);
    	char edj_path[PATH_MAX] = {0, };
    	app_get_resource(EDJ_FILE, edj_path, (int)PATH_MAX);
    	elm_layout_file_set(layout, edj_path, "verse_layout");
    	Evas_Object *entry = elm_entry_add(obj);
    	elm_entry_editable_set(entry, EINA_FALSE);
    	evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    	evas_object_size_hint_align_set(entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
    	elm_entry_entry_set(entry, verse_item->verse);
    	evas_object_show(entry);
    	elm_object_part_content_set(layout,"elm.swallow.verse",entry);
    	sprintf(edj_path, "%d", verse_item->versecount+1);
    	elm_object_part_text_set(layout, "elm.text.verse_count", edj_path);
    	evas_object_show(layout);
    	return layout;
    }
    else return NULL;
}

static void
gl_del_cb(void *data, Evas_Object *obj)
{
   bible_verse_item *verse_item = (bible_verse_item*)data;
   free(verse_item->verse);
   free(verse_item);
}

static Evas_Object*
_home_screen(appdata_s *ad)
{
	Evas_Object *prev_btn, *nxt_btn, *search_btn;
	char edj_path[PATH_MAX] = {0, };
	/* Base Layout */
	app_get_resource(EDJ_FILE, edj_path, (int)PATH_MAX);
	ad->layout = elm_layout_add(ad->win);
	elm_layout_file_set(ad->layout, edj_path, GRP_MAIN);
	evas_object_size_hint_weight_set(ad->layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(ad->layout);
	elm_win_resize_object_add(ad->win, ad->layout);
	elm_layout_signal_callback_add(ad->layout, "elm,book,change", "elm", _change_book, (void*)ad);

	prev_btn = elm_button_add(ad->win);
	elm_object_text_set(prev_btn, "Back");
	elm_object_part_content_set(ad->layout, "elm.footer.prev.btn", prev_btn);
	evas_object_smart_callback_add(prev_btn, "clicked", _prev_chapter, (void*)ad);
	evas_object_show(prev_btn);

	nxt_btn = elm_button_add(ad->win);
	elm_object_text_set(nxt_btn, "Next");
	elm_object_part_content_set(ad->layout, "elm.footer.nxt.btn", nxt_btn);
	evas_object_smart_callback_add(nxt_btn, "clicked", _nxt_chapter, (void*)ad);
	evas_object_show(nxt_btn);

	search_btn = elm_button_add(ad->win);
	elm_object_text_set(search_btn, "Search");
	//elm_object_part_content_set(ad->layout, "elm.footer.search.btn", search_btn);
	//evas_object_smart_callback_add(search_btn, "clicked", _search_word, (void*)ad);
	//evas_object_show(search_btn);

	ad->itc = elm_genlist_item_class_new();
	ad->itc->item_style = "full";
	ad->itc->func.content_get = gl_content_get_cb;
	ad->itc->func.text_get = NULL;
	ad->itc->func.del = gl_del_cb;

	ad->genlist = elm_genlist_add(ad->layout);
	evas_object_size_hint_align_set(ad->genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(ad->genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_genlist_realization_mode_set(ad->genlist, EINA_TRUE);
	elm_object_part_content_set(ad->layout, "elm.swallow.content", ad->genlist);
	elm_genlist_mode_set(ad->genlist, ELM_LIST_COMPRESS);
	evas_object_smart_callback_add(ad->genlist, "drag,start,left", _nxt_chapter, ad);
	evas_object_smart_callback_add(ad->genlist, "drag,start,right", _prev_chapter, ad);

	_query_chapter((void*)ad, 0, 1);

	evas_object_show(ad->genlist);
	return ad->layout;
}

static Eina_Bool
naviframe_pop_cb(void *data, Elm_Object_Item *it)
{
	ui_app_exit();
	return EINA_FALSE;
}

static void
create_base_gui(appdata_s *ad)
{
	Elm_Object_Item *nf_item;
	Evas_Object *menu_btn;
	ad->win = elm_win_util_standard_add(PACKAGE, PACKAGE);
	elm_win_autodel_set(ad->win, EINA_TRUE);

	if (elm_win_wm_rotation_supported_get(ad->win)) {
		int rots[4] = { 0, 90, 180, 270 };
		elm_win_wm_rotation_available_rotations_set(ad->win, (const int *)(&rots), 4);
	}

	evas_object_smart_callback_add(ad->win, "delete,request", win_delete_request_cb, NULL);
	ad->naviframe = elm_naviframe_add(ad->win);
	elm_win_resize_object_add(ad->win, ad->naviframe);
	evas_object_size_hint_weight_set(ad->naviframe, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(ad->naviframe, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(ad->naviframe);
	eext_object_event_callback_add(ad->naviframe, EEXT_CALLBACK_BACK, eext_naviframe_back_cb, NULL);
	nf_item = elm_naviframe_item_push(ad->naviframe, NULL, NULL, NULL, _home_screen(ad), NULL);
	elm_naviframe_item_title_enabled_set(nf_item, EINA_FALSE, EINA_TRUE);
	elm_naviframe_item_pop_cb_set(nf_item, naviframe_pop_cb, ad);

	menu_btn = elm_button_add(ad->naviframe);
	elm_object_style_set(menu_btn, "naviframe/more/default");
	evas_object_smart_callback_add(menu_btn, "clicked", create_ctxpopup_more_button_cb, ad);
	elm_object_item_part_content_set(nf_item, "toolbar_more_btn", menu_btn);
	evas_object_show(menu_btn);

	eext_object_event_callback_add(ad->naviframe, EEXT_CALLBACK_BACK, eext_naviframe_back_cb, NULL);
	eext_object_event_callback_add(ad->naviframe, EEXT_CALLBACK_MORE, eext_naviframe_more_cb, NULL);

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
