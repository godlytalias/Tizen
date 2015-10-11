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
	Evas_Object* layout, *search_layout;
	Evas_Object* book_btn, *chapter_btn;
	Evas_Object* label, *naviframe;
	Evas_Object* genlist, *search_result_genlist;
	Evas_Object *list1, *list2, *search_entry;
	Elm_Genlist_Item_Class *itc, *search_itc;
	int count, versecount, chaptercount;
	int cur_chapter, cur_book;
	int nxt_chapter, nxt_book;
	Evas_Coord mouse_down, mouse_up;
	sqlite3 *db;
} appdata_s;


const static char *Books[] = {
						"Genesis", "Exodus", "Leviticus", "Numbers", "Deuteronomy", "Joshua",
						"Judges", "Ruth", "1 Samuel", "2 Samuel", "1 Kings", "2 Kings", "1 Chronicles",
						"2 Chronicles", "Ezra", "Nehemiah", "Esther", "Job", "Psalms", "Proverbs", "Ecclesiastes",
						"Song of Solomon", "Isaiah", "Jeremiah", "Lamentations", "Ezekiel", "Daniel",
						"Hosea", "Joel", "Amos", "Obadiah", "Jonah", "Micah", "Nahum", "Habakkuk", "Zephaniah",
						"Haggai", "Zechariah", "Malachi", "Matthew", "Mark", "Luke", "John", "Acts", "Romans",
						"1 Corinthians", "2 Corinthians", "Galatians", "Ephesians", "Philippians", "Colossians",
						"1 Thessalonians", "2 Thessalonians", "1 Timothy", "2 Timothy", "Titus", "Philemon",
						"Hebrews", "James", "1 Peter", "2 Peter", "1 John", "2 John", "3 John", "Jude", "Revelation"};


struct _bible_verse_item
{
   appdata_s *appdata;
   int bookcount, chaptercount, versecount;
   char *verse;
};

void app_get_resource(const char *, char *, int);
void _query_chapter(void*, int, int);
void _get_chapter_count_query(void*, int);
void _get_verse_count_query(void*, int,int);
void _database_query(char*, int func(void*,int,char**,char**), void*);
void _change_book(void *, Evas_Object*, char*, char*);
void _search_word(void *, Evas_Object*,void*);
void create_ctxpopup_more_button_cb(void*, Evas_Object*, void*);
int _get_bookcount(char*);