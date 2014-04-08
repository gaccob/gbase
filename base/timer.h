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
struct timerheap_t;

struct timerheap_t* timer_create_heap();

void timer_release(struct timerheap_t* timer);

//  interval==NULL means once
//  return registered timer id
//  if fail, return TIMER_INVALID_ID
int timer_register(struct timerheap_t* timer, struct timeval* interval,
                    struct timeval* delay, timer_callback cb, void* args);

void timer_unregister(struct timerheap_t* timer, int timerid);

void timer_poll(struct timerheap_t* timer, struct timeval* now);

#ifdef __cplusplus
}
#endif

#endif // TIMER_H_

