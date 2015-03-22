#include <stdio.h>
#include <assert.h>
#include "core/fsm.h"

#define S_INIT 1
#define S_LOADING 2
#define S_PLAYING 3
#define S_LOGOUT 4

#define EV_LOGIN 1
#define EV_LOAD 2
#define EV_LOGOUT 3

void
test_fsm_init_enter(void* args) {
    printf("\tenter status [init]\n");
}
void
test_fsm_init_exit(void* args) {
    printf("\texit  status [init]\n");
}

void
test_fsm_loading_enter(void* args) {
    printf("\tenter status [loading]\n");
}
void
test_fsm_loading_exit(void* args) {
    printf("\texit  status [loading]\n");
}

void
test_fsm_playing_enter(void* args) {
    printf("\tenter status [playing]\n");
}
void
test_fsm_playing_exit(void* args) {
    printf("\texit  status [playing]\n");
}

void
test_fsm_logout_enter(void* args) {
    printf("\tenter status [logout]\n");
}
void
test_fsm_logout_exit(void* args) {
    printf("\texit  status [logout]\n");
}

int
test_fsm_handle_login(void* args) {
    return args ? FSM_FAIL : FSM_OK;
}

int
test_fsm_handle_load(void* args) {
    return FSM_OK;
}

int
test_fsm_handle_logout(void* args) {
    return FSM_OK;
}

int
test_core_fsm(char* param) {
    struct fsm_t* fsm = fsm_create(4);
    assert(fsm);

    FSM_STATUS(fsm, S_INIT, test_fsm_init_enter, test_fsm_init_exit);
    FSM_STATUS(fsm, S_LOADING, test_fsm_loading_enter, test_fsm_loading_exit);
    FSM_STATUS(fsm, S_PLAYING, test_fsm_playing_enter, test_fsm_playing_exit);
    FSM_STATUS(fsm, S_LOGOUT, test_fsm_logout_enter, test_fsm_logout_exit);

    FSM_EVENT(fsm, EV_LOGIN, test_fsm_handle_login);
    FSM_EVENT(fsm, EV_LOAD, test_fsm_handle_load);
    FSM_EVENT(fsm, EV_LOGOUT, test_fsm_handle_logout);

    FSM_RULE(fsm, EV_LOGIN, FSM_OK, S_INIT, S_LOADING);
    FSM_RULE(fsm, EV_LOGIN, FSM_OK, S_LOGOUT, S_LOADING);
    FSM_RULE(fsm, EV_LOGIN, FSM_FAIL, FSM_WILDCARD_STATUS, S_INIT);
    FSM_RULE(fsm, EV_LOGIN, FSM_OK, FSM_WILDCARD_STATUS, S_INIT);

    FSM_RULE(fsm, EV_LOAD, FSM_OK, S_LOADING, S_PLAYING);
    FSM_RULE(fsm, EV_LOAD, FSM_FAIL, S_LOADING, S_INIT);
    FSM_RULE(fsm, EV_LOAD, FSM_OK, FSM_WILDCARD_STATUS, FSM_WILDCARD_STATUS);

    FSM_RULE(fsm, EV_LOGOUT, FSM_OK, FSM_WILDCARD_STATUS, S_LOGOUT);

    int ret = fsm_start(fsm, S_INIT);
    if (ret != FSM_OK) {
        fprintf(stderr, "fsm start fail: %d\n", ret);
        return ret;
    }

    FSM_TRIGGER(fsm, EV_LOGIN, NULL);
    FSM_TRIGGER(fsm, EV_LOAD, NULL);
    FSM_TRIGGER(fsm, EV_LOAD, NULL);
    FSM_TRIGGER(fsm, EV_LOAD, NULL);
    FSM_TRIGGER(fsm, EV_LOGOUT, NULL);
    FSM_TRIGGER(fsm, EV_LOGIN, NULL);
    FSM_TRIGGER(fsm, EV_LOGOUT, NULL);
    FSM_TRIGGER(fsm, EV_LOGIN, (void*)1);
    FSM_TRIGGER(fsm, EV_LOGIN, NULL);
    FSM_TRIGGER(fsm, EV_LOAD, NULL);
    FSM_TRIGGER(fsm, EV_LOGOUT, NULL);

    fsm_release(fsm);
    return 0;
}

