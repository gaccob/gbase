#include "fsm.h"

#define FSM_STATUS_SIZE sizeof(fsm_status_t)
#define FSM_RULES_SIZE(nstatus) ((nstatus * (nstatus - 1)) >> 1)
#define FSM_EVENT_SIZE(nstatus) (sizeof(fsm_event_t) \
    + (sizeof(fsm_rule_t*) * FSM_RULES_SIZE(nstatus))

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
    fsm->status = (fsm_status_t*)MALLOC(FSM_STATUS_SIZE * fsm->status_size);
    fsm->status_table = idtable_init(fsm->status_size);
    assert(fsm->status && fsm->status_table);

    fsm->events_size = events_size;
    fsm->events_count = 0;
    fsm->events = (fsm_event_t*)MALLOC(FSM_EVENT_SIZE(status_size) * fsm->events_size);
    fsm->events_table = idtable_init(fsm->events_size);
    assert(fsm->events && fsm->events_table);

    fsm->current = FSM_INVALID_STATUS;
    return fsm;
}

int fsm_register_status(fsm_t* fsm, int status, const char* name,
                        fsm_status_func_t enter, fsm_status_func_t exit)
{
    fsm_status_t* st;
    int ret;

    if (!fsm || !name || status <= 0) {
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
    fsm_rule_t* rule;
    int ret;

    if (!fsm || !name || event <= 0) {
        return FSM_FAIL;
    }
    if (fsm->events_count >= fsm->events_size) {
        return FSM_FULL;
    }

    et = (fsm_event_t*)idtabel_get(fsm->events_table, event);
    if (!et) { 
        et = &fsm->events[fsm->events_count ++];
        et->id = event;
        et->rules_count = 0;
        snprintf(et->name, sizeof(et->name), "%s", name);
        ret = idtable_add(st->events_table, et->id, (void*)et);
        assert(ret == 0);
    }

    if (et->rules_count >= FSM_RULES_SIZE(fsm->status_size)) {
        return FSM_FULL;
    }
    et->rules[et->rules_count] = (fsm_rule_t*)MALLOC(sizeof(fsm_rule_t));
    rule = et->rules[et->rules_count ++];
    rule->from_status = from_status;
    rule->to_status = to_status;
    rule->handle = handle;
    return FSM_OK;
}


