#ifndef __bible_H__
#define __bible_H__

#include <app.h>
#include <app_preference.h>
#include <Elementary.h>
#include <system_settings.h>
#include <efl_extension.h>
#include <dlog.h>
#include <package_info.h>
#include <package_manager.h>
#include "bible_strings.h"

#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "bible"

#if !defined(PACKAGE)
#define PACKAGE "org.tizen.bible"
#endif

#define EDJ_FILE "edje/bible.edj"
#define GRP_MAIN "main"
#define DB_NAME "holybible.db"
#define BIBLE_TABLE_NAME "bible"
#define BIBLE_VERSE_COLUMN "verse"
#define LONGPRESS_TIMEOUT 1.0

#endif /* __bible_H__ */

typedef struct _bible_verse_item bible_verse_item;
typedef struct app_struct_list app_struct;

typedef struct appdata{
	struct app_struct_list *app_list_head, *app_list_tail;
	Evas_Object* win;
	Evas_Object* layout, *search_layout, *bookmark_note_layout;
	Evas_Object* label, *naviframe, *select_all_check;
	Evas_Object* genlist, *old_genlist, *search_result_genlist, *bookmarks_notes_genlist;
	Evas_Object *list1, *list2, *search_entry, *note_entry;
	Evas_Object *check_entire, *check_ot, *check_nt, *check_custom, *check_strict, *check_whole;
	Evas_Object *from_dropdown, *to_dropdown;
	Evas_Object *menu_ctxpopup;
	Elm_Genlist_Item_Class *itc, *search_itc, *bookmarks_itc;
	Ecore_Timer *long_timer;
	Elm_Transit *transit;
	Elm_Object_Item *readmode_item;
	char *parallel_db_path;
	int search_from, search_to;
	int count, versecount, chaptercount;
	int cur_chapter, cur_book, cur_verse;
	int nxt_chapter, nxt_book;
	Evas_Coord mouse_x, mouse_y;
	uint mouse_down_time;
	char edj_path[PATH_MAX];
	int long_press_mode:1; //0 - up / prev, 1 - down / next
	Eina_Bool share_copy_mode:1;
	Eina_Bool exit_mode:1;
	Eina_Bool long_pressed:1;
	Eina_Bool panel_mode:1;
	Eina_Bool rotate_flag:1;
	Eina_Bool app_control_mode:1;
} appdata_s;

#define PARALLEL_READING_SUPPORT_VERSION 0.5

typedef struct app_struct_list {
	char *app_name;
	char *app_id;
	struct app_struct_list *app_next;
}app_struct;

const static char *Books[] = {
		"उत्पत्ति","निर्गमन","लैव्यवस्था","गिनती","व्यवस्थाविवरण","यहोशू ",
		"न्यायियों","रूत","1 शमूएल","2 शमूएल","1 राजा","2 राजा",
		"1 इतिहास","2 इतिहास","एज्रा","नहेमायाह","एस्तेर","अय्यूब",
		"भजन संहिता","नीतिवचन ","सभोपदेशक","श्रेष्ठगीत","यशायाह","यिर्मयाह",
		"विलापगीत","यहेजकेल","दानिय्येल","होशे","योएल","आमोस","ओबद्दाह",
		"योना","मीका","नहूम","हबक्कूक","सपन्याह","हाग्गै","जकर्याह",
		"मलाकी","मत्ती","मरकुस","लूका","यूहन्ना","प्रेरितों के काम","रोमियो",
		"1 कुरिन्थियों","2 कुरिन्थियों","गलातियों","इफिसियों","फिलिप्पियों","कुलुस्सियों",
		"1 थिस्सलुनीकियों","2 थिस्सलुनीकियों","1 तीमुथियुस","2 तीमुथियुस","तीतुस",
		"फिलेमोन","इब्रानियों","याकूब","1 पतरस","2 पतरस","1 यूहन्ना","2 यूहन्ना",
		"3 यूहन्ना","यहूदा","प्रकाशित वाक्य"
		};


struct _bible_verse_item
{
   appdata_s *appdata;
   Elm_Object_Item *it;
   char *verse, *verse_s;
   int bookcount, chaptercount, versecount;
   Eina_Bool bookmark : 1;
   Eina_Bool note : 1;
};

void _query_chapter(void*, int, int);
void _get_chapter_count_query(void*, int);
void _get_verse_count_query(void*, int,int);
void _app_database_query(char*, int func(void*,int,char**,char**), void*);
void _database_query(char*, int func(void*,int,char**,char**), void*);
void _change_book(void *, Evas_Object*, const char*, const char*);
void _search_word(void *, Evas_Object*,void*);
void _clear_table(char *table);
Eina_Bool _keyword_check(const char *keyword, appdata_s *ad);
void create_ctxpopup_more_menu(void*);
void show_ctxpopup_more_button_cb(void*, Evas_Object*, void*);
void hide_ctxpopup_more_button_cb(void*, Evas_Object*, void*);
int _get_bookcount(char*);
void _loading_progress(Evas_Object *);
Evas_Object* _loading_progress_show(Evas_Object *);
void _loading_progress_hide(Evas_Object*);
void _load_appdata(appdata_s *);
void _save_appdata(appdata_s *);
void move_more_ctxpopup(void*, Evas_Object*, void*);
void gl_del_cb(void*, Evas_Object*);
void _check_bookmarks(appdata_s *);
void _check_notes(appdata_s *);
void _get_chapter(void *, Evas_Object *, void *);
void _popup_del(void *, Evas_Object *, void *);
void _show_verse(void *, int);
void note_remove_cb(void *, Evas_Object *, void *);
void _remove_bookmark_query(void *data, Evas_Object *obj, void *event_info);
void _bookmark_verse_cb(void *data, Evas_Object *obj, void *event_info);
void _add_note_cb(void *data, Evas_Object *obj, void *event_info);
void _share_verse_cb(appdata_s *);
void _copy_verse_cb(appdata_s *);
void _copy_verse_done_cb(void *data, Evas_Object *obj, void *event_info);
void _share_verse_done_cb(void *data, Evas_Object *obj, void *event_info);
void _cancel_cb(void *, Evas_Object *, void *);
void _app_no_memory(appdata_s *);
void _change_read_mode(appdata_s *, Eina_Bool);
void _search_genlist_free(appdata_s*);
void _bible_verse_show(void *data, Evas_Object *obj, void *event_info);
void _verse_display_widget_list(appdata_s *);
void _verse_item_widget_cb(void *data, Evas_Object *obj, void *event_info);
void _insert_widget_verse(int bookcount, int chaptercount, int versecount, char *verse);
void _delete_widget_verse(int bookcount, int chaptercount, int versecount);
