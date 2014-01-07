#ifndef FSM_H_
#define FSM_H_

#ifdef __cplusplus
extern "C" {
#endif

#define FSM_NAME_LEN 32

#define FSM_INVALID_STATUS -1

typedef void (*fsm_status_func_t) (void* args);

typedef struct fsm_status_t {
    int id;

    // name for debug
    char name[FSM_NAME_LEN];

    // enter & exit will not happend when status enter itself
    fsm_status_func_t enter;
    fsm_status_func_t exit;
} fsm_status_t;

typedef int (*fsm_event_func_t) (void* args);

typedef struct fsm_event_t {
    int id;

    // name for debug
    char name[FSM_NAME_LEN];

    int from_status;
    int to_status;
    fsm_event_func_t handle;
} fsm_event_t;

typedef struct fsm_t {
    int status_size;
    int status_count;
    fsm_status_t* status;
    struct idtable_t* status_table;

    int events_size;
    int events_count;
    fsm_event_t* events;
    struct idtable_t* events_table;
} fsm_t;

#define FSM_OK 0
#define FSM_FAIL -1
#define FSM_EXISTED -2
#define FSM_NOT_EXISTED -3
#define FSM_FULL -4

struct fsm_t* fsm_init(int status_size, int events_size);

// status > 0
int fsm_register_status(struct fsm_t* fsm, int status, const char* name,
                        fsm_status_func_t enter, fsm_status_func_t exit);

// event > 0
int fsm_register_event(struct fsm_t* fsm, int event, const char* name,
                       int from_status, int to_status, fsm_event_func_t handle);

int fsm_trigger(struct fsm_t* fsm, int event, void* args)
{
    if (!fsm) {
        return FSM_FAIL;
    }

    fsm_event_t* et = (fsm_event_t*)idtable_get(fsm->events_table, event);
    if (!et) {
        return FSM_NOT_EXISTED;
    }


}

#ifdef __cplusplus
}
#endif

#endif
