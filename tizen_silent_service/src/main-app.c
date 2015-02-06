/*
 * Copyright 2014 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "main-app.h"
#include "defines-app.h"
#include "types-app.h"
#include "logger.h"
#include "proxy-client.h"
#include <notification.h>
#include <system_settings.h>
#include <Ecore.h>
#include <telephony.h>
#include <contacts.h>
#include <tone_player.h>
#include <sound_manager.h>

#include <service_app.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>

// app event callbacks
static bool _on_create_cb(void *user_data);
static void _on_terminate_cb(void *user_data);

static int _app_init(app_data *app);
static int _app_send_response(app_data *app, bundle *const msg);
static int _app_process_received_message(bundle *rec_msg, bundle *resp_msg);
static void* _app_timer_thread_func(void *data);
static int _on_proxy_client_msg_received_cb(void *data, bundle *const rec_msg);

telephony_handle_list_s list;
static bool init, silent_mode;
static int callno, timelimit;
void init_telephony(void);
void de_init_telephony(void);
notification_h tel_noti = NULL, noti=NULL, notif=NULL;
int noti_err = NOTIFICATION_ERROR_NONE;

static void
service_silent_mode(system_settings_key_e key, void *user_data)
{
	bool silent;
	system_settings_get_value_bool(SYSTEM_SETTINGS_KEY_SOUND_SILENT_MODE, &silent);
	if(silent)
	{
		silent_mode = true;
		init_telephony();
		char *s = malloc(100);
		sprintf(s,"If %d calls within %d minutes",callno,timelimit);

	        	tel_noti = notification_create(NOTIFICATION_TYPE_NOTI);
	        	notification_set_time(tel_noti,0);
	        	notification_set_text(tel_noti,NOTIFICATION_TEXT_TYPE_TITLE,"Silent Call Alert",NULL,NOTIFICATION_VARIABLE_TYPE_NONE);
	        	notification_set_text(tel_noti,NOTIFICATION_TEXT_TYPE_CONTENT,s,NULL,NOTIFICATION_VARIABLE_TYPE_NONE);
	        	notification_set_sound(tel_noti,NOTIFICATION_SOUND_TYPE_DEFAULT,NULL);
	        	noti_err = notification_post(tel_noti);
		notification_status_message_post("Silent Call Alert");
		free(s);
	}
	else if(init)
	{
		silent_mode = false;
		de_init_telephony();
		notification_delete(tel_noti);
		notification_free(tel_noti);
	}
	return;
}

static void caller_data_cb(const char *view_uri,void *user_data)
{
	int ptime = ecore_time_unix_get();
	    	char *caller = malloc(50);
	    	ptime = ptime - (timelimit*60); //reducing 'timelimit' minutes
	    	contacts_list_h list = NULL, nlist = NULL;
	    	contacts_query_h query = NULL;
	    	contacts_filter_h filter = NULL;

	    	//gets last caller
	    	contacts_query_create(_contacts_phone_log._uri,&query);
	    	contacts_filter_create(_contacts_phone_log._uri, &filter);
	    	contacts_filter_add_int(filter, _contacts_phone_log.log_type, CONTACTS_MATCH_EQUAL, CONTACTS_PLOG_TYPE_VOICE_OUTGOING);
	    	contacts_filter_add_operator(filter,CONTACTS_FILTER_OPERATOR_AND);
	    	contacts_filter_add_int(filter, _contacts_phone_log.log_time, CONTACTS_MATCH_GREATER_THAN, ptime);
	    	contacts_query_set_filter(query,filter);
	    	contacts_db_get_records_with_query(query,0,0,&nlist);
	    	contacts_filter_destroy(filter);
	    	contacts_query_destroy(query);

	    	contacts_record_h nrecord;
	    	if (contacts_list_get_current_record_p(nlist, &nrecord) != TELEPHONY_ERROR_NONE)
	    	{
				ERR("No caller");
	    	}
	    	contacts_record_get_str(nrecord,_contacts_phone_log.address,&caller);

	  	//gets frequency of caller
	    	contacts_query_create(_contacts_phone_log._uri,&query);
	    	contacts_filter_create(_contacts_phone_log._uri, &filter);
	    	contacts_filter_add_int(filter, _contacts_phone_log.log_type, CONTACTS_MATCH_EQUAL, CONTACTS_PLOG_TYPE_VOICE_OUTGOING);
	    	contacts_filter_add_operator(filter,CONTACTS_FILTER_OPERATOR_AND);
	    	contacts_filter_add_int(filter, _contacts_phone_log.log_time, CONTACTS_MATCH_GREATER_THAN, ptime);
	    	contacts_filter_add_operator(filter,CONTACTS_FILTER_OPERATOR_AND);
	    	contacts_filter_add_str(filter, _contacts_phone_log.address, CONTACTS_MATCH_EXACTLY, caller);
	    	contacts_query_set_filter(query,filter);
	    	contacts_db_get_records_with_query(query,0,0,&list);
	    	contacts_filter_destroy(filter);
	    	contacts_query_destroy(query);
	    	contacts_record_h record;
	    	int count;
	    	contacts_list_get_count(list,&count);
	    	if (contacts_list_get_current_record_p(list, &record) != TELEPHONY_ERROR_NONE)
	    	{
	    		ERR("No record!");
	    	}
	    	if(count>=callno){
	    		char *s = malloc(100);
	    		int max;
	    		sound_manager_get_max_volume(SOUND_TYPE_ALARM,&max);
	    		sprintf(s,"%s had called %d times",caller,count);
	    		if(notif == NULL){
	    		notif = notification_create(NOTIFICATION_TYPE_ONGOING);
	    			        	notification_set_time(notif,0);
	    			        	notification_set_layout(notif,NOTIFICATION_LY_ONGOING_EVENT);
	    			        	notification_set_text(notif,NOTIFICATION_TEXT_TYPE_TITLE,"Silent Call Alert",NULL,NOTIFICATION_VARIABLE_TYPE_NONE);
	    			        	notification_set_text(notif,NOTIFICATION_TEXT_TYPE_CONTENT,s,NULL,NOTIFICATION_VARIABLE_TYPE_NONE);
	    			        	notification_set_sound(notif,NOTIFICATION_SOUND_TYPE_DEFAULT,NULL);
	    			        	noti_err = notification_post(notif);
	    		}
	    		else
	    		{
	    			char *text = malloc(100);
	    			notification_get_text(notif,NOTIFICATION_TEXT_TYPE_CONTENT,&text);
	    			if((strlen(caller)<=strlen(text)) && strncmp(text,caller,strlen(caller))==0)
	    			{
	    				notification_set_time(notif,0);
	    			  	notification_set_layout(notif,NOTIFICATION_LY_ONGOING_EVENT);
	    				notification_set_text(notif,NOTIFICATION_TEXT_TYPE_TITLE,"Silent Call Alert",NULL,NOTIFICATION_VARIABLE_TYPE_NONE);
	    				notification_set_text(notif,NOTIFICATION_TEXT_TYPE_CONTENT,s,NULL,NOTIFICATION_VARIABLE_TYPE_NONE);
	    				notification_set_sound(notif,NOTIFICATION_SOUND_TYPE_DEFAULT,NULL);
	    				noti_err = notification_update(notif);
	    			}
	    			else
	    			{
	    				sprintf(s,"%s\n%s",s,text);
	    				notification_set_time(notif,0);
	    				notification_set_layout(notif,NOTIFICATION_LY_ONGOING_EVENT);
	    				notification_set_text(notif,NOTIFICATION_TEXT_TYPE_TITLE,"Silent Call Alert",NULL,NOTIFICATION_VARIABLE_TYPE_NONE);
	    				notification_set_text(notif,NOTIFICATION_TEXT_TYPE_CONTENT,s,NULL,NOTIFICATION_VARIABLE_TYPE_NONE);
	    				notification_set_sound(notif,NOTIFICATION_SOUND_TYPE_DEFAULT,NULL);
	    				noti_err = notification_update(notif);
	    			}
	    			free(text);
	    		}
	    		notification_status_message_post("Alert!");
	    		silent_mode = false;
	    		sound_manager_set_volume(SOUND_TYPE_ALARM,max);
	    		tone_player_start(TONE_TYPE_CDMA_CALL_SIGNAL_ISDN_PING_RING,SOUND_TYPE_ALARM,3000,NULL);
	        	free(s);
	    	}
	    	free(caller);
	    	contacts_db_remove_changed_cb(_contacts_phone_log._uri,caller_data_cb,NULL);
	    	return;
}

void check_cb(telephony_h handle, telephony_noti_e noti_id, void *data, void* user_data)
{
    telephony_call_state_e call_state = *(int *)data;

    if(call_state == TELEPHONY_CALL_STATE_CONNECTING)
	{
    	contacts_db_add_changed_cb(_contacts_phone_log._uri ,caller_data_cb,NULL);
	}
    return;
}

void init_telephony(void)
{
	int count;
	contacts_connect();
	if(telephony_init(&list)==0)
	{
		count = list.count;
		for(int i=0;i<count;i++)
		{
			telephony_set_noti_cb(list.handle[i],TELEPHONY_NOTI_VOICE_CALL_STATE,check_cb,NULL);
		}
	}
	init = true;
}

void de_init_telephony()
{
	int count = list.count;
	for(int i=0;i<count;i++)
	{
		telephony_unset_noti_cb(list.handle[i],TELEPHONY_NOTI_VOICE_CALL_STATE);
	}
	telephony_deinit(&list);
	contacts_disconnect();
	init = false;
}



app_data *app_create()
{
    app_data *app = calloc(1, sizeof(app_data));
    return app;
}

void app_destroy(app_data *app)
{
    free(app);
}

int app_run(app_data *app, int argc, char **argv)
{
    RETVM_IF(!app, -1, "Application data is NULL");

    service_app_lifecycle_callback_s cbs =
    {
        .create = _on_create_cb,
        .terminate = _on_terminate_cb
    };

    return service_app_main(argc, argv, &cbs, app);
}


static bool _on_create_cb(void *user_data)
{
	RETVM_IF(!user_data, false, "User_data is NULL");
	init = false;

	system_settings_set_changed_cb(SYSTEM_SETTINGS_KEY_SOUND_SILENT_MODE,service_silent_mode,NULL);
	service_silent_mode(SYSTEM_SETTINGS_KEY_SOUND_SILENT_MODE,NULL);

	RETVM_IF(_app_init(user_data) != SVC_RES_OK, false, "Failed to initialise application data");

        	noti = notification_create(NOTIFICATION_TYPE_NOTI);
        	notification_set_time(noti,0);
        	notification_set_text(noti,NOTIFICATION_TEXT_TYPE_TITLE,"Silent Call Alert",NULL,NOTIFICATION_VARIABLE_TYPE_NONE);
        	notification_set_text(noti,NOTIFICATION_TEXT_TYPE_CONTENT,"Alert service activated!",NULL,NOTIFICATION_VARIABLE_TYPE_NONE);
        	notification_set_sound(noti,NOTIFICATION_SOUND_TYPE_DEFAULT,NULL);
        	noti_err = notification_post(noti);
    notification_status_message_post("Silent call alert active");

    return true;
}

static void _on_terminate_cb(void *user_data)
{
    if (user_data)
    {
    	 app_data *app = user_data;
    	if(app->tmr_thread)
        {
            pthread_mutex_lock(&app->tmr_mutex);
            app->run_timer = false;
            app->run_svc = false;
            pthread_mutex_unlock(&app->tmr_mutex);
            pthread_cond_signal(&app->tmr_cond);

            pthread_join(app->tmr_thread, NULL);
            app->tmr_thread = 0;
        }
        pthread_cond_destroy(&app->tmr_cond);
        pthread_mutex_destroy(&app->tmr_mutex);
        pthread_mutex_destroy(&app->proxy_locker);
        proxy_client_destroy(app->proxy_client);
    }
}

static int _on_proxy_client_msg_received_cb(void *data, bundle *const rec_msg)
{
    int result = SVC_RES_FAIL;
    RETVM_IF(!data, result, "Data is NULL");

    app_data *app = data;

    bundle *resp_msg = bundle_create();
    RETVM_IF(!resp_msg, result, "Failed to create bundle");

    result = _app_process_received_message(rec_msg, resp_msg);
    if (result == SVC_RES_FAIL)
    {
        ERR("Failed to generate response bundle");
        bundle_free(resp_msg);
        return result;
    }
    else if (result == 2)
    {
    	    	system_settings_unset_changed_cb(SYSTEM_SETTINGS_KEY_SOUND_SILENT_MODE);
    	        proxy_client_destroy(app->proxy_client);
    	        notification_delete(noti);
    	        notification_free(noti);
    	        notification_status_message_post("Deactivated silent call alert");
    	exit(0);
    }

    if(result != SVC_RES_OK)
    {
        ERR("Failed to execute operation");
    }
    bundle_free(resp_msg);

    return result;
}

static int _app_init(app_data *app)
{
    int result = SVC_RES_FAIL;
    RETVM_IF(!app, result, "Application data is NULL");
    app->proxy_client = proxy_client_create();
    RETVM_IF(!app->proxy_client, result, "Failed to create proxy client");

    result = proxy_client_register_port(app->proxy_client, LOCAL_MESSAGE_PORT_NAME);
    if (result != SVC_RES_OK)
    {
        ERR("Failed to register proxy client port\n\n\n");
        proxy_client_destroy(app->proxy_client);
        app->proxy_client = NULL;
        return result;
    }

    result = proxy_client_register_msg_receive_callback(app->proxy_client, _on_proxy_client_msg_received_cb, app);
    if (result != SVC_RES_OK)
    {
        ERR("Failed to register proxy client on message receive callback");
        proxy_client_destroy(app->proxy_client);
        app->proxy_client = NULL;
        return result;
    }

    result = pthread_cond_init(&app->tmr_cond, NULL);
    if(result != 0)
    {
        ERR("Failed to init timer condition ");
        proxy_client_destroy(app->proxy_client);
        app->proxy_client = NULL;
        return SVC_RES_FAIL;
    }

    result = pthread_mutex_init(&app->tmr_mutex, NULL);
    if(result != 0)
    {
        ERR("Failed to init timer mutex");
        pthread_cond_destroy(&app->tmr_cond);
        proxy_client_destroy(app->proxy_client);
        app->proxy_client = NULL;
        return SVC_RES_FAIL;
    }

    result = pthread_mutex_init(&app->proxy_locker, NULL);
    if(result != 0)
    {
        ERR("Failed to init message response mutex ");
        pthread_cond_destroy(&app->tmr_cond);
        pthread_mutex_destroy(&app->tmr_mutex);
        proxy_client_destroy(app->proxy_client);
        app->proxy_client = NULL;
        return SVC_RES_FAIL;
    }

    app->run_svc = true;
    app->run_timer = false;

    result = pthread_create(&app->tmr_thread, NULL, _app_timer_thread_func, app);
    if(result != 0)
    {
        ERR("Failed to create timer thread");
        pthread_cond_destroy(&app->tmr_cond);
        pthread_mutex_destroy(&app->tmr_mutex);
        pthread_mutex_destroy(&app->proxy_locker);
        proxy_client_destroy(app->proxy_client);
        app->proxy_client = NULL;
        return SVC_RES_FAIL;
    }
    return SVC_RES_OK;
}

static int _app_process_received_message(bundle *rec_msg,  bundle *resp_msg)
{
    RETVM_IF(!rec_msg, SVC_RES_FAIL,"Received message is NULL");
    RETVM_IF(!resp_msg, SVC_RES_FAIL,"Response message is NULL");

    char *resp_key_val = malloc(100);
    const char *rec_key_val = bundle_get_val(rec_msg, "command");

    if(strcmp(rec_key_val,"stop")==0){
    	return 2;
    }
    else if(strncmp(rec_key_val,"callno",6)==0)
    {
    	callno=atoi(rec_key_val+6);
    	sprintf(resp_key_val,"Call No : %d", callno);

    	    RETVM_IF(bundle_add(resp_msg, "server", resp_key_val) != 0, SVC_RES_FAIL, "Failed to add data by key to bundle");
    	if(init)
    	{
    	char *s = malloc(100);
    	sprintf(s,"If %d calls within %d minutes",callno,timelimit);

    		notification_set_time(tel_noti,0);
    		        	notification_set_text(tel_noti,NOTIFICATION_TEXT_TYPE_TITLE,"Silent Call Alert",NULL,NOTIFICATION_VARIABLE_TYPE_NONE);
    		        	notification_set_text(tel_noti,NOTIFICATION_TEXT_TYPE_CONTENT,s,NULL,NOTIFICATION_VARIABLE_TYPE_NONE);
    		        	notification_set_sound(tel_noti,NOTIFICATION_SOUND_TYPE_DEFAULT,NULL);
    		        	noti_err = notification_update(tel_noti);
        free(s);
    	}

    	    return SVC_RES_OK;
    }
    else if(strncmp(rec_key_val,"timelt",6)==0)
        {
        	timelimit=atoi(rec_key_val+6);
        	sprintf(resp_key_val,"Time Limit: %d", timelimit);

        	if(init)
        	    	{
        	    	char *s = malloc(100);
        	    	sprintf(s,"If %d calls within %d minutes",callno,timelimit);

        	    		notification_set_time(tel_noti,0);
        	    		        	notification_set_text(tel_noti,NOTIFICATION_TEXT_TYPE_TITLE,"Silent Call Alert",NULL,NOTIFICATION_VARIABLE_TYPE_NONE);
        	    		        	notification_set_text(tel_noti,NOTIFICATION_TEXT_TYPE_CONTENT,s,NULL,NOTIFICATION_VARIABLE_TYPE_NONE);
        	    		        	notification_set_sound(tel_noti,NOTIFICATION_SOUND_TYPE_DEFAULT,NULL);
        	    		        	noti_err = notification_update(tel_noti);
        	        free(s);
        	    	}


        	    RETVM_IF(bundle_add(resp_msg, "server", resp_key_val) != 0, SVC_RES_FAIL, "Failed to add data by key to bundle");
        	return SVC_RES_OK;
        }
    else
    {
    sprintf(resp_key_val,"from server : %s", rec_key_val);

    RETVM_IF(bundle_add(resp_msg, "server", resp_key_val) != 0, SVC_RES_FAIL, "Failed to add data by key to bundle");
    return SVC_RES_OK;
    }
}


static void* _app_timer_thread_func(void *data)
{
    RETVM_IF(!data, NULL, "Data is NULL");
    app_data *app = data;

    pthread_mutex_lock(&app->tmr_mutex);
    bool was_timedwait = false;

    while(app->run_svc)
    {
        if(app->run_timer)
        {

            struct timespec nowTs;
            clock_gettime(CLOCK_REALTIME, &nowTs);
            nowTs.tv_sec += TIMER_INTERVAL_RUN;
            pthread_cond_timedwait(&app->tmr_cond, &app->tmr_mutex, &nowTs);
        }
        else
        {
            was_timedwait = false;
            pthread_cond_wait(&app->tmr_cond, &app->tmr_mutex);
        }
    }
    pthread_mutex_unlock(&app->tmr_mutex);

    return NULL;
}

static int _app_send_response(app_data *app, bundle *const msg)
{
    int res = SVC_RES_FAIL;
    pthread_mutex_lock(&app->proxy_locker);
    res = proxy_client_send_message(app->proxy_client, msg);
    pthread_mutex_unlock(&app->proxy_locker);

    return res;
}
