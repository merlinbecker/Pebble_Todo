//@author merlinbecker
//@created 2017/04/27
//configured following the instructions under:
//https://github.com/pebble/clay
//message sending is used from here:
//https://github.com/MarSoft/PebbleNotes

//codes: app_ready:1

var Clay = require('pebble-clay');
var clayConfig = require('./config');
var clay = new Clay(clayConfig);

var settings = JSON.parse(localStorage.getItem('clay-settings'));
var API_URL=settings.apiUrl;
var API_USERNAME=settings.username;
var API_PASSWORD=settings.password;

Pebble.addEventListener('ready', function(e) {
  sendMessage({'MESSAGECODE': 1});
});

Pebble.addEventListener('appmessage', function(dict) {
  console.log("Nachricht erhalten!!");
  console.log(dict.payload.MESSAGECODE);
  //form the authentication header
  var messagecode=dict.payload.MESSAGECODE;
  var scope=dict.payload.SCOPE;
  //get projects
  var request = new XMLHttpRequest();
  var the_url=API_URL+API_USERNAME; 
  var data="";
  if(messagecode==2&&scope===0){
    the_url+="/projects";
    request.open("GET",the_url);
  }
  //get tasks of specific project
  else if(messagecode==2&&scope==1){
    var projname=dict.payload.PROJECTNAME;
    the_url+="/projects/"+projname;
    request.open("GET",the_url);
  }
  //PUT a task status
  else if(messagecode==3&&scope==1){
     the_url+="/tasks";
     request.open("PUT", the_url);
    request.setRequestHeader("Content-Type", "application/json");
   /* data=JSON.stringify
xmlhttp.send(JSON.stringify({name:"John Rambo", time:"2pm"}));
  TODO: HIER WEITER!!
  
  */
}
  request.setRequestHeader("Authorization", "Basic " + btoa(API_USERNAME+":"+API_PASSWORD));
  request.addEventListener('load', function(event) {
   if (request.status >= 200 && request.status < 300) {
     var data=JSON.parse(request.responseText);
     var i=0;
     //projects received
     if(messagecode==2&&scope===0){
       sendMessage({
         'MESSAGECODE':20,
         'SCOPE':0,
         'COUNT':data.length
        });
       for(i=0;i<data.length;i++){
         sendMessage(
         {
           "MESSAGECODE":21,
           "SCOPE":0,
           "ITEMNUM":i,
           "PROJECTNAME":data[i].project_name
         });
       }
       sendMessage({
         "MESSAGECODE":22,
         "SCOPE":0,
         "COUNT":data.length
       });
       console.log("SENDING FINISHED!");
     }
     //tasks received
     if(messagecode==2&&scope===1){
       sendMessage({
         'MESSAGECODE':20,
         'SCOPE':1,
         'COUNT':data.length
        });
       for(i=0;i<data.length;i++){
         sendMessage(
         {
           "MESSAGECODE":21,
           "SCOPE":1,
           "ITEMNUM":i,
           //"PROJECTNAME":data[i].project_name
           "TASKID":data[i].task_id,
           "STATUS":data[i].status,
           "DESCRIPTION":data[i].description
         });
       }
       sendMessage({
         "MESSAGECODE":22,
         "SCOPE":1,
         "COUNT":data.length
       });
       console.log("SENDING FINISHED!");
     }
     console.log("RESPONSE TEST LAENGE "+data.length);
     console.log(data);
   } else {
      console.warn(request.statusText, request.responseText);
   }
});
request.send(); 
});


var g_msg_buffer = [];
var g_msg_transaction = null;
var g_msg_timeout = 8000;

function sendMessage(data, success, failure) {
	function sendNext() {
		g_msg_transaction = null;
		var next = g_msg_buffer.shift();
		if(next) {
			sendMessage(next);
		}
	}
	if(g_msg_transaction) { 
		g_msg_buffer.push(data);
	} else { // free
		g_msg_transaction = Pebble.sendAppMessage(data,
			function(e) {
				console.log("Message sent for transactionId=" + e.data.transactionId);
				window.clearTimeout(msgTimeout);
				if(g_msg_transaction >= 0 && g_msg_transaction != e.data.transactionId) // -1 if unsupported
					console.log("### Confused! Message sent which is not a current message. "+
							"Current="+g_msg_transaction+", sent="+e.data.transactionId);
				if(success)
					success();
				sendNext();
			},
		   	function(e) {
				console.log("Failed to send message for transactionId=" + e.data.transactionId +
						", error is "+(e.error && "message" in e.error ? e.error.message : "(none)"));
				window.clearTimeout(msgTimeout);
				if(g_msg_transaction >= 0 && g_msg_transaction != e.data.transactionId)
					console.log("### Confused! Message not sent, but it is not a current message. "+
							"Current="+g_msg_transaction+", unsent="+e.data.transactionId);
				if(failure === true) {
					if(success)
						success();
				} else if(failure)
					failure();
				sendNext();
			}
		);
		if(!g_msg_transaction) { // buggy sendAppMessage: on iOS returns undefined, on emulator returns null
			g_msg_transaction = -1; // just a dummy "non-false" value for sendNext and friends
		}
		
    var msgTimeout = setTimeout(function() {
			console.log("Message timeout! Sending next.");
			// FIXME: it could be really delivered. Maybe add special handler?
			if(failure === true) {
				if(success)
					success();
			} else if(failure) {
				failure();
			}
			sendNext();
		}, g_msg_timeout);
		console.log("transactionId="+g_msg_transaction+" for msg "+JSON.stringify(data));
	}
}