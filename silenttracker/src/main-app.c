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

#include <service_app.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <stdio.h>
#include <Ecore.h>

// app event callbacks
static bool _on_create_cb(void *user_data);
static void _on_terminate_cb(void *user_data);
static void _on_low_memory_callback(void *data);
static void _on_low_battery_callback(void *data);

static int _app_init(app_data *app);
static int _app_send_response(app_data *app, bundle *const msg);
static int _app_process_received_message(bundle *rec_msg, app_data* app);
static int _on_proxy_client_msg_received_cb(void *data, bundle *const rec_msg);

sqlite3 *db;

app_data *app_create()
{
    app_data *app = calloc(1, sizeof(app_data));
    return app;
}

void app_destroy(app_data *app)
{
    free(app);
}

void create_database()
{
	char *Errmsg;
	char *sql = "CREATE TABLE IF NOT EXISTS silenttracker(TIME INTEGER NOT NULL, FROM_PROF TEXT NOT NULL, TO_PROF TEXT NOT NULL);";
	sqlite3_exec(db,sql,NULL,0,&Errmsg);
}

int app_run(app_data *app, int argc, char **argv)
{
    RETVM_IF(!app, -1, "Application data is NULL");

    service_app_event_callback_s cbs =
    {
        .create = _on_create_cb,
        .terminate = _on_terminate_cb,
        .low_memory = _on_low_memory_callback,
        .low_battery = _on_low_battery_callback
    };

    return service_app_main(argc, argv, &cbs, app);
}

static bool _on_create_cb(void *user_data)
{

    RETVM_IF(!user_data, false, "User_data is NULL");
    RETVM_IF(_app_init(user_data) != SVC_RES_OK, false, "Failed to initialise application data");
    return true;
}

static void _on_terminate_cb(void *user_data)
{
    if (user_data)
    {
        app_data *app = user_data;
        proxy_client_destroy(app->proxy_client);

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
    	sqlite3_shutdown();
    }
}

void _on_low_memory_callback(void *data)
{
    return service_app_exit();
}

void _on_low_battery_callback(void *data)
{
    return service_app_exit();
}

static int _on_proxy_client_msg_received_cb(void *data, bundle *const rec_msg)
{
    int result = SVC_RES_FAIL;
    RETVM_IF(!data, result, "Data is NULL");

    app_data *app = data;

    _app_process_received_message(rec_msg,app);

    return result;
}

static void insert_data()
{
	char *sql=malloc(200);
	char *Errmsg;
	int timeval = ecore_time_unix_get();
	char *fromprof = "Silent";
	char *toprof = "General";
	sprintf(sql,"INSERT INTO silenttracker VALUES(\'%d\',\'%s\',\'%s\');",timeval,fromprof,toprof);
	sqlite3_exec(db,sql,NULL,0,&Errmsg);
	notification_status_message_post(Errmsg);
	free(Errmsg);
	free(sql);
}

static int callback(void *apptemp, int argc, char **argv, char **azColName)
{
   app_data *app = (app_data *)apptemp;
   int i;
   bundle *resp_msg = bundle_create();
   notification_status_message_post("getting data..");
   char *msg = malloc(100);
   char *time = malloc(20);
   for (i=0; i<argc; i++)
   {
       sprintf(msg,"%s -> %s", &argv[1][i], &argv[2][i]);
   	   bundle_add(resp_msg, "change",msg);
   	   sprintf(time,"%s", &argv[0][i]);
   	   bundle_add(resp_msg, "time", time);
   }
   _app_send_response(app,resp_msg);
   bundle_free(resp_msg);
   free(msg);
   free(time);
   return 0;
}

static void send_data(app_data *app)
{
	char *Errmsg;
	char *sql = "SELECT * FROM silenttracker;";
	sqlite3_exec(db,sql,callback,app,&Errmsg);
	notification_status_message_post(Errmsg);
	free(Errmsg);
}

static int _app_init(app_data *app)
{
    int result = SVC_RES_FAIL;
    RETVM_IF(!app, result, "Application data is NULL");
    app->proxy_client = proxy_client_create();
    RETVM_IF(!app->proxy_client, result, "Failed to create proxy client");

    //database initialization
	sqlite3_config(SQLITE_CONFIG_URI,1);
	sqlite3_initialize();
	char *path = malloc(100);
	char *resource = app_get_data_path();
	sprintf(path,"%s/%s",resource,"test.db");
	free(path);
	sqlite3_open(path,&db);

    result = proxy_client_register_port(app->proxy_client, LOCAL_MESSAGE_PORT_NAME);
    if (result != SVC_RES_OK)
    {
        ERR("Failed to register proxy client port");
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

    return SVC_RES_OK;
}

static int _app_process_received_message(bundle *rec_msg,
        app_data *app)
{
    RETVM_IF(!rec_msg, SVC_RES_FAIL,"Received message is NULL");

    const char *rec_key_val = bundle_get_val(rec_msg, "command");

    if (strcmp(rec_key_val,"connect") == 0)
    {

    }
    else if (strcmp(rec_key_val,"start") == 0)
    {
        create_database();
    }
    else if (strcmp(rec_key_val,"insert") == 0)
    {
        insert_data();
    }
    else if (strcmp(rec_key_val,"get") == 0)
    {
        send_data(app);
    }
    else
    {

    }


    return SVC_RES_OK;
}


static int _app_send_response(app_data *app, bundle *const msg)
{
    int res = SVC_RES_FAIL;

    pthread_mutex_lock(&app->proxy_locker);
    res = proxy_client_send_message(app->proxy_client, msg);
    pthread_mutex_unlock(&app->proxy_locker);

    return res;
}
