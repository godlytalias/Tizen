#include <tizen.h>
#include <sqlite3.h>
#include "bible.h"

static void
ctxpopup_dismissed_cb(void *data, Evas_Object *obj, void *event_info)
{
	evas_object_del(data);
	data = NULL;
}

static void
_popup_del(void *data, Evas_Object *obj, void *event_info)
{
	evas_object_del(data);
	data = NULL;
}

static void
move_more_ctxpopup(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *win;
	Evas_Coord w, h;
	int pos = -1;
	Evas_Object *ctxpopup = (Evas_Object*)data;

	/* We convince the top widget is a window */
	win = elm_object_top_widget_get(ctxpopup);
	elm_win_screen_size_get(win, NULL, NULL, &w, &h);
	pos = elm_win_rotation_get(win);

	switch (pos) {
		case 0:
		case 180:
			evas_object_move(ctxpopup, (w / 2), h);
			break;
		case 90:
			evas_object_move(ctxpopup,  (h / 2), w);
			break;
		case 270:
			evas_object_move(ctxpopup, (h / 2), w);
			break;
	}
}

static void
ctxpopup_item_select_cb(void *data, Evas_Object *obj, void *event_info)
{
	const char *title_label = elm_object_item_text_get((Elm_Object_Item *) event_info);
	char text_content[2048];
	Evas_Object *ctxpopup = (Evas_Object*)data;
	Evas_Object *popup = elm_popup_add(elm_object_top_widget_get(obj));
	elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, 0.5);
	elm_object_part_text_set(popup, "title,text", title_label);
	Evas_Object *content_box = elm_box_add(popup);
	evas_object_size_hint_weight_set(content_box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(content_box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	if (!strcmp(title_label, "About"))
	{
		Evas_Object *label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		sprintf(text_content,_("<align=center><b>HOLY BIBLE (MKJV)</b></align>"));
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		sprintf(text_content,_("<align=center><font_size=20>Copyright © 2015</font_size></align>"));
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		sprintf(text_content,_("<align=center><em><font_size=20>GTA v0.1</font_size></em></align>"));
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		sprintf(text_content,_(" "));
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		sprintf(text_content, _("<align=center><font_size=20>"
	"This program is free software: you can redistribute it and/or modify "
	"it under the terms of the GNU General Public License as published by "
	"the Free Software Foundation, either version 3 of the License, or "
	"(at your option) any later version.</font_size></align>"));

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		sprintf(text_content, _("<align=center><font_size=20>"
		"This program is distributed in the hope that it will be useful, "
		"but WITHOUT ANY WARRANTY; without even the implied warranty of "
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the "
		"GNU General Public License for more details.</font_size></align>"));

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		sprintf(text_content,_(" "));
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		sprintf(text_content, _("<align=center><font_size=25>"
		"Report the bugs or suggestions to "
		"Godly T.Alias (<em>godlytalias@yahoo.co.in</em>).</font_size></align>"));

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);
	}
	else if (!strcmp(title_label, "Help"))
	{
		Evas_Object *label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		sprintf(text_content,_("<align=center><b>HOLY BIBLE (MKJV)</b></align>"));
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		sprintf(text_content,_("<align=center><font_size=20>Copyright © 2015</font_size></align>"));
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		sprintf(text_content,_("<align=center><em><font_size=20>GTA v0.1</font_size></em></align>"));
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		sprintf(text_content,_(" "));
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		sprintf(text_content, _("<align=left><font_size=25>"
	"<b>Changing chapters</b></font_size></align>"));

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);


		sprintf(text_content, _("<align=left><font_size=20>"
		"Users can change the chapters either by clicking the "
		"Back / Next buttons available in the bottom bar "
		"or by swiping left / right in the screen. Also if user clicks "
		"on the header part, new window with all the chapters will be listed out "
		"where user can select the Book and Chapter which they want.</font_size></align>"));

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		sprintf(text_content,_(" "));
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		sprintf(text_content, _("<align=left><font_size=25>"
	"<b>Selecting Books / Chapters</b></font_size></align>"));

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);


		sprintf(text_content, _("<align=left><font_size=20>"
		"User can click on the header part which opens a new window with all "
		"the Books and Chapters listed out "
		"where user can select the Book and Chapter which they want.</font_size></align>"));

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		sprintf(text_content,_(" "));
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);

		sprintf(text_content, _("<align=center><font_size=15>"
		"Report the bugs or suggestions to "
		"Godly T.Alias (<em>godlytalias@yahoo.co.in</em>).</font_size></align>"));

		label = elm_label_add(popup);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_label_line_wrap_set(label, ELM_WRAP_WORD);
		elm_object_text_set(label, text_content);
		evas_object_show(label);
		elm_box_pack_end(content_box, label);
	}
	elm_object_content_set(popup, content_box);
	evas_object_show(content_box);
	Evas_Object *button = elm_button_add(popup);
	elm_object_text_set(button, "OK");
	elm_object_part_content_set(popup, "button1", button);
	evas_object_show(button);
	evas_object_show(popup);
	evas_object_smart_callback_add(button, "clicked", _popup_del, popup);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, eext_popup_back_cb, NULL);
	elm_ctxpopup_dismiss(ctxpopup);
}

void
create_ctxpopup_more_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *win;
	appdata_s *ad = (appdata_s*)data;

	Evas_Object *ctxpopup = elm_ctxpopup_add(ad->win);
	elm_ctxpopup_auto_hide_disabled_set(ctxpopup, EINA_TRUE);
	elm_object_style_set(ctxpopup, "more/default");
	eext_object_event_callback_add(ctxpopup, EEXT_CALLBACK_BACK, eext_ctxpopup_back_cb, NULL);
	eext_object_event_callback_add(ctxpopup, EEXT_CALLBACK_MORE, eext_ctxpopup_back_cb, NULL);
	evas_object_smart_callback_add(ctxpopup, "dismissed", ctxpopup_dismissed_cb, ctxpopup);

	win = elm_object_top_widget_get(ad->naviframe);
	evas_object_smart_callback_add(win, "rotation,changed", move_more_ctxpopup, ctxpopup);

	elm_ctxpopup_item_append(ctxpopup, "About", NULL, ctxpopup_item_select_cb, ctxpopup);
	elm_ctxpopup_item_append(ctxpopup, "Help", NULL, ctxpopup_item_select_cb, ctxpopup);

	elm_ctxpopup_direction_priority_set(ctxpopup, ELM_CTXPOPUP_DIRECTION_UP, ELM_CTXPOPUP_DIRECTION_UNKNOWN, ELM_CTXPOPUP_DIRECTION_UNKNOWN, ELM_CTXPOPUP_DIRECTION_UNKNOWN);
	move_more_ctxpopup(ctxpopup, ctxpopup, NULL);
	evas_object_show(ctxpopup);
}
