#include <tizen.h>
#include <sqlite3.h>
#include "bible.h"

static void
_search_keyword(void *data,
        Evas_Object *obj ,
        void *event_info EINA_UNUSED)
{
	elm_entry_entry_set(data, "");
}

void
_search_word(void *data,
              Evas_Object *obj ,
              void *event_info EINA_UNUSED)
{
	appdata_s *ad = (appdata_s*)data;
	char edj_path[PATH_MAX] = {0, };
	app_get_resource(EDJ_FILE, edj_path, (int)PATH_MAX);
	Evas_Object *layout = elm_layout_add(ad->win);
	elm_layout_file_set(layout, edj_path, "search_layout");
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(layout);
	Evas_Object *entry = elm_entry_add(layout);
	elm_entry_single_line_set(entry, EINA_TRUE);
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_object_part_text_set(entry, "elm.guide", "Enter the keyword");
	evas_object_show(entry);
	elm_object_part_content_set(layout, "elm.swallow.entry", entry);
	Evas_Object *go_btn = elm_button_add(layout);
	elm_object_text_set(go_btn, "Go");
	evas_object_smart_callback_add(go_btn, "clicked", _search_keyword, (void*)ad);
	evas_object_show(go_btn);
	elm_object_part_content_set(layout, "elm.swallow.go", go_btn);
	elm_naviframe_item_push(ad->naviframe, "Search", NULL, NULL, layout, NULL);
}
