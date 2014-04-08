#include <assert.h>
#include "core/thread.h"
#include "core/atom.h"

atom_t atom_flag;
uint32_t atom_cmp;
#define ATOM_LOOP 1000000

THREAD_FUNC
func(void* arg) {
    int index;
    for (index = 0; index < ATOM_LOOP; index ++) {
        //atom_inc(&atom_flag);
        atom_add(&atom_flag, 2);
        atom_cmp += 2;
    }
    THREAD_RETURN;
}

int
test_atom() {
    thread_t t1, t2;
    atom_flag = 0;
    atom_cmp = 0;
    THREAD_CREATE(t1, func, NULL);
    THREAD_CREATE(t2, func, NULL);
    THREAD_JOIN(t1);
    THREAD_JOIN(t2);
    printf("\natom_flag=%u atom_cmp=%u...\n", atom_flag, atom_cmp);
    return 0;
}

