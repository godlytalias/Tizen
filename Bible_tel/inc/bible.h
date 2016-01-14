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

#endif /* __bible_H__ */

typedef struct _bible_verse_item bible_verse_item;
typedef struct app_struct_list app_struct;

typedef struct appdata{
	Evas_Object* win;
	Evas_Object* layout, *search_layout, *bookmark_note_layout;
	Evas_Object* label, *naviframe;
	Evas_Object* genlist, *search_result_genlist, *bookmarks_notes_genlist;
	Evas_Object *list1, *list2, *search_entry, *note_entry;
	Evas_Object *check_entire, *check_ot, *check_nt, *check_custom, *check_strict, *check_whole;
	Evas_Object *from_dropdown, *to_dropdown;
	Evas_Object *menu_ctxpopup;
	Elm_Genlist_Item_Class *itc, *search_itc, *bookmarks_itc;
	Evas_Coord mouse_x, mouse_y;
	Eina_Bool share_copy_mode:1;
	Elm_Object_Item *readmode_item;
	uint mouse_down_time;
	int search_from, search_to;
	int count, versecount, chaptercount;
	int cur_chapter, cur_book;
	int nxt_chapter, nxt_book;
	char edj_path[PATH_MAX];
	char *parallel_db_path;
	struct app_struct_list *app_list_head, *app_list_tail;
} appdata_s;

#define PARALLEL_READING_SUPPORT_VERSION 0.5

typedef struct app_struct_list {
	char *app_name;
	char *app_id;
	struct app_struct_list *app_next;
}app_struct;

const static char *Books[] = {
		"ఆదికాండము", "నిర్గమకాండము", "లేవీయకాండము", "సంఖ్యాకాండము", "ద్వితీయోపదేశకాండమ", "యెహొషువ",
		"న్యాయాధిపతులు", "రూతు", "సమూయేలు మొదటి గ్రంథము", "సమూయేలు రెండవ గ్రంథము", "రాజులు మొదటి గ్రంథము",
		"రాజులు రెండవ గ్రంథము", "దినవృత్తాంతములు మొదటి గ్రంథము", "దినవృత్తాంతములు రెండవ గ్రంథము", "ఎజ్రా", "నెహెమ్యా", "ఎస్తేరు", "యోబు గ్రంథము",
		"కీర్తనల గ్రంథము", "సామెతలు", "ప్రసంగి", "పరమగీతము", "యెషయా గ్రంథము", "యిర్మీయా", "విలాపవాక్యములు", "యెహెజ్కేలు",
		"దానియేలు", "హొషేయ", "యోవేలు", "ఆమోసు", "ఓబద్యా", "యోనా", "మీకా", "నహూము", "హబక్కూకు", "జెఫన్యా",
		"హగ్గయి", "జెకర్యా", "మలాకీ", "మత్తయి సువార్త", "మార్కు సువార్త", "లూకా సువార్త", "యోహాను సువార్త", "అపొస్తలుల కార్యములు",
		"రోమీయులకు", "1 కొరింథీయులకు", "2 కొరింథీయులకు", "గలతీయులకు", "ఎఫెసీయులకు", "ఫిలిప్పీయులకు",
		"కొలొస్సయులకు", "1 థెస్సలొనీకయులకు", "2 థెస్సలొనీకయులకు", "1 తిమోతికి", "2 తిమోతికి", "తీతుకు", "ఫిలేమోనుకు",
		"హెబ్రీయులకు", "యాకోబు", "1 పేతురు", "2 పేతురు", "1 యోహాను", "2 యోహాను", "3 యోహాను", "యూదా", "ప్రకటన గ్రంథము"};


struct _bible_verse_item
{
   appdata_s *appdata;
   int bookcount, chaptercount, versecount;
   Eina_Bool bookmark : 1;
   Eina_Bool note : 1;
   Elm_Object_Item *it;
   char *verse, *verse_s;
};

void _query_chapter(void*, int, int);
void _get_chapter_count_query(void*, int);
void _get_verse_count_query(void*, int,int);
void _app_database_query(char*, int func(void*,int,char**,char**), void*);
void _database_query(char*, int func(void*,int,char**,char**), void*);
void _change_book(void *, Evas_Object*, const char*, const char*);
void _search_word(void *, Evas_Object*,void*);
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
void _share_verse_cb(appdata_s *);
void _copy_verse_cb(appdata_s *);
void _cancel_cb(void *, Evas_Object *, void *);
void _app_no_memory(appdata_s *);
void _change_read_mode(appdata_s *, Eina_Bool);
