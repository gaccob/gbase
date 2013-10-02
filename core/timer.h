#ifndef TIMER_H_
#define TIMER_H_

//
// a timer manager relies on callback
//

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"

//  return < 0 means to erase timer
//  return >= 0 success, and if timer is continual, continue
//  should nerver call timer_unregister in callback
typedef int (*timer_callback)(void* args);
struct heaptimer_t;

struct heaptimer_t* timer_init();

void timer_release(struct heaptimer_t* timer);

//  interval==NULL means once
//  return registered timer id
int timer_register(struct heaptimer_t* timer, struct timeval* interval,
                   struct timeval* delay, timer_callback cb, void* args);

void timer_unregister(struct heaptimer_t* timer, int timer_id);

void timer_poll(struct heaptimer_t* timer, struct timeval* now);

#ifdef __cplusplus
}
#endif

#endif // TIMER_H_

