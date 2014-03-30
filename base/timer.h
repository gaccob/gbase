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
struct timer_t;

struct timer_t* timer_create();

void timer_release(struct timer_t* timer);

//  interval==NULL means once
//  return registered timer id
//  if fail, return TIMER_INVALID_ID
int timer_register(struct timer_t* timer, struct timeval* interval,
                   struct timeval* delay, timer_callback cb, void* args);

void timer_unregister(struct timer_t* timer, int timer_id);

void timer_poll(struct timer_t* timer, struct timeval* now);

#ifdef __cplusplus
}
#endif

#endif // TIMER_H_

