/*
 * Copyright (c) 2012 Samsung Electronics Co., Ltd. All rights reserved.
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

/*jslint devel:true*/
/*global $, tizen, window, document, history*/

var gServiceAppId = 'E4W9zCiCoO.silenttracker',
    gServicePortName = 'SAMPLE_PORT',
    gLocalMessagePortName = 'SAMPLE_PORT_REPLY',
    gLocalMessagePort,
    gRemoteMessagePort,
    gLocalMessagePortWatchId,
    started = false,
    start;

function writeToScreen(message,unixtime) {
    'use strict';
    var today = new Date(unixtime*1000),
        time = today.getFullYear() + '-' + (today.getMonth() + 1) + '-' +
            today.getDate() + ' ' + today.getHours() + ':' +
            today.getMinutes(),
        str = '<li><a href="#"><h3>' +
            message +
            '</h3><p>' +
            time +
            '</p></a></li>';

    $('#logs').append(str).listview('refresh');
    document.getElementById('logs').scrollIntoView(false);
}


function sendCommand(command) {
    'use strict';

    gRemoteMessagePort.sendMessage([{ key: 'command', value: command }],
        gLocalMessagePort);
}

function onReceive(data) {
    'use strict';
    var message, i, time;
    for (i in data) {
        if (data.hasOwnProperty(i)) {
        	if(data[i].key === 'change')
             message = data[i].value;
        	if(data[i].key === 'time')
        	{ time = data[i].value;
              writeToScreen(message,time);}
        }
    }

   
}

function startMessagePort() {
    'use strict';
    try {
        gLocalMessagePort = tizen.messageport
            .requestLocalMessagePort(gLocalMessagePortName);
        gLocalMessagePortWatchId = gLocalMessagePort
            .addMessagePortListener(function onDataReceive(data, remote) {
                onReceive(data, remote);
            });
    } catch (e) {
        gLocalMessagePort = null;
    }

    try {
        gRemoteMessagePort = tizen.messageport
            .requestRemoteMessagePort(gServiceAppId, gServicePortName);
    } catch (er) {
        gRemoteMessagePort = null;
    }
}

function showAlert(message) {
    'use strict';
    var alertPopup = $('#alert-popup');
    alertPopup.find('#message').html(message);
    alertPopup.popup('open', {positionTo: 'window'});
}

function launchServiceApp() {
    'use strict';
    function onSuccess() {
        console.log('Service App launched successfully!');
        console.log('Restart...');
        started=true;
    }

    function onError(err) {
        console.error('Service Applaunch failed', err);
        showAlert('Failed to launch HybridServiceApp!');
    }

    try {
        console.log('Launching [' + gServiceAppId + '] ...');
        tizen.application.launch(gServiceAppId, onSuccess, onError);
    } catch (exc) {
        console.error('Exception while launching HybridServiceApp: ' +
            exc.message);
        showAlert('Exception while launching HybridServiceApp:<br>' +
            exc.message);
    }
}

function start() {
    'use strict';
    launchServiceApp();
    startMessagePort();
    sendCommand('start');
    sendCommand('get');
}

function stop() {
    'use strict';
	sendCommand('stop');
}

function getAppsInfoSuccessCB(apps) {
    'use strict';
    var i;
    for (i = 0; i < apps.length; i = i + 1) {
        if (apps[i].id === gServiceAppId) {
            console.log('Found installed Service App');
            break;
        }
    }
    if (i >= apps.length) {
        return;
    }
    launchServiceApp();
}

function getAppsInfoErrorCB(err) {
    'use strict';
    console.error('getAppsInfo failed', err);
}

function listInstalledApps() {
    'use strict';
    try {
        tizen.application.getAppsInfo(getAppsInfoSuccessCB, getAppsInfoErrorCB);
    } catch (exc) {
    }
}

function set_values(){
	if(started){
	 $("a#btn-start").text("STOP");
	 if(!gLocalMessagePort)
		 startMessagePort();
	 sendCommand("get");
	}
}

function onGetAppsContextSuccess(contexts) {
    'use strict';
    var i, appInfo;
    for (i = 0; i < contexts.length; i = i + 1) {
        try {
            appInfo = tizen.application.getAppInfo(contexts[i].appId);
        } catch (exc) {
            console.log('Exception while getting application info: ' +
                exc.message);
            showAlert('Exception while getting application info:<br>' +
                exc.message);
        }
        if (appInfo.id === gServiceAppId) {
            started=true;
            console.log('Running Service App found');
            break;
        }
    }
    set_values();
}

function onGetAppsContextError(err) {
    'use strict';
    console.error('getAppsContext exc', err);
    set_values();
}

function check_start() {
    'use strict';
    $.mobile.loading('show', {theme:"b", text:"Loading...", textonly:false, textVisible: true}); 
    started=false;
    try {
        tizen.application.getAppsContext(onGetAppsContextSuccess,
            onGetAppsContextError);
    } catch (exc) {
        console.error('Get AppContext Error', err);
    }
}

$(document).delegate('#main', 'pageinit', function onMainPageInit() {
    'use strict';
    
    check_start();
    
    $('#btn-start').on('tap', function onStartBtnTap() {
    	if(started){
    		$('#btn-start').text('START');
    		stop();
    	}
    	else{
    		$('#btn-start').text('STOP');
    		start();
    	}
    	$('#btn-start').button('refresh');
        return false;
    });

    $('#btn-exit').on('tap', function onStopBtnTap() {
    	 try {
             tizen.application.getCurrentApplication().exit();
         } catch (exc) {
             console.error('Error: ', exc.message);
         }
    });
    
    $('#btn-ins').on('tap', function onAddBtnTap() {
    	$('#details').popup('close');
    	$('#logs').empty();
   	   if(started)
   		   {
   		   		sendCommand('insert');
   		   }
   	   else
   		   alert("Tracking service is not started!");
   });
    
    $(window).on('tizenhwkey', function onTizenHWKey(e) {
        if (e.originalEvent.keyName === 'back') {
            if ($.mobile.activePage.attr('id') === 'main') {
                try {
                    tizen.application.getCurrentApplication().exit();
                } catch (exc) {
                    console.error('Error: ', exc.message);
                }
            } else {
                history.back();
            }
        }
    });
});
