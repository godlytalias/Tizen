#include <tizen.h>
#include <sqlite3.h>
#include "bible.h"

static void
_clear_item_data(void *data)
{
	Evas_Object *list = (Evas_Object*)data;
	Elm_Object_Item *item;
	bible_verse_item *item_data;
	item = elm_list_first_item_get(list);
	while(item)
	{
		item_data = (bible_verse_item*)elm_object_item_data_get(item);
		if (item_data) free(item_data);
		item = elm_list_item_next(item);
	}
}

static Eina_Bool
naviframe_pop_cb(void *data, Elm_Object_Item *it)
{
	appdata_s *ad = (appdata_s*)data;
	_get_chapter_count_query(data, ad->cur_book);
    _clear_item_data(ad->list1);
	return EINA_TRUE;
}

static void
_back_btn_clicked(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = (appdata_s*)data;
	_loading_progress(ad->win);
	elm_naviframe_item_pop(ad->naviframe);
}

static void
_done_btn_clicked(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = (appdata_s*)data;
	_loading_progress(ad->win);
	_query_chapter(ad, ad->nxt_book, ad->nxt_chapter);
	elm_naviframe_item_pop(ad->naviframe);
}

static void
_chapter_selected(void *data, Evas_Object *obj, void *event_info)
{
	bible_verse_item *item_v = (bible_verse_item*)data;
	appdata_s *ad = item_v->appdata;
	Elm_Object_Item *item = elm_list_selected_item_get(ad->list2);
	ad->nxt_chapter = atoi(elm_object_item_text_get(item));
	ad->nxt_book = item_v->bookcount;
}

static void
_book_selected(void *data, Evas_Object *obj, void *event_info)
{
	int book;
	int i;
	char chapter[4];
	bible_verse_item *item_v;
	item_v = (bible_verse_item*)data;
	Elm_Object_Item *item;
	appdata_s *ad = item_v->appdata;
	book = item_v->bookcount;

	if (book == ad->nxt_book) return;

	_get_chapter_count_query(ad, book);
	elm_list_clear(ad->list2);
	for(i = 0; i < ad->chaptercount; i++) {
		sprintf(chapter, "%d", i+1);
		item = elm_list_item_append(ad->list2, chapter, NULL, NULL, _chapter_selected, data);
	}
	if (ad->nxt_book == -1) {
		int count = 0;
		item = elm_list_first_item_get(ad->list2);
		while(item && (count < (ad->cur_chapter - 1)))
		{
			item = elm_list_item_next(item);
			count ++;
		}
		if (item) elm_list_item_selected_set(item, EINA_TRUE);
	}
	else
	  elm_list_item_selected_set(elm_list_first_item_get(ad->list2), EINA_TRUE);
	elm_list_go(ad->list2);
	evas_object_show(ad->list2);
	item = elm_list_selected_item_get(ad->list2);
	elm_list_item_show(item);
}

static Evas_Object*
_select_chapter_layout(appdata_s *ad)
{
	Evas_Object *layout = elm_layout_add(ad->win);
	Elm_Object_Item *item;
	int i;
	ad->nxt_book = -1;
	ad->nxt_chapter = -1;
	elm_layout_file_set(layout, ad->edj_path, "select_chapter_layout");
	ad->list1 = elm_list_add(layout);
	elm_list_select_mode_set(ad->list1, ELM_OBJECT_SELECT_MODE_ALWAYS);
	ad->list2 = elm_list_add(layout);
	elm_list_select_mode_set(ad->list2, ELM_OBJECT_SELECT_MODE_ALWAYS);
	for(i = 0; i < 66; i++) {
		bible_verse_item *item_v = malloc(sizeof(bible_verse_item));
		item_v->appdata = ad;
		item_v->bookcount = i;
		item_v->chaptercount = 1;
		elm_list_item_append(ad->list1, Books[i], NULL, NULL, _book_selected, (void*)item_v);
	}
	item = elm_list_first_item_get(ad->list1);
	for(i = 0; i < ad->cur_book; i++)
		item = elm_list_item_next(item);
	elm_list_item_selected_set(item, EINA_TRUE);
	elm_list_go(ad->list1);
	evas_object_show(ad->list1);
	elm_list_item_show(item);
	elm_object_part_content_set(layout, "elm.swallow.content1", ad->list1);
	elm_object_part_content_set(layout, "elm.swallow.content2", ad->list2);
	evas_object_show(layout);
	return layout;
}

void
_change_book(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	appdata_s *ad = (appdata_s*)data;
	Elm_Object_Item *nf_it;

	Evas_Object *back_btn = elm_button_add(obj);
	elm_object_style_set(back_btn, "naviframe/title_cancel");
	elm_object_text_set(back_btn, "Cancel");
	evas_object_show(back_btn);
	Evas_Object *done_btn = elm_button_add(obj);
	elm_object_style_set(done_btn, "naviframe/title_done");
	elm_object_text_set(done_btn, "Done");
	evas_object_show(done_btn);
	nf_it = elm_naviframe_item_push(ad->naviframe, "അദ്ധ്യായങ്ങൾ", NULL, NULL, _select_chapter_layout(ad), NULL);
	elm_naviframe_item_pop_cb_set(nf_it, naviframe_pop_cb, ad);
	elm_object_item_part_content_set(nf_it, "title_left_btn", back_btn);
	elm_object_item_part_content_set(nf_it, "title_right_btn", done_btn);
	evas_object_smart_callback_add(back_btn, "clicked", _back_btn_clicked, ad);
	evas_object_smart_callback_add(done_btn, "clicked", _done_btn_clicked, ad);
}
