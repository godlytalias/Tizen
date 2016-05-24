#ifndef __widget_H__
#define __widget_H__

#include <widget_app.h>
#include <widget_app_efl.h>
#include <Elementary.h>
#include <dlog.h>

#define DB_NAME "holybible.db"
#define BIBLE_TABLE_NAME "bible"
#define BIBLE_VERSE_COLUMN "verse"

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

typedef struct widget_instance_data widget_instance_data_s;

typedef struct _bible_verse_item {
	char *verse;
	int bookcount, chaptercount, versecount;
	widget_instance_data_s *wid;
	struct _bible_verse_item *next;
} bible_verse_item_w;

struct widget_instance_data {
	Evas_Object *win;
	Evas_Object *conform;
	Evas_Object *entry;
	Evas_Object *layout;
	Evas_Object *scroller;
	bible_verse_item_w *verse_item_head, *verse_item_tail;
	char edj_path[PATH_MAX];
	int cur_book, cur_chapter, cur_verse;
};

void _query_verse(void *data, int book, int chapter, int verse);
