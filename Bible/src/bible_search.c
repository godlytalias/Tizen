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

static Eina_Bool
_search_navi_pop_cb(void *data, Elm_Object_Item *it)
{
	ecore_idler_add(_genlist_free_idler, data);
	return EINA_TRUE;
}

Evas_Object*
search_gl_content_get_cb(void *data, Evas_Object *obj, const char *part)
{
    bible_verse_item *verse_item = (bible_verse_item*)data;
    if(strcmp(part, "elm.swallow.content") == 0)
    {
    	Evas_Object *layout = elm_layout_add(obj);
    	evas_object_size_hint_min_set(layout, 480, 158);
    	char edj_path[PATH_MAX] = {0, };
    	app_get_resource(EDJ_FILE, edj_path, (int)PATH_MAX);
    	elm_layout_file_set(layout, edj_path, "search_verse_layout");
    	sprintf(edj_path, "%s %d : %d", Books[verse_item->bookcount], verse_item->chaptercount, verse_item->versecount+1);
    	elm_object_part_text_set(layout, "elm.text.reference", edj_path);
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
}

static void
_search_result_selected(void *data, Evas_Object *obj, void *event_info)
{
	bible_verse_item *verse_item = (bible_verse_item*)data;
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
		   if (!strcmp(azColName[i], "Versecount"))
			   verse_item->versecount = atoi(argv[i]);
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
	ad->search_result_genlist = elm_genlist_add(ad->naviframe);
	evas_object_size_hint_weight_set(ad->search_result_genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(ad->search_result_genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_genlist_select_mode_set(ad->search_result_genlist, ELM_OBJECT_SELECT_MODE_ALWAYS);
	ad->search_itc = elm_genlist_item_class_new();
	ad->search_itc->item_style = "full";
	ad->search_itc->func.content_get = search_gl_content_get_cb;
	ad->search_itc->func.text_get = NULL;
	ad->search_itc->func.del = search_gl_del_cb;
	_database_query(search_query, _get_search_results, ad);
	elm_object_part_content_set(ad->search_layout, "elm.swallow.result", ad->search_result_genlist);
	evas_object_show(ad->search_result_genlist);
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
		elm_object_text_set(toast_popup, "Search keyword is too large");
		evas_object_show(toast_popup);
		return;
	}
	else if (keyword && (strlen(keyword) < 2))
		{
			Evas_Object *toast_popup = elm_popup_add(ad->win);
			elm_popup_timeout_set(toast_popup, 2);
			elm_object_style_set(toast_popup, "toast");
			elm_object_text_set(toast_popup, "Search keyword is too small");
			evas_object_show(toast_popup);
			return;
		}
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
              Evas_Object *obj ,
              void *event_info EINA_UNUSED)
{
	appdata_s *ad = (appdata_s*)data;
	Elm_Object_Item *nf_it;
	char edj_path[PATH_MAX] = {0, };
	app_get_resource(EDJ_FILE, edj_path, (int)PATH_MAX);
	ad->search_layout = elm_layout_add(ad->win);
	elm_layout_file_set(ad->search_layout, edj_path, "search_layout");
	evas_object_size_hint_align_set(ad->search_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(ad->search_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(ad->search_layout);
	ad->search_entry = elm_entry_add(ad->search_layout);
	elm_entry_single_line_set(ad->search_entry, EINA_TRUE);
	elm_entry_scrollable_set(ad->search_entry, EINA_TRUE);
	elm_object_part_text_set(ad->search_entry, "elm.guide", "Enter the keyword");
	evas_object_show(ad->search_entry);
	elm_object_part_content_set(ad->search_layout, "elm.swallow.entry", ad->search_entry);
	Evas_Object *go_btn = elm_button_add(ad->search_layout);
	elm_object_text_set(go_btn, "Go");
	evas_object_smart_callback_add(go_btn, "clicked", _search_keyword, (void*)ad);
	evas_object_show(go_btn);
	elm_object_part_content_set(ad->search_layout, "elm.swallow.go", go_btn);
	nf_it = elm_naviframe_item_push(ad->naviframe, "Search", NULL, NULL, ad->search_layout, NULL);
	elm_naviframe_item_pop_cb_set(nf_it, _search_navi_pop_cb, ad);
}
