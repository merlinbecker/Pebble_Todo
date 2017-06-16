#pragma once
#ifndef _PROJECTLIST_H
#define _PROJECTLIST_H

typedef struct {
	char* projname;
} PL_Item;

void pl_init();
void pl_deinit();
void pl_show();
bool pl_is_active();
void pl_set_count(int);
void pl_set_item(int, PL_Item);

#endif
