#ifndef __widget_H__
#define __widget_H__

#include <widget_app.h>
#include <widget_app_efl.h>
#include <Elementary.h>
#include <app_preference.h>
#include <efl_extension.h>
#include <dlog.h>

#define DB_NAME "appdata.db"
#define WIDGET_TABLE_NAME "versewidget"
#define WIDGET_VERSE_COLUMN "verse"

#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "godly"

#if !defined(PACKAGE)
#define PACKAGE "org.widget.gtaholybible"
#endif

#define EDJ_FILE "edje/widget.edj"
#define GRP_MAIN "main"
#define DEFAULT_WIDGET_VERSE "And there is salvation in no other One; for there is no other name under Heaven given among men by which we must be saved." //Acts 4:12
#define RED "Red"
#define GREEN "Green"
#define BLUE "Blue"
#define ALPHA "Alpha"
#define CLOSE "Close"
#define DISPLAYED_VERSE "Displayed Verse"
#define FONT_SIZE "Font size"
#define FONT_COLOR "Font color"
#define BG_COLOR "Background Color"
#define YOUR_COLOR_HERE "YOUR COLOR HERE"
#define BOLD "Bold"
#define REGULAR "Regular"
#define LIGHT "Light"

#endif /* __widget_H__ */


typedef struct widget_instance_data widget_instance_data_s;
typedef struct _widget_bible_verse_item widget_bible_verse_item;

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


static char *Font_Style[] = {
		"Bold", "Regular", "Light", NULL
};

struct widget_instance_data {
	Evas_Object *win;
	Evas_Object *conform;
	Evas_Object *entry;
	Evas_Object *layout;
	Evas_Object *scroller;
	Evas_Object *settings_window, *settings_nf;
	char edj_path[PATH_MAX];
	char *verse, *font_style;
	int cur_book, cur_chapter, cur_verse, verse_order, font_size;
	int text_r, text_g, text_b, text_a;
	int bg_r, bg_g, bg_b, bg_a;
};

struct _widget_bible_verse_item {
	char *verse;
	widget_instance_data_s *wid;
	int bookcount, chaptercount, versecount;
};

void _query_verse(void *data);
void _widget_settings(widget_instance_data_s *wid);
void _database_query(char *query, int func(void*,int,char**,char**), void *data);
