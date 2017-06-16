#pragma once
#ifndef _COMM_H
#define _COMM_H

void comm_get_projects();
void comm_get_tasks(char *);

void comm_get_task_details(char*,int);
void comm_update_task_status(char*,int,char*);
void comm_create_task(char*,char*);

typedef void(* CommJSReadyCallback)(void *data);

void comm_on_js_ready(CommJSReadyCallback *,void*);


bool comm_is_available();
bool comm_is_available_silent();

void comm_init();
void comm_deinit();


#endif