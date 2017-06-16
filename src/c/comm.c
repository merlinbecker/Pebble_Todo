#include <pebble.h>
#include "comm.h"
#include "misc.h"
#include "task.h"
#include "consts.h"
#include "statusbar.h"
#include "projectlist.h"


static bool comm_js_ready=false;
static CommJSReadyCallback comm_js_ready_cb;
static void *comm_js_ready_cb_data;
static int comm_array_size=-1;


static bool comm_is_bluetooth_available(){
  if(!bluetooth_connection_service_peek()){
    sb_show("No bluetooth connection!");
    return false;
  }
  return true;
}
bool comm_is_available(){
  if(!comm_is_bluetooth_available())return false;
  if(!comm_js_ready){
    sb_show("JS not available, please try again later!");
    return false;
  }
  return true;
}

bool comm_is_availables_silent(){
  return bluetooth_connection_service_peek() && comm_js_ready;
}

void comm_get_projects_cb(void *arg){
  comm_get_projects();
}

void comm_get_projects(){
  if(!comm_js_ready){
    comm_js_ready_cb=comm_get_projects_cb;
    sb_show("Waiting for JS...");
    comm_is_bluetooth_available();
    return;
  }
  if(!comm_is_available()) return;
  sb_show("Connecting...");
  LOG("FETCHING PROJECTS");
  
  DictionaryIterator *iter;
  Tuplet code=TupletInteger(MESSAGE_KEY_MESSAGECODE,GET);
  Tuplet scope=TupletInteger(MESSAGE_KEY_SCOPE,PROJECT);
  
  app_message_outbox_begin(&iter);
  dict_write_tuplet(iter,&code);
  dict_write_tuplet(iter,&scope);
  app_message_outbox_send();
}


/**
hhier kommt noch mehr!
*/
void comm_get_tasks_cb(void* arg){
  comm_get_tasks((char*) arg);
}

void comm_get_tasks(char* projectname){
  if(!comm_js_ready){
    comm_js_ready_cb=comm_get_tasks_cb;
    comm_js_ready_cb_data=(void *)projectname;
    comm_is_available();
    return;
  }
  if(!comm_is_available())
    return;
  sb_show("Connecting....");
  LOG("Queryring tasks for projectname %s",projectname);
  DictionaryIterator *iter;
  Tuplet code=TupletInteger(MESSAGE_KEY_MESSAGECODE,GET);
  Tuplet scope =TupletInteger(MESSAGE_KEY_SCOPE,TASK);
  Tuplet proj=TupletCString(MESSAGE_KEY_PROJECTNAME,projectname);
  
  app_message_outbox_begin(&iter);
  dict_write_tuplet(iter,&code);
  dict_write_tuplet(iter,&scope);
  dict_write_tuplet(iter,&proj);
  app_message_outbox_send();
}

void comm_query_task_details(char* projname, int taskId) {
	LOG("Querying task details for %d (not implemented)", taskId);
}

void comm_update_task_status(char* projname,int taskId, char* newStatus){
  if(!comm_is_available()){
    return;
  }
  LOG(strcat("UPDATE status for task %d in project ",projname),taskId);
  sb_show("UPDATING...");
  DictionaryIterator *iter;
  Tuplet code=TupletInteger(MESSAGE_KEY_MESSAGECODE,PUT);
  Tuplet scope=TupletInteger(MESSAGE_KEY_SCOPE,TASK);
  Tuplet proj=TupletCString(MESSAGE_KEY_PROJECTNAME,projname);
  Tuplet task=TupletInteger(MESSAGE_KEY_TASKID,taskId);
  Tuplet status=TupletCString(MESSAGE_KEY_STATUS,newStatus);
  
  app_message_outbox_begin(&iter);
  dict_write_tuplet(iter,&code);
  dict_write_tuplet(iter,&scope);
  dict_write_tuplet(iter,&proj);
  dict_write_tuplet(iter,&task);
  dict_write_tuplet(iter,&status);
  app_message_outbox_send();
}

void comm_create_task(char *projname,char *title){
  if(!comm_is_available()){
    return;
  }
  LOG("Creating new task with title %s ind project %s",title,projname);
  sb_show("Creating...");
  DictionaryIterator *iter;
  Tuplet code=TupletInteger(MESSAGE_KEY_MESSAGECODE,POST);
  Tuplet scope=TupletInteger(MESSAGE_KEY_SCOPE,TASK);
  Tuplet proj=TupletCString(MESSAGE_KEY_PROJECTNAME,projname);
  Tuplet tTitle=TupletCString(MESSAGE_KEY_DESCRIPTION,title);
  app_message_outbox_begin(&iter);
  dict_write_tuplet(iter,&code);
  dict_write_tuplet(iter,&scope);
  dict_write_tuplet(iter,&proj);
  dict_write_tuplet(iter,&tTitle);
  
  app_message_outbox_send();
}

static void comm_in_received_handler(DictionaryIterator *iter, void *context){
  Tuple *tCode,*tMessage,*tScope;
  
  LOG("Used: %d , free: %d ",(int)heap_bytes_used(),(int)heap_bytes_free());
  
  tCode=dict_find(iter,MESSAGE_KEY_MESSAGECODE);
  assert(tCode,"Message without code");
  int code=(int)tCode->value->int32;
  LOG("Message code: %d",code);
  
  if(code==JS_SUCCESS){ //JS is loaded
    comm_js_ready=true;
    if(comm_js_ready_cb){
      LOG("JS READY Callback awaiting, calling");
      comm_js_ready_cb(comm_js_ready_cb_data);
    }
    return;
  }
  
  tScope=dict_find(iter,MESSAGE_KEY_SCOPE);
  assert(tScope,"No scope!");
  int scope =(int)tScope->value->int32;
  LOG("Message scope: %d",scope);
  
  if(scope==PROJECT){
   //do nothing!
  }
  else if(scope==TASK){
    assert(ts_is_active(), "Ignoring Tasks-related message because tasks is inactive");
  }
  else{
    return;
  }
  
  if(code==ARRAY_START){
    int count=(int)dict_find(iter,MESSAGE_KEY_COUNT)->value->int32;
    LOG("Items count : %d",count);
    comm_array_size=count;
    if(scope==PROJECT){
      pl_set_count(count);
    }
    else if(scope==TASK){
      ts_set_count(count);
    }
    else LOG("ERR!");
    snprintf(sb_printf_alloc(32),32,"Loading...");
    sb_printf_update();
  }
  else if(code==ARRAY_ITEM){
    assert(comm_array_size>0,"Unexpected array item!");
    int i=(int)dict_find(iter,MESSAGE_KEY_ITEMNUM)->value->int32;;
      assert(i<comm_array_size,"Index %d exceeds size %d",i,comm_array_size);
      snprintf(sb_printf_get(),32,"Loading... %d%%",100*(i+1)/comm_array_size);
      sb_printf_update();
    LOG("Statusbar Updated: %d",100*(i+1)/comm_array_size);
    if(scope==PROJECT){
      char *title=dict_find(iter,MESSAGE_KEY_PROJECTNAME)->value->cstring;
      pl_set_item(i,(PL_Item){
        .projname=title
      });
    }else{
      int taskId = (int)dict_find(iter, MESSAGE_KEY_TASKID)->value->int32;
			char *status = (char*)dict_find(iter,MESSAGE_KEY_STATUS)->value->cstring;
      char *desc=(char*)dict_find(iter,MESSAGE_KEY_DESCRIPTION)->value->cstring;
			LOG("Item No: %d, Id=%d, status=%s", i, taskId, status);
			ts_set_item(i, (Task_Item){
				.id = taskId,
				.status = status,
				.description = desc
			});
    }
    
    
  }
  else if(code==ARRAY_END){
    comm_array_size=-1;
    sb_hide();
  }
  else{
    LOG("Unexpected message code: %d",code);
  }
}

static void comm_in_dropped_handler(AppMessageResult reason, void *context){
  LOG("Message dropped: reason=%d",reason);
}

static void comm_out_sent_handler(DictionaryIterator *sent,void *context){
  LOG("Message sent");
}

static void comm_out_failed_handler(DictionaryIterator *failed,AppMessageResult reason,void *context){
  LOG("Message send failed: reason=%d",reason);
}

void comm_init(){
  app_message_register_inbox_received(comm_in_received_handler);
  app_message_register_inbox_dropped(comm_in_dropped_handler);
  app_message_register_outbox_sent(comm_out_sent_handler);
  app_message_register_outbox_failed(comm_out_failed_handler);
  
  app_message_open(MIN(app_message_inbox_size_maximum(),OUTBOX_DESIRED_MAX),APP_MESSAGE_INBOX_SIZE_MINIMUM);
}
void comm_deinit(){
  app_message_deregister_callbacks();
}