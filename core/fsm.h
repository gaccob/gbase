#ifndef FSM_H_
#define FSM_H_

#ifdef __cplusplus
extern "C" {
#endif

#define FSM_WILDCARD_STATUS -1

typedef void (*fsm_status_func_t) (void* args);
typedef int (*fsm_event_func_t) (void* args);

struct fsm_t;

#define FSM_OK 0
#define FSM_FAIL -1
#define FSM_EXISTED -2
#define FSM_NOT_EXISTED -3
#define FSM_FULL -4

struct fsm_t* fsm_create(int size);
void fsm_release(struct fsm_t* fsm);

// status > 0
int fsm_register_status(struct fsm_t* fsm, int status,
                        fsm_status_func_t enter, fsm_status_func_t exit);

// event > 0
int fsm_register_event(struct fsm_t* fsm, int event,
                       fsm_event_func_t handle);

// from status could be wildcard
// if from & to both wildcard, that means keep current status and ignore event
int fsm_register_rule(struct fsm_t* fsm, int event, int ret_code,
                      int from_status, int to_status);

// set started status
int fsm_start(struct fsm_t* fsm, int status);

// first look for exact rule
// then try wildcard rules
int fsm_trigger(struct fsm_t* fsm, int event, void* args);

#define FSM_STATUS(fsm, st, enter, exit) \
    do { \
        int _ret = fsm_register_status(fsm, st, enter, exit); \
        assert(_ret == FSM_OK); \
    } while (0)

#define FSM_EVENT(fsm, ev, handle) \
    do { \
        int _ret = fsm_register_event(fsm, ev, handle); \
        assert(_ret == FSM_OK); \
    } while (0)

#define FSM_RULE(fsm, event, ret, from, to) \
    do { \
        int _ret = fsm_register_rule(fsm, event, ret, from, to); \
        assert(_ret == FSM_OK); \
    } while (0)

#define FSM_TRIGGER(fsm, event, args) \
    do { \
        int _ret = fsm_trigger(fsm, event, args); \
        assert(_ret == FSM_OK); \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif
