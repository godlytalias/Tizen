#include <tizen.h>
#include <sqlite3.h>
#include "bible.h"

void
_app_database_query(char *query, int func(void*,int,char**,char**), void *data)
{
	sqlite3 *db = NULL;
	char *err_msg;

	   char *db_path = malloc(200);
	   char *res_path = app_get_data_path();
	   sprintf(db_path, "%sappdata.db", res_path);
	   sqlite3_open_v2(db_path, &db, SQLITE_OPEN_FULLMUTEX | SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	   free(res_path);
	   free(db_path);
	   sqlite3_exec(db, query, func, data, &err_msg);
	   sqlite3_free(err_msg);
	   sqlite3_close(db);
	   db = NULL;
}

int
_get_bookcount(char *book)
{
	int i=0;
	while (i < 66 && strcmp(book, Books[i]))
		i++;
	return i;
}

static int
_check(void *data, int argc, char **argv, char **azColName)
{
	return 0;
}

static void
_drop_table(char *table_name, void *data)
{
	char query[128];
	sprintf(query, "DROP TABLE IF EXISTS %s;", table_name);
	_app_database_query(query, &_check, data);
}

static int
_get_app_data(void *data, int argc, char **argv, char **azColName)
{
	appdata_s *ad = (appdata_s*)data;
	if (!strcmp(azColName[0], "bookcount"))
		ad->cur_book = atoi(argv[0]);
	if (!strcmp(azColName[1], "chaptercount"))
		ad->cur_chapter = atoi(argv[1]);
	return 0;
}

void
_load_appdata(appdata_s *ad)
{
	char query[256];
	sprintf(query, "SELECT bookcount,chaptercount FROM appinitdata;");
	_app_database_query(query, &_get_app_data, ad);
	sprintf(query, "create table if not exists bookmark(bookcount INT, chaptercount INT, versecount INT, verse VARCHAR(1024));");
	_app_database_query(query, &_check, ad);
	sprintf(query, "create table if not exists notes(bookcount INT, chaptercount INT, versecount INT, note VARCHAR(8192));");
	_app_database_query(query, &_check, ad);
}

void
_save_appdata(appdata_s *ad)
{
	char query[256];
	_drop_table("appinitdata", ad);
	sprintf(query, "CREATE TABLE appinitdata(bookcount int, chaptercount int);");
	_app_database_query(query, &_check, ad);
	sprintf(query, "INSERT INTO appinitdata VALUES(%d, %d);", ad->cur_book, ad->cur_chapter);
	_app_database_query(query, &_check, ad);
}

void
_database_query(char *query, int func(void*,int,char**,char**), void *data)
{
	sqlite3 *db = NULL;
	char *err_msg;

	   char *db_path = malloc(200);
	   char *res_path = app_get_resource_path();
	   sprintf(db_path, "%sholybible.db", res_path);
	   sqlite3_open(db_path, &(db));
	   free(res_path);
	   free(db_path);
	   sqlite3_exec(db, query, func, data, &err_msg);
	   sqlite3_free(err_msg);
	   sqlite3_close(db);
	   db = NULL;
}

static int
_get_verse_count(void *data, int argc, char **argv, char **azColName)
{
	appdata_s *ad = (appdata_s*)data;
	ad->versecount = atoi(argv[0]);
	return 0;
}

void
_get_verse_count_query(void *data, int book, int chapter)
{
	char query[128];

	sprintf(query, "select count(Versecount) from hin_bible where Book='%s' and Chapter=%d;", Books[book], chapter);
	_database_query(query, &_get_verse_count, data);
}

static int
_get_chapter_count(void *data, int argc, char **argv, char **azColName)
{
	appdata_s *ad = (appdata_s*)data;
	ad->chaptercount = atoi(argv[0]);
	return 0;
}

void
_get_chapter_count_query(void *data, int book)
{
	char query[128];

	sprintf(query, "select count(distinct Chapter) from hin_bible where Book='%s';", Books[book]);
	_database_query(query, &_get_chapter_count, data);
}

static int
_put_notes(void *data, int argc, char **argv, char **azColName)
{
   bible_verse_item *verse_item = (bible_verse_item*)data;
   if (argc == 1 && (atoi(argv[0]) > 0))
	   verse_item->note = EINA_TRUE;
   else
	   verse_item->note = EINA_FALSE;
   return 0;
}

static int
_put_bookmarks(void *data, int argc, char **argv, char **azColName)
{
   bible_verse_item *verse_item = (bible_verse_item*)data;
   if (argc == 1 && (atoi(argv[0]) > 0))
	   verse_item->bookmark = EINA_TRUE;
   else
	   verse_item->bookmark = EINA_FALSE;
   return 0;
}

void
_check_notes(appdata_s *ad)
{
   Elm_Object_Item *it = elm_genlist_first_item_get(ad->genlist);
   bible_verse_item *verse_item;
   char query[512];
   while (it)
   {
	   verse_item = (bible_verse_item*)elm_object_item_data_get(it);
	   sprintf(query, "SELECT count(note) FROM notes WHERE bookcount = %d AND chaptercount = %d AND versecount = %d", verse_item->bookcount, verse_item->chaptercount, verse_item->versecount);
	   _app_database_query(query, &_put_notes, verse_item);
	   it = elm_genlist_item_next_get(it);
   }
}

void
_check_bookmarks(appdata_s *ad)
{
   Elm_Object_Item *it = elm_genlist_first_item_get(ad->genlist);
   bible_verse_item *verse_item;
   char query[512];
   while (it)
   {
	   verse_item = (bible_verse_item*)elm_object_item_data_get(it);
	   sprintf(query, "SELECT count(versecount) FROM bookmark WHERE bookcount = %d AND chaptercount = %d AND versecount = %d", verse_item->bookcount, verse_item->chaptercount, verse_item->versecount);
	   _app_database_query(query, &_put_bookmarks, verse_item);
	   it = elm_genlist_item_next_get(it);
   }
}

static int
_get_verse_list(void *data, int argc, char **argv, char **azColName)
{
   appdata_s *ad = (appdata_s*) data;
   Elm_Object_Item *it;

   bible_verse_item *verse_item = malloc(sizeof(bible_verse_item));
   verse_item->versecount = ad->count;
   verse_item->verse = strdup(argv[0]);
   verse_item->bookcount = ad->cur_book;
   verse_item->chaptercount = ad->cur_chapter;
   verse_item->appdata = ad;
   verse_item->bookmark = EINA_FALSE;
   verse_item->note = EINA_FALSE;
   it = elm_genlist_item_append(ad->genlist, ad->itc, (void*)verse_item, NULL, ELM_GENLIST_ITEM_NONE, NULL, (void*)verse_item);
   elm_object_item_data_set(it, (void*)verse_item);
   verse_item->it = it;
   ad->count++;

   return 0;
}

void
_query_chapter(void *data, int book, int chapter)
{
	appdata_s *ad = (appdata_s*)data;
	char query[128];

	if (ad->genlist)
		elm_genlist_clear(ad->genlist);

    ad->count = 0;
    ad->cur_book = book;
    ad->cur_chapter = chapter;
    sprintf(query, "select h_verse from hin_bible where Book='%s' and Chapter=%d", Books[book], chapter);

	_database_query(query, &_get_verse_list, data);
	_check_bookmarks(ad);
	_check_notes(ad);

	sprintf(query, "%s %d", Books[book], chapter);
	elm_object_part_text_set(ad->layout, "elm.text.book_title", query);
}
