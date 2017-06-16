#include <pebble.h>
#include "task.h"
#include "comm.h"
#include "misc.h"
#include "statusbar.h" 
#include "consts.h"

#define CUSTOM_FONT "RESOURCE_ID_GOTHIC_18_BOLD"
#define ICON_SPACES 5
#define ITEM_RECT GRect(0,0,144,44)
#define ICON_START GPoint(0,3)

static Window *wndTasks;
static MenuLayer *mlTasks;
static GBitmap *bmpTasks[2];
static GFont menuFont;

static char*listId="!?!";
static int ts_count=-1;
static int ts_max_count=-1;
static Task_Item *ts_items=NULL;

#ifdef PBL_MICROPHONE
static DictationSession *session;

static void ts_create_task_cb(DictationSession *session, DictationSessionStatus status, char *transcription, void *ctx){
  if(status!=DictationSessionStatusSuccess){
    LOG("Dictation session failed with status %d",status);
    dictation_session_destroy(session);
    return;
  }
  comm_create_task(listId,transcription);
  dictation_session_destroy(session);
}

static void ts_create_task(){
  session=dictation_session_create(0,ts_create_task_cb,NULL);
  assert_oom(session,"Could not create dication session");
  dictation_session_enable_confirmation(session,true);
  dictation_session_enable_error_dialogs(session,true);
  dictation_session_start(session);
}
#endif

static uint16_t ts_get_num_sections_cb(MenuLayer *ml,void *context){
  #ifdef PBL_MICROPHONE
    if(ts_count>0&&ts_count==ts_max_count)return 2;
  #endif
  return 1; 
}

static uint16_t ts_get_num_rows_cb(MenuLayer *ml, uint16_t section_index,void *context){
  #ifdef PBL_MICROPHONE
  int act_section=1;
  if(section_index==act_section && ts_count >0 && ts_count == ts_max_count) return 1;
  #endif
  
  if(ts_count<0)
    return 1;
  else if(ts_count==0)
    return 1;
  else if(!ts_items)
    return 1;
  else 
    return ts_count;
}

static int16_t ts_get_header_height_cb(MenuLayer *ml,uint16_t section, void *context){
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void ts_draw_header_cb(GContext *ctx,const Layer *cell_layer,uint16_t section,void *context){
  char *header;
#ifdef PBL_MICROPHONE
  if(section==1 && ts_count>0 &&ts_count==ts_max_count)
    header ="Aktionen";
  else
#endif
    header=listId;
menu_cell_basic_header_draw(ctx,cell_layer,header);
}

static void ts_twoline_cell_draw(GContext *ctx, const Layer *layer, char *title, GBitmap *icon,char *status){
  char *buf=NULL;
  if(icon){
    buf=malloc(strlen(title)+ ICON_SPACES +1);
    assert_oom(buf,"OOM while allocating draw buffer!");
    if(!buf)
        LOG("Used: %d, free: %d", heap_bytes_used(),heap_bytes_free());
    memset(buf, ' ', ICON_SPACES);
    strcpy(buf+ICON_SPACES,title);
  }else{
    buf=title;
  }
  graphics_context_set_text_color(ctx,menu_cell_layer_is_highlighted(layer)? GColorWhite: GColorBlack);
  graphics_draw_text(ctx,buf,menuFont,ITEM_RECT,GTextOverflowModeFill, GTextAlignmentLeft,NULL);
  if(icon){
    graphics_draw_bitmap_in_rect(ctx,icon,(GRect){.origin=ICON_START,.size=gbitmap_get_bounds(icon).size});
    free(buf);
  }
}
static void ts_draw_row_cb(GContext *ctx, const Layer *cell_layer, MenuIndex *idx,void *context){
#ifdef PBL_MICROPHONE
  if(idx->section==1&&ts_count>0&&ts_count==ts_max_count){
    menu_cell_basic_draw(ctx,cell_layer,"Task erstellen",NULL,NULL);
    return;
  }
  #endif
  char *title;
  GBitmap *icon=NULL;
  char* status="todo";
  if(ts_max_count==0){
    title="No tasks in this list!";
  }
  else if(idx->row >=ts_count)
    title="<...>";
  else if(ts_max_count==1&&ts_items[idx->row].description[0]=='\0')
    title="<empty>";
  else if(!ts_items)
    title="<OOM>";
  else{
    title=ts_items[idx->row].description;
    if(!title)
      title="<OOM>";
    LOG("!!!STATUS!! %s",ts_items[idx->row].status);
    if(strcmp(ts_items[idx->row].status,"todo")==0){
      LOG("DAS WIORD GEPICKT!");
      icon=bmpTasks[0];
    }else icon=bmpTasks[1];
    status=ts_items[idx->row].status;
  }
  ts_twoline_cell_draw(ctx,cell_layer,title,icon,status);
}
static void ts_select_click_cb(MenuLayer *ml,MenuIndex *idx,void *context){
  #ifdef PBL_MICROPHONE
  if(idx->section==1){
    ts_create_task();
    return;
  }
  #endif
  if(ts_max_count==0||idx->row==ts_count)return;
  Task_Item task=ts_items[idx->row];
  char *newstat;
  if(strcmp(task.status,"todo")==0)newstat="have-done";
  else newstat="todo";
  comm_update_task_status(listId, task.id,newstat);
}

static void ts_select_long_click_cb(MenuLayer *ml, MenuIndex *idx, void *context){
 return; //do nothing here
  /*
  #ifdef PBL_MICROPHONE
    if(idx->section==1)return;
  #endif
  if(ts_max_count==0 || idx->row>ts_count) return;
  //if(heap_bytes_free()<OOM_MIN)
  */
}

static void ts_window_load(Window *wnd){
  Layer *wnd_layer = window_get_root_layer(wnd);
  GRect bounds=layer_get_bounds(wnd_layer);
  
  mlTasks=menu_layer_create(bounds);
  assert_oom(mlTasks, "OOM while creating menu layer");
  menu_layer_set_callbacks(mlTasks,NULL,(MenuLayerCallbacks){
    .get_num_sections=ts_get_num_sections_cb,
    .get_num_rows=ts_get_num_rows_cb,
    .get_header_height=ts_get_header_height_cb,
    .draw_header=ts_draw_header_cb,
    .draw_row=ts_draw_row_cb,
    .select_click=ts_select_click_cb,
    .select_long_click=ts_select_long_click_cb,
  });
  menu_layer_set_click_config_onto_window(mlTasks,wnd);
  layer_add_child(wnd_layer,menu_layer_get_layer(mlTasks));
}

static void ts_window_unload(Window *wnd){
  menu_layer_destroy(mlTasks);
}

static void ts_free_items(){
//  LOG("Used: %d, free: %d",heap_bytes_used(),heap_bytes_free());
  LOG("Freeing items");
  for(int i=0;i<ts_count;i++){
    free(ts_items[i].description);
  }
  free(ts_items);
  ts_items=NULL;
// LOG("Used: %d, free: %d",heap_bytes_used(),heap_bytes,free());
}

void ts_init(){
  wndTasks=window_create();
  assert_oom(wndTasks,"OOM while creating task window");
  window_set_window_handlers(wndTasks,(WindowHandlers){
    .load=ts_window_load,
    .disappear=sb_window_disappear_cb,
    .unload=ts_window_unload,
  });
  LOG("Tasks module init- loading resources...");
  bmpTasks[0]=gbitmap_create_with_resource(RESOURCE_ID_TASK_TODO);
  bmpTasks[1]=gbitmap_create_with_resource(RESOURCE_ID_TASK_DONE);
  menuFont=fonts_get_system_font(CUSTOM_FONT);
  LOG("Tasks module initialized,window is %p",wndTasks);
}

void ts_deinit(){
  window_destroy(wndTasks);
  ts_free_items();
  gbitmap_destroy(bmpTasks[0]);
  gbitmap_destroy(bmpTasks[1]);
}

void ts_show(char* proj){
  LOG("Showing tasks for list %s",proj);
  if(strcmp(proj,listId)>0){
    
  if(ts_items){
      ts_free_items();
    }
    ts_count =-1;
    ts_max_count=-1;
  }else{
    LOG("SETTING INDEX!");
    menu_layer_set_selected_index(mlTasks,MenuIndex(1,0),MenuRowAlignTop,false);
  }
  listId=proj;
  LOG("PUSHING STACK");
  window_stack_push(wndTasks,true);
  LOG("FETCHING TASKS!");
  if(ts_count<0)comm_get_tasks(proj);
}

bool ts_is_active(){
  return window_stack_get_top_window()==wndTasks;
}

char* ts_current_listId(){
  return listId;
}

char* ts_current_if_complete(){
  if(strcmp(listId,"!?!")>0&&ts_count>0&&!ts_items){
    return "!?!";
  }
  return listId;
}

void ts_set_count(int count){
  LOG("Setting count: %d",count);
  if(ts_items)ts_free_items();
  ts_items=malloc(sizeof(Task_Item)*count);
  ts_count=0;
  ts_max_count=count;
  if(count>0&&!ts_items){
    APP_LOG(APP_LOG_LEVEL_ERROR,"OOM while allocating items!");
    ts_max_count=0;
  }
}

void ts_set_item(int i, Task_Item data){
  LOG("new item %d",i);
  assert(ts_max_count >0,"Trying to set irem while not initialized");
  assert(ts_max_count>0,"Unexpected item index: %d, max count is %d",i,ts_max_count);
  ts_items[i].id=data.id;
  ts_items[i].status=data.status;
  int tlen=strlen(data.description);
  if(heap_bytes_free()-tlen>OOM_SAFEGUARD){
    ts_items[i].description=malloc(tlen+1);
    if(ts_items[i].description){
      strcpy(ts_items[i].description,data.description);
    }else{
      APP_LOG(APP_LOG_LEVEL_ERROR,"OOM while allocating description!");
      sb_show("OOM");
    }
  }
  else{
      ts_items[i].description=NULL;
  }
  LOG("RESULTING description : %s",ts_items[i].description);
  ts_count++;
  menu_layer_reload_data(mlTasks);
  LOG("current count is %d",ts_count);
  if(ts_count==ts_max_count){
    menu_layer_set_selected_index(mlTasks,MenuIndex(1,0),MenuRowAlignTop,false);
  }
  }

void ts_append_item(Task_Item data){
  LOG("Additional item with id %d",data.id);
  assert(ts_max_count >= 0, "Trying to append item while not initialized!");
	assert(ts_max_count == ts_count, "Trying to add task while not fully loaded!");
	assert_oom(heap_bytes_free() > OOM_SAFEGUARD, "Almost OOM - ignoring item!");
	ts_count++;
	ts_max_count++;
	// increase array memory
  ts_items = realloc(ts_items, sizeof(Task_Item)*ts_count);
  int i = ts_max_count-1; // last task
  ts_items[i].id = data.id;
	ts_items[i].status = data.status;
	ts_items[i].description = malloc(strlen(data.description)+1);
  if(ts_items[i].description)
		strcpy(ts_items[i].description, data.description);
	else
		APP_LOG(APP_LOG_LEVEL_ERROR, "OOM while allocating title");
  menu_layer_reload_data(mlTasks);
	LOG("Task appended, new count is %d", ts_count);
}

void ts_update_item_state_by_id(int id, char* status) {
	LOG("Updating state for itemId %d", id);
	for(int i=0; i<ts_count; i++) {
		if(ts_items[i].id == id) {
      
			assert(strcmp(ts_items[i].status,status)>0, "Tried to update with the old state");
			ts_items[i].status = status;
			if(ts_is_active()) {
				menu_layer_reload_data(mlTasks);
			} 
			return;
		}
	}
	APP_LOG(APP_LOG_LEVEL_ERROR, "NOTICE: Couldn't find desired item ID %d to update", id);
}
