#include <sqlite3.h>
#include <tizen.h>
#include "widget.h"

void
_database_query(char *query, int func(void*,int,char**,char**), void *data)
{
	sqlite3 *db = NULL;
	char *err_msg;

	char *db_path = malloc(200);
	char *res_path = app_get_shared_resource_path();
	sprintf(db_path, "%s%s", res_path, DB_NAME);
	sqlite3_open(db_path, &(db));
	free(res_path);
	free(db_path);
	sqlite3_exec(db, query, func, data, &err_msg);
	sqlite3_free(err_msg);
	sqlite3_close(db);
	db = NULL;
}

static int
_get_verse(void *data, int argc, char **argv, char **azColName)
{
	widget_instance_data_s *wid = (widget_instance_data_s*) data;
	if (argv == NULL) return 0;

	bible_verse_item *verse_item = malloc(sizeof(bible_verse_item));
	verse_item->verse = strdup(argv[0]);
	verse_item->bookcount = wid->cur_book;
	verse_item->chaptercount = wid->cur_chapter;
	verse_item->versecount = wid->cur_verse;
	verse_item->wid = wid;
	if (!wid->verse_item_head)
	{
		wid->verse_item_head = wid->verse_item_tail = verse_item;
		verse_item->next = NULL;
	}
	else
	{
		wid->verse_item_tail->next = verse_item;
		wid->verse_item_tail = verse_item;
		verse_item->next = wid->verse_item_head;
	}

	return 0;
}

void
_query_verse(void *data, int book, int chapter, int verse)
{
	widget_instance_data_s *wid = (widget_instance_data_s*)data;
	char query[512];

	wid->cur_book = book;
	wid->cur_chapter = chapter;
	wid->cur_verse = verse;

	sprintf(query, "select %s from %s where Book=%d and Chapter=%d and Versecount=%d", BIBLE_VERSE_COLUMN, BIBLE_TABLE_NAME, book, chapter, verse);
	_database_query(query, &_get_verse, data);
}
