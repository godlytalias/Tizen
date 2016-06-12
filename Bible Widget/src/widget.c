#include <tizen.h>
#include "widget.h"

static void
widget_get_resource(const char *edj_file_in, char *edj_path_out, int edj_path_max)
{
	char *res_path = app_get_resource_path();
	if (res_path) {
		snprintf(edj_path_out, edj_path_max, "%s%s", res_path, edj_file_in);
		free(res_path);
	}
}

static void
_widget_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	widget_instance_data_s *wid = (widget_instance_data_s*)data;
	_widget_settings(wid);
}

static void
_load_prefs(widget_instance_data_s *wid)
{
	bool exist;
	int type;
	char style[64];

	preference_is_existing("font_size", &exist);
	if (exist)
		preference_get_int("font_size", &(wid->font_size));
	else
	{
		wid->font_size = 25;
		preference_set_int("font_size", 25);
	}

	preference_is_existing("text_r", &exist);
	if (exist)
		preference_get_int("text_r", &(wid->text_r));
	else
	{
		wid->text_r = 0;
		preference_set_int("text_r", 0);
	}

	preference_is_existing("text_g", &exist);
	if (exist)
		preference_get_int("text_g", &(wid->text_g));
	else
	{
		wid->text_g = 0;
		preference_set_int("text_g", 0);
	}

	preference_is_existing("text_b", &exist);
	if (exist)
		preference_get_int("text_b", &(wid->text_b));
	else
	{
		wid->text_b = 0;
		preference_set_int("text_b", 0);
	}

	preference_is_existing("text_a", &exist);
	if (exist)
		preference_get_int("text_a", &(wid->text_a));
	else
	{
		wid->text_a = 255;
		preference_set_int("text_a", 255);
	}

	preference_is_existing("bg_r", &exist);
	if (exist)
		preference_get_int("bg_r", &(wid->bg_r));
	else
	{
		wid->bg_r = 0;
		preference_set_int("bg_r", 0);
	}

	preference_is_existing("bg_g", &exist);
	if (exist)
		preference_get_int("bg_g", &(wid->bg_g));
	else
	{
		wid->bg_g = 0;
		preference_set_int("bg_g", 0);
	}

	preference_is_existing("bg_b", &exist);
	if (exist)
		preference_get_int("bg_b", &(wid->bg_b));
	else
	{
		wid->bg_b = 0;
		preference_set_int("bg_b", 0);
	}

	preference_is_existing("bg_a", &exist);
	if (exist)
		preference_get_int("bg_a", &(wid->bg_a));
	else
	{
		wid->bg_a = 0;
		preference_set_int("bg_a", 0);
	}

	preference_is_existing("font_type", &exist);
	if (exist)
	{
		preference_get_int("font_type", &type);
		wid->font_style = Font_Style[type];
	}
	else
	{
		wid->font_style = Font_Style[1];
		preference_set_int("font_type", 1);
	}

	preference_is_existing("align", &exist);
	if (exist)
		preference_get_int("align", &(wid->align));
	else
	{
		wid->align = 0;
		preference_set_int("align", 0);
	}

	sprintf(style, "Tizen:style=%s", wid->font_style);
	edje_text_class_set("GTAwidget", style, wid->font_size);
	edje_color_class_set("GTAwidget", wid->text_r, wid->text_g, wid->text_b, wid->text_a, 0, 0, 0, 0, 0, 0, 0, 0);
	edje_color_class_set("GTAwidgetbg", wid->bg_r, wid->bg_g, wid->bg_b, wid->bg_a, 0, 0, 0, 0, 0, 0, 0, 0);
}

static int
widget_instance_create(widget_context_h context, bundle *content, int w, int h, void *user_data)
{
	widget_instance_data_s *wid = (widget_instance_data_s*) malloc(sizeof(widget_instance_data_s));
	int ret;

	if (content != NULL) {
		/* Recover the previous status with the bundle object. */
	}
	wid->verse_order = 1;
	wid->verse = NULL;
	/* Window */
	ret = widget_app_get_elm_win(context, &wid->win);
	if (ret != WIDGET_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "failed to get window. err = %d", ret);
		return WIDGET_ERROR_FAULT;
	}
	widget_get_resource(EDJ_FILE, wid->edj_path, (int)PATH_MAX);
	_load_prefs(wid);

	evas_object_resize(wid->win, w, h);

	/* Conformant */
	wid->conform = elm_conformant_add(wid->win);
	elm_app_base_scale_set(1.8);
	evas_object_size_hint_weight_set(wid->conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(wid->win, wid->conform);
	evas_object_show(wid->conform);

	wid->scroller = elm_scroller_add(wid->conform);

	wid->layout = elm_layout_add(wid->scroller);
	elm_layout_file_set(wid->layout, wid->edj_path, GRP_MAIN);
	evas_object_size_hint_weight_set(wid->layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(wid->layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_min_set(wid->layout, w, h);
	elm_layout_signal_callback_add(wid->layout, "elm,action,clicked", "elm", _widget_clicked_cb, wid);

	/* Show window after base gui is set up */
	elm_object_content_set(wid->scroller, wid->layout);
	elm_object_content_set(wid->conform, wid->scroller);
	evas_object_show(wid->win);

	_query_verse(wid);
	widget_app_context_set_tag(context, wid);
	return WIDGET_ERROR_NONE;
}

static int
widget_instance_destroy(widget_context_h context, widget_app_destroy_type_e reason, bundle *content, void *user_data)
{
	widget_instance_data_s *wid = NULL;
	widget_app_context_get_tag(context,(void**)&wid);

	if (wid->win)
		evas_object_del(wid->win);

	if (wid->verse) free(wid->verse);
	free(wid);

	return WIDGET_ERROR_NONE;
}

static int
widget_instance_pause(widget_context_h context, void *user_data)
{
	/* Take necessary actions when widget instance becomes invisible. */
	return WIDGET_ERROR_NONE;
}

static int
widget_instance_resume(widget_context_h context, void *user_data)
{
	/* Take necessary actions when widget instance becomes visible. */
	void *tag;
	widget_app_context_get_tag(context, &tag);
	widget_instance_data_s *wid = (widget_instance_data_s*)tag;
	double cur_time, prev_time;
	cur_time = ecore_time_unix_get();
	preference_get_double("versechangetime", &prev_time);
	if ((cur_time - prev_time) > 1800)
	{
		wid->verse_order++;
		_query_verse(wid);
	}
	return WIDGET_ERROR_NONE;
}

static int
widget_instance_update(widget_context_h context, bundle *content,
                             int force, void *user_data)
{
	/* Take necessary actions when widget instance should be updated. */
	void *tag;
	widget_app_context_get_tag(context, &tag);
	widget_instance_data_s *wid = (widget_instance_data_s*)tag;
	wid->verse_order++;
	_query_verse(wid);
	return WIDGET_ERROR_NONE;
}

static int
widget_instance_resize(widget_context_h context, int w, int h, void *user_data)
{
	/* Take necessary actions when the size of widget instance was changed. */
	return WIDGET_ERROR_NONE;
}

static void
widget_app_lang_changed(app_event_info_h event_info, void *user_data)
{
	/* APP_EVENT_LANGUAGE_CHANGED */
	char *locale = NULL;
	app_event_get_language(event_info, &locale);
	elm_language_set(locale);
	free(locale);
}

static void
widget_app_region_changed(app_event_info_h event_info, void *user_data)
{
	/* APP_EVENT_REGION_FORMAT_CHANGED */
}

static widget_class_h
widget_app_create(void *user_data)
{
	/* Hook to take necessary actions before main event loop starts.
	   Initialize UI resources.
	   Make a class for widget instance.
	*/
	app_event_handler_h handlers[5] = {NULL, };

	widget_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED],
		APP_EVENT_LANGUAGE_CHANGED, widget_app_lang_changed, user_data);
	widget_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED],
		APP_EVENT_REGION_FORMAT_CHANGED, widget_app_region_changed, user_data);

	widget_instance_lifecycle_callback_s ops = {
		.create = widget_instance_create,
		.destroy = widget_instance_destroy,
		.pause = widget_instance_pause,
		.resume = widget_instance_resume,
		.update = widget_instance_update,
		.resize = widget_instance_resize,
	};

	return widget_app_class_create(ops, user_data);
}

static void
widget_app_terminate(void *user_data)
{
	/* Release all resources. */
}

int
main(int argc, char *argv[])
{
	widget_app_lifecycle_callback_s ops = {0,};
	int ret;

	ops.create = widget_app_create;
	ops.terminate = widget_app_terminate;

	ret = widget_app_main(argc, argv, &ops, NULL);
	if (ret != WIDGET_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "widget_app_main() is failed. err = %d", ret);
	}

	return ret;
}
