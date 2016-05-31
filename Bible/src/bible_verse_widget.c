#include <tizen.h>
#include "bible.h"

static void
_verse_widget_item_del_cb(void *data, Evas_Object *obj)
{
	bible_verse_item *verse_item = (bible_verse_item*)data;
	free(verse_item->verse);
	free(verse_item);
}

static void
_verse_widget_del_item(void *data, Evas_Object *obj, void *event_info)
{
	bible_verse_item *verse_item = (bible_verse_item*)data;
	Evas_Object *genlist = (Evas_Object*)evas_object_data_get(obj, "genlist");
	if (elm_genlist_items_count(genlist) == 1)
	{
		Evas_Object *layout = elm_object_parent_widget_get(genlist);
		elm_layout_signal_emit(layout, "elm,holy_bible,bg,show", "elm");
	}
	_delete_widget_verse(verse_item->bookcount, verse_item->chaptercount, verse_item->versecount);
	elm_object_item_del(verse_item->it);
}

Evas_Object*
_verse_widget_content_get_cb(void *data, Evas_Object *obj, const char *part)
{
	if (!strcmp(part, "elm.swallow.end"))
	{
		Evas_Object *ic_cancel = elm_button_add(obj);
		elm_object_style_set(ic_cancel, "circle");
		evas_object_size_hint_min_set(ic_cancel, ELM_SCALE_SIZE(48), ELM_SCALE_SIZE(48));
		Evas_Object *icon = elm_icon_add(obj);
		elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_THEME_FDO);
		elm_icon_standard_set(icon, "delete");
		evas_object_size_hint_min_set(icon, ELM_SCALE_SIZE(32), ELM_SCALE_SIZE(32));
		evas_object_show(icon);
		elm_object_content_set(ic_cancel, icon);
		evas_object_data_set(ic_cancel, "genlist", obj);
		evas_object_smart_callback_add(ic_cancel, "clicked", _verse_widget_del_item, data);
		return ic_cancel;
	}
	else return NULL;
}

char*
_verse_widget_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	char text[1024];
	bible_verse_item *verse_item = (bible_verse_item*)data;
	if (!strcmp(part, "elm.text"))
	{
		sprintf(text, "%s %d:%d", Books[verse_item->bookcount], verse_item->chaptercount, verse_item->versecount + 1);
		return strdup(text);
	}
	else if (!strcmp(part, "elm.text.multiline"))
	{
		return strdup(verse_item->verse);
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
	bible_verse_item *verse_item = (bible_verse_item*)malloc(sizeof(bible_verse_item));
	verse_item->bookcount = atoi(argv[0]);
	verse_item->chaptercount = atoi(argv[1]);
	verse_item->versecount = atoi(argv[2]);
	verse_item->verse = strdup(argv[3]);
	it = elm_genlist_item_append(genlist, itc, verse_item, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	verse_item->it = it;
	return 0;
}

static void
_genlist_item_reorder_cb(void *data, Evas_Object *obj, void *event_info)
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
	itc->item_style = "multiline";
	itc->func.text_get = _verse_widget_text_get_cb;
	itc->func.content_get = _verse_widget_content_get_cb;
	itc->func.del = _verse_widget_item_del_cb;
	evas_object_data_set(genlist, "itc", itc);
	sprintf(query, "select * from versewidget;");
	_app_database_query(query, &_get_verse, genlist);
	elm_layout_content_set(layout, "elm.swallow.content", genlist);
	if (elm_genlist_items_count(genlist) > 0)
		elm_layout_signal_emit(layout, "elm,holy_bible,bg,hide", "elm");
	elm_naviframe_item_push(ad->naviframe, "Verse Widget", NULL, NULL, layout, NULL);
	elm_genlist_item_class_free(itc);
}
