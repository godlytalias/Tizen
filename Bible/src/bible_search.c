#include <tizen.h>
#include <sqlite3.h>
#include "bible.h"

static Eina_Bool _hoversel_item_add(void *data);

static Eina_Bool
_genlist_free_idler(void *data)
{
	appdata_s *ad = (appdata_s*)data;
	if (ad->search_result_genlist)
		elm_genlist_clear(ad->search_result_genlist);
	ad->search_result_genlist = NULL;
	if (ad->search_itc)
		elm_genlist_item_class_free(ad->search_itc);
	ad->search_itc = NULL;
	return ECORE_CALLBACK_CANCEL;
}

static void
_go_top(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	appdata_s *ad = (appdata_s*)data;
	elm_genlist_item_bring_in(elm_genlist_first_item_get(ad->search_result_genlist), ELM_GENLIST_ITEM_SCROLLTO_IN);
}

static void
_go_bottom(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	appdata_s *ad = (appdata_s*)data;
	elm_genlist_item_bring_in(elm_genlist_last_item_get(ad->search_result_genlist), ELM_GENLIST_ITEM_SCROLLTO_IN);
}

Evas_Object*
search_gl_content_get_cb(void *data, Evas_Object *obj, const char *part)
{
    bible_verse_item *verse_item = (bible_verse_item*)data;
    if(strcmp(part, "elm.swallow.content") == 0)
    {
    	char reference[512];
    	Evas_Object *layout = elm_layout_add(obj);
    	elm_layout_file_set(layout, verse_item->appdata->edj_path, "search_verse_layout");
    	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    	sprintf(reference, "%s %d : %d", Books[verse_item->bookcount], verse_item->chaptercount, verse_item->versecount+1);
    	elm_object_part_text_set(layout, "elm.text.reference", reference);
    	elm_object_part_text_set(layout, "elm.text.verse", verse_item->verse);
    	evas_object_show(layout);
    	return layout;
    }
    else return NULL;
}

static void
search_gl_del_cb(void *data, Evas_Object *obj)
{
   bible_verse_item *verse_item = (bible_verse_item*)data;
   free(verse_item->verse);
   free(verse_item);
   verse_item = NULL;
}

static void
_dismiss_verse_popup(void *data, Evas_Object *obj, void *event_info)
{
	evas_object_hide(data);
	evas_object_del(data);
}

static void
_copy_verse(void *data, Evas_Object *obj, void *event_info)
{
	bible_verse_item *verse_item = (bible_verse_item*)data;
	char buf[2048];
	if (strcmp(elm_entry_selection_get(obj), verse_item->verse)) return;
	elm_entry_select_none(obj);
	sprintf(buf, "%s ~ %s %d : %d", verse_item->verse, Books[verse_item->bookcount], verse_item->chaptercount, verse_item->versecount + 1);
	elm_cnp_selection_set(obj, ELM_SEL_TYPE_CLIPBOARD, ELM_SEL_FORMAT_TEXT, buf, strlen(buf));
	return;
}

static void
_search_result_selected(void *data, Evas_Object *obj, void *event_info)
{
	bible_verse_item *verse_item = (bible_verse_item*)data;
	char title[128];
	char style[256];
	int fontsize = 25;
	Elm_Object_Item *item = (Elm_Object_Item*)event_info;
	Evas_Object *verse_popup = elm_popup_add(verse_item->appdata->search_layout);
	elm_popup_align_set(verse_popup, ELM_NOTIFY_ALIGN_FILL, 1.0);
	sprintf(title, "%s %d : %d", Books[verse_item->bookcount], verse_item->chaptercount, verse_item->versecount + 1);
	elm_object_part_text_set(verse_popup, "title,text", title);
	Evas_Object *layout = elm_layout_add(obj);
	elm_layout_file_set(layout, verse_item->appdata->edj_path, "standard_layout");
	Evas_Object *verse_entry = elm_entry_add(obj);
	elm_entry_editable_set(verse_entry, EINA_FALSE);
	evas_object_size_hint_weight_set(verse_entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	preference_get_int("fontsize", &fontsize);
	sprintf(style, "DEFAULT='font=Tizen:style=Regular align=left font_size=%d color=#000000 wrap=mixed'hilight=' + font_weight=Bold'", fontsize + 5);
	elm_entry_text_style_user_push(verse_entry, style);
	elm_entry_entry_set(verse_entry, verse_item->verse);
	evas_object_smart_callback_add(verse_entry,"selection,copy",_copy_verse,(void*)verse_item);
	elm_layout_content_set(layout, "elm.swallow.content", verse_entry);
	elm_object_content_set(verse_popup, layout);
	Evas_Object *ok = elm_button_add(verse_popup);
	elm_object_text_set(ok, OK);
	evas_object_smart_callback_add(ok, "clicked", _dismiss_verse_popup, verse_popup);
	elm_object_part_content_set(verse_popup, "button1", ok);
	evas_object_show(verse_popup);
	eext_object_event_callback_add(verse_popup, EEXT_CALLBACK_BACK, eext_popup_back_cb, NULL);
	elm_genlist_item_selected_set(item, EINA_FALSE);
}

static void
_gl_longpressed_cb(void *data, Evas_Object *obj, void *event_info)
{
    Elm_Object_Item *item = (Elm_Object_Item*)event_info;
    char popup_text[128];
    bible_verse_item *verse_item = (bible_verse_item*)elm_object_item_data_get(item);
    appdata_s *ad = (appdata_s*)data;
    Evas_Object *popup = elm_popup_add(ad->win);
	elm_genlist_item_selected_set(item, EINA_FALSE);
    elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, 0.5);
    sprintf(popup_text, "<align='center'>%s %s %d : %d ?</align>",GO_TO, Books[verse_item->bookcount], verse_item->chaptercount, verse_item->versecount + 1);
    elm_object_text_set(popup, popup_text);
    Evas_Object *button1 = elm_button_add(ad->win);
    elm_object_text_set(button1, NO);
    evas_object_smart_callback_add(button1, "clicked", _popup_del, popup);
    Evas_Object *button2 = elm_button_add(ad->win);
    elm_object_text_set(button2, YES);
    evas_object_smart_callback_add(button2, "clicked", _get_chapter, popup);
    evas_object_data_set(popup, "verse_item", verse_item);
    elm_object_part_content_set(popup, "button1", button1);
    elm_object_part_content_set(popup, "button2", button2);
    eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, eext_popup_back_cb, popup);
    evas_object_show(popup);
}

static void
_up_arrow_show(void *data, Evas_Object *obj, void *event_info)
{
   appdata_s *ad = (appdata_s*)data;
   elm_layout_signal_emit(ad->search_layout, "elm,holy_bible,up,show", "elm");
}

static void
_down_arrow_show(void *data, Evas_Object *obj, void *event_info)
{
   appdata_s *ad = (appdata_s*)data;
   elm_layout_signal_emit(ad->search_layout, "elm,holy_bible,down,show", "elm");
}

static int
_get_search_results(void *data, int argc, char **argv, char **azColName)
{
	   appdata_s *ad = (appdata_s*) data;
	   Elm_Object_Item *gl_item;
	   int i;
	   bible_verse_item *verse_item = malloc(sizeof(bible_verse_item));
	   for (i = 0; i < argc; i++)
	   {
		   if (!strcmp(azColName[i], "Book"))
			   verse_item->bookcount = atoi(argv[i]);
		   if (!strcmp(azColName[i], "Chapter"))
			   verse_item->chaptercount = atoi(argv[i]);
		   if (!strcmp(azColName[i], "Versecount")) {
			   verse_item->versecount = atoi(argv[i]);
			   verse_item->versecount--;
		   }
		   if (!strcmp(azColName[i], BIBLE_VERSE_COLUMN))
			   verse_item->verse = strdup(argv[i]);
	   }
	   verse_item->appdata = ad;

	   gl_item = elm_genlist_item_append(ad->search_result_genlist, ad->search_itc, (void*)verse_item, NULL, ELM_GENLIST_ITEM_NONE, _search_result_selected, (void*)verse_item);
	   elm_object_item_data_set(gl_item, verse_item);
	   return 0;
}

static void
_bible_search_query(char* search_query, appdata_s *ad)
{
	if (ad->search_result_genlist)
		_genlist_free_idler(ad);
	char toast[64];
	int res_count = 0;
	ad->search_result_genlist = elm_genlist_add(ad->naviframe);
	elm_object_style_set(ad->search_result_genlist, "handler");
	evas_object_size_hint_weight_set(ad->search_result_genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(ad->search_result_genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_genlist_select_mode_set(ad->search_result_genlist, ELM_OBJECT_SELECT_MODE_ALWAYS);
	elm_genlist_homogeneous_set(ad->search_result_genlist, EINA_TRUE);
	ad->search_itc = elm_genlist_item_class_new();
	ad->search_itc->item_style = "full";
	ad->search_itc->func.content_get = search_gl_content_get_cb;
	ad->search_itc->func.text_get = NULL;
	ad->search_itc->func.del = search_gl_del_cb;
	_database_query(search_query, _get_search_results, ad);
	elm_object_part_content_set(ad->search_layout, "elm.swallow.result", ad->search_result_genlist);
	evas_object_smart_callback_add(ad->search_result_genlist, "drag,start,up", _down_arrow_show, ad);
	evas_object_smart_callback_add(ad->search_result_genlist, "drag,start,down", _up_arrow_show, ad);
	evas_object_smart_callback_add(ad->search_result_genlist, "longpressed", _gl_longpressed_cb, ad);
	elm_layout_signal_callback_add(ad->search_layout, "elm,holy_bible,top", "elm", _go_top, ad);
	elm_layout_signal_callback_add(ad->search_layout, "elm,holy_bible,bottom", "elm", _go_bottom, ad);
	evas_object_show(ad->search_result_genlist);
	res_count = elm_genlist_items_count(ad->search_result_genlist);
	if (res_count > 0)
		elm_layout_signal_emit(ad->search_layout, "elm,holy_bible,bg,hide", "elm");
	else
		elm_layout_signal_emit(ad->search_layout, "elm,holy_bible,bg,show", "elm");
	sprintf(toast, "Got %d results", res_count);
	Evas_Object *toastp = elm_popup_add(ad->naviframe);
	elm_object_style_set(toastp, "toast");
	elm_popup_allow_events_set(toastp, EINA_TRUE);
	elm_object_text_set(toastp, toast);
	evas_object_show(toastp);
	elm_popup_timeout_set(toastp, 2.0);
	evas_object_smart_callback_add(toastp, "timeout", eext_popup_back_cb, toastp);
	elm_object_focus_set(ad->search_result_genlist, EINA_TRUE);
}

static void
_search_keyword(void *data,
        Evas_Object *obj ,
        void *event_info EINA_UNUSED)
{
	appdata_s *ad = (appdata_s*)data;
	char *keyword = strdup(elm_entry_entry_get(ad->search_entry));
	char *ch;
	char keyword_query[2560];
	char search_query[2560];
	char condition_key[5];
	if (keyword && (strlen(keyword) > 1024))
	{
		Evas_Object *toast_popup = elm_popup_add(ad->naviframe);
		elm_popup_timeout_set(toast_popup, 2);
		elm_object_style_set(toast_popup, "toast");
		elm_object_text_set(toast_popup, SEARCH_KEYWORD_IS_TOO_LARGE);
		evas_object_smart_callback_add(toast_popup, "timeout", _dismiss_verse_popup, toast_popup);
		evas_object_show(toast_popup);
		return;
	}
	else if (keyword && (strlen(keyword) < 2))
		{
			Evas_Object *toast_popup = elm_popup_add(ad->naviframe);
			elm_popup_timeout_set(toast_popup, 2);
			elm_object_style_set(toast_popup, "toast");
			elm_object_text_set(toast_popup, SEARCH_KEYWORD_IS_TOO_SMALL);
			evas_object_smart_callback_add(toast_popup, "timeout", _dismiss_verse_popup, toast_popup);
			evas_object_show(toast_popup);
			return;
		}

	_loading_progress(ad->win);
	if (keyword) {
		ch = strtok(keyword, " ");
		if (elm_check_state_get(ad->check_whole))
		   sprintf(keyword_query, "( %s LIKE '%% %s %%'", BIBLE_VERSE_COLUMN, ch);
		else
		   sprintf(keyword_query, "( %s LIKE '%%%s%%'", BIBLE_VERSE_COLUMN, ch);
	}
	ch = strtok(NULL, " ");
	if (elm_check_state_get(ad->check_strict))
		strcpy(condition_key, "AND");
	else
		strcpy(condition_key, "OR");
	while (ch)
	{
		if (elm_check_state_get(ad->check_whole))
		   sprintf(keyword_query, "%s %s %s LIKE '%% %s %%'", keyword_query, condition_key, BIBLE_VERSE_COLUMN, ch);
		else
		   sprintf(keyword_query, "%s %s %s LIKE '%%%s%%'", keyword_query, condition_key, BIBLE_VERSE_COLUMN, ch);
		ch = strtok(NULL, " ");
	}
	strcat(keyword_query, " )");
	if (!elm_check_state_get(ad->check_entire))
	{
		char book_list[1024];
		int t;
		if (elm_check_state_get(ad->check_nt))
		{
			strcpy(book_list, "'");
			strcat(book_list, Books[39]);
			strcat(book_list, "'");
			for (t = 40; t < 66; t++)
			{
				strcat(book_list, ", '");
				strcat(book_list, Books[t]);
				strcat(book_list, "'");
			}
		}
		else if (elm_check_state_get(ad->check_ot))
		{
			strcpy(book_list, "'");
			strcat(book_list, Books[0]);
			strcat(book_list, "'");
			for (t = 1; t < 39; t++)
			{
				strcat(book_list, ", '");
				strcat(book_list, Books[t]);
				strcat(book_list, "'");
			}
		}
		else if (elm_check_state_get(ad->check_custom))
		{
			strcpy(book_list, "'");
			strcat(book_list, Books[ad->search_from]);
			strcat(book_list, "'");
			for (t = ad->search_from + 1; t <= ad->search_to; t++)
			{
				strcat(book_list, ", '");
				strcat(book_list, Books[t]);
				strcat(book_list, "'");
			}
		}
		sprintf(keyword_query, "%s AND Book in (%s)", keyword_query, book_list);
	}
	sprintf(search_query, "SELECT Book, Chapter, Versecount, %s FROM %s WHERE %s;", BIBLE_VERSE_COLUMN, BIBLE_TABLE_NAME, keyword_query);
	_bible_search_query(search_query, ad);
}

static void
panel_toggle(void *data, Evas_Object *obj, void *event_info)
{
	if (!obj)
	{
		appdata_s *ad = (appdata_s*)data;
		Evas_Object* panel_btn = (Evas_Object*) elm_object_item_part_content_get
				  (elm_naviframe_top_item_get(ad->naviframe), "drawers");
		evas_object_smart_callback_call(panel_btn, "clicked", NULL);
	}
	else
	{
		Evas_Object* panel = (Evas_Object*)data;
		appdata_s *ad = (appdata_s*)evas_object_data_get(panel, "appdata");
		if (!elm_object_disabled_get(panel)) elm_panel_toggle(panel);
		if (!elm_panel_hidden_get(panel))
		{
			if (elm_check_state_get(ad->check_custom) && elm_object_disabled_get(ad->to_dropdown))
			{
				elm_check_state_set(ad->check_entire, EINA_TRUE);
				evas_object_smart_callback_call(ad->check_entire, "changed", NULL);
			}
		}
		else
		{
			if (elm_hoversel_expanded_get(ad->from_dropdown))
				elm_hoversel_hover_end(ad->from_dropdown);
			if (elm_hoversel_expanded_get(ad->to_dropdown))
				elm_hoversel_hover_end(ad->to_dropdown);
		}
	}
}

static Evas_Object*
create_drawer_layout(Evas_Object *parent)
{
	Evas_Object *layout;
	layout = elm_layout_add(parent);
	elm_layout_theme_set(layout, "layout", "drawer", "panel");
	evas_object_show(layout);

	return layout;
}

static void
_dropdown_item_select(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = (appdata_s*)data;
	int i;
	Elm_Object_Item *item = (Elm_Object_Item*)event_info;
	const char *book = elm_object_item_text_get(item);
	elm_object_text_set(obj, book);
	if (obj != ad->to_dropdown)
	{
		elm_object_text_set(ad->to_dropdown, book);
		for (i = 0; i < 66 && strcmp(book, Books[i]); i++);
		ad->search_from = i;
		ad->search_to = i;
		evas_object_data_set(ad->to_dropdown, "appdata", (void*)ad);
		elm_hoversel_clear(ad->to_dropdown);
		ecore_idler_add(_hoversel_item_add, ad->to_dropdown);
	}
	else
	{
		for (i = ad->search_from; i < 66 && strcmp(book, Books[i]); i++);
		ad->search_to = i;
	}
}

static Eina_Bool
_hoversel_item_creation(void *data)
{
	int i;
	Evas_Object *hoversel = (Evas_Object*)data;
	appdata_s *ad = (appdata_s*)evas_object_data_get(hoversel, "appdata");
	elm_object_disabled_set(hoversel, EINA_FALSE);
	for(i = ad->search_from; i < 66; i++)
			elm_hoversel_item_add(hoversel, Books[i], NULL, 0, _dropdown_item_select, ad);
	Evas_Object *loading = (Evas_Object*)evas_object_data_get(hoversel, "loading");
	evas_object_del(loading);
	return ECORE_CALLBACK_DONE;
}

static Eina_Bool
_hoversel_item_add(void *data)
{
	Evas_Object *hoversel = (Evas_Object*)data;
	appdata_s *ad = (appdata_s*)evas_object_data_get(hoversel, "appdata");
	Evas_Object *loading = elm_popup_add(ad->naviframe);
	elm_object_style_set(loading, "toast");
	elm_object_text_set(loading, LOADING_BOOKS);
	evas_object_data_set(hoversel, "loading", loading);
	evas_object_show(loading);
	ecore_timer_add(0.3, _hoversel_item_creation, hoversel);
	return ECORE_CALLBACK_DONE;
}

static void
_entire_pref_changed(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = (appdata_s*)data;
	if (elm_check_state_get(ad->check_entire))
	{
		elm_check_state_set(ad->check_nt, EINA_FALSE);
		elm_check_state_set(ad->check_ot, EINA_FALSE);
		elm_check_state_set(ad->check_custom, EINA_FALSE);
		elm_object_disabled_set(ad->from_dropdown, EINA_TRUE);
		elm_object_text_set(ad->from_dropdown, Books[0]);
		elm_object_disabled_set(ad->to_dropdown, EINA_TRUE);
		elm_object_text_set(ad->to_dropdown, Books[65]);
	}
	else elm_check_state_set(ad->check_entire, EINA_TRUE);
}

static void
_nt_pref_changed(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = (appdata_s*)data;
	if (elm_check_state_get(ad->check_nt))
	{
		elm_check_state_set(ad->check_entire, EINA_FALSE);
		elm_check_state_set(ad->check_ot, EINA_FALSE);
		elm_check_state_set(ad->check_custom, EINA_FALSE);
		elm_object_disabled_set(ad->from_dropdown, EINA_TRUE);
		elm_object_text_set(ad->from_dropdown, Books[39]);
		elm_object_disabled_set(ad->to_dropdown, EINA_TRUE);
		elm_object_text_set(ad->to_dropdown, Books[65]);
	}
	else elm_check_state_set(ad->check_nt, EINA_TRUE);
}

static void
_ot_pref_changed(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = (appdata_s*)data;
	if (elm_check_state_get(ad->check_ot))
	{
		elm_check_state_set(ad->check_entire, EINA_FALSE);
		elm_check_state_set(ad->check_nt, EINA_FALSE);
		elm_check_state_set(ad->check_custom, EINA_FALSE);
		elm_object_disabled_set(ad->from_dropdown, EINA_TRUE);
		elm_object_text_set(ad->from_dropdown, Books[0]);
		elm_object_disabled_set(ad->to_dropdown, EINA_TRUE);
		elm_object_text_set(ad->to_dropdown, Books[38]);
	}
	else elm_check_state_set(ad->check_ot, EINA_TRUE);
}

static void
_custom_pref_changed(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = (appdata_s*)data;
	if (elm_check_state_get(ad->check_custom))
	{
		elm_check_state_set(ad->check_entire, EINA_FALSE);
		elm_check_state_set(ad->check_ot, EINA_FALSE);
		elm_check_state_set(ad->check_nt, EINA_FALSE);
		ad->search_from = 0;
		ad->search_to = 65;
		evas_object_data_set(ad->from_dropdown, "appdata", (void*)ad);
		if (eina_list_count(elm_hoversel_items_get(ad->from_dropdown)) < 66)
		   ecore_idler_add(_hoversel_item_add, ad->from_dropdown);
		else
			elm_object_disabled_set(ad->from_dropdown, EINA_FALSE);
		elm_object_text_set(ad->from_dropdown, SELECT_BOOK);
		elm_object_text_set(ad->to_dropdown, SELECT_BOOK);
	}
	else elm_check_state_set(ad->check_custom, EINA_TRUE);
}

static Evas_Object*
create_panel(appdata_s *ad)
{
	Evas_Object *panel, *layout, *rect;
	char label_str[256];

	/* Panel */
	panel = elm_panel_add(ad->naviframe);
	elm_panel_scrollable_set(panel, EINA_TRUE);

	/* Default is visible, hide the content in default. */
	elm_panel_hidden_set(panel, EINA_TRUE);
	evas_object_show(panel);

	Evas_Object *scroller = elm_scroller_add(panel);
	evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(scroller, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(scroller);

	layout = elm_layout_add(scroller);
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_layout_file_set(layout, ad->edj_path, "panel_layout");
	elm_layout_text_set(layout, "elm.text.title", SEARCH_PREFERENCES);

	Evas_Object *main_table = elm_table_add(panel);
	evas_object_size_hint_weight_set(main_table, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(main_table, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_table_homogeneous_set(main_table, EINA_TRUE);
	evas_object_show(main_table);
	elm_table_padding_set(main_table, 0, ELM_SCALE_SIZE(16));

	Evas_Object *label = elm_label_add(panel);
	sprintf(label_str, "<color=#000000FF><align=left><font_size=30>%s</font></align></color>", ENTIRE_BIBLE);
	elm_object_text_set(label, label_str);
	evas_object_show(label);
	elm_table_pack(main_table, label, 0, 0, 1, 1);

	ad->check_entire = elm_check_add(panel);
	elm_object_style_set(ad->check_entire, "on&off");
	evas_object_show(ad->check_entire);
	elm_check_state_set(ad->check_entire, EINA_TRUE);
	evas_object_smart_callback_add(ad->check_entire, "changed", _entire_pref_changed, ad);
	elm_table_pack(main_table, ad->check_entire, 1, 0, 1, 1);

	rect = evas_object_rectangle_add(evas_object_evas_get(main_table));
	evas_object_size_hint_min_set(rect, ELM_SCALE_SIZE(352), ELM_SCALE_SIZE(2));
	evas_object_color_set(rect, 0, 50, 50, 200);
	evas_object_show(rect);
	elm_table_pack(main_table, rect, 0, 1, 2, 1);

	Evas_Object *label_new = elm_label_add(panel);
	sprintf(label_str, "<color=#000000FF><align=left><font_size=30>%s</font></align></color>", NEW_TESTAMENT);
	elm_object_text_set(label_new, label_str);
	evas_object_show(label_new);
	elm_table_pack(main_table, label_new, 0, 2, 1, 1);

	ad->check_nt = elm_check_add(panel);
	elm_object_style_set(ad->check_nt, "on&off");
	evas_object_show(ad->check_nt);
	evas_object_smart_callback_add(ad->check_nt, "changed", _nt_pref_changed, ad);
	elm_table_pack(main_table, ad->check_nt, 1, 2, 1, 1);

	Evas_Object *label_old = elm_label_add(panel);
	sprintf(label_str, "<color=#000000FF><align=left><font_size=30>%s</font></align></color>", OLD_TESTAMENT);
	elm_object_text_set(label_old, label_str);
	evas_object_show(label_old);
	elm_table_pack(main_table, label_old, 0, 3, 1, 1);

	ad->check_ot = elm_check_add(panel);
	elm_object_style_set(ad->check_ot, "on&off");
	evas_object_show(ad->check_ot);
	evas_object_smart_callback_add(ad->check_ot, "changed", _ot_pref_changed, ad);
	elm_table_pack(main_table, ad->check_ot, 1, 3, 1, 1);

	rect = evas_object_rectangle_add(evas_object_evas_get(main_table));
	evas_object_size_hint_min_set(rect, ELM_SCALE_SIZE(352), ELM_SCALE_SIZE(2));
	evas_object_color_set(rect, 0, 50, 50, 200);
	evas_object_show(rect);
	elm_table_pack(main_table, rect, 0, 4, 2, 1);

	Evas_Object *label_custom = elm_label_add(panel);
	sprintf(label_str, "<color=#000000FF><align=left><font_size=30>%s</font></align></color>", CUSTOM_SEARCH);
	elm_object_text_set(label_custom, label_str);
	evas_object_show(label_custom);
	elm_table_pack(main_table, label_custom, 0, 5, 1, 1);

	ad->check_custom = elm_check_add(panel);
	elm_object_style_set(ad->check_custom, "on&off");
	evas_object_show(ad->check_custom);
	evas_object_smart_callback_add(ad->check_custom, "changed", _custom_pref_changed, ad);
	elm_table_pack(main_table, ad->check_custom, 1, 5, 1, 1);

	Evas_Object *label_from = elm_label_add(panel);
	sprintf(label_str, "<color=#000000FF><align=left><font_size=30>%s</font></align></color>", FROM);
	elm_object_text_set(label_from, label_str);
	evas_object_show(label_from);
	elm_table_pack(main_table, label_from, 0, 6, 1, 1);

	ad->from_dropdown = elm_hoversel_add(panel);
	elm_hoversel_hover_parent_set(ad->from_dropdown, panel);
	evas_object_size_hint_weight_set(ad->from_dropdown, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(ad->from_dropdown, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_text_set(ad->from_dropdown, Books[0]);
	evas_object_show(ad->from_dropdown);
	elm_object_disabled_set(ad->from_dropdown, EINA_TRUE);
	elm_table_pack(main_table, ad->from_dropdown, 1, 6, 1, 1);

	Evas_Object *label_to = elm_label_add(panel);
	sprintf(label_str, "<color=#000000FF><align=left><font_size=30>%s</font></align></color>", TO);
	elm_object_text_set(label_to, label_str);
	evas_object_show(label_to);
	elm_table_pack(main_table, label_to, 0, 7, 1, 1);

	ad->to_dropdown = elm_hoversel_add(panel);
	elm_hoversel_hover_parent_set(ad->to_dropdown, panel);
	evas_object_size_hint_weight_set(ad->to_dropdown, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(ad->to_dropdown, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_text_set(ad->to_dropdown, Books[65]);
	evas_object_show(ad->to_dropdown);
	elm_object_disabled_set(ad->to_dropdown, EINA_TRUE);
	elm_table_pack(main_table, ad->to_dropdown, 1, 7, 1, 1);

	rect = evas_object_rectangle_add(evas_object_evas_get(main_table));
	evas_object_size_hint_min_set(rect, ELM_SCALE_SIZE(352), ELM_SCALE_SIZE(2));
	evas_object_color_set(rect, 0, 50, 50, 200);
	evas_object_show(rect);
	elm_table_pack(main_table, rect, 0, 8, 2, 1);

	Evas_Object *label_whole = elm_label_add(panel);
	sprintf(label_str, "<color=#000000FF><align=left><font_size=30>%s</font></align></color>", WHOLE_WORD);
	elm_object_text_set(label_whole, label_str);
	evas_object_show(label_whole);
	elm_table_pack(main_table, label_whole, 0, 9, 1, 1);

	ad->check_whole = elm_check_add(panel);
	evas_object_show(ad->check_whole);
	elm_table_pack(main_table, ad->check_whole, 1, 9, 1, 1);

	Evas_Object *label_strict = elm_label_add(panel);
	sprintf(label_str, "<color=#000000FF><align=left><font_size=30>%s</font></align></color>", STRICT_SEARCH);
	elm_object_text_set(label_strict, label_str);
	evas_object_show(label_strict);
	elm_table_pack(main_table, label_strict, 0, 10, 1, 1);

	ad->check_strict = elm_check_add(panel);
	evas_object_show(ad->check_strict);
	elm_check_state_set(ad->check_strict, EINA_TRUE);
	elm_table_pack(main_table, ad->check_strict, 1, 10, 1, 1);

	elm_object_content_set(scroller, layout);
	elm_layout_content_set(layout, "elm.swallow.content", main_table);
	elm_object_content_set(panel, scroller);

	return panel;
}

static Eina_Bool
_panel_create(void *data)
{
	appdata_s *ad = (appdata_s*)data;

	Evas_Object *playout = create_drawer_layout(ad->naviframe);
	elm_layout_content_set(ad->search_layout, "elm.swallow.panel", playout);

	Evas_Object *panel = create_panel(ad);
	evas_object_data_set(panel, "appdata", ad);
	elm_panel_orient_set(panel, ELM_PANEL_ORIENT_LEFT);
	elm_object_part_content_set(playout, "elm.swallow.left", panel);
	evas_object_data_set(ad->search_layout, "panel", panel);

	Evas_Object *btn = elm_button_add(ad->naviframe);
	elm_object_style_set(btn, "naviframe/drawers");
	evas_object_smart_callback_add(btn, "clicked", panel_toggle, panel);
	elm_object_item_part_content_set(elm_naviframe_top_item_get(ad->naviframe), "drawers", btn);
	return ECORE_CALLBACK_DONE;
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
_content_mouse_up(void *data,
        Evas *evas EINA_UNUSED,
        Evas_Object *obj,
        void *event_info)
{
   appdata_s *ad = (appdata_s*)data;
   int x_del, y_del;
   Evas_Event_Mouse_Up *ev = (Evas_Event_Mouse_Up*)event_info;
   if ((ev->timestamp - ad->mouse_down_time) > 1000) return;
   x_del = ev->canvas.x - ad->mouse_x;
   y_del = ev->canvas.y - ad->mouse_y;
   if (abs(x_del) < (2 * abs(y_del))) return;
   if (abs(x_del) < 100) return;
   if (x_del > 0)
	   panel_toggle(ad, NULL, NULL);
}

static Eina_Bool
_search_navi_pop_cb(void *data, Elm_Object_Item *it)
{
	appdata_s *ad = (appdata_s*)data;
	Evas_Object *panel = (Evas_Object*)evas_object_data_get(ad->search_layout, "panel");
	if (!elm_panel_hidden_get(panel))
	{
		panel_toggle(panel, panel, NULL);
		return EINA_FALSE;
	}
	ecore_idler_add(_genlist_free_idler, data);
	elm_layout_signal_callback_del(ad->search_layout, "elm,holy_bible,top", "elm", _go_top);
	elm_layout_signal_callback_del(ad->search_layout, "elm,holy_bible,bottom", "elm", _go_bottom);
	_loading_progress(ad->win);
	return EINA_TRUE;
}

void
_search_word(void *data,
              Evas_Object *obj EINA_UNUSED,
              void *event_info EINA_UNUSED)
{
	appdata_s *ad = (appdata_s*)data;
	Elm_Object_Item *nf_it;
	ad->search_layout = elm_layout_add(ad->win);
	elm_layout_file_set(ad->search_layout, ad->edj_path, "search_layout");
	evas_object_size_hint_align_set(ad->search_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(ad->search_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(ad->search_layout);
	ad->search_entry = elm_entry_add(ad->search_layout);
	evas_object_size_hint_weight_set(ad->search_entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(ad->search_entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_entry_single_line_set(ad->search_entry, EINA_TRUE);
	elm_entry_scrollable_set(ad->search_entry, EINA_TRUE);
	elm_object_part_text_set(ad->search_entry, "elm.guide", ENTER_THE_KEYWORD);
	elm_entry_input_panel_return_key_type_set(ad->search_entry, ELM_INPUT_PANEL_RETURN_KEY_TYPE_SEARCH);
	evas_object_show(ad->search_entry);
	elm_object_part_content_set(ad->search_layout, "elm.swallow.entry", ad->search_entry);
	Evas_Object *go_btn = elm_button_add(ad->search_layout);
	elm_object_text_set(go_btn, GO);
	evas_object_smart_callback_add(go_btn, "clicked", _search_keyword, (void*)ad);
	evas_object_smart_callback_add(ad->search_entry, "activated", _search_keyword, (void*)ad);
	evas_object_show(go_btn);
	elm_object_part_content_set(ad->search_layout, "elm.swallow.go", go_btn);
	evas_object_event_callback_add(ad->search_layout, EVAS_CALLBACK_MOUSE_DOWN, _content_mouse_down, ad);
	evas_object_event_callback_add(ad->search_layout, EVAS_CALLBACK_MOUSE_UP, _content_mouse_up, ad);
	nf_it = elm_naviframe_item_push(ad->naviframe, SEARCH, NULL, NULL, ad->search_layout, "drawers");
	elm_naviframe_item_pop_cb_set(nf_it, _search_navi_pop_cb, ad);
	ecore_idler_add(_panel_create, ad);
}
