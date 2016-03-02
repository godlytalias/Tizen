#include <tizen.h>
#include "bible.h"

static Eina_Bool
verse_navi_pop_cb(void *data, Elm_Object_Item *it)
{
	Elm_Object_Item *sel_it = (Elm_Object_Item*)data;
	elm_genlist_item_selected_set(sel_it, EINA_FALSE);
	Evas_Object *layout = elm_object_item_content_get(it);
	bible_verse_item *verse_it = (bible_verse_item*)
								  evas_object_data_get(layout,
										  "verse_item");
	elm_genlist_item_show(verse_it->it, ELM_GENLIST_ITEM_SCROLLTO_MIDDLE);
	return EINA_TRUE;
}

static Evas_Object*
_create_verse_show_view(Evas_Object *layout, bible_verse_item *verse_item)
{
	appdata_s *ad = verse_item->appdata;
	int fontsize;
	int readmode = 0;
	char style[256];

	evas_object_data_set(layout, "verse_item", verse_item);
    Evas_Object *scroller = elm_scroller_add(layout);
    Evas_Object *verse_layout = elm_layout_add(layout);
	evas_object_size_hint_align_set(verse_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(verse_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_layout_file_set(verse_layout, ad->edj_path, "verse_show_in_layout");
    Evas_Object *entry = elm_entry_add(layout);
    elm_entry_editable_set(entry, EINA_FALSE);
	preference_get_int("fontsize", &fontsize);
	preference_get_int("readmode", &readmode);
	if (fontsize < 25) fontsize = 40;
	else fontsize += 30;
	if (readmode == 1)
	{
		sprintf(style, "DEFAULT='font=Tizen:style=Regular align=left font_size=%d color=#000000 wrap=mixed'hilight=' + font_weight=Bold'", fontsize);
	}
	else
	{
		sprintf(style, "DEFAULT='font=Tizen:style=Regular align=left font_size=%d color=#ffffff wrap=mixed'hilight=' + font_weight=Bold'", fontsize);
	    elm_layout_signal_emit(layout, "elm,holy_bible,night_mode,on", "elm");
	}

	elm_layout_signal_emit(layout, "elm,holy_bible,labels,hide", "elm");
	if (verse_item->bookmark) elm_layout_signal_emit(layout, "elm,holy_bible,bookmark,show", "elm");
	if (verse_item->note) elm_layout_signal_emit(layout, "elm,holy_bible,note,show", "elm");

    if (verse_item->verse_s)
    	elm_layout_signal_emit(verse_layout, "elm,holy_bible,verse_view,sec", "elm");

	elm_entry_text_style_user_push(entry, style);
    elm_entry_entry_set(entry, verse_item->verse);
    elm_layout_content_set(verse_layout, "elm.swallow.verse", entry);

    if (verse_item->verse_s)
    {
    	Evas_Object *entry_s = elm_entry_add(layout);
    	elm_entry_editable_set(entry_s, EINA_FALSE);
    	elm_entry_text_style_user_push(entry_s, style);
    	elm_entry_entry_set(entry_s, verse_item->verse_s);
    	elm_layout_content_set(verse_layout, "elm.swallow.verse_s", entry_s);
    }

    elm_object_content_set(scroller, verse_layout);
    return scroller;
}

static void
_content_mouse_down(void *data,
		Evas *evas EINA_UNUSED,
		Evas_Object *obj,
		void *event_info)
{
	appdata_s *ad = (appdata_s*)data;

	Evas_Event_Mouse_Down *ev = (Evas_Event_Mouse_Down*)event_info;
	ad->mouse_x = ev->canvas.x;
	ad->mouse_y = ev->canvas.y;
	ad->mouse_down_time = ev->timestamp;
}

static void
_transit_del(void *data, Elm_Transit *transit)
{
	Evas_Object *obj = (Evas_Object*)data;
	bible_verse_item *next_verse_item = (bible_verse_item*)evas_object_data_get(obj, "next_verse_item");
	if (next_verse_item)
	{
		Evas_Object *layout = (Evas_Object*)evas_object_data_get(obj, "layout");
		elm_layout_content_unset(layout, "elm.swallow.verse");
		Evas_Object *new_verse = _create_verse_show_view(layout, next_verse_item);
		elm_layout_content_set(layout, "elm.swallow.verse", new_verse);
		evas_object_freeze_events_set(layout, EINA_FALSE);
	}
	evas_object_del(obj);
}

static void
_verse_show(Evas_Object *layout)
{
	char reference[256];
	bible_verse_item *verse_item = (bible_verse_item*)evas_object_data_get(layout, "verse_item");
	Evas_Object *scroller = elm_layout_content_get(layout, "elm.swallow.verse");
	Evas_Object *in_verse_layout = elm_object_content_get(scroller);
	Evas_Object *entry = elm_layout_content_get(in_verse_layout, "elm.swallow.verse");
	elm_entry_entry_set(entry, verse_item->verse);
	if (verse_item->verse_s)
	{
		Evas_Object *entry_s = elm_layout_content_get(in_verse_layout, "elm.swallow.verse_s");
		elm_entry_entry_set(entry_s, verse_item->verse_s);
	}
    sprintf(reference, "%s %d : %d",
    		Books[verse_item->bookcount], verse_item->chaptercount,
    		verse_item->versecount + 1);
    elm_layout_text_set(layout, "elm.text.reference", reference);
}

static Eina_Bool
_longpress_cb(void *data)
{
	Evas_Object *obj = (Evas_Object*)data;
	appdata_s *ad = (appdata_s*)evas_object_data_get(obj, "appdata");
	Elm_Object_Item *item;
	bible_verse_item *verse_item = (bible_verse_item*)evas_object_data_get(obj, "verse_item");
	ad->long_pressed = EINA_TRUE;

	if (ad->long_press_mode == 0)
		item = elm_genlist_first_item_get(ad->genlist);
	else
		item = elm_genlist_last_item_get(ad->genlist);
	if (item)
	{
		verse_item = elm_object_item_data_get(item);
		evas_object_data_set(obj, "verse_item", verse_item);
		_verse_show(obj);
	}
	ad->long_timer = NULL;
	return ECORE_CALLBACK_CANCEL;
}

static void
_next_up(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	appdata_s *ad = (appdata_s*)data;
	bible_verse_item *verse_item = (bible_verse_item*)evas_object_data_get(obj, "verse_item");
	Elm_Object_Item *item;

	if (ad->long_timer)
	{
		ecore_timer_del(ad->long_timer);
		ad->long_timer = NULL;
	}
	if (!ad->long_pressed)
	{
		item = elm_genlist_item_next_get(verse_item->it);
		if (item)
		{
			verse_item = elm_object_item_data_get(item);
			evas_object_data_set(obj, "verse_item", verse_item);
			_verse_show(obj);
		}
	}
	ad->long_pressed = EINA_FALSE;
}

static void
_next_down(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	appdata_s *ad = (appdata_s*)data;
	ad->long_pressed = EINA_FALSE;
	ad->long_press_mode = 1;
	if (ad->long_timer)
	{
		ecore_timer_del(ad->long_timer);
		ad->long_timer = NULL;
	}
	ad->long_timer = ecore_timer_add(LONGPRESS_TIMEOUT, _longpress_cb, obj);
}

static void
_prev_up(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	appdata_s *ad = (appdata_s*)data;
	bible_verse_item *verse_item = (bible_verse_item*)evas_object_data_get(obj, "verse_item");
	Elm_Object_Item *item;

	if (ad->long_timer)
	{
		ecore_timer_del(ad->long_timer);
		ad->long_timer = NULL;
	}
	if (!ad->long_pressed)
	{
		item = elm_genlist_item_prev_get(verse_item->it);
		if (item)
		{
			verse_item = elm_object_item_data_get(item);
			evas_object_data_set(obj, "verse_item", verse_item);
			_verse_show(obj);
		}
	}
	ad->long_pressed = EINA_FALSE;
}

static void
_prev_down(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	appdata_s *ad = (appdata_s*)data;
	ad->long_pressed = EINA_FALSE;
	ad->long_press_mode = 0;
	if (ad->long_timer)
	{
		ecore_timer_del(ad->long_timer);
		ad->long_timer = NULL;
	}
	ad->long_timer = ecore_timer_add(LONGPRESS_TIMEOUT, _longpress_cb, obj);
}

static void
_content_mouse_up(void *data,
		Evas *evas EINA_UNUSED,
		Evas_Object *obj,
		void *event_info)
{
	Evas_Coord x, y, w, h;
	Elm_Object_Item *next_it;
	char reference[256];
	appdata_s *ad = (appdata_s*)data;

	int x_del, y_del;
	Evas_Event_Mouse_Up *ev = (Evas_Event_Mouse_Up*)event_info;
	if (ad->long_timer) return;
	if ((ev->timestamp - ad->mouse_down_time) > 1000) return;
	x_del = ev->canvas.x - ad->mouse_x;
	y_del = ev->canvas.y - ad->mouse_y;
	if (abs(x_del) < (2 * abs(y_del))) return;
	if (abs(x_del) < 100) return;

	bible_verse_item *verse_item = (bible_verse_item*)evas_object_data_get(obj, "verse_item");
	if (x_del < 0)
		next_it = elm_genlist_item_next_get(verse_item->it);
	else
		next_it = elm_genlist_item_prev_get(verse_item->it);
	if (!next_it) return;

	evas_object_freeze_events_set(obj, EINA_TRUE);
	Evas_Object *verse = elm_layout_content_unset(obj, "elm.swallow.verse");
	evas_object_geometry_get(verse, &x, &y, NULL, NULL);
	evas_object_geometry_get(obj, NULL, NULL, &w, &h);

	bible_verse_item *next_verse_item = (bible_verse_item*)elm_object_item_data_get(next_it);
	Elm_Transit *transit = elm_transit_add();
	elm_transit_object_add(transit, verse);

	Evas_Object *next_transit_obj = _create_verse_show_view(obj, next_verse_item);
	Elm_Transit *transit_nxt = elm_transit_add();
	elm_transit_object_add(transit_nxt, next_transit_obj);

	if (x_del < 0)
	{
		elm_transit_effect_translation_add(transit, x, 0, x - w, 0);
		elm_transit_effect_translation_add(transit_nxt, x + w, y, x, y);
	}
	else
	{
		elm_transit_effect_translation_add(transit, x, 0, x + w, 0);
		elm_transit_effect_translation_add(transit_nxt, x - w, y, x, y);
	}
    sprintf(reference, "%s %d : %d",
    		Books[next_verse_item->bookcount], next_verse_item->chaptercount,
    		next_verse_item->versecount + 1);
    elm_layout_text_set(obj, "elm.text.reference", reference);
	elm_layout_content_set(obj, "elm.swallow.verse", next_transit_obj);
	elm_transit_tween_mode_set(transit, ELM_TRANSIT_TWEEN_MODE_DECELERATE);
	elm_transit_tween_mode_set(transit_nxt, ELM_TRANSIT_TWEEN_MODE_DECELERATE);
	elm_transit_duration_set(transit, 0.3);
	elm_transit_duration_set(transit_nxt, 0.3);
	elm_transit_del_cb_set(transit, _transit_del, verse);
	elm_transit_del_cb_set(transit_nxt, _transit_del, next_transit_obj);
	evas_object_data_set(next_transit_obj, "next_verse_item", next_verse_item);
	evas_object_data_set(next_transit_obj, "layout", obj);
	elm_transit_go(transit_nxt);
	elm_transit_go(transit);
}

void
_bible_verse_show(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = (appdata_s*)data;
	Elm_Object_Item *it = (Elm_Object_Item*)event_info;
	Elm_Object_Item *nf_it;
	char reference[256];

	bible_verse_item *verse_item = (bible_verse_item*)elm_object_item_data_get(it);
	Evas_Object *layout = elm_layout_add(obj);
	elm_layout_file_set(layout, ad->edj_path, "verse_show_layout");
    sprintf(reference, "%s %d : %d",
    		Books[verse_item->bookcount], verse_item->chaptercount, verse_item->versecount + 1);
    elm_layout_text_set(layout, "elm.text.reference", reference);
    Evas_Object *verse_view = _create_verse_show_view(layout, verse_item);
    elm_layout_content_set(layout, "elm.swallow.verse", verse_view);
	nf_it = elm_naviframe_item_push(ad->naviframe, NULL, NULL, NULL, layout, NULL);
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_TRUE);
	elm_naviframe_item_pop_cb_set(nf_it, verse_navi_pop_cb, it);
	ad->long_timer = NULL;
	evas_object_data_set(layout, "appdata", ad);
	evas_object_event_callback_add(layout, EVAS_CALLBACK_MOUSE_DOWN, _content_mouse_down, ad);
	evas_object_event_callback_add(layout, EVAS_CALLBACK_MOUSE_UP, _content_mouse_up, ad);
	elm_layout_signal_callback_add(layout, "elm,holy_bible,next,up", "elm", _next_up, ad);
	elm_layout_signal_callback_add(layout, "elm,holy_bible,next,down", "elm", _next_down, ad);
	elm_layout_signal_callback_add(layout, "elm,holy_bible,prev,up", "elm", _prev_up, ad);
	elm_layout_signal_callback_add(layout, "elm,holy_bible,prev,down", "elm", _prev_down, ad);
}
