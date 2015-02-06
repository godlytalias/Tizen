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

var gServiceAppId = 'mwtpyWDgYs.tizen_silent_service',
    gServicePortName = 'SAMPLE_PORT',
    gLocalMessagePortName = 'SAMPLE_PORT_REPLY',
    gLocalMessagePort,
    gRemoteMessagePort,
    gLocalMessagePortWatchId,
    isStarting = false,
    start;

var started;

function sendCommand(command) {
    'use strict';

    gRemoteMessagePort.sendMessage([{ key: 'command', value: command }],
        gLocalMessagePort);
}

function onReceive(data) {
    'use strict';
    var message, i;

    for (i in data) {
        if (data.hasOwnProperty(i) && data[i].key === 'server') {
            message = data[i].value;
            alert(message);
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

    isStarting = false;

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
    }

    function onError(err) {
        console.error('Service Applaunch failed', err);
        isStarting = false;
       // showAlert('Failed to launch HybridServiceApp!');
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
    started=true;
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
            console.log('Running Service App found');
            started=true;
            break;
        }
    }

    if (i >= contexts.length) {
        console.log('Running Service App not found.');
        started=false;
    }
}

function onGetAppsContextError(err) {
    'use strict';
    console.error('getAppsContext exc', err);
}

function check_start() {
    'use strict';
    try {
        tizen.application.getAppsContext(onGetAppsContextSuccess,
            onGetAppsContextError);
    } catch (exc) {
        console.error('Get AppContext Error', err);
    }
}

function getAppsInfoSuccessCB(apps) {
    'use strict';
    var i;
    for (i = 0; i < apps.length; i = i + 1) {
        if (apps[i].id === gServiceAppId) {
            console.log('Found installed Service App ');
            break;
        }
    }
    if (i >= apps.length) {
        writeToScreen('Service App not installed');
        isStarting = false;
        return;
    }
    launchServiceApp();
}

function getAppsInfoErrorCB(err) {
    'use strict';
    console.error('getAppsInfo failed', err);
    isStarting = false;
}

function listInstalledApps() {
    'use strict';
    try {
        tizen.application.getAppsInfo(getAppsInfoSuccessCB, getAppsInfoErrorCB);
    } catch (exc) {
        writeToScreen('Get Installed App Info Error');
    }
}

function set_slider(){
	var v;
	if(localStorage.getItem("alertstat")===null)
		started = false;
	else if(localStorage.getItem("alertstat")=="on")
		started = true;
	else
		started = false;
	
	if(localStorage.getItem("callno")===null)
		localStorage.setItem("callno", "5");
	if(localStorage.getItem("timelimit")===null)
		localStorage.setItem("timelimit", "5");
	
 	if(started){
 		 $('#alertstat').val('on').slider("refresh");
 		 v=localStorage.getItem("callno");
 		 $('#callno').val(v).slider("refresh");
 		 v=localStorage.getItem("timelimit");
 		 $('#timelimit').val(v).slider("refresh");
 	}
 	else
 		{
 		 $('#alertstat').val('off').slider("refresh");
 		 $('#callno').slider('disable');
 		 $('#timelimit').slider('disable');
 		}
}

$(document).delegate('#main', 'pageinit', function onMainPageInit() {
    'use strict';
    
    set_slider();
   
    $('#alertstat').bind('change',function(evt, val){
    	var myflip = $(this);
    	var selected = myflip[0].value;
    	if(selected==="on")
    		{
    		localStorage.setItem("alertstat", "on");
    		 $('#callno').slider('enable');
    		 $('#timelimit').slider('enable');
    		 if (!gLocalMessagePort) {
    	            isStarting = true;
    	            launchServiceApp();
    	            startMessagePort(); 
    	            sendCommand("callno5");
    	            sendCommand("timelt5");
    	        }
    	        return false;
    		}
    	else
    		{
    		localStorage.setItem("alertstat", "off");
    		 $('#callno').slider('disable');
     		 $('#timelimit').slider('disable');
    		if (gRemoteMessagePort) {
                sendCommand('stop');
            } else {
                startMessagePort();
                sendCommand('stop');
            }
            return false;
    	}
    });
    
    $('#callno').on('slidestop',function(){
    	var val;
    	var myflip = $(this);
    	val = myflip[0].value;
    	localStorage.setItem("callno", val);
    	sendCommand("callno"+val);
    });
    
    $('#timelimit').on('slidestop',function(){
    	var val;
    	var myflip = $(this);
    	val = myflip[0].value;
    	localStorage.setItem("timelimit", val);
    	sendCommand("timelt"+val);
    });

    $('#btn-help').on('tap', function onClearBtnTap() {
        sendCommand("checking..");
        return false;
    });
    
    $('#btn-exit').on('tap', function onClearBtnTap() {
        tizen.application.getCurrentApplication().exit();
        return false;
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
