#pragma once
#ifndef _TASKS_H
#define _TASKS_H

typedef struct{
  int id;
  char* status;
  char* description;
}Task_Item;

void ts_init();
void ts_deinit();

void ts_show(char*);
bool ts_is_active();
char* ts_current_listId();
char* ts_current_if_complete();
void ts_set_count(int);
void ts_set_item(int, Task_Item);
void ts_append_item(Task_Item);
void ts_update_item_state_by_id(int,char*);

#endif