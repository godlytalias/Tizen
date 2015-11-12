#include <tizen.h>
#include <sqlite3.h>
#include "bible.h"

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

static Eina_Bool
_search_navi_pop_cb(void *data, Elm_Object_Item *it)
{
	ecore_idler_add(_genlist_free_idler, data);
	appdata_s *ad = (appdata_s*)data;
	elm_layout_signal_callback_del(ad->search_layout, "elm,holy_bible,top", "elm", _go_top);
	elm_layout_signal_callback_del(ad->search_layout, "elm,holy_bible,bottom", "elm", _go_bottom);
	_loading_progress(ad->win);
	return EINA_TRUE;
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
	Elm_Object_Item *item = (Elm_Object_Item*)event_info;
	Evas_Object *verse_popup = elm_popup_add(verse_item->appdata->search_result_genlist);
	elm_popup_align_set(verse_popup, ELM_NOTIFY_ALIGN_FILL, 1.0);
	sprintf(title, "%s %d : %d", Books[verse_item->bookcount], verse_item->chaptercount, verse_item->versecount + 1);
	elm_object_part_text_set(verse_popup, "title,text", title);
	Evas_Object *verse_entry = elm_entry_add(obj);
	elm_entry_editable_set(verse_entry, EINA_FALSE);
	evas_object_size_hint_weight_set(verse_entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_entry_entry_set(verse_entry, verse_item->verse);
	elm_entry_text_style_user_push(verse_entry, "DEFAULT='font=Tizen:style=Regular align=left font_size=30 color=#000000 wrap=mixed'hilight=' + font_weight=Bold'");
	evas_object_smart_callback_add(verse_entry,"selection,copy",_copy_verse,(void*)verse_item);
	elm_object_content_set(verse_popup, verse_entry);
	Evas_Object *ok = elm_button_add(verse_popup);
	elm_object_text_set(ok, "बंद");
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
    sprintf(popup_text, "<align='center'>%s %d : %d से जाओ ?</align>", Books[verse_item->bookcount], verse_item->chaptercount, verse_item->versecount + 1);
    elm_object_text_set(popup, popup_text);
    Evas_Object *button1 = elm_button_add(ad->win);
    elm_object_text_set(button1, "नहीं");
    evas_object_smart_callback_add(button1, "clicked", _popup_del, popup);
    Evas_Object *button2 = elm_button_add(ad->win);
    elm_object_text_set(button2, "ठीक");
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
	   int i;
	   bible_verse_item *verse_item = malloc(sizeof(bible_verse_item));
	   for (i = 0; i < argc; i++)
	   {
		   if (!strcmp(azColName[i], "Book"))
			   verse_item->bookcount = _get_bookcount(argv[i]);
		   if (!strcmp(azColName[i], "Chapter"))
			   verse_item->chaptercount = atoi(argv[i]);
		   if (!strcmp(azColName[i], "Versecount")) {
			   verse_item->versecount = atoi(argv[i]);
			   verse_item->versecount--;
		   }
		   if (!strcmp(azColName[i], "e_verse"))
			   verse_item->verse = strdup(argv[i]);
	   }
	   verse_item->appdata = ad;
	   elm_genlist_item_append(ad->search_result_genlist, ad->search_itc, (void*)verse_item, NULL, ELM_GENLIST_ITEM_NONE, _search_result_selected, (void*)verse_item);
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
	sprintf(toast, "%d रिसलृट मिला", res_count);
	Evas_Object *toastp = elm_popup_add(ad->win);
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
	char keyword_query[2048];
	char search_query[2048];
	if (keyword && (strlen(keyword) > 1024))
	{
		Evas_Object *toast_popup = elm_popup_add(ad->win);
		elm_popup_timeout_set(toast_popup, 2);
		elm_object_style_set(toast_popup, "toast");
		elm_object_text_set(toast_popup, "कीवर्ड बहुत बड़ा है");
		evas_object_smart_callback_add(toast_popup, "timeout", _dismiss_verse_popup, toast_popup);
		evas_object_show(toast_popup);
		return;
	}
	else if (keyword && (strlen(keyword) < 2))
		{
			Evas_Object *toast_popup = elm_popup_add(ad->win);
			elm_popup_timeout_set(toast_popup, 2);
			elm_object_style_set(toast_popup, "toast");
			elm_object_text_set(toast_popup, "कीवर्ड बहुत छोटा है");
			evas_object_smart_callback_add(toast_popup, "timeout", _dismiss_verse_popup, toast_popup);
			evas_object_show(toast_popup);
			return;
		}

	_loading_progress(ad->win);
	if (keyword) {
		ch = strtok(keyword, " ");
		sprintf(keyword_query, "e_verse LIKE '%%%s%%'", ch);
	}
	ch = strtok(NULL, " ");
	while (ch)
	{
		sprintf(keyword_query, "%s AND e_verse LIKE '%%%s%%'", keyword_query, ch);
		ch = strtok(NULL, " ");
	}
	sprintf(search_query, "SELECT Book, Chapter, Versecount, e_verse FROM eng_bible WHERE %s;", keyword_query);
	_bible_search_query(search_query, ad);
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
	elm_object_part_text_set(ad->search_entry, "elm.guide", "कीवर्ड दर्ज करें");
	elm_entry_input_panel_return_key_type_set(ad->search_entry, ELM_INPUT_PANEL_RETURN_KEY_TYPE_SEARCH);
	evas_object_show(ad->search_entry);
	elm_object_part_content_set(ad->search_layout, "elm.swallow.entry", ad->search_entry);
	Evas_Object *go_btn = elm_button_add(ad->search_layout);
	elm_object_text_set(go_btn, "ठीक");
	evas_object_smart_callback_add(go_btn, "clicked", _search_keyword, (void*)ad);
	evas_object_smart_callback_add(ad->search_entry, "activated", _search_keyword, (void*)ad);
	evas_object_show(go_btn);
	elm_object_part_content_set(ad->search_layout, "elm.swallow.go", go_btn);
	nf_it = elm_naviframe_item_push(ad->naviframe, "खोज", NULL, NULL, ad->search_layout, NULL);
	elm_naviframe_item_pop_cb_set(nf_it, _search_navi_pop_cb, ad);
}
