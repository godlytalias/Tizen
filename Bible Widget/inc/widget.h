#ifndef __widget_H__
#define __widget_H__

#include <widget_app.h>
#include <widget_app_efl.h>
#include <Elementary.h>
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

#endif /* __widget_H__ */

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

typedef struct widget_instance_data widget_instance_data_s;

struct widget_instance_data {
	Evas_Object *win;
	Evas_Object *conform;
	Evas_Object *entry;
	Evas_Object *layout;
	Evas_Object *scroller;
	char edj_path[PATH_MAX];
	char *verse;
	int cur_book, cur_chapter, cur_verse, verse_order;
};

void _query_verse(void *data);
