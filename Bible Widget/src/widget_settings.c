#include <tizen.h>
#include "widget.h"

static Eina_Bool
_window_close(void *data, Elm_Object_Item *it)
{
	widget_instance_data_s *wid = (widget_instance_data_s*)data;
	evas_object_del(wid->settings_window);
	return EINA_TRUE;
}

static char*
_settings_option_text_get(void *data, Evas_Object *obj, const char *part)
{
	int id = (atoi(data));
	widget_instance_data_s *wid = (widget_instance_data_s*)evas_object_data_get(obj, "widget_data");
	switch(id)
	{
	case 1:
		if (!strcmp(part, "elm.text"))
			return strdup("Displayed Verse");
		else if (!strcmp(part, "elm.text.multiline"))
		{
			char verse[1024];
			sprintf(verse, "<font color=#0000ff>%s %d:%d</font><br>%s", Books[wid->cur_book], wid->cur_chapter, wid->cur_verse, wid->verse);
			return strdup(verse);
		}
		else return NULL;
		break;
	case 2:
		if (!strcmp(part, "elm.text"))
			return strdup("Font size");
		else return NULL;
		break;
	case 3:
		if (!strcmp(part, "elm.text"))
			return strdup("Font color");
		else return NULL;
		break;
	case 4:
		if (!strcmp(part, "elm.text"))
			return strdup("Background Color");
		else return NULL;
		break;
	default:
		return NULL;
		break;
	}
	return NULL;
}

static void
_settings_option_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item*)event_info;
	elm_genlist_item_selected_set(it, EINA_FALSE);
}

static void
_add_option_items(Evas_Object *genlist, widget_instance_data_s *wid)
{
	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	itc->item_style = "multiline";
	itc->func.text_get = _settings_option_text_get;
	elm_genlist_item_append(genlist, itc, "1", NULL, ELM_GENLIST_ITEM_NONE, _settings_option_selected_cb, wid);
	elm_genlist_item_append(genlist, itc, "2", NULL, ELM_GENLIST_ITEM_NONE, _settings_option_selected_cb, wid);
	elm_genlist_item_append(genlist, itc, "3", NULL, ELM_GENLIST_ITEM_NONE, _settings_option_selected_cb, wid);
	elm_genlist_item_append(genlist, itc, "4", NULL, ELM_GENLIST_ITEM_NONE, _settings_option_selected_cb, wid);
	elm_genlist_item_class_free(itc);
}

void
_widget_settings(widget_instance_data_s *wid)
{
	wid->settings_window = elm_win_util_standard_add("Bible Widget", "Bible Widget");
	Evas_Object *nf = elm_naviframe_add(wid->settings_window);
	Evas_Object *genlist = elm_genlist_add(nf);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	evas_object_data_set(genlist, "widget_data", wid);
	_add_option_items(genlist, wid);
	elm_win_resize_object_add(wid->settings_window, nf);
	Elm_Object_Item *nf_it = elm_naviframe_item_push(nf, "Bible Widget", NULL, NULL, genlist, NULL);
	elm_naviframe_item_pop_cb_set(nf_it, _window_close, wid);
	eext_object_event_callback_add(nf, EEXT_CALLBACK_BACK, eext_naviframe_back_cb, NULL);
	evas_object_show(nf);
	evas_object_show(wid->settings_window);
}
