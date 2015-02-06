var backEventListener = null;
var api_key = "9ed8e1cfed724312df2db3fabcb4da181b74e25e";
var pageno;
var sdate,edate;
var item;
var pcount;
var ppage=10;

function getitems(){
	$.ajax({
    	type: "POST",
    	   dataType: "json",
    	   contentType: "application/json; charset=utf-8",
    	   data: JSON.stringify({   
    	      ID: 2212,      
    	   }),
        url: "http://api.dataweave.in/v1/commodities/listAllCommodities/?api_key="+api_key,
        beforeSend: function() {
            $.mobile.loading('show', {theme:"b", text:"Loading...", textonly:false, textVisible: true}); 
        },
        success: function(resptxt){
        	putitems(resptxt);
        },
        complete: function() {
            $.mobile.loading('hide'); 
        }
});	
}

$(document).on('pageshow','#foo',function(){
	 getitems();
});

$(document).on('pageinit','#splash',function(){ // the .on() method does require jQuery 1.7 + but this will allow you to have the contained code only run when the #splash page is initialized.
	setTimeout(function(){
        $.mobile.changePage("index.html",{transition: "flow", changeHash: false});
    }, 1000);
});

var unregister = function() {
    if ( backEventListener !== null ) {
        document.removeEventListener( 'tizenhwkey', backEventListener );
        backEventListener = null;
        window.tizen.application.getCurrentApplication().exit();
    }
}

$(function(){
    document.oncontextmenu = function() {return false;};
    $(document).mousedown( function() {
        if ( e.button == 2 )
        { 
            //alert('Right mouse button!'); 
            return false; 
        }
        return true;
    });
});

$(window).on("tizenhwkey", function(e) 
		{
		   if (e.originalEvent.keyName === "menu") 
		   {
		      $("#popupmenu").popup("open");
		   }
		});

//Initialize function
var init = function () {
    // register once
			
		
    if ( backEventListener !== null ) {
        return;
    }
    pageno=0;
    item = "";
    sdate = "";
    edate = "";
    pcount = 0;
    // TODO:: Do your initialization job
    console.log("init() called");
    var backEvent = function(e) {
        if ( e.keyName == "back" ) {
            try {
                if ( $.mobile.urlHistory.activeIndex <= 0 ) {
                    // if first page, terminate app
                    document.removeEventListener( 'tizenhwkey', backEventListener );
                    backEventListener = null;
                    window.tizen.application.getCurrentApplication().exit();
                } else {
                    // move previous page
                    $.mobile.urlHistory.activeIndex -= 1;
                    $.mobile.urlHistory.clearForward();
                    pageno=pageno-1;
                    window.history.back();
                }
            } catch( ex ) {
                unregister();
            }
        }
    }
    
    // add eventListener for tizenhwkey (Back Button)
    document.addEventListener( 'tizenhwkey', backEvent );
    backEventListener = backEvent;
};


$(document).bind( 'pageinit', init );
$(document).unload( unregister );




function putitems(resptxt){
	var commodity = document.getElementById('item');
	content='';
	for(i=0;i<resptxt.count;i++){
		content+= '<option value="'+resptxt.data[i]+'">'+resptxt.data[i]+'</option>';
	}
	commodity.innerHTML = content;
}

function go()
{
    item = document.getElementById('item').value;
    sdate = document.getElementById('sdate').value.replace(/-/g, '');
    edate = document.getElementById('edate').value.replace(/-/g,'');
    
    var startdate = new Date(document.getElementById('sdate').value);
    var enddate = new Date(document.getElementById('edate').value);
    var today = new Date();
    
    if(enddate.getTime()<= today.getTime()){
    if(startdate.getTime() <= enddate.getTime() ){
    
    if(item!="" && sdate!="" && edate!="")
    	{
    	pageno=1;
    	sendgetreq(1);
    	}
    else
    	alert("Fill the fields!");
    
	}
	else
		alert("Start date should not exceed end date!");
    }
    else
    	alert("Date given should not exceed current date!");
}

function gocom()
{
    item = document.getElementById('itemcom').value;
    if(item!="")
    	{
    	pageno=1;
    	sendgetreq(2);
    	}
    else
    	alert("Fill the fields!");
}

function nextpage(id)
{
	if(pageno<Math.ceil(pcount/ppage)){
	pageno=pageno+1;
	sendgetreq(id);
	}
	else
		alert("Last page!");
}

function prevpage(id)
{
	if(pageno>1){
	pageno=pageno-1;
	sendgetreq(id);
	}
	else
		alert("No previous page!");
}

function home(id)
{
	if(id==1){
	$.mobile.changePage("index.html", {
        allowSamePageTransition: true,
        transition: 'fade',
        reloadPage: true
    });
	}
	else
		{
		$.mobile.changePage("ecommerce.html", {
	        allowSamePageTransition: true,
	        transition: 'fade',
	        reloadPage: true
	    });
		}
}

function showdata1(resptxt)
{
	var dataspace = document.getElementById('dataspace');
		var content = '<div data-role="main" class="ui-content" style="overflow:auto;">'+
		          '<table data-role="table" data-mode="columntoggle" class="ui-responsive ui-shadow" id="myTable">' +
	              '<thead>'+
	              '<tr><th colspan="5"> Price details of '+item+'</th></tr>'+
		          '<tr>'+
		          '<th data-priority="1">Date</th>'+
		          '<th data-priority="2" align="center">Market (State)</th>'+
		          '<th data-priority="3" align="center">Quantity (Tonnes)</th>'+
		          '<th data-priority="4" align="center">Mod. Price (Min - Max)</th>'+
		          '<th data-priority="5" align="center">Origin</th>'+
		          '</tr>'+
		          '</thead><tbody>';
		pcount = resptxt.count;
		if(pcount>0){
		var limit = (resptxt.count-((pageno-1)*ppage))<ppage? resptxt.count-((pageno-1)*ppage) : ppage;
		 for(i=0;i<limit;i++){
			 content+='<tr>';
			 content+='<td>'+ resptxt.data[i].date +'</td>';
			 content+='<td align="center">'+ resptxt.data[i].market +' ('+resptxt.data[i].state+')</td>';
			 content+='<td align="center">'+ resptxt.data[i].Arrivals_Tonnes +'</td>';
			 content+='<td align="center">'+ resptxt.data[i].Modal_Price + resptxt.data[i].unit.substring(0,6) + '(' + resptxt.data[i].Minimum_Price + ' - ' + resptxt.data[i].Maximum_Price +')</td>';
			 content+='<td align="center">'+ resptxt.data[i].origin +' </td>';
			 content+='</tr>';
		 }
		}
		else
			{
			content+='<tr><td colspan="5" align="center"> No Data available for the item on given date </td></tr>';
			}
		 content+='</tbody></table></div>';
		 if(pcount>0){
		 content+='<div data-role="footer"> <h6><center>';
		 content+='Page '+pageno+' of '+ Math.ceil(pcount/ppage) +'';
		 content+='</center></h6></div>';}
		 dataspace.innerHTML = content;
		 document.getElementById('footer1').style.display="none";
		 document.getElementById('footer2').style.display="block";
}

function openurl(url)
{
	var width = window.screen.width/3;
	var height = window.screen.height/3;
	var leftPosition, topPosition;
	leftPosition = (window.screen.width / 2) - ((width / 2) + 10);
    topPosition = (window.screen.height / 2) - ((height / 2) + 50);
	window.open(url, item, "status=no,height=" + height + ",width=" + width + ",resizable=yes,left=" + leftPosition + ",top=" + topPosition + ",screenX=" + leftPosition + ",screenY=" + topPosition + ",toolbar=no,menubar=no,scrollbars=yes,location=no,directories=no");
}

function showdata2(resptxt)
{
	var dataspace = document.getElementById('dataspacecom');
		var content = '<div data-role="main" class="ui-content" style="overflow:auto;">'+
		          '<table data-role="table" data-mode="columntoggle" class="ui-responsive ui-shadow" id="myTable">' +
	              '<thead>'+
	              '<tr><th colspan="5"> Price details of '+item+'</th></tr>'+
		          '<tr>'+
		          '<th data-priority="1">Title</th>'+
		          '<th data-priority="2" align="center">MRP</th>'+
		          '<th data-priority="3" align="center">Selling Price</th>'+
		          '<th data-priority="4" align="center">Source</th>'+
		          '<th data-priority="5" align="center">Category</th>'+
		          '</tr>'+
		          '</thead><tbody>';
		pcount = resptxt.count;
		if(pcount>0){
		var limit = (resptxt.count-((pageno-1)*ppage))<ppage? resptxt.count-((pageno-1)*ppage) : ppage;
		 for(i=0;i<limit;i++){
			 content+='<tr>';
			 content+='<td><a href="#" onclick="openurl(\''+resptxt.data[i].url+'\')">' + resptxt.data[i].title +'</a></td>';
			 content+='<td align="center">'+ resptxt.data[i].mrp +'</td>';
			 content+='<td align="center">'+ resptxt.data[i].available_price +'</td>';
			 content+='<td align="center">'+ resptxt.data[i].source + '</td>';
			 content+='<td align="center">'+ resptxt.data[i].category +' </td>';
			 content+='</tr>';
		 }
		}
		else
			{
			content+='<tr><td colspan="5" align="center"> No Data available for '+item+' on given date </td></tr>';
			}
		 content+='</tbody></table></div>';
		 if(pcount>0){
		 content+='<div data-role="footer"> <h6><center>';
		 content+='Page '+pageno+' of '+ Math.ceil(pcount/ppage) +'';
		 content+='</center></h6></div>';}
		 dataspace.innerHTML = content;
		 document.getElementById('footercom1').style.display="none";
		 document.getElementById('footercom2').style.display="block";

}

function ajax_request(request,id)
{
	$.ajax({
    	type: "POST",
    	   dataType: "json",
    	   contentType: "application/json; charset=utf-8",
    	   data: JSON.stringify({   
    	      ID: 2212,      
    	   }),
        url: request,
        beforeSend: function() {
            $.mobile.loading('show', {theme:"b", text:"Fetching data...", textonly:false, textVisible: true}); 
        },
        error: function(){
        	$.mobile.loading('hide');
        	alert("Please check your network connection!");
        },
        success: function(resptxt){
        	
        	if(id==1){
        		var div = document.getElementById('dataspace');
            	div.innerHTML = "";
        		showdata1(resptxt);}
        	else{
        		var div = document.getElementById('dataspacecom');
            	div.innerHTML = "";
        		showdata2(resptxt);
        	}
        },
        complete: function() {
            $.mobile.loading('hide'); 
        }
});	
}

function sendgetreq(id)
{
    var myArr;
    var request;
    if(id==1) request = "http://api.dataweave.in/v1/commodities/findByCommodity/?api_key="+api_key+"&commodity="+item+"&start_date="+sdate+"&end_date="+edate+"&page="+pageno+'&per_page='+ppage;
    else request = "http://api.dataweave.in/v1/price_intelligence/findProduct/?api_key="+api_key+"&product="+item+"&page="+pageno+'&per_page='+ppage;
    ajax_request(request,id);
}


/*
$(document).on('pagebeforeshow','#foo',function()
{
	$(document).on('click','#ecomlink',function()
	{
		$.mobile.changePage("#foo2",{transition:"none"});		
	});
});

$(document).on('pagebeforeshow','#foo2',function()
		{
			$(document).on('click','#marketlink',function()
			{
				$.mobile.changePage("#foo",{transition:"none"});		
			});
		});
*/