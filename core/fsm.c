#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include "base/hash.h"
#include "base/idtable.h"
#include "fsm.h"

typedef struct fsm_status_t {
    int id;
    // enter & exit will not happend when status enter itself
    fsm_status_func_t enter;
    fsm_status_func_t exit;
} fsm_status_t;

typedef struct fsm_event_t {
    int id;
    fsm_event_func_t handle;
} fsm_event_t;

typedef struct fsm_rule_t {
    struct {
        int status;
        int event;
        int ret_code;
    } input;
    struct {
        int status;
    } output;
} fsm_rule_t;

//  finite status machine
//  input:
//      current status
//      event, and its ret code
//  output:
//      next status

// status, events, rules allocated when fsm init
typedef struct fsm_t {
    int current;
    int size;

    int status_count;
    fsm_status_t* status;
    struct idtable_t* status_table;

    // default size: size * size / 2
    int events_count;
    fsm_event_t* events;
    struct idtable_t* events_table;

    // default size: size * size * 2
    int rules_count;
    fsm_rule_t* rules;
    struct hash_t* rules_table;
} fsm_t;

uint32_t _fsm_rule_hash(const void* data)
{
    fsm_rule_t* rule = (fsm_rule_t*)data;
    return hash_jhash((void*)&rule->input, sizeof(rule->input));
}

int _fsm_rule_cmp(const void* data1, const void* data2)
{
    fsm_rule_t* rule1 = (fsm_rule_t*)data1;
    fsm_rule_t* rule2 = (fsm_rule_t*)data2;
    return (rule1->input.status < rule2->input.status)
        || ((rule1->input.status == rule2->input.status)
            && (rule1->input.event < rule2->input.event))
        || ((rule1->input.status == rule2->input.status)
            && (rule1->input.event == rule2->input.event)
            && (rule1->input.ret_code < rule2->input.ret_code));
}

#define FSM_STATUS_SIZE(nsize) (nsize)
#define FSM_EVENTS_SIZE(nsize) (((nsize) * ((nsize) - 1)) >> 1)
#define FSM_RULES_SIZE(nsize) (((nsize) * ((nsize) - 1)) << 1)

fsm_t* fsm_init(int size)
{
    fsm_t* fsm;
    if (size <= 0) {
        return NULL;
    }

    fsm = (fsm_t*)MALLOC(sizeof(fsm_t));
    assert(fsm);
    fsm->current = FSM_WILDCARD_STATUS;
    fsm->size = size;

    fsm->status_count = 0;
    fsm->status = (fsm_status_t*)MALLOC(sizeof(fsm_status_t) * FSM_STATUS_SIZE(size));
    fsm->status_table = idtable_init(FSM_STATUS_SIZE(size));
    assert(fsm->status && fsm->status_table);

    fsm->events_count = 0;
    fsm->events = (fsm_event_t*)MALLOC(sizeof(fsm_event_t) * FSM_EVENTS_SIZE(size));
    fsm->events_table = idtable_init(FSM_EVENTS_SIZE(size));
    assert(fsm->events && fsm->events_table);

    fsm->rules_count = 0;
    fsm->rules = (fsm_rule_t*)MALLOC(sizeof(fsm_rule_t) * FSM_RULES_SIZE(size));
    fsm->rules_table = hash_init(_fsm_rule_hash, _fsm_rule_cmp, FSM_RULES_SIZE(size) * 3);
    return fsm;
}

void fsm_release(struct fsm_t* fsm)
{
    if (fsm) {
        if (fsm->status_table) {
            idtable_release(fsm->status_table);
            fsm->status_table = NULL;
        }
        if (fsm->status) {
            FREE(fsm->status);
            fsm->status = NULL;
        }
        if (fsm->events_table) {
            idtable_release(fsm->events_table);
            fsm->events_table = NULL;
        }
        if (fsm->events) {
            FREE(fsm->events);
            fsm->events = NULL;
        }
        if (fsm->rules_table) {
            hash_release(fsm->rules_table);
            fsm->rules_table = NULL;
        }
        if (fsm->rules) {
            FREE(fsm->rules);
            fsm->rules = NULL;
        }
        FREE(fsm);
    }
}

int fsm_register_status(fsm_t* fsm, int status,
                        fsm_status_func_t enter,
                        fsm_status_func_t exit)
{
    fsm_status_t* st;
    int ret;

    if (!fsm || status <= 0) {
        return FSM_FAIL;
    }
    if (fsm->status_count >= FSM_STATUS_SIZE(fsm->size)) {
        return FSM_FULL;
    }
    if (idtable_get(fsm->status_table, status)) {
        return FSM_EXISTED;
    }

    st = &fsm->status[fsm->status_count ++];
    st->id = status;
    st->enter = enter;
    st->exit = exit;
    ret = idtable_add(fsm->status_table, st->id, (void*)st);
    assert(ret == 0);
    return FSM_OK;
}

int fsm_register_event(fsm_t* fsm, int event,
                       fsm_event_func_t handle)
{
    fsm_event_t* et;
    int ret;

    if (!fsm || event <= 0) {
        return FSM_FAIL;
    }
    if (fsm->events_count >= FSM_EVENTS_SIZE(fsm->size)) {
        return FSM_FULL;
    }
    if (idtable_get(fsm->events_table, event)) {
        return FSM_EXISTED;
    }

    et = &fsm->events[fsm->events_count ++];
    et->id = event;
    et->handle = handle;
    ret = idtable_add(fsm->events_table, et->id, (void*)et);
    assert(ret == 0);
    return FSM_OK;
}

int fsm_register_rule(struct fsm_t* fsm, int event, int ret_code,
                      int from_status, int to_status)
{
    fsm_rule_t* rule;
    int ret;

    if (!fsm || event < 0
        || (from_status < 0 && from_status != FSM_WILDCARD_STATUS)
        || (to_status < 0 && to_status != FSM_WILDCARD_STATUS)) {
        return FSM_FAIL;
    }
    if (fsm->rules_count >= FSM_RULES_SIZE(fsm->size)) {
        return FSM_FULL;
    }
    if (!idtable_get(fsm->events_table, event)) {
        return FSM_NOT_EXISTED;
    }
    if ((from_status != FSM_WILDCARD_STATUS
        && !idtable_get(fsm->status_table, from_status))
        || (to_status != FSM_WILDCARD_STATUS
        && !idtable_get(fsm->status_table, to_status))) {
        return FSM_NOT_EXISTED;
    }

    rule = &fsm->rules[fsm->rules_count ++];
    rule->input.status = from_status;
    rule->input.event = event;
    rule->input.ret_code = ret_code;
    rule->output.status = to_status;
    ret = hash_insert(fsm->rules_table, (void*)rule);
    if (ret < 0) {
        -- fsm->rules_count;
        return FSM_EXISTED;
    }
    return FSM_OK;
}

int fsm_start(struct fsm_t* fsm, int status)
{
    if (!fsm || !idtable_get(fsm->status_table, status)) {
        return FSM_FAIL;
    }
    fsm->current = status;
    return FSM_OK;
}

int fsm_trigger(struct fsm_t* fsm, int event, void* args)
{
    fsm_rule_t* rule;
    fsm_event_t* ev;
    fsm_status_t* st;
    fsm_rule_t tmp;

    if (!fsm) {
        return FSM_FAIL;
    }

    // event
    ev = idtable_get(fsm->events_table, event);
    if (!ev) {
        return FSM_NOT_EXISTED;
    }

    // rule
    tmp.input.status = fsm->current;
    tmp.input.event = event;
    tmp.input.ret_code = ev->handle(args);
    rule = hash_find(fsm->rules_table, (void*)&tmp);
    if (!rule) {
        // try wildcard status
        tmp.input.status = FSM_WILDCARD_STATUS;
        rule = hash_find(fsm->rules_table, (void*)&tmp);
        if (!rule) {
            return FSM_NOT_EXISTED;
        }
    }

    printf("trigger: [status %d] -> [status %d], event %d ret %d\n",
        fsm->current, rule->output.status == FSM_WILDCARD_STATUS
        ? fsm->current : rule->output.status,
        rule->input.event, rule->input.ret_code);

    // re-enter
    if (rule->output.status == fsm->current
        || rule->output.status == FSM_WILDCARD_STATUS) {
        // nothing to do ..
    } else {
        st = idtable_get(fsm->status_table, fsm->current);
        assert(st);
        if (st->exit) {
            st->exit(args);
        }

        fsm->current = (rule->output.status == FSM_WILDCARD_STATUS
            ? fsm->current : rule->output.status);

        st = idtable_get(fsm->status_table, fsm->current);
        assert(st);
        if (st->enter) {
            st->enter(args);
        }
    }

    return FSM_OK;
}

