#include <tizen.h>
#include "bible.h"

static void
_reset_verse_widget_db(Evas_Object *obj, bible_verse_item *verse_item)
{
	_clear_table("versewidget");
	Elm_Object_Item *it = elm_genlist_first_item_get(obj);
	while(it)
	{
		bible_verse_item *verse_item = (bible_verse_item*)elm_object_item_data_get(it);
		_insert_widget_verse(verse_item->bookcount, verse_item->chaptercount, verse_item->versecount, verse_item->verse);
		it = elm_genlist_item_next_get(it);
	}
}

static void
_verse_widget_item_del_cb(void *data, Evas_Object *obj)
{
	bible_verse_item *verse_item = (bible_verse_item*)data;
	free(verse_item->verse);
	free(verse_item);
}

static void
_delete_widget_verse_cb(void *data, Evas_Object *obj, void *event_info)
{
	bible_verse_item *verse_item = (bible_verse_item*)data;
	Evas_Object *genlist = (Evas_Object*)evas_object_data_get(obj, "genlist");
	Evas_Object *popup = (Evas_Object*)evas_object_data_get(obj, "popup");
	if (elm_genlist_items_count(genlist) == 1)
	{
		Evas_Object *layout = elm_object_parent_widget_get(genlist);
		elm_layout_signal_emit(layout, "elm,holy_bible,bg,show", "elm");
	}
	elm_object_item_del(verse_item->it);
	_reset_verse_widget_db(genlist, verse_item);
	_popup_del(popup, popup, NULL);
}

static void
_verse_widget_del_item(void *data, Evas_Object *obj, void *event_info)
{
	char text[128];
	Evas_Object *button;
	bible_verse_item *verse_item = (bible_verse_item*)data;
	Evas_Object *popup = elm_popup_add(verse_item->appdata->naviframe);
	elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, 1.0);
	elm_object_part_text_set(popup, "title,text", DELETE);
	sprintf(text, "%s %s %d : %d ?", REMOVE, Books[verse_item->bookcount], verse_item->chaptercount, verse_item->versecount + 1);
	elm_object_text_set(popup, text);
	button = elm_button_add(popup);
	elm_object_text_set(button, YES);
	evas_object_smart_callback_add(button, "clicked", _delete_widget_verse_cb, data);
	evas_object_data_set(button, "genlist", evas_object_data_get(obj, "genlist"));
	evas_object_data_set(button, "popup", popup);
	elm_object_part_content_set(popup, "button2", button);
	button = elm_button_add(popup);
	elm_object_text_set(button, NO);
	evas_object_smart_callback_add(button, "clicked", _popup_del, popup);
	elm_object_part_content_set(popup, "button1", button);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, _popup_del, popup);
	evas_object_show(popup);
}

Evas_Object*
_verse_widget_content_get_cb(void *data, Evas_Object *obj, const char *part)
{
	if (!strcmp(part, "elm.swallow.content"))
	{
		char text[1024];
		char *path = NULL;
		bible_verse_item *verse_item = (bible_verse_item*)data;
		Evas_Object *layout = elm_layout_add(obj);
		elm_layout_file_set(layout, verse_item->appdata->edj_path, "widget_verse_item_layout");
		Evas_Object *icon = elm_image_add(obj);
		path = app_get_resource_path();
		if (path)
		{
			sprintf(text, "%s%s", path, "edje/images/delete.png");
			elm_image_file_set(icon, text, NULL);
			free(path);
		}
		evas_object_size_hint_min_set(icon, ELM_SCALE_SIZE(24), ELM_SCALE_SIZE(24));
		evas_object_show(icon);
		evas_object_data_set(icon, "genlist", obj);
		evas_object_smart_callback_add(icon, "clicked", _verse_widget_del_item, data);
		sprintf(text, "%s %d:%d", Books[verse_item->bookcount], verse_item->chaptercount, verse_item->versecount + 1);
		elm_layout_text_set(layout, "elm.text.reference", text);
		elm_layout_text_set(layout, "elm.text.verse", verse_item->verse);
		elm_layout_content_set(layout, "elm.icon", icon);
		elm_layout_sizing_eval(layout);
		evas_object_show(layout);
		return layout;
	}
	else return NULL;
}

void
_verse_item_widget_cb(void *data, Evas_Object *obj, void *event_info)
{
	bible_verse_item *verse_item = (bible_verse_item*)data;
	_popup_del(obj, NULL, NULL);
	_delete_widget_verse(verse_item->bookcount,
			verse_item->chaptercount, verse_item->versecount);
	_insert_widget_verse(verse_item->bookcount,
			verse_item->chaptercount, verse_item->versecount, verse_item->verse);
	Evas_Object *toast = elm_popup_add(verse_item->appdata->naviframe);
	elm_object_style_set(toast, "toast");
	elm_popup_allow_events_set(toast, EINA_TRUE);
	elm_object_text_set(toast, VERSE_ADDED_TO_WIDGET);
	elm_popup_timeout_set(toast, 2.0);
	evas_object_show(toast);
	evas_object_smart_callback_add(toast, "timeout", eext_popup_back_cb, toast);
}

int
_get_verse(void *data, int argc, char **argv, char **azColName)
{
	Elm_Object_Item *it;
	Evas_Object *genlist = (Evas_Object*)data;
	Elm_Genlist_Item_Class *itc = (Elm_Genlist_Item_Class*)evas_object_data_get(genlist, "itc");
	appdata_s *ad = (appdata_s*)evas_object_data_get(genlist, "appdata");
	bible_verse_item *verse_item = (bible_verse_item*)malloc(sizeof(bible_verse_item));
	verse_item->bookcount = atoi(argv[0]);
	verse_item->chaptercount = atoi(argv[1]);
	verse_item->versecount = atoi(argv[2]);
	verse_item->verse = strdup(argv[3]);
	verse_item->appdata = ad;
	it = elm_genlist_item_append(genlist, itc, verse_item, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	verse_item->it = it;
	return 0;
}

static void
_genlist_item_reorder_cb(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item*)event_info;
	bible_verse_item *verse_item = (bible_verse_item*)elm_object_item_data_get(it);
	_clear_table("versewidget");
	_reset_verse_widget_db(obj, verse_item);
}

void
_verse_display_widget_list(appdata_s *ad)
{
	char query[128];
	Evas_Object *layout = elm_layout_add(ad->naviframe);
	elm_layout_file_set(layout, ad->edj_path, "bookmarks_layout");
	Evas_Object *genlist = elm_genlist_add(layout);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	elm_genlist_select_mode_set(genlist, ELM_OBJECT_SELECT_MODE_NONE);
	elm_genlist_reorder_mode_set(genlist, EINA_TRUE);
	evas_object_smart_callback_add(genlist, "moved", _genlist_item_reorder_cb, ad);
	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	itc->item_style = "full";
	itc->func.text_get = NULL;
	itc->func.content_get = _verse_widget_content_get_cb;
	itc->func.del = _verse_widget_item_del_cb;
	evas_object_data_set(genlist, "itc", itc);
	evas_object_data_set(genlist, "appdata", ad);
	sprintf(query, "select * from versewidget;");
	_app_database_query(query, &_get_verse, genlist);
	elm_layout_content_set(layout, "elm.swallow.content", genlist);
	if (elm_genlist_items_count(genlist) > 0)
		elm_layout_signal_emit(layout, "elm,holy_bible,bg,hide", "elm");
	elm_naviframe_item_push(ad->naviframe, "Verse Widget", NULL, NULL, layout, NULL);
	elm_genlist_item_class_free(itc);
}
