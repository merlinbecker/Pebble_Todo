#include <pebble.h>
#ifndef _CONSTS_H
#define _CONSTS_H

#define OUTBOX_DESIRED_MAX 1024
#define OOM_SAFEGUARD 768
typedef enum {
  JS_SUCCESS=1,
  GET=2,
  PUT=3,//update
  POST=4,//create
  ARRAY_START = 20,
  ARRAY_ITEM=21,
	ARRAY_END = 22
} MessageCodes;

typedef enum{
  PROJECT=0,
  TASK=1
}MessageScopes;


#endif