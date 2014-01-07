#include "fsm.h"

fsm_t* fsm_init(int status_size, int events_size)
{
    fsm_t* fsm;
    if (status_size <= 0 || events_size <= 0) {
        return NULL;
    }

    fsm = (fsm_t*)MALLOC(sizeof(fsm_t));
    assert(fsm);

    fsm->status_size = status_size;
    fsm->status_count = 0;
    fsm->status = (fsm_status_t*)MALLOC(sizeof(fsm_status_t) * fsm->status_size);
    fsm->status_table = idtable_init(fsm->status_size);
    assert(fsm->status && fsm->status_table);

    fsm->events_size = events_size;
    fsm->events_count = 0;
    fsm->events = (fsm_event_t*)MALLOC(sizeof(fsm_event_t) * fsm->events_size);
    fsm->events_table = idtable_init(fsm->events_size);
    assert(fsm->events && fsm->events_table);

    return fsm;
}

int fsm_register_status(fsm_t* fsm, int status, const char* name,
                        fsm_status_func_t enter, fsm_status_func_t exit)
{
    fsm_status_t* st;
    int ret;

    if (!fsm || !name) {
        return FSM_FAIL;
    }
    if (fsm->status_count >= fsm->status_size) {
        return FSM_FULL;
    }
    if (idtabel_get(fsm->status_table, status)) {
        return FSM_EXISTED;
    }

    st = &fsm->status[fsm->status_count ++];
    st->id = status;
    snprintf(st->name, sizeof(st->name), "%s", name);
    st->enter = enter;
    st->quit = quit;
    ret = idtable_add(st->status_table, st->id, (void*)st);
    assert(ret == 0);

    return FSM_OK;
}

int fsm_register_event(fsm_t* fsm, int event, const char* name,
                       int from_status, int to_status, fsm_event_func_t handle)
{
    fsm_event_t* et;
    int ret;

    if (!fsm || !name) {
        return FSM_FAIL;
    }
    if (fsm->events_count >= fsm->events_size) {
        return FSM_FULL;
    }
    if (idtabel_get(fsm->events_table, event)) {
        return FSM_EXISTED;
    }

    et = &fsm->events[fsm->events_count ++];
    et->id = event;
    snprintf(et->name, sizeof(et->name), "%s", name);
    et->from_status = from_status;
    et->to_status = to_status;
    et->handle = handle;
    ret = idtable_add(st->events_table, et->id, (void*)et);
    assert(ret == 0);

    return FSM_OK;
}


