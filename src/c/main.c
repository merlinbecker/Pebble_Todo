/**
* TODO LIST API APP
* @author MerlinBecker
* @created 2017-05-03
* @version 1.0
* @see: https://github.com/MarSoft/PebbleNotes
***/

#include <pebble.h>
#include "comm.h"
#include "statusbar.h"
#include "projectlist.h"
#include "task.h"

static void init(void) {
  comm_init();
  sb_init();
  pl_init();
  ts_init();
  pl_show();
}

static void deinit(void) {
  sb_deinit();
  ts_deinit();
  pl_deinit();
  
  comm_deinit();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}