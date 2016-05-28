#include <tizen.h>
#include "widget.h"

static Eina_Bool
_window_close(void *data, Elm_Object_Item *it)
{
	char style[64];
	widget_instance_data_s *wid = (widget_instance_data_s*)data;
	sprintf(style, "Tizen:style=%s", wid->font_style);
	edje_text_class_set("GTAwidget", style, wid->font_size);
	edje_color_class_set("GTAwidget", wid->text_r, wid->text_g, wid->text_b, wid->text_a, 0, 0, 0, 0, 0, 0, 0, 0);
	edje_color_class_set("GTAwidgetbg", wid->bg_r, wid->bg_g, wid->bg_b, wid->bg_a, 0, 0, 0, 0, 0, 0, 0, 0);
	evas_object_del(wid->settings_window);
	return EINA_TRUE;
}

static void
_popup_del(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup = (Evas_Object*)data;
	Evas_Object *genlist = (Evas_Object*)evas_object_data_get(popup, "genlist");
	Elm_Object_Item *it = elm_genlist_selected_item_get(genlist);
	elm_genlist_item_selected_set(it, EINA_FALSE);
	elm_genlist_item_update(it);
	evas_object_del(popup);
}

static void
_font_size_changed(void *data, Evas_Object *obj, void *event_info)
{
	widget_instance_data_s *wid = (widget_instance_data_s*)data;
	wid->font_size = elm_slider_value_get(obj);
	preference_set_int("font_size", wid->font_size);
}

static void
_font_r_changed(void *data, Evas_Object *obj, void *event_info)
{
	widget_instance_data_s *wid = (widget_instance_data_s*)data;
	wid->text_r = elm_slider_value_get(obj);
	preference_set_int("text_r", wid->text_r);
	edje_color_class_set("GTAcolor", wid->text_r, wid->text_g, wid->text_b, wid->text_a, 0, 0, 0, 0, 0, 0, 0, 0);
}

static void
_font_g_changed(void *data, Evas_Object *obj, void *event_info)
{
	widget_instance_data_s *wid = (widget_instance_data_s*)data;
	wid->text_g = elm_slider_value_get(obj);
	preference_set_int("text_g", wid->text_g);
	edje_color_class_set("GTAcolor", wid->text_r, wid->text_g, wid->text_b, wid->text_a, 0, 0, 0, 0, 0, 0, 0, 0);
}

static void
_font_b_changed(void *data, Evas_Object *obj, void *event_info)
{
	widget_instance_data_s *wid = (widget_instance_data_s*)data;
	wid->text_b = elm_slider_value_get(obj);
	preference_set_int("text_b", wid->text_b);
	edje_color_class_set("GTAcolor", wid->text_r, wid->text_g, wid->text_b, wid->text_a, 0, 0, 0, 0, 0, 0, 0, 0);
}

static void
_font_a_changed(void *data, Evas_Object *obj, void *event_info)
{
	widget_instance_data_s *wid = (widget_instance_data_s*)data;
	wid->text_a = elm_slider_value_get(obj);
	preference_set_int("text_a", wid->text_a);
	edje_color_class_set("GTAcolor", wid->text_r, wid->text_g, wid->text_b, wid->text_a, 0, 0, 0, 0, 0, 0, 0, 0);
}

static void
_bg_r_changed(void *data, Evas_Object *obj, void *event_info)
{
	widget_instance_data_s *wid = (widget_instance_data_s*)data;
	wid->bg_r = elm_slider_value_get(obj);
	preference_set_int("bg_r", wid->bg_r);
	edje_color_class_set("GTAcolor", wid->bg_r, wid->bg_g, wid->bg_b, wid->bg_a, 0, 0, 0, 0, 0, 0, 0, 0);
}

static void
_bg_g_changed(void *data, Evas_Object *obj, void *event_info)
{
	widget_instance_data_s *wid = (widget_instance_data_s*)data;
	wid->bg_g = elm_slider_value_get(obj);
	preference_set_int("bg_g", wid->bg_g);
	edje_color_class_set("GTAcolor", wid->bg_r, wid->bg_g, wid->bg_b, wid->bg_a, 0, 0, 0, 0, 0, 0, 0, 0);
}

static void
_bg_b_changed(void *data, Evas_Object *obj, void *event_info)
{
	widget_instance_data_s *wid = (widget_instance_data_s*)data;
	wid->bg_b = elm_slider_value_get(obj);
	preference_set_int("bg_b", wid->bg_b);
	edje_color_class_set("GTAcolor", wid->bg_r, wid->bg_g, wid->bg_b, wid->bg_a, 0, 0, 0, 0, 0, 0, 0, 0);
}

static void
_bg_a_changed(void *data, Evas_Object *obj, void *event_info)
{
	widget_instance_data_s *wid = (widget_instance_data_s*)data;
	wid->bg_a = elm_slider_value_get(obj);
	preference_set_int("bg_a", wid->bg_a);
	edje_color_class_set("GTAcolor", wid->bg_r, wid->bg_g, wid->bg_b, wid->bg_a, 0, 0, 0, 0, 0, 0, 0, 0);
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
			return strdup(DISPLAYED_VERSE);
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
			return strdup(FONT_SIZE);
		if (!strcmp(part, "elm.text.multiline"))
		{
			char value[8];
			sprintf(value, "%d", wid->font_size);
			return strdup(value);
		}
		else return NULL;
		break;
	case 3:
		if (!strcmp(part, "elm.text"))
			return strdup(FONT_COLOR);
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
			return strdup(BG_COLOR);
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

static char*
_genlist_item_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	widget_bible_verse_item *verse_item = (widget_bible_verse_item*)data;
	if (!strcmp(part, "elm.text.multiline"))
		return strdup(verse_item->verse);
	else if (!strcmp(part, "elm.text"))
	{
		char buf[1024];
		sprintf(buf, "%s %d : %d", Books[verse_item->bookcount], verse_item->chaptercount, verse_item->versecount);
		return strdup(buf);
	}
	else return NULL;
}

static void
_genlist_item_del_cb(void *data, Evas_Object *obj)
{
	widget_bible_verse_item *verse_item = (widget_bible_verse_item*)data;
	if (verse_item->verse) free(verse_item->verse);
	if (verse_item) free(verse_item);
}

static void
_genlist_item_sel_cb(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item*)event_info;
	widget_bible_verse_item *verse_item = (widget_bible_verse_item*)elm_object_item_data_get(it);
	if(!elm_genlist_item_selected_get(it))
		elm_genlist_item_selected_set(it, EINA_TRUE);
	verse_item->wid->verse_order = elm_genlist_item_index_get(it);
	_query_verse(verse_item->wid);
}

static void
_type_change_cb(void *data, Evas_Object *obj, void *event_info)
{
	widget_instance_data_s *wid = (widget_instance_data_s*)data;
	int val = elm_radio_value_get(obj);
	wid->font_style = Font_Style[val];
	preference_set_int("font_type", val);
}

static int
_get_verse(void *data, int argc, char **argv, char **azColName)
{
	Evas_Object *genlist = (Evas_Object*)data;
	if (argv == NULL) return 0;
	Elm_Object_Item *it;

	widget_bible_verse_item *verse_item = (widget_bible_verse_item*)malloc(sizeof(widget_bible_verse_item));
	Elm_Genlist_Item_Class *itc = (Elm_Genlist_Item_Class*)evas_object_data_get(genlist, "itc");
	widget_instance_data_s *wid = (widget_instance_data_s*)evas_object_data_get(genlist, "wid");
	verse_item->verse = strdup(argv[3]);
	verse_item->bookcount = atoi(argv[0]);
	verse_item->chaptercount = atoi(argv[1]);
	verse_item->versecount = atoi(argv[2]) + 1;
	verse_item->wid = wid;
	it = elm_genlist_item_append(genlist, itc, verse_item, NULL, ELM_GENLIST_ITEM_NONE, _genlist_item_sel_cb, verse_item);
	if ((verse_item->bookcount == wid->cur_book) &&
			(verse_item->chaptercount == wid->cur_chapter) &&
			(verse_item->versecount == wid->cur_verse))
	{
		elm_genlist_item_selected_set(it, EINA_TRUE);
		elm_genlist_item_show(it, ELM_GENLIST_ITEM_SCROLLTO_IN);
	}
	return 0;
}

static void
_settings_option_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	char buf[1024];
	Evas_Object *slider, *button;
	Elm_Object_Item *it = (Elm_Object_Item*)event_info;
	widget_instance_data_s *wid = (widget_instance_data_s*)data;
	int option_id = atoi((char*)elm_object_item_data_get(it));
	Evas_Object *popup, *layout, *box, *check, *radio_group;
	popup = elm_popup_add(wid->settings_nf);
	evas_object_data_set(popup, "genlist", obj);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(popup, EVAS_HINT_FILL, EVAS_HINT_FILL);
	switch(option_id)
	{
	case 1:
		elm_object_part_text_set(popup, "title,text", DISPLAYED_VERSE);
		Evas_Object *genlist = elm_genlist_add(popup);
		elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
		elm_genlist_highlight_mode_set(genlist, EINA_TRUE);
		elm_genlist_select_mode_set(genlist, ELM_OBJECT_SELECT_MODE_ALWAYS);
		sprintf(buf, "select * from %s;", WIDGET_TABLE_NAME);
		Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
		itc->item_style = "multiline";
		itc->func.text_get = _genlist_item_text_get_cb;
		itc->func.del = _genlist_item_del_cb;
		evas_object_data_set(genlist, "wid", wid);
		evas_object_data_set(genlist, "itc", itc);
		_database_query(buf, &_get_verse, genlist);
		if (elm_genlist_items_count(genlist) == 0)
		{
			widget_bible_verse_item *verse_item = (widget_bible_verse_item*)malloc(sizeof(widget_bible_verse_item));
			verse_item->verse = strdup(DEFAULT_WIDGET_VERSE);
			verse_item->bookcount = 43;
			verse_item->chaptercount = 4;
			verse_item->versecount = 12;
			verse_item->wid = wid;
			Elm_Object_Item *it = elm_genlist_item_append(genlist, itc, verse_item, NULL, ELM_GENLIST_ITEM_NONE, _genlist_item_sel_cb, verse_item);
			elm_genlist_item_selected_set(it, EINA_TRUE);
		}
		elm_object_content_set(popup, genlist);
		elm_genlist_item_class_free(itc);
		break;
	case 2:
		elm_object_part_text_set(popup, "title,text", FONT_SIZE);
		layout = elm_layout_add(popup);
		elm_layout_file_set(layout, wid->edj_path, "standard_layout");
		slider = elm_slider_add(layout);
		elm_slider_min_max_set(slider, 15, 42);
		elm_slider_step_set(slider, 1.0);
		elm_slider_indicator_show_set(slider, EINA_TRUE);
		elm_slider_indicator_format_set(slider, "%1.0f");
		elm_slider_value_set(slider, (double)wid->font_size);
		evas_object_smart_callback_add(slider, "changed", _font_size_changed, wid);
		box = elm_box_add(popup);
		elm_box_horizontal_set(box, EINA_TRUE);
		elm_box_padding_set(box, ELM_SCALE_SIZE(32), ELM_SCALE_SIZE(32));
		check = elm_radio_add(box);
		elm_radio_state_value_set(check, 0);
		radio_group = check;
		elm_object_text_set(check, BOLD);
		evas_object_smart_callback_add(check, "changed", _type_change_cb, wid);
		elm_box_pack_end(box, check);
		evas_object_show(check);
		check = elm_radio_add(box);
		elm_radio_state_value_set(check, 1);
		elm_radio_group_add(check, radio_group);
		elm_object_text_set(check, REGULAR);
		evas_object_smart_callback_add(check, "changed", _type_change_cb, wid);
		elm_box_pack_end(box, check);
		evas_object_show(check);
		check = elm_radio_add(box);
		elm_radio_state_value_set(check, 2);
		elm_radio_group_add(check, radio_group);
		elm_object_text_set(check, LIGHT);
		evas_object_smart_callback_add(check, "changed", _type_change_cb, wid);
		elm_box_pack_end(box, check);
		int type = 1;
		preference_get_int("font_type", &type);
		elm_radio_value_set(radio_group, type);
		evas_object_show(check);
		evas_object_show(box);
		elm_layout_content_set(layout, "elm.swallow.content", slider);
		elm_layout_content_set(layout, "elm.swallow.content2", box);
		evas_object_show(layout);
		elm_object_content_set(popup, layout);
		break;
	case 3:
		elm_object_part_text_set(popup, "title,text", FONT_COLOR);
		layout = elm_layout_add(popup);
		elm_layout_file_set(layout, wid->edj_path, "color_change_layout");
		edje_color_class_set("GTAcolor", wid->text_r, wid->text_g, wid->text_b, wid->text_a, 0, 0, 0, 0, 0, 0, 0, 0);
		evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, 1.0);

		elm_layout_text_set(layout, "elm.text.red", RED);
		slider = elm_slider_add(popup);
		elm_slider_min_max_set(slider, 0, 255);
		elm_slider_step_set(slider, 1.0);
		elm_slider_value_set(slider, (double)wid->text_r);
		elm_slider_indicator_show_set(slider, EINA_TRUE);
		elm_slider_indicator_format_set(slider, "%1.0f");
		evas_object_smart_callback_add(slider, "changed", _font_r_changed, wid);
		elm_layout_content_set(layout, "elm.swallow.red", slider);

		elm_layout_text_set(layout, "elm.text.green", GREEN);
		slider = elm_slider_add(popup);
		elm_slider_min_max_set(slider, 0, 255);
		elm_slider_step_set(slider, 1.0);
		elm_slider_indicator_show_set(slider, EINA_TRUE);
		elm_slider_value_set(slider, (double)wid->text_g);
		elm_slider_indicator_format_set(slider, "%1.0f");
		evas_object_smart_callback_add(slider, "changed", _font_g_changed, wid);
		elm_layout_content_set(layout, "elm.swallow.green", slider);

		elm_layout_text_set(layout, "elm.text.blue", BLUE);
		slider = elm_slider_add(popup);
		elm_slider_min_max_set(slider, 0, 255);
		elm_slider_step_set(slider, 1.0);
		elm_slider_indicator_show_set(slider, EINA_TRUE);
		elm_slider_indicator_format_set(slider, "%1.0f");
		elm_slider_value_set(slider, (double)wid->text_b);
		evas_object_smart_callback_add(slider, "changed", _font_b_changed, wid);
		elm_layout_content_set(layout, "elm.swallow.blue", slider);

		elm_layout_text_set(layout, "elm.text.alpha", ALPHA);
		slider = elm_slider_add(popup);
		elm_slider_min_max_set(slider, 0, 255);
		elm_slider_step_set(slider, 1.0);
		elm_slider_indicator_show_set(slider, EINA_TRUE);
		elm_slider_indicator_format_set(slider, "%1.0f");
		elm_slider_value_set(slider, (double)wid->text_a);
		evas_object_smart_callback_add(slider, "changed", _font_a_changed, wid);
		elm_layout_content_set(layout, "elm.swallow.alpha", slider);
		evas_object_show(slider);

		evas_object_show(layout);
		elm_object_content_set(popup, layout);
		break;
	case 4:
		elm_object_part_text_set(popup, "title,text", BG_COLOR);
		layout = elm_layout_add(popup);
		elm_layout_file_set(layout, wid->edj_path, "color_change_layout");
		edje_color_class_set("GTAcolor", wid->bg_r, wid->bg_g, wid->bg_b, wid->bg_a, 0, 0, 0, 0, 0, 0, 0, 0);
		evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, 1.0);

		elm_layout_text_set(layout, "elm.text.red", RED);
		slider = elm_slider_add(popup);
		elm_slider_min_max_set(slider, 0, 255);
		elm_slider_step_set(slider, 1.0);
		elm_slider_indicator_show_set(slider, EINA_TRUE);
		elm_slider_value_set(slider, (double)wid->bg_r);
		elm_slider_indicator_format_set(slider, "%1.0f");
		evas_object_smart_callback_add(slider, "changed", _bg_r_changed, wid);
		elm_layout_content_set(layout, "elm.swallow.red", slider);

		elm_layout_text_set(layout, "elm.text.green", GREEN);
		slider = elm_slider_add(popup);
		elm_slider_min_max_set(slider, 0, 255);
		elm_slider_step_set(slider, 1.0);
		elm_slider_indicator_show_set(slider, EINA_TRUE);
		elm_slider_indicator_format_set(slider, "%1.0f");
		elm_slider_value_set(slider, (double)wid->bg_g);
		evas_object_smart_callback_add(slider, "changed", _bg_g_changed, wid);
		elm_layout_content_set(layout, "elm.swallow.green", slider);

		elm_layout_text_set(layout, "elm.text.blue", BLUE);
		slider = elm_slider_add(popup);
		elm_slider_min_max_set(slider, 0, 255);
		elm_slider_step_set(slider, 1.0);
		elm_slider_indicator_show_set(slider, EINA_TRUE);
		elm_slider_indicator_format_set(slider, "%1.0f");
		elm_slider_value_set(slider, (double)wid->bg_b);
		evas_object_smart_callback_add(slider, "changed", _bg_b_changed, wid);
		elm_layout_content_set(layout, "elm.swallow.blue", slider);

		elm_layout_text_set(layout, "elm.text.alpha", ALPHA);
		slider = elm_slider_add(popup);
		elm_slider_min_max_set(slider, 0, 255);
		elm_slider_step_set(slider, 1.0);
		elm_slider_indicator_show_set(slider, EINA_TRUE);
		elm_slider_indicator_format_set(slider, "%1.0f");
		elm_slider_value_set(slider, (double)wid->bg_a);
		evas_object_smart_callback_add(slider, "changed", _bg_a_changed, wid);
		elm_layout_content_set(layout, "elm.swallow.alpha", slider);

		elm_layout_text_set(layout, "elm.text.color", YOUR_COLOR_HERE);
		elm_object_content_set(popup, layout);
		break;
	default: break;
	}

	button = elm_button_add(popup);
	elm_object_text_set(button, CLOSE);
	elm_object_part_content_set(popup, "button1", button);
	evas_object_smart_callback_add(button, "clicked", _popup_del, popup);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, _popup_del, popup);
	evas_object_show(popup);
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
	elm_win_indicator_mode_set(wid->settings_window, ELM_WIN_INDICATOR_SHOW);
	elm_win_indicator_opacity_set(wid->settings_window, ELM_WIN_INDICATOR_OPAQUE);
	Evas_Object *conformant = elm_conformant_add(wid->settings_window);
	wid->settings_nf = elm_naviframe_add(conformant);
	elm_object_content_set(conformant, wid->settings_nf);
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
