#include <sqlite3.h>
#include <tizen.h>
#include "widget.h"

void
_database_query(char *query, int func(void*,int,char**,char**), void *data)
{
	sqlite3 *db = NULL;
	char *err_msg = NULL;

	char *db_path = malloc(200);
	char *res_path = app_get_data_path();
	sprintf(db_path, "%s%s", res_path, DB_NAME);
	sqlite3_open_v2(db_path, &db, SQLITE_OPEN_FULLMUTEX | SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	free(res_path);
	free(db_path);
	sqlite3_exec(db, query, func, data, &err_msg);
	if (err_msg) sqlite3_free(err_msg);
	sqlite3_close(db);
	db = NULL;
}

static int
_get_verse(void *data, int argc, char **argv, char **azColName)
{
	widget_instance_data_s *wid = (widget_instance_data_s*) data;
	if (argv == NULL) return 0;

	wid->verse = strdup(argv[3]);
	wid->cur_book = atoi(argv[0]);
	wid->cur_chapter = atoi(argv[1]);
	wid->cur_verse = atoi(argv[2]) + 1;
	return 0;
}

void
_query_verse(void *data)
{
	widget_instance_data_s *wid = (widget_instance_data_s*)data;
	char query[512];
	char verse[1024];

	if (wid->verse) free(wid->verse);
	wid->verse = NULL;
	verse_query:
	sprintf(query, "select * from %s where ROWID=%d;", WIDGET_TABLE_NAME, wid->verse_order);
	_database_query(query, &_get_verse, data);
	if (!wid->verse && wid->verse_order > 1)
	{
		wid->verse_order = 1;
		goto verse_query;
	}

	sprintf(verse, "%s<br><align=right>%s %d:%d</align>", wid->verse, Books[wid->cur_book], wid->cur_chapter, wid->cur_verse);
	elm_layout_text_set(wid->layout, "elm.text.verse", verse);
}
