#ifndef TIMER_H_
#define TIMER_H_

// a timer manager relies on callback

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"

#define TIMER_INVALID_ID -1

//  return < 0 means to erase timer
//  return >= 0 success, and if timer is continual, continue
//  should nerver call timer_unregister in callback
typedef int (*timer_callback)(void* args);
typedef struct timeval tv_t;
typedef struct timerheap_t timerheap_t;

timerheap_t* timer_create_heap();
void timer_release(timerheap_t*);

//  interval==NULL means once
//  return registered timer id
//  if fail, return TIMER_INVALID_ID
int timer_register(timerheap_t*,
                   tv_t* interval,
                   tv_t* delay,
                   timer_callback cb,
                   void* args);

void timer_unregister(timerheap_t*, int timerid);

void timer_poll(timerheap_t*, tv_t* now);

#ifdef __cplusplus
}
#endif

#endif // TIMER_H_

