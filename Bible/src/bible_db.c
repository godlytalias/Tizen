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
	bool existing = false;
	ad->parallel_db_path = NULL;

	sprintf(query, "SELECT bookcount,chaptercount FROM appinitdata;");
	_app_database_query(query, &_get_app_data, ad);
	sprintf(query, "create table if not exists bookmark(bookcount INT, chaptercount INT, versecount INT, verse VARCHAR(1024));");
	_app_database_query(query, &_check, ad);
	sprintf(query, "create table if not exists notes(bookcount INT, chaptercount INT, versecount INT, note VARCHAR(8192));");
	_app_database_query(query, &_check, ad);
	preference_is_existing("fontsize", &existing);
	if (!existing)
	{
		preference_set_int("fontsize", 25);
		edje_text_class_set("GTA1", "Tizen:style=Regular", 25);
		edje_text_class_set("GTA1B", "Tizen:style=Bold", 25);
		edje_text_class_set("GTA1L", "Tizen:style=Light", 25);
	}
	else
	{
		int val = 25;
		preference_get_int("fontsize", &val);
		edje_text_class_set("GTA1", "Tizen:style=Regular", val);
		edje_text_class_set("GTA1B", "Tizen:style=Bold", val);
		edje_text_class_set("GTA1L", "Tizen:style=Light", val);
	}
	preference_is_existing("parallel", &existing);
	if (!existing)
		preference_set_boolean("parallel", existing);
	else
	{
		bool parallel = false;
		preference_get_boolean("parallel", &parallel);
		if (parallel)
		{
			char *para_app_id = NULL;
			char *db_path = NULL;
			package_info_h pkg_info;
			if (preference_get_string("parallel_app_id", &para_app_id) == PREFERENCE_ERROR_NONE)
			{
				if (package_manager_get_package_info(para_app_id, &pkg_info) == PACKAGE_MANAGER_ERROR_NONE)
				{
					if (package_info_get_root_path(pkg_info, &db_path) != PACKAGE_MANAGER_ERROR_NONE)
					{
						preference_set_boolean("parallel", !parallel);
						preference_remove("parallel_app_id");
						ad->parallel_db_path = NULL;
					}
					else
					{
						ad->parallel_db_path = (char*)malloc(sizeof(char) * strlen(db_path) + 32);
						strcpy(ad->parallel_db_path, db_path);
						strcat(ad->parallel_db_path, "/shared/res/holybible.db");
					}
					package_info_destroy(pkg_info);
					if (db_path) free(db_path);
				}
				else
				{
					preference_set_boolean("parallel", false);
					preference_remove("parallel_app_id");
				}
			}
			else
			{
				preference_set_boolean("parallel", false);
				preference_remove("parallel_app_id");
			}
			if (para_app_id) free(para_app_id);
		}
	}
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

	sprintf(query, "select count(Versecount) from %s where Book=%d and Chapter=%d;", BIBLE_TABLE_NAME, book, chapter);
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

	sprintf(query, "select count(distinct Chapter) from %s where Book=%d;", BIBLE_TABLE_NAME, book);
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
   if (argv == NULL) return 0;

   bible_verse_item *verse_item = malloc(sizeof(bible_verse_item));
   verse_item->versecount = ad->count;
   verse_item->verse = strdup(argv[0]);
   if (argc > 1)
	   verse_item->verse_s = strdup(argv[1]);
   else
	   verse_item->verse_s = NULL;
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
	bool parallel = false;
	char query[512];

	if (ad->genlist)
		elm_genlist_clear(ad->genlist);

	Evas_Object *verse_popup = (Evas_Object*)evas_object_data_get(ad->genlist, "verse_popup");
	if (verse_popup)
	{
		elm_ctxpopup_dismiss(verse_popup);
	}

    ad->count = 0;
    ad->cur_book = book;
    ad->cur_chapter = chapter;
    preference_get_boolean("parallel", &parallel);
    if (!parallel)
    	sprintf(query, "select %s from %s where Book=%d and Chapter=%d", BIBLE_VERSE_COLUMN, BIBLE_TABLE_NAME, book, chapter);
    else
    {
    	if (ad->parallel_db_path)
    		sprintf (query, "attach database '%s' as db1;select verse,verse_s from "
    				"(select verse,Book,Chapter,Versecount from %s where Book=%d and Chapter=%d) t1 inner join "
    				"(select verse as verse_s,Book,Chapter,Versecount from db1.%s where Book=%d and Chapter=%d) t2 on "
    				"(t1.Book=t2.Book and t1.Chapter=t2.Chapter and t1.Versecount=t2.Versecount);",
    				ad->parallel_db_path, BIBLE_TABLE_NAME, book, chapter, BIBLE_TABLE_NAME, book, chapter);
    	else
        	sprintf(query, "select %s from %s where Book=%d and Chapter=%d", BIBLE_VERSE_COLUMN, BIBLE_TABLE_NAME, book, chapter);
    }
    _database_query(query, &_get_verse_list, data);
    if (elm_genlist_items_count(ad->genlist) == 0)
    {
    	preference_set_boolean("parallel", false);
		preference_remove("parallel_app_id");
    	_query_chapter(ad, ad->cur_book, ad->cur_chapter);
    	return;
    }
	_check_bookmarks(ad);
	_check_notes(ad);

	sprintf(query, "%s %d", Books[book], chapter);
	elm_object_part_text_set(ad->layout, "elm.text.book_title", query);
}
