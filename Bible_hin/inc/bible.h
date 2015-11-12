#ifndef __bible_H__
#define __bible_H__

#include <app.h>
#include <Elementary.h>
#include <system_settings.h>
#include <efl_extension.h>
#include <dlog.h>

#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "bible"

#if !defined(PACKAGE)
#define PACKAGE "org.tizen.bible"
#endif

#define EDJ_FILE "edje/bible.edj"
#define GRP_MAIN "main"


#endif /* __bible_H__ */

typedef struct _bible_verse_item bible_verse_item;

typedef struct appdata{
	Evas_Object* win;
	Evas_Object* layout, *search_layout, *bookmark_note_layout;
	Evas_Object* label, *naviframe;
	Evas_Object* genlist, *search_result_genlist, *bookmarks_notes_genlist;
	Evas_Object *list1, *list2, *search_entry, *note_entry;
	Elm_Genlist_Item_Class *itc, *search_itc, *bookmarks_itc;
	Evas_Coord mouse_x, mouse_y;
	uint mouse_down_time;
	int count, versecount, chaptercount;
	int cur_chapter, cur_book;
	int nxt_chapter, nxt_book;
	char edj_path[PATH_MAX];
} appdata_s;


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
   int bookcount, chaptercount, versecount;
   Eina_Bool bookmark, note;
   Elm_Object_Item *it;
   char *verse;
};

void _query_chapter(void*, int, int);
void _get_chapter_count_query(void*, int);
void _get_verse_count_query(void*, int,int);
void _app_database_query(char*, int func(void*,int,char**,char**), void*);
void _database_query(char*, int func(void*,int,char**,char**), void*);
void _change_book(void *, Evas_Object*, const char*, const char*);
void _search_word(void *, Evas_Object*,void*);
void create_ctxpopup_more_button_cb(void*, Evas_Object*, void*);
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