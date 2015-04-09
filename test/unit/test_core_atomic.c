#include <assert.h>
#include "core/thread.h"
#include "core/atom.h"

static atom_t   _atom_ref = 0;
static uint32_t _int_ref = 0;

static int _loop = 1000000;

static void*
_atom_run(void* arg) {
    for (int i= 0; i < _loop; ++ i) {
        atom_add(&_atom_ref, 2);
        atom_inc(&_atom_ref);
        _int_ref += 3;
    }
    return NULL;
}

int
test_core_atomic(const char* param) {
    if (param) {
        _loop = atoi(param);
    }

    _int_ref = 10;
    atom_set(&_atom_ref, 10);

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, (1 << 20));

    pthread_t t1, t2;
    pthread_create(&t1, &attr, _atom_run, NULL);
    pthread_create(&t2, &attr, _atom_run, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("multi-thread reference[%u] by atomic\n", _atom_ref);
    printf("multi-thread reference[%u] under race\n", _int_ref);
    return 0;
}

