#include <tizen.h>
#include "widget.h"

static Eina_Bool
_window_close(void *data, Elm_Object_Item *it)
{
	widget_instance_data_s *wid = (widget_instance_data_s*)data;
	edje_text_class_set("GTAwidget", "Tizen:style=Regular", wid->font_size);
	edje_color_class_set("GTAwidget", wid->text_r, wid->text_g, wid->text_b, wid->text_a, 0, 0, 0, 0, 0, 0, 0, 0);
	edje_color_class_set("GTAwidgetbg", wid->bg_r, wid->bg_g, wid->bg_b, wid->bg_a, 0, 0, 0, 0, 0, 0, 0, 0);
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
		if (!strcmp(part, "elm.text.multiline"))
		{
			int val;
			char value[8];
			preference_get_int("fontsize", &val);
			sprintf(value, "%d", val);
			return strdup(value);
		}
		else return NULL;
		break;
	case 3:
		if (!strcmp(part, "elm.text"))
			return strdup("Font color");
		if (!strcmp(part, "elm.text.multiline"))
		{
			char color[64];
			sprintf(color, "Red: %d<br>Green: %d<br>Blue: %d<br>Alpha: %d", wid->text_r, wid->text_g, wid->text_b, wid->text_a);
			return strdup(color);
		}
		else return NULL;
		break;
	case 4:
		if (!strcmp(part, "elm.text"))
			return strdup("Background Color");
		if (!strcmp(part, "elm.text.multiline"))
		{
			char color[64];
			sprintf(color, "Red: %d<br>Green: %d<br>Blue: %d<br>Alpha: %d", wid->bg_r, wid->bg_g, wid->bg_b, wid->bg_a);
			return strdup(color);
		}
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
	widget_instance_data_s *wid = (widget_instance_data_s*)data;
	elm_genlist_item_selected_set(it, EINA_FALSE);
	int option_id = atoi((char*)elm_object_item_data_get(it));
	Evas_Object *popup, *colorsel, *box;
	switch(option_id)
	{
	case 1: break;
	case 2: break;
	case 3: break;
	case 4:
		popup = elm_popup_add(wid->settings_nf);
		evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(popup, EVAS_HINT_FILL, EVAS_HINT_FILL);
		box = elm_box_add(popup);
		elm_box_padding_set(box, 32, 32);
		evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, 0.5);


		Evas_Object *check = elm_check_add(popup);
		evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_object_text_set(check, "Transparent Background");
		elm_box_pack_end(box, check);
		evas_object_show(check);

		Evas_Object *button = elm_button_add(popup);
		elm_object_text_set(button, "Close");
		elm_object_part_content_set(popup, "button1", button);
		evas_object_smart_callback_add(button, "clicked", eext_popup_back_cb, NULL);

		evas_object_show(box);
		elm_object_content_set(popup, box);
		eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, eext_popup_back_cb, NULL);
		evas_object_show(popup);
		break;
	default: break;
	}
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
	wid->settings_nf = elm_naviframe_add(wid->settings_window);
	Evas_Object *genlist = elm_genlist_add(wid->settings_nf);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	evas_object_data_set(genlist, "widget_data", wid);
	_add_option_items(genlist, wid);
	elm_win_resize_object_add(wid->settings_window, wid->settings_nf);
	Elm_Object_Item *nf_it = elm_naviframe_item_push(wid->settings_nf, "Bible Widget", NULL, NULL, genlist, NULL);
	elm_naviframe_item_pop_cb_set(nf_it, _window_close, wid);
	eext_object_event_callback_add(wid->settings_nf, EEXT_CALLBACK_BACK, eext_naviframe_back_cb, NULL);
	evas_object_show(wid->settings_nf);
	evas_object_show(wid->settings_window);
}
