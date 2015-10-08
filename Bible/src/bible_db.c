#include <tizen.h>
#include <sqlite3.h>
#include "bible.h"

void
_database_query(char *query, int func(void*,int,char**,char**), void *data)
{
	appdata_s *ad = (appdata_s*)data;
	char *err_msg;

	if (!ad->db) {
	   char *db_path = malloc(200);
	   sprintf(db_path, "%sholybible_eng.db", app_get_resource_path());
	   sqlite3_open(db_path, &(ad->db));
	   free(db_path);
	   sqlite3_exec(ad->db, query, func, (void*)ad, &err_msg);
	   sqlite3_free(err_msg);
	   sqlite3_close(ad->db);
	   ad->db = NULL;
	}
	else
	{
	   sqlite3_exec(ad->db, query, func, (void*)ad, &err_msg);
	   sqlite3_free(err_msg);
	}

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

	sprintf(query, "select count(Versecount) from eng_bible where Book='%s' and Chapter=%d;", Books[book], chapter);
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

	sprintf(query, "select count(distinct Chapter) from eng_bible where Book='%s';", Books[book]);
	_database_query(query, &_get_chapter_count, data);
}

static int
_get_verse_list(void *data, int argc, char **argv, char **azColName)
{
   appdata_s *ad = (appdata_s*) data;
   int i;
   for (i = 0; i < argc; i++)
   {
	   bible_verse_item *verse_item = malloc(sizeof(bible_verse_item));
	   verse_item->versecount = ad->count;
	   verse_item->verse = strdup(argv[i]);
	   verse_item->appdata = ad;
	   elm_genlist_item_append(ad->genlist, ad->itc, (void*)verse_item, NULL, ELM_GENLIST_ITEM_FIELD_CONTENT, NULL, (void*)verse_item);
	   ad->count++;
   }
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
    sprintf(query, "select e_verse from eng_bible where Book='%s' and Chapter=%d", Books[book], chapter);

	_database_query(query, &_get_verse_list, data);

	sprintf(query, "%s %d", Books[book], chapter);
	elm_object_part_text_set(ad->layout, "elm.text.book_title", query);
}
