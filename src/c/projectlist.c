#include <pebble.h>
#include "projectlist.h"

#include <pebble.h>
#include "projectlist.h"
#include "comm.h"
#include "task.h"
#include "statusbar.h"
#include "misc.h"

static Window *wndProjlists;
static MenuLayer *mlProjlists;

static int pl_count = -1; // how many items were loaded ATM
static int pl_max_count = -1; // how many items we are expecting (i.e. buffer size)
static PL_Item *pl_items = NULL; // buffer for items
static bool is_first_launch = true;

static void pl_draw_row_cb(GContext *ctx, const Layer *cell_layer, MenuIndex *idx, void *context) {
	char *title;
	if(pl_max_count == 0) // empty list
		title = "No tasklists! Please create one via the API";
	else if(idx->row >= pl_count) // no such item (yet?)
		title = "<...>";
	else if(pl_items[idx->row].projname)
		title = pl_items[idx->row].projname;
	else
		title = "<OOM>";
  menu_cell_title_draw(ctx, cell_layer, title);

}
static uint16_t pl_get_num_rows_cb(MenuLayer *ml, uint16_t section_index, void *context) {
	if(pl_count < 0) // not initialized
		return 0; // statusbar must already contain "Connecting..." message
	else if(pl_count == 0) // no data
		return 1;
	else
		return pl_count;
}
static void pl_select_click_cb(MenuLayer *ml, MenuIndex *idx, void *context) {
	assert(idx->row < pl_count, "Invalid index!"); // this will fire when there are no any lists loaded
	PL_Item sel = pl_items[idx->row];
	
  
  if(strcmp(sel.projname,ts_current_if_complete())==0 || comm_is_available()) // already loaded or may be loaded
		LOG("SHOWING TASKS FOR PROJECT %s",sel.projname);
    ts_show(sel.projname);
}

static void pl_window_load(Window *wnd) {
	Layer *wnd_layer = window_get_root_layer(wnd);
	GRect bounds = layer_get_bounds(wnd_layer);

	mlProjlists = menu_layer_create(bounds);
	assert_oom(mlProjlists, "OOM while creating menu layer");
	menu_layer_set_callbacks(mlProjlists, NULL, (MenuLayerCallbacks) {
		.draw_row = pl_draw_row_cb,
		.get_num_rows = pl_get_num_rows_cb,
		.select_click = pl_select_click_cb,
	});
	menu_layer_set_click_config_onto_window(mlProjlists, wnd);
	layer_add_child(wnd_layer, menu_layer_get_layer(mlProjlists));
}
static void pl_window_unload(Window *wnd) {
	menu_layer_destroy(mlProjlists);
}
static void pl_free_items() {
	for(int i=0; i<pl_count; i++)
		free(pl_items[i].projname);
	free(pl_items);
	pl_items = NULL;
}

/* Public functions */

void pl_init() {
	LOG("Used: %d, free: %d", heap_bytes_used(), heap_bytes_free());
	wndProjlists = window_create();
	assert_oom(wndProjlists, "OOM while creating window");
	//window_set_click_config_provider(wndTasklists, tl_click_config_provider);
	window_set_window_handlers(wndProjlists, (WindowHandlers) {
		.load = pl_window_load,
		.disappear = sb_window_disappear_cb,
		.unload = pl_window_unload,
	});
	LOG("TaskLists module initialized, window is %p", wndProjlists);
}
void pl_deinit() {
	window_destroy(wndProjlists);
	pl_free_items();
}
void pl_show() {
	window_stack_push(wndProjlists, true);
	sb_show("Starte Merliste...");
	if(pl_count < 0){
    
  }
	comm_get_projects();
}
bool pl_is_active() {
	return window_stack_get_top_window() == wndProjlists;
}
void pl_set_count(int count) {
	LOG("Setting count: %d", count);
	if(pl_items)
		pl_free_items();
	pl_items = malloc(sizeof(PL_Item)*count);
	pl_max_count = count;
	pl_count = 0;
	if(!pl_items) {
		APP_LOG(APP_LOG_LEVEL_ERROR, "OOM while allocating tasklists items");
		sb_show("OOM");
		pl_max_count = 0;
	}
}
void pl_set_item(int i, PL_Item data) {
	LOG("New item %d", i);
	assert(pl_max_count > 0, "Trying to set item while not initialized!");
	assert(pl_max_count > i, "Unexpected item index: %d, max count is %d", i, pl_max_count);
	
	pl_items[i].projname = malloc(strlen(data.projname)+1);
	if(pl_items[i].projname) {
		strcpy(pl_items[i].projname, data.projname);
	} else {
		assert_oom(false, "OOM while allocating tasklist item title");
		LOG("Used: %d, free: %d", heap_bytes_used(), heap_bytes_free());
	}
	pl_count++;
	menu_layer_reload_data(mlProjlists);
	LOG("Current count is %d", pl_count);

	if(is_first_launch) {
		if(pl_max_count == 1) {
			// there is just one list, and we received it -> open it!
			pl_select_click_cb(mlProjlists, &(MenuIndex){
				.section = 0,
				.row = 0,
			}, NULL);
		}
		is_first_launch = false;
	}
}
